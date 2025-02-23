/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
    Copyright (c) 2003 Zack Rusin <zack@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqfile.h>
#include <tqtextstream.h>
#include <tqdom.h>
#include <tqregexp.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include <iostream>

static const TDECmdLineOptions options[] =
{
  { "d", 0, 0 },
  { "directory <dir>", I18N_NOOP("Directory to generate files in"), "." },
  { "+file.kcfg", I18N_NOOP("Input kcfg XML file"), 0 },
  { "+file.kcfgc", I18N_NOOP("Code generation options file"), 0 },
  TDECmdLineLastOption
};


bool globalEnums;
bool itemAccessors;
bool dpointer;
TQStringList allNames;
TQRegExp *validNameRegexp;
TQString This;
TQString Const;  

class CfgEntry
{
  public:
    struct Choice
    {
      TQString name;
      TQString label;
      TQString whatsThis;
    };

    CfgEntry( const TQString &group, const TQString &type, const TQString &key,
              const TQString &name, const TQString &label,
              const TQString &whatsThis, const TQString &code,
              const TQString &defaultValue, const TQValueList<Choice> &choices,
              bool hidden )
      : mGroup( group ), mType( type ), mKey( key ), mName( name ),
        mLabel( label ), mWhatsThis( whatsThis ), mCode( code ),
        mDefaultValue( defaultValue ),
        mChoices( choices ), mHidden( hidden )
    {
    }

    void setGroup( const TQString &group ) { mGroup = group; }
    TQString group() const { return mGroup; }

    void setType( const TQString &type ) { mType = type; }
    TQString type() const { return mType; }

    void setKey( const TQString &key ) { mKey = key; }
    TQString key() const { return mKey; }

    void setName( const TQString &name ) { mName = name; }
    TQString name() const { return mName; }

    void setLabel( const TQString &label ) { mLabel = label; }
    TQString label() const { return mLabel; }

    void setWhatsThis( const TQString &whatsThis ) { mWhatsThis = whatsThis; }
    TQString whatsThis() const { return mWhatsThis; }

    void setDefaultValue( const TQString &d ) { mDefaultValue = d; }
    TQString defaultValue() const { return mDefaultValue; }

    void setCode( const TQString &d ) { mCode = d; }
    TQString code() const { return mCode; }

    void setMinValue( const TQString &d ) { mMin = d; }
    TQString minValue() const { return mMin; }

    void setMaxValue( const TQString &d ) { mMax = d; }
    TQString maxValue() const { return mMax; }

    void setParam( const TQString &d ) { mParam = d; }
    TQString param() const { return mParam; }

    void setParamName( const TQString &d ) { mParamName = d; }
    TQString paramName() const { return mParamName; }

    void setParamType( const TQString &d ) { mParamType = d; }
    TQString paramType() const { return mParamType; }

    void setChoices( const TQValueList<Choice> &d ) { mChoices = d; }
    TQValueList<Choice> choices() const { return mChoices; }

    void setParamValues( const TQStringList &d ) { mParamValues = d; }
    TQStringList paramValues() const { return mParamValues; }

    void setParamDefaultValues( const TQStringList &d ) { mParamDefaultValues = d; }
    TQString paramDefaultValue(int i) const { return mParamDefaultValues[i]; }

    void setParamMax( int d ) { mParamMax = d; }
    int paramMax() const { return mParamMax; }

    bool hidden() const { return mHidden; }

    void dump() const
    {
      kdDebug() << "<entry>" << endl;
      kdDebug() << "  group: " << mGroup << endl;
      kdDebug() << "  type: " << mType << endl;
      kdDebug() << "  key: " << mKey << endl;
      kdDebug() << "  name: " << mName << endl;
      kdDebug() << "  label: " << mLabel << endl;
// whatsthis
      kdDebug() << "  code: " << mCode << endl;
//      kdDebug() << "  values: " << mValues.join(":") << endl;

      if (!param().isEmpty())
      {
        kdDebug() << "  param name: "<< mParamName << endl;
        kdDebug() << "  param type: "<< mParamType << endl;
        kdDebug() << "  paramvalues: " << mParamValues.join(":") << endl;
      }
      kdDebug() << "  default: " << mDefaultValue << endl;
      kdDebug() << "  hidden: " << mHidden << endl;
      kdDebug() << "  min: " << mMin << endl;
      kdDebug() << "  max: " << mMax << endl;
      kdDebug() << "</entry>" << endl;
    }

  private:
    TQString mGroup;
    TQString mType;
    TQString mKey;
    TQString mName;
    TQString mLabel;
    TQString mWhatsThis;
    TQString mCode;
    TQString mDefaultValue;
    TQString mParam;
    TQString mParamName;
    TQString mParamType;
    TQValueList<Choice> mChoices;
    TQStringList mParamValues;
    TQStringList mParamDefaultValues;
    int mParamMax;
    bool mHidden;
    TQString mMin;
    TQString mMax;
};

class Param {
public:
  TQString name;
  TQString type;
};

// returns the name of an member variable
// use itemPath to know the full path
// like using d-> in case of dpointer
static TQString varName(const TQString &n)
{
  TQString result;
  if ( !dpointer ) {
    result = "m"+n;
    result[1] = result[1].upper();
  }
  else {
    result = n;
    result[0] = result[0].lower();
  }
  return result;
}

static TQString varPath(const TQString &n)
{
  TQString result;
  if ( dpointer ) {
    result = "d->"+varName(n);
  }
  else {
    result = varName(n);
  }
  return result;
}

static TQString enumName(const TQString &n)
{
  TQString result = "Enum"+n;
  result[4] = result[4].upper();
  return result;
}

static TQString setFunction(const TQString &n, const TQString &className = TQString())
{
  TQString result = "set"+n;
  result[3] = result[3].upper();

  if ( !className.isEmpty() )
    result = className + "::" + result;
  return result;
}


static TQString getFunction(const TQString &n, const TQString &className = TQString())
{
  TQString result = n;
  result[0] = result[0].lower();

  if ( !className.isEmpty() )
    result = className + "::" + result;
  return result;
}


static void addQuotes( TQString &s )
{
  if ( s.left( 1 ) != "\"" ) s.prepend( "\"" );
  if ( s.right( 1 ) != "\"" ) s.append( "\"" );
}

static TQString quoteString( const TQString &s )
{
  TQString r = s;
  r.replace( "\\", "\\\\" );
  r.replace( "\"", "\\\"" );
  r.replace( "\r", "" );
  r.replace( "\n", "\\n\"\n\"" );
  return "\"" + r + "\"";
}

static TQString literalString( const TQString &s )
{
  bool isAscii = true;
  for(int i = s.length(); i--;)
     if (s[i].unicode() > 127) isAscii = false;

  if (isAscii)
     return "TQString::fromLatin1( " + quoteString(s) + " )";
  else
     return "TQString::fromUtf8( " + quoteString(s) + " )";
}

static TQString dumpNode(const TQDomNode &node)
{
  TQString msg;
  TQTextStream s(&msg, IO_WriteOnly );
  node.save(s, 0);

  msg = msg.simplifyWhiteSpace();
  if (msg.length() > 40)
    return msg.left(37)+"...";
  return msg;
}

static TQString filenameOnly(TQString path)
{
   int i = path.findRev('/');
   if (i >= 0)
      return path.mid(i+1);
   return path;
}

static void preProcessDefault( TQString &defaultValue, const TQString &name,
                               const TQString &type,
                               const TQValueList<CfgEntry::Choice> &choices,
                               TQString &code )
{
    if ( type == "String" && !defaultValue.isEmpty() ) {
      defaultValue = literalString(defaultValue);

    } else if ( type == "Path" && !defaultValue.isEmpty() ) {
      defaultValue = literalString( defaultValue );

    } else if ( (type == "StringList" || type == "PathList") && !defaultValue.isEmpty() ) {
      TQTextStream cpp( &code, IO_WriteOnly | IO_Append );
      if (!code.isEmpty())
         cpp << endl;

      cpp << "  TQStringList default" << name << ";" << endl;
      TQStringList defaults = TQStringList::split( ",", defaultValue );
      TQStringList::ConstIterator it;
      for( it = defaults.begin(); it != defaults.end(); ++it ) {
        cpp << "  default" << name << ".append( TQString::fromUtf8( \"" << *it << "\" ) );"
            << endl;
      }
      defaultValue = "default" + name;

    } else if ( type == "Color" && !defaultValue.isEmpty() ) {
      TQRegExp colorRe("\\d+,\\s*\\d+,\\s*\\d+");
      if (colorRe.exactMatch(defaultValue))
      {
        defaultValue = "TQColor( " + defaultValue + " )";
      }
      else
      {
        defaultValue = "TQColor( \"" + defaultValue + "\" )";
      }

    } else if ( type == "Enum" ) {
      if ( !globalEnums ) {
        TQValueList<CfgEntry::Choice>::ConstIterator it;
        for( it = choices.begin(); it != choices.end(); ++it ) {
          if ( (*it).name == defaultValue ) {
            defaultValue.prepend( enumName(name) + "::");
            break;
          }
        }
      }

    } else if ( type == "IntList" ) {
      TQTextStream cpp( &code, IO_WriteOnly | IO_Append );
      if (!code.isEmpty())
         cpp << endl;

      cpp << "  TQValueList<int> default" << name << ";" << endl;
      TQStringList defaults = TQStringList::split( ",", defaultValue );
      TQStringList::ConstIterator it;
      for( it = defaults.begin(); it != defaults.end(); ++it ) {
        cpp << "  default" << name << ".append( " << *it << " );"
            << endl;
      }
      defaultValue = "default" + name;
    }
}


CfgEntry *parseEntry( const TQString &group, const TQDomElement &element )
{
  bool defaultCode = false;
  TQString type = element.attribute( "type" );
  TQString name = element.attribute( "name" );
  TQString key = element.attribute( "key" );
  TQString hidden = element.attribute( "hidden" );
  TQString label;
  TQString whatsThis;
  TQString defaultValue;
  TQString code;
  TQString param;
  TQString paramName;
  TQString paramType;
  TQValueList<CfgEntry::Choice> choices;
  TQStringList paramValues;
  TQStringList paramDefaultValues;
  TQString minValue;
  TQString maxValue;
  int paramMax = 0;

  TQDomNode n;
  for ( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement e = n.toElement();
    TQString tag = e.tagName();
    if ( tag == "label" ) label = e.text();
    else if ( tag == "whatsthis" ) whatsThis = e.text();
    else if ( tag == "min" ) minValue = e.text();
    else if ( tag == "max" ) maxValue = e.text();
    else if ( tag == "code" ) code = e.text();
    else if ( tag == "parameter" )
    {
      param = e.attribute( "name" );
      paramType = e.attribute( "type" );
      if ( param.isEmpty() ) {
        kdError() << "Parameter must have a name: " << dumpNode(e) << endl;
        return 0;
      }
      if ( paramType.isEmpty() ) {
        kdError() << "Parameter must have a type: " << dumpNode(e) << endl;
        return 0;
      }
      if ((paramType == "Int") || (paramType == "UInt"))
      {
         bool ok;
         paramMax = e.attribute("max").toInt(&ok);
         if (!ok)
         {
           kdError() << "Integer parameter must have a maximum (e.g. max=\"0\"): " << dumpNode(e) << endl;
           return 0;
         }
      }
      else if (paramType == "Enum")
      {
         TQDomNode n2;
         for ( n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
           TQDomElement e2 = n2.toElement();
           if (e2.tagName() == "values")
           {
             TQDomNode n3;
             for ( n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling() ) {
               TQDomElement e3 = n3.toElement();
               if (e3.tagName() == "value")
               {
                  paramValues.append( e3.text() );
               }
             }
             break;
           }
         }
         if (paramValues.isEmpty())
         {
           kdError() << "No values specified for parameter '" << param << "'." << endl;
           return 0;
         }
         paramMax = paramValues.count()-1;
      }
      else
      {
        kdError() << "Parameter '" << param << "' has type " << paramType << " but must be of type int, uint or Enum." << endl;
        return 0;
      }
    }
    else if ( tag == "default" )
    {
      if (e.attribute("param").isEmpty())
      {
        defaultValue = e.text();
        if (e.attribute( "code" ) == "true")
          defaultCode = true;
      }
    }
    else if ( tag == "choices" ) {
      TQDomNode n2;
      for( n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
        TQDomElement e2 = n2.toElement();
        if ( e2.tagName() == "choice" ) {
          TQDomNode n3;
          CfgEntry::Choice choice;
          choice.name = e2.attribute( "name" );
          if ( choice.name.isEmpty() ) {
            kdError() << "Tag <choice> requires attribute 'name'." << endl;
          }
          for( n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling() ) {
            TQDomElement e3 = n3.toElement();
            if ( e3.tagName() == "label" ) choice.label = e3.text();
            if ( e3.tagName() == "whatsthis" ) choice.whatsThis = e3.text();
          }
          choices.append( choice );
        }
      }
    }
  }

  bool nameIsEmpty = name.isEmpty();
  if ( nameIsEmpty && key.isEmpty() ) {
    kdError() << "Entry must have a name or a key: " << dumpNode(element) << endl;
    return 0;
  }

  if ( key.isEmpty() ) {
    key = name;
  }

  if ( nameIsEmpty ) {
    name = key;
    name.replace( " ", TQString() );
  } else if ( name.contains( ' ' ) ) {
    kdWarning()<<"Entry '"<<name<<"' contains spaces! <name> elements can't contain speces!"<<endl;
    name.remove( ' ' );
  }

  if (name.contains("$("))
  {
    if (param.isEmpty())
    {
      kdError() << "Name may not be parameterized: " << name << endl;
      return 0;
    }
  }
  else
  {
    if (!param.isEmpty())
    {
      kdError() << "Name must contain '$(" << param << ")': " << name << endl;
      return 0;
    }
  }

  if ( label.isEmpty() ) {
    label = key;
  }

  if ( type.isEmpty() ) type = "String"; // XXX : implicit type might be bad

  if (!param.isEmpty())
  {
    // Adjust name
    paramName = name;
    name.replace("$("+param+")", TQString());
    // Lookup defaults for indexed entries
    for(int i = 0; i <= paramMax; i++)
    {
      paramDefaultValues.append(TQString());
    }

    TQDomNode n;
    for ( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
      TQDomElement e = n.toElement();
      TQString tag = e.tagName();
      if ( tag == "default" )
      {
        TQString index = e.attribute("param");
        if (index.isEmpty())
           continue;

        bool ok;
        int i = index.toInt(&ok);
        if (!ok)
        {
          i = paramValues.findIndex(index);
          if (i == -1)
          {
            kdError() << "Index '" << index << "' for default value is unknown." << endl;
            return 0;
          }
        }

        if ((i < 0) || (i > paramMax))
        {
          kdError() << "Index '" << i << "' for default value is out of range [0, "<< paramMax<<"]." << endl;
          return 0;
        }

        TQString tmpDefaultValue = e.text();

        if (e.attribute( "code" ) != "true")
           preProcessDefault(tmpDefaultValue, name, type, choices, code);

        paramDefaultValues[i] = tmpDefaultValue;
      }
    }
  }

  if (!validNameRegexp->exactMatch(name))
  {
    if (nameIsEmpty)
      kdError() << "The key '" << key << "' can not be used as name for the entry because "
                   "it is not a valid name. You need to specify a valid name for this entry." << endl;
    else
      kdError() << "The name '" << name << "' is not a valid name for an entry." << endl;
    return 0;
  }

  if (allNames.contains(name))
  {
    if (nameIsEmpty)
      kdError() << "The key '" << key << "' can not be used as name for the entry because "
                   "it does not result in a unique name. You need to specify a unique name for this entry." << endl;
    else
      kdError() << "The name '" << name << "' is not unique." << endl;
    return 0;
  }
  allNames.append(name);

  if (!defaultCode)
  {
    preProcessDefault(defaultValue, name, type, choices, code);
  }

  CfgEntry *result = new CfgEntry( group, type, key, name, label, whatsThis,
                                   code, defaultValue, choices,
                                   hidden == "true" );
  if (!param.isEmpty())
  {
    result->setParam(param);
    result->setParamName(paramName);
    result->setParamType(paramType);
    result->setParamValues(paramValues);
    result->setParamDefaultValues(paramDefaultValues);
    result->setParamMax(paramMax);
  }
  result->setMinValue(minValue);
  result->setMaxValue(maxValue);

  return result;
}

/**
  Return parameter declaration for given type.
*/
TQString param( const TQString &type )
{
    if ( type == "String" )           return "const TQString &";
    else if ( type == "StringList" )  return "const TQStringList &";
    else if ( type == "Font" )        return "const TQFont &";
    else if ( type == "Rect" )        return "const TQRect &";
    else if ( type == "Size" )        return "const TQSize &";
    else if ( type == "Color" )       return "const TQColor &";
    else if ( type == "Point" )       return "const TQPoint &";
    else if ( type == "Int" )         return "int";
    else if ( type == "UInt" )        return "uint";
    else if ( type == "Bool" )        return "bool";
    else if ( type == "Double" )      return "double";
    else if ( type == "DateTime" )    return "const TQDateTime &";
    else if ( type == "Int64" )       return "TQ_INT64";
    else if ( type == "UInt64" )      return "TQ_UINT64";
    else if ( type == "IntList" )     return "const TQValueList<int> &";
    else if ( type == "Enum" )        return "int";
    else if ( type == "Path" )        return "const TQString &";
    else if ( type == "PathList" )    return "const TQStringList &";
    else if ( type == "Password" )    return "const TQString &";
    else {
        kdError() <<"tdeconfig_compiler does not support type \""<< type <<"\""<<endl;
        return "TQString"; //For now, but an assert would be better
    }
}

/**
  Actual C++ storage type for given type.
*/
TQString cppType( const TQString &type )
{
    if ( type == "String" )           return "TQString";
    else if ( type == "StringList" )  return "TQStringList";
    else if ( type == "Font" )        return "TQFont";
    else if ( type == "Rect" )        return "TQRect";
    else if ( type == "Size" )        return "TQSize";
    else if ( type == "Color" )       return "TQColor";
    else if ( type == "Point" )       return "TQPoint";
    else if ( type == "Int" )         return "int";
    else if ( type == "UInt" )        return "uint";
    else if ( type == "Bool" )        return "bool";
    else if ( type == "Double" )      return "double";
    else if ( type == "DateTime" )    return "TQDateTime";
    else if ( type == "Int64" )       return "TQ_INT64";
    else if ( type == "UInt64" )      return "TQ_UINT64";
    else if ( type == "IntList" )     return "TQValueList<int>";
    else if ( type == "Enum" )        return "int";
    else if ( type == "Path" )        return "TQString";
    else if ( type == "PathList" )    return "TQStringList";
    else if ( type == "Password" )    return "TQString";
    else {
        kdError()<<"tdeconfig_compiler does not support type \""<< type <<"\""<<endl;
        return "TQString"; //For now, but an assert would be better
    }
}

TQString defaultValue( const TQString &type )
{
    if ( type == "String" )           return "\"\""; // Use empty string, not null string!
    else if ( type == "StringList" )  return "TQStringList()";
    else if ( type == "Font" )        return "TDEGlobalSettings::generalFont()";
    else if ( type == "Rect" )        return "TQRect()";
    else if ( type == "Size" )        return "TQSize()";
    else if ( type == "Color" )       return "TQColor(128, 128, 128)";
    else if ( type == "Point" )       return "TQPoint()";
    else if ( type == "Int" )         return "0";
    else if ( type == "UInt" )        return "0";
    else if ( type == "Bool" )        return "false";
    else if ( type == "Double" )      return "0.0";
    else if ( type == "DateTime" )    return "TQDateTime()";
    else if ( type == "Int64" )       return "0";
    else if ( type == "UInt64" )      return "0";
    else if ( type == "IntList" )     return "TQValueList<int>()";
    else if ( type == "Enum" )        return "0";
    else if ( type == "Path" )        return "\"\""; // Use empty string, not null string!
    else if ( type == "PathList" )    return "TQStringList()";
    else if ( type == "Password" )    return "\"\""; // Use empty string, not null string!
    else {
        kdWarning()<<"Error, tdeconfig_compiler doesn't support the \""<< type <<"\" type!"<<endl;
        return "TQString"; //For now, but an assert would be better
    }
}

TQString itemType( const TQString &type )
{
  TQString t;

  t = type;
  t.replace( 0, 1, t.left( 1 ).upper() );

  return t;
}

static TQString itemDeclaration(const CfgEntry *e)
{
  if (itemAccessors)
     return TQString();

  TQString fCap = e->name();
  fCap[0] = fCap[0].upper();
  return "  TDEConfigSkeleton::Item"+itemType( e->type() ) +
         "  *item" + fCap +
         ( (!e->param().isEmpty())?(TQString("[%1]").arg(e->paramMax()+1)) : TQString()) +
         ";\n";
}

// returns the name of an item variable
// use itemPath to know the full path
// like using d-> in case of dpointer
static TQString itemVar(const CfgEntry *e)
{
  TQString result;
  if (itemAccessors)
  {
    if ( !dpointer )  
    {
      result = "m" + e->name() + "Item";
      result[1] = result[1].upper();
    }
    else
    {
      result = e->name() + "Item";
      result[0] = result[0].lower();
    }
  }
  else
  {
    result = "item" + e->name();
    result[4] = result[4].upper();
  }
  return result;
}

static TQString itemPath(const CfgEntry *e)
{
  TQString result;
  if ( dpointer ) {
    result = "d->"+itemVar(e);
  }
  else {
    result = itemVar(e);
  }
  return result;
}

TQString newItem( const TQString &type, const TQString &name, const TQString &key,
                 const TQString &defaultValue, const TQString &param = TQString())
{
  TQString t = "new TDEConfigSkeleton::Item" + itemType( type ) +
              "( currentGroup(), " + key + ", " + varPath( name ) + param;
  if ( type == "Enum" ) t += ", values" + name;
  if ( !defaultValue.isEmpty() ) {
    t += ", ";
    if ( type == "String" ) t += defaultValue;
    else t+= defaultValue;
  }
  t += " );";

  return t;
}

TQString paramString(const TQString &s, const CfgEntry *e, int i)
{
  TQString result = s;
  TQString needle = "$("+e->param()+")";
  if (result.contains(needle))
  {
    TQString tmp;
    if (e->paramType() == "Enum")
    {
      tmp = e->paramValues()[i];
    }
    else
    {
      tmp = TQString::number(i);
    }

    result.replace(needle, tmp);
  }
  return result;
}

TQString paramString(const TQString &group, const TQValueList<Param> &parameters)
{
  TQString paramString = group;
  TQString arguments;
  int i = 1;
  for (TQValueList<Param>::ConstIterator it = parameters.begin();
       it != parameters.end(); ++it)
  {
     if (paramString.contains("$("+(*it).name+")"))
     {
       TQString tmp;
       tmp.sprintf("%%%d", i++);
       paramString.replace("$("+(*it).name+")", tmp);
       arguments += ".arg( mParam"+(*it).name+" )";
     }
  }
  if (arguments.isEmpty())
    return "TQString::fromLatin1( \""+group+"\" )";

  return "TQString::fromLatin1( \""+paramString+"\" )"+arguments;
}

/* int i is the value of the parameter */
TQString userTextsFunctions( CfgEntry *e, TQString itemVarStr=TQString(), TQString i=TQString() )
{
  TQString txt;
  if (itemVarStr.isNull()) itemVarStr=itemPath(e);
  if ( !e->label().isEmpty() ) {
    txt += "  " + itemVarStr + "->setLabel( i18n(";
    if ( !e->param().isEmpty() )
      txt += quoteString(e->label().replace("$("+e->param()+")", i));
    else
      txt+= quoteString(e->label());
    txt+= ") );\n";
  }
  if ( !e->whatsThis().isEmpty() ) {
    txt += "  " + itemVarStr + "->setWhatsThis( i18n(";
    if ( !e->param().isEmpty() )
      txt += quoteString(e->whatsThis().replace("$("+e->param()+")", i));
    else
      txt+= quoteString(e->whatsThis());
    txt+=") );\n";
  }
  return txt;
}

// returns the member accesor implementation
// which should go in the h file if inline
// or the cpp file if not inline
TQString memberAccessorBody( CfgEntry *e )
{    
    TQString result;
    TQTextStream out(&result, IO_WriteOnly);
    TQString n = e->name();
    TQString t = e->type();

    out << "return " << This << varPath(n);
    if (!e->param().isEmpty()) out << "[i]";
    out << ";" << endl;
   
    return result;
}

// returns the member mutator implementation
// which should go in the h file if inline
// or the cpp file if not inline
TQString memberMutatorBody( CfgEntry *e )
{
  TQString result;
  TQTextStream out(&result, IO_WriteOnly);
  TQString n = e->name();
  TQString t = e->type();

  if (!e->minValue().isEmpty())
  {
    out << "if (v < " << e->minValue() << ")" << endl;
    out << "{" << endl;
    out << "  kdDebug() << \"" << setFunction(n);
    out << ": value \" << v << \" is less than the minimum value of ";
    out << e->minValue()<< "\" << endl;" << endl;
    out << "  v = " << e->minValue() << ";" << endl;
    out << "}" << endl;
  }
  
  if (!e->maxValue().isEmpty())
  {
    out << endl << "if (v > " << e->maxValue() << ")" << endl;
    out << "{" << endl;
    out << "  kdDebug() << \"" << setFunction(n);
    out << ": value \" << v << \" is greater than the maximum value of ";
    out << e->maxValue()<< "\" << endl;" << endl;
    out << "  v = " << e->maxValue() << ";" << endl;
    out << "}" << endl << endl;
  }

  out << "if (!" << This << "isImmutable( TQString::fromLatin1( \"";
  if (!e->param().isEmpty())
  {
    out << e->paramName().replace("$("+e->param()+")", "%1") << "\" ).arg( ";
    if ( e->paramType() == "Enum" ) {
      out << "TQString::fromLatin1( ";

      if (globalEnums)
        out << enumName(e->param()) << "ToString[i]";
      else
        out << enumName(e->param()) << "::enumToString[i]";
        
        out << " )";
    }
    else
    {
      out << "i";
    }
    out << " )";
  }
  else
  {
    out << n << "\" )";
  }
  out << " ))" << endl;
  out << "  " << This << varPath(n);
  if (!e->param().isEmpty())
    out << "[i]";
  out << " = v;" << endl;    

  return result;
}

// returns the item accesor implementation
// which should go in the h file if inline
// or the cpp file if not inline
TQString itemAccessorBody( CfgEntry *e )
{    
    TQString result;
    TQTextStream out(&result, IO_WriteOnly);

    out << "return " << itemPath(e);
    if (!e->param().isEmpty()) out << "[i]";
    out << ";" << endl;

    return result;
}

//indents text adding X spaces per line
TQString indent(TQString text, int spaces)
{
    TQString result;
    TQTextStream out(&result, IO_WriteOnly);
    TQTextStream in(&text, IO_ReadOnly);
    TQString currLine;
    while ( !in.atEnd() )
    {
      currLine = in.readLine();
      if (!currLine.isEmpty())
        for (int i=0; i < spaces; i++)
          out << " ";
      out << currLine << endl;
    }
    return result;
}


int main( int argc, char **argv )
{
  TDEAboutData aboutData( "tdeconfig_compiler", I18N_NOOP("TDE .kcfg compiler"), "0.3",
    I18N_NOOP("TDEConfig Compiler") , TDEAboutData::License_LGPL );
  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  aboutData.addAuthor( "Waldo Bastian", 0, "bastian@kde.org" );
  aboutData.addAuthor( "Zack Rusin", 0, "zack@kde.org" );
  aboutData.addCredit( "Reinhold Kainhofer", "Fix for parametrized entries",
      "reinhold@kainhofer.com", "http://reinhold.kainhofer.com" );
  aboutData.addCredit( "Duncan Mac-Vicar P.", "dpointer support",
      "duncan@kde.org", "http://www.mac-vicar.com/~duncan" );

  TDECmdLineArgs::init( argc, argv, &aboutData );
  TDECmdLineArgs::addCmdLineOptions( options );

  TDEInstance app( &aboutData );

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

  if ( args->count() < 2 ) {
    kdError() << "Too few arguments." << endl;
    return 1;
  }
  if ( args->count() > 2 ) {
    kdError() << "Too many arguments." << endl;
    return 1;
  }

  validNameRegexp = new TQRegExp("[a-zA-Z_][a-zA-Z0-9_]*");

  TQString baseDir = TQFile::decodeName(args->getOption("directory"));
  if (!baseDir.endsWith("/"))
    baseDir.append("/");

  TQString inputFilename = args->url( 0 ).path();
  TQString codegenFilename = args->url( 1 ).path();

  if (!codegenFilename.endsWith(".kcfgc"))
  {
    kdError() << "Codegen options file must have extension .kcfgc" << endl;
    return 1;
  }
  TQString baseName = args->url( 1 ).fileName();
  baseName = baseName.left(baseName.length() - 6);

  KSimpleConfig codegenConfig( codegenFilename, true );

  TQString nameSpace = codegenConfig.readEntry("NameSpace");
  TQString className = codegenConfig.readEntry("ClassName");
  TQString inherits = codegenConfig.readEntry("Inherits");
  TQString visibility = codegenConfig.readEntry("Visibility");
  if (!visibility.isEmpty()) visibility+=" ";
  bool singleton = codegenConfig.readBoolEntry("Singleton", false);
  bool staticAccessors = singleton;
  //bool useDPointer = codegenConfig.readBoolEntry("DPointer", false);
  bool customAddons = codegenConfig.readBoolEntry("CustomAdditions");
  TQString memberVariables = codegenConfig.readEntry("MemberVariables");
  TQStringList headerIncludes = codegenConfig.readListEntry("IncludeFiles");
  TQStringList mutators = codegenConfig.readListEntry("Mutators");
  bool allMutators = false;
  if ((mutators.count() == 1) && (mutators[0].lower() == "true"))
     allMutators = true;
  itemAccessors = codegenConfig.readBoolEntry( "ItemAccessors", false );
  bool setUserTexts = codegenConfig.readBoolEntry( "SetUserTexts", false );

  globalEnums = codegenConfig.readBoolEntry( "GlobalEnums", false );

  dpointer = (memberVariables == "dpointer");

  TQFile input( inputFilename );

  TQDomDocument doc;
  TQString errorMsg;
  int errorRow;
  int errorCol;
  if ( !doc.setContent( &input, &errorMsg, &errorRow, &errorCol ) ) {
    kdError() << "Unable to load document." << endl;
    kdError() << "Parse error in " << args->url( 0 ).fileName() << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg << endl;
    return 1;
  }

  TQDomElement cfgElement = doc.documentElement();

  if ( cfgElement.isNull() ) {
    kdError() << "No document in kcfg file" << endl;
    return 1;
  }

  TQString cfgFileName;
  bool cfgFileNameArg = false;
  TQValueList<Param> parameters;
  TQStringList includes;

  TQPtrList<CfgEntry> entries;
  entries.setAutoDelete( true );

  TQDomNode n;
  for ( n = cfgElement.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement e = n.toElement();

    TQString tag = e.tagName();

    if ( tag == "include" ) {
      TQString includeFile = e.text();
      if (!includeFile.isEmpty())
        includes.append(includeFile);

    } else if ( tag == "kcfgfile" ) {
      cfgFileName = e.attribute( "name" );
      cfgFileNameArg = e.attribute( "arg" ).lower() == "true";
      TQDomNode n2;
      for( n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
        TQDomElement e2 = n2.toElement();
        if ( e2.tagName() == "parameter" ) {
          Param p;
          p.name = e2.attribute( "name" );
          p.type = e2.attribute( "type" );
          if (p.type.isEmpty())
             p.type = "String";
          parameters.append( p );
        }
      }

    } else if ( tag == "group" ) {
      TQString group = e.attribute( "name" );
      if ( group.isEmpty() ) {
        kdError() << "Group without name" << endl;
        return 1;
      }
      TQDomNode n2;
      for( n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
        TQDomElement e2 = n2.toElement();
        if ( e2.tagName() != "entry" ) continue;
        CfgEntry *entry = parseEntry( group, e2 );
        if ( entry ) entries.append( entry );
        else {
          kdError() << "Can't parse entry." << endl;
          return 1;
        }
      }
    }
  }

  if ( inherits.isEmpty() ) inherits = "TDEConfigSkeleton";

  if ( className.isEmpty() ) {
    kdError() << "Class name missing" << endl;
    return 1;
  }

  if ( singleton && !parameters.isEmpty() ) {
    kdError() << "Singleton class can not have parameters" << endl;
    return 1;
  }

  if ( !cfgFileName.isEmpty() && cfgFileNameArg)
  {
    kdError() << "Having both a fixed filename and a filename as argument is not possible." << endl;
    return 1;
  }

  if ( entries.isEmpty() ) {
    kdWarning() << "No entries." << endl;
  }

#if 0
  CfgEntry *cfg;
  for( cfg = entries.first(); cfg; cfg = entries.next() ) {
    cfg->dump();
  }
#endif

  TQString headerFileName = baseName + ".h";
  TQString implementationFileName = baseName + ".cpp";
  TQString cppPreamble; // code to be inserted at the beginnin of the cpp file, e.g. initialization of static values

  TQFile header( baseDir + headerFileName );
  if ( !header.open( IO_WriteOnly ) ) {
    kdError() << "Can't open '" << headerFileName << "' for writing." << endl;
    return 1;
  }

  TQTextStream h( &header );

  h << "// This file is generated by tdeconfig_compiler from " << args->url(0).fileName() << "." << endl;
  h << "// All changes you do to this file will be lost." << endl;

  h << "#ifndef " << ( !nameSpace.isEmpty() ? nameSpace.upper() + "_" : "" )
    << className.upper() << "_H" << endl;
  h << "#define " << ( !nameSpace.isEmpty() ? nameSpace.upper() + "_" : "" )
    << className.upper() << "_H" << endl << endl;

  // Includes
  TQStringList::ConstIterator it;
  for( it = headerIncludes.begin(); it != headerIncludes.end(); ++it ) {
    h << "#include <" << *it << ">" << endl;
  }

  if ( headerIncludes.count() > 0 ) h << endl;

  if ( !singleton && cfgFileNameArg && parameters.isEmpty() )
    h << "#include <tdeglobal.h>" << endl;

  h << "#include <tdeconfigskeleton.h>" << endl;
  h << "#include <kdebug.h>" << endl << endl;

  // Includes
  for( it = includes.begin(); it != includes.end(); ++it ) {
    h << "#include <" << *it << ">" << endl;
  }


  if ( !nameSpace.isEmpty() )
    h << "namespace " << nameSpace << " {" << endl << endl;

  // Private class declaration
  if ( dpointer )
    h << "class " << className << "Private;" << endl << endl;

  // Class declaration header
  h << "class " << visibility << className << " : public " << inherits << endl;
  h << "{" << endl;
  h << "  public:" << endl;

  // enums
  CfgEntry *e;
  for( e = entries.first(); e; e = entries.next() ) {
    TQValueList<CfgEntry::Choice> choices = e->choices();
    if ( !choices.isEmpty() ) {
      TQStringList values;
      TQValueList<CfgEntry::Choice>::ConstIterator itChoice;
      for( itChoice = choices.begin(); itChoice != choices.end(); ++itChoice ) {
        values.append( (*itChoice).name );
      }
      if ( globalEnums ) {
        h << "    enum { " << values.join( ", " ) << " };" << endl;
      } else {
        h << "    class " << enumName(e->name()) << endl;
        h << "    {" << endl;
        h << "      public:" << endl;
        h << "      enum type { " << values.join( ", " ) << ", COUNT };" << endl;
        h << "    };" << endl;
      }
    }
    TQStringList values = e->paramValues();
    if ( !values.isEmpty() ) {
      if ( globalEnums ) {
        h << "    enum { " << values.join( ", " ) << " };" << endl;
        h << "    static const char* const " << enumName(e->param()) << "ToString[];" << endl;
        cppPreamble += "const char* const " + className + "::" + enumName(e->param()) + "ToString[] = " +
            "{ \"" + values.join( "\", \"" ) + "\" };\n";
      } else {
        h << "    class " << enumName(e->param()) << endl;
        h << "    {" << endl;
        h << "      public:" << endl;
        h << "      enum type { " << values.join( ", " ) << ", COUNT };" << endl;
        h << "      static const char* const enumToString[];" << endl;
        h << "    };" << endl;
        cppPreamble += "const char* const " + className + "::" + enumName(e->param()) + "::enumToString[] = " +
            "{ \"" + values.join( "\", \"" ) + "\" };\n";
      }
    }
  }

  h << endl;

  // Constructor or singleton accessor
  if ( !singleton ) {
    h << "    " << className << "(";
    if (cfgFileNameArg)
       h << " TDESharedConfig::Ptr config" << (parameters.isEmpty() ? " = TDEGlobal::sharedConfig()" : ", ");
    for (TQValueList<Param>::ConstIterator it = parameters.begin();
         it != parameters.end(); ++it)
    {
       if (it != parameters.begin())
         h << ",";
       h << " " << param((*it).type) << " " << (*it).name;
    }
    h << " );" << endl;
  } else {
    h << "    static " << className << " *self();" << endl;
    if (cfgFileNameArg)
      h << "    static void instance(const char * cfgfilename);" << endl;
  }

  // Destructor
  h << "    ~" << className << "();" << endl << endl;

  // global variables
  if (staticAccessors)
    This = "self()->";
  else
    Const = " const";

  for( e = entries.first(); e; e = entries.next() ) {
    TQString n = e->name();
    TQString t = e->type();

    // Manipulator
    if (allMutators || mutators.contains(n))
    {
      h << "    /**" << endl;
      h << "      Set " << e->label() << endl;
      h << "    */" << endl;
      if (staticAccessors)
        h << "    static" << endl;
      h << "    void " << setFunction(n) << "( ";
      if (!e->param().isEmpty())
        h << cppType(e->paramType()) << " i, ";
      h << param( t ) << " v )";
      // function body inline only if not using dpointer
      // for BC mode
      if ( !dpointer )
      {
        h << endl << "    {" << endl;
        h << indent(memberMutatorBody(e), 6 );      
        h << "    }" << endl;
      }
      else
      {
        h << ";" << endl;
      }
    }
    h << endl;
    // Accessor
    h << "    /**" << endl;
    h << "      Get " << e->label() << endl;
    h << "    */" << endl;
    if (staticAccessors)
      h << "    static" << endl;
    h << "    " << cppType(t) << " " << getFunction(n) << "(";
    if (!e->param().isEmpty())
      h << " " << cppType(e->paramType()) <<" i ";
    h << ")" << Const;
    // function body inline only if not using dpointer
    // for BC mode
    if ( !dpointer )
    {
       h << endl << "    {" << endl;
      h << indent(memberAccessorBody(e), 6 );      
       h << "    }" << endl;
    }
    else
    {
      h << ";" << endl;
    }

    // Item accessor
    if ( itemAccessors ) {
      h << endl;
      h << "    /**" << endl;
      h << "      Get Item object corresponding to " << n << "()"
        << endl;
      h << "    */" << endl;
      h << "    Item" << itemType( e->type() ) << " *"
        << getFunction( n ) << "Item(";
      if (!e->param().isEmpty()) {
        h << " " << cppType(e->paramType()) << " i ";
      }
      h << ")";
      if (! dpointer )
      {
        h << endl << "    {" << endl;
        h << indent( itemAccessorBody(e), 6);
        h << "    }" << endl;
      }
      else
      {
        h << ";" << endl;
      }
    }

    h << endl;
  }

  // Static writeConfig method for singleton
  if ( singleton ) {
    h << "    static" << endl;
    h << "    void writeConfig()" << endl;
    h << "    {" << endl;
    h << "      static_cast<TDEConfigSkeleton*>(self())->writeConfig();" << endl;
    h << "    }" << endl;
  }

  h << "  protected:" << endl;

  // Private constructor for singleton
  if ( singleton ) {
    h << "    " << className << "(";
    if ( cfgFileNameArg )
      h << "const char *arg";
    h << ");" << endl;
    h << "    static " << className << " *mSelf;" << endl << endl;
  }

  // Member variables
  if ( !memberVariables.isEmpty() && memberVariables != "private" && memberVariables != "dpointer") {
    h << "  " << memberVariables << ":" << endl;
  }

  // Class Parameters
  for (TQValueList<Param>::ConstIterator it = parameters.begin();
       it != parameters.end(); ++it)
  {
     h << "    " << cppType((*it).type) << " mParam" << (*it).name << ";" << endl;
  }

  if ( memberVariables != "dpointer" )
  {
    TQString group;
    for( e = entries.first(); e; e = entries.next() ) {
      if ( e->group() != group ) {
        group = e->group();
        h << endl;
        h << "    // " << group << endl;
      }
      h << "    " << cppType(e->type()) << " " << varName(e->name());
      if (!e->param().isEmpty())
      {
        h << TQString("[%1]").arg(e->paramMax()+1);
      }
      h << ";" << endl;
    }

    h << endl << "  private:" << endl;
    if ( itemAccessors ) {
      for( e = entries.first(); e; e = entries.next() ) {
        h << "    Item" << itemType( e->type() ) << " *" << itemVar( e );
        if (!e->param().isEmpty() ) h << TQString("[%1]").arg( e->paramMax()+1 );
        h << ";" << endl;
      }
    }
  
  }
  else
  {
    // use a private class for both member variables and items
    h << "  private:" << endl;
    h << "    " + className + "Private *d;" << endl;
  }
  
  if (customAddons)
  {
     h << "    // Include custom additions" << endl;
     h << "    #include \"" << filenameOnly(baseName) << "_addons.h\"" <<endl;
  }

  h << "};" << endl << endl;

  if ( !nameSpace.isEmpty() ) h << "}" << endl << endl;

  h << "#endif" << endl << endl;


  header.close();

  TQFile implementation( baseDir + implementationFileName );
  if ( !implementation.open( IO_WriteOnly ) ) {
    kdError() << "Can't open '" << implementationFileName << "' for writing."
              << endl;
    return 1;
  }

  TQTextStream cpp( &implementation );


  cpp << "// This file is generated by tdeconfig_compiler from " << args->url(0).fileName() << "." << endl;
  cpp << "// All changes you do to this file will be lost." << endl << endl;

  cpp << "#include \"" << headerFileName << "\"" << endl << endl;

  if ( setUserTexts ) cpp << "#include <tdelocale.h>" << endl << endl;

  // Header required by singleton implementation
  if ( singleton )
    cpp << "#include <kstaticdeleter.h>" << endl << endl;
  if ( singleton && cfgFileNameArg )
    cpp << "#include <kdebug.h>" << endl << endl;

  if ( !nameSpace.isEmpty() )
    cpp << "using namespace " << nameSpace << ";" << endl << endl;

  TQString group;

  // private class implementation
  if ( dpointer )
  {
    cpp << "class " << className << "Private" << endl;
    cpp << "{" << endl;
    cpp << "  public:" << endl;
    for( e = entries.first(); e; e = entries.next() ) {
      if ( e->group() != group ) {
        group = e->group();
        cpp << endl;
        cpp << "    // " << group << endl;
      }
      cpp << "    " << cppType(e->type()) << " " << varName(e->name());
      if (!e->param().isEmpty())
      {
        cpp << TQString("[%1]").arg(e->paramMax()+1);
      }
      cpp << ";" << endl;
    }
    cpp << endl << "    // items" << endl;
    for( e = entries.first(); e; e = entries.next() ) {
      cpp << "    TDEConfigSkeleton::Item" << itemType( e->type() ) << " *" << itemVar( e );
      if (!e->param().isEmpty() ) cpp << TQString("[%1]").arg( e->paramMax()+1 );
        cpp << ";" << endl;
    }

    cpp << "};" << endl << endl;
  }

  // Singleton implementation
  if ( singleton ) {
    cpp << className << " *" << className << "::mSelf = 0;" << endl;
    cpp << "static KStaticDeleter<" << className << "> static" << className << "Deleter;" << endl << endl;

    cpp << className << " *" << className << "::self()" << endl;
    cpp << "{" << endl;
    if ( cfgFileNameArg ) {
      cpp << "  if (!mSelf)" << endl;
      cpp << "     kdFatal() << \"you need to call " << className << "::instance before using\" << endl;" << endl;
    } else {
    cpp << "  if ( !mSelf ) {" << endl;
    cpp << "    static" << className << "Deleter.setObject( mSelf, new " << className << "() );" << endl;
    cpp << "    mSelf->readConfig();" << endl;
    cpp << "  }" << endl << endl;
    }
    cpp << "  return mSelf;" << endl;
    cpp << "}" << endl << endl;

    if ( cfgFileNameArg ) {
      cpp << "void " << className << "::instance(const char *cfgfilename)" << endl;
      cpp << "{" << endl;
      cpp << "  if (mSelf) {" << endl;
      cpp << "     kdError() << \"" << className << "::instance called after the first use - ignoring\" << endl;" << endl;
      cpp << "     return;" << endl;
      cpp << "  }" << endl;
      cpp << "  static" << className << "Deleter.setObject( mSelf, new " << className << "(cfgfilename) );" << endl;
      cpp << "  mSelf->readConfig();" << endl;
      cpp << "}" << endl << endl;
    }
  }

  if ( !cppPreamble.isEmpty() )
    cpp << cppPreamble << endl;

  // Constructor
  cpp << className << "::" << className << "( ";
  if ( cfgFileNameArg ) {
    if ( !singleton )
      cpp << " TDESharedConfig::Ptr config";
    else
      cpp << " const char *config";
    cpp << (parameters.isEmpty() ? " " : ", ");
  }

  for (TQValueList<Param>::ConstIterator it = parameters.begin();
       it != parameters.end(); ++it)
  {
     if (it != parameters.begin())
       cpp << ",";
     cpp << " " << param((*it).type) << " " << (*it).name;
  }
  cpp << " )" << endl;

  cpp << "  : " << inherits << "(";
  if ( !cfgFileName.isEmpty() ) cpp << " TQString::fromLatin1( \"" << cfgFileName << "\" ";
  if ( cfgFileNameArg ) cpp << " config ";
  if ( !cfgFileName.isEmpty() ) cpp << ") ";
  cpp << ")" << endl;

  // Store parameters
  for (TQValueList<Param>::ConstIterator it = parameters.begin();
       it != parameters.end(); ++it)
  {
     cpp << "  , mParam" << (*it).name << "(" << (*it).name << ")" << endl;
  }

  cpp << "{" << endl;

  if (dpointer)
    cpp << "  d = new " + className + "Private;" << endl;
  // Needed in case the singleton class is used as baseclass for
  // another singleton.
  if ( singleton )
    cpp << "  mSelf = this;" << endl;

  group = TQString();
  for( e = entries.first(); e; e = entries.next() ) {
    if ( e->group() != group ) {
      if ( !group.isEmpty() ) cpp << endl;
      group = e->group();
      cpp << "  setCurrentGroup( " << paramString(group, parameters) << " );" << endl << endl;
    }

    TQString key = paramString(e->key(), parameters);
    if ( !e->code().isEmpty())
    {
      cpp << e->code() << endl;
    }
    if ( e->type() == "Enum" ) {
      cpp << "  TQValueList<TDEConfigSkeleton::ItemEnum::Choice> values"
          << e->name() << ";" << endl;
      TQValueList<CfgEntry::Choice> choices = e->choices();
      TQValueList<CfgEntry::Choice>::ConstIterator it;
      for( it = choices.begin(); it != choices.end(); ++it ) {
        cpp << "  {" << endl;
        cpp << "    TDEConfigSkeleton::ItemEnum::Choice choice;" << endl;
        cpp << "    choice.name = TQString::fromLatin1( \"" << (*it).name << "\" );" << endl;
        if ( setUserTexts ) {
          if ( !(*it).label.isEmpty() )
            cpp << "    choice.label = i18n(" << quoteString((*it).label) << ");" << endl;
          if ( !(*it).whatsThis.isEmpty() )
            cpp << "    choice.whatsThis = i18n(" << quoteString((*it).whatsThis) << ");" << endl;
        }
        cpp << "    values" << e->name() << ".append( choice );" << endl;
        cpp << "  }" << endl;
      }
    }

    if (!dpointer)
      cpp << itemDeclaration(e);

    if (e->param().isEmpty())
    {
      // Normal case
      cpp << "  " << itemPath(e) << " = "
          << newItem( e->type(), e->name(), key, e->defaultValue() ) << endl;

      if ( !e->minValue().isEmpty() )
        cpp << "  " << itemPath(e) << "->setMinValue(" << e->minValue() << ");" << endl;
      if ( !e->maxValue().isEmpty() )
        cpp << "  " << itemPath(e) << "->setMaxValue(" << e->maxValue() << ");" << endl;

      if ( setUserTexts )
        cpp << userTextsFunctions( e );

      cpp << "  addItem( " << itemPath(e);
      TQString quotedName = e->name();
      addQuotes( quotedName );
      if ( quotedName != key ) cpp << ", TQString::fromLatin1( \"" << e->name() << "\" )";
      cpp << " );" << endl;
    }
    else
    {
      // Indexed
      for(int i = 0; i <= e->paramMax(); i++)
      {
        TQString defaultStr;
        TQString itemVarStr(itemPath(e)+TQString("[%1]").arg(i));

        if ( !e->paramDefaultValue(i).isEmpty() )
          defaultStr = e->paramDefaultValue(i);
        else if ( !e->defaultValue().isEmpty() )
          defaultStr = paramString(e->defaultValue(), e, i);
        else
          defaultStr = defaultValue( e->type() );

        cpp << "  " << itemVarStr << " = "
            << newItem( e->type(), e->name(), paramString(key, e, i), defaultStr, TQString("[%1]").arg(i) )
            << endl;

        if ( setUserTexts )
          cpp << userTextsFunctions( e, itemVarStr, e->paramName() );

        // Make mutators for enum parameters work by adding them with $(..) replaced by the
        // param name. The check for isImmutable in the set* functions doesn't have the param
        // name available, just the corresponding enum value (int), so we need to store the
        // param names in a separate static list!.
        cpp << "  addItem( " << itemVarStr << ", TQString::fromLatin1( \"";
        if ( e->paramType()=="Enum" )
          cpp << e->paramName().replace( "$("+e->param()+")", "%1").arg(e->paramValues()[i] );
        else
          cpp << e->paramName().replace( "$("+e->param()+")", "%1").arg(i);
        cpp << "\" ) );" << endl;
      }
    }
  }

  cpp << "}" << endl << endl;

  if (dpointer)
  {
    // setters and getters go in Cpp if in dpointer mode
    for( e = entries.first(); e; e = entries.next() )
    {
      TQString n = e->name();
      TQString t = e->type();
  
      // Manipulator
      if (allMutators || mutators.contains(n))
      {
        cpp << "void " << setFunction(n, className) << "( ";
        if (!e->param().isEmpty())
          cpp << cppType(e->paramType()) << " i, ";
        cpp << param( t ) << " v )" << endl;
        // function body inline only if not using dpointer
        // for BC mode
        cpp << "{" << endl;
        cpp << indent(memberMutatorBody(e), 6);      
        cpp << "}" << endl << endl;
      }
  
      // Accessor
      cpp << cppType(t) << " " << getFunction(n, className) << "(";
      if (!e->param().isEmpty())
        cpp << " " << cppType(e->paramType()) <<" i ";
      cpp << ")" << Const << endl;
      // function body inline only if not using dpointer
      // for BC mode
      cpp << "{" << endl;
      cpp << indent(memberAccessorBody(e), 2);      
      cpp << "}" << endl << endl;
  
      // Item accessor
      if ( itemAccessors )
      {
        cpp << endl;
        cpp << "TDEConfigSkeleton::Item" << itemType( e->type() ) << " *"
          << getFunction( n, className ) << "Item(";
        if (!e->param().isEmpty()) {
          cpp << " " << cppType(e->paramType()) << " i ";
        }
        cpp << ")" << endl;
        cpp << "{" << endl;
        cpp << indent(itemAccessorBody(e), 2);
        cpp << "}" << endl;
      }
  
      cpp << endl;
    }
  }

  // Destructor
  cpp << className << "::~" << className << "()" << endl;
  cpp << "{" << endl;
  if ( singleton ) {
    if ( dpointer )
      cpp << "  delete d;" << endl;
    cpp << "  if ( mSelf == this )" << endl;
    cpp << "    static" << className << "Deleter.setObject( mSelf, 0, false );" << endl;
  }
  cpp << "}" << endl << endl;

  implementation.close();
}

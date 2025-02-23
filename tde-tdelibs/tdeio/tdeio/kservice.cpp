/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 - 2001 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 1999        David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

// $Id$

#include <config.h>

#include "kservice.h"
#include "kservice_p.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

#include <tqregexp.h>
#include <tqstring.h>
#include <tqfile.h>
#include <tqdir.h>
#include <tqtl.h>

#include <ksimpleconfig.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <tdeconfigbase.h>
#include <kstandarddirs.h>
#include <dcopclient.h>

#include "kservicefactory.h"
#include "kservicetypefactory.h"
#include "kservicetype.h"
#include "kuserprofile.h"
#include "tdesycoca.h"

class KService::KServicePrivate
{
public:
  TQStringList categories;
  TQString menuId;
};

KService::KService( const TQString & _name, const TQString &_exec, const TQString &_icon)
 : KSycocaEntry( TQString::null)
{
  d = new KServicePrivate;
  m_bValid = true;
  m_bDeleted = false;
  m_strType = "Application";
  m_strName = _name;
  m_strExec = _exec;
  m_strIcon = _icon;
  m_bTerminal = false;
  m_bAllowAsDefault = true;
  m_initialPreference = 10;
}


KService::KService( const TQString & _fullpath )
 : KSycocaEntry( _fullpath)
{
  KDesktopFile config( _fullpath );

  init(&config);
}

KService::KService( KDesktopFile *config )
 : KSycocaEntry( config->fileName())
{
  init(config);
}

void
KService::init( KDesktopFile *config )
{
  d = new KServicePrivate;
  m_bValid = true;

  bool absPath = !TQDir::isRelativePath(entryPath());
  bool kde4application = config->fileName().contains("/share/applications/kde4/");
  TQString kde4applicationprefix;
  if (kde4application) {
    // extract prefix
    kde4applicationprefix = config->fileName();
    int pos = kde4applicationprefix.find("/share/applications/kde4/");
    kde4applicationprefix.truncate(pos);
  }

  config->setDesktopGroup();

  TQMap<TQString, TQString> entryMap = config->entryMap(config->group());

  entryMap.remove("Encoding"); // reserved as part of Desktop Entry Standard
  entryMap.remove("Version");  // reserved as part of Desktop Entry Standard

  m_bDeleted = config->readBoolEntry( "Hidden", false );
  entryMap.remove("Hidden");
  if (m_bDeleted)
  {
    //kdDebug() << "Hidden=true for " << entryPath() << endl;
    m_bValid = false;
    return;
  }

  m_strName = config->readName();
  entryMap.remove("Name");
  if ( m_strName.isEmpty() )
  {
    if (config->readEntry( "Exec" ).isEmpty())
    {
      //kdWarning(7012) << "The desktop entry file " << entryPath()
      //              << " has no Name and no Exec" << endl;
      m_bValid = false;
      return;
    }
    // Try to make up a name.
    m_strName = entryPath();
    int i = m_strName.findRev('/');
    m_strName = m_strName.mid(i+1);
    i = m_strName.findRev('.');
    if (i != -1)
       m_strName = m_strName.left(i);
  }

  m_strType = config->readType();
  entryMap.remove("Type");
  if ( m_strType.isEmpty() )
  {
    /*kdWarning(7012) << "The desktop entry file " << entryPath()
                    << " has no Type=... entry."
                    << " It should be \"Application\" or \"Service\"" << endl;
    m_bValid = false;
    return;*/
    m_strType = "Application";
  } else if ( m_strType != "Application" && m_strType != "Service" )
  {
    kdWarning(7012) << "The desktop entry file " << entryPath()
                    << " has Type=" << m_strType
                    << " instead of \"Application\" or \"Service\"" << endl;
    m_bValid = false;
    return;
  }

  // In case Try Exec is set, check if the application is available
  if (!config->tryExec()) {
      //kdDebug(7012) << "tryExec said false for " << entryPath() << endl;
      m_bDeleted = true;
      m_bValid = false;
      return;
  }

  TQString resource = config->resource();

  if ( (m_strType == "Application") &&
       (!resource.isEmpty()) &&
       (resource != "apps") &&
       !absPath)
  {
    kdWarning(7012) << "The desktop entry file " << entryPath()
           << " has Type=" << m_strType << " but is located under \"" << resource
           << "\" instead of \"apps\"" << endl;
    m_bValid = false;
    return;
  }

  if ( (m_strType == "Service") &&
       (!resource.isEmpty()) &&
       (resource != "services") &&
       !absPath)
  {
    kdWarning(7012) << "The desktop entry file " << entryPath()
           << " has Type=" << m_strType << " but is located under \"" << resource
           << "\" instead of \"services\"" << endl;
    m_bValid = false;
    return;
  }

  TQString name = entryPath();
  int pos = name.findRev('/');
  if (pos != -1)
     name = name.mid(pos+1);
  pos = name.find('.');
  if (pos != -1)
     name = name.left(pos);

  m_strExec = config->readPathEntry( "Exec" );
  if (kde4application && !m_strExec.startsWith("/")) {
    m_strExec = "XDG_DATA_DIRS=" + kde4applicationprefix + "/share XDG_CONFIG_DIRS=/etc/xdg/ PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:$PATH "+m_strExec;
  }
  else if (config->readBoolEntry("X-TDE-SubstituteUID") || config->readBoolEntry("X-KDE-SubstituteUID")) {
    TQString path = TQString::fromLocal8Bit(getenv("PATH"));
    TQString command;
    TQString params;
    int space = m_strExec.find(" ");
    if (space==-1) {
      command = m_strExec;
    }
    else {
      command = m_strExec.left(space);
      params = m_strExec.mid(space);
    }
    path.replace(TQRegExp("(^|:)(/usr/local|/usr)/bin($|:)"), "\\1\\2/sbin:\\2/bin\\3");
    path.replace(TQRegExp("(^|:)/bin($|:)"), "\\1/sbin:/bin\\2");
    m_strExec = TDEStandardDirs::findExe(command, path);
    if (!m_strExec.isEmpty() && !params.isEmpty()) {
      m_strExec += params;
    }
  }

  entryMap.remove("Exec");

  m_strIcon = config->readEntry( "Icon", "unknown" );
  if (kde4application) {
    if (TQFile::exists(kde4applicationprefix + "/share/icons/oxygen/22x22/apps/" + m_strIcon + ".png")) {
      m_strIcon = kde4applicationprefix + "/share/icons/oxygen/22x22/apps/" + m_strIcon + ".png";
    } else if (TQFile::exists(kde4applicationprefix + "/share/icons/hicolor/22x22/apps/" + m_strIcon + ".png")) {
      m_strIcon = kde4applicationprefix + "/share/icons/hicolor/22x22/apps/" + m_strIcon + ".png";
    }
  }
  entryMap.remove("Icon");
  m_bTerminal = (config->readBoolEntry( "Terminal" )); // should be a property IMHO
  entryMap.remove("Terminal");
  m_strTerminalOptions = config->readEntry( "TerminalOptions" ); // should be a property IMHO
  entryMap.remove("TerminalOptions");
  m_strPath = config->readPath();
  entryMap.remove("Path");
  m_strComment = config->readComment();
  entryMap.remove("Comment");
  m_strGenName = config->readGenericName();
#ifdef KDE4_MENU_SUFFIX
  if (kde4application) {
    m_strGenName += " [KDE4]";
  }
#endif
  entryMap.remove("GenericName");
  TQString untranslatedGenericName = config->readEntryUntranslated( "GenericName" );
  if (!untranslatedGenericName.isEmpty())
    entryMap.insert("UntranslatedGenericName", untranslatedGenericName);

  m_lstKeywords = config->readListEntry("Keywords", ';');
  entryMap.remove("Keywords");
  d->categories = config->readListEntry("Categories", ';');
  entryMap.remove("Categories");
  m_strLibrary = config->readEntry( "X-TDE-Library" );
  entryMap.remove("X-TDE-Library");
  m_strInit = config->readEntry("X-TDE-Init" );
  entryMap.remove("X-TDE-Init");

  m_lstServiceTypes = config->readListEntry( "X-TDE-ServiceTypes" );
  entryMap.remove("X-TDE-ServiceTypes");
  // For compatibility with KDE 1.x
  if (!kde4application)
     m_lstServiceTypes += config->readListEntry( "MimeType", ';' );
  entryMap.remove("MimeType");

  if ( m_strType == "Application" && !m_lstServiceTypes.contains("Application") )
    // Applications implement the service type "Application" ;-)
    m_lstServiceTypes += "Application";

  TQString dcopServiceType = config->readEntry("X-DCOP-ServiceType").lower();
  entryMap.remove("X-DCOP-ServiceType");
  if (dcopServiceType == "unique")
     m_DCOPServiceType = DCOP_Unique;
  else if (dcopServiceType == "multi")
     m_DCOPServiceType = DCOP_Multi;
  else if (dcopServiceType == "wait")
     m_DCOPServiceType = DCOP_Wait;
  else
     m_DCOPServiceType = DCOP_None;

  m_strDesktopEntryName = name.lower();
  if (kde4application)
     m_strDesktopEntryName = "kde4-" + m_strDesktopEntryName;

  m_bAllowAsDefault = config->readBoolEntry( "AllowDefault", true );
  entryMap.remove("AllowDefault");

  m_initialPreference = config->readNumEntry( "X-TDE-InitialPreference", 1 );
  entryMap.remove("X-TDE-InitialPreference");
  if ( m_initialPreference == 1 )
     m_initialPreference = config->readNumEntry( "X-TDE-InitialPreference", 1 );
  entryMap.remove("X-TDE-InitialPreference");

  // Store all additional entries in the property map.
  // A TQMap<TQString,TQString> would be easier for this but we can't
  // brake BC, so we have to store it in m_mapProps.
//  tqWarning("Path = %s", entryPath().latin1());
  TQMap<TQString,TQString>::ConstIterator it = entryMap.begin();
  for( ; it != entryMap.end();++it)
  {
     //tqDebug("   Key = %s Data = %s", it.key().latin1(), it.data().latin1());
      TQString key = it.key();
      if (kde4application && key=="OnlyShowIn" && it.data()=="KDE;")
         key = "NotShowIn";
      m_mapProps.insert( key, TQVariant( it.data()));
  }
}

KService::KService( TQDataStream& _str, int offset ) : KSycocaEntry( _str, offset )
{
  d = new KServicePrivate;
  load( _str );
}

KService::~KService()
{
  //debug("KService::~KService()");
  delete d;
}

TQPixmap KService::pixmap( TDEIcon::Group _group, int _force_size, int _state, TQString * _path ) const
{
  TDEIconLoader *iconLoader=TDEGlobal::iconLoader();
  if (!iconLoader->extraDesktopThemesAdded())
  {
      TQPixmap pixmap=iconLoader->loadIcon( m_strIcon, _group, _force_size, _state, _path, true );
      if (!pixmap.isNull() ) return pixmap;

      iconLoader->addExtraDesktopThemes();
  }

  return iconLoader->loadIcon( m_strIcon, _group, _force_size, _state, _path );
}

void KService::load( TQDataStream& s )
{
  // dummies are here because of fields that were removed, to keep bin compat.
  // Feel free to re-use, but fields for Applications only (not generic services)
  // should rather be added to application.desktop
  TQ_INT8 def, term, dummy1, dummy2;
  TQ_INT8 dst, initpref;
  TQString dummyStr1, dummyStr2;
  int dummyI1, dummyI2;
  TQ_UINT32 dummyUI32;

  // WARNING: IN KDE 3.x THIS NEEDS TO REMAIN COMPATIBLE WITH KDE 2.x!
  // !! This data structure should remain binary compatible at all times !!
  // You may add new fields at the end. Make sure to update the version
  // number in tdesycoca.h
  s >> m_strType >> m_strName >> m_strExec >> m_strIcon
    >> term >> m_strTerminalOptions
    >> m_strPath >> m_strComment >> m_lstServiceTypes >> def >> m_mapProps
    >> m_strLibrary >> dummyI1 >> dummyI2
    >> dst
    >> m_strDesktopEntryName
    >> dummy1 >> dummyStr1 >> initpref >> dummyStr2 >> dummy2
    >> m_lstKeywords >> m_strInit >> dummyUI32 >> m_strGenName
    >> d->categories >> d->menuId;

  m_bAllowAsDefault = def;
  m_bTerminal = term;
  m_DCOPServiceType = (DCOPServiceType_t) dst;
  m_initialPreference = initpref;

  m_bValid = true;
}

void KService::save( TQDataStream& s )
{
  KSycocaEntry::save( s );
  TQ_INT8 def = m_bAllowAsDefault, initpref = m_initialPreference;
  TQ_INT8 term = m_bTerminal;
  TQ_INT8 dst = (TQ_INT8) m_DCOPServiceType;
  TQ_INT8 dummy1 = 0, dummy2 = 0; // see ::load
  TQString dummyStr1, dummyStr2;
  int dummyI1 = 0, dummyI2 = 0;
  TQ_UINT32 dummyUI32 = 0;

  // WARNING: IN KDE 3.x THIS NEEDS TO REMAIN COMPATIBLE WITH KDE 2.x!
  // !! This data structure should remain binary compatible at all times !!
  // You may add new fields at the end. Make sure to update the version
  // number in tdesycoca.h
  s << m_strType << m_strName << m_strExec << m_strIcon
    << term << m_strTerminalOptions
    << m_strPath << m_strComment << m_lstServiceTypes << def << m_mapProps
    << m_strLibrary << dummyI1 << dummyI2
    << dst
    << m_strDesktopEntryName
    << dummy1 << dummyStr1 << initpref << dummyStr2 << dummy2
    << m_lstKeywords << m_strInit << dummyUI32 << m_strGenName
    << d->categories << d->menuId;
}

bool KService::hasServiceType( const TQString& _servicetype ) const
{
  if (!m_bValid) return false; // safety test

  //kdDebug(7012) << "Testing " << m_strDesktopEntryName << " for " << _servicetype << endl;

  KMimeType::Ptr mimePtr = KMimeType::mimeType( _servicetype );
  if ( mimePtr && mimePtr == KMimeType::defaultMimeTypePtr() )
      mimePtr = 0;

  bool isNumber;
  // For each service type we are associated with, if it doesn't
  // match then we try its parent service types.
  TQStringList::ConstIterator it = m_lstServiceTypes.begin();
  for( ; it != m_lstServiceTypes.end(); ++it )
  {
      (*it).toInt(&isNumber);
      if (isNumber)
         continue;
      //kdDebug(7012) << "    has " << (*it) << endl;
      KServiceType::Ptr ptr = KServiceType::serviceType( *it );
      if ( ptr && ptr->inherits( _servicetype ) )
          return true;

      // The mimetype inheritance ("is also") works the other way.
      // e.g. if we're looking for a handler for mimePtr==smb-workgroup
      // then a handler for inode/directory is ok.
      if ( mimePtr && mimePtr->is( *it ) )
          return true;
  }
  return false;
}

int KService::initialPreferenceForMimeType( const TQString& mimeType ) const
{
  if (!m_bValid) return 0; // safety test

  bool isNumber;

  // For each service type we are associated with
  TQStringList::ConstIterator it = m_lstServiceTypes.begin();
  for( ; it != m_lstServiceTypes.end(); ++it )
  {
      (*it).toInt(&isNumber);
      if (isNumber)
         continue;
      //kdDebug(7012) << "    has " << (*it) << endl;
      KServiceType::Ptr ptr = KServiceType::serviceType( *it );
      if ( !ptr || !ptr->inherits( mimeType ) )
          continue;

      int initalPreference = m_initialPreference;
      ++it;
      if (it != m_lstServiceTypes.end())
      {
         int i = (*it).toInt(&isNumber);
         if (isNumber)
            initalPreference = i;
      }
      return initalPreference;
  }

  KMimeType::Ptr mimePtr = KMimeType::mimeType( mimeType );
  if ( mimePtr && mimePtr == KMimeType::defaultMimeTypePtr() )
      mimePtr = 0;

  // Try its parent service types.
  it = m_lstServiceTypes.begin();
  for( ; it != m_lstServiceTypes.end(); ++it )
  {
      (*it).toInt(&isNumber);
      if (isNumber)
         continue;

      // The mimetype inheritance ("is also") works the other way.
      // e.g. if we're looking for a handler for mimePtr==smb-workgroup
      // then a handler for inode/directory is ok.
      if ( !mimePtr || !mimePtr->is( *it ) )
          continue;

      int initalPreference = m_initialPreference;
      ++it;
      if (it != m_lstServiceTypes.end())
      {
         int i = (*it).toInt(&isNumber);
         if (isNumber)
            initalPreference = i;
      }
      return initalPreference;
  }
  return 0;
}

class KServiceReadProperty : public TDEConfigBase
{
public:
   KServiceReadProperty(const TQString &_key, const TQCString &_value)
	: key(_key), value(_value) { }

   bool internalHasGroup(const TQCString &) const { /*tqDebug("hasGroup(const TQCString &)");*/ return false; }

   TQStringList groupList() const { return TQStringList(); }

   TQMap<TQString,TQString> entryMap(const TQString &group) const
      { Q_UNUSED(group); return TQMap<TQString,TQString>(); }

   void reparseConfiguration() { }

   KEntryMap internalEntryMap( const TQString &pGroup) const
   { Q_UNUSED(pGroup); return KEntryMap(); }

   KEntryMap internalEntryMap() const { return KEntryMap(); }

   void putData(const KEntryKey &_key, const KEntry& _data, bool _checkGroup)
   { Q_UNUSED(_key); Q_UNUSED(_data); Q_UNUSED(_checkGroup); }

   KEntry lookupData(const KEntryKey &_key) const
   { Q_UNUSED(_key); KEntry entry; entry.mValue = value; return entry; }
protected:
   TQString key;
   TQCString value;
};

TQVariant KService::property( const TQString& _name) const
{
   return property( _name, TQVariant::Invalid);
}

// Return a string TQVariant if string isn't null, and invalid variant otherwise
// (the variant must be invalid if the field isn't in the .desktop file)
// This allows trader queries like "exist Library" to work.
static TQVariant makeStringVariant( const TQString& string )
{
    // Using isEmpty here would be wrong.
    // Empty is "specified but empty", null is "not specified" (in the .desktop file)
    return string.isNull() ? TQVariant() : TQVariant( string );
}

TQVariant KService::property( const TQString& _name, TQVariant::Type t ) const
{
  if ( _name == "Type" )
    return TQVariant( m_strType ); // can't be null
  else if ( _name == "Name" )
    return TQVariant( m_strName ); // can't be null
  else if ( _name == "Exec" )
    return makeStringVariant( m_strExec );
  else if ( _name == "Icon" )
    return makeStringVariant( m_strIcon );
  else if ( _name == "Terminal" )
    return TQVariant( static_cast<int>(m_bTerminal) );
  else if ( _name == "TerminalOptions" )
    return makeStringVariant( m_strTerminalOptions );
  else if ( _name == "Path" )
    return makeStringVariant( m_strPath );
  else if ( _name == "Comment" )
    return makeStringVariant( m_strComment );
  else if ( _name == "GenericName" )
    return makeStringVariant( m_strGenName );
  else if ( _name == "ServiceTypes" )
    return TQVariant( m_lstServiceTypes );
  else if ( _name == "AllowAsDefault" )
    return TQVariant( static_cast<int>(m_bAllowAsDefault) );
  else if ( _name == "InitialPreference" )
    return TQVariant( m_initialPreference );
  else if ( _name == "Library" )
    return makeStringVariant( m_strLibrary );
  else if ( _name == "DesktopEntryPath" ) // can't be null
    return TQVariant( entryPath() );
  else if ( _name == "DesktopEntryName")
    return TQVariant( m_strDesktopEntryName ); // can't be null
  else if ( _name == "Categories")
    return TQVariant( d->categories );
  else if ( _name == "Keywords")
    return TQVariant( m_lstKeywords );

  // Ok we need to convert the property from a TQString to its real type.
  // Maybe the caller helped us.
  if (t == TQVariant::Invalid)
  {
    // No luck, let's ask KServiceTypeFactory what the type of this property
    // is supposed to be.
    t = KServiceTypeFactory::self()->findPropertyTypeByName(_name);
    if (t == TQVariant::Invalid)
    {
      kdDebug(7012) << "Request for unknown property '" << _name << "'\n";
      return TQVariant(); // Unknown property: Invalid variant.
    }
  }

  // Then we use a homebuild class based on TDEConfigBase to convert the TQString.
  // For some often used property types we do the conversion ourselves.
  TQStringVariantMap::ConstIterator it = m_mapProps.find( _name );
  if ( (it == m_mapProps.end()) || (!it.data().isValid()))
  {
     //kdDebug(7012) << "Property not found " << _name << endl;
     return TQVariant(); // No property set.
  }

  switch(t)
  {
    case TQVariant::String:
        return it.data();
    case TQVariant::Bool:
    case TQVariant::Int:
        {
           TQString aValue = it.data().toString();
           int val = 0;
           if (aValue == "true" || aValue == "on" || aValue == "yes")
              val = 1;
           else
           {
              bool bOK;
              val = aValue.toInt( &bOK );
              if( !bOK )
                 val = 0;
           }
           if (t == TQVariant::Bool)
           {
               return TQVariant((bool)val);
           }
           return TQVariant(val);
        }
    default:
        // All others
        KServiceReadProperty ksrp(_name, it.data().toString().utf8());
        return ksrp.readPropertyEntry(_name, t);
  }
}

TQStringList KService::propertyNames() const
{
  TQStringList res;

  TQStringVariantMap::ConstIterator it = m_mapProps.begin();
  for( ; it != m_mapProps.end(); ++it )
    res.append( it.key() );

  res.append( "Type" );
  res.append( "Name" );
  res.append( "Comment" );
  res.append( "GenericName" );
  res.append( "Icon" );
  res.append( "Exec" );
  res.append( "Terminal" );
  res.append( "TerminalOptions" );
  res.append( "Path" );
  res.append( "ServiceTypes" );
  res.append( "AllowAsDefault" );
  res.append( "InitialPreference" );
  res.append( "Library" );
  res.append( "DesktopEntryPath" );
  res.append( "DesktopEntryName" );
  res.append( "Keywords" );
  res.append( "Categories" );

  return res;
}

KService::List KService::allServices()
{
  return KServiceFactory::self()->allServices();
}

KService::Ptr KService::serviceByName( const TQString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByName( _name );
  return KService::Ptr( s );
}

KService::Ptr KService::serviceByDesktopPath( const TQString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByDesktopPath( _name );
  return KService::Ptr( s );
}

KService::Ptr KService::serviceByDesktopName( const TQString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByDesktopName( _name.lower() );
  if (!s && !_name.startsWith("tde-"))
     s = KServiceFactory::self()->findServiceByDesktopName( "tde-"+_name.lower() );
  return KService::Ptr( s );
}

KService::Ptr KService::serviceByMenuId( const TQString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByMenuId( _name );
  return KService::Ptr( s );
}

KService::Ptr KService::serviceByStorageId( const TQString& _storageId )
{
  KService::Ptr service = KService::serviceByMenuId( _storageId );
  if (service)
     return service;

  service = KService::serviceByDesktopPath(_storageId);
  if (service)
     return service;

  if (!TQDir::isRelativePath(_storageId) && TQFile::exists(_storageId))
     return new KService(_storageId);

  TQString tmp = _storageId;
  tmp = tmp.mid(tmp.findRev('/')+1); // Strip dir

  if (tmp.endsWith(".desktop"))
     tmp.truncate(tmp.length()-8);

  if (tmp.endsWith(".kdelnk"))
     tmp.truncate(tmp.length()-7);

  service = KService::serviceByDesktopName(tmp);

  return service;
}

KService::List KService::allInitServices()
{
  return KServiceFactory::self()->allInitServices();
}

bool KService::substituteUid() const {
  bool suid = false;
  TQVariant v;
  v = property("X-TDE-SubstituteUID", TQVariant::Bool);
  if (v.isValid()) {
    if (v.toBool()) suid = true;
  }
  v = property("X-KDE-SubstituteUID", TQVariant::Bool);
  if (v.isValid()) {
    if (v.toBool()) suid = true;
  }
  return suid;
}

TQString KService::username() const {
  // See also KDesktopFile::tryExec()
  TQString user;
  TQVariant v = property("X-TDE-Username", TQVariant::String);
  user = v.isValid() ? v.toString() : TQString::null;
  if (user.isEmpty())
     user = ::getenv("ADMIN_ACCOUNT");
  if (user.isEmpty())
     user = "root";
  return user;
}

bool KService::noDisplay() const {
  TQStringVariantMap::ConstIterator it = m_mapProps.find( "NoDisplay" );
  if ( (it != m_mapProps.end()) && (it.data().isValid()))
  {
     TQString aValue = it.data().toString().lower();
     if (aValue == "true" || aValue == "on" || aValue == "yes")
        return true;
  }

  it = m_mapProps.find( "OnlyShowIn" );
  if ( (it != m_mapProps.end()) && (it.data().isValid()))
  {
     TQString aValue = it.data().toString();
     TQStringList aList = TQStringList::split(';', aValue);
#ifdef WITH_OLD_XDG_STD
     if ((!aList.contains("TDE")) && (!aList.contains("KDE")))
        return true;
#else
     if (!aList.contains("TDE"))
        return true;
#endif
  }

  it = m_mapProps.find( "NotShowIn" );
  if ( (it != m_mapProps.end()) && (it.data().isValid()))
  {
     TQString aValue = it.data().toString();
     TQStringList aList = TQStringList::split(';', aValue);
#ifdef WITH_OLD_XDG_STD
     if ((aList.contains("TDE")) || (aList.contains("KDE")))
        return true;
#else
     if (aList.contains("TDE"))
        return true;
#endif
  }

  if (!kapp->authorizeControlModule(d->menuId))
     return true;

  return false;
}

TQString KService::untranslatedGenericName() const {
  TQVariant v = property("UntranslatedGenericName", TQVariant::String);
  return v.isValid() ? v.toString() : TQString::null;
}

bool KService::SuSEunimportant() const {
  TQStringVariantMap::ConstIterator it = m_mapProps.find( "X-SuSE-Unimportant" );
  if ( (it == m_mapProps.end()) || (!it.data().isValid()))
  {
     return false;
  }

  TQString aValue = it.data().toString();
  if (aValue == "true" || aValue == "on" || aValue == "yes")
     return true;
  else
     return false;
}

TQString KService::parentApp() const {
  TQStringVariantMap::ConstIterator it = m_mapProps.find( "X-TDE-ParentApp" );
  if ( (it == m_mapProps.end()) || (!it.data().isValid()))
  {
     return TQString::null;
  }

  return it.data().toString();
}

bool KService::allowMultipleFiles() const {
  // Can we pass multiple files on the command line or do we have to start the application for every single file ?
  if ( m_strExec.find( "%F" ) != -1 || m_strExec.find( "%U" ) != -1 ||
       m_strExec.find( "%N" ) != -1 || m_strExec.find( "%D" ) != -1 )
    return true;
  else
    return false;
}

TQStringList KService::categories() const
{
  return d->categories;
}

TQString KService::menuId() const
{
  return d->menuId;
}

void KService::setMenuId(const TQString &menuId)
{
  d->menuId = menuId;
}

TQString KService::storageId() const
{
  if (!d->menuId.isEmpty())
     return d->menuId;
  return entryPath();
}

TQString KService::locateLocal()
{
  if (d->menuId.isEmpty() || desktopEntryPath().startsWith(".hidden") ||
      (TQDir::isRelativePath(desktopEntryPath()) && d->categories.isEmpty()))
     return KDesktopFile::locateLocal(desktopEntryPath());

  return ::locateLocal("xdgdata-apps", d->menuId);
}

TQString KService::newServicePath(bool showInMenu, const TQString &suggestedName,
                                TQString *menuId, const TQStringList *reservedMenuIds)
{
   TQString base = suggestedName;
   if (!showInMenu)
     base.prepend("tde-");

   TQString result;
   for(int i = 1; true; i++)
   {
      if (i == 1)
         result = base + ".desktop";
      else
         result = base + TQString("-%1.desktop").arg(i);

      if (reservedMenuIds && reservedMenuIds->contains(result))
         continue;

      // Lookup service by menu-id
      KService::Ptr s = serviceByMenuId(result);
      if (s)
         continue;

      if (showInMenu)
      {
         if (!locate("xdgdata-apps", result).isEmpty())
            continue;
      }
      else
      {
         TQString file = result.mid(4); // Strip "tde-"
         if (!locate("apps", ".hidden/"+file).isEmpty())
            continue;
      }

      break;
   }
   if (menuId)
      *menuId = result;

   if (showInMenu)
   {
       return ::locateLocal("xdgdata-apps", result);
   }
   else
   {
       TQString file = result.mid(4); // Strip "tde-"
       return ::locateLocal("apps", ".hidden/"+file);
   }
}


void KService::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }


void KService::rebuildKSycoca(TQWidget *parent)
{
  KServiceProgressDialog dlg(parent, "tdesycoca_progress",
                      i18n("Updating System Configuration"),
                      i18n("Updating system configuration."));

  TQByteArray data;
  DCOPClient *client = kapp->dcopClient();

  int result = client->callAsync("kded", "tdebuildsycoca", "recreate()",
               data, &dlg, TQ_SLOT(slotFinished()));

  if (result)
  {
     dlg.exec();
  }
}

KServiceProgressDialog::KServiceProgressDialog(TQWidget *parent, const char *name,
                          const TQString &caption, const TQString &text)
 : KProgressDialog(parent, name, caption, text, true)
{
  connect(&m_timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(slotProgress()));
  progressBar()->setTotalSteps(20);
  m_timeStep = 700;
  m_timer.start(m_timeStep);
  setAutoClose(false);
}

void
KServiceProgressDialog::slotProgress()
{
  int p = progressBar()->progress();
  if (p == 18)
  {
     progressBar()->reset();
     progressBar()->setProgress(1);
     m_timeStep = m_timeStep * 2;
     m_timer.start(m_timeStep);
  }
  else
  {
     progressBar()->setProgress(p+1);
  }
}

void
KServiceProgressDialog::slotFinished()
{
  progressBar()->setProgress(20);
  m_timer.stop();
  TQTimer::singleShot(1000, this, TQ_SLOT(close()));
}

#include "kservice_p.moc"

/*
   Copyright (C) 1999 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <sys/param.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include <tqdir.h>
#include <tqfile.h>
#include <tqasciidict.h>
#include <tqstrlist.h>

#include "tdecmdlineargs.h"
#include <tdeaboutdata.h>
#include <tdelocale.h>
#include <tdeapplication.h>
#include <tdeglobal.h>
#include <kstringhandler.h>
#include <kstaticdeleter.h>

#ifdef TQ_WS_X11
#define DISPLAY "DISPLAY"
#elif defined(TQ_WS_QWS)
#define DISPLAY "QWS_DISPLAY"
#endif

#ifdef TQ_WS_WIN
#include <win32_utils.h>
#endif

template class TQAsciiDict<TQCString>;
template class TQPtrList<TDECmdLineArgs>;

class TDECmdLineParsedOptions : public TQAsciiDict<TQCString>
{
public:
   TDECmdLineParsedOptions()
     : TQAsciiDict<TQCString>( 7 ) { }

   // WABA: Huh?
   // The compiler doesn't find TDECmdLineParsedOptions::write(s) by itself ???
   // WABA: No, because there is another write function that hides the
   // write function in the base class even though this function has a
   // different signature. (obscure C++ feature)
   TQDataStream& save( TQDataStream &s) const
   { return TQGDict::write(s); }

   TQDataStream& load( TQDataStream &s)
   { return TQGDict::read(s); }

protected:
   virtual TQDataStream& write( TQDataStream &s, TQPtrCollection::Item data) const
   {
      TQCString *str = (TQCString *) data;
      s << (*str);
      return s;
   }

   virtual TQDataStream& read( TQDataStream &s, TQPtrCollection::Item &item)
   {
      TQCString *str = new TQCString;
      s >> (*str);
      item = (void *)str;
      return s;
   }

};

class TDECmdLineParsedArgs : public TQStrList
{
public:
   TDECmdLineParsedArgs()
     : TQStrList( true ) { }
   TQDataStream& save( TQDataStream &s) const
   { return TQGList::write(s); }

   TQDataStream& load( TQDataStream &s)
   { return TQGList::read(s); }
};


class TDECmdLineArgsList: public TQPtrList<TDECmdLineArgs>
{
public:
   TDECmdLineArgsList() { }
};

TDECmdLineArgsList *TDECmdLineArgs::argsList = 0;
int TDECmdLineArgs::argc = 0;
char **TDECmdLineArgs::argv = 0;
char *TDECmdLineArgs::mCwd = 0;
static KStaticDeleter <char> mCwdd;
const TDEAboutData *TDECmdLineArgs::about = 0;
bool TDECmdLineArgs::parsed = false;
bool TDECmdLineArgs::ignoreUnknown = false;

//
// Static functions
//

void
TDECmdLineArgs::init(int _argc, char **_argv, const char *_appname, const char* programName,
                   const char *_description, const char *_version, bool noKApp)
{
   init(_argc, _argv,
        new TDEAboutData(_appname, programName, _version, _description),
        noKApp);
}

void
TDECmdLineArgs::init(int _argc, char **_argv, const char *_appname,
                   const char *_description, const char *_version, bool noKApp)
{
   init(_argc, _argv,
        new TDEAboutData(_appname, _appname, _version, _description),
        noKApp);
}

void
TDECmdLineArgs::initIgnore(int _argc, char **_argv, const char *_appname )
{
   init(_argc, _argv,
        new TDEAboutData(_appname, _appname, "unknown", "TDE Application", false));
   ignoreUnknown = true;
}

void
TDECmdLineArgs::init(const TDEAboutData* ab)
{
   char **_argv = (char **) malloc(sizeof(char *));
   _argv[0] = (char *) ab->appName();
   init(1,_argv,ab, true);
}


void
TDECmdLineArgs::init(int _argc, char **_argv, const TDEAboutData *_about, bool noKApp)
{
   argc = _argc;
   argv = _argv;

   if (!argv)
   {
      fprintf(stderr, "\n\nFAILURE (TDECmdLineArgs):\n");
      fprintf(stderr, "Passing null-pointer to 'argv' is not allowed.\n\n");

      assert( 0 );
      exit(255);
   }

   // Strip path from argv[0]
   if (argc) {
     char *p = strrchr( argv[0], '/');
     if (p)
       argv[0] = p+1;
   }

   about = _about;
   parsed = false;
   mCwd = mCwdd.setObject(mCwd, new char [PATH_MAX+1], true);
   (void) getcwd(mCwd, PATH_MAX);
#ifdef TQ_WS_WIN
   win32_slashify(mCwd, PATH_MAX);
#endif
   if (!noKApp)
      TDEApplication::addCmdLineOptions();
}

TQString TDECmdLineArgs::cwd()
{
   return TQFile::decodeName(TQCString(mCwd));
}

const char * TDECmdLineArgs::appName()
{
   if (!argc) return 0;
   return argv[0];
}

void
TDECmdLineArgs::addCmdLineOptions( const TDECmdLineOptions *options, const char *name,
         const char *id, const char *afterId)
{
   if (!argsList)
      argsList = new TDECmdLineArgsList();

   int pos = argsList->count();

   if (pos && id && argsList->last() && !argsList->last()->name)
      pos--;

   TDECmdLineArgs *args;
   int i = 0;
   for(args = argsList->first(); args; args = argsList->next(), i++)
   {
      if (!id && !args->id)
         return; // Options already present.

      if (id && args->id && (::qstrcmp(id, args->id) == 0))
   return; // Options already present.

      if (afterId && args->id && (::qstrcmp(afterId, args->id) == 0))
         pos = i+1;
   }

   assert( parsed == false ); // You must add _ALL_ cmd line options
                              // before accessing the arguments!
   args = new TDECmdLineArgs(options, name, id);
   argsList->insert(pos, args);
}

void
TDECmdLineArgs::saveAppArgs( TQDataStream &ds)
{
   if (!parsed)
      parseAllArgs();

   // Remove Qt and TDE options.
   removeArgs("qt");
   removeArgs("tde");

   TQCString qCwd = mCwd;
   ds << qCwd;

   uint count = argsList ? argsList->count() : 0;
   ds << count;

   if (!count) return;

   TDECmdLineArgs *args;
   for(args = argsList->first(); args; args = argsList->next())
   {
      ds << TQCString(args->id);
      args->save(ds);
   }
}

void
TDECmdLineArgs::loadAppArgs( TQDataStream &ds)
{
   parsed = true; // don't reparse argc/argv!

   // Remove Qt and TDE options.
   removeArgs("qt");
   removeArgs("tde");

   TDECmdLineArgs *args;
   if ( argsList ) {
      for(args = argsList->first(); args; args = argsList->next())
      {
         args->clear();
      }
   }

   if (ds.atEnd())
      return;

   TQCString qCwd;
   ds >> qCwd;
   delete [] mCwd;

   mCwd = mCwdd.setObject(mCwd, new char[qCwd.length()+1], true);
   strncpy(mCwd, qCwd.data(), qCwd.length()+1);

   uint count;
   ds >> count;

   while(count--)
   {
     TQCString id;
     ds >> id;
     assert( argsList );
     for(args = argsList->first(); args; args = argsList->next())
     {
       if (args->id  == id)
       {
          args->load(ds);
          break;
       }
     }
   }
   parsed = true;
}

TDECmdLineArgs *TDECmdLineArgs::parsedArgs(const char *id)
{
   TDECmdLineArgs *args = argsList ? argsList->first() : 0;
   while(args)
   {
      if ((id && ::qstrcmp(args->id, id) == 0) || (!id && !args->id))
      {
          if (!parsed)
             parseAllArgs();
          return args;
      }
      args = argsList->next();
   }

   return args;
}

void TDECmdLineArgs::removeArgs(const char *id)
{
   TDECmdLineArgs *args = argsList ? argsList->first() : 0;
   while(args)
   {
      if (args->id && id && ::qstrcmp(args->id, id) == 0)
      {
          if (!parsed)
             parseAllArgs();
          break;
      }
      args = argsList->next();
   }

   if (args)
      delete args;
}

/*
 * @return:
 *  0 - option not found.
 *  1 - option found      // -fork
 *  2 - inverse option found ('no') // -nofork
 *  3 - option + arg found    // -fork now
 *
 *  +4 - no more options follow         // !fork
 */
static int
findOption(const TDECmdLineOptions *options, TQCString &opt,
           const char *&opt_name, const char *&def, bool &enabled)
{
   int result;
   bool inverse;
   int len = opt.length();
   while(options && options->name)
   {
      result = 0;
      inverse = false;
      opt_name = options->name;
      if ((opt_name[0] == ':') || (opt_name[0] == 0))
      {
         options++;
         continue;
      }

      if (opt_name[0] == '!')
      {
         opt_name++;
         result = 4;
      }
      if ((opt_name[0] == 'n') && (opt_name[1] == 'o'))
      {
         opt_name += 2;
         inverse = true;
      }
      if (strncmp(opt.data(), opt_name, len) == 0)
      {
         opt_name += len;
         if (!opt_name[0])
         {
            if (inverse)
               return result+2;

            if (!options->description)
            {
               options++;
               if (!options->name)
                  return result+0;
               TQCString nextOption = options->name;
               int p = nextOption.find(' ');
               if (p > 0)
                  nextOption = nextOption.left(p);
               if (nextOption[0] == '!')
                  nextOption = nextOption.mid(1);
               if (strncmp(nextOption.data(), "no", 2) == 0)
               {
                  nextOption = nextOption.mid(2);
                  enabled = !enabled;
               }
               result = findOption(options, nextOption, opt_name, def, enabled);
               assert(result);
               opt = nextOption;
               return result;
            }

            return 1;
         }
         if (opt_name[0] == ' ')
         {
            opt_name++;
            def = options->def;
            return result+3;
         }
      }

      options++;
   }
   return 0;
}


void
TDECmdLineArgs::findOption(const char *_opt, TQCString opt, int &i, bool _enabled, bool &moreOptions)
{
   TDECmdLineArgs *args = argsList->first();
   const char *opt_name;
   const char *def;
   TQCString argument;
   int j = opt.find('=');
   if (j != -1)
   {
      argument = opt.mid(j+1);
      opt = opt.left(j);
   }

   bool enabled = true;
   int result = 0;
   while (args)
   {
      enabled = _enabled;
      result = ::findOption(args->options, opt, opt_name, def, enabled);
      if (result) break;
      args = argsList->next();
   }
   if (!args && (_opt[0] == '-') && _opt[1] && (_opt[1] != '-'))
   {
      // Option not found check if it is a valid option
      // in the style of -Pprinter1 or ps -aux
      int p = 1;
      while (true)
      {
         TQCString singleCharOption = " ";
         singleCharOption[0] = _opt[p];
         args = argsList->first();
         while (args)
         {
            enabled = _enabled;
            result = ::findOption(args->options, singleCharOption, opt_name, def, enabled);
            if (result) break;
            args = argsList->next();
         }
         if (!args)
            break; // Unknown argument

         p++;
         if (result == 1) // Single option
         {
            args->setOption(singleCharOption, enabled);
            if (_opt[p])
               continue; // Next option
            else
               return; // Finished
         }
         else if (result == 3) // This option takes an argument
         {
            if (argument.isEmpty())
            {
               argument = _opt+p;
            }
            args->setOption(singleCharOption, (const char*)argument);
            return;
         }
         break; // Unknown argument
      }
      args = 0;
      result = 0;
   }

   if (!args || !result)
   {
      if (ignoreUnknown)
         return;
      enable_i18n();
      usage( i18n("Unknown option '%1'.").arg(TQString::fromLocal8Bit(_opt)));
   }

   if ((result & 4) != 0)
   {
      result &= ~4;
      moreOptions = false;
   }

   if (result == 3) // This option takes an argument
   {
      if (!enabled)
      {
         if (ignoreUnknown)
            return;
         enable_i18n();
         usage( i18n("Unknown option '%1'.").arg(TQString::fromLocal8Bit(_opt)));
      }
      if (argument.isEmpty())
      {
         i++;
         if (i >= argc)
         {
            enable_i18n();
            usage( i18n("'%1' missing.").arg( opt_name));
         }
         argument = argv[i];
      }
      args->setOption(opt, (const char*)argument);
   }
   else
   {
      args->setOption(opt, enabled);
   }
}

void
TDECmdLineArgs::printQ(const TQString &msg)
{
   TQCString localMsg = msg.local8Bit();
   fprintf(stdout, "%s", localMsg.data());
}

void
TDECmdLineArgs::parseAllArgs()
{
   bool allowArgs = false;
   bool inOptions = true;
   bool everythingAfterArgIsArgs = false;
   TDECmdLineArgs *appOptions = argsList->last();
   if (!appOptions->id)
   {
     const TDECmdLineOptions *option = appOptions->options;
     while(option && option->name)
     {
       if (option->name[0] == '+')
           allowArgs = true;
       if ( option->name[0] == '!' && option->name[1] == '+' )
       {
           allowArgs = true;
           everythingAfterArgIsArgs = true;
       }
       option++;
     }
   }
   for(int i = 1; i < argc; i++)
   {
      if (!argv[i])
         continue;

      if ((argv[i][0] == '-') && argv[i][1] && inOptions)
      {
         bool enabled = true;
         const char *option = &argv[i][1];
         const char *orig = argv[i];
         if (option[0] == '-')
         {
            option++;
            argv[i]++;
            if (!option[0])
            {
               inOptions = false;
               continue;
            }
         }
         if (::qstrcmp(option, "help") == 0)
         {
            usage(0);
         }
         else if (strncmp(option, "help-",5) == 0)
         {
            usage(option+5);
         }
         else if ( (::qstrcmp(option, "version") == 0) ||
                   (::qstrcmp(option, "v") == 0))
         {
            printQ( TQString("Qt: %1\n").arg(tqVersion()));
            printQ( TQString("TDE: %1\n").arg(TDE_VERSION_STRING));
            printQ( TQString("%1: %2\n").
      arg(about->programName()).arg(about->version()));
            exit(0);
         } else if ( (::qstrcmp(option, "license") == 0) )
         {
            enable_i18n();
            printQ( about->license() );
            printQ( "\n" );
            exit(0);
         } else if ( ::qstrcmp( option, "author") == 0 ) {
             enable_i18n();
       if ( about ) {
         const TQValueList<TDEAboutPerson> authors = about->authors();
         if ( !authors.isEmpty() ) {
           TQString authorlist;
           for (TQValueList<TDEAboutPerson>::ConstIterator it = authors.begin(); it != authors.end(); ++it ) {
             TQString email;
             if ( !(*it).emailAddress().isEmpty() )
               email = " <" + (*it).emailAddress() + ">";
             authorlist += TQString("    ") + (*it).name() + email + "\n";
           }
           printQ( i18n("the 2nd argument is a list of name+address, one on each line","%1 was written by\n%2").arg ( TQString(about->programName()) ).arg( authorlist ) );
         }
       } else {
         printQ( i18n("This application was written by somebody who wants to remain anonymous.") );
       }
       if (about)
       {
         if (!about->customAuthorTextEnabled ())
         {
           if (about->bugAddress().isEmpty()
                 || about->bugAddress() == "submit@bugs.trinitydesktop.org"
                 || about->bugAddress() == "http://bugs.trinitydesktop.org" )
             printQ( i18n( "Please use http://bugs.trinitydesktop.org to report bugs.\n" ) );
           else {
             if( about->authors().count() == 1 && about->authors().first().emailAddress() == about->bugAddress() )
               printQ( i18n( "Please report bugs to %1.\n" ).arg( about->authors().first().emailAddress() ) );
             else
               printQ( i18n( "Please report bugs to %1.\n" ).arg(about->bugAddress()) );
           }
         }
         else
         {
           printQ(about->customAuthorPlainText());
         }
       }
       exit(0);
         } else {
           if ((option[0] == 'n') && (option[1] == 'o'))
           {
              option += 2;
              enabled = false;
           }
           findOption(orig, option, i, enabled, inOptions);
         }
      }
      else
      {
         // Check whether appOptions allows these arguments
         if (!allowArgs)
         {
            if (ignoreUnknown)
               continue;
            enable_i18n();
            usage( i18n("Unexpected argument '%1'.").arg(TQString::fromLocal8Bit(argv[i])));
         }
         else
         {
            appOptions->addArgument(argv[i]);
            if (everythingAfterArgIsArgs)
                inOptions = false;
         }
      }
   }
   parsed = true;
}

/**
 * For TDEApplication only:
 *
 * Return argc
 */
int *
TDECmdLineArgs::tqt_argc()
{
   if (!argsList)
      TDEApplication::addCmdLineOptions(); // Lazy bastards!

   static int tqt_argc = -1;
   if( tqt_argc != -1 )
      return &tqt_argc;

   TDECmdLineArgs *args = parsedArgs("qt");
   assert(args); // No qt options have been added!
   if (!argv)
   {
      fprintf(stderr, "\n\nFAILURE (TDECmdLineArgs):\n");
      fprintf(stderr, "Application has not called TDECmdLineArgs::init(...).\n\n");

      assert( 0 );
      exit(255);
   }

   assert(argc >= (args->count()+1));
   tqt_argc = args->count() +1;
   return &tqt_argc;
}

/**
 * For TDEApplication only:
 *
 * Return argv
 */
char ***
TDECmdLineArgs::tqt_argv()
{
   if (!argsList)
      TDEApplication::addCmdLineOptions(); // Lazy bastards!

   static char** tqt_argv;
   if( tqt_argv != NULL )
      return &tqt_argv;

   TDECmdLineArgs *args = parsedArgs("qt");
   assert(args); // No qt options have been added!
   if (!argv)
   {
      fprintf(stderr, "\n\nFAILURE (TDECmdLineArgs):\n");
      fprintf(stderr, "Application has not called TDECmdLineArgs::init(...).\n\n");

      assert( 0 );
      exit(255);
   }

   tqt_argv = new char*[ args->count() + 2 ];
   tqt_argv[ 0 ] = tqstrdup( appName());
   int i = 0;
   for(; i < args->count(); i++)
   {
      tqt_argv[i+1] = tqstrdup((char *) args->arg(i));
   }
   tqt_argv[i+1] = 0;

   return &tqt_argv;
}

void
TDECmdLineArgs::enable_i18n()
{
    // called twice or too late
    if (TDEGlobal::_locale)
      return;

    if (!TDEGlobal::_instance) {
  TDEInstance *instance = new TDEInstance(about);
  (void) instance->config();
  // Don't delete instance!
    }
}

void
TDECmdLineArgs::usage(const TQString &error)
{
    assert(TDEGlobal::_locale);
    TQCString localError = error.local8Bit();
    if (localError[error.length()-1] == '\n')
  localError = localError.left(error.length()-1);
    fprintf(stderr, "%s: %s\n", argv[0], localError.data());

    TQString tmp = i18n("Use --help to get a list of available command line options.");
    localError = tmp.local8Bit();
    fprintf(stderr, "%s: %s\n", argv[0], localError.data());
    exit(254);
}

void
TDECmdLineArgs::usage(const char *id)
{
   enable_i18n();
   assert(argsList != 0); // It's an error to call usage(...) without
                          // having done addCmdLineOptions first!

   TQString optionFormatString   = "  %1 %2\n";
   TQString optionFormatStringDef  = "  %1 %2 [%3]\n";
   TQString optionHeaderString = i18n("\n%1:\n");
   TQString tmp;
   TQString usage;

   TDECmdLineArgs *args = argsList->last();

   if (!(args->id) && (args->options) &&
       (args->options->name) && (args->options->name[0] != '+'))
   {
      usage = i18n("[options] ")+usage;
   }

   while(args)
   {
      if (args->name)
      {
         usage = i18n("[%1-options]").arg(args->name)+" "+usage;
      }
      args = argsList->prev();
   }

   TDECmdLineArgs *appOptions = argsList->last();
   if (!appOptions->id)
   {
     const TDECmdLineOptions *option = appOptions->options;
     while(option && option->name)
     {
       if (option->name[0] == '+')
          usage = usage + (option->name+1) + " ";
       else if ( option->name[0] == '!' && option->name[1] == '+' )
          usage = usage + (option->name+2) + " ";

       option++;
     }
   }

   printQ(i18n("Usage: %1 %2\n").arg(argv[0]).arg(usage));
   printQ("\n"+about->shortDescription()+"\n");

   printQ(optionHeaderString.arg(i18n("Generic options")));
   printQ(optionFormatString.arg("--help", -25).arg(i18n("Show help about options")));

   args = argsList->first();
   while(args)
   {
      if (args->name && args->id)
      {
         TQString option = TQString("--help-%1").arg(args->id);
         TQString desc = i18n("Show %1 specific options").arg(args->name);

         printQ(optionFormatString.arg(option, -25).arg(desc));
      }
      args = argsList->next();
   }

   printQ(optionFormatString.arg("--help-all",-25).arg(i18n("Show all options")));
   printQ(optionFormatString.arg("--author",-25).arg(i18n("Show author information")));
   printQ(optionFormatString.arg("-v, --version",-25).arg(i18n("Show version information")));
   printQ(optionFormatString.arg("--license",-25).arg(i18n("Show license information")));
   printQ(optionFormatString.arg("--", -25).arg(i18n("End of options")));

   args = argsList->first(); // Sets current to 1st.

   bool showAll = id && (::qstrcmp(id, "all") == 0);

   if (!showAll)
   {
     while(args)
     {
       if (!id && !args->id) break;
       if (id && (::qstrcmp(args->id, id) == 0)) break;
       args = argsList->next();
     }
   }

   while(args)
   {
     bool hasArgs = false;
     bool hasOptions = false;
     TQString optionsHeader;
     if (args->name)
        optionsHeader = optionHeaderString.arg(i18n("%1 options").arg(TQString::fromLatin1(args->name)));
     else
        optionsHeader = i18n("\nOptions:\n");

     while (args)
     {
       const TDECmdLineOptions *option = args->options;
       TQCString opt = "";
//
       while(option && option->name)
       {
         TQString description;
         TQString descriptionRest;
         TQStringList dl;

         // Option header
         if (option->name[0] == ':')
         {
            if (option->description)
            {
               optionsHeader = "\n"+i18n(option->description);
               if (!optionsHeader.endsWith("\n"))
                  optionsHeader.append("\n");
               hasOptions = false;
            }
            option++;
            continue;
         }

         // Free-form comment
         if (option->name[0] == 0)
         {
            if (option->description)
            {
               TQString tmp = "\n"+i18n(option->description);
               if (!tmp.endsWith("\n"))
                  tmp.append("\n");
               printQ(tmp);
            }
            option++;
            continue;
         }

         // Options
         if (option->description)
         {
            description = i18n(option->description);
            dl = TQStringList::split("\n", description, true);
            description = dl.first();
            dl.remove( dl.begin() );
         }
         TQCString name = option->name;
         if (name[0] == '!')
             name = name.mid(1);

         if (name[0] == '+')
         {
            if (!hasArgs)
            {
               printQ(i18n("\nArguments:\n"));
               hasArgs = true;
            }

            name = name.mid(1);
            if ((name[0] == '[') && (name[name.length()-1] == ']'))
         name = name.mid(1, name.length()-2);
            printQ(optionFormatString.arg(TQString(name), -25)
     .arg(description));
         }
         else
         {
            if (!hasOptions)
            {
               printQ(optionsHeader);
               hasOptions = true;
            }

            if ((name.length() == 1) || (name[1] == ' '))
               name = "-"+name;
            else
               name = "--"+name;
            if (!option->description)
            {
               opt = name + ", ";
            }
            else
            {
               opt = opt + name;
               if (!option->def)
               {
                  printQ(optionFormatString.arg(TQString(opt), -25)
                         .arg(description));
               }
               else
               {
                  printQ(optionFormatStringDef.arg(TQString(opt), -25)
                         .arg(description).arg(option->def));
               }
               opt = "";
            }
         }
         for(TQStringList::Iterator it = dl.begin();
             it != dl.end();
             ++it)
         {
            printQ(optionFormatString.arg("", -25).arg(*it));
         }

         option++;
       }
       args = argsList->next();
       if (!args || args->name || !args->id) break;
     }
     if (!showAll) break;
   }

   exit(254);
}

//
// Member functions
//

/**
 *  Constructor.
 *
 *  The given arguments are assumed to be constants.
 */
TDECmdLineArgs::TDECmdLineArgs( const TDECmdLineOptions *_options,
                            const char *_name, const char *_id)
  : options(_options), name(_name), id(_id)
{
  parsedOptionList = 0;
  parsedArgList = 0;
  isQt = (::qstrcmp(id, "qt") == 0);
}

/**
 *  Destructor.
 */
TDECmdLineArgs::~TDECmdLineArgs()
{
  delete parsedOptionList;
  delete parsedArgList;
  if (argsList)
     argsList->removeRef(this);
}

void
TDECmdLineArgs::clear()
{
   delete parsedArgList;
   parsedArgList = 0;
   delete parsedOptionList;
   parsedOptionList = 0;
}

void
TDECmdLineArgs::reset()
{
   if ( argsList ) {
      argsList->setAutoDelete( true );
      argsList->clear();
      delete argsList;
      argsList = 0;
   }
   parsed = false;
}

void
TDECmdLineArgs::save( TQDataStream &ds) const
{
   uint count = 0;
   if (parsedOptionList)
      parsedOptionList->save( ds );
   else
      ds << count;

   if (parsedArgList)
      parsedArgList->save( ds );
   else
      ds << count;
}

void
TDECmdLineArgs::load( TQDataStream &ds)
{
   if (!parsedOptionList) parsedOptionList = new TDECmdLineParsedOptions;
   if (!parsedArgList) parsedArgList = new TDECmdLineParsedArgs;

   parsedOptionList->load( ds );
   parsedArgList->load( ds );

   if (parsedOptionList->count() == 0)
   {
      delete parsedOptionList;
      parsedOptionList = 0;
   }
   if (parsedArgList->count() == 0)
   {
      delete parsedArgList;
      parsedArgList = 0;
   }
}

void
TDECmdLineArgs::setOption(const TQCString &opt, bool enabled)
{
   if (isQt)
   {
      // Qt does it own parsing.
      TQCString arg = "-";
      if( !enabled )
          arg += "no";
      arg += opt;
      addArgument(arg);
   }
   if (!parsedOptionList) {
  parsedOptionList = new TDECmdLineParsedOptions;
  parsedOptionList->setAutoDelete(true);
   }

   if (enabled)
      parsedOptionList->replace( opt, new TQCString("t") );
   else
      parsedOptionList->replace( opt, new TQCString("f") );
}

void
TDECmdLineArgs::setOption(const TQCString &opt, const char *value)
{
   if (isQt)
   {
      // Qt does it's own parsing.
      TQCString arg = "-";
      arg += opt;
      addArgument(arg);
      addArgument(value);

#ifdef TQ_WS_X11
      // Hack coming up!
      if (arg == "-display")
      {
         setenv(DISPLAY, value, true);
      }
#endif
   }
   if (!parsedOptionList) {
  parsedOptionList = new TDECmdLineParsedOptions;
  parsedOptionList->setAutoDelete(true);
   }

   parsedOptionList->insert( opt, new TQCString(value) );
}

TQCString
TDECmdLineArgs::getOption(const char *_opt) const
{
   TQCString *value = 0;
   if (parsedOptionList)
   {
      value = parsedOptionList->find(_opt);
   }

   if (value)
      return (*value);

   // Look up the default.
   const char *opt_name;
   const char *def;
   bool dummy = true;
   TQCString opt = _opt;
   int result = ::findOption( options, opt, opt_name, def, dummy) & ~4;

   if (result != 3)
   {
      fprintf(stderr, "\n\nFAILURE (TDECmdLineArgs):\n");
      fprintf(stderr, "Application requests for getOption(\"%s\") but the \"%s\" option\n",
                      _opt, _opt);
      fprintf(stderr, "has never been specified via addCmdLineOptions( ... )\n\n");

      assert( 0 );
      exit(255);
   }
   return TQCString(def);
}

QCStringList
TDECmdLineArgs::getOptionList(const char *_opt) const
{
   QCStringList result;
   if (!parsedOptionList)
      return result;

   while(true)
   {
      TQCString *value = parsedOptionList->take(_opt);
      if (!value)
         break;
      result.prepend(*value);
      delete value;
   }

   // Reinsert items in dictionary
   // WABA: This is rather silly, but I don't want to add restrictions
   // to the API like "you can only call this function once".
   // I can't access all items without taking them out of the list.
   // So taking them out and then putting them back is the only way.
   for(QCStringList::ConstIterator it=result.begin();
       it != result.end();
       ++it)
   {
      parsedOptionList->insert(_opt, new TQCString(*it));
   }
   return result;
}

bool
TDECmdLineArgs::isSet(const char *_opt) const
{
   // Look up the default.
   const char *opt_name;
   const char *def;
   bool dummy = true;
   TQCString opt = _opt;
   int result = ::findOption( options, opt, opt_name, def, dummy) & ~4;

   if (result == 0)
   {
      fprintf(stderr, "\n\nFAILURE (TDECmdLineArgs):\n");
      fprintf(stderr, "Application requests for isSet(\"%s\") but the \"%s\" option\n",
                      _opt, _opt);
      fprintf(stderr, "has never been specified via addCmdLineOptions( ... )\n\n");

      assert( 0 );
      exit(255);
   }

   TQCString *value = 0;
   if (parsedOptionList)
   {
      value = parsedOptionList->find(opt);
   }

   if (value)
   {
      if (result == 3)
         return true;
      else
         return ((*value)[0] == 't');
   }

   if (result == 3)
      return false; // String option has 'false' as default.

   // We return 'true' as default if the option was listed as '-nofork'
   // We return 'false' as default if the option was listed as '-fork'
   return (result == 2);
}

int
TDECmdLineArgs::count() const
{
   if (!parsedArgList)
      return 0;
   return parsedArgList->count();
}

const char *
TDECmdLineArgs::arg(int n) const
{
   if (!parsedArgList || (n >= (int) parsedArgList->count()))
   {
      fprintf(stderr, "\n\nFAILURE (TDECmdLineArgs): Argument out of bounds\n");
      fprintf(stderr, "Application requests for arg(%d) without checking count() first.\n",
                      n);

      assert( 0 );
      exit(255);
   }

   return parsedArgList->at(n);
}

KURL
TDECmdLineArgs::url(int n) const
{
   return makeURL( arg(n) );
}

KURL TDECmdLineArgs::makeURL(const char *_urlArg)
{
   const TQString urlArg = TQFile::decodeName(_urlArg);
   TQFileInfo fileInfo(urlArg);
   if (!fileInfo.isRelative()) { // i.e. starts with '/', on unix
      KURL result;
      result.setPath(urlArg);
      return result; // Absolute path.
   }

   if ( KURL::isRelativeURL(urlArg) || fileInfo.exists() ) {
      KURL result;
      result.setPath( cwd()+'/'+urlArg );
      result.cleanPath();
      return result;  // Relative path
   }

   return KURL(urlArg); // Argument is a URL
}

void
TDECmdLineArgs::addArgument(const char *argument)
{
   if (!parsedArgList)
      parsedArgList = new TDECmdLineParsedArgs;

   parsedArgList->append(argument);
}

static const TDECmdLineOptions kde_tempfile_option[] =
{
   { "tempfile",       I18N_NOOP("The files/URLs opened by the application will be deleted after use"), 0},
   TDECmdLineLastOption
};

void
TDECmdLineArgs::addTempFileOption()
{
    TDECmdLineArgs::addCmdLineOptions( kde_tempfile_option, "TDE-tempfile", "tde-tempfile" );
}

bool TDECmdLineArgs::isTempFileSet()
{
    TDECmdLineArgs* args = TDECmdLineArgs::parsedArgs( "tde-tempfile" );
    if ( args )
        return args->isSet( "tempfile" );
    return false;
}

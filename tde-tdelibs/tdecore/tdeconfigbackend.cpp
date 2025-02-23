/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include <config.h>

#include <unistd.h>
#include <ctype.h>
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>

#include <tqdir.h>
#include <tqfileinfo.h>
#include <tqtextcodec.h>
#include <tqtextstream.h>

#include "tdeconfigbackend.h"
#include "tdeconfigbase.h"
#include <tdeapplication.h>
#include <tdeglobal.h>
#include <kprocess.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <ksavefile.h>
#include <kurl.h>
#include <kde_file.h>

extern bool checkAccess(const TQString& pathname, int mode);
/* translate escaped escape sequences to their actual values. */
static TQCString printableToString(const char *str, int l)
{
  // Strip leading white-space.
  while((l>0) &&
        ((*str == ' ') || (*str == '\t') || (*str == '\r')))
  {
     str++; l--;
  }

  // Strip trailing white-space.
  while((l>0) &&
        ((str[l-1] == ' ') || (str[l-1] == '\t') || (str[l-1] == '\r')))
  {
     l--;
  }

  TQCString result(l + 1);
  char *r = result.data();

  for(int i = 0; i < l;i++, str++)
  {
     if (*str == '\\')
     {
        i++, str++;
        if (i >= l) // End of line. (Line ends with single slash)
        {
           *r++ = '\\';
           break;
        }
        switch(*str)
        {
           case 's':
              *r++ = ' ';
              break;
           case 't':
              *r++ = '\t';
              break;
           case 'n':
              *r++ = '\n';
              break;
           case 'r':
              *r++ = '\r';
              break;
           case '\\':
              *r++ = '\\';
              break;
           default:
              *r++ = '\\';
              *r++ = *str;
        }
     }
     else
     {
        *r++ = *str;
     }
  }
  result.truncate(r-result.data());
  return result;
}

static TQCString stringToPrintable(const TQCString& str){
  TQCString result(str.length()*2); // Maximum 2x as long as source string
  char *r = const_cast<TQCString&>(result).data();
  char *s = const_cast<TQCString&>(str).data();

  if (!s) return TQCString("");

  // Escape leading space
  if (*s == ' ')
  {
     *r++ = '\\'; *r++ = 's';
     s++;
  }

  if (*s)
  {
   while(*s)
   {
    if (*s == '\n')
    {
      *r++ = '\\'; *r++ = 'n';
    }
    else if (*s == '\t')
    {
      *r++ = '\\'; *r++ = 't';
    }
    else if (*s == '\r')
    {
      *r++ = '\\'; *r++ = 'r';
    }
    else if (*s == '\\')
    {
      *r++ = '\\'; *r++ = '\\';
    }
    else
    {
      *r++ = *s;
    }
    s++;
   }
   // Escape trailing space
   if (*(r-1) == ' ')
   {
      *(r-1) = '\\'; *r++ = 's';
   }
  }

  result.truncate(r - result.data());
  return result;
}

static TQCString decodeGroup(const char*s, int l)
{
  TQCString result(l);
  char *r = result.data();

  l--; // Correct for trailing \0
  while(l)
  {
    if ((*s == '[') && (l > 1))
    {
       if ((*(s+1) == '['))
       {
          l--;
          s++;
       }
    }
    if ((*s == ']') && (l > 1))
    {
       if ((*(s+1) == ']'))
       {
          l--;
          s++;
       }
    }
    *r++ = *s++;
    l--;
  }
  result.truncate(r - result.data());
  return result;
}

static TQCString encodeGroup(const TQCString &str)
{
  int l = str.length();
  TQCString result(l*2+1);
  char *r = const_cast<TQCString&>(result).data();
  char *s = const_cast<TQCString&>(str).data();
  while(l)
  {
    if ((*s == '[') || (*s == ']'))
       *r++ = *s;
    *r++ = *s++;
    l--;
  }
  result.truncate(r - result.data());
  return result;
}

static TQCString encodeKey(const char* key)
{
    TQCString newKey(key);

    newKey.replace('[', "%5b");
    newKey.replace(']', "%5d");

    return newKey;
}

static TQCString decodeKey(const char* key)
{
    TQCString newKey(key);

    newKey.replace("%5b", "[");
    newKey.replace("%5d", "]");

    return newKey;
}

class TDEConfigBackEnd::TDEConfigBackEndPrivate
{
public:
   TQDateTime localLastModified;
   uint      localLastSize;
   TDELockFile::Ptr localLockFile;
   TDELockFile::Ptr globalLockFile;
};

void TDEConfigBackEnd::changeFileName(const TQString &_fileName,
                                    const char * _resType,
                                    bool _useKDEGlobals)
{
   mfileName = _fileName;
   resType = _resType;
   useKDEGlobals = _useKDEGlobals;
   if (mfileName.isEmpty()) {
      mLocalFileName = TQString::null;
   }
   else if (!TQDir::isRelativePath(mfileName)) {
      mLocalFileName = mfileName;
   }
   else {
      mLocalFileName = TDEGlobal::dirs()->saveLocation(resType, TQString(), false) + mfileName;
   }

   if (useKDEGlobals) {
      mGlobalFileName = TDEGlobal::dirs()->saveLocation("config", TQString(), false) + TQString::fromLatin1("kdeglobals");
   }
   else {
      mGlobalFileName = TQString::null;
   }

   d->localLastModified = TQDateTime();
   d->localLastSize = 0;
   d->localLockFile = 0;
   d->globalLockFile = 0;
}

TDELockFile::Ptr TDEConfigBackEnd::lockFile(bool bGlobal)
{
   if (bGlobal)
   {
      if (d->globalLockFile)
         return d->globalLockFile;
      
      if (!mGlobalFileName.isEmpty())
      {
         d->globalLockFile = new TDELockFile(mGlobalFileName+".lock");
         return d->globalLockFile;
      }
   }
   else
   {
      if (d->localLockFile)
         return d->localLockFile;
      
      if (!mLocalFileName.isEmpty())
      {
         d->localLockFile = new TDELockFile(mLocalFileName+".lock");
         return d->localLockFile;
      }
   }
   return 0;
}

TDEConfigBackEnd::TDEConfigBackEnd(TDEConfigBase *_config,
			       const TQString &_fileName,
			       const char * _resType,
			       bool _useKDEGlobals)
  : pConfig(_config), bFileImmutable(false), mConfigState(TDEConfigBase::NoAccess), mFileMode(-1)
{
   d = new TDEConfigBackEndPrivate;
   changeFileName(_fileName, _resType, _useKDEGlobals);
}

TDEConfigBackEnd::~TDEConfigBackEnd()
{
   delete d;
}

void TDEConfigBackEnd::setFileWriteMode(int mode)
{
  mFileMode = mode;
}

bool TDEConfigINIBackEnd::parseConfigFiles()
{
  // Check if we can write to the local file.
  mConfigState = TDEConfigBase::ReadOnly;
  if (!mLocalFileName.isEmpty() && !pConfig->isReadOnly())
  {
     if (checkAccess(mLocalFileName, W_OK))
     {
        mConfigState = TDEConfigBase::ReadWrite;
     }
     else
     {
        // Create the containing dir, maybe it wasn't there
        KURL path;
        path.setPath(mLocalFileName);
        TQString dir=path.directory();
        TDEStandardDirs::makeDir(dir);

        if (checkAccess(mLocalFileName, W_OK))
        {
           mConfigState = TDEConfigBase::ReadWrite;
        }
     }
     TQFileInfo info(mLocalFileName);
     d->localLastModified = info.lastModified();
     d->localLastSize = info.size();
  }

  // Parse all desired files from the least to the most specific.
  bFileImmutable = false;

  // Parse the general config files
  if (useKDEGlobals) {
    TQStringList tdercs = TDEGlobal::dirs()->
      findAllResources("config", TQString::fromLatin1("kdeglobals"));

#ifdef TQ_WS_WIN
    TQString etc_tderc = TQFile::decodeName( TQCString(getenv("WINDIR")) + "\\tderc" );
#else
    TQString etc_tderc = TQString::fromLatin1("/etc/tderc");
#endif

    if (checkAccess(etc_tderc, R_OK))
      tdercs += etc_tderc;

    tdercs += TDEGlobal::dirs()->
      findAllResources("config", TQString::fromLatin1("system.kdeglobals"));

    TQStringList::ConstIterator it;

    for (it = tdercs.fromLast(); it != tdercs.end(); --it) {

      TQFile aConfigFile( *it );
      if (!aConfigFile.open( IO_ReadOnly ))
	   continue;
      parseSingleConfigFile( aConfigFile, 0L, true, (*it != mGlobalFileName) );
      aConfigFile.close();
      if (bFileImmutable)
         break;
    }
  }

  bool bReadFile = !mfileName.isEmpty();
  while(bReadFile) {
    bReadFile = false;
    TQString bootLanguage;
    if (useKDEGlobals && localeString.isEmpty() && !TDEGlobal::_locale) {
       // Boot strap language
       bootLanguage = TDELocale::_initLanguage(pConfig);
       setLocaleString(bootLanguage.utf8());
    }

    bFileImmutable = false;
    TQStringList list;
    if ( !TQDir::isRelativePath(mfileName) )
       list << mfileName;
    else
       list = TDEGlobal::dirs()->findAllResources(resType, mfileName);

    TQStringList::ConstIterator it;

    for (it = list.fromLast(); it != list.end(); --it) {

      TQFile aConfigFile( *it );
      // we can already be sure that this file exists
      bool bIsLocal = (*it == mLocalFileName);
      if (aConfigFile.open( IO_ReadOnly )) {
         parseSingleConfigFile( aConfigFile, 0L, false, !bIsLocal );
         aConfigFile.close();
         if (bFileImmutable)
            break;
      }
    }
    if (TDEGlobal::dirs()->isRestrictedResource(resType, mfileName))
       bFileImmutable = true;
    TQString currentLanguage;
    if (!bootLanguage.isEmpty())
    {
       currentLanguage = TDELocale::_initLanguage(pConfig);
       // If the file changed the language, we need to read the file again
       // with the new language setting.
       if (bootLanguage != currentLanguage)
       {
          bReadFile = true;
          setLocaleString(currentLanguage.utf8());
       }
    }
  }
  if (bFileImmutable)
     mConfigState = TDEConfigBase::ReadOnly;

  return true;
}

#ifdef HAVE_MMAP
#ifdef SIGBUS
static sigjmp_buf mmap_jmpbuf;
struct sigaction mmap_old_sigact;

extern "C" {
   static void mmap_sigbus_handler(int)
   {
      siglongjmp (mmap_jmpbuf, 1);
   }
}
#endif
#endif

extern bool kde_kiosk_exception;

void TDEConfigINIBackEnd::parseSingleConfigFile(TQFile &rFile,
					      KEntryMap *pWriteBackMap,
					      bool bGlobal, bool bDefault)
{
   const char *s; // May get clobbered by sigsetjump, but we don't use them afterwards.
   const char *eof; // May get clobbered by sigsetjump, but we don't use them afterwards.
   TQByteArray data;

   if (!rFile.isOpen()) // come back, if you have real work for us ;->
      return;

   //using kdDebug() here leads to an infinite loop
   //remove this for the release, aleXXX
   //tqWarning("Parsing %s, global = %s default = %s",
   //           rFile.name().latin1(), bGlobal ? "true" : "false", bDefault ? "true" : "false");

   TQCString aCurrentGroup("<default>");

   unsigned int ll = localeString.length();

#ifdef HAVE_MMAP
   static volatile const char *map;
   map = ( const char* ) mmap(0, rFile.size(), PROT_READ, MAP_PRIVATE,
                                          rFile.handle(), 0);

   if ( map != MAP_FAILED )
   {
      s = (const char*) map;
      eof = s + rFile.size();

#ifdef SIGBUS
      struct sigaction act;
      act.sa_handler = mmap_sigbus_handler;
      sigemptyset( &act.sa_mask );
#ifdef SA_ONESHOT
      act.sa_flags = SA_ONESHOT;
#else
      act.sa_flags = SA_RESETHAND;
#endif      
      sigaction( SIGBUS, &act, &mmap_old_sigact );

      if (sigsetjmp (mmap_jmpbuf, 1))
      {
tqWarning("SIGBUS while reading %s", rFile.name().latin1());
         munmap(( char* )map, rFile.size());
         sigaction (SIGBUS, &mmap_old_sigact, 0);
         return;
      }
#endif
   }
   else
#endif
   {
      rFile.at(0);
      data = rFile.readAll();
      s = data.data();
      eof = s + data.size();
   }

   bool fileOptionImmutable = false;
   bool groupOptionImmutable = false;
   bool groupSkip = false;
   bool foundGettextDomain = false;
   TQCString gettextDomain;

   int line = 0;
   for(; s < eof; s++)
   {
      line++;

      while((s < eof) && isspace(*s) && (*s != '\n'))
         s++; //skip leading whitespace, shouldn't happen too often

      //skip empty lines, lines starting with #
      if ((s < eof) && ((*s == '\n') || (*s == '#')))
      {
    sktoeol:	//skip till end-of-line
         while ((s < eof) && (*s != '\n'))
            s++;
         continue; // Empty or comment or no keyword
      }
      const char *startLine = s;

      if (*s == '[')  //group
      {
         // In a group [[ and ]] have a special meaning
         while ((s < eof) && (*s != '\n')) 
         {
            if (*s == ']')
            {
               if ((s+1 < eof) && (*(s+1) == ']'))
                  s++; // Skip "]]"
               else
                  break;
            }

            s++; // Search till end of group
         }
         const char *e = s;
         while ((s < eof) && (*s != '\n')) s++; // Search till end of line / end of file
         if ((e >= eof) || (*e != ']'))
         {
            fprintf(stderr, "Invalid group header at %s:%d\n", rFile.name().latin1(), line);
            continue;
         }
         // group found; get the group name by taking everything in
         // between the brackets
         if ((e-startLine == 3) &&
             (startLine[1] == '$') &&
             (startLine[2] == 'i'))
         {
            if (!kde_kiosk_exception)
               fileOptionImmutable = true;
            continue;
         }

         aCurrentGroup = decodeGroup(startLine + 1, e - startLine);
         //cout<<"found group ["<<aCurrentGroup<<"]"<<endl;

         // Backwards compatibility
         if (aCurrentGroup == "KDE Desktop Entry")
            aCurrentGroup = "Desktop Entry";

         groupOptionImmutable = fileOptionImmutable;

         e++;
         if ((e+2 < eof) && (*e++ == '[') && (*e++ == '$')) // Option follows
         {
            if ((*e == 'i') && !kde_kiosk_exception)
            {
               groupOptionImmutable = true;
            }
         }

         KEntryKey groupKey(aCurrentGroup, 0);
         KEntry entry = pConfig->lookupData(groupKey);
         groupSkip = entry.bImmutable;

         if (groupSkip && !bDefault)
            continue;

         entry.bImmutable |= groupOptionImmutable;
         pConfig->putData(groupKey, entry, false);

         if (pWriteBackMap)
         {
            // add the special group key indicator
            (*pWriteBackMap)[groupKey] = entry;
         }

         continue;
      }
      if (groupSkip && !bDefault)
        goto sktoeol; // Skip entry


      bool optionImmutable = groupOptionImmutable;
      bool optionDeleted = false;
      bool optionExpand = false;
      const char *endOfKey = 0, *locale = 0, *elocale = 0;
      for (; (s < eof) && (*s != '\n'); s++)
      {
         if (*s == '=') //find the equal sign
         {
	    if (!endOfKey)
        	endOfKey = s;
            goto haveeq;
	 }
	 if (*s == '[') //find the locale or options.
	 {
            const char *option;
            const char *eoption;
	    endOfKey = s;
	    option = ++s;
	    for (;; s++)
	    {
		if ((s >= eof) || (*s == '\n') || (*s == '=')) {
		    fprintf(stderr, "Invalid entry (missing ']') at %s:%d\n", rFile.name().latin1(), line);
		    goto sktoeol;
		}
		if (*s == ']')
		    break;
	    }
	    eoption = s;
            if (*option != '$')
            {
              // Locale
              if (locale) {
		fprintf(stderr, "Invalid entry (second locale!?) at %s:%d\n", rFile.name().latin1(), line);
		goto sktoeol;
              }
              locale = option;
              elocale = eoption;
            }
            else
            {
              // Option
              while (option < eoption)
              {
                 option++;
                 if ((*option == 'i') && !kde_kiosk_exception)
                    optionImmutable = true;
                 else if (*option == 'e')
                    optionExpand = true;
                 else if (*option == 'd')
                 {
                    optionDeleted = true;
                    goto haveeq;
                 }
		 else if (*option == ']')
		    break;
              }
            }
         }
      }
      fprintf(stderr, "Invalid entry (missing '=') at %s:%d\n", rFile.name().latin1(), line);
      continue;

   haveeq:
      for (endOfKey--; ; endOfKey--)
      {
	 if (endOfKey < startLine)
	 {
	   fprintf(stderr, "Invalid entry (empty key) at %s:%d\n", rFile.name().latin1(), line);
	   goto sktoeol;
	 }
	 if (!isspace(*endOfKey))
	    break;
      }

      const char *st = ++s;
      while ((s < eof) && (*s != '\n')) s++; // Search till end of line / end of file

      if (locale) {
          unsigned int cl = static_cast<unsigned int>(elocale - locale);
          if ((ll != cl) || memcmp(locale, localeString.data(), ll))
          {
              // backward compatibility. C == en_US
              if ( cl != 1 || ll != 5 || *locale != 'C' || memcmp(localeString.data(), "en_US", 5)) {
                  //cout<<"mismatched locale '"<<TQCString(locale, elocale-locale +1)<<"'"<<endl;
                  // We can ignore this one
                  if (!pWriteBackMap)
                      continue; // We just ignore it
                  // We just store it as is to be able to write it back later.
                  endOfKey = elocale;
                  locale = 0;
              }
          }
      }

      // insert the key/value line
      TQCString key(startLine, endOfKey - startLine + 2);
      TQCString val = printableToString(st, s - st);
      //tqDebug("found key '%s' with value '%s'", key.data(), val.data());

      if (TQString(key.data()) == "X-Ubuntu-Gettext-Domain") {
	gettextDomain = val.data();
	foundGettextDomain = true;
      }

      KEntryKey aEntryKey(aCurrentGroup, decodeKey(key));
      aEntryKey.bLocal = (locale != 0);
      aEntryKey.bDefault = bDefault;

      KEntry aEntry;
      aEntry.mValue = val;
      aEntry.bGlobal = bGlobal;
      aEntry.bImmutable = optionImmutable;
      aEntry.bDeleted = optionDeleted;
      aEntry.bExpand = optionExpand;
      aEntry.bNLS = (locale != 0);

      if (pWriteBackMap) {
         // don't insert into the config object but into the temporary
         // scratchpad map
         pWriteBackMap->insert(aEntryKey, aEntry);
      } else {
         // directly insert value into config object
         // no need to specify localization; if the key we just
         // retrieved was localized already, no need to localize it again.
         pConfig->putData(aEntryKey, aEntry, false);
      }
   }
   // Look up translations using TDELocale
   // https://launchpad.net/distros/ubuntu/+spec/langpacks-desktopfiles-kde
   // This calls TDELocale up to 10 times for each config file (and each TDEConfig has up to 4 files)
   // so I'll see how much of a performance hit it is
   // it also only acts on the last group in a file
   // Ideas: only translate most important fields, only translate "Desktop Entry" files,
   //        do translation per TDEConfig not per single file
   if (!pWriteBackMap) {
     TQFile file("file.txt");
     if (foundGettextDomain) {

       TDELocale locale(gettextDomain);

       TQString language = locale.language();
       translateKey(locale, aCurrentGroup, TQCString("Name"));
       translateKey(locale, aCurrentGroup, TQCString("Comment"));
       translateKey(locale, aCurrentGroup, TQCString("Language"));
       translateKey(locale, aCurrentGroup, TQCString("Keywords"));
       translateKey(locale, aCurrentGroup, TQCString("About"));
       translateKey(locale, aCurrentGroup, TQCString("Description"));
       translateKey(locale, aCurrentGroup, TQCString("GenericName"));
       translateKey(locale, aCurrentGroup, TQCString("Query"));
       translateKey(locale, aCurrentGroup, TQCString("ExtraNames"));
       translateKey(locale, aCurrentGroup, TQCString("X-TDE-Submenu"));
     }
   }


   if (fileOptionImmutable)
      bFileImmutable = true;

#ifdef HAVE_MMAP
   if (map)
   {
      munmap(( char* )map, rFile.size());
#ifdef SIGBUS
      sigaction (SIGBUS, &mmap_old_sigact, 0);
#endif
   }
#endif
}

void TDEConfigINIBackEnd::translateKey(TDELocale& locale, TQCString currentGroup, TQCString key) {
  KEntryKey entryKey = KEntryKey(currentGroup, key);
  KEntry entry = pConfig->lookupData(entryKey);
  if (TQString(entry.mValue) != "") {
    TQString orig = key + "=" + entry.mValue;
    TQString translate = locale.translate(key + "=" + entry.mValue);
    if (TQString::compare(orig, translate) != 0) {
      translate = translate.mid(key.length() + 1);
      entry.mValue = translate.utf8();
      entryKey.bLocal = true;
      entry.bNLS = true;
      pConfig->putData(entryKey, entry, false);
    }
  }
}

void TDEConfigINIBackEnd::sync(bool bMerge)
{
  // write-sync is only necessary if there are dirty entries
  if (!pConfig->isDirty())
    return;

  bool bEntriesLeft = true;

  // find out the file to write to (most specific writable file)
  // try local app-specific file first

  if (!mfileName.isEmpty()) {
    // Create the containing dir if needed
    if ((resType!="config") && !TQDir::isRelativePath(mLocalFileName))
    {
       KURL path;
       path.setPath(mLocalFileName);
       TQString dir=path.directory();
       TDEStandardDirs::makeDir(dir);
    }

    // Can we allow the write? We can, if the program
    // doesn't run SUID. But if it runs SUID, we must
    // check if the user would be allowed to write if
    // it wasn't SUID.
    if (checkAccess(mLocalFileName, W_OK)) {
      // File is writable
      TDELockFile::Ptr lf;

      bool mergeLocalFile = bMerge;
      // Check if the file has been updated since.
      if (mergeLocalFile)
      {
         lf = lockFile(false); // Lock file for local file
         if (lf && lf->isLocked())
            lf = 0; // Already locked, we don't need to lock/unlock again

         if (lf) 
         {
            lf->lock( TDELockFile::LockForce );
            // But what if the locking failed? Ignore it for now...
         }
         
         TQFileInfo info(mLocalFileName);
         if ((d->localLastSize == info.size()) &&
             (d->localLastModified == info.lastModified()))
         {
            // Not changed, don't merge.
            mergeLocalFile = false;
         }
         else
         {
            // Changed...
            d->localLastModified = TQDateTime();
            d->localLastSize = 0;
         }
      }

      bEntriesLeft = writeConfigFile( mLocalFileName, false, mergeLocalFile );
      
      // Only if we didn't have to merge anything can we use our in-memory state
      // the next time around. Otherwise the config-file may contain entries
      // that are different from our in-memory state which means we will have to 
      // do a merge from then on. 
      // We do not automatically update the in-memory state with the on-disk 
      // state when writing the config to disk. We only do so when 
      // KCOnfig::reparseConfiguration() is called.
      // For KDE 4.0 we may wish to reconsider that.
      if (!mergeLocalFile)
      {
         TQFileInfo info(mLocalFileName);
         d->localLastModified = info.lastModified();
         d->localLastSize = info.size();
      }
      if (lf) lf->unlock();
    }
  }

  // only write out entries to the kdeglobals file if there are any
  // entries marked global (indicated by bEntriesLeft) and
  // the useKDEGlobals flag is set.
  if (bEntriesLeft && useKDEGlobals) {

    // can we allow the write? (see above)
    if (checkAccess ( mGlobalFileName, W_OK )) {
      TDELockFile::Ptr lf = lockFile(true); // Lock file for global file
      if (lf && lf->isLocked())
         lf = 0; // Already locked, we don't need to lock/unlock again

      if (lf) 
      {
         lf->lock( TDELockFile::LockForce );
         // But what if the locking failed? Ignore it for now...
      }
      writeConfigFile( mGlobalFileName, true, bMerge ); // Always merge
      if (lf) lf->unlock();
    }
  }

}

static void writeEntries(FILE *pStream, const KEntryMap& entryMap, bool defaultGroup, bool &firstEntry, const TQCString &localeString)
{
  // now write out all other groups.
  TQCString currentGroup;
  for (KEntryMapConstIterator aIt = entryMap.begin();
       aIt != entryMap.end(); ++aIt)
  {
     const KEntryKey &key = aIt.key();

     // Either proces the default group or all others
     if ((key.mGroup != "<default>") == defaultGroup)
        continue; // Skip

     // Skip default values and group headers.
     if ((key.bDefault) || key.mKey.isEmpty())
        continue; // Skip

     const KEntry &currentEntry = *aIt;

     KEntryMapConstIterator aTestIt = aIt;
     ++aTestIt;
     bool hasDefault = (aTestIt != entryMap.end());
     if (hasDefault)
     {
        const KEntryKey &defaultKey = aTestIt.key();
        if ((!defaultKey.bDefault) ||
            (defaultKey.mKey != key.mKey) ||
            (defaultKey.mGroup != key.mGroup) ||
            (defaultKey.bLocal != key.bLocal))
           hasDefault = false;
     }


     if (hasDefault)
     {
        // Entry had a default value
        if ((currentEntry.mValue == (*aTestIt).mValue) &&
            (currentEntry.bDeleted == (*aTestIt).bDeleted))
           continue; // Same as default, don't write.
     }
     else
     {
        // Entry had no default value.
        if (currentEntry.bDeleted)
           continue; // Don't write deleted entries if there is no default.
     }

     if (!defaultGroup && (currentGroup != key.mGroup)) {
	if (!firstEntry)
	    fprintf(pStream, "\n");
	currentGroup = key.mGroup;
	fprintf(pStream, "[%s]\n", encodeGroup(currentGroup).data());
     }

     firstEntry = false;
     // it is data for a group
     fputs(encodeKey(key.mKey.data()), pStream); // Key

     if ( currentEntry.bNLS )
     {
        fputc('[', pStream);
        fputs(localeString.data(), pStream);
        fputc(']', pStream);
     }

     if (currentEntry.bDeleted)
     {
        fputs("[$d]\n", pStream); // Deleted
     }
     else
     {
        if (currentEntry.bImmutable || currentEntry.bExpand)
        {
           fputc('[', pStream);
           fputc('$', pStream);
           if (currentEntry.bImmutable)
              fputc('i', pStream);
           if (currentEntry.bExpand)
              fputc('e', pStream);

           fputc(']', pStream);
        }
        fputc('=', pStream);
        fputs(stringToPrintable(currentEntry.mValue).data(), pStream);
        fputc('\n', pStream);
     }
  } // for loop
}

bool TDEConfigINIBackEnd::getEntryMap(KEntryMap &aTempMap, bool bGlobal,
                                    TQFile *mergeFile)
{
  bool bEntriesLeft = false;
  bFileImmutable = false;

  // Read entries from disk
  if (mergeFile && mergeFile->open(IO_ReadOnly))
  {
     // fill the temporary structure with entries from the file
     parseSingleConfigFile(*mergeFile, &aTempMap, bGlobal, false );

     if (bFileImmutable) // File has become immutable on disk
        return bEntriesLeft;
  }

  KEntryMap aMap = pConfig->internalEntryMap();

  // augment this structure with the dirty entries from the config object
  for (KEntryMapIterator aIt = aMap.begin();
       aIt != aMap.end(); ++aIt)
  {
    const KEntry &currentEntry = *aIt;
    if(aIt.key().bDefault)
    {
       aTempMap.replace(aIt.key(), currentEntry);
       continue;
    }

    if (mergeFile && !currentEntry.bDirty)
       continue;

    // only write back entries that have the same
    // "globality" as the file
    if (currentEntry.bGlobal != bGlobal)
    {
       // wrong "globality" - might have to be saved later
       bEntriesLeft = true;
       continue;
    }

    // put this entry from the config object into the
    // temporary map, possibly replacing an existing entry
    KEntryMapIterator aIt2 = aTempMap.find(aIt.key());
    if (aIt2 != aTempMap.end() && (*aIt2).bImmutable)
       continue; // Bail out if the on-disk entry is immutable

    aTempMap.insert(aIt.key(), currentEntry, true);
  } // loop

  return bEntriesLeft;
}

/* antlarr: KDE 4.0:  make the first parameter "const TQString &" */
bool TDEConfigINIBackEnd::writeConfigFile(TQString filename, bool bGlobal,
					bool bMerge)
{
  // is the config object read-only?
  if (pConfig->isReadOnly())
    return true; // pretend we wrote it

  KEntryMap aTempMap;
  TQFile *mergeFile = (bMerge ? new TQFile(filename) : 0);
  bool bEntriesLeft = getEntryMap(aTempMap, bGlobal, mergeFile);
  delete mergeFile;
  if (bFileImmutable)
    return true; // pretend we wrote it

  // OK now the temporary map should be full of ALL entries.
  // write it out to disk.

  // Check if file exists:
  int fileMode = -1;
  bool createNew = true;

  KDE_struct_stat buf;
  if (KDE_stat(TQFile::encodeName(filename), &buf) == 0)
  {
     if (buf.st_uid == getuid())
     {
        // Preserve file mode if file exists and is owned by user.
        fileMode = buf.st_mode & 0777;
     }
     else
     {
        // File is not owned by user:
        // Don't create new file but write to existing file instead.
        createNew = false;
     }
  }

  KSaveFile *pConfigFile = 0;
  FILE *pStream = 0;

  if (createNew)
  {
     pConfigFile = new KSaveFile( filename, 0600 );

     if (pConfigFile->status() != 0)
     {
        delete pConfigFile;
        return bEntriesLeft;
     }

     if (!bGlobal && (fileMode == -1))
        fileMode = mFileMode;

     if (fileMode != -1)
     {
        fchmod(pConfigFile->handle(), fileMode);
     }

     pStream = pConfigFile->fstream();
  }
  else
  {
     // Open existing file.
     // We use open() to ensure that we call without O_CREAT.
     int fd = KDE_open( TQFile::encodeName(filename), O_WRONLY | O_TRUNC );
     if (fd < 0)
     {
        return bEntriesLeft;
     }
     pStream = KDE_fdopen( fd, "w");
     if (!pStream)
     {
        close(fd);
        return bEntriesLeft;
     }
  }

  writeEntries(pStream, aTempMap);

  if (pConfigFile)
  {
     bool bEmptyFile = (ftell(pStream) == 0);
     if ( bEmptyFile && ((fileMode == -1) || (fileMode == 0600)) )
     {
        // File is empty and doesn't have special permissions: delete it.
        ::unlink(TQFile::encodeName(filename));
        pConfigFile->abort();
     }
     else
     {
        // Normal case: Close the file
        pConfigFile->close();
     }
     delete pConfigFile;
  }
  else
  {
     fclose(pStream);
  }

  return bEntriesLeft;
}

void TDEConfigINIBackEnd::writeEntries(FILE *pStream, const KEntryMap &aTempMap)
{
  bool firstEntry = true;

  // Write default group
  ::writeEntries(pStream, aTempMap, true, firstEntry, localeString);

  // Write all other groups
  ::writeEntries(pStream, aTempMap, false, firstEntry, localeString);
}

void TDEConfigBackEnd::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void TDEConfigINIBackEnd::virtual_hook( int id, void* data )
{ TDEConfigBackEnd::virtual_hook( id, data ); }

bool TDEConfigBackEnd::checkConfigFilesWritable(bool warnUser)
{
  // WARNING: Do NOT use the event loop as it may not exist at this time.
  bool allWritable = true;
  TQString errorMsg;
  if ( !mLocalFileName.isEmpty() && !bFileImmutable && !checkAccess(mLocalFileName,W_OK) )
  {
    errorMsg = i18n("Will not save configuration.\n");
    allWritable = false;
    errorMsg += i18n("Configuration file \"%1\" not writable.\n").arg(mLocalFileName);
  }
  // We do not have an immutability flag for kdeglobals. However, making kdeglobals mutable while making
  // the local config file immutable is senseless.
  if ( !mGlobalFileName.isEmpty() && useKDEGlobals && !bFileImmutable && !checkAccess(mGlobalFileName,W_OK) )
  {
    if ( errorMsg.isEmpty() )
      errorMsg = i18n("Will not save configuration.\n");
    errorMsg += i18n("Configuration file \"%1\" not writable.\n").arg(mGlobalFileName);
    allWritable = false;
  }

  if (warnUser && !allWritable)
  {
    // Note: We don't ask the user if we should not ask this question again because we can't save the answer.
    errorMsg += i18n("Please contact your system administrator.");
    TQString cmdToExec = TDEStandardDirs::findExe(TQString("kdialog"));
    TDEApplication *app = kapp;
    if (!cmdToExec.isEmpty() && app)
    {
      TDEProcess lprocess;
      lprocess << cmdToExec << "--title" << app->instanceName() << "--msgbox" << TQCString(errorMsg.local8Bit());
      lprocess.start( TDEProcess::Block );
    }
  }
  return allWritable;
}

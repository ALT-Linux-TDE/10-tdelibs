/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
   Copyright (C) 1999 Stephan Kulow <coolo@kde.org>
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

/*
 * Author: Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
 * Version:	$Id$
 * Generated:	Thu Mar  5 16:05:28 EST 1998
 */

#include "config.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include <tqregexp.h>
#include <tqasciidict.h>
#include <tqdict.h>
#include <tqdir.h>
#include <tqfileinfo.h>
#include <tqstring.h>
#include <tqstringlist.h>

#include "kstandarddirs.h"
#include "tdeconfig.h"
#include "kinstance.h"
#include "kshell.h"
#include "ksimpleconfig.h"
#include "kuser.h"
#include "kstaticdeleter.h"
#include <kde_file.h>

template class TQDict<TQStringList>;

class TDEStandardDirs::TDEStandardDirsPrivate
{
public:
   TDEStandardDirsPrivate()
    : restrictionsActive(false),
      dataRestrictionActive(false),
      checkRestrictions(true)
   { }

   bool restrictionsActive;
   bool dataRestrictionActive;
   bool checkRestrictions;
   TQAsciiDict<bool> restrictions;
   TQStringList xdgdata_prefixes;
   TQStringList xdgconf_prefixes;
};

// Singleton, with data shared by all kstandarddirs instances.
// Used in static methods like findExe()
class TDEStandardDirsSingleton
{
public:
   TQString defaultprefix;
   TQString defaultbindir;
   static TDEStandardDirsSingleton* self();
private:
   static TDEStandardDirsSingleton* s_self;
};
static KStaticDeleter<TDEStandardDirsSingleton> kstds_sd;
TDEStandardDirsSingleton* TDEStandardDirsSingleton::s_self = 0;
TDEStandardDirsSingleton* TDEStandardDirsSingleton::self() {
    if ( !s_self )
        kstds_sd.setObject( s_self, new TDEStandardDirsSingleton );
    return s_self;
}

static const char* const types[] = {"html", "html-bundle", "icon", "apps", "sound",
			      "data", "locale", "locale-bundle", "services", "mime",
			      "servicetypes", "config", "exe",
			      "wallpaper", "lib", "pixmap", "templates",
			      "module", "tqtplugins",
			      "xdgdata-apps", "xdgdata-dirs", "xdgconf-menu",
			      "xdgdata-icon", "xdgdata-pixmap", "xdgconf-autostart",
			      "kcfg", "emoticons", 0 };

static int tokenize( TQStringList& token, const TQString& str,
		const TQString& delim );

TDEStandardDirs::TDEStandardDirs( ) : addedCustoms(false)
{
    d = new TDEStandardDirsPrivate;
    dircache.setAutoDelete(true);
    relatives.setAutoDelete(true);
    absolutes.setAutoDelete(true);
    savelocations.setAutoDelete(true);
    addKDEDefaults();
}

TDEStandardDirs::~TDEStandardDirs()
{
    delete d;
}

bool TDEStandardDirs::isRestrictedResource(const char *type, const TQString& relPath) const
{
   if (!d || !d->restrictionsActive)
      return false;

   if (d->restrictions[type])
      return true;

   if (strcmp(type, "data")==0)
   {
      applyDataRestrictions(relPath);
      if (d->dataRestrictionActive)
      {
         d->dataRestrictionActive = false;
         return true;
      }
   }
   return false;
}

void TDEStandardDirs::applyDataRestrictions(const TQString &relPath) const
{
   TQString key;
   int i = relPath.find(TQChar('/'));
   if (i != -1)
      key = "data_"+relPath.left(i);
   else
      key = "data_"+relPath;

   if (d && d->restrictions[key.latin1()])
      d->dataRestrictionActive = true;
}


TQStringList TDEStandardDirs::allTypes() const
{
    TQStringList list;
    for (int i = 0; types[i] != 0; ++i)
        list.append(TQString::fromLatin1(types[i]));
    return list;
}

static void priorityAdd(TQStringList &prefixes, const TQString& dir, bool priority)
{
    if (priority && !prefixes.isEmpty())
    {
        // Add in front but behind $TDEHOME
        TQStringList::iterator it = prefixes.begin();
        it++;
        prefixes.insert(it, 1, dir);
    }
    else
    {
        prefixes.append(dir);
    }
}

void TDEStandardDirs::addPrefix( const TQString& _dir )
{
    addPrefix(_dir, false);
}

void TDEStandardDirs::addPrefix( const TQString& _dir, bool priority )
{
    if (_dir.isEmpty())
	return;

    TQString dir = _dir;
    if (dir.at(dir.length() - 1) != TQChar('/'))
	dir += TQChar('/');

    if (!prefixes.contains(dir)) {
        priorityAdd(prefixes, dir, priority);
	dircache.clear();
    }
}

void TDEStandardDirs::addXdgConfigPrefix( const TQString& _dir )
{
    addXdgConfigPrefix(_dir, false);
}

void TDEStandardDirs::addXdgConfigPrefix( const TQString& _dir, bool priority )
{
    if (_dir.isEmpty())
	return;

    TQString dir = _dir;
    if (dir.at(dir.length() - 1) != TQChar('/'))
	dir += TQChar('/');

    if (!d->xdgconf_prefixes.contains(dir)) {
        priorityAdd(d->xdgconf_prefixes, dir, priority);
	dircache.clear();
    }
}

void TDEStandardDirs::addXdgDataPrefix( const TQString& _dir )
{
    addXdgDataPrefix(_dir, false);
}

void TDEStandardDirs::addXdgDataPrefix( const TQString& _dir, bool priority )
{
    if (_dir.isEmpty())
	return;

    TQString dir = _dir;
    if (dir.at(dir.length() - 1) != TQChar('/'))
	dir += TQChar('/');

    if (!d->xdgdata_prefixes.contains(dir)) {
	priorityAdd(d->xdgdata_prefixes, dir, priority);
	dircache.clear();
    }
}

TQString TDEStandardDirs::kfsstnd_prefixes()
{
   return prefixes.join(TQChar(KPATH_SEPARATOR));
}

TQString TDEStandardDirs::kfsstnd_xdg_conf_prefixes()
{
   return d->xdgconf_prefixes.join(TQChar(KPATH_SEPARATOR));
}

TQString TDEStandardDirs::kfsstnd_xdg_data_prefixes()
{
   return d->xdgdata_prefixes.join(TQChar(KPATH_SEPARATOR));
}

bool TDEStandardDirs::addResourceType( const char *type,
				     const TQString& relativename )
{
    return addResourceType(type, relativename, true);
}
bool TDEStandardDirs::addResourceType( const char *type,
				     const TQString& relativename,
				     bool priority )
{
    if (relativename.isEmpty())
       return false;

    TQStringList *rels = relatives.find(type);
    if (!rels) {
	rels = new TQStringList();
	relatives.insert(type, rels);
    }
    TQString copy = relativename;
    if (copy.at(copy.length() - 1) != TQChar('/'))
	copy += TQChar('/');
    if (!rels->contains(copy)) {
        if (priority)
	    rels->prepend(copy);
	else
	    rels->append(copy);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

bool TDEStandardDirs::addResourceDir( const char *type,
				    const TQString& absdir)
{
    // KDE4: change priority to bring in line with addResourceType
    return addResourceDir(type, absdir, false);
}

bool TDEStandardDirs::addResourceDir( const char *type,
				    const TQString& absdir,
				    bool priority)
{
    TQStringList *paths = absolutes.find(type);
    if (!paths) {
	paths = new TQStringList();
	absolutes.insert(type, paths);
    }
    TQString copy = absdir;
    if (copy.at(copy.length() - 1) != TQChar('/'))
      copy += TQChar('/');

    if (!paths->contains(copy)) {
        if (priority)
            paths->prepend(copy);
        else
	    paths->append(copy);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

TQString TDEStandardDirs::findResource( const char *type,
				     const TQString& filename ) const
{
	if (!TQDir::isRelativePath(filename))
	return filename; // absolute dirs are absolute dirs, right? :-/

#if 0
kdDebug() << "Find resource: " << type << endl;
for (TQStringList::ConstIterator pit = prefixes.begin();
     pit != prefixes.end();
     pit++)
{
  kdDebug() << "Prefix: " << *pit << endl;
}
#endif

    TQString dir = findResourceDir(type, filename);
    if (dir.isEmpty())
	return dir;
    else return dir + filename;
}

static TQ_UINT32 updateHash(const TQString &file, TQ_UINT32 hash)
{
    TQCString cFile = TQFile::encodeName(file);
    KDE_struct_stat buff;
    if ((access(cFile, R_OK) == 0) &&
        (KDE_stat( cFile, &buff ) == 0) &&
        (S_ISREG( buff.st_mode )))
    {
       hash = hash + (TQ_UINT32) buff.st_ctime;
    }
    return hash;
}

TQ_UINT32 TDEStandardDirs::calcResourceHash( const char *type,
			      const TQString& filename, bool deep) const
{
    TQ_UINT32 hash = 0;

    if (!TQDir::isRelativePath(filename))
    {
        // absolute dirs are absolute dirs, right? :-/
	return updateHash(filename, hash);
    }
    if (d && d->restrictionsActive && (strcmp(type, "data")==0))
       applyDataRestrictions(filename);
    TQStringList candidates = resourceDirs(type);
    TQString fullPath;

    for (TQStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); ++it)
    {
        hash = updateHash(*it + filename, hash);
        if (!deep && hash)
           return hash;
    }
    return hash;
}


TQStringList TDEStandardDirs::findDirs( const char *type,
                                     const TQString& reldir ) const
{
    TQDir testdir;
    TQStringList list;
    if (!TQDir::isRelativePath(reldir))
    {
        testdir.setPath(reldir);
        if (testdir.exists())
        {
            if (reldir.endsWith("/"))
               list.append(reldir);
            else
               list.append(reldir+TQChar('/'));
        }
        return list;
    }

    checkConfig();

    if (d && d->restrictionsActive && (strcmp(type, "data")==0))
       applyDataRestrictions(reldir);
    TQStringList candidates = resourceDirs(type);

    for (TQStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); ++it) {
        testdir.setPath(*it + reldir);
        if (testdir.exists())
            list.append(testdir.absPath() + TQChar('/'));
    }

    return list;
}

TQString TDEStandardDirs::findResourceDir( const char *type,
					const TQString& filename) const
{
#ifndef NDEBUG
    if (filename.isEmpty()) {
      fprintf(stderr, "filename for type %s in TDEStandardDirs::findResourceDir is not supposed to be empty!!", type);
      return TQString::null;
    }
#endif

    if (d && d->restrictionsActive && (strcmp(type, "data")==0))
       applyDataRestrictions(filename);
    TQStringList candidates = resourceDirs(type);
    TQString fullPath;

    for (TQStringList::ConstIterator it = candidates.begin();
      it != candidates.end(); ++it) {
      if (exists(*it + filename)) {
#ifdef TQ_WS_WIN //this ensures we're using installed .la files
          if ((*it).isEmpty() && filename.right(3)==".la") {
#ifndef NDEBUG
              fprintf(stderr, "TDEStandardDirs::findResourceDir() found .la in cwd: skipping. (fname=%s)\n", filename.ascii());
#endif
              continue;
          }
#endif //TQ_WS_WIN
          return *it;
      }
    }

#ifndef NDEBUG
    if(false && strcmp(type, "locale"))
      fprintf(stderr, "KStdDirs::findResDir(): can't find \"%s\" in type \"%s\".\n", filename.ascii(), type);
#endif

    return TQString::null;
}

bool TDEStandardDirs::exists(const TQString &fullPath)
{
    KDE_struct_stat buff;
    if ((access(TQFile::encodeName(fullPath), R_OK) == 0) && (KDE_stat( TQFile::encodeName(fullPath), &buff ) == 0)) {
	if (fullPath.at(fullPath.length() - 1) != TQChar('/')) {
	    if (S_ISREG( buff.st_mode ))
		return true;
	}
	else {
	    if (S_ISDIR( buff.st_mode )) {
		return true;
	    }
	}
    }
    return false;
}

static void lookupDirectory(const TQString& path, const TQString &relPart,
			    const TQRegExp &regexp,
			    TQStringList& list,
			    TQStringList& relList,
			    bool recursive, bool unique)
{
  TQString pattern = regexp.pattern();
  if (recursive || pattern.contains('?') || pattern.contains('*'))
  {
    if (path.isEmpty()) //for sanity
      return;
    // We look for a set of files.
    DIR *dp = opendir( TQFile::encodeName(path));
    if (!dp)
      return;

#ifdef TQ_WS_WIN
    assert(path.at(path.length() - 1) == TQChar('/') || path.at(path.length() - 1) == TQChar('\\'));
#else
    assert(path.at(path.length() - 1) == TQChar('/'));
#endif

    struct dirent *ep;
    KDE_struct_stat buff;

    TQString _dot(".");
    TQString _dotdot("..");

    while( ( ep = readdir( dp ) ) != 0L )
    {
      TQString fn( TQFile::decodeName(ep->d_name));
      if (fn == _dot || fn == _dotdot || TQChar(fn.at(fn.length() - 1)).latin1() == TQChar('~').latin1())
	continue;

      if (!recursive && !regexp.exactMatch(fn))
	continue; // No match

      TQString pathfn = path + fn;
      if ( KDE_stat( TQFile::encodeName(pathfn), &buff ) != 0 ) {
	fprintf(stderr, "Error stat'ing %s : %d\n", pathfn.ascii(), errno);
	continue; // Couldn't stat (e.g. no read permissions)
      }
      if ( recursive ) {
	if ( S_ISDIR( buff.st_mode )) {
	  lookupDirectory(pathfn + TQChar('/'), relPart + fn + TQChar('/'), regexp, list, relList, recursive, unique);
	}
        if (!regexp.exactMatch(fn))
	  continue; // No match
      }
      if ( S_ISREG( buff.st_mode))
      {
        if (!unique || !relList.contains(relPart + fn))
        {
	    list.append( pathfn );
	    relList.append( relPart + fn );
        }
      }
    }
    closedir( dp );
  }
  else
  {
     // We look for a single file.
     TQString fn = pattern;
     TQString pathfn = path + fn;
     KDE_struct_stat buff;
     if ( KDE_stat( TQFile::encodeName(pathfn), &buff ) != 0 )
        return; // File not found
     if ( S_ISREG( buff.st_mode))
     {
       if (!unique || !relList.contains(relPart + fn))
       {
         list.append( pathfn );
         relList.append( relPart + fn );
       }
     }
  }
}

static void lookupPrefix(const TQString& prefix, const TQString& relpath,
                         const TQString& relPart,
			 const TQRegExp &regexp,
			 TQStringList& list,
			 TQStringList& relList,
			 bool recursive, bool unique)
{
    if (relpath.isEmpty()) {
       lookupDirectory(prefix, relPart, regexp, list,
		       relList, recursive, unique);
       return;
    }
    TQString path;
    TQString rest;

    if (relpath.length())
    {
       int slash = relpath.find(TQChar('/'));
       if (slash < 0)
	   rest = relpath.left(relpath.length() - 1);
       else {
	   path = relpath.left(slash);
	   rest = relpath.mid(slash + 1);
       }
    }

    if (prefix.isEmpty()) //for sanity
      return;
#ifdef TQ_WS_WIN
    assert(prefix.at(prefix.length() - 1) == TQChar('/') || prefix.at(prefix.length() - 1) == TQChar('\\'));
#else
    assert(prefix.at(prefix.length() - 1) == TQChar('/'));
#endif
    KDE_struct_stat buff;

    if (path.contains('*') || path.contains('?')) {

	TQRegExp pathExp(path, true, true);
	DIR *dp = opendir( TQFile::encodeName(prefix) );
	if (!dp) {
	    return;
	}

	struct dirent *ep;

        TQString _dot(".");
        TQString _dotdot("..");

	while( ( ep = readdir( dp ) ) != 0L )
	    {
		TQString fn( TQFile::decodeName(ep->d_name));
		if (fn == _dot || fn == _dotdot || fn.at(fn.length() - 1) == TQChar('~'))
		    continue;

		if ( !pathExp.exactMatch(fn) )
		    continue; // No match
		TQString rfn = relPart+fn;
		fn = prefix + fn;
		if ( KDE_stat( TQFile::encodeName(fn), &buff ) != 0 ) {
		    fprintf(stderr, "Error statting %s : %d\n", fn.ascii(), errno);
		    continue; // Couldn't stat (e.g. no permissions)
		}
		if ( S_ISDIR( buff.st_mode ))
		    lookupPrefix(fn + TQChar('/'), rest, rfn + TQChar('/'), regexp, list, relList, recursive, unique);
	    }

	closedir( dp );
    } else {
        // Don't stat, if the dir doesn't exist we will find out
        // when we try to open it.
        lookupPrefix(prefix + path + TQChar('/'), rest,
                     relPart + path + TQChar('/'), regexp, list,
                     relList, recursive, unique);
    }
}

TQStringList
TDEStandardDirs::findAllResources( const char *type,
			         const TQString& filter,
				 bool recursive,
			         bool unique,
                                 TQStringList &relList) const
{
    TQStringList list;
    TQString filterPath;
    TQString filterFile;

    if (filter.length())
    {
       int slash = filter.findRev('/');
       if (slash < 0)
	   filterFile = filter;
       else {
	   filterPath = filter.left(slash + 1);
	   filterFile = filter.mid(slash + 1);
       }
    }

    checkConfig();

    TQStringList candidates;
	if (!TQDir::isRelativePath(filter)) // absolute path
    {
#ifdef Q_OS_WIN
        candidates << filterPath.left(3); //e.g. "C:\"
        filterPath = filterPath.mid(3);
#else
        candidates << "/";
        filterPath = filterPath.mid(1);
#endif
    }
    else
    {
        if (d && d->restrictionsActive && (strcmp(type, "data")==0))
            applyDataRestrictions(filter);
        candidates = resourceDirs(type);
    }
    if (filterFile.isEmpty())
	filterFile = "*";

    TQRegExp regExp(filterFile, true, true);

    for (TQStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); ++it)
    {
        lookupPrefix(*it, filterPath, "", regExp, list,
                     relList, recursive, unique);
    }

    return list;
}

TQStringList
TDEStandardDirs::findAllResources( const char *type,
			         const TQString& filter,
				 bool recursive,
			         bool unique) const
{
    TQStringList relList;
    return findAllResources(type, filter, recursive, unique, relList);
}

TQString
TDEStandardDirs::realPath(const TQString &dirname)
{
    char realpath_buffer[MAXPATHLEN + 1];
    memset(realpath_buffer, 0, MAXPATHLEN + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( TQFile::encodeName(dirname).data(), realpath_buffer) != 0) {
        // success, use result from realpath
        int len = strlen(realpath_buffer);
        realpath_buffer[len] = TQChar('/');
        realpath_buffer[len+1] = 0;
        return TQFile::decodeName(realpath_buffer);
    }

    return dirname;
}

TQString
TDEStandardDirs::realFilePath(const TQString &filename)
{
    char realpath_buffer[MAXPATHLEN + 1];
    memset(realpath_buffer, 0, MAXPATHLEN + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( TQFile::encodeName(filename).data(), realpath_buffer) != 0) {
        // success, use result from realpath
        return TQFile::decodeName(realpath_buffer);
    }

    return filename;
}

void TDEStandardDirs::createSpecialResource(const char *type)
{
   char hostname[256];
   hostname[0] = 0;
   if( getenv("XAUTHLOCALHOSTNAME"))
       strlcpy(hostname, getenv("XAUTHLOCALHOSTNAME"), 255 );
   else
       gethostname(hostname, 255);
   TQString dir = TQString("%1%2-%3").arg(localtdedir()).arg(type).arg(hostname);
   char link[1024];
   link[1023] = 0;
   int result = readlink(TQFile::encodeName(dir).data(), link, 1023);
   bool relink = (result == -1) && (errno == ENOENT);
   if (result > 0)
   {
      link[result] = 0;
      if (!TQDir::isRelativePath(link))
      {
         KDE_struct_stat stat_buf;
         int res = KDE_lstat(link, &stat_buf);
         if ((res == -1) && (errno == ENOENT))
         {
            relink = true;
         }
         else if ((res == -1) || (!S_ISDIR(stat_buf.st_mode)))
         {
            fprintf(stderr, "Error: \"%s\" is not a directory.\n", link);
            relink = true;
         }
         else if (stat_buf.st_uid != getuid())
         {
            fprintf(stderr, "Error: \"%s\" is owned by uid %d instead of uid %d.\n", link, stat_buf.st_uid, getuid());
            relink = true;
         }
      }
   }
#ifdef TQ_WS_WIN
   if (relink)
   {
      if (!makeDir(dir, 0700))
         fprintf(stderr, "failed to create \"%s\"", dir.latin1());
      else
         result = readlink(TQFile::encodeName(dir).data(), link, 1023);
   }
#else //UNIX
   if (relink)
   {
      TQString srv = findExe(TQString::fromLatin1("lnusertemp"), kfsstnd_defaultbindir());
      if (srv.isEmpty())
         srv = findExe(TQString::fromLatin1("lnusertemp"));
      if (!srv.isEmpty())
      {
         if (system(TQFile::encodeName(srv)+" "+type) < 0 ) {
             result = readlink(TQFile::encodeName(dir).data(), link, 1023);
         }
         else {
             result = -1;
         }
      }
   }
   if (result > 0)
   {
      link[result] = 0;
      if (link[0] == TQChar('/').latin1()) {
         dir = TQFile::decodeName(link);
      }
      else {
         dir = TQDir::cleanDirPath(dir+TQFile::decodeName(link));
      }
   }
#endif
   addResourceDir(type, dir+TQChar('/'));
}

TQStringList TDEStandardDirs::resourceDirs(const char *type) const
{
    TQStringList *candidates = dircache.find(type);

    if (!candidates) { // filling cache
        if (strcmp(type, "socket") == 0)
           const_cast<TDEStandardDirs *>(this)->createSpecialResource(type);
        else if (strcmp(type, "tmp") == 0)
           const_cast<TDEStandardDirs *>(this)->createSpecialResource(type);
        else if (strcmp(type, "cache") == 0)
           const_cast<TDEStandardDirs *>(this)->createSpecialResource(type);

        TQDir testdir;

        candidates = new TQStringList();
        TQStringList *dirs;

        bool restrictionActive = false;
        if (d && d->restrictionsActive)
        {
           if (d->dataRestrictionActive)
              restrictionActive = true;
           else if (d->restrictions["all"])
              restrictionActive = true;
           else if (d->restrictions[type])
              restrictionActive = true;
           d->dataRestrictionActive = false; // Reset
        }

        dirs = relatives.find(type);
        if (dirs)
        {
            bool local = true;
            const TQStringList *prefixList = 0;
            if (strncmp(type, "xdgdata-", 8) == 0)
                prefixList = &(d->xdgdata_prefixes);
            else if (strncmp(type, "xdgconf-", 8) == 0)
                prefixList = &(d->xdgconf_prefixes);
            else
                prefixList = &prefixes;

            for (TQStringList::ConstIterator pit = prefixList->begin();
                 pit != prefixList->end();
                 ++pit)
            {
                for (TQStringList::ConstIterator it = dirs->begin();
                     it != dirs->end(); ++it) {
                    TQString path = realPath(*pit + *it);
                    testdir.setPath(path);
                    if (local && restrictionActive)
                       continue;
                    if ((local || testdir.exists()) && !candidates->contains(path))
                        candidates->append(path);
                }
		// UGLY HACK - Chris CHeney
		if (local && (!strcmp("config", type)))
		  candidates->append("/etc/trinity/");
		//
                local = false;
            }
        }
        dirs = absolutes.find(type);
        if (dirs)
            for (TQStringList::ConstIterator it = dirs->begin();
                 it != dirs->end(); ++it)
            {
                testdir.setPath(*it);
                if (testdir.exists())
                {
                    TQString filename = realPath(*it);
                    if (!candidates->contains(filename))
                        candidates->append(filename);
                }
            }
        dircache.insert(type, candidates);
    }

#if 0
    kdDebug() << "found dirs for resource " << type << ":" << endl;
    for (TQStringList::ConstIterator pit = candidates->begin();
	 pit != candidates->end();
	 pit++)
    {
	fprintf(stderr, "%s\n", (*pit).latin1());
    }
#endif


  return *candidates;
}

TQStringList TDEStandardDirs::systemPaths( const TQString& pstr )
{
    TQStringList tokens;
    TQString p = pstr;

    if( p.isNull() )
    {
	p = getenv( "PATH" );
    }

    TQString delimiters(TQChar(KPATH_SEPARATOR));
    delimiters += "\b";
    tokenize( tokens, p, delimiters );

    TQStringList exePaths;

    // split path using : or \b as delimiters
    for( unsigned i = 0; i < tokens.count(); i++ )
    {
	p = tokens[ i ];

        if ( p[ 0 ] == TQChar('~') )
        {
            int len = p.find( TQChar('/') );
            if ( len == -1 )
                len = p.length();
            if ( len == 1 )
            {
                p.replace( 0, 1, TQDir::homeDirPath() );
            }
            else
            {
                TQString user = p.mid( 1, len - 1 );
                struct passwd *dir = getpwnam( user.local8Bit().data() );
                if ( dir && strlen( dir->pw_dir ) )
                    p.replace( 0, len, TQString::fromLocal8Bit( dir->pw_dir ) );
            }
        }

	exePaths << p;
    }

    return exePaths;
}


TQString TDEStandardDirs::findExe( const TQString& appname,
				const TQString& pstr, bool ignore)
{
#ifdef TQ_WS_WIN
    TQString real_appname = appname + ".exe";
#else
    TQString real_appname = appname;
#endif
    TQFileInfo info;

    // absolute or relative path given
    if (real_appname.find(TQDir::separator()) >= 0)
    {
        info.setFile( real_appname );
        if( info.exists() && ( ignore || info.isExecutable() )
            && info.isFile() ) {
            return info.absFilePath();
        }
        return TQString::null;
    }

    TQString p = TQString("%1/%2").arg(kfsstnd_defaultbindir()).arg(real_appname);
    info.setFile( p );
    if( info.exists() && ( ignore || info.isExecutable() )
         && ( info.isFile() || info.isSymLink() )  ) {
         return p;
    }

    TQStringList exePaths = systemPaths( pstr );
    for (TQStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
	p = (*it) + "/";
	p += real_appname;

	// Check for executable in this tokenized path
	info.setFile( p );

	if( info.exists() && ( ignore || info.isExecutable() )
           && ( info.isFile() || info.isSymLink() )  ) {
	    return p;
	}
    }

    // If we reach here, the executable wasn't found.
    // So return empty string.

    return TQString::null;
}

int TDEStandardDirs::findAllExe( TQStringList& list, const TQString& appname,
			const TQString& pstr, bool ignore )
{
#ifdef TQ_WS_WIN
    TQString real_appname = appname + ".exe";
#else
    TQString real_appname = appname;
#endif
    TQFileInfo info;
    TQString p;
    list.clear();

    TQStringList exePaths = systemPaths( pstr );
    for (TQStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
	p = (*it) + "/";
	p += real_appname;

	info.setFile( p );

	if( info.exists() && (ignore || info.isExecutable())
	    && info.isFile() ) {
	    list.append( p );
	}
    }

    return list.count();
}

static int tokenize( TQStringList& tokens, const TQString& str,
		     const TQString& delim )
{
    int len = str.length();
    TQString token = "";

    for( int index = 0; index < len; index++)
    {
	if ( delim.find( str[ index ] ) >= 0 )
	{
	    tokens.append( token );
	    token = "";
	}
	else
	{
	    token += str[ index ];
	}
    }
    if ( token.length() > 0 )
    {
	tokens.append( token );
    }

    return tokens.count();
}

TQString TDEStandardDirs::kde_default(const char *type) {
    if (!strcmp(type, "data"))
	return "share/apps/";
    if (!strcmp(type, "html-bundle"))
	return "share/doc-bundle/HTML/";
    if (!strcmp(type, "html"))
	return "share/doc/tde/HTML/";
    if (!strcmp(type, "icon"))
	return "share/icons/";
    if (!strcmp(type, "config"))
	return "share/config/";
    if (!strcmp(type, "pixmap"))
	return "share/pixmaps/";
    if (!strcmp(type, "apps"))
	return "share/applnk/";
    if (!strcmp(type, "sound"))
	return "share/sounds/";
    if (!strcmp(type, "locale-bundle"))
	return "share/locale-bundle/";
    if (!strcmp(type, "locale"))
	return "share/locale/";
    if (!strcmp(type, "services"))
	return "share/services/";
    if (!strcmp(type, "servicetypes"))
	return "share/servicetypes/";
    if (!strcmp(type, "mime"))
	return "share/mimelnk/";
    if (!strcmp(type, "cgi"))
	return "lib/cgi-bin/";
    if (!strcmp(type, "wallpaper"))
	return "share/wallpapers/";
    if (!strcmp(type, "templates"))
	return "share/templates/";
    if (!strcmp(type, "exe"))
	return "bin/";
    if (!strcmp(type, "lib"))
	return "lib" KDELIBSUFF "/";
    if (!strcmp(type, "module"))
	return "lib" KDELIBSUFF "/trinity/";
    if (!strcmp(type, "tqtplugins"))
        return "lib" KDELIBSUFF "/trinity/plugins";
    if (!strcmp(type, "xdgdata-apps"))
        return "applications/";
    if (!strcmp(type, "xdgdata-icon"))
        return "icons/";
    if (!strcmp(type, "xdgdata-pixmap"))
        return "pixmaps/";
    if (!strcmp(type, "xdgdata-dirs"))
        return "desktop-directories/";
    if (!strcmp(type, "xdgconf-menu"))
        return "menus/";
    if (!strcmp(type, "xdgconf-autostart"))
        return "autostart/";
    if (!strcmp(type, "kcfg"))
	return "share/config.kcfg";
    if (!strcmp(type, "emoticons"))
			return "share/emoticons";


    tqFatal("unknown resource type %s", type);
    return TQString::null;
}

TQString TDEStandardDirs::saveLocation(const char *type,
				    const TQString& suffix,
				    bool create) const
{
    checkConfig();

    TQString *pPath = savelocations.find(type);
    if (!pPath)
    {
       TQStringList *dirs = relatives.find(type);
       if (!dirs && (
                     (strcmp(type, "socket") == 0) ||
                     (strcmp(type, "tmp") == 0) ||
                     (strcmp(type, "cache") == 0) ))
       {
          (void) resourceDirs(type); // Generate socket|tmp|cache resource.
          dirs = relatives.find(type); // Search again.
       }
       if (dirs)
       {
          // Check for existence of typed directory + suffix
          if (strncmp(type, "xdgdata-", 8) == 0)
             pPath = new TQString(realPath(localxdgdatadir() + dirs->last()));
          else if (strncmp(type, "xdgconf-", 8) == 0)
             pPath = new TQString(realPath(localxdgconfdir() + dirs->last()));
          else
             pPath = new TQString(realPath(localtdedir() + dirs->last()));
       }
       else {
          dirs = absolutes.find(type);
          if (!dirs)
             tqFatal("TDEStandardDirs: The resource type %s is not registered", type);
          pPath = new TQString(realPath(dirs->last()));
       }

       savelocations.insert(type, pPath);
    }
    TQString fullPath = *pPath + (pPath->endsWith("/") ? "" : "/") + suffix;

    KDE_struct_stat st;
    if (KDE_stat(TQFile::encodeName(fullPath), &st) != 0 || !(S_ISDIR(st.st_mode))) {
	if(!create) {
	    return fullPath;
	}
	if(!makeDir(fullPath, 0700)) {
	    return fullPath;
	}
        dircache.remove(type);
    }
    if (!fullPath.endsWith("/")) {
	    fullPath += "/";
    }
    return fullPath;
}

TQString TDEStandardDirs::relativeLocation(const char *type, const TQString &absPath)
{
    TQString fullPath = absPath;
    int i = absPath.findRev('/');
    if (i != -1)
    {
       fullPath = realPath(absPath.left(i+1))+absPath.mid(i+1); // Normalize
    }

    TQStringList candidates = resourceDirs(type);

    for (TQStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); ++it)
      if (fullPath.startsWith(*it))
      {
	return fullPath.mid((*it).length());
      }

    return absPath;
}


bool TDEStandardDirs::makeDir(const TQString& dir, int mode)
{
    // we want an absolute path
    if (TQDir::isRelativePath(dir))
        return false;

    TQString target = dir;
    uint len = target.length();

    // append trailing slash if missing
    if (dir.at(len - 1) != TQChar('/'))
        target += TQChar('/');

    TQString base("");
    uint i = 1;

    while( i < len )
    {
        KDE_struct_stat st;
        int pos = target.find(TQChar('/'), i);
        base += target.mid(i - 1, pos - i + 1);
        TQCString baseEncoded = TQFile::encodeName(base);
        // bail out if we encountered a problem
        if (KDE_stat(baseEncoded, &st) != 0)
        {
          // Directory does not exist....
          // Or maybe a dangling symlink ?
          if (KDE_lstat(baseEncoded, &st) == 0)
              (void)unlink(baseEncoded); // try removing

	  if ( KDE_mkdir(baseEncoded, (mode_t) mode) != 0) {
            baseEncoded.prepend( "trying to create local folder " );
	    perror(baseEncoded.data());
	    return false; // Couldn't create it :-(
	  }
        }
        i = pos + 1;
    }
    return true;
}

static TQString readEnvPath(const char *env)
{
   TQCString c_path = getenv(env);
   if (c_path.isEmpty())
      return TQString::null;
#ifdef Q_OS_WIN
   //win32 paths are case-insensitive: avoid duplicates on various dir lists
   return TQFile::decodeName(c_path).lower();
#else
   return TQFile::decodeName(c_path);
#endif
}

#ifdef __linux__
static TQString executablePrefix()
{
   char path_buffer[MAXPATHLEN + 1];
   path_buffer[MAXPATHLEN] = 0;
   int length = readlink ("/proc/self/exe", path_buffer, MAXPATHLEN);
   if (length == -1)
      return TQString::null;

   path_buffer[length] = TQChar('\0');

   TQString path = TQFile::decodeName(path_buffer);

   if(path.isEmpty())
      return TQString::null;

   int pos = path.findRev('/'); // Skip filename
   if(pos <= 0)
      return TQString::null;
   pos = path.findRev(TQChar('/'), pos - 1); // Skip last directory
   if(pos <= 0)
      return TQString::null;

   return path.left(pos);
}
#endif

TQString TDEStandardDirs::kfsstnd_defaultprefix()
{
   TDEStandardDirsSingleton* s = TDEStandardDirsSingleton::self();
   if (!s->defaultprefix.isEmpty())
      return s->defaultprefix;
#ifdef TQ_WS_WIN
   s->defaultprefix = readEnvPath("TDEDIR");
   if (s->defaultprefix.isEmpty()) {
      s->defaultprefix = TQFile::decodeName("c:\\kde");
      //TODO: find other location (the Registry?)
   }
#else //UNIX
   s->defaultprefix = TDEDIR;
#endif
   if (s->defaultprefix.isEmpty()) {
      fprintf(stderr, "TDEStandardDirs::kfsstnd_defaultprefix(): default TDE prefix not found!\n");
   }
   return s->defaultprefix;
}

TQString TDEStandardDirs::kfsstnd_defaultbindir()
{
   TDEStandardDirsSingleton* s = TDEStandardDirsSingleton::self();
   if (!s->defaultbindir.isEmpty())
      return s->defaultbindir;
#ifdef TQ_WS_WIN
   s->defaultbindir = kfsstnd_defaultprefix() + TQString::fromLatin1("/bin");
#else //UNIX
   s->defaultbindir = __TDE_BINDIR;
   if (s->defaultbindir.isEmpty())
      s->defaultbindir = kfsstnd_defaultprefix() + TQString::fromLatin1("/bin");
#endif
   if (s->defaultbindir.isEmpty()) {
      fprintf(stderr, "TDEStandardDirs::kfsstnd_defaultbindir(): default binary TDE dir not found!\n");
   }
  return s->defaultbindir;
}

void TDEStandardDirs::addKDEDefaults()
{
    TQStringList tdedirList;

    // begin TDEDIRS
    TQString tdedirs = readEnvPath("TDEDIRS");
    if (!tdedirs.isEmpty())
    {
        tokenize(tdedirList, tdedirs, TQChar(KPATH_SEPARATOR));
    }
    else
    {
        TQString tdedir = readEnvPath("TDEDIR");
        if (!tdedir.isEmpty())
        {
           tdedir = KShell::tildeExpand(tdedir);
           tdedirList.append(tdedir);
        }
    }

#ifndef Q_OS_WIN //no default TDEDIR on win32 defined
    tdedirList.append(TDEDIR);
#endif

#ifdef __KDE_EXECPREFIX
    TQString execPrefix(__KDE_EXECPREFIX);
    if (execPrefix!="NONE")
       tdedirList.append(execPrefix);
#endif
#ifdef __linux__
    const TQString linuxExecPrefix = executablePrefix();
    if ( !linuxExecPrefix.isEmpty() )
       tdedirList.append( linuxExecPrefix );
#endif

    // We treat root differently to prevent a "su" shell messing up the
    // file permissions in the user's home directory.
    TQString localKdeDir;
    if (getuid() == 0) {
      localKdeDir = readEnvPath("TDEROOTHOME");
      if (localKdeDir.isEmpty() == true)
        localKdeDir = readEnvPath("TDEHOME");
    }
    else {
      localKdeDir = readEnvPath("TDEHOME");
    }
    if (!localKdeDir.isEmpty())
    {
       if (localKdeDir[localKdeDir.length()-1] != TQChar('/'))
          localKdeDir += TQChar('/');
    }
    else
    {
       localKdeDir =  TQDir::homeDirPath() + "/.trinity/";
    }

    if (localKdeDir != TQString("-/"))
    {
        localKdeDir = KShell::tildeExpand(localKdeDir);
        addPrefix(localKdeDir);
    }

	TQStringList::ConstIterator end(tdedirList.end());
    for (TQStringList::ConstIterator it = tdedirList.begin();
	 it != end; ++it)
    {
        TQString dir = KShell::tildeExpand(*it);
	addPrefix(dir);
    }
    // end TDEDIRS

    // begin XDG_CONFIG_XXX
    TQStringList xdgdirList;
    TQString xdgdirs = readEnvPath("XDG_CONFIG_DIRS");
    if (!xdgdirs.isEmpty())
    {
	tokenize(xdgdirList, xdgdirs, TQChar(KPATH_SEPARATOR));
    }
    else
    {
	xdgdirList.clear();
        xdgdirList.append("/etc/xdg");
#ifdef TQ_WS_WIN
        xdgdirList.append(kfsstnd_defaultprefix() + "/etc/xdg");
#else
        xdgdirList.append(KDESYSCONFDIR "/xdg");
#endif
    }

    TQString localXdgDir = readEnvPath("XDG_CONFIG_HOME");
    if (!localXdgDir.isEmpty())
    {
       if (localXdgDir[localXdgDir.length()-1] != TQChar('/'))
          localXdgDir += TQChar('/');
    }
    else
    {
       localXdgDir =  TQDir::homeDirPath() + "/.config/";
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgConfigPrefix(localXdgDir);

    for (TQStringList::ConstIterator it = xdgdirList.begin();
	 it != xdgdirList.end(); ++it)
    {
        TQString dir = KShell::tildeExpand(*it);
	addXdgConfigPrefix(dir);
    }
    // end XDG_CONFIG_XXX

    // begin XDG_DATA_XXX
    xdgdirs = readEnvPath("XDG_DATA_DIRS");
    if (!xdgdirs.isEmpty())
    {
	tokenize(xdgdirList, xdgdirs, TQChar(KPATH_SEPARATOR));
    }
    else
    {
	xdgdirList.clear();
        for (TQStringList::ConstIterator it = tdedirList.begin();
           it != tdedirList.end(); ++it)
        {
           TQString dir = *it;
           if (dir[dir.length()-1] != TQChar('/'))
             dir += TQChar('/');
           xdgdirList.append(dir+"share/");
        }

        xdgdirList.append("/usr/local/share/");
        xdgdirList.append("/usr/share/");
    }

    localXdgDir = readEnvPath("XDG_DATA_HOME");
    if (!localXdgDir.isEmpty())
    {
       if (localXdgDir[localXdgDir.length()-1] != TQChar('/'))
          localXdgDir += TQChar('/');
    }
    else
    {
       localXdgDir = TQDir::homeDirPath() + "/.local/share/";
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgDataPrefix(localXdgDir);

    for (TQStringList::ConstIterator it = xdgdirList.begin();
	 it != xdgdirList.end(); ++it)
    {
        TQString dir = KShell::tildeExpand(*it);
	addXdgDataPrefix(dir);
    }
    // end XDG_DATA_XXX


    uint index = 0;
    while (types[index] != 0) {
	addResourceType(types[index], kde_default(types[index]));
	index++;
    }

    addResourceDir("home", TQDir::homeDirPath());

    addResourceDir("locale", "/usr/share/locale-langpack/", true);
}

void TDEStandardDirs::checkConfig() const
{
    if (!addedCustoms && TDEGlobal::_instance && TDEGlobal::_instance->_config)
        const_cast<TDEStandardDirs*>(this)->addCustomized(TDEGlobal::_instance->_config);
}

static TQStringList lookupProfiles(const TQString &mapFile)
{
    TQStringList profiles;

    if (mapFile.isEmpty() || !TQFile::exists(mapFile))
    {
       profiles << "default";
       return profiles;
    }

    struct passwd *pw = getpwuid(geteuid());
    if (!pw)
    {
        profiles << "default";
        return profiles; // Not good
    }

    TQCString user = pw->pw_name;

    gid_t sup_gids[512];
    int sup_gids_nr = getgroups(512, sup_gids);

    KSimpleConfig mapCfg(mapFile, true);
    mapCfg.setGroup("Users");
    if (mapCfg.hasKey(user.data()))
    {
        profiles = mapCfg.readListEntry(user.data());
        return profiles;
    }

    mapCfg.setGroup("General");
    TQStringList groups = mapCfg.readListEntry("groups");

    mapCfg.setGroup("Groups");

    for( TQStringList::ConstIterator it = groups.begin();
         it != groups.end(); ++it )
    {
        TQCString grp = (*it).utf8();
        // Check if user is in this group
        struct group *grp_ent = getgrnam(grp);
        if (!grp_ent) continue;
        gid_t gid = grp_ent->gr_gid;
        if (pw->pw_gid == gid)
        {
            // User is in this group --> add profiles
            profiles += mapCfg.readListEntry(*it);
        }
        else
        {
            for(int i = 0; i < sup_gids_nr; i++)
            {
                if (sup_gids[i] == gid)
                {
                    // User is in this group --> add profiles
                    profiles += mapCfg.readListEntry(*it);
                    break;
                }
            }
        }
    }

    if (profiles.isEmpty())
        profiles << "default";
    return profiles;
}

extern bool kde_kiosk_admin;

bool TDEStandardDirs::addCustomized(TDEConfig *config)
{
    if (addedCustoms && !d->checkRestrictions) // there are already customized entries
        return false; // we just quit and hope they are the right ones

    // save the numbers of config directories. If this changes,
    // we will return true to give TDEConfig a chance to reparse
    uint configdirs = resourceDirs("config").count();

    // Remember original group
    TQString oldGroup = config->group();

    if (!addedCustoms)
    {
        // We only add custom entries once
        addedCustoms = true;

        // reading the prefixes in
        TQString group = TQString::fromLatin1("Directories");
        config->setGroup(group);

        TQString kioskAdmin = config->readEntry("kioskAdmin");
        if (!kioskAdmin.isEmpty() && !kde_kiosk_admin)
        {
            int i = kioskAdmin.find(':');
            TQString user = kioskAdmin.left(i);
            TQString host = kioskAdmin.mid(i+1);

            KUser thisUser;
            char hostname[ 256 ];
            hostname[ 0 ] = TQChar('\0');
            if (!gethostname( hostname, 255 ))
                hostname[sizeof(hostname)-1] = TQChar('\0');

            if ((user == thisUser.loginName()) &&
                (host.isEmpty() || (host == hostname)))
            {
                kde_kiosk_admin = true;
            }
        }

        bool readProfiles = true;

        if (kde_kiosk_admin && !TQCString(getenv("TDE_KIOSK_NO_PROFILES")).isEmpty())
            readProfiles = false;

        TQString userMapFile = config->readEntry("userProfileMapFile");
        TQString profileDirsPrefix = config->readEntry("profileDirsPrefix");
        if (!profileDirsPrefix.isEmpty() && !profileDirsPrefix.endsWith("/"))
            profileDirsPrefix.append('/');

        TQStringList profiles;
        if (readProfiles)
            profiles = lookupProfiles(userMapFile);
        TQString profile;

        bool priority = false;
        while(true)
        {
            config->setGroup(group);
            TQStringList list = config->readListEntry("prefixes");
            for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
            {
                addPrefix(*it, priority);
                addXdgConfigPrefix(*it+"/etc/xdg", priority);
                addXdgDataPrefix(*it+"/share", priority);
            }
            // If there are no prefixes defined, check if there is a directory
            // for this profile under <profileDirsPrefix>
            if (list.isEmpty() && !profile.isEmpty() && !profileDirsPrefix.isEmpty())
            {
                TQString dir = profileDirsPrefix + profile;
                addPrefix(dir, priority);
                addXdgConfigPrefix(dir+"/etc/xdg", priority);
                addXdgDataPrefix(dir+"/share", priority);
            }

            // iterating over all entries in the group Directories
            // to find entries that start with dir_$type
            TQMap<TQString, TQString> entries = config->entryMap(group);
            for (TQMap<TQString, TQString>::ConstIterator it2 = entries.begin();
                 it2 != entries.end(); it2++)
            {
                TQString key = it2.key();
                if (key.startsWith("dir_")) {
                    // generate directory list, there may be more than 1.
                    TQStringList dirs = TQStringList::split(',', *it2);
                    TQStringList::Iterator sIt(dirs.begin());
                    TQString resType = key.mid(4, key.length());
                    for (; sIt != dirs.end(); ++sIt)
                    {
                        addResourceDir(resType.latin1(), *sIt, priority);
                    }
                }
            }
            if (profiles.isEmpty())
                break;
            profile = profiles.back();
            group = TQString::fromLatin1("Directories-%1").arg(profile);
            profiles.pop_back();
            priority = true;
        }
    }

    // Process KIOSK restrictions.
    if (!kde_kiosk_admin || TQCString(getenv("TDE_KIOSK_NO_RESTRICTIONS")).isEmpty())
    {
        config->setGroup("KDE Resource Restrictions");
        TQMap<TQString, TQString> entries = config->entryMap("KDE Resource Restrictions");
        for (TQMap<TQString, TQString>::ConstIterator it2 = entries.begin();
            it2 != entries.end(); it2++)
        {
            TQString key = it2.key();
            if (!config->readBoolEntry(key, true))
            {
                d->restrictionsActive = true;
                d->restrictions.insert(key.latin1(), &d->restrictionsActive); // Anything will do
                dircache.remove(key.latin1());
            }
        }
    }

    config->setGroup(oldGroup);

    // check if the number of config dirs changed
    bool configDirsChanged = (resourceDirs("config").count() != configdirs);
    // If the config dirs changed, we check kiosk restrictions again.
    d->checkRestrictions = configDirsChanged;
    // return true if the number of config dirs changed: reparse config file
    return configDirsChanged;
}

TQString TDEStandardDirs::localtdedir() const
{
    // Return the prefix to use for saving
    return prefixes.first();
}

TQString TDEStandardDirs::localxdgdatadir() const
{
    // Return the prefix to use for saving
    return d->xdgdata_prefixes.first();
}

TQString TDEStandardDirs::localxdgconfdir() const
{
    // Return the prefix to use for saving
    return d->xdgconf_prefixes.first();
}


// just to make code more readable without macros
TQString locate( const char *type,
		const TQString& filename, const TDEInstance* inst )
{
    return inst->dirs()->findResource(type, filename);
}

TQString locateLocal( const char *type,
	             const TQString& filename, const TDEInstance* inst )
{
    return locateLocal(type, filename, true, inst);
}

TQString locateLocal( const char *type,
	             const TQString& filename, bool createDir, const TDEInstance* inst )
{
    // try to find slashes. If there are some, we have to
    // create the subdir first
    int slash = filename.findRev('/')+1;
    if (!slash) // only one filename
	return inst->dirs()->saveLocation(type, TQString::null, createDir) + filename;

    // split path from filename
    TQString dir = filename.left(slash);
    TQString file = filename.mid(slash);
    return inst->dirs()->saveLocation(type, dir, createDir) + file;
}

/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Pietro Iglio <iglio@kde.org>
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>

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

// $Id$

#include <stdlib.h>
#include <unistd.h>

#include <tqfile.h>
#include <tqdir.h>
#include <tqtextstream.h>

#include <kdebug.h>
#include "kurl.h"
#include "tdeconfigbackend.h"
#include "tdeapplication.h"
#include "kstandarddirs.h"
#include "kmountpoint.h"
#include "kcatalogue.h"
#include "tdelocale.h"

#include "kdesktopfile.h"
#include "kdesktopfile.moc"

KDesktopFile::KDesktopFile(const TQString &fileName, bool bReadOnly,
			   const char * resType)
  : TDEConfig(TQString::fromLatin1(""), bReadOnly, false)
{
  // TDEConfigBackEnd will try to locate the filename that is provided
  // based on the resource type specified, _only_ if the filename
  // is not an absolute path.
  backEnd->changeFileName(fileName, resType, false);
  setReadOnly(bReadOnly);
  reparseConfiguration();
  setDesktopGroup();
}

KDesktopFile::~KDesktopFile()
{
  // no need to do anything
}

TQString KDesktopFile::locateLocal(const TQString &path)
{
  TQString local;
  if (path.endsWith(".directory"))
  {
    local = path;
    if (!TQDir::isRelativePath(local))
    {
      // Relative wrt apps?
      local = TDEGlobal::dirs()->relativeLocation("apps", path);
    }

    if (TQDir::isRelativePath(local))
    {
      local = ::locateLocal("apps", local); // Relative to apps
    }
    else
    {
      // XDG Desktop menu items come with absolute paths, we need to 
      // extract their relative path and then build a local path.
      local = TDEGlobal::dirs()->relativeLocation("xdgdata-dirs", local);
      if (!TQDir::isRelativePath(local))
      {
        // Hm, that didn't work...
        // What now? Use filename only and hope for the best.
        local = path.mid(path.findRev('/')+1);
      }
      local = ::locateLocal("xdgdata-dirs", local);
    }
  }
  else
  {
    if (TQDir::isRelativePath(path))
    {
      local = ::locateLocal("apps", path); // Relative to apps
    }
    else
    {
      // XDG Desktop menu items come with absolute paths, we need to 
      // extract their relative path and then build a local path.
      local = TDEGlobal::dirs()->relativeLocation("xdgdata-apps", path);
      if (!TQDir::isRelativePath(local))
      {
        // What now? Use filename only and hope for the best.
        local = path.mid(path.findRev('/')+1);
      }
      local = ::locateLocal("xdgdata-apps", local);
    }
  }
  return local;
}

bool KDesktopFile::isDesktopFile(const TQString& path)
{
  int len = path.length();

  if(len > 8 && path.right(8) == TQString::fromLatin1(".desktop"))
    return true;
  else if(len > 7 && path.right(7) == TQString::fromLatin1(".kdelnk"))
    return true;
  else
    return false;
}

bool KDesktopFile::isAuthorizedDesktopFile(const TQString& path)
{
  if (!kapp || kapp->authorize("run_desktop_files"))
     return true;

  if (path.isEmpty())
     return false; // Empty paths are not ok.
  
  if (TQDir::isRelativePath(path))
     return true; // Relative paths are ok.
     
  TDEStandardDirs *dirs = TDEGlobal::dirs();
  if (TQDir::isRelativePath( dirs->relativeLocation("apps", path) ))
     return true;
  if (TQDir::isRelativePath( dirs->relativeLocation("xdgdata-apps", path) ))
     return true;
  if (TQDir::isRelativePath( dirs->relativeLocation("services", path) ))
     return true;
  if (dirs->relativeLocation("data", path).startsWith("kdesktop/Desktop"))
     return true;
     
  kdWarning() << "Access to '" << path << "' denied because of 'run_desktop_files' restriction." << endl;
  return false;
}

TQString KDesktopFile::translatedEntry(const char* key) const
{
  if (hasTranslatedKey(key))
    return readEntry(key);

  if (hasKey(key)) {
    TQString value = readEntryUntranslated(key);
    TQString fName = fileName();
    fName = fName.mid(fName.findRev('/')+1);
    TQString po_lookup_key = TQString::fromLatin1(key) + "(" + fName + "): " + value;
    TQString po_value = TDEGlobal::locale()->translate(po_lookup_key.utf8().data());

    if (po_value == po_lookup_key)
      return value;

    return po_value;
  }

  return TQString::null;
} 

TQString KDesktopFile::readType() const
{
  return readEntry("Type");
}

TQString KDesktopFile::readIcon() const
{
  return readEntry("Icon");
}

TQString KDesktopFile::readName() const
{
  return translatedEntry("Name");
}

TQString KDesktopFile::readComment() const
{
  return translatedEntry("Comment");
}

TQString KDesktopFile::readGenericName() const
{
  return translatedEntry("GenericName");
}

TQString KDesktopFile::readPath() const
{
  return readPathEntry("Path");
}

TQString KDesktopFile::readDevice() const
{
  return readEntry("Dev");
}

TQString KDesktopFile::readURL() const
{
    if (hasDeviceType()) {
        TQString device = readDevice();
        KMountPoint::List mountPoints = KMountPoint::possibleMountPoints();
	
        for(KMountPoint::List::ConstIterator it = mountPoints.begin();
            it != mountPoints.end(); ++it)
        {
            KMountPoint *mp = *it;
            if (mp->mountedFrom() == device)
            {
                KURL u;
                u.setPath( mp->mountPoint() );
                return u.url();
            }
        }
        return TQString::null;
    } else {
	TQString url = readPathEntry("URL");
        if ( !url.isEmpty() && !TQDir::isRelativePath(url) )
        {
            // Handle absolute paths as such (i.e. we need to escape them)
            KURL u;
            u.setPath( url );
            return u.url();
        }
        return url;
    }
}

TQStringList KDesktopFile::readActions() const
{
    return readListEntry("Actions", ';');
}

void KDesktopFile::setActionGroup(const TQString &group)
{
    setGroup(TQString::fromLatin1("Desktop Action ") + group);
}

bool KDesktopFile::hasActionGroup(const TQString &group) const
{
  return hasGroup(TQString::fromLatin1("Desktop Action ") + group);
}

bool KDesktopFile::hasLinkType() const
{
  return readEntry("Type") == TQString::fromLatin1("Link");
}

bool KDesktopFile::hasApplicationType() const
{
  return readEntry("Type") == TQString::fromLatin1("Application");
}

bool KDesktopFile::hasMimeTypeType() const
{
  return readEntry("Type") == TQString::fromLatin1("MimeType");
}

bool KDesktopFile::hasDeviceType() const
{
  return readEntry("Type") == TQString::fromLatin1("FSDev") ||
         readEntry("Type") == TQString::fromLatin1("FSDevice");
}

bool KDesktopFile::tryExec() const
{
  // Test for TryExec and "X-TDE-AuthorizeAction" 
  TQString te = readPathEntry("TryExec");

  if (!te.isEmpty()) {
    if (!TQDir::isRelativePath(te)) {
      if (::access(TQFile::encodeName(te), X_OK))
	return false;
    } else {
      // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
      // Environment PATH may contain filenames in 8bit locale cpecified
      // encoding (Like a filenames).
      TQStringList dirs = TQStringList::split(':', TQFile::decodeName(::getenv("PATH")));
      TQStringList::Iterator it(dirs.begin());
      bool match = false;
      for (; it != dirs.end(); ++it) {
	TQString fName = *it + "/" + te;
	if (::access(TQFile::encodeName(fName), X_OK) == 0)
	{
	  match = true;
	  break;
	}
      }
      // didn't match at all
      if (!match)
        return false;
    }
  }
  TQStringList list = readListEntry("X-TDE-AuthorizeAction");
  if (kapp && !list.isEmpty())
  {
     for(TQStringList::ConstIterator it = list.begin();
         it != list.end();
         ++it)
     {
        if (!kapp->authorize((*it).stripWhiteSpace()))
           return false;
     }
  }

  // See also KService::username()
  bool su = readBoolEntry("X-TDE-SubstituteUID");
  if (su)
  {
      TQString user = readEntry("X-TDE-Username");
      if (user.isEmpty())
        user = ::getenv("ADMIN_ACCOUNT");
      if (user.isEmpty())
        user = "root";
      if (!kapp->authorize("user/"+user))
        return false;
  }
  else {
      // Respect KDE su request if present
      su = readBoolEntry("X-KDE-SubstituteUID");
      if (su)
      {
          TQString user = readEntry("X-KDE-Username");
          if (user.isEmpty())
            user = ::getenv("ADMIN_ACCOUNT");
          if (user.isEmpty())
            user = "root";
          if (!kapp->authorize("user/"+user))
            return false;
      }
  }

  return true;
}

/**
 * @return the filename as passed to the constructor.
 */
TQString
KDesktopFile::fileName() const { return backEnd->fileName(); }

/**
 * @return the resource type as passed to the constructor.
 */
TQString
KDesktopFile::resource() const { return backEnd->resource(); }

TQStringList
KDesktopFile::sortOrder() const
{
  return readListEntry("SortOrder");
}

void KDesktopFile::virtual_hook( int id, void* data )
{ TDEConfig::virtual_hook( id, data ); }

TQString KDesktopFile::readDocPath() const
{
  return readPathEntry( "X-DocPath" );
}

KDesktopFile* KDesktopFile::copyTo(const TQString &file) const
{
  KDesktopFile *config = new KDesktopFile(TQString::null, false);
  TDEConfig::copyTo(file, config);
  config->setDesktopGroup();
  return config;
}

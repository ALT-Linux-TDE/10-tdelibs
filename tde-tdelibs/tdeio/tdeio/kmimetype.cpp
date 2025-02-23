/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
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

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

#include <kprotocolinfo.h>
#include <tdeio/global.h>
#include "kmimetype.h"
#include "kservicetypefactory.h"
#include "kmimemagic.h"
#include "kservice.h"
#include "krun.h"
#include "kautomount.h"
#include <kdirnotify_stub.h>

#include <tqstring.h>
#include <tqfile.h>
#include <kmessageboxwrapper.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <tdeapplication.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kdirwatch.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <tdesycoca.h>
#include <kde_file.h>

template class TDESharedPtr<KMimeType>;
template class TQValueList<KMimeType::Ptr>;

KMimeType::Ptr KMimeType::s_pDefaultType = 0L;
bool KMimeType::s_bChecked = false;

void KMimeType::buildDefaultType()
{
  assert ( !s_pDefaultType );
  // Try to find the default type
  KServiceType * mime = KServiceTypeFactory::self()->
        findServiceTypeByName( defaultMimeType() );

  if (mime && mime->isType( KST_KMimeType ))
  {
      s_pDefaultType = KMimeType::Ptr((KMimeType *) mime);
  }
  else
  {
     errorMissingMimeType( defaultMimeType() );
     TDEStandardDirs stdDirs;
     TQString sDefaultMimeType = stdDirs.resourceDirs("mime").first()+defaultMimeType()+".desktop";
     s_pDefaultType = new KMimeType( sDefaultMimeType, defaultMimeType(),
                                     "unknown", "mime", TQStringList() );
  }
}

KMimeType::Ptr KMimeType::defaultMimeTypePtr()
{
  if ( !s_pDefaultType ) // we need a default type first
    buildDefaultType();
  return s_pDefaultType;
}

// Check for essential mimetypes
void KMimeType::checkEssentialMimeTypes()
{
  if ( s_bChecked ) // already done
    return;
  if ( !s_pDefaultType ) // we need a default type first
    buildDefaultType();

  s_bChecked = true; // must be done before building mimetypes

  // No Mime-Types installed ?
  // Lets do some rescue here.
  if ( !KServiceTypeFactory::self()->checkMimeTypes() )
  {
    KMessageBoxWrapper::error( 0L, i18n( "No mime types installed." ) );
    return; // no point in going any further
  }

  if ( KMimeType::mimeType( "inode/directory" ) == s_pDefaultType )
    errorMissingMimeType( "inode/directory" );
  if ( KMimeType::mimeType( "inode/directory-locked" ) == s_pDefaultType )
    errorMissingMimeType( "inode/directory-locked" );
  if ( KMimeType::mimeType( "inode/blockdevice" ) == s_pDefaultType )
    errorMissingMimeType( "inode/blockdevice" );
  if ( KMimeType::mimeType( "inode/chardevice" ) == s_pDefaultType )
    errorMissingMimeType( "inode/chardevice" );
  if ( KMimeType::mimeType( "inode/socket" ) == s_pDefaultType )
    errorMissingMimeType( "inode/socket" );
  if ( KMimeType::mimeType( "inode/fifo" ) == s_pDefaultType )
    errorMissingMimeType( "inode/fifo" );
  if ( KMimeType::mimeType( "application/x-shellscript" ) == s_pDefaultType )
    errorMissingMimeType( "application/x-shellscript" );
  if ( KMimeType::mimeType( "application/x-executable" ) == s_pDefaultType )
    errorMissingMimeType( "application/x-executable" );
  if ( KMimeType::mimeType( "application/x-desktop" ) == s_pDefaultType )
    errorMissingMimeType( "application/x-desktop" );
}

void KMimeType::errorMissingMimeType( const TQString& _type )
{
  TQString tmp = i18n( "Could not find mime type\n%1" ).arg( _type );

  KMessageBoxWrapper::sorry( 0, tmp );
}

KMimeType::Ptr KMimeType::mimeType( const TQString& _name )
{
  KServiceType * mime = KServiceTypeFactory::self()->findServiceTypeByName( _name );

  if ( !mime || !mime->isType( KST_KMimeType ) )
  {
    // When building tdesycoca, findServiceTypeByName doesn't create an object
    // but returns one from a dict.
    if ( !KSycoca::self()->isBuilding() )
        delete mime;
    if ( !s_pDefaultType )
      buildDefaultType();
    return s_pDefaultType;
  }

  // We got a mimetype
  return KMimeType::Ptr((KMimeType *) mime);
}

KMimeType::List KMimeType::allMimeTypes()
{
  return KServiceTypeFactory::self()->allMimeTypes();
}

KMimeType::Ptr KMimeType::findByURL( const KURL& _url, mode_t _mode,
                                     bool _is_local_file, bool _fast_mode )
{
  checkEssentialMimeTypes();
  TQString path = _url.path();

  if ( !_fast_mode && !_is_local_file && _url.isLocalFile() )
    _is_local_file = true;

  if ( !_fast_mode && _is_local_file && (_mode == 0 || _mode == (mode_t)-1) )
  {
    KDE_struct_stat buff;
    if ( KDE_stat( TQFile::encodeName(path), &buff ) != -1 )
      _mode = buff.st_mode;
  }

  // Look at mode_t first
  if ( S_ISDIR( _mode ) )
  {
    // Special hack for local files. We want to see whether we
    // are allowed to enter the directory
    if ( _is_local_file )
    {
      if ( access( TQFile::encodeName(path), R_OK ) == -1 )
        return mimeType( "inode/directory-locked" );
    }
    return mimeType( "inode/directory" );
  }
  if ( S_ISCHR( _mode ) )
    return mimeType( "inode/chardevice" );
  if ( S_ISBLK( _mode ) )
    return mimeType( "inode/blockdevice" );
  if ( S_ISFIFO( _mode ) )
    return mimeType( "inode/fifo" );
  if ( S_ISSOCK( _mode ) )
    return mimeType( "inode/socket" );
  // KMimeMagic can do that better for local files
  if ( !_is_local_file && S_ISREG( _mode ) && ( _mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) )
    return mimeType( "application/x-executable" );

  TQString fileName ( _url.fileName() );

  static const TQString& slash = TDEGlobal::staticQString("/");
  if ( ! fileName.isNull() && !path.endsWith( slash ) )
  {
      // Try to find it out by looking at the filename
      KMimeType::Ptr mime = KServiceTypeFactory::self()->findFromPattern( fileName );
      if ( mime )
      {
        // Found something - can we trust it ? (e.g. don't trust *.pl over HTTP, could be anything)
        if ( _is_local_file || _url.hasSubURL() || // Explicitly trust suburls
             KProtocolInfo::determineMimetypeFromExtension( _url.protocol() ) )
        {
            if ( _is_local_file && !_fast_mode ) {
                if ( mime->patternsAccuracy()<100 )
                {
                    KMimeMagicResult* result =
                            KMimeMagic::self()->findFileType( path );

                    if ( result && result->isValid() && result->accuracy() > 0 ) {
                        KMimeType::Ptr resultMime = mimeType( result->mimeType() );
                        if (resultMime->patternsAccuracy() > 0) {
                            return resultMime;
                        }
                    }
                }
            }

            return mime;
        }
      }

      static const TQString& dotdesktop = TDEGlobal::staticQString(".desktop");
      static const TQString& dotkdelnk = TDEGlobal::staticQString(".kdelnk");
      static const TQString& dotdirectory = TDEGlobal::staticQString(".directory");

      // Another filename binding, hardcoded, is .desktop:
      if ( fileName.endsWith( dotdesktop ) )
        return mimeType( "application/x-desktop" );
      // Another filename binding, hardcoded, is .kdelnk;
      // this is preserved for backwards compatibility
      if ( fileName.endsWith( dotkdelnk ) )
        return mimeType( "application/x-desktop" );
      // .directory files are detected as x-desktop by mimemagic
      // but don't have a Type= entry. Better cheat and say they are text files
      if ( fileName == dotdirectory )
        return mimeType( "text/plain" );
    }

  if ( !_is_local_file || _fast_mode )
  {
    TQString def = KProtocolInfo::defaultMimetype( _url );
    if ( !def.isEmpty() && def != defaultMimeType() )
    {
       // The protocol says it always returns a given mimetype (e.g. text/html for "man:")
       return mimeType( def );
    }
    if ( path.endsWith( slash ) || path.isEmpty() )
    {
      // We have no filename at all. Maybe the protocol has a setting for
      // which mimetype this means (e.g. directory).
      // For HTTP (def==defaultMimeType()) we don't assume anything,
      // because of redirections (e.g. freshmeat downloads).
      if ( def.isEmpty() )
      {
          // Assume inode/directory, if the protocol supports listing.
          if ( KProtocolInfo::supportsListing( _url ) )
              return mimeType( TQString::fromLatin1("inode/directory") );
          else
              return defaultMimeTypePtr(); // == 'no idea', e.g. for "data:,foo/"
      }
    }

    // No more chances for non local URLs
    return defaultMimeTypePtr();
  }

  // Do some magic for local files
  //kdDebug(7009) << TQString("Mime Type finding for '%1'").arg(path) << endl;
  KMimeMagicResult* result = KMimeMagic::self()->findFileType( path );

  // If we still did not find it, we must assume the default mime type
  if ( !result || !result->isValid() )
    return defaultMimeTypePtr();

  // The mimemagic stuff was successful
  return mimeType( result->mimeType() );
}

KMimeType::Ptr KMimeType::findByURL( const KURL& _url, mode_t _mode,
                                     bool _is_local_file, bool _fast_mode,
                                     bool *accurate)
{
    KMimeType::Ptr mime = findByURL(_url, _mode, _is_local_file, _fast_mode);
    if (accurate) *accurate = !(_fast_mode) || ((mime->patternsAccuracy() == 100) && mime != defaultMimeTypePtr());
    return mime;
}

KMimeType::Ptr KMimeType::diagnoseFileName(const TQString &fileName, TQString &pattern)
{
  return KServiceTypeFactory::self()->findFromPattern( fileName, &pattern );
}

KMimeType::Ptr KMimeType::findByPath( const TQString& path, mode_t mode, bool fast_mode )
{
    KURL u;
    u.setPath(path);
    return findByURL( u, mode, true, fast_mode );
}

KMimeType::Ptr KMimeType::findByContent( const TQByteArray &data, int *accuracy )
{
  KMimeMagicResult *result = KMimeMagic::self()->findBufferType(data);
  if (accuracy)
      *accuracy = result->accuracy();
  return mimeType( result->mimeType() );
}

KMimeType::Ptr KMimeType::findByFileContent( const TQString &fileName, int *accuracy )
{
  KMimeMagicResult *result = KMimeMagic::self()->findFileType(fileName);
  if (accuracy)
      *accuracy = result->accuracy();
  return mimeType( result->mimeType() );
}

#define GZIP_MAGIC1	0x1f
#define GZIP_MAGIC2	0x8b

KMimeType::Format KMimeType::findFormatByFileContent( const TQString &fileName )
{
  KMimeType::Format result;
  result.compression = Format::NoCompression;
  KMimeType::Ptr mime = findByPath(fileName);

  result.text = mime->name().startsWith("text/");
  TQVariant v = mime->property("X-TDE-text");
  if (v.isValid())
     result.text = v.toBool();

  if (mime->name().startsWith("inode/"))
     return result;

  TQFile f(fileName);
  if (f.open(IO_ReadOnly))
  {
     unsigned char buf[10+1];
     int l = f.readBlock((char *)buf, 10);
     if ((l > 2) && (buf[0] == GZIP_MAGIC1) && (buf[1] == GZIP_MAGIC2))
        result.compression = Format::GZipCompression;
  }
  return result;
}

KMimeType::KMimeType( const TQString & _fullpath, const TQString& _type, const TQString& _icon,
                      const TQString& _comment, const TQStringList& _patterns )
  : KServiceType( _fullpath, _type, _icon, _comment )
{
  m_lstPatterns = _patterns;
}

KMimeType::KMimeType( const TQString & _fullpath ) : KServiceType( _fullpath )
{
  KDesktopFile _cfg( _fullpath, true );
  init ( &_cfg );

  if ( !isValid() )
    kdWarning(7009) << "mimetype not valid '" << m_strName << "' (missing entry in the file ?)" << endl;
}

KMimeType::KMimeType( KDesktopFile *config ) : KServiceType( config )
{
  init( config );

  if ( !isValid() )
    kdWarning(7009) << "mimetype not valid '" << m_strName << "' (missing entry in the file ?)" << endl;
}

void KMimeType::init( KDesktopFile * config )
{
  config->setDesktopGroup();
  m_lstPatterns = config->readListEntry( "Patterns", ';' );

  // Read the X-TDE-AutoEmbed setting and store it in the properties map
  TQString XKDEAutoEmbed = TQString::fromLatin1("X-TDE-AutoEmbed");
  if ( config->hasKey( XKDEAutoEmbed ) )
    m_mapProps.insert( XKDEAutoEmbed, TQVariant( config->readBoolEntry( XKDEAutoEmbed ) ) );

  TQString XKDEText = TQString::fromLatin1("X-TDE-text");
  if ( config->hasKey( XKDEText ) )
    m_mapProps.insert( XKDEText, config->readBoolEntry( XKDEText ) );

  TQString XKDEIsAlso = TQString::fromLatin1("X-TDE-IsAlso");
  if ( config->hasKey( XKDEIsAlso ) ) {
    TQString inherits = config->readEntry( XKDEIsAlso );
    if ( inherits != name() )
        m_mapProps.insert( XKDEIsAlso, inherits );
    else
        kdWarning(7009) << "Error: " << inherits << " inherits from itself!!!!" << endl;
  }

  TQString XKDEPatternsAccuracy = TQString::fromLatin1("X-TDE-PatternsAccuracy");
  if ( config->hasKey( XKDEPatternsAccuracy ) )
    m_mapProps.insert( XKDEPatternsAccuracy, config->readEntry( XKDEPatternsAccuracy ) );

}

KMimeType::KMimeType( TQDataStream& _str, int offset ) : KServiceType( _str, offset )
{
  loadInternal( _str ); // load our specific stuff
}

void KMimeType::load( TQDataStream& _str )
{
  KServiceType::load( _str );
  loadInternal( _str );
}

void KMimeType::loadInternal( TQDataStream& _str )
{
  // kdDebug(7009) << "KMimeType::load( TQDataStream& ) : loading list of patterns" << endl;
  _str >> m_lstPatterns;
}

void KMimeType::save( TQDataStream& _str )
{
  KServiceType::save( _str );
  // Warning adding/removing fields here involves a binary incompatible change - update version
  // number in tdesycoca.h
  _str << m_lstPatterns;
}

TQVariant KMimeType::property( const TQString& _name ) const
{
  if ( _name == "Patterns" )
    return TQVariant( m_lstPatterns );

  return KServiceType::property( _name );
}

TQStringList KMimeType::propertyNames() const
{
  TQStringList res = KServiceType::propertyNames();
  res.append( "Patterns" );

  return res;
}

KMimeType::~KMimeType()
{
}

TQPixmap KMimeType::pixmap( TDEIcon::Group _group, int _force_size, int _state,
                           TQString * _path ) const
{
  TDEIconLoader *iconLoader=TDEGlobal::iconLoader();
  TQString iconName=icon( TQString::null, false );
  if (!iconLoader->extraDesktopThemesAdded())
  {
    TQPixmap pixmap=iconLoader->loadIcon( iconName, _group, _force_size, _state, _path, true );
    if (!pixmap.isNull() ) return pixmap;

    iconLoader->addExtraDesktopThemes();
  }

  return iconLoader->loadIcon( iconName , _group, _force_size, _state, _path, false );
}

TQPixmap KMimeType::pixmap( const KURL& _url, TDEIcon::Group _group, int _force_size,
                           int _state, TQString * _path ) const
{
  TDEIconLoader *iconLoader=TDEGlobal::iconLoader();
  TQString iconName=icon( _url, _url.isLocalFile() );
  if (!iconLoader->extraDesktopThemesAdded())
  {
    TQPixmap pixmap=iconLoader->loadIcon( iconName, _group, _force_size, _state, _path, true );
    if (!pixmap.isNull() ) return pixmap;

    iconLoader->addExtraDesktopThemes();
  }

  return iconLoader->loadIcon( iconName , _group, _force_size, _state, _path, false );
}

TQPixmap KMimeType::pixmapForURL( const KURL & _url, mode_t _mode, TDEIcon::Group _group,
                                 int _force_size, int _state, TQString * _path )
{
  TDEIconLoader *iconLoader=TDEGlobal::iconLoader();
  TQString iconName = iconForURL( _url, _mode );

  if (!iconLoader->extraDesktopThemesAdded())
  {
    TQPixmap pixmap=iconLoader->loadIcon( iconName, _group, _force_size, _state, _path, true );
    if (!pixmap.isNull() ) return pixmap;

    iconLoader->addExtraDesktopThemes();
  }

  return iconLoader->loadIcon( iconName , _group, _force_size, _state, _path, false );

}

TQString KMimeType::iconForURL( const KURL & _url, mode_t _mode )
{
    const KMimeType::Ptr mt = findByURL( _url, _mode, _url.isLocalFile(),
                                         false /*HACK*/);
    static const TQString& unknown = TDEGlobal::staticQString("unknown");
    const TQString mimeTypeIcon = mt->icon( _url, _url.isLocalFile() );
    TQString i = mimeTypeIcon;

    // if we don't find an icon, maybe we can use the one for the protocol
    if ( i == unknown || i.isEmpty() || mt == defaultMimeTypePtr()
        // and for the root of the protocol (e.g. trash:/) the protocol icon has priority over the mimetype icon
        || _url.path().length() <= 1 )
    {
        i = favIconForURL( _url ); // maybe there is a favicon?

        if ( i.isEmpty() )
            i = KProtocolInfo::icon( _url.protocol() );

        // root of protocol: if we found nothing, revert to mimeTypeIcon (which is usually "folder")
        if ( _url.path().length() <= 1 && ( i == unknown || i.isEmpty() ) )
            i = mimeTypeIcon;

        // special case: root directory (/) -- Gitea issue #128
        if ( _url == KURL("file:///") )
            i = "folder_red";
    }
    return i;
}

TQString KMimeType::favIconForURL( const KURL& url )
{
    // this method will be called quite often, so better not read the config
    // again and again.
    static bool useFavIcons = true;
    static bool check = true;
    if ( check ) {
        check = false;
        TDEConfig *config = TDEGlobal::config();
        TDEConfigGroupSaver cs( config, "HTML Settings" );
        useFavIcons = config->readBoolEntry( "EnableFavicon", true );
    }

    if ( url.isLocalFile() || !url.protocol().startsWith("http")
         || !useFavIcons )
        return TQString::null;

    DCOPRef kded( "kded", "favicons" );
    DCOPReply result = kded.call( "iconForURL(KURL)", url );
    if ( result.isValid() )
        return result;

    return TQString::null;
}

TQString KMimeType::parentMimeType() const
{
  TQVariant v = property("X-TDE-IsAlso");
  return v.toString();
}

bool KMimeType::is( const TQString& mimeTypeName ) const
{
  if ( name() == mimeTypeName )
      return true;
  TQString st = parentMimeType();
  //if (st.isEmpty()) kdDebug(7009)<<"Parent mimetype is empty"<<endl;
  while ( !st.isEmpty() )
  {
      //kdDebug(7009)<<"Checking parent mime type: "<<st<<endl;
      KMimeType::Ptr ptr = KMimeType::mimeType( st );
      if (!ptr) return false; //error
      if ( ptr->name() == mimeTypeName )
          return true;
      st = ptr->parentMimeType();
  }
  return false;
}

int KMimeType::patternsAccuracy() const {
  TQVariant v = property("X-TDE-PatternsAccuracy");
  if (!v.isValid()) return 100;
  else
      return v.toInt();
}


/*******************************************************
 *
 * KFolderType
 *
 ******************************************************/

TQString KFolderType::icon( const TQString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::icon( _url, _is_local );

  return KFolderType::icon( KURL(_url), _is_local );
}

TQString KFolderType::icon( const KURL& _url, bool _is_local ) const
{
  if ( !_is_local )
    return KMimeType::icon( _url, _is_local );

  KURL u( _url );
  u.addPath( ".directory" );

  TQString icon;
  // using TDEStandardDirs as this one checks for path being
  // a file instead of a directory
  if ( TDEStandardDirs::exists( u.path() ) )
  {
    KSimpleConfig cfg( u.path(), true );
    cfg.setDesktopGroup();
    icon = cfg.readEntry( "Icon" );
    TQString empty_icon = cfg.readEntry( "EmptyIcon" );

    if ( !empty_icon.isEmpty() )
    {
      bool isempty = false;
      DIR *dp = 0L;
      struct dirent *ep;
      dp = opendir( TQFile::encodeName(_url.path()) );
      if ( dp )
      {
        TQValueList<TQCString> entries;
        // Note that readdir isn't guaranteed to return "." and ".." first (#79826)
        ep=readdir( dp ); if ( ep ) entries.append( ep->d_name );
        ep=readdir( dp ); if ( ep ) entries.append( ep->d_name );
        if ( (ep=readdir( dp )) == 0L ) // third file is NULL entry -> empty directory
          isempty = true;
        else {
          entries.append( ep->d_name );
          if ( readdir( dp ) == 0 ) { // only three
            // check if we got "." ".." and ".directory"
            isempty = entries.find( "." ) != entries.end() &&
                      entries.find( ".." ) != entries.end() &&
                      entries.find( ".directory" ) != entries.end();
          }
        }
        if (!isempty && !strcmp(ep->d_name, ".directory"))
          isempty = (readdir(dp) == 0L);
        closedir( dp );
      }

      if ( isempty )
        return empty_icon;
    }
  }

  if ( icon.isEmpty() )
    return KMimeType::icon( _url, _is_local );

  if ( icon.startsWith( "./" ) ) {
    // path is relative with respect to the location
    // of the .directory file (#73463)
    KURL v( _url );
    v.addPath( icon.mid( 2 ) );
    icon = v.path();
  }

  return icon;
}

TQString KFolderType::comment( const TQString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::comment( _url, _is_local );

  return KFolderType::comment( KURL(_url), _is_local );
}

TQString KFolderType::comment( const KURL& _url, bool _is_local ) const
{
  if ( !_is_local )
    return KMimeType::comment( _url, _is_local );

  KURL u( _url );
  u.addPath( ".directory" );

  KDesktopFile cfg( u.path(), true );
  TQString comment = cfg.readComment();
  if ( comment.isEmpty() )
    return KMimeType::comment( _url, _is_local );

  return comment;
}

/*******************************************************
 *
 * KDEDesktopMimeType
 *
 ******************************************************/

TQString KDEDesktopMimeType::icon( const TQString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::icon( _url, _is_local );

  KURL u( _url );
  return icon( u, _is_local );
}

TQString KDEDesktopMimeType::icon( const KURL& _url, bool _is_local ) const
{
  if ( !_is_local )
    return KMimeType::icon( _url, _is_local );

  KSimpleConfig cfg( _url.path(), true );
  cfg.setDesktopGroup();
  TQString icon = cfg.readEntry( "Icon" );
  TQString type = cfg.readEntry( "Type" );

  if ( type == "FSDevice" || type == "FSDev") // need to provide FSDev for
                                              // backwards compatibility
  {
    TQString unmount_icon = cfg.readEntry( "UnmountIcon" );
    TQString dev = cfg.readEntry( "Dev" );
    if ( !icon.isEmpty() && !unmount_icon.isEmpty() && !dev.isEmpty() )
    {
      TQString mp = TDEIO::findDeviceMountPoint( dev );
      // Is the device not mounted ?
      if ( mp.isNull() )
        return unmount_icon;
    }
  } else if ( type == "Link" ) {
      const TQString emptyIcon = cfg.readEntry( "EmptyIcon" );
      if ( !emptyIcon.isEmpty() ) {
          const TQString u = cfg.readPathEntry( "URL" );
          const KURL url( u );
          if ( url.protocol() == "trash" ) {
              // We need to find if the trash is empty, preferrably without using a TDEIO job.
              // So instead tdeio_trash leaves an entry in its config file for us.
              KSimpleConfig trashConfig( "trashrc", true );
              trashConfig.setGroup( "Status" );
              if ( trashConfig.readBoolEntry( "Empty", true ) ) {
                  return emptyIcon;
              }
          }
      }
  }

  if ( icon.isEmpty() )
    return KMimeType::icon( _url, _is_local );

  return icon;
}

TQPixmap KDEDesktopMimeType::pixmap( const KURL& _url, TDEIcon::Group _group, int _force_size,
                                    int _state, TQString * _path ) const
{
  TQString _icon = icon( _url, _url.isLocalFile() );
  TQPixmap pix = TDEGlobal::iconLoader()->loadIcon( _icon, _group,
        _force_size, _state, _path, false );
  if ( pix.isNull() )
      pix = TDEGlobal::iconLoader()->loadIcon( "unknown", _group,
        _force_size, _state, _path, false );
  return pix;
}

TQString KDEDesktopMimeType::comment( const TQString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::comment( _url, _is_local );

  KURL u( _url );
  return comment( u, _is_local );
}

TQString KDEDesktopMimeType::comment( const KURL& _url, bool _is_local ) const
{
  if ( !_is_local )
    return KMimeType::comment( _url, _is_local );

  KDesktopFile cfg( _url.path(), true );
  TQString comment = cfg.readComment();
  if ( comment.isEmpty() )
    return KMimeType::comment( _url, _is_local );

  return comment;
}

pid_t KDEDesktopMimeType::run( const KURL& u, bool _is_local )
{
  // It might be a security problem to run external untrusted desktop
  // entry files
  if ( !_is_local )
    return 0;

  KSimpleConfig cfg( u.path(), true );
  cfg.setDesktopGroup();
  TQString type = cfg.readEntry( "Type" );
  if ( type.isEmpty() )
  {
    TQString tmp = i18n("The desktop entry file %1 "
                       "has no Type=... entry.").arg(u.path() );
    KMessageBoxWrapper::error( 0, tmp);
    return 0;
  }

  //kdDebug(7009) << "TYPE = " << type.data() << endl;

  if ( type == "FSDevice" )
    return runFSDevice( u, cfg );
  else if ( type == "Application" )
    return runApplication( u, u.path() );
  else if ( type == "Link" )
  {
    cfg.setDollarExpansion( true ); // for URL=file:$HOME (Simon)
    return runLink( u, cfg );
  }
  else if ( type == "MimeType" )
    return runMimeType( u, cfg );


  TQString tmp = i18n("The desktop entry of type\n%1\nis unknown.").arg( type );
  KMessageBoxWrapper::error( 0, tmp);

  return 0;
}

pid_t KDEDesktopMimeType::runFSDevice( const KURL& _url, const KSimpleConfig &cfg )
{
  pid_t retval = 0;

  TQString dev = cfg.readEntry( "Dev" );

  if ( dev.isEmpty() )
  {
    TQString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.").arg( _url.path() );
    KMessageBoxWrapper::error( 0, tmp);
    return retval;
  }

  TQString mp = TDEIO::findDeviceMountPoint( dev );
  // Is the device already mounted ?
  if ( !mp.isNull() )
  {
    KURL mpURL;
    mpURL.setPath( mp );
    // Open a new window
    retval = KRun::runURL( mpURL, TQString::fromLatin1("inode/directory") );
  }
  else
  {
    bool ro = cfg.readBoolEntry( "ReadOnly", false );
    TQString fstype = cfg.readEntry( "FSType" );
    if ( fstype == "Default" ) // KDE-1 thing
      fstype = TQString::null;
    TQString point = cfg.readEntry( "MountPoint" );
#ifndef TQ_WS_WIN
    (void) new KAutoMount( ro, fstype, dev, point, _url.path() );
#endif
    retval = -1; // we don't want to return 0, but we don't want to return a pid
  }

  return retval;
}

pid_t KDEDesktopMimeType::runApplication( const KURL& , const TQString & _serviceFile )
{
  KService s( _serviceFile );
  if ( !s.isValid() )
    // The error message was already displayed, so we can just quit here
    return 0;

  KURL::List lst;
  return KRun::run( s, lst );
}

pid_t KDEDesktopMimeType::runLink( const KURL& _url, const KSimpleConfig &cfg )
{
  TQString u = cfg.readPathEntry( "URL" );
  if ( u.isEmpty() )
  {
    TQString tmp = i18n("The desktop entry file\n%1\nis of type Link but has no URL=... entry.").arg( _url.prettyURL() );
    KMessageBoxWrapper::error( 0, tmp );
    return 0;
  }

  KURL url ( u );
  KRun* run = new KRun(url);

  // X-TDE-LastOpenedWith holds the service desktop entry name that
  // was should be preferred for opening this URL if possible.
  // This is used by the Recent Documents menu for instance.
  TQString lastOpenedWidth = cfg.readEntry( "X-TDE-LastOpenedWith" );
  if ( !lastOpenedWidth.isEmpty() )
      run->setPreferredService( lastOpenedWidth );

  return -1; // we don't want to return 0, but we don't want to return a pid
}

pid_t KDEDesktopMimeType::runMimeType( const KURL& url , const KSimpleConfig & )
{
  // Hmm, can't really use keditfiletype since we might be looking
  // at the global file, or at a file not in share/mimelnk...

  TQStringList args;
  args << "openProperties";
  args << url.path();

  int pid;
  if ( !TDEApplication::tdeinitExec("kfmclient", args, 0, &pid) )
      return pid;

  TDEProcess p;
  p << "kfmclient" << args;
  p.start(TDEProcess::DontCare);
  return p.pid();
}

TQValueList<KDEDesktopMimeType::Service> KDEDesktopMimeType::builtinServices( const KURL& _url )
{
  TQValueList<Service> result;

  if ( !_url.isLocalFile() )
    return result;

  KSimpleConfig cfg( _url.path(), true );
  cfg.setDesktopGroup();
  TQString type = cfg.readEntry( "Type" );

  if ( type.isEmpty() )
    return result;

  if ( type == "FSDevice" )
  {
    TQString dev = cfg.readEntry( "Dev" );
    if ( dev.isEmpty() )
    {
      TQString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.").arg( _url.path() );
      KMessageBoxWrapper::error( 0, tmp);
    }
    else
    {
      TQString mp = TDEIO::findDeviceMountPoint( dev );
      // not mounted ?
      if ( mp.isEmpty() )
      {
        Service mount;
        mount.m_strName = i18n("Mount");
        mount.m_type = ST_MOUNT;
        result.append( mount );
      }
      else
      {
        Service unmount;
#ifdef HAVE_VOLMGT
        /*
         *  Solaris' volume management can only umount+eject
         */
        unmount.m_strName = i18n("Eject");
#else
        unmount.m_strName = i18n("Unmount");
#endif
        unmount.m_type = ST_UNMOUNT;
        result.append( unmount );
      }
    }
  }

  return result;
}

TQValueList<KDEDesktopMimeType::Service> KDEDesktopMimeType::userDefinedServices( const TQString& path, bool bLocalFiles )
{
  KSimpleConfig cfg( path, true );
  return userDefinedServices( path, cfg, bLocalFiles );
}

TQValueList<KDEDesktopMimeType::Service> KDEDesktopMimeType::userDefinedServices( const TQString& path, TDEConfig& cfg, bool bLocalFiles )
{
 return userDefinedServices( path, cfg, bLocalFiles, KURL::List() );
}

TQValueList<KDEDesktopMimeType::Service> KDEDesktopMimeType::userDefinedServices( const TQString& path, TDEConfig& cfg, bool bLocalFiles, const KURL::List & file_list )
{
  TQValueList<Service> result;

  cfg.setDesktopGroup();

  if ( !cfg.hasKey( "Actions" ) && !cfg.hasKey( "X-TDE-GetActionMenu") )
    return result;

  if ( cfg.hasKey( "TryExec" ) )
  {
      TQString tryexec = cfg.readPathEntry( "TryExec" );
      TQString exe =  TDEStandardDirs::findExe( tryexec );
      if (exe.isEmpty()) {
          return result;
      }
  }

  TQStringList keys;

  if( cfg.hasKey( "X-TDE-GetActionMenu" )) {
    TQString dcopcall = cfg.readEntry( "X-TDE-GetActionMenu" );
    const TQCString app = TQString(dcopcall.section(' ', 0,0)).utf8();

    TQByteArray dataToSend;
    TQDataStream dataStream(dataToSend, IO_WriteOnly);
    dataStream << file_list;
    TQCString replyType;
    TQByteArray replyData;
    TQCString object = TQString(dcopcall.section(' ', 1,-2)).utf8();
    TQString function = dcopcall.section(' ', -1);
    if(!function.endsWith("(KURL::List)")) {
      kdWarning() << "Desktop file " << path << " contains an invalid X-TDE-ShowIfDcopCall - the function must take the exact parameter (KURL::List) and must be specified." << endl;
    } else {
      if(kapp->dcopClient()->call( app, object,
                   function.utf8(),
                   dataToSend, replyType, replyData, true, -1)
	    && replyType == "TQStringList" ) {

        TQDataStream dataStreamIn(replyData, IO_ReadOnly);
        dataStreamIn >> keys;
      }
    }
  }

  keys += cfg.readListEntry( "Actions", ';' ); //the desktop standard defines ";" as separator!

  if ( keys.count() == 0 )
    return result;

  TQStringList::ConstIterator it = keys.begin();
  TQStringList::ConstIterator end = keys.end();
  for ( ; it != end; ++it )
  {
    //kdDebug(7009) << "CURRENT KEY = " << (*it) << endl;

    TQString group = *it;

    if (group == "_SEPARATOR_")
    {
        Service s;
        result.append(s);
        continue;
    }

    group.prepend( "Desktop Action " );

    bool bInvalidMenu = false;

    if ( cfg.hasGroup( group ) )
    {
      cfg.setGroup( group );

      if ( !cfg.hasKey( "Name" ) || !cfg.hasKey( "Exec" ) )
        bInvalidMenu = true;
      else
      {
        TQString exec = cfg.readPathEntry( "Exec" );
        if ( bLocalFiles || exec.contains("%U") || exec.contains("%u") )
        {
          Service s;
          s.m_strName = cfg.readEntry( "Name" );
          s.m_strIcon = cfg.readEntry( "Icon" );
          s.m_strExec = exec;
          s.m_type = ST_USER_DEFINED;
          s.m_display = !cfg.readBoolEntry( "NoDisplay" );
          result.append( s );
        }
      }
    }
    else
      bInvalidMenu = true;

    if ( bInvalidMenu )
    {
      TQString tmp = i18n("The desktop entry file\n%1\n has an invalid menu entry\n%2.").arg( path ).arg( *it );
      KMessageBoxWrapper::error( 0, tmp );
    }
  }

  return result;
}

void KDEDesktopMimeType::executeService( const TQString& _url, KDEDesktopMimeType::Service& _service )
{
    KURL u;
    u.setPath(_url);
    KURL::List lst;
    lst.append( u );
    executeService( lst, _service );
}

void KDEDesktopMimeType::executeService( const KURL::List& urls, KDEDesktopMimeType::Service& _service )
{
  //kdDebug(7009) << "EXECUTING Service " << _service.m_strName << endl;

  if ( _service.m_type == ST_USER_DEFINED )
  {
    kdDebug() << "KDEDesktopMimeType::executeService " << _service.m_strName
              << " first url's path=" << urls.first().path() << " exec=" << _service.m_strExec << endl;
    KRun::run( _service.m_strExec, urls, _service.m_strName, _service.m_strIcon, _service.m_strIcon );
    // The action may update the desktop file. Example: eject unmounts (#5129).
    KDirNotify_stub allDirNotify("*", "KDirNotify*");
    allDirNotify.FilesChanged( urls );
    return;
  }
  else if ( _service.m_type == ST_MOUNT || _service.m_type == ST_UNMOUNT )
  {
    Q_ASSERT( urls.count() == 1 );
    TQString path = urls.first().path();
    //kdDebug(7009) << "MOUNT&UNMOUNT" << endl;

    KSimpleConfig cfg( path, true );
    cfg.setDesktopGroup();
    TQString dev = cfg.readEntry( "Dev" );
    if ( dev.isEmpty() )
    {
      TQString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.").arg( path );
      KMessageBoxWrapper::error( 0, tmp );
      return;
    }
    TQString mp = TDEIO::findDeviceMountPoint( dev );

    if ( _service.m_type == ST_MOUNT )
    {
      // Already mounted? Strange, but who knows ...
      if ( !mp.isEmpty() )
      {
        kdDebug(7009) << "ALREADY Mounted" << endl;
        return;
      }

      bool ro = cfg.readBoolEntry( "ReadOnly", false );
      TQString fstype = cfg.readEntry( "FSType" );
      if ( fstype == "Default" ) // KDE-1 thing
          fstype = TQString::null;
      TQString point = cfg.readEntry( "MountPoint" );
#ifndef TQ_WS_WIN
      (void)new KAutoMount( ro, fstype, dev, point, path, false );
#endif
    }
    else if ( _service.m_type == ST_UNMOUNT )
    {
      // Not mounted? Strange, but who knows ...
      if ( mp.isEmpty() )
        return;

#ifndef TQ_WS_WIN
      (void)new KAutoUnmount( mp, path );
#endif
    }
  }
  else
    assert( 0 );
}

const TQString & KMimeType::defaultMimeType()
{
    static const TQString & s_strDefaultMimeType =
        TDEGlobal::staticQString( "application/octet-stream" );
    return s_strDefaultMimeType;
}

void KMimeType::virtual_hook( int id, void* data )
{ KServiceType::virtual_hook( id, data ); }

void KFolderType::virtual_hook( int id, void* data )
{ KMimeType::virtual_hook( id, data ); }

void KDEDesktopMimeType::virtual_hook( int id, void* data )
{ KMimeType::virtual_hook( id, data ); }

void KExecMimeType::virtual_hook( int id, void* data )
{ KMimeType::virtual_hook( id, data ); }

#include "kmimetyperesolver.moc"


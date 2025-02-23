/* This file is part of the KDE project

   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (c) 1999, 2000 Preston Brown <pbrown@kde.org>
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (c) 2000 David Faure <faure@kde.org>
   Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

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

/*
 * kpropertiesdialog.cpp
 * View/Edit Properties of files, locally or remotely
 *
 * some FilePermissionsPropsPlugin-changes by
 *  Henner Zeller <zeller@think.de>
 * some layout management by
 *  Bertrand Leconte <B.Leconte@mail.dotcom.fr>
 * the rest of the layout management, bug fixes, adaptation to libtdeio,
 * template feature by
 *  David Faure <faure@kde.org>
 * More layout, cleanups, and fixes by
 *  Preston Brown <pbrown@kde.org>
 * Plugin capability, cleanups and port to KDialogBase by
 *  Simon Hausmann <hausmann@kde.org>
 * KDesktopPropsPlugin by
 *  Waldo Bastian <bastian@kde.org>
 */

#include <config.h>
extern "C" {
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>
}
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <algorithm>
#include <functional>

#include <tqfile.h>
#include <tqdir.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqcheckbox.h>
#include <tqstrlist.h>
#include <tqstringlist.h>
#include <tqtextstream.h>
#include <tqpainter.h>
#include <tqlayout.h>
#include <tqcombobox.h>
#include <tqgroupbox.h>
#include <tqwhatsthis.h>
#include <tqtooltip.h>
#include <tqstyle.h>
#include <tqprogressbar.h>
#include <tqvbox.h>
#include <tqvaluevector.h>

#ifdef USE_POSIX_ACL
extern "C" {
#include <sys/param.h>
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif
}
#endif

#include <tdeapplication.h>
#include <kdialog.h>
#include <kdirsize.h>
#include <kdirwatch.h>
#include <kdirnotify_stub.h>
#include <kdiskfreesp.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kicondialog.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kstandarddirs.h>
#include <tdeio/job.h>
#include <tdeio/chmodjob.h>
#include <tdeio/renamedlg.h>
#include <tdeio/netaccess.h>
#include <tdeio/kservicetypefactory.h>
#include <tdefiledialog.h>
#include <kmimetype.h>
#include <kmountpoint.h>
#include <kiconloader.h>
#include <tdemessagebox.h>
#include <kservice.h>
#include <kcompletion.h>
#include <klineedit.h>
#include <kseparator.h>
#include <ksqueezedtextlabel.h>
#include <klibloader.h>
#include <ktrader.h>
#include <tdeparts/componentfactory.h>
#include <kmetaprops.h>
#include <kpreviewprops.h>
#include <kprocess.h>
#include <krun.h>
#include <tdelistview.h>
#include <kacl.h>
#include "tdefilesharedlg.h"

#include "kpropertiesdesktopbase.h"
#include "kpropertiesdesktopadvbase.h"
#include "kpropertiesmimetypebase.h"
#ifdef USE_POSIX_ACL
#include "kacleditwidget.h"
#endif

#include "kpropertiesdialog.h"

#ifdef TQ_WS_WIN
# include <win32_utils.h>
#endif

static TQString nameFromFileName(TQString nameStr)
{
   if ( nameStr.endsWith(".desktop") )
      nameStr.truncate( nameStr.length() - 8 );
   if ( nameStr.endsWith(".kdelnk") )
      nameStr.truncate( nameStr.length() - 7 );
   // Make it human-readable (%2F => '/', ...)
   nameStr = TDEIO::decodeFileName( nameStr );
   return nameStr;
}

mode_t KFilePermissionsPropsPlugin::fperm[3][4] = {
        {S_IRUSR, S_IWUSR, S_IXUSR, S_ISUID},
        {S_IRGRP, S_IWGRP, S_IXGRP, S_ISGID},
        {S_IROTH, S_IWOTH, S_IXOTH, S_ISVTX}
    };

class KPropertiesDialog::KPropertiesDialogPrivate
{
public:
  KPropertiesDialogPrivate()
  {
    m_aborted = false;
    fileSharePage = 0;
  }
  ~KPropertiesDialogPrivate()
  {
  }
  bool m_aborted:1;
  TQWidget* fileSharePage;
};

KPropertiesDialog::KPropertiesDialog (KFileItem* item,
                                      TQWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed, i18n( "Properties for %1" ).arg(TDEIO::decodeFileName(item->url().fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal)
{
  d = new KPropertiesDialogPrivate;
  assert( item );
  m_items.append( new KFileItem(*item) ); // deep copy

  m_singleUrl = item->url();
  assert(!m_singleUrl.isEmpty());

  init (modal, autoShow);
}

KPropertiesDialog::KPropertiesDialog (const TQString& title,
                                      TQWidget* parent, const char* name, bool modal)
  : KDialogBase (KDialogBase::Tabbed, i18n ("Properties for %1").arg(title),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal)
{
  d = new KPropertiesDialogPrivate;

  init (modal, false);
}

KPropertiesDialog::KPropertiesDialog (KFileItemList _items,
                                      TQWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed,
                 // TODO: replace <never used> with "Properties for 1 item". It's very confusing how it has to be translated otherwise
                 // (empty translation before the "\n" is not allowed by msgfmt...)
		 _items.count()>1 ? i18n( "<never used>","Properties for %n Selected Items",_items.count()) :
		 i18n( "Properties for %1" ).arg(TDEIO::decodeFileName(_items.first()->url().fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal)
{
  d = new KPropertiesDialogPrivate;

  assert( !_items.isEmpty() );
  m_singleUrl = _items.first()->url();
  assert(!m_singleUrl.isEmpty());

  KFileItemListIterator it ( _items );
  // Deep copy
  for ( ; it.current(); ++it )
      m_items.append( new KFileItem( **it ) );

  init (modal, autoShow);
}

#ifndef KDE_NO_COMPAT
KPropertiesDialog::KPropertiesDialog (const KURL& _url, mode_t /* _mode is now unused */,
                                      TQWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed,
		 i18n( "Properties for %1" ).arg(TDEIO::decodeFileName(_url.fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal),
  m_singleUrl( _url )
{
  d = new KPropertiesDialogPrivate;

  TDEIO::UDSEntry entry;

  TDEIO::NetAccess::stat(_url, entry, parent);

  m_items.append( new KFileItem( entry, _url ) );
  init (modal, autoShow);
}
#endif

KPropertiesDialog::KPropertiesDialog (const KURL& _url,
                                      TQWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed,
		 i18n( "Properties for %1" ).arg(TDEIO::decodeFileName(_url.fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal),
  m_singleUrl( _url )
{
  d = new KPropertiesDialogPrivate;

  TDEIO::UDSEntry entry;

  TDEIO::NetAccess::stat(_url, entry, parent);

  m_items.append( new KFileItem( entry, _url ) );
  init (modal, autoShow);
}

KPropertiesDialog::KPropertiesDialog (const KURL& _tempUrl, const KURL& _currentDir,
                                      const TQString& _defaultName,
                                      TQWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed,
		 i18n( "Properties for %1" ).arg(TDEIO::decodeFileName(_tempUrl.fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal),

  m_singleUrl( _tempUrl ),
  m_defaultName( _defaultName ),
  m_currentDir( _currentDir )
{
  d = new KPropertiesDialogPrivate;

  assert(!m_singleUrl.isEmpty());

  // Create the KFileItem for the _template_ file, in order to read from it.
  m_items.append( new KFileItem( KFileItem::Unknown, KFileItem::Unknown, m_singleUrl ) );
  init (modal, autoShow);
}

bool KPropertiesDialog::showDialog(KFileItem* item, TQWidget* parent,
                                   const char* name, bool modal)
{
#ifdef TQ_WS_WIN
  TQString localPath = item->localPath();
  if (!localPath.isEmpty())
    return showWin32FilePropertyDialog(localPath);
#endif
  new KPropertiesDialog(item, parent, name, modal);
  return true;
}

bool KPropertiesDialog::showDialog(const KURL& _url, TQWidget* parent,
                                   const char* name, bool modal)
{
#ifdef TQ_WS_WIN
  if (_url.isLocalFile())
    return showWin32FilePropertyDialog( _url.path() );
#endif
  new KPropertiesDialog(_url, parent, name, modal);
  return true;
}

bool KPropertiesDialog::showDialog(const KFileItemList& _items, TQWidget* parent,
                                   const char* name, bool modal)
{
  if (_items.count()==1)
    return KPropertiesDialog::showDialog(_items.getFirst(), parent, name, modal);
  new KPropertiesDialog(_items, parent, name, modal);
  return true;
}

void KPropertiesDialog::init (bool modal, bool autoShow)
{
  m_pageList.setAutoDelete( true );
  m_items.setAutoDelete( true );

  insertPages();

  if (autoShow)
    {
      if (!modal)
        show();
      else
        exec();
    }
}

void KPropertiesDialog::showFileSharingPage()
{
  if (d->fileSharePage) {
     showPage( pageIndex( d->fileSharePage));
  }
}

void KPropertiesDialog::setFileSharingPage(TQWidget* page) {
  d->fileSharePage = page;
}


void KPropertiesDialog::setFileNameReadOnly( bool ro )
{
    KPropsDlgPlugin *it;

    for ( it=m_pageList.first(); it != 0L; it=m_pageList.next() )
    {
        KFilePropsPlugin* plugin = dynamic_cast<KFilePropsPlugin*>(it);
        if ( plugin ) {
            plugin->setFileNameReadOnly( ro );
            break;
        }
    }
}

void KPropertiesDialog::slotStatResult( TDEIO::Job * )
{
}

KPropertiesDialog::~KPropertiesDialog()
{
  m_pageList.clear();
  delete d;
}

void KPropertiesDialog::insertPlugin (KPropsDlgPlugin* plugin)
{
  connect (plugin, TQ_SIGNAL (changed ()),
           plugin, TQ_SLOT (setDirty ()));

  m_pageList.append (plugin);
}

bool KPropertiesDialog::canDisplay( KFileItemList _items )
{
  // TODO: cache the result of those calls. Currently we parse .desktop files far too many times
  return KFilePropsPlugin::supports( _items ) ||
         KFilePermissionsPropsPlugin::supports( _items ) ||
         KDesktopPropsPlugin::supports( _items ) ||
         KBindingPropsPlugin::supports( _items ) ||
         KURLPropsPlugin::supports( _items ) ||
         KDevicePropsPlugin::supports( _items ) ||
         KFileMetaPropsPlugin::supports( _items ) ||
         KPreviewPropsPlugin::supports( _items );
}

void KPropertiesDialog::slotOk()
{
  KPropsDlgPlugin *page;
  d->m_aborted = false;

  KFilePropsPlugin * filePropsPlugin = 0L;
  if ( m_pageList.first()->isA("KFilePropsPlugin") )
    filePropsPlugin = static_cast<KFilePropsPlugin *>(m_pageList.first());

  // If any page is dirty, then set the main one (KFilePropsPlugin) as
  // dirty too. This is what makes it possible to save changes to a global
  // desktop file into a local one. In other cases, it doesn't hurt.
  for ( page = m_pageList.first(); page != 0L; page = m_pageList.next() )
    if ( page->isDirty() && filePropsPlugin )
    {
        filePropsPlugin->setDirty();
        break;
    }

  // Apply the changes in the _normal_ order of the tabs now
  // This is because in case of renaming a file, KFilePropsPlugin will call
  // KPropertiesDialog::rename, so other tab will be ok with whatever order
  // BUT for file copied from templates, we need to do the renaming first !
  for ( page = m_pageList.first(); page != 0L && !d->m_aborted; page = m_pageList.next() )
    if ( page->isDirty() )
    {
      kdDebug( 250 ) << "applying changes for " << page->className() << endl;
      page->applyChanges();
      // applyChanges may change d->m_aborted.
    }
    else
      kdDebug( 250 ) << "skipping page " << page->className() << endl;

  if ( !d->m_aborted && filePropsPlugin )
    filePropsPlugin->postApplyChanges();

  if ( !d->m_aborted )
  {
    emit applied();
    emit propertiesClosed();
    deleteLater();
    accept();
  } // else, keep dialog open for user to fix the problem.
}

void KPropertiesDialog::slotCancel()
{
  emit canceled();
  emit propertiesClosed();

  deleteLater();
  done( Rejected );
}

void KPropertiesDialog::insertPages()
{
  if (m_items.isEmpty())
    return;

  if ( KFilePropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KFilePropsPlugin( this );
    insertPlugin (p);
  }

  if ( KFilePermissionsPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KFilePermissionsPropsPlugin( this );
    insertPlugin (p);
  }

  if ( KDesktopPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KDesktopPropsPlugin( this );
    insertPlugin (p);
  }

  if ( KBindingPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KBindingPropsPlugin( this );
    insertPlugin (p);
  }

  if ( KURLPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KURLPropsPlugin( this );
    insertPlugin (p);
  }

  if ( KDevicePropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KDevicePropsPlugin( this );
    insertPlugin (p);
  }

  if ( KFileMetaPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KFileMetaPropsPlugin( this );
    insertPlugin (p);
  }

  if ( KPreviewPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KPreviewPropsPlugin( this );
    insertPlugin (p);
  }

  if ( kapp->authorizeTDEAction("sharefile") &&
       KFileSharePropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KFileSharePropsPlugin( this );
    insertPlugin (p);
  }

  //plugins

  if ( m_items.count() != 1 )
    return;

  KFileItem *item = m_items.first();
  TQString mimetype = item->mimetype();

  if ( mimetype.isEmpty() )
    return;

  TQString query = TQString::fromLatin1(
      "('KPropsDlg/Plugin' in ServiceTypes) and "
      "((not exist [X-TDE-Protocol]) or "
      " ([X-TDE-Protocol] == '%1'  )   )"          ).arg(item->url().protocol());

  kdDebug( 250 ) << "trader query: " << query << endl;
  TDETrader::OfferList offers = TDETrader::self()->query( mimetype, query );
  TDETrader::OfferList::ConstIterator it = offers.begin();
  TDETrader::OfferList::ConstIterator end = offers.end();
  for (; it != end; ++it )
  {
    KPropsDlgPlugin *plugin = KParts::ComponentFactory
        ::createInstanceFromLibrary<KPropsDlgPlugin>( (*it)->library().local8Bit().data(),
                                                      this,
                                                      (*it)->name().latin1() );
    if ( !plugin )
        continue;

    insertPlugin( plugin );
  }
}

void KPropertiesDialog::updateUrl( const KURL& _newUrl )
{
  Q_ASSERT( m_items.count() == 1 );
  kdDebug(250) << "KPropertiesDialog::updateUrl (pre)" << _newUrl.url() << endl;
  KURL newUrl = _newUrl;
  emit saveAs(m_singleUrl, newUrl);
  kdDebug(250) << "KPropertiesDialog::updateUrl (post)" << newUrl.url() << endl;

  m_singleUrl = newUrl;
  m_items.first()->setURL( newUrl );
  assert(!m_singleUrl.isEmpty());
  // If we have an Desktop page, set it dirty, so that a full file is saved locally
  // Same for a URL page (because of the Name= hack)
  for ( TQPtrListIterator<KPropsDlgPlugin> it(m_pageList); it.current(); ++it )
   if ( it.current()->isA("KExecPropsPlugin") || // KDE4 remove me
        it.current()->isA("KURLPropsPlugin") ||
        it.current()->isA("KDesktopPropsPlugin"))
   {
     //kdDebug(250) << "Setting page dirty" << endl;
     it.current()->setDirty();
     break;
   }
}

void KPropertiesDialog::rename( const TQString& _name )
{
  Q_ASSERT( m_items.count() == 1 );
  kdDebug(250) << "KPropertiesDialog::rename " << _name << endl;
  KURL newUrl;
  // if we're creating from a template : use currentdir
  if ( !m_currentDir.isEmpty() )
  {
    newUrl = m_currentDir;
    newUrl.addPath( _name );
  }
  else
  {
    TQString tmpurl = m_singleUrl.url();
    if ( tmpurl.at(tmpurl.length() - 1) == '/')
      // It's a directory, so strip the trailing slash first
      tmpurl.truncate( tmpurl.length() - 1);
    newUrl = tmpurl;
    newUrl.setFileName( _name );
  }
  updateUrl( newUrl );
}

void KPropertiesDialog::abortApplying()
{
  d->m_aborted = true;
}

class KPropsDlgPlugin::KPropsDlgPluginPrivate
{
public:
  KPropsDlgPluginPrivate()
  {
  }
  ~KPropsDlgPluginPrivate()
  {
  }

  bool m_bDirty;
};

KPropsDlgPlugin::KPropsDlgPlugin( KPropertiesDialog *_props )
: TQObject( _props, 0L )
{
  d = new KPropsDlgPluginPrivate;
  properties = _props;
  fontHeight = 2*properties->fontMetrics().height();
  d->m_bDirty = false;
}

KPropsDlgPlugin::~KPropsDlgPlugin()
{
  delete d;
}

bool KPropsDlgPlugin::isDesktopFile( KFileItem * _item )
{
  // only local files
  bool isLocal;
  KURL url = _item->mostLocalURL( isLocal );
  if ( !isLocal )
    return false;

  // only regular files
  if ( !S_ISREG( _item->mode() ) )
    return false;

  TQString t( url.path() );

  // only if readable
  FILE *f = fopen( TQFile::encodeName(t), "r" );
  if ( f == 0L )
    return false;
  fclose(f);

  // return true if desktop file
  return ( (_item->mimetype() == "application/x-desktop")
      || (_item->mimetype() == "media/builtin-mydocuments")
      || (_item->mimetype() == "media/builtin-mycomputer")
      || (_item->mimetype() == "media/builtin-mynetworkplaces")
      || (_item->mimetype() == "media/builtin-printers")
      || (_item->mimetype() == "media/builtin-trash")
      || (_item->mimetype() == "media/builtin-webbrowser") );
}

void KPropsDlgPlugin::setDirty( bool b )
{
  d->m_bDirty = b;
}

void KPropsDlgPlugin::setDirty()
{
  d->m_bDirty = true;
}

bool KPropsDlgPlugin::isDirty() const
{
  return d->m_bDirty;
}

void KPropsDlgPlugin::applyChanges()
{
  kdWarning(250) << "applyChanges() not implemented in page !" << endl;
}

///////////////////////////////////////////////////////////////////////////////

class KFilePropsPlugin::KFilePropsPluginPrivate
{
public:
  KFilePropsPluginPrivate()
  {
    dirSizeJob = 0L;
    dirSizeUpdateTimer = 0L;
    m_lined = 0;
    m_freeSpaceLabel = 0;
  }
  ~KFilePropsPluginPrivate()
  {
    if ( dirSizeJob )
      dirSizeJob->kill();
  }

  KDirSize * dirSizeJob;
  TQTimer *dirSizeUpdateTimer;
  TQFrame *m_frame;
  bool bMultiple;
  bool bIconChanged;
  bool bKDesktopMode;
  bool bDesktopFile;
  TQLabel *m_freeSpaceLabel;
  TQString mimeType;
  TQString oldFileName;
  KLineEdit* m_lined;
};

KFilePropsPlugin::KFilePropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KFilePropsPluginPrivate;
  d->bMultiple = (properties->items().count() > 1);
  d->bIconChanged = false;
  d->bKDesktopMode = (TQCString(tqApp->name()) == "kdesktop"); // nasty heh?
  d->bDesktopFile = KDesktopPropsPlugin::supports(properties->items());
  kdDebug(250) << "KFilePropsPlugin::KFilePropsPlugin bMultiple=" << d->bMultiple << endl;

  // We set this data from the first item, and we'll
  // check that the other items match against it, resetting when not.
  bool isLocal;
  KFileItem * item = properties->item();
  KURL url = item->mostLocalURL( isLocal );
  bool isReallyLocal = item->url().isLocalFile();
  bool bDesktopFile = isDesktopFile(item);
  kdDebug() << "url=" << url << " bDesktopFile=" << bDesktopFile << " isLocal=" << isLocal << " isReallyLocal=" << isReallyLocal << endl;
  mode_t mode = item->mode();
  bool hasDirs = item->isDir() && !item->isLink();
  bool hasRoot = url.path() == TQString::fromLatin1("/");
  TQString iconStr = KMimeType::iconForURL(url, mode);
  TQString directory = properties->kurl().directory();
  TQString protocol = properties->kurl().protocol();
  TQString mimeComment = item->mimeComment();
  d->mimeType = item->mimetype();
  bool hasTotalSize;
  TDEIO::filesize_t totalSize = item->size(hasTotalSize);
  TQString magicMimeComment;
  if ( isLocal ) {
      KMimeType::Ptr magicMimeType = KMimeType::findByFileContent( url.path() );
      if ( magicMimeType->name() != KMimeType::defaultMimeType() ) {
          magicMimeComment = magicMimeType->comment();
      }
  }

  // Those things only apply to 'single file' mode
  TQString filename = TQString::null;
  bool isTrash = false;
  bool isDevice = false;
  bool isMediaNode = false;
  m_bFromTemplate = false;

  // And those only to 'multiple' mode
  uint iDirCount = hasDirs ? 1 : 0;
  uint iFileCount = 1-iDirCount;

  d->m_frame = properties->addPage (i18n("&General"));

  TQVBoxLayout *vbl = new TQVBoxLayout( d->m_frame, 0,
                                      KDialog::spacingHint(), "vbl");
  TQGridLayout *grid = new TQGridLayout(0, 3); // unknown rows
  grid->setColStretch(0, 0);
  grid->setColStretch(1, 0);
  grid->setColStretch(2, 1);
  grid->addColSpacing(1, KDialog::spacingHint());
  vbl->addLayout(grid);
  int curRow = 0;

  if ( !d->bMultiple )
  {
    TQString path;
    if ( !m_bFromTemplate ) {
      isTrash = ( properties->kurl().protocol().find( "trash", 0, false)==0 );
      if ( properties->kurl().protocol().find("device", 0, false)==0) {
        isDevice = true;
      }
      if (d->mimeType.startsWith("media/")) {
        isMediaNode = true;
      }
      // Extract the full name, but without file: for local files
      if ( isReallyLocal ) {
        path = properties->kurl().path();
      }
      else {
        path = properties->kurl().prettyURL();
      }
    } else {
      path = properties->currentDir().path(1) + properties->defaultName();
      directory = properties->currentDir().prettyURL();
    }

    if (KExecPropsPlugin::supports(properties->items()) || // KDE4 remove me
        d->bDesktopFile ||
        KBindingPropsPlugin::supports(properties->items())) {
      determineRelativePath( path );
    }

    // Extract the file name only
    filename = properties->defaultName();
    if ( filename.isEmpty() ) { // no template
      filename = item->name(); // this gives support for UDS_NAME, e.g. for tdeio_trash or tdeio_system
    } else {
      m_bFromTemplate = true;
      setDirty(); // to enforce that the copy happens
    }
    d->oldFileName = filename;

    // Make it human-readable
    filename = nameFromFileName( filename );

    if ( d->bKDesktopMode && d->bDesktopFile ) {
        KDesktopFile config( url.path(), true /* readonly */ );
        if ( config.hasKey( "Name" ) ) {
            filename = config.readName();
        }
    }

    oldName = filename;
  }
  else
  {
    // Multiple items: see what they have in common
    KFileItemList items = properties->items();
    KFileItemListIterator it( items );
    for ( ++it /*no need to check the first one again*/ ; it.current(); ++it )
    {
      KURL url = (*it)->url();
      kdDebug(250) << "KFilePropsPlugin::KFilePropsPlugin " << url.prettyURL() << endl;
      // The list of things we check here should match the variables defined
      // at the beginning of this method.
      if ( url.isLocalFile() != isLocal )
        isLocal = false; // not all local
      if ( bDesktopFile && isDesktopFile(*it) != bDesktopFile )
        bDesktopFile = false; // not all desktop files
      if ( (*it)->mode() != mode )
        mode = (mode_t)0;
      if ( KMimeType::iconForURL(url, mode) != iconStr )
        iconStr = "application-vnd.tde.tdemultiple";
      if ( url.directory() != directory )
        directory = TQString::null;
      if ( url.protocol() != protocol )
        protocol = TQString::null;
      if ( !mimeComment.isNull() && (*it)->mimeComment() != mimeComment )
        mimeComment = TQString::null;
      if ( isLocal && !magicMimeComment.isNull() ) {
          KMimeType::Ptr magicMimeType = KMimeType::findByFileContent( url.path() );
          if ( magicMimeType->comment() != magicMimeComment )
              magicMimeComment = TQString::null;
      }

      if ( url.path() == TQString::fromLatin1("/") )
        hasRoot = true;
      if ( (*it)->isDir() && !(*it)->isLink() )
      {
        iDirCount++;
        hasDirs = true;
      }
      else
      {
        iFileCount++;
	bool hasSize;
        totalSize += (*it)->size(hasSize);
	hasTotalSize = hasTotalSize || hasSize;
      }
    }
  }

  if (!isReallyLocal && !protocol.isEmpty())
  {
    directory += ' ';
    directory += '(';
    directory += protocol;
    directory += ')';
  }

  if ( !isDevice && !isMediaNode && !isTrash && (bDesktopFile || S_ISDIR(mode)) && !d->bMultiple /*not implemented for multiple*/ )
  {
    TDEIconButton *iconButton = new TDEIconButton( d->m_frame );
    int bsize = 66 + 2 * iconButton->style().pixelMetric(TQStyle::PM_ButtonMargin);
    iconButton->setFixedSize(bsize, bsize);
    iconButton->setIconSize(48);
    iconButton->setStrictIconSize(false);
    // This works for everything except Device icons on unmounted devices
    // So we have to really open .desktop files
    TQString iconStr = KMimeType::findByURL( url, mode )->icon( url, isLocal );
    if ( bDesktopFile && isLocal ) {
      KDesktopFile config( url.path(), true );
      config.setDesktopGroup();
      iconStr = config.readEntry( "Icon" );
      if ( config.hasDeviceType() ) {
	iconButton->setIconType( TDEIcon::Desktop, TDEIcon::Device );
      }
      else {
	iconButton->setIconType( TDEIcon::Desktop, TDEIcon::Application );
      }
    }
    else {
      iconButton->setIconType( TDEIcon::Desktop, TDEIcon::Place );
    }
    iconButton->setIcon(iconStr);
    iconArea = iconButton;
    connect( iconButton, TQ_SIGNAL( iconChanged(TQString) ),
             this, TQ_SLOT( slotIconChanged() ) );
  } else {
    TQLabel *iconLabel = new TQLabel( d->m_frame );
    int bsize = 66 + 2 * iconLabel->style().pixelMetric(TQStyle::PM_ButtonMargin);
    iconLabel->setFixedSize(bsize, bsize);
    if (isMediaNode) {
      // Display the correct device icon
      iconLabel->setPixmap( TDEGlobal::iconLoader()->loadIcon( item->iconName(), TDEIcon::Desktop, 48) );
    }
    else {
      // Display the generic folder icon
      iconLabel->setPixmap( TDEGlobal::iconLoader()->loadIcon( iconStr, TDEIcon::Desktop, 48) );
    }
    iconArea = iconLabel;
  }
  grid->addWidget(iconArea, curRow, 0, TQt::AlignLeft);

  if (d->bMultiple || isTrash || isDevice || isMediaNode || hasRoot)
  {
    TQLabel *lab = new TQLabel(d->m_frame );
    if ( d->bMultiple )
      lab->setText( TDEIO::itemsSummaryString( iFileCount + iDirCount, iFileCount, iDirCount, 0, false ) );
    else
      lab->setText( filename );
    nameArea = lab;
  } else
  {
    d->m_lined = new KLineEdit( d->m_frame );
    d->m_lined->setText(filename);
    nameArea = d->m_lined;
    d->m_lined->setFocus();

    // Enhanced rename: Don't highlight the file extension.
    TQString pattern;
    KServiceTypeFactory::self()->findFromPattern( filename, &pattern );
    if (!pattern.isEmpty() && pattern.at(0)=='*' && pattern.find('*',1)==-1)
      d->m_lined->setSelection(0, filename.length()-pattern.stripWhiteSpace().length()+1);
    else
    {
      int lastDot = filename.findRev('.');
      if (lastDot > 0)
        d->m_lined->setSelection(0, lastDot);
    }

    connect( d->m_lined, TQ_SIGNAL( textChanged( const TQString & ) ),
             this, TQ_SLOT( nameFileChanged(const TQString & ) ) );
  }

  grid->addWidget(nameArea, curRow++, 2);

  KSeparator* sep = new KSeparator( KSeparator::HLine, d->m_frame);
  grid->addMultiCellWidget(sep, curRow, curRow, 0, 2);
  ++curRow;

  TQLabel *l;
  if ( !mimeComment.isEmpty() && !isDevice && !isMediaNode && !isTrash)
  {
    l = new TQLabel(i18n("Type:"), d->m_frame );

    grid->addWidget(l, curRow, 0);

    TQHBox *box = new TQHBox(d->m_frame);
    box->setSpacing(20);
    l = new TQLabel(mimeComment, box );

#ifdef TQ_WS_X11
    //TODO: wrap for win32 or mac?
    TQPushButton *button = new TQPushButton(box);

    TQIconSet iconSet = SmallIconSet(TQString::fromLatin1("configure"));
    TQPixmap pixMap = iconSet.pixmap( TQIconSet::Small, TQIconSet::Normal );
    button->setIconSet( iconSet );
    button->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
    if ( d->mimeType == KMimeType::defaultMimeType() )
       TQToolTip::add(button, i18n("Create new file type"));
    else
       TQToolTip::add(button, i18n("Edit file type"));

    connect( button, TQ_SIGNAL( clicked() ), TQ_SLOT( slotEditFileType() ));

    if (!kapp->authorizeTDEAction("editfiletype"))
       button->hide();
#endif

    grid->addWidget(box, curRow++, 2);
  }

  if ( !magicMimeComment.isEmpty() && magicMimeComment != mimeComment )
  {
    l = new TQLabel(i18n("Contents:"), d->m_frame );
    grid->addWidget(l, curRow, 0);

    l = new TQLabel(magicMimeComment, d->m_frame );
    grid->addWidget(l, curRow++, 2);
  }

  if ( !directory.isEmpty() )
  {
    l = new TQLabel( i18n("Location:"), d->m_frame );
    grid->addWidget(l, curRow, 0);

    l = new KSqueezedTextLabel( d->m_frame );
    l->setText( directory );
    grid->addWidget(l, curRow++, 2);
  }

  if( hasDirs || hasTotalSize ) {
    l = new TQLabel(i18n("Size:"), d->m_frame );
    grid->addWidget(l, curRow, 0);

    m_sizeLabel = new TQLabel( d->m_frame );
    grid->addWidget( m_sizeLabel, curRow++, 2 );
  } else {
    m_sizeLabel = 0;
  }

  if ( !hasDirs ) // Only files [and symlinks]
  {
    if(hasTotalSize) {
      m_sizeLabel->setText(TDEIO::convertSizeWithBytes(totalSize));
    }

    m_sizeDetermineButton = 0L;
    m_sizeStopButton = 0L;
  }
  else // Directory
  {
    TQHBoxLayout * sizelay = new TQHBoxLayout(KDialog::spacingHint());
    grid->addLayout( sizelay, curRow++, 2 );

    // buttons
    m_sizeDetermineButton = new TQPushButton( i18n("Calculate"), d->m_frame );
    m_sizeStopButton = new TQPushButton( i18n("Stop"), d->m_frame );
    connect( m_sizeDetermineButton, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotSizeDetermine() ) );
    connect( m_sizeStopButton, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotSizeStop() ) );
    sizelay->addWidget(m_sizeDetermineButton, 0);
    sizelay->addWidget(m_sizeStopButton, 0);
    sizelay->addStretch(10); // so that the buttons don't grow horizontally

    // auto-launch for local dirs only, but not for '/' or medias
    if ( isReallyLocal && !hasRoot )
    {
      m_sizeDetermineButton->setText( i18n("Refresh") );
      slotSizeDetermine();
    }
    else
      m_sizeStopButton->setEnabled( false );
  }

  if (!d->bMultiple && item->isLink()) {
    l = new TQLabel(i18n("Points to:"), d->m_frame );
    grid->addWidget(l, curRow, 0);

    l = new KSqueezedTextLabel(item->linkDest(), d->m_frame );
    grid->addWidget(l, curRow++, 2);
  }

  if (!d->bMultiple) // Dates for multiple don't make much sense...
  {
    TQDateTime dt;
    bool hasTime;
    time_t tim = item->time(TDEIO::UDS_CREATION_TIME, hasTime);
    if ( hasTime )
    {
      l = new TQLabel(i18n("Created:"), d->m_frame );
      grid->addWidget(l, curRow, 0);

      dt.setTime_t( tim );
      l = new TQLabel(TDEGlobal::locale()->formatDateTime(dt), d->m_frame );
      grid->addWidget(l, curRow++, 2);
    }

    tim = item->time(TDEIO::UDS_MODIFICATION_TIME, hasTime);
    if ( hasTime )
    {
      l = new TQLabel(i18n("Modified:"), d->m_frame );
      grid->addWidget(l, curRow, 0);

      dt.setTime_t( tim );
      l = new TQLabel(TDEGlobal::locale()->formatDateTime(dt), d->m_frame );
      grid->addWidget(l, curRow++, 2);
    }

    tim = item->time(TDEIO::UDS_ACCESS_TIME, hasTime);
    if ( hasTime )
    {
      l = new TQLabel(i18n("Accessed:"), d->m_frame );
      grid->addWidget(l, curRow, 0);

      dt.setTime_t( tim );
      l = new TQLabel(TDEGlobal::locale()->formatDateTime(dt), d->m_frame );
      grid->addWidget(l, curRow++, 2);
    }
  }

  if ( isLocal && hasDirs )  // only for directories
  {
    sep = new KSeparator( KSeparator::HLine, d->m_frame);
    grid->addMultiCellWidget(sep, curRow, curRow, 0, 2);
    ++curRow;

    TQString mountPoint = TDEIO::findPathMountPoint( url.path() );

    if (mountPoint != "/")
    {
        l = new TQLabel(i18n("Mounted on:"), d->m_frame );
        grid->addWidget(l, curRow, 0);

        l = new KSqueezedTextLabel( mountPoint, d->m_frame );
        grid->addWidget( l, curRow++, 2 );
    }

    l = new TQLabel(i18n("Free disk space:"), d->m_frame );
    grid->addWidget(l, curRow, 0);

    d->m_freeSpaceLabel = new TQLabel( d->m_frame );
    grid->addWidget( d->m_freeSpaceLabel, curRow++, 2 );

    KDiskFreeSp * job = new KDiskFreeSp;
    connect( job, TQ_SIGNAL( foundMountPoint( const unsigned long&, const unsigned long&,
             const unsigned long&, const TQString& ) ),
             this, TQ_SLOT( slotFoundMountPoint( const unsigned long&, const unsigned long&,
          const unsigned long&, const TQString& ) ) );
    job->readDF( mountPoint );
  }

  vbl->addStretch(1);
}

// TQString KFilePropsPlugin::tabName () const
// {
//   return i18n ("&General");
// }

void KFilePropsPlugin::setFileNameReadOnly( bool ro )
{
  if ( d->m_lined )
  {
    d->m_lined->setReadOnly( ro );
    if (ro)
    {
       // Don't put the initial focus on the line edit when it is ro
       TQPushButton *button = properties->actionButton(KDialogBase::Ok);
       if (button)
          button->setFocus();
    }
  }
}

void KFilePropsPlugin::slotEditFileType()
{
#ifdef TQ_WS_X11
  TQString mime;
  if ( d->mimeType == KMimeType::defaultMimeType() ) {
    int pos = d->oldFileName.findRev( '.' );
    if ( pos != -1 )
	mime = "*" + d->oldFileName.mid(pos);
    else
	mime = "*";
  }
  else
    mime = d->mimeType;
    //TODO: wrap for win32 or mac?
  TQString keditfiletype = TQString::fromLatin1("keditfiletype");
  KRun::runCommand( keditfiletype
                    + " --parent " + TQString::number( (ulong)properties->topLevelWidget()->winId())
                    + " " + TDEProcess::quote(mime),
                    keditfiletype, keditfiletype /*unused*/);
#endif
}

void KFilePropsPlugin::slotIconChanged()
{
  d->bIconChanged = true;
  emit changed();
}

void KFilePropsPlugin::nameFileChanged(const TQString &text )
{
  properties->enableButtonOK(!text.isEmpty());
  emit changed();
}

void KFilePropsPlugin::determineRelativePath( const TQString & path )
{
    // now let's make it relative
    TQStringList dirs;
    if (KBindingPropsPlugin::supports(properties->items()))
    {
       m_sRelativePath =TDEGlobal::dirs()->relativeLocation("mime", path);
       if (m_sRelativePath.startsWith("/"))
          m_sRelativePath = TQString::null;
    }
    else
    {
       m_sRelativePath =TDEGlobal::dirs()->relativeLocation("apps", path);
       if (m_sRelativePath.startsWith("/"))
       {
          m_sRelativePath =TDEGlobal::dirs()->relativeLocation("xdgdata-apps", path);
          if (m_sRelativePath.startsWith("/"))
             m_sRelativePath = TQString::null;
          else
             m_sRelativePath = path;
       }
    }
    if ( m_sRelativePath.isEmpty() )
    {
      if (KBindingPropsPlugin::supports(properties->items()))
        kdWarning(250) << "Warning : editing a mimetype file out of the mimetype dirs!" << endl;
    }
}

void KFilePropsPlugin::slotFoundMountPoint( const TQString&,
					    unsigned long kBSize,
					    unsigned long /*kBUsed*/,
					    unsigned long kBAvail )
{
    d->m_freeSpaceLabel->setText(
	// xgettext:no-c-format  --  Don't warn about translating the %1 out of %2 part.
	i18n("Available space out of total partition size (percent used)", "%1 out of %2 (%3% used)")
	.arg(TDEIO::convertSizeFromKB(kBAvail))
	.arg(TDEIO::convertSizeFromKB(kBSize))
	.arg( 100 - (int)(100.0 * kBAvail / kBSize) ));
}

// attention: copy&paste below, due to compiler bug
// it doesn't like those unsigned long parameters -- unsigned long& are ok :-/
void KFilePropsPlugin::slotFoundMountPoint( const unsigned long& kBSize,
					    const unsigned long& /*kBUsed*/,
					    const unsigned long& kBAvail,
					    const TQString& )
{
    d->m_freeSpaceLabel->setText(
	// xgettext:no-c-format  --  Don't warn about translating the %1 out of %2 part.
	i18n("Available space out of total partition size (percent used)", "%1 out of %2 (%3% used)")
	.arg(TDEIO::convertSizeFromKB(kBAvail))
	.arg(TDEIO::convertSizeFromKB(kBSize))
	.arg( 100 - (int)(100.0 * kBAvail / kBSize) ));
}

void KFilePropsPlugin::slotDirSizeUpdate()
{
    TDEIO::filesize_t totalSize = d->dirSizeJob->totalSize();
    TDEIO::filesize_t totalFiles = d->dirSizeJob->totalFiles();
         TDEIO::filesize_t totalSubdirs = d->dirSizeJob->totalSubdirs();
    m_sizeLabel->setText( i18n("Calculating... %1 (%2)\n%3, %4")
			  .arg(TDEIO::convertSize(totalSize))
                         .arg(TDEGlobal::locale()->formatNumber(totalSize, 0))
        .arg(i18n("1 file","%n files",totalFiles))
        .arg(i18n("1 sub-folder","%n sub-folders",totalSubdirs)));
}

void KFilePropsPlugin::slotDirSizeFinished( TDEIO::Job * job )
{
  if (job->error())
    m_sizeLabel->setText( job->errorString() );
  else
  {
    TDEIO::filesize_t totalSize = static_cast<KDirSize*>(job)->totalSize();
	TDEIO::filesize_t totalFiles = static_cast<KDirSize*>(job)->totalFiles();
	TDEIO::filesize_t totalSubdirs = static_cast<KDirSize*>(job)->totalSubdirs();
    m_sizeLabel->setText( TQString::fromLatin1("%1 (%2)\n%3, %4")
			  .arg(TDEIO::convertSize(totalSize))
			  .arg(TDEGlobal::locale()->formatNumber(totalSize, 0))
        .arg(i18n("1 file","%n files",totalFiles))
        .arg(i18n("1 sub-folder","%n sub-folders",totalSubdirs)));
  }
  m_sizeStopButton->setEnabled(false);
  // just in case you change something and try again :)
  m_sizeDetermineButton->setText( i18n("Refresh") );
  m_sizeDetermineButton->setEnabled(true);
  d->dirSizeJob = 0L;
  delete d->dirSizeUpdateTimer;
  d->dirSizeUpdateTimer = 0L;
}

void KFilePropsPlugin::slotSizeDetermine()
{
  m_sizeLabel->setText( i18n("Calculating...") );
  kdDebug(250) << " KFilePropsPlugin::slotSizeDetermine() properties->item()=" <<  properties->item() << endl;
  kdDebug(250) << " URL=" << properties->item()->url().url() << endl;
  d->dirSizeJob = KDirSize::dirSizeJob( properties->items() );
  d->dirSizeUpdateTimer = new TQTimer(this);
  connect( d->dirSizeUpdateTimer, TQ_SIGNAL( timeout() ),
           TQ_SLOT( slotDirSizeUpdate() ) );
  d->dirSizeUpdateTimer->start(500);
  connect( d->dirSizeJob, TQ_SIGNAL( result( TDEIO::Job * ) ),
           TQ_SLOT( slotDirSizeFinished( TDEIO::Job * ) ) );
  m_sizeStopButton->setEnabled(true);
  m_sizeDetermineButton->setEnabled(false);

  // also update the "Free disk space" display
  if ( d->m_freeSpaceLabel )
  {
    bool isLocal;
    KFileItem * item = properties->item();
    KURL url = item->mostLocalURL( isLocal );
    TQString mountPoint = TDEIO::findPathMountPoint( url.path() );

    KDiskFreeSp * job = new KDiskFreeSp;
    connect( job, TQ_SIGNAL( foundMountPoint( const unsigned long&, const unsigned long&,
             const unsigned long&, const TQString& ) ),
             this, TQ_SLOT( slotFoundMountPoint( const unsigned long&, const unsigned long&,
          const unsigned long&, const TQString& ) ) );
    job->readDF( mountPoint );
  }
}

void KFilePropsPlugin::slotSizeStop()
{
  if ( d->dirSizeJob )
  {
    m_sizeLabel->setText( i18n("Stopped") );
    d->dirSizeJob->kill();
    d->dirSizeJob = 0;
  }
  if ( d->dirSizeUpdateTimer )
    d->dirSizeUpdateTimer->stop();

  m_sizeStopButton->setEnabled(false);
  m_sizeDetermineButton->setEnabled(true);
}

KFilePropsPlugin::~KFilePropsPlugin()
{
  delete d;
}

bool KFilePropsPlugin::supports( KFileItemList /*_items*/ )
{
  return true;
}

// Don't do this at home
void tqt_enter_modal( TQWidget *widget );
void tqt_leave_modal( TQWidget *widget );

void KFilePropsPlugin::applyChanges()
{
  if ( d->dirSizeJob ) {
    slotSizeStop();
  }

  kdDebug(250) << "KFilePropsPlugin::applyChanges" << endl;

  if (nameArea->inherits("TQLineEdit"))
  {
    TQString n = ((TQLineEdit *) nameArea)->text();
    // Remove trailing spaces (#4345)
    while ( n[n.length()-1].isSpace() )
      n.truncate( n.length() - 1 );
    if ( n.isEmpty() )
    {
      KMessageBox::sorry( properties, i18n("The new file name is empty."));
      properties->abortApplying();
      return;
    }

    // Do we need to rename the file ?
    kdDebug(250) << "oldname = " << oldName << endl;
    kdDebug(250) << "newname = " << n << endl;
    if ( oldName != n || m_bFromTemplate ) { // true for any from-template file
      TDEIO::Job * job = 0L;
      KURL oldurl = properties->kurl();

      TQString newFileName = TDEIO::encodeFileName(n);
      if (d->bDesktopFile && !newFileName.endsWith(".desktop") && !newFileName.endsWith(".kdelnk"))
         newFileName += ".desktop";

      // Tell properties. Warning, this changes the result of properties->kurl() !
      properties->rename( newFileName );

      // Update also relative path (for apps and mimetypes)
      if ( !m_sRelativePath.isEmpty() ) {
        determineRelativePath( properties->kurl().path() );
      }

      kdDebug(250) << "New URL = " << properties->kurl().url() << endl;
      kdDebug(250) << "old = " << oldurl.url() << endl;

      // Don't remove the template !!
      if ( !m_bFromTemplate ) { // (normal renaming)
        job = TDEIO::move( oldurl, properties->kurl() );
      }
      else { // Copying a template
        job = TDEIO::copy( oldurl, properties->kurl() );
      }

      connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ),
               TQ_SLOT( slotCopyFinished( TDEIO::Job * ) ) );
      connect( job, TQ_SIGNAL( renamed( TDEIO::Job *, const KURL &, const KURL & ) ),
               TQ_SLOT( slotFileRenamed( TDEIO::Job *, const KURL &, const KURL & ) ) );
      // wait for job
      TQWidget dummy(0,0,(WFlags)(WType_Dialog|WShowModal));
      tqt_enter_modal(&dummy);
      tqApp->enter_loop();
      tqt_leave_modal(&dummy);
      return;
    }
    properties->updateUrl(properties->kurl());
    // Update also relative path (for apps and mimetypes)
    if ( !m_sRelativePath.isEmpty() ) {
      determineRelativePath( properties->kurl().path() );
    }
  }

  // No job, keep going
  slotCopyFinished( 0L );
}

void KFilePropsPlugin::slotCopyFinished( TDEIO::Job * job )
{
  kdDebug(250) << "KFilePropsPlugin::slotCopyFinished" << endl;
  if (job)
  {
    // allow apply() to return
    tqApp->exit_loop();
    if ( job->error() )
    {
        job->showErrorDialog( d->m_frame );
        // Didn't work. Revert the URL to the old one
        properties->updateUrl( static_cast<TDEIO::CopyJob*>(job)->srcURLs().first() );
        properties->abortApplying(); // Don't apply the changes to the wrong file !
        return;
    }
  }

  assert( properties->item() );
  assert( !properties->item()->url().isEmpty() );

  // Save the file where we can -> usually in ~/.trinity/...
  if (KBindingPropsPlugin::supports(properties->items()) && !m_sRelativePath.isEmpty())
  {
    KURL newURL;
    newURL.setPath( locateLocal("mime", m_sRelativePath) );
    properties->updateUrl( newURL );
  }
  else if (d->bDesktopFile && !m_sRelativePath.isEmpty())
  {
    kdDebug(250) << "KFilePropsPlugin::slotCopyFinished " << m_sRelativePath << endl;
    KURL newURL;
    newURL.setPath( KDesktopFile::locateLocal(m_sRelativePath) );
    kdDebug(250) << "KFilePropsPlugin::slotCopyFinished path=" << newURL.path() << endl;
    properties->updateUrl( newURL );
  }

  if ( d->bKDesktopMode && d->bDesktopFile ) {
      // Renamed? Update Name field
      if ( d->oldFileName != properties->kurl().fileName() || m_bFromTemplate ) {
          KDesktopFile config( properties->kurl().path() );
          TQString nameStr = nameFromFileName(properties->kurl().fileName());
          config.writeEntry( "Name", nameStr );
          config.writeEntry( "Name", nameStr, true, false, true );
      }
  }
}

void KFilePropsPlugin::applyIconChanges()
{
  TDEIconButton *iconButton = ::tqt_cast<TDEIconButton *>( iconArea );
  if ( !iconButton || !d->bIconChanged )
    return;
  // handle icon changes - only local files (or pseudo-local) for now
  // TODO: Use KTempFile and TDEIO::file_copy with overwrite = true
  KURL url = properties->kurl();
  url = TDEIO::NetAccess::mostLocalURL( url, properties );
  if (url.isLocalFile()) {
    TQString path;

    if (S_ISDIR(properties->item()->mode()))
    {
      path = url.path(1) + TQString::fromLatin1(".directory");
      // don't call updateUrl because the other tabs (i.e. permissions)
      // apply to the directory, not the .directory file.
    }
    else
      path = url.path();

    // Get the default image
    TQString str = KMimeType::findByURL( url,
                                        properties->item()->mode(),
                                        true )->KServiceType::icon();
    // Is it another one than the default ?
    TQString sIcon;
    if ( str != iconButton->icon() )
      sIcon = iconButton->icon();
    // (otherwise write empty value)

    kdDebug(250) << "**" << path << "**" << endl;
    TQFile f( path );

    // If default icon and no .directory file -> don't create one
    if ( !sIcon.isEmpty() || f.exists() )
    {
        if ( !f.open( IO_ReadWrite ) ) {
          KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not "
				      "have sufficient access to write to <b>%1</b>.</qt>").arg(path));
          return;
        }
        f.close();

        KDesktopFile cfg(path);
        kdDebug(250) << "sIcon = " << (sIcon) << endl;
        kdDebug(250) << "str = " << (str) << endl;
        cfg.writeEntry( "Icon", sIcon );
        cfg.sync();
    }
  }
}

void KFilePropsPlugin::slotFileRenamed( TDEIO::Job *, const KURL &, const KURL & newUrl )
{
  // This is called in case of an existing local file during the copy/move operation,
  // if the user chooses Rename.
  properties->updateUrl( newUrl );
}

void KFilePropsPlugin::postApplyChanges()
{
  // Save the icon only after applying the permissions changes (#46192)
  applyIconChanges();

  KURL::List lst;
  KFileItemList items = properties->items();
  for ( KFileItemListIterator it( items ); it.current(); ++it )
    lst.append((*it)->url());
  KDirNotify_stub allDirNotify("*", "KDirNotify*");
  allDirNotify.FilesChanged( lst );
}

class KFilePermissionsPropsPlugin::KFilePermissionsPropsPluginPrivate
{
public:
  KFilePermissionsPropsPluginPrivate()
  {
  }
  ~KFilePermissionsPropsPluginPrivate()
  {
  }

  TQFrame *m_frame;
  TQCheckBox *cbRecursive;
  TQLabel *explanationLabel;
  TQComboBox *ownerPermCombo, *groupPermCombo, *othersPermCombo;
  TQCheckBox *extraCheckbox;
  mode_t partialPermissions;
  KFilePermissionsPropsPlugin::PermissionsMode pmode;
  bool canChangePermissions;
  bool isIrregular;
  bool hasExtendedACL;
  KACL extendedACL;
  KACL defaultACL;
  bool fileSystemSupportsACLs;
};

#define UniOwner    (S_IRUSR|S_IWUSR|S_IXUSR)
#define UniGroup    (S_IRGRP|S_IWGRP|S_IXGRP)
#define UniOthers   (S_IROTH|S_IWOTH|S_IXOTH)
#define UniRead     (S_IRUSR|S_IRGRP|S_IROTH)
#define UniWrite    (S_IWUSR|S_IWGRP|S_IWOTH)
#define UniExec     (S_IXUSR|S_IXGRP|S_IXOTH)
#define UniSpecial  (S_ISUID|S_ISGID|S_ISVTX)

// synced with PermissionsTarget
const mode_t KFilePermissionsPropsPlugin::permissionsMasks[3] = {UniOwner, UniGroup, UniOthers};
const mode_t KFilePermissionsPropsPlugin::standardPermissions[4] = { 0, UniRead, UniRead|UniWrite, (mode_t)-1 };

// synced with PermissionsMode and standardPermissions
const char *KFilePermissionsPropsPlugin::permissionsTexts[4][4] = {
  { I18N_NOOP("Forbidden"),
    I18N_NOOP("Can Read"),
    I18N_NOOP("Can Read & Write"),
    0 },
  { I18N_NOOP("Forbidden"),
    I18N_NOOP("Can View Content"),
    I18N_NOOP("Can View & Modify Content"),
    0 },
  { 0, 0, 0, 0}, // no texts for links
  { I18N_NOOP("Forbidden"),
    I18N_NOOP("Can View Content & Read"),
    I18N_NOOP("Can View/Read & Modify/Write"),
    0 }
};


KFilePermissionsPropsPlugin::KFilePermissionsPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KFilePermissionsPropsPluginPrivate;
  d->cbRecursive = 0L;
  grpCombo = 0L; grpEdit = 0;
  usrEdit = 0L;
  TQString path = properties->kurl().path(-1);
  TQString fname = properties->kurl().fileName();
  bool isLocal = properties->kurl().isLocalFile();
  bool isTrash = ( properties->kurl().protocol().find("trash", 0, false)==0 );
  bool IamRoot = (geteuid() == 0);

  KFileItem * item = properties->item();
  bool isLink = item->isLink();
  bool isDir = item->isDir(); // all dirs
  bool hasDir = item->isDir(); // at least one dir
  permissions = item->permissions(); // common permissions to all files
  d->partialPermissions = permissions; // permissions that only some files have (at first we take everything)
  d->isIrregular = isIrregular(permissions, isDir, isLink);
  strOwner = item->user();
  strGroup = item->group();
  d->hasExtendedACL = item->ACL().isExtended() || item->defaultACL().isValid();
  d->extendedACL = item->ACL();
  d->defaultACL = item->defaultACL();
  d->fileSystemSupportsACLs = false;

  if ( properties->items().count() > 1 )
  {
    // Multiple items: see what they have in common
    KFileItemList items = properties->items();
    KFileItemListIterator it( items );
    for ( ++it /*no need to check the first one again*/ ; it.current(); ++it )
    {
      if (!d->isIrregular)
        d->isIrregular |= isIrregular((*it)->permissions(),
                                      (*it)->isDir() == isDir,
                                      (*it)->isLink() == isLink);
      d->hasExtendedACL = d->hasExtendedACL || (*it)->hasExtendedACL();
      if ( (*it)->isLink() != isLink )
        isLink = false;
      if ( (*it)->isDir() != isDir )
        isDir = false;
      hasDir |= (*it)->isDir();
      if ( (*it)->permissions() != permissions )
      {
        permissions &= (*it)->permissions();
        d->partialPermissions |= (*it)->permissions();
      }
      if ( (*it)->user() != strOwner )
        strOwner = TQString::null;
      if ( (*it)->group() != strGroup )
        strGroup = TQString::null;
    }
  }

  if (isLink)
    d->pmode = PermissionsOnlyLinks;
  else if (isDir)
    d->pmode = PermissionsOnlyDirs;
  else if (hasDir)
    d->pmode = PermissionsMixed;
  else
    d->pmode = PermissionsOnlyFiles;

  // keep only what's not in the common permissions
  d->partialPermissions = d->partialPermissions & ~permissions;

  bool isMyFile = false;

  if (isLocal && !strOwner.isEmpty()) { // local files, and all owned by the same person
    struct passwd *myself = getpwuid( geteuid() );
    if ( myself != 0L )
    {
      isMyFile = (strOwner == TQString::fromLocal8Bit(myself->pw_name));
    } else
      kdWarning() << "I don't exist ?! geteuid=" << geteuid() << endl;
  } else {
    //We don't know, for remote files, if they are ours or not.
    //So we let the user change permissions, and
    //TDEIO::chmod will tell, if he had no right to do it.
    isMyFile = true;
  }

  d->canChangePermissions = (isMyFile || IamRoot) && (!isLink);


  // create GUI

  d->m_frame = properties->addPage(i18n("&Permissions"));

  TQBoxLayout *box = new TQVBoxLayout( d->m_frame, 0, KDialog::spacingHint() );

  TQWidget *l;
  TQLabel *lbl;
  TQGroupBox *gb;
  TQGridLayout *gl;
  TQPushButton* pbAdvancedPerm = 0;

  /* Group: Access Permissions */
  gb = new TQGroupBox ( 0, TQt::Vertical, i18n("Access Permissions"), d->m_frame );
  gb->layout()->setSpacing(KDialog::spacingHint());
  gb->layout()->setMargin(KDialog::marginHint());
  box->addWidget (gb);

  gl = new TQGridLayout (gb->layout(), 7, 2);
  gl->setColStretch(1, 1);

  l = d->explanationLabel = new TQLabel( "", gb );
  if (isLink)
    d->explanationLabel->setText(i18n("This file is a link and does not have permissions.",
				      "All files are links and do not have permissions.",
				      properties->items().count()));
  else if (!d->canChangePermissions)
    d->explanationLabel->setText(i18n("Only the owner can change permissions."));
  gl->addMultiCellWidget(l, 0, 0, 0, 1);

  lbl = new TQLabel( i18n("O&wner:"), gb);
  gl->addWidget(lbl, 1, 0);
  l = d->ownerPermCombo = new TQComboBox(gb);
  lbl->setBuddy(l);
  gl->addWidget(l, 1, 1);
  connect(l, TQ_SIGNAL( highlighted(int) ), this, TQ_SIGNAL( changed() ));
  TQWhatsThis::add(l, i18n("Specifies the actions that the owner is allowed to do."));

  lbl = new TQLabel( i18n("Gro&up:"), gb);
  gl->addWidget(lbl, 2, 0);
  l = d->groupPermCombo = new TQComboBox(gb);
  lbl->setBuddy(l);
  gl->addWidget(l, 2, 1);
  connect(l, TQ_SIGNAL( highlighted(int) ), this, TQ_SIGNAL( changed() ));
  TQWhatsThis::add(l, i18n("Specifies the actions that the members of the group are allowed to do."));

  lbl = new TQLabel( i18n("O&thers:"), gb);
  gl->addWidget(lbl, 3, 0);
  l = d->othersPermCombo = new TQComboBox(gb);
  lbl->setBuddy(l);
  gl->addWidget(l, 3, 1);
  connect(l, TQ_SIGNAL( highlighted(int) ), this, TQ_SIGNAL( changed() ));
  TQWhatsThis::add(l, i18n("Specifies the actions that all users, who are neither "
			  "owner nor in the group, are allowed to do."));

  if (!isLink) {
    l = d->extraCheckbox = new TQCheckBox(hasDir ?
					 i18n("Only own&er can rename and delete folder content") :
					 i18n("Is &executable"),
					 gb );
    connect( d->extraCheckbox, TQ_SIGNAL( clicked() ), this, TQ_SIGNAL( changed() ) );
    gl->addWidget(l, 4, 1);
    TQWhatsThis::add(l, hasDir ? i18n("Enable this option to allow only the folder's owner to "
				     "delete or rename the contained files and folders. Other "
				     "users can only add new files, which requires the 'Modify "
				     "Content' permission.")
		    : i18n("Enable this option to mark the file as executable. This only makes "
			   "sense for programs and scripts. It is required when you want to "
			   "execute them."));

    TQLayoutItem *spacer = new TQSpacerItem(0, 20, TQSizePolicy::Minimum, TQSizePolicy::Expanding);
    gl->addMultiCell(spacer, 5, 5, 0, 1);

    pbAdvancedPerm = new TQPushButton(i18n("A&dvanced Permissions"), gb);
    gl->addMultiCellWidget(pbAdvancedPerm, 6, 6, 0, 1, TQt::AlignRight);
    connect(pbAdvancedPerm, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotShowAdvancedPermissions() ));
  }
  else
    d->extraCheckbox = 0;


  /**** Group: Ownership ****/
  gb = new TQGroupBox ( 0, TQt::Vertical, i18n("Ownership"), d->m_frame );
  gb->layout()->setSpacing(KDialog::spacingHint());
  gb->layout()->setMargin(KDialog::marginHint());
  box->addWidget (gb);

  gl = new TQGridLayout (gb->layout(), 4, 3);
  gl->addRowSpacing(0, 10);

  /*** Set Owner ***/
  l = new TQLabel( i18n("User:"), gb );
  gl->addWidget (l, 1, 0);

  /* GJ: Don't autocomplete more than 1000 users. This is a kind of random
   * value. Huge sites having 10.000+ user have a fair chance of using NIS,
   * (possibly) making this unacceptably slow.
   * OTOH, it is nice to offer this functionality for the standard user.
   */
  int i, maxEntries = 1000;
  struct passwd *user;

  /* File owner: For root, offer a KLineEdit with autocompletion.
   * For a user, who can never chown() a file, offer a TQLabel.
   */
  if (IamRoot && isLocal)
  {
    usrEdit = new KLineEdit( gb );
    TDECompletion *kcom = usrEdit->completionObject();
    kcom->setOrder(TDECompletion::Sorted);
    setpwent();
    for (i=0; ((user = getpwent()) != 0L) && (i < maxEntries); i++)
      kcom->addItem(TQString::fromLatin1(user->pw_name));
    endpwent();
    usrEdit->setCompletionMode((i < maxEntries) ? TDEGlobalSettings::CompletionAuto :
                               TDEGlobalSettings::CompletionNone);
    usrEdit->setText(strOwner);
    gl->addWidget(usrEdit, 1, 1);
    connect( usrEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
             this, TQ_SIGNAL( changed() ) );
  }
  else
  {
    l = new TQLabel(strOwner, gb);
    gl->addWidget(l, 1, 1);
  }

  /*** Set Group ***/

  TQStringList groupList;
  TQCString strUser;
  user = getpwuid(geteuid());
  if (user != 0L)
    strUser = user->pw_name;

#ifdef Q_OS_UNIX
  gid_t *groups = NULL;
  int ng = 1;
  struct group *mygroup;
  gid_t *newgroups = NULL;

  groups = (gid_t *) malloc(ng * sizeof(gid_t));

  if (getgrouplist(strUser, user->pw_gid, groups, &ng) == -1) {
	  newgroups = (gid_t *) malloc(ng * sizeof(gid_t));
	  if (newgroups != NULL) {
	          free(groups);
		  groups = newgroups;
		  getgrouplist(strUser, user->pw_gid, groups, &ng);
	  } else ng = 1;
  }

  for (i = 0; i < ng; i++) {
	  mygroup = getgrgid(groups[i]);
	  if (mygroup != NULL) groupList += TQString::fromLocal8Bit(mygroup->gr_name);
  }

  free(groups);

#else //Q_OS_UNIX
  struct group *ge;

  /* add the effective Group to the list .. */
  ge = getgrgid (getegid());
  if (ge) {
    TQString name = TQString::fromLatin1(ge->gr_name);
    if (name.isEmpty())
      name.setNum(ge->gr_gid);
    if (groupList.find(name) == groupList.end())
      groupList += name;
  }
#endif //Q_OS_UNIX

  bool isMyGroup = groupList.contains(strGroup);

  /* add the group the file currently belongs to ..
   * .. if its not there already
   */
  if (!isMyGroup)
    groupList += strGroup;

  l = new TQLabel( i18n("Group:"), gb );
  gl->addWidget (l, 2, 0);

  /* Set group: if possible to change:
   * - Offer a KLineEdit for root, since he can change to any group.
   * - Offer a TQComboBox for a normal user, since he can change to a fixed
   *   (small) set of groups only.
   * If not changeable: offer a TQLabel.
   */
  if (IamRoot && isLocal)
  {
    grpEdit = new KLineEdit(gb);
    TDECompletion *kcom = new TDECompletion;
    kcom->setItems(groupList);
    grpEdit->setCompletionObject(kcom, true);
    grpEdit->setAutoDeleteCompletionObject( true );
    grpEdit->setCompletionMode(TDEGlobalSettings::CompletionAuto);
    grpEdit->setText(strGroup);
    gl->addWidget(grpEdit, 2, 1);
    connect( grpEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
             this, TQ_SIGNAL( changed() ) );
  }
  else if ((groupList.count() > 1) && isMyFile && isLocal)
  {
    grpCombo = new TQComboBox(gb, "combogrouplist");
    grpCombo->insertStringList(groupList);
    grpCombo->setCurrentItem(groupList.findIndex(strGroup));
    gl->addWidget(grpCombo, 2, 1);
    connect( grpCombo, TQ_SIGNAL( activated( int ) ),
             this, TQ_SIGNAL( changed() ) );
  }
  else
  {
    l = new TQLabel(strGroup, gb);
    gl->addWidget(l, 2, 1);
  }

  gl->setColStretch(2, 10);

  // "Apply recursive" checkbox
  if ( hasDir && !isLink && !isTrash  )
  {
      d->cbRecursive = new TQCheckBox( i18n("Apply changes to all subfolders and their contents"), d->m_frame );
      connect( d->cbRecursive, TQ_SIGNAL( clicked() ), this, TQ_SIGNAL( changed() ) );
      box->addWidget( d->cbRecursive );
  }

  updateAccessControls();


  if ( isTrash || !d->canChangePermissions )
  {
      //don't allow to change properties for file into trash
      enableAccessControls(false);
      if ( pbAdvancedPerm  && !d->hasExtendedACL )
          pbAdvancedPerm->setEnabled(false);
  }

  box->addStretch (10);
}

#ifdef USE_POSIX_ACL
static bool fileSystemSupportsACL( const TQCString& pathCString )
{
    bool fileSystemSupportsACLs = false;
#ifdef Q_OS_FREEBSD
    struct statfs buf;
    fileSystemSupportsACLs = ( statfs( pathCString.data(), &buf ) == 0 ) && ( buf.f_flags & MNT_ACLS );
#else
    fileSystemSupportsACLs =
      getxattr( pathCString.data(), "system.posix_acl_access", NULL, 0 ) >= 0
#ifdef ENODATA
			|| (errno == ENODATA)
#endif
#ifdef ENOATTR
			|| (errno == ENOATTR)
#endif
			;
#endif
    return fileSystemSupportsACLs;
}
#endif


void KFilePermissionsPropsPlugin::slotShowAdvancedPermissions() {

  bool isDir = (d->pmode == PermissionsOnlyDirs) || (d->pmode == PermissionsMixed);
  KDialogBase dlg(properties, 0, true, i18n("Advanced Permissions"),
		  KDialogBase::Ok|KDialogBase::Cancel);

  TQLabel *l, *cl[3];
  TQGroupBox *gb;
  TQGridLayout *gl;

  TQVBox *mainVBox = dlg.makeVBoxMainWidget();

  // Group: Access Permissions
  gb = new TQGroupBox ( 0, TQt::Vertical, i18n("Access Permissions"), mainVBox );
  gb->layout()->setSpacing(KDialog::spacingHint());
  gb->layout()->setMargin(KDialog::marginHint());

  gl = new TQGridLayout (gb->layout(), 6, 6);
  gl->addRowSpacing(0, 10);

  TQValueVector<TQWidget*> theNotSpecials;

  l = new TQLabel(i18n("Class"), gb );
  gl->addWidget(l, 1, 0);
  theNotSpecials.append( l );

  if (isDir)
    l = new TQLabel( i18n("Show\nEntries"), gb );
  else
    l = new TQLabel( i18n("Read"), gb );
  gl->addWidget (l, 1, 1);
  theNotSpecials.append( l );
  TQString readWhatsThis;
  if (isDir)
    readWhatsThis = i18n("This flag allows viewing the content of the folder.");
  else
    readWhatsThis = i18n("The Read flag allows viewing the content of the file.");
  TQWhatsThis::add(l, readWhatsThis);

  if (isDir)
    l = new TQLabel( i18n("Write\nEntries"), gb );
  else
    l = new TQLabel( i18n("Write"), gb );
  gl->addWidget (l, 1, 2);
  theNotSpecials.append( l );
  TQString writeWhatsThis;
  if (isDir)
    writeWhatsThis = i18n("This flag allows adding, renaming and deleting of files. "
			  "Note that deleting and renaming can be limited using the Sticky flag.");
  else
    writeWhatsThis = i18n("The Write flag allows modifying the content of the file.");
  TQWhatsThis::add(l, writeWhatsThis);

  TQString execWhatsThis;
  if (isDir) {
    l = new TQLabel( i18n("Enter folder", "Enter"), gb );
    execWhatsThis = i18n("Enable this flag to allow entering the folder.");
  }
  else {
    l = new TQLabel( i18n("Exec"), gb );
    execWhatsThis = i18n("Enable this flag to allow executing the file as a program.");
  }
  TQWhatsThis::add(l, execWhatsThis);
  theNotSpecials.append( l );
  // GJ: Add space between normal and special modes
  TQSize size = l->sizeHint();
  size.setWidth(size.width() + 15);
  l->setFixedSize(size);
  gl->addWidget (l, 1, 3);

  l = new TQLabel( i18n("Special"), gb );
  gl->addMultiCellWidget(l, 1, 1, 4, 5);
  TQString specialWhatsThis;
  if (isDir)
    specialWhatsThis = i18n("Special flag. Valid for the whole folder, the exact "
			    "meaning of the flag can be seen in the right hand column.");
  else
    specialWhatsThis = i18n("Special flag. The exact meaning of the flag can be seen "
			    "in the right hand column.");
  TQWhatsThis::add(l, specialWhatsThis);

  cl[0] = new TQLabel( i18n("User"), gb );
  gl->addWidget (cl[0], 2, 0);
  theNotSpecials.append( cl[0] );

  cl[1] = new TQLabel( i18n("Group"), gb );
  gl->addWidget (cl[1], 3, 0);
  theNotSpecials.append( cl[1] );

  cl[2] = new TQLabel( i18n("Others"), gb );
  gl->addWidget (cl[2], 4, 0);
  theNotSpecials.append( cl[2] );

  l = new TQLabel(i18n("Set UID"), gb);
  gl->addWidget(l, 2, 5);
  TQString setUidWhatsThis;
  if (isDir)
    setUidWhatsThis = i18n("If this flag is set, the owner of this folder will be "
			   "the owner of all new files.");
  else
    setUidWhatsThis = i18n("If this file is an executable and the flag is set, it will "
			   "be executed with the permissions of the owner.");
  TQWhatsThis::add(l, setUidWhatsThis);

  l = new TQLabel(i18n("Set GID"), gb);
  gl->addWidget(l, 3, 5);
  TQString setGidWhatsThis;
  if (isDir)
    setGidWhatsThis = i18n("If this flag is set, the group of this folder will be "
			   "set for all new files.");
  else
    setGidWhatsThis = i18n("If this file is an executable and the flag is set, it will "
			   "be executed with the permissions of the group.");
  TQWhatsThis::add(l, setGidWhatsThis);

  l = new TQLabel(i18n("File permission", "Sticky"), gb);
  gl->addWidget(l, 4, 5);
  TQString stickyWhatsThis;
  if (isDir)
    stickyWhatsThis = i18n("If the Sticky flag is set on a folder, only the owner "
			   "and root can delete or rename files. Otherwise everybody "
			   "with write permissions can do this.");
  else
    stickyWhatsThis = i18n("The Sticky flag on a file is ignored on Linux, but may "
			   "be used on some systems");
  TQWhatsThis::add(l, stickyWhatsThis);

  mode_t aPermissions, aPartialPermissions;
  mode_t dummy1, dummy2;

  if (!d->isIrregular) {
    switch (d->pmode) {
    case PermissionsOnlyFiles:
      getPermissionMasks(aPartialPermissions,
			 dummy1,
			 aPermissions,
			 dummy2);
      break;
    case PermissionsOnlyDirs:
    case PermissionsMixed:
      getPermissionMasks(dummy1,
			 aPartialPermissions,
			 dummy2,
			 aPermissions);
      break;
    case PermissionsOnlyLinks:
      aPermissions = UniRead | UniWrite | UniExec | UniSpecial;
      aPartialPermissions = 0;
      break;
    }
  }
  else {
    aPermissions = permissions;
    aPartialPermissions = d->partialPermissions;
  }

  // Draw Checkboxes
  bool allDisable = true;
  TQCheckBox *cba[3][4];
  for (int row = 0; row < 3 ; ++row) {
    for (int col = 0; col < 4; ++col) {
      TQCheckBox *cb = new TQCheckBox( gb );
      if ( col != 3 ) theNotSpecials.append( cb );
      cba[row][col] = cb;
      cb->setChecked(aPermissions & fperm[row][col]);
      if ( d->canChangePermissions )
      {
        allDisable = false;
      }
      if ( aPartialPermissions & fperm[row][col] )
      {
        cb->setTristate();
        cb->setNoChange();
      }
      else if (d->cbRecursive && d->cbRecursive->isChecked())
      {
        cb->setTristate();
      }

      cb->setEnabled( d->canChangePermissions );
      gl->addWidget (cb, row+2, col+1);
      switch(col) {
      case 0:
	TQWhatsThis::add(cb, readWhatsThis);
	break;
      case 1:
	TQWhatsThis::add(cb, writeWhatsThis);
	break;
      case 2:
	TQWhatsThis::add(cb, execWhatsThis);
	break;
      case 3:
	switch(row) {
	case 0:
	  TQWhatsThis::add(cb, setUidWhatsThis);
	  break;
	case 1:
	  TQWhatsThis::add(cb, setGidWhatsThis);
	  break;
	case 2:
	  TQWhatsThis::add(cb, stickyWhatsThis);
	  break;
	}
	break;
      }
    }
  }
  gl->setColStretch(6, 10);

#ifdef USE_POSIX_ACL
  KACLEditWidget *extendedACLs = 0;

  // FIXME make it work with partial entries
  if ( properties->items().count() == 1 ) {
      TQCString pathCString = TQFile::encodeName( properties->item()->url().path() );
      d->fileSystemSupportsACLs = fileSystemSupportsACL( pathCString );
  }
  if ( d->fileSystemSupportsACLs  ) {
    std::for_each( theNotSpecials.begin(), theNotSpecials.end(), std::mem_fn( &TQWidget::hide ) );
    extendedACLs = new KACLEditWidget( mainVBox );
    if ( d->extendedACL.isValid() && d->extendedACL.isExtended() )
      extendedACLs->setACL( d->extendedACL );
    else
      extendedACLs->setACL( KACL( aPermissions ) );

    if ( d->defaultACL.isValid() )
      extendedACLs->setDefaultACL( d->defaultACL );

    if ( properties->items().first()->isDir() )
      extendedACLs->setAllowDefaults( true );
    if ( !d->canChangePermissions )
      extendedACLs->setReadOnly( true );

  }
#endif
  if ( allDisable ) {
    dlg.enableButtonOK( false );
  }

  if (dlg.exec() != KDialogBase::Accepted)
    return;

  mode_t andPermissions = mode_t(~0);
  mode_t orPermissions = 0;
  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 4; ++col) {
      switch (cba[row][col]->state())
      {
      case TQCheckBox::On:
	orPermissions |= fperm[row][col];
	//fall through
      case TQCheckBox::Off:
	andPermissions &= ~fperm[row][col];
	break;
      default: // NoChange
	break;
      }
    }

  d->isIrregular = false;
  KFileItemList items = properties->items();
  for (KFileItemListIterator it(items); it.current(); ++it) {
    if (isIrregular(((*it)->permissions() & andPermissions) | orPermissions,
		    (*it)->isDir(), (*it)->isLink())) {
      d->isIrregular = true;
      break;
    }
  }

  permissions = orPermissions;
  d->partialPermissions = andPermissions;

#ifdef USE_POSIX_ACL
  // override with the acls, if present
  if ( extendedACLs ) {
    d->extendedACL = extendedACLs->getACL();
    d->defaultACL = extendedACLs->getDefaultACL();
    d->hasExtendedACL = d->extendedACL.isExtended() || d->defaultACL.isValid();
    permissions = d->extendedACL.basePermissions();
    permissions |= ( andPermissions | orPermissions ) & ( S_ISUID|S_ISGID|S_ISVTX );
  }
#endif

  updateAccessControls();
  emit changed();
}

// TQString KFilePermissionsPropsPlugin::tabName () const
// {
//   return i18n ("&Permissions");
// }

KFilePermissionsPropsPlugin::~KFilePermissionsPropsPlugin()
{
  delete d;
}

bool KFilePermissionsPropsPlugin::supports( KFileItemList _items )
{
  KFileItemList::const_iterator it = _items.constBegin();
  for ( ; it != _items.constEnd(); ++it ) {
    KFileItem *item = *it;
    if( !item->user().isEmpty() || !item->group().isEmpty() )
      return true;
  }
  return false;
}

// sets a combo box in the Access Control frame
void KFilePermissionsPropsPlugin::setComboContent(TQComboBox *combo, PermissionsTarget target,
						  mode_t permissions, mode_t partial) {
  combo->clear();
  if (d->pmode == PermissionsOnlyLinks) {
    combo->insertItem(i18n("Link"));
    combo->setCurrentItem(0);
    return;
  }

  mode_t tMask = permissionsMasks[target];
  int textIndex;
  for (textIndex = 0; standardPermissions[textIndex] != (mode_t)-1; textIndex++)
    if ((standardPermissions[textIndex]&tMask) == (permissions&tMask&(UniRead|UniWrite)))
      break;
  Q_ASSERT(standardPermissions[textIndex] != (mode_t)-1); // must not happen, would be irreglar

  for (int i = 0; permissionsTexts[(int)d->pmode][i]; i++)
    combo->insertItem(i18n(permissionsTexts[(int)d->pmode][i]));

  if (partial & tMask & ~UniExec) {
    combo->insertItem(i18n("Varying (No Change)"));
    combo->setCurrentItem(3);
  }
  else
    combo->setCurrentItem(textIndex);
}

// permissions are irregular if they cant be displayed in a combo box.
bool KFilePermissionsPropsPlugin::isIrregular(mode_t permissions, bool isDir, bool isLink) {
  if (isLink)                             // links are always ok
    return false;

  mode_t p = permissions;
  if (p & (S_ISUID | S_ISGID))  // setuid/setgid -> irregular
    return true;
  if (isDir) {
    p &= ~S_ISVTX;          // ignore sticky on dirs

    // check supported flag combinations
    mode_t p0 = p & UniOwner;
    if ((p0 != 0) && (p0 != (S_IRUSR | S_IXUSR)) && (p0 != UniOwner))
      return true;
    p0 = p & UniGroup;
    if ((p0 != 0) && (p0 != (S_IRGRP | S_IXGRP)) && (p0 != UniGroup))
      return true;
    p0 = p & UniOthers;
    if ((p0 != 0) && (p0 != (S_IROTH | S_IXOTH)) && (p0 != UniOthers))
      return true;
    return false;
  }
  if (p & S_ISVTX) // sticky on file -> irregular
    return true;

  // check supported flag combinations
  mode_t p0 = p & UniOwner;
  bool usrXPossible = !p0; // true if this file could be an executable
  if (p0 & S_IXUSR) {
    if ((p0 == S_IXUSR) || (p0 == (S_IWUSR | S_IXUSR)))
      return true;
    usrXPossible = true;
  }
  else if (p0 == S_IWUSR)
    return true;

  p0 = p & UniGroup;
  bool grpXPossible = !p0; // true if this file could be an executable
  if (p0 & S_IXGRP) {
    if ((p0 == S_IXGRP) || (p0 == (S_IWGRP | S_IXGRP)))
      return true;
    grpXPossible = true;
  }
  else if (p0 == S_IWGRP)
    return true;
  if (p0 == 0)
    grpXPossible = true;

  p0 = p & UniOthers;
  bool othXPossible = !p0; // true if this file could be an executable
  if (p0 & S_IXOTH) {
    if ((p0 == S_IXOTH) || (p0 == (S_IWOTH | S_IXOTH)))
      return true;
    othXPossible = true;
  }
  else if (p0 == S_IWOTH)
    return true;

  // check that there either all targets are executable-compatible, or none
  return (p & UniExec) && !(usrXPossible && grpXPossible && othXPossible);
}

// enables/disabled the widgets in the Access Control frame
void KFilePermissionsPropsPlugin::enableAccessControls(bool enable) {
	d->ownerPermCombo->setEnabled(enable);
	d->groupPermCombo->setEnabled(enable);
	d->othersPermCombo->setEnabled(enable);
	if (d->extraCheckbox)
	  d->extraCheckbox->setEnabled(enable);
        if ( d->cbRecursive )
            d->cbRecursive->setEnabled(enable);
}

// updates all widgets in the Access Control frame
void KFilePermissionsPropsPlugin::updateAccessControls() {
  setComboContent(d->ownerPermCombo, PermissionsOwner,
		  permissions, d->partialPermissions);
  setComboContent(d->groupPermCombo, PermissionsGroup,
		  permissions, d->partialPermissions);
  setComboContent(d->othersPermCombo, PermissionsOthers,
		  permissions, d->partialPermissions);

  switch(d->pmode) {
  case PermissionsOnlyLinks:
    enableAccessControls(false);
    break;
  case PermissionsOnlyFiles:
    enableAccessControls(d->canChangePermissions && !d->isIrregular && !d->hasExtendedACL);
    if (d->canChangePermissions)
      d->explanationLabel->setText(d->isIrregular || d->hasExtendedACL ?
				   i18n("This file uses advanced permissions",
				      "These files use advanced permissions.",
				      properties->items().count()) : "");
    if (d->partialPermissions & UniExec) {
      d->extraCheckbox->setTristate();
      d->extraCheckbox->setNoChange();
    }
    else {
      d->extraCheckbox->setTristate(false);
      d->extraCheckbox->setChecked(permissions & UniExec);
    }
    break;
  case PermissionsOnlyDirs:
    enableAccessControls(d->canChangePermissions && !d->isIrregular && !d->hasExtendedACL);
    // if this is a dir, and we can change permissions, don't dis-allow
    // recursive, we can do that for ACL setting.
    if ( d->cbRecursive )
       d->cbRecursive->setEnabled( d->canChangePermissions && !d->isIrregular );

    if (d->canChangePermissions)
      d->explanationLabel->setText(d->isIrregular || d->hasExtendedACL ?
				   i18n("This folder uses advanced permissions.",
				      "These folders use advanced permissions.",
				      properties->items().count()) : "");
    if (d->partialPermissions & S_ISVTX) {
      d->extraCheckbox->setTristate();
      d->extraCheckbox->setNoChange();
    }
    else {
      d->extraCheckbox->setTristate(false);
      d->extraCheckbox->setChecked(permissions & S_ISVTX);
    }
    break;
  case PermissionsMixed:
    enableAccessControls(d->canChangePermissions && !d->isIrregular && !d->hasExtendedACL);
    if (d->canChangePermissions)
      d->explanationLabel->setText(d->isIrregular || d->hasExtendedACL ?
				   i18n("These files use advanced permissions.") : "");
    break;
    if (d->partialPermissions & S_ISVTX) {
      d->extraCheckbox->setTristate();
      d->extraCheckbox->setNoChange();
    }
    else {
      d->extraCheckbox->setTristate(false);
      d->extraCheckbox->setChecked(permissions & S_ISVTX);
    }
    break;
  }
}

// gets masks for files and dirs from the Access Control frame widgets
void KFilePermissionsPropsPlugin::getPermissionMasks(mode_t &andFilePermissions,
						     mode_t &andDirPermissions,
						     mode_t &orFilePermissions,
						     mode_t &orDirPermissions) {
  andFilePermissions = mode_t(~UniSpecial);
  andDirPermissions = mode_t(~(S_ISUID|S_ISGID));
  orFilePermissions = 0;
  orDirPermissions = 0;
  if (d->isIrregular)
    return;

  mode_t m = standardPermissions[d->ownerPermCombo->currentItem()];
  if (m != (mode_t) -1) {
    orFilePermissions |= m & UniOwner;
    if ((m & UniOwner) &&
	((d->pmode == PermissionsMixed) ||
	 ((d->pmode == PermissionsOnlyFiles) && (d->extraCheckbox->state() == TQButton::NoChange))))
      andFilePermissions &= ~(S_IRUSR | S_IWUSR);
    else {
      andFilePermissions &= ~(S_IRUSR | S_IWUSR | S_IXUSR);
      if ((m & S_IRUSR) && (d->extraCheckbox->state() == TQButton::On))
	orFilePermissions |= S_IXUSR;
    }

    orDirPermissions |= m & UniOwner;
    if (m & S_IRUSR)
	orDirPermissions |= S_IXUSR;
    andDirPermissions &= ~(S_IRUSR | S_IWUSR | S_IXUSR);
  }

  m = standardPermissions[d->groupPermCombo->currentItem()];
  if (m != (mode_t) -1) {
    orFilePermissions |= m & UniGroup;
    if ((m & UniGroup) &&
	((d->pmode == PermissionsMixed) ||
	 ((d->pmode == PermissionsOnlyFiles) && (d->extraCheckbox->state() == TQButton::NoChange))))
      andFilePermissions &= ~(S_IRGRP | S_IWGRP);
    else {
      andFilePermissions &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
      if ((m & S_IRGRP) && (d->extraCheckbox->state() == TQButton::On))
	orFilePermissions |= S_IXGRP;
    }

    orDirPermissions |= m & UniGroup;
    if (m & S_IRGRP)
	orDirPermissions |= S_IXGRP;
    andDirPermissions &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
  }

  m = standardPermissions[d->othersPermCombo->currentItem()];
  if (m != (mode_t) -1) {
    orFilePermissions |= m & UniOthers;
    if ((m & UniOthers) &&
	((d->pmode == PermissionsMixed) ||
	 ((d->pmode == PermissionsOnlyFiles) && (d->extraCheckbox->state() == TQButton::NoChange))))
      andFilePermissions &= ~(S_IROTH | S_IWOTH);
    else {
      andFilePermissions &= ~(S_IROTH | S_IWOTH | S_IXOTH);
      if ((m & S_IROTH) && (d->extraCheckbox->state() == TQButton::On))
	orFilePermissions |= S_IXOTH;
    }

    orDirPermissions |= m & UniOthers;
    if (m & S_IROTH)
	orDirPermissions |= S_IXOTH;
    andDirPermissions &= ~(S_IROTH | S_IWOTH | S_IXOTH);
  }

  if (((d->pmode == PermissionsMixed) || (d->pmode == PermissionsOnlyDirs)) &&
      (d->extraCheckbox->state() != TQButton::NoChange)) {
    andDirPermissions &= ~S_ISVTX;
    if (d->extraCheckbox->state() == TQButton::On)
      orDirPermissions |= S_ISVTX;
  }
}

void KFilePermissionsPropsPlugin::applyChanges()
{
  mode_t orFilePermissions;
  mode_t orDirPermissions;
  mode_t andFilePermissions;
  mode_t andDirPermissions;

  if (!d->canChangePermissions)
    return;

  if (!d->isIrregular)
    getPermissionMasks(andFilePermissions,
		       andDirPermissions,
		       orFilePermissions,
		       orDirPermissions);
  else {
    orFilePermissions = permissions;
    andFilePermissions = d->partialPermissions;
    orDirPermissions = permissions;
    andDirPermissions = d->partialPermissions;
  }

  TQString owner, group;
  if (usrEdit)
    owner = usrEdit->text();
  if (grpEdit)
    group = grpEdit->text();
  else if (grpCombo)
    group = grpCombo->currentText();

  if (owner == strOwner)
      owner = TQString::null; // no change

  if (group == strGroup)
      group = TQString::null;

  bool recursive = d->cbRecursive && d->cbRecursive->isChecked();
  bool permissionChange = false;

  KFileItemList files, dirs;
  KFileItemList items = properties->items();
  for (KFileItemListIterator it(items); it.current(); ++it) {
    if ((*it)->isDir()) {
      dirs.append(*it);
      if ((*it)->permissions() != (((*it)->permissions() & andDirPermissions) | orDirPermissions))
	permissionChange = true;
    }
    else if ((*it)->isFile()) {
      files.append(*it);
      if ((*it)->permissions() != (((*it)->permissions() & andFilePermissions) | orFilePermissions))
	permissionChange = true;
    }
  }

  const bool ACLChange = ( d->extendedACL !=  properties->item()->ACL() );
  const bool defaultACLChange = ( d->defaultACL != properties->item()->defaultACL() );

  if ( owner.isEmpty() && group.isEmpty() && !recursive
      && !permissionChange && !ACLChange && !defaultACLChange )
    return;

  TDEIO::Job * job;
  if (files.count() > 0) {
    job = TDEIO::chmod( files, orFilePermissions, ~andFilePermissions,
        owner, group, false );
    if ( ACLChange && d->fileSystemSupportsACLs )
      job->addMetaData( "ACL_STRING", d->extendedACL.isValid()?d->extendedACL.asString():"ACL_DELETE" );
    if ( defaultACLChange && d->fileSystemSupportsACLs )
      job->addMetaData( "DEFAULT_ACL_STRING", d->defaultACL.isValid()?d->defaultACL.asString():"ACL_DELETE" );

    connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ),
        TQ_SLOT( slotChmodResult( TDEIO::Job * ) ) );
    // Wait for job
    TQWidget dummy(0,0,(WFlags)(WType_Dialog|WShowModal));
    tqt_enter_modal(&dummy);
    tqApp->enter_loop();
    tqt_leave_modal(&dummy);
  }
  if (dirs.count() > 0) {
    job = TDEIO::chmod( dirs, orDirPermissions, ~andDirPermissions,
        owner, group, recursive );
    if ( ACLChange && d->fileSystemSupportsACLs )
      job->addMetaData( "ACL_STRING", d->extendedACL.isValid()?d->extendedACL.asString():"ACL_DELETE" );
    if ( defaultACLChange && d->fileSystemSupportsACLs )
      job->addMetaData( "DEFAULT_ACL_STRING", d->defaultACL.isValid()?d->defaultACL.asString():"ACL_DELETE" );

    connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ),
        TQ_SLOT( slotChmodResult( TDEIO::Job * ) ) );
    // Wait for job
    TQWidget dummy(0,0,(WFlags)(WType_Dialog|WShowModal));
    tqt_enter_modal(&dummy);
    tqApp->enter_loop();
    tqt_leave_modal(&dummy);
  }
}

void KFilePermissionsPropsPlugin::slotChmodResult( TDEIO::Job * job )
{
  kdDebug(250) << "KFilePermissionsPropsPlugin::slotChmodResult" << endl;
  if (job->error())
    job->showErrorDialog( d->m_frame );
  // allow apply() to return
  tqApp->exit_loop();
}




class KURLPropsPlugin::KURLPropsPluginPrivate
{
public:
  KURLPropsPluginPrivate()
  {
  }
  ~KURLPropsPluginPrivate()
  {
  }

  TQFrame *m_frame;
};

KURLPropsPlugin::KURLPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KURLPropsPluginPrivate;
  d->m_frame = properties->addPage(i18n("U&RL"));
  TQVBoxLayout *layout = new TQVBoxLayout(d->m_frame, 0, KDialog::spacingHint());

  TQLabel *l;
  l = new TQLabel( d->m_frame, "Label_1" );
  l->setText( i18n("URL:") );
  layout->addWidget(l);

  URLEdit = new KURLRequester( d->m_frame, "URL Requester" );
  layout->addWidget(URLEdit);

  TQString path = properties->kurl().path();

  TQFile f( path );
  if ( !f.open( IO_ReadOnly ) ) {
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  URLStr = config.readPathEntry( "URL" );

  KFileItem * item = properties->item();

  if (item && item->mimetype().startsWith("media/builtin-")) {
    URLEdit->setEnabled(false);
  }

  if ( !URLStr.isNull() ) {
    URLEdit->setURL( URLStr );
  }

  connect( URLEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );

  layout->addStretch (1);
}

KURLPropsPlugin::~KURLPropsPlugin()
{
  delete d;
}

// TQString KURLPropsPlugin::tabName () const
// {
//   return i18n ("U&RL");
// }

bool KURLPropsPlugin::supports( KFileItemList _items )
{
  if ( _items.count() != 1 )
    return false;
  KFileItem * item = _items.first();
  // check if desktop file
  if ( !KPropsDlgPlugin::isDesktopFile( item ) )
    return false;

  // open file and check type
  KDesktopFile config( item->url().path(), true /* readonly */ );
  return config.hasLinkType();
}

void KURLPropsPlugin::applyChanges()
{
  TQString path = properties->kurl().path();
  KFileItem * item = properties->item();

  if (item && item->mimetype().startsWith("media/builtin-")) {
    return;
  }

  TQFile f( path );
  if ( !f.open( IO_ReadWrite ) ) {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have "
				"sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", TQString::fromLatin1("Link"));
  config.writePathEntry( "URL", URLEdit->url() );
  // Users can't create a Link .desktop file with a Name field,
  // but distributions can. Update the Name field in that case.
  if ( config.hasKey("Name") )
  {
    TQString nameStr = nameFromFileName(properties->kurl().fileName());
    config.writeEntry( "Name", nameStr );
    config.writeEntry( "Name", nameStr, true, false, true );

  }
}


/* ----------------------------------------------------
 *
 * KBindingPropsPlugin
 *
 * -------------------------------------------------- */

class KBindingPropsPlugin::KBindingPropsPluginPrivate
{
public:
  KBindingPropsPluginPrivate()
  {
  }
  ~KBindingPropsPluginPrivate()
  {
  }

  TQFrame *m_frame;
};

KBindingPropsPlugin::KBindingPropsPlugin( KPropertiesDialog *_props ) : KPropsDlgPlugin( _props )
{
  d = new KBindingPropsPluginPrivate;
  d->m_frame = properties->addPage(i18n("A&ssociation"));
  patternEdit = new KLineEdit( d->m_frame, "LineEdit_1" );
  commentEdit = new KLineEdit( d->m_frame, "LineEdit_2" );
  mimeEdit = new KLineEdit( d->m_frame, "LineEdit_3" );

  TQBoxLayout *mainlayout = new TQVBoxLayout(d->m_frame, 0, KDialog::spacingHint());
  TQLabel* tmpQLabel;

  tmpQLabel = new TQLabel( d->m_frame, "Label_1" );
  tmpQLabel->setText(  i18n("Pattern ( example: *.html;*.htm )") );
  tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
  mainlayout->addWidget(tmpQLabel, 1);

  //patternEdit->setGeometry( 10, 40, 210, 30 );
  //patternEdit->setText( "" );
  patternEdit->setMaxLength( 512 );
  patternEdit->setMinimumSize( patternEdit->sizeHint() );
  patternEdit->setFixedHeight( fontHeight );
  mainlayout->addWidget(patternEdit, 1);

  tmpQLabel = new TQLabel( d->m_frame, "Label_2" );
  tmpQLabel->setText(  i18n("Mime Type") );
  tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
  mainlayout->addWidget(tmpQLabel, 1);

  //mimeEdit->setGeometry( 10, 160, 210, 30 );
  mimeEdit->setMaxLength( 256 );
  mimeEdit->setMinimumSize( mimeEdit->sizeHint() );
  mimeEdit->setFixedHeight( fontHeight );
  mainlayout->addWidget(mimeEdit, 1);

  tmpQLabel = new TQLabel( d->m_frame, "Label_3" );
  tmpQLabel->setText(  i18n("Comment") );
  tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
  mainlayout->addWidget(tmpQLabel, 1);

  //commentEdit->setGeometry( 10, 100, 210, 30 );
  commentEdit->setMaxLength( 256 );
  commentEdit->setMinimumSize( commentEdit->sizeHint() );
  commentEdit->setFixedHeight( fontHeight );
  mainlayout->addWidget(commentEdit, 1);

  cbAutoEmbed = new TQCheckBox( i18n("Left click previews"), d->m_frame, "cbAutoEmbed" );
  mainlayout->addWidget(cbAutoEmbed, 1);

  mainlayout->addStretch (10);
  mainlayout->activate();

  TQFile f( _props->kurl().path() );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KSimpleConfig config( _props->kurl().path() );
  config.setDesktopGroup();
  TQString patternStr = config.readEntry( "Patterns" );
  TQString iconStr = config.readEntry( "Icon" );
  TQString commentStr = config.readEntry( "Comment" );
  m_sMimeStr = config.readEntry( "MimeType" );

  if ( !patternStr.isEmpty() )
    patternEdit->setText( patternStr );
  if ( !commentStr.isEmpty() )
    commentEdit->setText( commentStr );
  if ( !m_sMimeStr.isEmpty() )
    mimeEdit->setText( m_sMimeStr );
  cbAutoEmbed->setTristate();
  if ( config.hasKey( "X-TDE-AutoEmbed" ) )
      cbAutoEmbed->setChecked( config.readBoolEntry( "X-TDE-AutoEmbed" ) );
  else
      cbAutoEmbed->setNoChange();

  connect( patternEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( commentEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( mimeEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( cbAutoEmbed, TQ_SIGNAL( toggled( bool ) ),
           this, TQ_SIGNAL( changed() ) );
}

KBindingPropsPlugin::~KBindingPropsPlugin()
{
  delete d;
}

// TQString KBindingPropsPlugin::tabName () const
// {
//   return i18n ("A&ssociation");
// }

bool KBindingPropsPlugin::supports( KFileItemList _items )
{
  if ( _items.count() != 1 )
    return false;
  KFileItem * item = _items.first();
  // check if desktop file
  if ( !KPropsDlgPlugin::isDesktopFile( item ) )
    return false;

  // open file and check type
  KDesktopFile config( item->url().path(), true /* readonly */ );
  return config.hasMimeTypeType();
}

void KBindingPropsPlugin::applyChanges()
{
  TQString path = properties->kurl().path();
  TQFile f( path );

  if ( !f.open( IO_ReadWrite ) )
  {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have "
				"sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", TQString::fromLatin1("MimeType") );

  config.writeEntry( "Patterns",  patternEdit->text() );
  config.writeEntry( "Comment", commentEdit->text() );
  config.writeEntry( "Comment",
		     commentEdit->text(), true, false, true ); // for compat
  config.writeEntry( "MimeType", mimeEdit->text() );
  if ( cbAutoEmbed->state() == TQButton::NoChange )
      config.deleteEntry( "X-TDE-AutoEmbed", false );
  else
      config.writeEntry( "X-TDE-AutoEmbed", cbAutoEmbed->isChecked() );
  config.sync();
}

/* ----------------------------------------------------
 *
 * KDevicePropsPlugin
 *
 * -------------------------------------------------- */

class KDevicePropsPlugin::KDevicePropsPluginPrivate
{
public:
  KDevicePropsPluginPrivate()
  {
  }
  ~KDevicePropsPluginPrivate()
  {
  }

  TQFrame *m_frame;
  TQStringList mountpointlist;
  TQLabel *m_freeSpaceText;
  TQLabel *m_freeSpaceLabel;
  TQProgressBar *m_freeSpaceBar;
};

KDevicePropsPlugin::KDevicePropsPlugin( KPropertiesDialog *_props ) : KPropsDlgPlugin( _props )
{
  d = new KDevicePropsPluginPrivate;
  d->m_frame = properties->addPage(i18n("De&vice"));

  TQStringList devices;
  KMountPoint::List mountPoints = KMountPoint::possibleMountPoints();

  for(KMountPoint::List::ConstIterator it = mountPoints.begin();
      it != mountPoints.end(); ++it)
  {
     KMountPoint *mp = *it;
     TQString mountPoint = mp->mountPoint();
     TQString device = mp->mountedFrom();
     kdDebug()<<"mountPoint :"<<mountPoint<<" device :"<<device<<" mp->mountType() :"<<mp->mountType()<<endl;

     if ((mountPoint != "-") && (mountPoint != "none") && !mountPoint.isEmpty()
          && device != "none")
     {
        devices.append( device + TQString::fromLatin1(" (")
                        + mountPoint + TQString::fromLatin1(")") );
        m_devicelist.append(device);
        d->mountpointlist.append(mountPoint);
     }
  }

  TQGridLayout *layout = new TQGridLayout( d->m_frame, 0, 2, 0,
                                        KDialog::spacingHint());
  layout->setColStretch(1, 1);

  TQLabel* label;
  label = new TQLabel( d->m_frame );
  label->setText( devices.count() == 0 ?
                      i18n("Device (/dev/fd0):") : // old style
                      i18n("Device:") ); // new style (combobox)
  layout->addWidget(label, 0, 0);

  device = new TQComboBox( true, d->m_frame, "ComboBox_device" );
  device->insertStringList( devices );
  layout->addWidget(device, 0, 1);
  connect( device, TQ_SIGNAL( activated( int ) ),
           this, TQ_SLOT( slotActivated( int ) ) );

  readonly = new TQCheckBox( d->m_frame, "CheckBox_readonly" );
  readonly->setText(  i18n("Read only") );
  layout->addWidget(readonly, 1, 1);

  label = new TQLabel( d->m_frame );
  label->setText( i18n("File system:") );
  layout->addWidget(label, 2, 0);

  TQLabel *fileSystem = new TQLabel( d->m_frame );
  layout->addWidget(fileSystem, 2, 1);

  label = new TQLabel( d->m_frame );
  label->setText( devices.count()==0 ?
                      i18n("Mount point (/mnt/floppy):") : // old style
                      i18n("Mount point:")); // new style (combobox)
  layout->addWidget(label, 3, 0);

  mountpoint = new TQLabel( d->m_frame, "LineEdit_mountpoint" );

  layout->addWidget(mountpoint, 3, 1);

  // show disk free
  d->m_freeSpaceText = new TQLabel(i18n("Free disk space:"), d->m_frame );
  layout->addWidget(d->m_freeSpaceText, 4, 0);

  d->m_freeSpaceLabel = new TQLabel( d->m_frame );
  layout->addWidget( d->m_freeSpaceLabel, 4, 1 );

  d->m_freeSpaceBar = new TQProgressBar( d->m_frame, "freeSpaceBar" );
  layout->addMultiCellWidget(d->m_freeSpaceBar, 5, 5, 0, 1);

  // we show it in the slot when we know the values
  d->m_freeSpaceText->hide();
  d->m_freeSpaceLabel->hide();
  d->m_freeSpaceBar->hide();

  KSeparator* sep = new KSeparator( KSeparator::HLine, d->m_frame);
  layout->addMultiCellWidget(sep, 6, 6, 0, 1);

  unmounted = new TDEIconButton( d->m_frame );
  int bsize = 66 + 2 * unmounted->style().pixelMetric(TQStyle::PM_ButtonMargin);
  unmounted->setFixedSize(bsize, bsize);
  unmounted->setIconType(TDEIcon::Desktop, TDEIcon::Device);
  layout->addWidget(unmounted, 7, 0);

  label = new TQLabel( i18n("Unmounted Icon"),  d->m_frame );
  layout->addWidget(label, 7, 1);

  layout->setRowStretch(8, 1);

  TQString path( _props->kurl().path() );

  TQFile f( path );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  TQString deviceStr = config.readEntry( "Dev" );
  TQString mountPointStr = config.readEntry( "MountPoint" );
  bool ro = config.readBoolEntry( "ReadOnly", false );
  TQString unmountedStr = config.readEntry( "UnmountIcon" );

  TQString fsType = config.readEntry("FSType");
  fileSystem->setText( (fsType.stripWhiteSpace() != "") ? i18n(fsType.local8Bit()) : "" );

  device->setEditText( deviceStr );
  if ( !deviceStr.isEmpty() ) {
    // Set default options for this device (first matching entry)
    int index = m_devicelist.findIndex(deviceStr);
    if (index != -1)
    {
      //kdDebug(250) << "found it " << index << endl;
      slotActivated( index );
    }
  }

  if ( !mountPointStr.isEmpty() )
  {
    mountpoint->setText( mountPointStr );
    updateInfo();
  }

  readonly->setChecked( ro );

  if ( unmountedStr.isEmpty() )
    unmountedStr = KMimeType::defaultMimeTypePtr()->KServiceType::icon(); // default icon

  unmounted->setIcon( unmountedStr );

  connect( device, TQ_SIGNAL( activated( int ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( device, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( readonly, TQ_SIGNAL( toggled( bool ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( unmounted, TQ_SIGNAL( iconChanged( TQString ) ),
           this, TQ_SIGNAL( changed() ) );

  connect( device, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SLOT( slotDeviceChanged() ) );

  processLockouts();
}

KDevicePropsPlugin::~KDevicePropsPlugin()
{
  delete d;
}

// TQString KDevicePropsPlugin::tabName () const
// {
//   return i18n ("De&vice");
// }

void KDevicePropsPlugin::processLockouts()
{
  if (device->currentText().stripWhiteSpace() != "")
  {
    properties->enableButtonOK(true);
  }
  else
  {
    properties->enableButtonOK(false);
  }
}

void KDevicePropsPlugin::updateInfo()
{
  // we show it in the slot when we know the values
  d->m_freeSpaceText->hide();
  d->m_freeSpaceLabel->hide();
  d->m_freeSpaceBar->hide();

  if ( !mountpoint->text().isEmpty() )
  {
    KDiskFreeSp * job = new KDiskFreeSp;
    connect( job, TQ_SIGNAL( foundMountPoint( const unsigned long&, const unsigned long&,
                                           const unsigned long&, const TQString& ) ),
             this, TQ_SLOT( slotFoundMountPoint( const unsigned long&, const unsigned long&,
                                              const unsigned long&, const TQString& ) ) );

    job->readDF( mountpoint->text() );
  }

  processLockouts();
}

void KDevicePropsPlugin::slotActivated( int index )
{
  // Update mountpoint so that it matches the device that was selected in the combo
  device->setEditText( m_devicelist[index] );
  mountpoint->setText( d->mountpointlist[index] );

  updateInfo();
}

void KDevicePropsPlugin::slotDeviceChanged()
{
  // Update mountpoint so that it matches the typed device
  int index = m_devicelist.findIndex( device->currentText() );
  if ( index != -1 )
    mountpoint->setText( d->mountpointlist[index] );
  else
    mountpoint->setText( TQString::null );

  updateInfo();
}

void KDevicePropsPlugin::slotFoundMountPoint( const unsigned long& kBSize,
                                              const unsigned long& /*kBUsed*/,
                                              const unsigned long& kBAvail,
                                              const TQString& )
{
  d->m_freeSpaceText->show();
  d->m_freeSpaceLabel->show();

  int percUsed = 100 - (int)(100.0 * kBAvail / kBSize);

  d->m_freeSpaceLabel->setText(
      // xgettext:no-c-format  --  Don't warn about translating the %1 out of %2 part.
      i18n("Available space out of total partition size (percent used)", "%1 out of %2 (%3% used)")
      .arg(TDEIO::convertSizeFromKB(kBAvail))
      .arg(TDEIO::convertSizeFromKB(kBSize))
      .arg( 100 - (int)(100.0 * kBAvail / kBSize) ));

  d->m_freeSpaceBar->setProgress(percUsed, 100);
  d->m_freeSpaceBar->show();
}

bool KDevicePropsPlugin::supports( KFileItemList _items )
{
  if ( _items.count() != 1 )
    return false;
  KFileItem * item = _items.first();
  // check if desktop file
  if ( !KPropsDlgPlugin::isDesktopFile( item ) )
    return false;
  // open file and check type
  KDesktopFile config( item->url().path(), true /* readonly */ );
  return config.hasDeviceType();
}

void KDevicePropsPlugin::applyChanges()
{
  TQString path = properties->kurl().path();
  TQFile f( path );
  if ( !f.open( IO_ReadWrite ) )
  {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have sufficient "
				"access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", TQString::fromLatin1("FSDevice") );

  config.writeEntry( "Dev", device->currentText() );
  config.writeEntry( "MountPoint", mountpoint->text() );

  config.writeEntry( "UnmountIcon", unmounted->icon() );
  kdDebug(250) << "unmounted->icon() = " << unmounted->icon() << endl;

  config.writeEntry( "ReadOnly", readonly->isChecked() );

  config.sync();
}


/* ----------------------------------------------------
 *
 * KDesktopPropsPlugin
 *
 * -------------------------------------------------- */


KDesktopPropsPlugin::KDesktopPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  TQFrame *frame = properties->addPage(i18n("&Application"));
  TQVBoxLayout *mainlayout = new TQVBoxLayout( frame, 0, KDialog::spacingHint() );

  w = new KPropertiesDesktopBase(frame);
  mainlayout->addWidget(w);

  bool bKDesktopMode = (TQCString(tqApp->name()) == "kdesktop"); // nasty heh?

  if (bKDesktopMode)
  {
    // Hide Name entry
    w->nameEdit->hide();
    w->nameLabel->hide();
  }

  w->pathEdit->setMode(KFile::Directory | KFile::LocalOnly);
  w->pathEdit->lineEdit()->setAcceptDrops(false);

  connect( w->nameEdit, TQ_SIGNAL( textChanged( const TQString & ) ), this, TQ_SIGNAL( changed() ) );
  connect( w->genNameEdit, TQ_SIGNAL( textChanged( const TQString & ) ), this, TQ_SIGNAL( changed() ) );
  connect( w->commentEdit, TQ_SIGNAL( textChanged( const TQString & ) ), this, TQ_SIGNAL( changed() ) );
  connect( w->commandEdit, TQ_SIGNAL( textChanged( const TQString & ) ), this, TQ_SIGNAL( changed() ) );
  connect( w->pathEdit, TQ_SIGNAL( textChanged( const TQString & ) ), this, TQ_SIGNAL( changed() ) );

  connect( w->browseButton, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotBrowseExec() ) );
  connect( w->addFiletypeButton, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotAddFiletype() ) );
  connect( w->delFiletypeButton, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotDelFiletype() ) );
  connect( w->advancedButton, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotAdvanced() ) );

  // now populate the page
  TQString path = _props->kurl().path();
  TQFile f( path );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KDesktopFile  config( path );
  TQString nameStr = config.readName();
  TQString genNameStr = config.readGenericName();
  TQString commentStr = config.readComment();
  TQString commandStr = config.readPathEntry( "Exec" );
  if (commandStr.left(12) == "ksystraycmd ")
  {
    commandStr.remove(0, 12);
    m_systrayBool = true;
  }
  else
    m_systrayBool = false;

  m_origCommandStr = commandStr;
  TQString pathStr = config.readPathEntry( "Path" );
  m_terminalBool = config.readBoolEntry( "Terminal" );
  m_terminalOptionStr = config.readEntry( "TerminalOptions" );
  m_suidBool = config.readBoolEntry( "X-TDE-SubstituteUID" ) || config.readBoolEntry( "X-KDE-SubstituteUID" );
  if( config.hasKey( "X-TDE-Username" ))
    m_suidUserStr = config.readEntry( "X-TDE-Username" );
  else
    m_suidUserStr = config.readEntry( "X-KDE-Username" );
  if( config.hasKey( "StartupNotify" ))
    m_startupBool = config.readBoolEntry( "StartupNotify", true );
  else
    m_startupBool = config.readBoolEntry( "X-TDE-StartupNotify", true );
  m_dcopServiceType = config.readEntry("X-DCOP-ServiceType").lower();

  TQStringList mimeTypes = config.readListEntry( "MimeType", ';' );

  if ( nameStr.isEmpty() || bKDesktopMode ) {
    // We'll use the file name if no name is specified
    // because we _need_ a Name for a valid file.
    // But let's do it in apply, not here, so that we pick up the right name.
    setDirty();
  }
  if ( !bKDesktopMode )
    w->nameEdit->setText(nameStr);

  w->genNameEdit->setText( genNameStr );
  w->commentEdit->setText( commentStr );
  w->commandEdit->setText( commandStr );
  w->pathEdit->lineEdit()->setText( pathStr );
  w->filetypeList->setAllColumnsShowFocus(true);

  KMimeType::Ptr defaultMimetype = KMimeType::defaultMimeTypePtr();
  for(TQStringList::ConstIterator it = mimeTypes.begin();
      it != mimeTypes.end(); )
  {
    KMimeType::Ptr p = KMimeType::mimeType(*it);
    ++it;
    TQString preference;
    if (it != mimeTypes.end())
    {
       bool numeric;
       (*it).toInt(&numeric);
       if (numeric)
       {
         preference = *it;
         ++it;
       }
    }
    if (p && (p != defaultMimetype))
    {
       new TQListViewItem(w->filetypeList, p->name(), p->comment(), preference);
    }
  }

}

KDesktopPropsPlugin::~KDesktopPropsPlugin()
{
}

void KDesktopPropsPlugin::slotSelectMimetype()
{
  TQListView *w = (TQListView*)sender();
  TQListViewItem *item = w->firstChild();
  while(item)
  {
     if (item->isSelected())
        w->setSelected(item, false);
     item = item->nextSibling();
  }
}

void KDesktopPropsPlugin::slotAddFiletype()
{
  KDialogBase dlg(w, "KPropertiesMimetypes", true,
                  i18n("Add File Type for %1").arg(properties->kurl().fileName()),
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok);

  KGuiItem okItem(i18n("&Add"), TQString::null /* no icon */,
                  i18n("Add the selected file types to\nthe list of supported file types."),
                  i18n("Add the selected file types to\nthe list of supported file types."));
  dlg.setButtonOK(okItem);

  KPropertiesMimetypeBase *mw = new KPropertiesMimetypeBase(&dlg);

  dlg.setMainWidget(mw);

  {
     mw->listView->setRootIsDecorated(true);
     mw->listView->setSelectionMode(TQListView::Extended);
     mw->listView->setAllColumnsShowFocus(true);
     mw->listView->setFullWidth(true);
     mw->listView->setMinimumSize(500,400);

     connect(mw->listView, TQ_SIGNAL(selectionChanged()),
             this, TQ_SLOT(slotSelectMimetype()));
     connect(mw->listView, TQ_SIGNAL(doubleClicked( TQListViewItem *, const TQPoint &, int )),
             &dlg, TQ_SLOT( slotOk()));

     TQMap<TQString,TQListViewItem*> majorMap;
     TQListViewItem *majorGroup;
     KMimeType::List mimetypes = KMimeType::allMimeTypes();
     TQValueListIterator<KMimeType::Ptr> it(mimetypes.begin());
     for (; it != mimetypes.end(); ++it) {
        TQString mimetype = (*it)->name();
        if (mimetype == KMimeType::defaultMimeType())
           continue;
        int index = mimetype.find("/");
        TQString maj = mimetype.left(index);
        TQString min = mimetype.mid(index+1);

        TQMapIterator<TQString,TQListViewItem*> mit = majorMap.find( maj );
        if ( mit == majorMap.end() ) {
           majorGroup = new TQListViewItem( mw->listView, maj );
           majorGroup->setExpandable(true);
           mw->listView->setOpen(majorGroup, true);
           majorMap.insert( maj, majorGroup );
        }
        else
        {
           majorGroup = mit.data();
        }

        TQListViewItem *item = new TQListViewItem(majorGroup, min, (*it)->comment());
        item->setPixmap(0, (*it)->pixmap(TDEIcon::Small, IconSize(TDEIcon::Small)));
     }
     TQMapIterator<TQString,TQListViewItem*> mit = majorMap.find( "all" );
     if ( mit != majorMap.end())
     {
        mw->listView->setCurrentItem(mit.data());
        mw->listView->ensureItemVisible(mit.data());
     }
  }

  if (dlg.exec() == KDialogBase::Accepted)
  {
     KMimeType::Ptr defaultMimetype = KMimeType::defaultMimeTypePtr();
     TQListViewItem *majorItem = mw->listView->firstChild();
     while(majorItem)
     {
        TQString major = majorItem->text(0);

        TQListViewItem *minorItem = majorItem->firstChild();
        while(minorItem)
        {
           if (minorItem->isSelected())
           {
              TQString mimetype = major + "/" + minorItem->text(0);
              KMimeType::Ptr p = KMimeType::mimeType(mimetype);
              if (p && (p != defaultMimetype))
              {
                 mimetype = p->name();
                 bool found = false;
                 TQListViewItem *item = w->filetypeList->firstChild();
                 while (item)
                 {
                    if (mimetype == item->text(0))
                    {
                       found = true;
                       break;
                    }
                    item = item->nextSibling();
                 }
                 if (!found) {
                    new TQListViewItem(w->filetypeList, p->name(), p->comment());
                    emit changed();
                 }
              }
           }
           minorItem = minorItem->nextSibling();
        }

        majorItem = majorItem->nextSibling();
     }

  }
}

void KDesktopPropsPlugin::slotDelFiletype()
{
  delete w->filetypeList->currentItem();
  emit changed();
}

void KDesktopPropsPlugin::checkCommandChanged()
{
  if (KRun::binaryName(w->commandEdit->text(), true) !=
      KRun::binaryName(m_origCommandStr, true))
  {
    TQString m_origCommandStr = w->commandEdit->text();
    m_dcopServiceType= TQString::null; // Reset
  }
}

void KDesktopPropsPlugin::applyChanges()
{
  kdDebug(250) << "KDesktopPropsPlugin::applyChanges" << endl;
  TQString path = properties->kurl().path();

  TQFile f( path );

  if ( !f.open( IO_ReadWrite ) ) {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have "
				"sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  // If the command is changed we reset certain settings that are strongly
  // coupled to the command.
  checkCommandChanged();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", TQString::fromLatin1("Application"));
  config.writeEntry( "Comment", w->commentEdit->text() );
  config.writeEntry( "Comment", w->commentEdit->text(), true, false, true ); // for compat
  config.writeEntry( "GenericName", w->genNameEdit->text() );
  config.writeEntry( "GenericName", w->genNameEdit->text(), true, false, true ); // for compat

  if (m_systrayBool)
    config.writePathEntry( "Exec", w->commandEdit->text().prepend("ksystraycmd ") );
  else
    config.writePathEntry( "Exec", w->commandEdit->text() );
  config.writePathEntry( "Path", w->pathEdit->lineEdit()->text() );

  // Write mimeTypes
  TQStringList mimeTypes;
  for( TQListViewItem *item = w->filetypeList->firstChild();
       item; item = item->nextSibling() )
  {
    TQString preference = item->text(2);
    mimeTypes.append(item->text(0));
    if (!preference.isEmpty())
       mimeTypes.append(preference);
  }

  config.writeEntry( "MimeType", mimeTypes, ';' );

  if ( !w->nameEdit->isHidden() ) {
      TQString nameStr = w->nameEdit->text();
      config.writeEntry( "Name", nameStr );
      config.writeEntry( "Name", nameStr, true, false, true );
  }

  config.writeEntry("Terminal", m_terminalBool);
  config.writeEntry("TerminalOptions", m_terminalOptionStr);
  config.writeEntry("X-TDE-SubstituteUID", m_suidBool);
  config.writeEntry("X-TDE-Username", m_suidUserStr);
  config.writeEntry("StartupNotify", m_startupBool);
  config.writeEntry("X-DCOP-ServiceType", m_dcopServiceType);
  config.sync();

  // KSycoca update needed?
  TQString sycocaPath = TDEGlobal::dirs()->relativeLocation("apps", path);
  bool updateNeeded = !sycocaPath.startsWith("/");
  if (!updateNeeded)
  {
     sycocaPath = TDEGlobal::dirs()->relativeLocation("xdgdata-apps", path);
     updateNeeded = !sycocaPath.startsWith("/");
  }
  if (updateNeeded)
     KService::rebuildKSycoca(w);
}


void KDesktopPropsPlugin::slotBrowseExec()
{
  KURL f = KFileDialog::getOpenURL( TQString::null,
                                      TQString::null, w );
  if ( f.isEmpty() )
    return;

  if ( !f.isLocalFile()) {
    KMessageBox::sorry(w, i18n("Only executables on local file systems are supported."));
    return;
  }

  TQString path = f.path();
  KRun::shellQuote( path );
  w->commandEdit->setText( path );
}

void KDesktopPropsPlugin::slotAdvanced()
{
  KDialogBase dlg(w, "KPropertiesDesktopAdv", true,
      i18n("Advanced Options for %1").arg(properties->kurl().fileName()),
      KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok);
  KPropertiesDesktopAdvBase *w = new KPropertiesDesktopAdvBase(&dlg);

  dlg.setMainWidget(w);

  // If the command is changed we reset certain settings that are strongly
  // coupled to the command.
  checkCommandChanged();

  // check to see if we use konsole if not do not add the nocloseonexit
  // because we don't know how to do this on other terminal applications
  TDEConfigGroup confGroup( TDEGlobal::config(), TQString::fromLatin1("General") );
  TQString preferredTerminal = confGroup.readPathEntry("TerminalApplication",
						  TQString::fromLatin1("konsole"));

  bool terminalCloseBool = false;

  if (preferredTerminal == "konsole")
  {
     terminalCloseBool = (m_terminalOptionStr.contains( "--noclose" ) > 0);
     w->terminalCloseCheck->setChecked(terminalCloseBool);
     m_terminalOptionStr.replace( "--noclose", "");
  }
  else
  {
     w->terminalCloseCheck->hide();
  }

  w->terminalCheck->setChecked(m_terminalBool);
  w->terminalEdit->setText(m_terminalOptionStr);
  w->terminalCloseCheck->setEnabled(m_terminalBool);
  w->terminalEdit->setEnabled(m_terminalBool);
  w->terminalEditLabel->setEnabled(m_terminalBool);

  w->suidCheck->setChecked(m_suidBool);
  w->suidEdit->setText(m_suidUserStr);
  w->suidEdit->setEnabled(m_suidBool);
  w->suidEditLabel->setEnabled(m_suidBool);

  w->startupInfoCheck->setChecked(m_startupBool);
  w->systrayCheck->setChecked(m_systrayBool);

  if (m_dcopServiceType == "unique")
    w->dcopCombo->setCurrentItem(2);
  else if (m_dcopServiceType == "multi")
    w->dcopCombo->setCurrentItem(1);
  else if (m_dcopServiceType == "wait")
    w->dcopCombo->setCurrentItem(3);
  else
    w->dcopCombo->setCurrentItem(0);

  // Provide username completion up to 1000 users.
  TDECompletion *kcom = new TDECompletion;
  kcom->setOrder(TDECompletion::Sorted);
  struct passwd *pw;
  int i, maxEntries = 1000;
  setpwent();
  for (i=0; ((pw = getpwent()) != 0L) && (i < maxEntries); i++)
    kcom->addItem(TQString::fromLatin1(pw->pw_name));
  endpwent();
  if (i < maxEntries)
  {
    w->suidEdit->setCompletionObject(kcom, true);
    w->suidEdit->setAutoDeleteCompletionObject( true );
    w->suidEdit->setCompletionMode(TDEGlobalSettings::CompletionAuto);
  }
  else
  {
    delete kcom;
  }

  connect( w->terminalEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( w->terminalCloseCheck, TQ_SIGNAL( toggled( bool ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( w->terminalCheck, TQ_SIGNAL( toggled( bool ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( w->suidCheck, TQ_SIGNAL( toggled( bool ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( w->suidEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( w->startupInfoCheck, TQ_SIGNAL( toggled( bool ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( w->systrayCheck, TQ_SIGNAL( toggled( bool ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( w->dcopCombo, TQ_SIGNAL( highlighted( int ) ),
           this, TQ_SIGNAL( changed() ) );

  if ( dlg.exec() == TQDialog::Accepted )
  {
    m_terminalOptionStr = w->terminalEdit->text().stripWhiteSpace();
    m_terminalBool = w->terminalCheck->isChecked();
    m_suidBool = w->suidCheck->isChecked();
    m_suidUserStr = w->suidEdit->text().stripWhiteSpace();
    m_startupBool = w->startupInfoCheck->isChecked();
    m_systrayBool = w->systrayCheck->isChecked();

    if (w->terminalCloseCheck->isChecked())
    {
      m_terminalOptionStr.append(" --noclose");
    }

    switch(w->dcopCombo->currentItem())
    {
      case 1:  m_dcopServiceType = "multi"; break;
      case 2:  m_dcopServiceType = "unique"; break;
      case 3:  m_dcopServiceType = "wait"; break;
      default: m_dcopServiceType = "none"; break;
    }
  }
}

bool KDesktopPropsPlugin::supports( KFileItemList _items )
{
  if ( _items.count() != 1 )
    return false;
  KFileItem * item = _items.first();
  // check if desktop file
  if ( !KPropsDlgPlugin::isDesktopFile( item ) )
    return false;
  // open file and check type
  KDesktopFile config( item->url().path(), true /* readonly */ );
  return config.hasApplicationType() && kapp->authorize("run_desktop_files") && kapp->authorize("shell_access");
}

void KPropertiesDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KPropsDlgPlugin::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }





/**
 * The following code is obsolete and only kept for binary compatibility
 * To be removed in KDE 4
 */

class KExecPropsPlugin::KExecPropsPluginPrivate
{
public:
  KExecPropsPluginPrivate()
  {
  }
  ~KExecPropsPluginPrivate()
  {
  }

  TQFrame *m_frame;
  TQCheckBox *nocloseonexitCheck;
};

KExecPropsPlugin::KExecPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KExecPropsPluginPrivate;
  d->m_frame = properties->addPage(i18n("E&xecute"));
  TQVBoxLayout * mainlayout = new TQVBoxLayout( d->m_frame, 0,
      KDialog::spacingHint());

  // Now the widgets in the top layout

  TQLabel* l;
  l = new TQLabel( i18n( "Comman&d:" ), d->m_frame );
  mainlayout->addWidget(l);

  TQHBoxLayout * hlayout;
  hlayout = new TQHBoxLayout(KDialog::spacingHint());
  mainlayout->addLayout(hlayout);

  execEdit = new KLineEdit( d->m_frame );
  TQWhatsThis::add(execEdit,i18n(
    "Following the command, you can have several place holders which will be replaced "
    "with the actual values when the actual program is run:\n"
    "%f - a single file name\n"
    "%F - a list of files; use for applications that can open several local files at once\n"
    "%u - a single URL\n"
    "%U - a list of URLs\n"
    "%d - the folder of the file to open\n"
    "%D - a list of folders\n"
    "%i - the icon\n"
    "%m - the mini-icon\n"
    "%c - the caption"));
  hlayout->addWidget(execEdit, 1);

  l->setBuddy( execEdit );

  execBrowse = new TQPushButton( d->m_frame );
  execBrowse->setText( i18n("&Browse...") );
  hlayout->addWidget(execBrowse);

  // The groupbox about swallowing
  TQGroupBox* tmpQGroupBox;
  tmpQGroupBox = new TQGroupBox( i18n("Panel Embedding"), d->m_frame );
  tmpQGroupBox->setColumnLayout( 0, TQt::Horizontal );

  mainlayout->addWidget(tmpQGroupBox);

  TQGridLayout *grid = new TQGridLayout(tmpQGroupBox->layout(), 2, 2);
  grid->setSpacing( KDialog::spacingHint() );
  grid->setColStretch(1, 1);

  l = new TQLabel( i18n( "&Execute on click:" ), tmpQGroupBox );
  grid->addWidget(l, 0, 0);

  swallowExecEdit = new KLineEdit( tmpQGroupBox );
  grid->addWidget(swallowExecEdit, 0, 1);

  l->setBuddy( swallowExecEdit );

  l = new TQLabel( i18n( "&Window title:" ), tmpQGroupBox );
  grid->addWidget(l, 1, 0);

  swallowTitleEdit = new KLineEdit( tmpQGroupBox );
  grid->addWidget(swallowTitleEdit, 1, 1);

  l->setBuddy( swallowTitleEdit );

  // The groupbox about run in terminal

  tmpQGroupBox = new TQGroupBox( d->m_frame );
  tmpQGroupBox->setColumnLayout( 0, TQt::Horizontal );

  mainlayout->addWidget(tmpQGroupBox);

  grid = new TQGridLayout(tmpQGroupBox->layout(), 3, 2);
  grid->setSpacing( KDialog::spacingHint() );
  grid->setColStretch(1, 1);

  terminalCheck = new TQCheckBox( tmpQGroupBox );
  terminalCheck->setText( i18n("&Run in terminal") );
  grid->addMultiCellWidget(terminalCheck, 0, 0, 0, 1);

  // check to see if we use konsole if not do not add the nocloseonexit
  // because we don't know how to do this on other terminal applications
  TDEConfigGroup confGroup( TDEGlobal::config(), TQString::fromLatin1("General") );
  TQString preferredTerminal = confGroup.readPathEntry("TerminalApplication",
						  TQString::fromLatin1("konsole"));

  int posOptions = 1;
  d->nocloseonexitCheck = 0L;
  if (preferredTerminal == "konsole")
  {
    posOptions = 2;
    d->nocloseonexitCheck = new TQCheckBox( tmpQGroupBox );
    d->nocloseonexitCheck->setText( i18n("Do not &close when command exits") );
    grid->addMultiCellWidget(d->nocloseonexitCheck, 1, 1, 0, 1);
  }

  terminalLabel = new TQLabel( i18n( "&Terminal options:" ), tmpQGroupBox );
  grid->addWidget(terminalLabel, posOptions, 0);

  terminalEdit = new KLineEdit( tmpQGroupBox );
  grid->addWidget(terminalEdit, posOptions, 1);

  terminalLabel->setBuddy( terminalEdit );

  // The groupbox about run with substituted uid.

  tmpQGroupBox = new TQGroupBox( d->m_frame );
  tmpQGroupBox->setColumnLayout( 0, TQt::Horizontal );

  mainlayout->addWidget(tmpQGroupBox);

  grid = new TQGridLayout(tmpQGroupBox->layout(), 2, 2);
  grid->setSpacing(KDialog::spacingHint());
  grid->setColStretch(1, 1);

  suidCheck = new TQCheckBox(tmpQGroupBox);
  suidCheck->setText(i18n("Ru&n as a different user"));
  grid->addMultiCellWidget(suidCheck, 0, 0, 0, 1);

  suidLabel = new TQLabel(i18n( "&Username:" ), tmpQGroupBox);
  grid->addWidget(suidLabel, 1, 0);

  suidEdit = new KLineEdit(tmpQGroupBox);
  grid->addWidget(suidEdit, 1, 1);

  suidLabel->setBuddy( suidEdit );

  mainlayout->addStretch(1);

  // now populate the page
  TQString path = _props->kurl().path();
  TQFile f( path );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KSimpleConfig config( path );
  config.setDollarExpansion( false );
  config.setDesktopGroup();
  execStr = config.readPathEntry( "Exec" );
  swallowExecStr = config.readPathEntry( "SwallowExec" );
  swallowTitleStr = config.readEntry( "SwallowTitle" );
  termBool = config.readBoolEntry( "Terminal" );
  termOptionsStr = config.readEntry( "TerminalOptions" );
  suidBool = config.readBoolEntry( "X-TDE-SubstituteUID" );
  suidUserStr = config.readEntry( "X-TDE-Username" );

  if ( !swallowExecStr.isNull() )
    swallowExecEdit->setText( swallowExecStr );
  if ( !swallowTitleStr.isNull() )
    swallowTitleEdit->setText( swallowTitleStr );

  if ( !execStr.isNull() )
    execEdit->setText( execStr );

  if ( d->nocloseonexitCheck )
  {
    d->nocloseonexitCheck->setChecked( (termOptionsStr.contains( "--noclose" ) > 0) );
    termOptionsStr.replace( "--noclose", "");
  }
  if ( !termOptionsStr.isNull() )
    terminalEdit->setText( termOptionsStr );

  terminalCheck->setChecked( termBool );
  enableCheckedEdit();

  suidCheck->setChecked( suidBool );
  suidEdit->setText( suidUserStr );
  enableSuidEdit();

  // Provide username completion up to 1000 users.
  TDECompletion *kcom = new TDECompletion;
  kcom->setOrder(TDECompletion::Sorted);
  struct passwd *pw;
  int i, maxEntries = 1000;
  setpwent();
  for (i=0; ((pw = getpwent()) != 0L) && (i < maxEntries); i++)
    kcom->addItem(TQString::fromLatin1(pw->pw_name));
  endpwent();
  if (i < maxEntries)
  {
    suidEdit->setCompletionObject(kcom, true);
    suidEdit->setAutoDeleteCompletionObject( true );
    suidEdit->setCompletionMode(TDEGlobalSettings::CompletionAuto);
  }
  else
  {
    delete kcom;
  }

  connect( swallowExecEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( swallowTitleEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( execEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( terminalEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  if (d->nocloseonexitCheck)
    connect( d->nocloseonexitCheck, TQ_SIGNAL( toggled( bool ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( terminalCheck, TQ_SIGNAL( toggled( bool ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( suidCheck, TQ_SIGNAL( toggled( bool ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( suidEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );

  connect( execBrowse, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotBrowseExec() ) );
  connect( terminalCheck, TQ_SIGNAL( clicked() ), this,  TQ_SLOT( enableCheckedEdit() ) );
  connect( suidCheck, TQ_SIGNAL( clicked() ), this,  TQ_SLOT( enableSuidEdit() ) );

}

KExecPropsPlugin::~KExecPropsPlugin()
{
  delete d;
}

void KExecPropsPlugin::enableCheckedEdit()
{
  bool checked = terminalCheck->isChecked();
  terminalLabel->setEnabled( checked );
  if (d->nocloseonexitCheck)
    d->nocloseonexitCheck->setEnabled( checked );
  terminalEdit->setEnabled( checked );
}

void KExecPropsPlugin::enableSuidEdit()
{
  bool checked = suidCheck->isChecked();
  suidLabel->setEnabled( checked );
  suidEdit->setEnabled( checked );
}

bool KExecPropsPlugin::supports( KFileItemList _items )
{
  if ( _items.count() != 1 )
    return false;
  KFileItem * item = _items.first();
  // check if desktop file
  if ( !KPropsDlgPlugin::isDesktopFile( item ) )
    return false;
  // open file and check type
  KDesktopFile config( item->url().path(), true /* readonly */ );
  return config.hasApplicationType() && kapp->authorize("run_desktop_files") && kapp->authorize("shell_access");
}

void KExecPropsPlugin::applyChanges()
{
  kdDebug(250) << "KExecPropsPlugin::applyChanges" << endl;
  TQString path = properties->kurl().path();

  TQFile f( path );

  if ( !f.open( IO_ReadWrite ) ) {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have "
				"sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", TQString::fromLatin1("Application"));
  config.writePathEntry( "Exec", execEdit->text() );
  config.writePathEntry( "SwallowExec", swallowExecEdit->text() );
  config.writeEntry( "SwallowTitle", swallowTitleEdit->text() );
  config.writeEntry( "Terminal", terminalCheck->isChecked() );
  TQString temp = terminalEdit->text();
  if (d->nocloseonexitCheck )
    if ( d->nocloseonexitCheck->isChecked() )
      temp += TQString::fromLatin1("--noclose ");
  temp = temp.stripWhiteSpace();
  config.writeEntry( "TerminalOptions", temp );
  config.writeEntry( "X-TDE-SubstituteUID", suidCheck->isChecked() );
  config.writeEntry( "X-TDE-Username", suidEdit->text() );
}


void KExecPropsPlugin::slotBrowseExec()
{
    KURL f = KFileDialog::getOpenURL( TQString::null,
                                      TQString::null, d->m_frame );
    if ( f.isEmpty() )
        return;

    if ( !f.isLocalFile()) {
        KMessageBox::sorry(d->m_frame, i18n("Only executables on local file systems are supported."));
        return;
    }

    TQString path = f.path();
    KRun::shellQuote( path );
    execEdit->setText( path );
}

class TDEApplicationPropsPlugin::TDEApplicationPropsPluginPrivate
{
public:
  TDEApplicationPropsPluginPrivate()
  {
      m_kdesktopMode = TQCString(tqApp->name()) == "kdesktop"; // nasty heh?
  }
  ~TDEApplicationPropsPluginPrivate()
  {
  }

  TQFrame *m_frame;
  bool m_kdesktopMode;
};

TDEApplicationPropsPlugin::TDEApplicationPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new TDEApplicationPropsPluginPrivate;
  d->m_frame = properties->addPage(i18n("&Application"));
  TQVBoxLayout *toplayout = new TQVBoxLayout( d->m_frame, 0, KDialog::spacingHint());

  TQIconSet iconSet;
  TQPixmap pixMap;

  addExtensionButton = new TQPushButton( TQString::null, d->m_frame );
  iconSet = SmallIconSet( "back" );
  addExtensionButton->setIconSet( iconSet );
  pixMap = iconSet.pixmap( TQIconSet::Small, TQIconSet::Normal );
  addExtensionButton->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
  connect( addExtensionButton, TQ_SIGNAL( clicked() ),
            TQ_SLOT( slotAddExtension() ) );

  delExtensionButton = new TQPushButton( TQString::null, d->m_frame );
  iconSet = SmallIconSet( "forward" );
  delExtensionButton->setIconSet( iconSet );
  delExtensionButton->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
  connect( delExtensionButton, TQ_SIGNAL( clicked() ),
            TQ_SLOT( slotDelExtension() ) );

  TQLabel *l;

  TQGridLayout *grid = new TQGridLayout(2, 2);
  grid->setColStretch(1, 1);
  toplayout->addLayout(grid);

  if ( d->m_kdesktopMode )
  {
      // in kdesktop the name field comes from the first tab
      nameEdit = 0L;
  }
  else
  {
      l = new TQLabel(i18n("Name:"), d->m_frame, "Label_4" );
      grid->addWidget(l, 0, 0);

      nameEdit = new KLineEdit( d->m_frame, "LineEdit_3" );
      grid->addWidget(nameEdit, 0, 1);
  }

  l = new TQLabel(i18n("Description:"),  d->m_frame, "Label_5" );
  grid->addWidget(l, 1, 0);

  genNameEdit = new KLineEdit( d->m_frame, "LineEdit_4" );
  grid->addWidget(genNameEdit, 1, 1);

  l = new TQLabel(i18n("Comment:"),  d->m_frame, "Label_3" );
  grid->addWidget(l, 2, 0);

  commentEdit = new KLineEdit( d->m_frame, "LineEdit_2" );
  grid->addWidget(commentEdit, 2, 1);

  l = new TQLabel(i18n("File types:"), d->m_frame);
  toplayout->addWidget(l, 0, AlignLeft);

  grid = new TQGridLayout(4, 3);
  grid->setColStretch(0, 1);
  grid->setColStretch(2, 1);
  grid->setRowStretch( 0, 1 );
  grid->setRowStretch( 3, 1 );
  toplayout->addLayout(grid, 2);

  extensionsList = new TQListBox( d->m_frame );
  extensionsList->setSelectionMode( TQListBox::Extended );
  grid->addMultiCellWidget(extensionsList, 0, 3, 0, 0);

  grid->addWidget(addExtensionButton, 1, 1);
  grid->addWidget(delExtensionButton, 2, 1);

  availableExtensionsList = new TQListBox( d->m_frame );
  availableExtensionsList->setSelectionMode( TQListBox::Extended );
  grid->addMultiCellWidget(availableExtensionsList, 0, 3, 2, 2);

  TQString path = properties->kurl().path() ;
  TQFile f( path );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KDesktopFile config( path );
  TQString commentStr = config.readComment();
  TQString genNameStr = config.readGenericName();

  TQStringList selectedTypes = config.readListEntry( "X-TDE-ServiceTypes" );
  // For compatibility with KDE 1.x
  selectedTypes += config.readListEntry( "MimeType", ';' );

  TQString nameStr = config.readName();
  if ( nameStr.isEmpty() || d->m_kdesktopMode ) {
    // We'll use the file name if no name is specified
    // because we _need_ a Name for a valid file.
    // But let's do it in apply, not here, so that we pick up the right name.
    setDirty();
  }

  commentEdit->setText( commentStr );
  genNameEdit->setText( genNameStr );
  if ( nameEdit )
      nameEdit->setText( nameStr );

  selectedTypes.sort();
  TQStringList::Iterator sit = selectedTypes.begin();
  for( ; sit != selectedTypes.end(); ++sit ) {
    if ( !((*sit).isEmpty()) )
      extensionsList->insertItem( *sit );
  }

  KMimeType::List mimeTypes = KMimeType::allMimeTypes();
  TQValueListIterator<KMimeType::Ptr> it2 = mimeTypes.begin();
  for ( ; it2 != mimeTypes.end(); ++it2 )
    addMimeType ( (*it2)->name() );

  updateButton();

  connect( extensionsList, TQ_SIGNAL( highlighted( int ) ),
           this, TQ_SLOT( updateButton() ) );
  connect( availableExtensionsList, TQ_SIGNAL( highlighted( int ) ),
           this, TQ_SLOT( updateButton() ) );

  connect( addExtensionButton, TQ_SIGNAL( clicked() ),
           this, TQ_SIGNAL( changed() ) );
  connect( delExtensionButton, TQ_SIGNAL( clicked() ),
           this, TQ_SIGNAL( changed() ) );
  if ( nameEdit )
      connect( nameEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
               this, TQ_SIGNAL( changed() ) );
  connect( commentEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( genNameEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( availableExtensionsList, TQ_SIGNAL( selected( int ) ),
           this, TQ_SIGNAL( changed() ) );
  connect( extensionsList, TQ_SIGNAL( selected( int ) ),
           this, TQ_SIGNAL( changed() ) );
}

TDEApplicationPropsPlugin::~TDEApplicationPropsPlugin()
{
  delete d;
}

// TQString TDEApplicationPropsPlugin::tabName () const
// {
//   return i18n ("&Application");
// }

void TDEApplicationPropsPlugin::updateButton()
{
    addExtensionButton->setEnabled(availableExtensionsList->currentItem()>-1);
    delExtensionButton->setEnabled(extensionsList->currentItem()>-1);
}

void TDEApplicationPropsPlugin::addMimeType( const TQString & name )
{
  // Add a mimetype to the list of available mime types if not in the extensionsList

  bool insert = true;

  for ( uint i = 0; i < extensionsList->count(); i++ )
    if ( extensionsList->text( i ) == name )
      insert = false;

  if ( insert )
  {
    availableExtensionsList->insertItem( name );
    availableExtensionsList->sort();
  }
}

bool TDEApplicationPropsPlugin::supports( KFileItemList _items )
{
  // same constraints as KExecPropsPlugin : desktop file with Type = Application
  return KExecPropsPlugin::supports( _items );
}

void TDEApplicationPropsPlugin::applyChanges()
{
  TQString path = properties->kurl().path();

  TQFile f( path );

  if ( !f.open( IO_ReadWrite ) ) {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not "
				"have sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", TQString::fromLatin1("Application"));
  config.writeEntry( "Comment", commentEdit->text() );
  config.writeEntry( "Comment", commentEdit->text(), true, false, true ); // for compat
  config.writeEntry( "GenericName", genNameEdit->text() );
  config.writeEntry( "GenericName", genNameEdit->text(), true, false, true ); // for compat

  TQStringList selectedTypes;
  for ( uint i = 0; i < extensionsList->count(); i++ )
    selectedTypes.append( extensionsList->text( i ) );

  config.writeEntry( "MimeType", selectedTypes, ';' );
  config.writeEntry( "X-TDE-ServiceTypes", "" );
  // hmm, actually it should probably be the contrary (but see also typeslistitem.cpp)

  TQString nameStr = nameEdit ? nameEdit->text() : TQString::null;
  if ( nameStr.isEmpty() ) // nothing entered, or widget not existing at all (kdesktop mode)
    nameStr = nameFromFileName(properties->kurl().fileName());

  config.writeEntry( "Name", nameStr );
  config.writeEntry( "Name", nameStr, true, false, true );

  config.sync();
}

void TDEApplicationPropsPlugin::slotAddExtension()
{
  TQListBoxItem *item = availableExtensionsList->firstItem();
  TQListBoxItem *nextItem;

  while ( item )
  {
    nextItem = item->next();

    if ( item->isSelected() )
    {
      extensionsList->insertItem( item->text() );
      availableExtensionsList->removeItem( availableExtensionsList->index( item ) );
    }

    item = nextItem;
  }

  extensionsList->sort();
  updateButton();
}

void TDEApplicationPropsPlugin::slotDelExtension()
{
  TQListBoxItem *item = extensionsList->firstItem();
  TQListBoxItem *nextItem;

  while ( item )
  {
    nextItem = item->next();

    if ( item->isSelected() )
    {
      availableExtensionsList->insertItem( item->text() );
      extensionsList->removeItem( extensionsList->index( item ) );
    }

    item = nextItem;
  }

  availableExtensionsList->sort();
  updateButton();
}



#include "kpropertiesdialog.moc"

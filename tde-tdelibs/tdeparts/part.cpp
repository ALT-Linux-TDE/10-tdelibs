/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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

#include <tdeparts/part.h>
#include <tdeparts/event.h>
#include <tdeparts/plugin.h>
#include <tdeparts/mainwindow.h>
#include <tdeparts/partmanager.h>

#include <tqapplication.h>
#include <tqfile.h>
#include <tqpoint.h>
#include <tqpointarray.h>
#include <tqpainter.h>
#include <tqtextstream.h>
#include <tqfileinfo.h>

#include <kinstance.h>
#include <tdelocale.h>
#include <tdetempfile.h>
#include <tdemessagebox.h>
#include <tdeio/job.h>
#include <kstandarddirs.h>
#include <tdefiledialog.h>
#include <kdirnotify_stub.h>

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <kdebug.h>

template class TQPtrList<KXMLGUIClient>;

using namespace KParts;

namespace KParts
{

class PartBasePrivate
{
public:
  PartBasePrivate()
  {
      m_pluginLoadingMode = PartBase::LoadPlugins;
  }
  ~PartBasePrivate()
  {
  }
  PartBase::PluginLoadingMode m_pluginLoadingMode;
};

class PartPrivate
{
public:
  PartPrivate()
  {
    m_bSelectable = true;
  }
  ~PartPrivate()
  {
  }

  bool m_bSelectable;
};
}

PartBase::PartBase()
{
  d = new PartBasePrivate;
  m_obj = 0L;
}

PartBase::~PartBase()
{
  delete d;
}

void PartBase::setPartObject( TQObject *obj )
{
  m_obj = obj;
}

TQObject *PartBase::partObject() const
{
  return m_obj;
}

void PartBase::setInstance( TDEInstance *inst )
{
  setInstance( inst, true );
}

void PartBase::setInstance( TDEInstance *inst, bool bLoadPlugins )
{
  KXMLGUIClient::setInstance( inst );
  TDEGlobal::locale()->insertCatalogue( inst->instanceName() );
  // install 'instancename'data resource type
  TDEGlobal::dirs()->addResourceType( inst->instanceName() + "data",
                                    TDEStandardDirs::kde_default( "data" )
                                    + TQString::fromLatin1( inst->instanceName() ) + '/' );
  if ( bLoadPlugins )
    loadPlugins( m_obj, this, instance() );
}

void PartBase::loadPlugins( TQObject *parent, KXMLGUIClient *parentGUIClient, TDEInstance *instance )
{
  if( d->m_pluginLoadingMode != DoNotLoadPlugins )
    Plugin::loadPlugins( parent, parentGUIClient, instance, d->m_pluginLoadingMode == LoadPlugins );
}

void PartBase::setPluginLoadingMode( PluginLoadingMode loadingMode )
{
    d->m_pluginLoadingMode = loadingMode;
}

Part::Part( TQObject *parent, const char* name )
 : TQObject( parent, name )
{
  d = new PartPrivate;
  m_widget = 0L;
  m_manager = 0L;
  PartBase::setPartObject( this );
}

Part::~Part()
{
  kdDebug(1000) << "Part::~Part " << this << endl;

  if ( m_widget )
  {
    // We need to disconnect first, to avoid calling it !
    disconnect( m_widget, TQ_SIGNAL( destroyed() ),
                this, TQ_SLOT( slotWidgetDestroyed() ) );
  }

  if ( m_manager )
    m_manager->removePart(this);

  if ( m_widget )
  {
    kdDebug(1000) << "deleting widget " << m_widget << " " << m_widget->name() << endl;
    delete (TQWidget*) m_widget;
  }

  delete d;
}

void Part::embed( TQWidget * parentWidget )
{
  if ( widget() )
    widget()->reparent( parentWidget, 0, TQPoint( 0, 0 ), true );
}

TQWidget *Part::widget()
{
  return m_widget;
}

void Part::setManager( PartManager *manager )
{
  m_manager = manager;
}

PartManager *Part::manager() const
{
  return m_manager;
}

Part *Part::hitTest( TQWidget *widget, const TQPoint & )
{
  if ( (TQWidget *)m_widget != widget )
    return 0L;

  return this;
}

void Part::setWidget( TQWidget *widget )
{
  assert ( !m_widget ); // otherwise we get two connects
  m_widget = widget;
  connect( m_widget, TQ_SIGNAL( destroyed() ),
           this, TQ_SLOT( slotWidgetDestroyed() ) );

  // Tell the actionCollection() which widget its
  //  action shortcuts should be connected to.
  actionCollection()->setWidget( widget );

  // Since KParts objects are XML-based, shortcuts should
  //  be connected to the widget when the XML settings
  //  are processed, rather than on TDEAction construction.
  actionCollection()->setAutoConnectShortcuts( false );
}

void Part::setSelectable( bool selectable )
{
  d->m_bSelectable = selectable;
}

bool Part::isSelectable() const
{
  return d->m_bSelectable;
}

void Part::customEvent( TQCustomEvent *event )
{
  if ( PartActivateEvent::test( event ) )
  {
    partActivateEvent( (PartActivateEvent *)event );
    return;
  }

  if ( PartSelectEvent::test( event ) )
  {
    partSelectEvent( (PartSelectEvent *)event );
    return;
  }

  if ( GUIActivateEvent::test( event ) )
  {
    guiActivateEvent( (GUIActivateEvent *)event );
    return;
  }

  TQObject::customEvent( event );
}

void Part::partActivateEvent( PartActivateEvent * )
{
}

void Part::partSelectEvent( PartSelectEvent * )
{
}

void Part::guiActivateEvent( GUIActivateEvent * )
{
}

TQWidget *Part::hostContainer( const TQString &containerName )
{
  if ( !factory() )
    return 0L;

  return factory()->container( containerName, this );
}

void Part::slotWidgetDestroyed()
{
  kdDebug(1000) << "KPart::slotWidgetDestroyed(), deleting part " << name() << endl;
  m_widget = 0;
  delete this;
}

//////////////////////////////////////////////////

namespace KParts
{

class ReadOnlyPartPrivate
{
public:
  ReadOnlyPartPrivate()
  {
    m_job = 0L;
    m_uploadJob = 0L;
    m_showProgressInfo = true;
    m_saveOk = false;
    m_waitForSave = false;
    m_duringSaveAs = false;
  }
  ~ReadOnlyPartPrivate()
  {
  }

  TDEIO::FileCopyJob * m_job;
  TDEIO::FileCopyJob * m_uploadJob;
  KURL m_originalURL; // for saveAs
  TQString m_originalFilePath; // for saveAs
  bool m_showProgressInfo : 1;
  bool m_saveOk : 1;
  bool m_waitForSave : 1;
  bool m_duringSaveAs : 1;
};

}

ReadOnlyPart::ReadOnlyPart( TQObject *parent, const char *name )
 : Part( parent, name ), m_bTemp( false )
{
  d = new ReadOnlyPartPrivate;
}

ReadOnlyPart::~ReadOnlyPart()
{
  ReadOnlyPart::closeURL();
  delete d;
}

void ReadOnlyPart::setProgressInfoEnabled( bool show )
{
  d->m_showProgressInfo = show;
}

bool ReadOnlyPart::isProgressInfoEnabled() const
{
  return d->m_showProgressInfo;
}

#ifndef KDE_NO_COMPAT
void ReadOnlyPart::showProgressInfo( bool show )
{
  d->m_showProgressInfo = show;
}
#endif

bool ReadOnlyPart::openURL( const KURL &url )
{
  if ( !url.isValid() )
    return false;
  if ( !closeURL() )
    return false;
  m_url = url;
  if ( m_url.isLocalFile() )
  {
    emit started( 0 );
    m_file = m_url.path();
    bool ret = openFile();
    if (ret)
    {
        emit completed();
        emit setWindowCaption( m_url.prettyURL() );
    };
    return ret;
  }
  else
  {
    m_bTemp = true;
    // Use same extension as remote file. This is important for mimetype-determination (e.g. koffice)
    TQString fileName = url.fileName();
    TQFileInfo fileInfo(fileName);
    TQString ext = fileInfo.extension();
    TQString extension;
    if ( !ext.isEmpty() && url.query().isNull() ) // not if the URL has a query, e.g. cgi.pl?something
        extension = "."+ext; // keep the '.'
    KTempFile tempFile( TQString::null, extension );
    m_file = tempFile.name();

    KURL destURL;
    destURL.setPath( m_file );
    d->m_job = TDEIO::file_copy( m_url, destURL, 0600, true, false, d->m_showProgressInfo );
    d->m_job->setWindow( widget() ? widget()->topLevelWidget() : 0 );
    emit started( d->m_job );
    connect( d->m_job, TQ_SIGNAL( result( TDEIO::Job * ) ), this, TQ_SLOT( slotJobFinished ( TDEIO::Job * ) ) );
    return true;
  }
}

void ReadOnlyPart::abortLoad()
{
  if ( d->m_job )
  {
    //kdDebug(1000) << "Aborting job " << d->m_job << endl;
    d->m_job->kill();
    d->m_job = 0;
  }
}

bool ReadOnlyPart::closeURL()
{
  abortLoad(); //just in case

  if ( m_bTemp )
  {
    unlink( TQFile::encodeName(m_file) );
    m_bTemp = false;
  }
  // It always succeeds for a read-only part,
  // but the return value exists for reimplementations
  // (e.g. pressing cancel for a modified read-write part)
  return true;
}

void ReadOnlyPart::slotJobFinished( TDEIO::Job * job )
{
  kdDebug(1000) << "ReadOnlyPart::slotJobFinished" << endl;
  assert( job == d->m_job );
  d->m_job = 0;
  if (job->error())
    emit canceled( job->errorString() );
  else
  {
    if ( openFile() )
      emit setWindowCaption( m_url.prettyURL() );
    emit completed();
  }
}

void ReadOnlyPart::guiActivateEvent( GUIActivateEvent * event )
{
  if (event->activated())
  {
    if (!m_url.isEmpty())
    {
      kdDebug(1000) << "ReadOnlyPart::guiActivateEvent -> " << m_url.prettyURL() << endl;
      emit setWindowCaption( m_url.prettyURL() );
    } else emit setWindowCaption( "" );
  }
}

bool ReadOnlyPart::openStream( const TQString& mimeType, const KURL& url )
{
    if ( !closeURL() )
        return false;
    m_url = url;
    return doOpenStream( mimeType );
}

bool ReadOnlyPart::writeStream( const TQByteArray& data )
{
    return doWriteStream( data );
}

bool ReadOnlyPart::closeStream()
{
    return doCloseStream();
}

//////////////////////////////////////////////////

ReadWritePart::ReadWritePart( TQObject *parent, const char *name )
 : ReadOnlyPart( parent, name ), m_bModified( false ), m_bClosing( false )
{
  m_bReadWrite = true;
}

ReadWritePart::~ReadWritePart()
{
  // parent destructor will delete temp file
  // we can't call our own closeURL() here, because
  // "cancel" wouldn't cancel anything. We have to assume
  // the app called closeURL() before destroying us.
}

void ReadWritePart::setReadWrite( bool readwrite )
{
  // Perhaps we should check isModified here and issue a warning if true
  m_bReadWrite = readwrite;
}

void ReadWritePart::setModified( bool modified )
{
  kdDebug(1000) << "ReadWritePart::setModified( " << (modified ? "true" : "false") << ")" << endl;
  if ( !m_bReadWrite && modified )
  {
      kdError(1000) << "Can't set a read-only document to 'modified' !" << endl;
      return;
  }
  m_bModified = modified;
}

void ReadWritePart::setModified()
{
  setModified( true );
}

bool ReadWritePart::queryClose()
{
  if ( !isReadWrite() || !isModified() )
    return true;

  TQString docName = url().fileName();
  if (docName.isEmpty()) docName = i18n( "Untitled" );

  int res = KMessageBox::warningYesNoCancel( widget(),
          i18n( "The document \"%1\" has been modified.\n"
                "Do you want to save your changes or discard them?" ).arg( docName ),
          i18n( "Close Document" ), KStdGuiItem::save(), KStdGuiItem::discard() );

  bool abortClose=false;
  bool handled=false;

  switch(res) {
  case KMessageBox::Yes :
    sigQueryClose(&handled,&abortClose);
    if (!handled)
    {
      if (m_url.isEmpty())
      {
          KURL url = KFileDialog::getSaveURL();
          if (url.isEmpty())
            return false;

          saveAs( url );
      }
      else
      {
          save();
      }
    } else if (abortClose) return false;
    return waitSaveComplete();
  case KMessageBox::No :
    return true;
  default : // case KMessageBox::Cancel :
    return false;
  }
}

bool ReadWritePart::closeURL()
{
  abortLoad(); //just in case
  if ( isReadWrite() && isModified() )
  {
    if (!queryClose())
       return false;
  }
  // Not modified => ok and delete temp file.
  return ReadOnlyPart::closeURL();
}

bool ReadWritePart::closeURL( bool promptToSave )
{
  return promptToSave ? closeURL() : ReadOnlyPart::closeURL();
}

bool ReadWritePart::save()
{
  d->m_saveOk = false;
  if ( m_file.isEmpty() ) // document was created empty
      prepareSaving();
  if( saveFile() )
    return saveToURL();
  else
    emit canceled(TQString::null);
  return false;
}

bool ReadWritePart::saveAs( const KURL & kurl )
{
  if (!kurl.isValid())
  {
      kdError(1000) << "saveAs: Malformed URL " << kurl.url() << endl;
      return false;
  }
  d->m_duringSaveAs = true;
  d->m_originalURL = m_url;
  d->m_originalFilePath = m_file;
  m_url = kurl; // Store where to upload in saveToURL
  prepareSaving();
  bool result = save(); // Save local file and upload local file
  if (result)
    emit setWindowCaption( m_url.prettyURL() );
  else
  {
    m_url = d->m_originalURL;
    m_file = d->m_originalFilePath;
    d->m_duringSaveAs = false;
    d->m_originalURL = KURL();
    d->m_originalFilePath = TQString::null;
  }

  return result;
}

// Set m_file correctly for m_url
void ReadWritePart::prepareSaving()
{
  // Local file
  if ( m_url.isLocalFile() )
  {
    if ( m_bTemp ) // get rid of a possible temp file first
    {              // (happens if previous url was remote)
      unlink( TQFile::encodeName(m_file) );
      m_bTemp = false;
    }
    m_file = m_url.path();
  }
  else
  { // Remote file
    // We haven't saved yet, or we did but locally - provide a temp file
    if ( m_file.isEmpty() || !m_bTemp )
    {
      KTempFile tempFile;
      m_file = tempFile.name();
      m_bTemp = true;
    }
    // otherwise, we already had a temp file
  }
}

bool ReadWritePart::saveToURL()
{
  if ( m_url.isLocalFile() )
  {
    setModified( false );
    emit completed();
    // if m_url is a local file there won't be a temp file -> nothing to remove
    assert( !m_bTemp );
    d->m_saveOk = true;
    d->m_duringSaveAs = false;
    d->m_originalURL = KURL();
    d->m_originalFilePath = TQString::null;
    return true; // Nothing to do
  }
  else
  {
    if (d->m_uploadJob)
    {
       unlink(TQFile::encodeName(d->m_uploadJob->srcURL().path()));
       d->m_uploadJob->kill();
       d->m_uploadJob = 0;
    }
    KTempFile tempFile;
    TQString uploadFile = tempFile.name();
    KURL uploadUrl;
    uploadUrl.setPath( uploadFile );
    tempFile.unlink();
    // Create hardlink
    if (::link(TQFile::encodeName(m_file), TQFile::encodeName(uploadFile)) != 0)
    {
       // Uh oh, some error happened.
       return false;
    }
    d->m_uploadJob = TDEIO::file_move( uploadUrl, m_url, -1, true /*overwrite*/ );
    d->m_uploadJob->setWindow( widget() ? widget()->topLevelWidget() : 0 );
    connect( d->m_uploadJob, TQ_SIGNAL( result( TDEIO::Job * ) ), this, TQ_SLOT( slotUploadFinished (TDEIO::Job *) ) );
    return true;
  }
}

void ReadWritePart::slotUploadFinished( TDEIO::Job * )
{
  if (d->m_uploadJob->error())
  {
    unlink(TQFile::encodeName(d->m_uploadJob->srcURL().path()));
    TQString error = d->m_uploadJob->errorString();
    d->m_uploadJob = 0;
    if (d->m_duringSaveAs) {
      m_url = d->m_originalURL;
      m_file = d->m_originalFilePath;
    }
    emit canceled( error );
  }
  else
  {
    KDirNotify_stub allDirNotify("*", "KDirNotify*");
    KURL dirUrl( m_url );
    dirUrl.setPath( dirUrl.directory() );
    allDirNotify.FilesAdded( dirUrl );

    d->m_uploadJob = 0;
    setModified( false );
    emit completed();
    d->m_saveOk = true;
  }
  d->m_duringSaveAs = false;
  d->m_originalURL = KURL();
  d->m_originalFilePath = TQString::null;
  if (d->m_waitForSave)
  {
     tqApp->exit_loop();
  }
}

// Trolls: Nothing to see here, please step away.
void tqt_enter_modal( TQWidget *widget );
void tqt_leave_modal( TQWidget *widget );

bool ReadWritePart::waitSaveComplete()
{
  if (!d->m_uploadJob)
     return d->m_saveOk;

  d->m_waitForSave = true;

  TQWidget dummy(0,0,(WFlags)(WType_Dialog | WShowModal));
  dummy.setFocusPolicy( TQWidget::NoFocus );
  tqt_enter_modal(&dummy);
  tqApp->enter_loop();
  tqt_leave_modal(&dummy);

  d->m_waitForSave = false;

  return d->m_saveOk;
}

#include "part.moc"

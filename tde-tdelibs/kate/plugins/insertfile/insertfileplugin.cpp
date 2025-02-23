/* This file is part of the KDE libraries
   Copyright (C) 2002 Anders Lund <anders@alweb.dk>

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

#include "insertfileplugin.h"
#include "insertfileplugin.moc"

#include <tdetexteditor/document.h>
#include <tdetexteditor/viewcursorinterface.h>
#include <tdetexteditor/editinterface.h>

#include <assert.h>
#include <tdeio/job.h>
#include <tdeaction.h>
#include <tdefiledialog.h>
#include <kgenericfactory.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kpushbutton.h>
#include <tdetempfile.h>
#include <kurl.h>

#include <tqfile.h>
#include <tqtextstream.h>

K_EXPORT_COMPONENT_FACTORY( tdetexteditor_insertfile, KGenericFactory<InsertFilePlugin>( "tdetexteditor_insertfile" ) )


//BEGIN InsertFilePlugin
InsertFilePlugin::InsertFilePlugin( TQObject *parent, const char* name, const TQStringList& )
	: KTextEditor::Plugin ( (KTextEditor::Document*) parent, name )
{
}

InsertFilePlugin::~InsertFilePlugin()
{
}

void InsertFilePlugin::addView(KTextEditor::View *view)
{
  InsertFilePluginView *nview = new InsertFilePluginView (view, "Insert File Plugin");
  m_views.append (nview);
}

void InsertFilePlugin::removeView(KTextEditor::View *view)
{
  for (uint z=0; z < m_views.count(); z++)
    if (m_views.at(z)->parentClient() == view)
    {
       InsertFilePluginView *nview = m_views.at(z);
       m_views.remove (nview);
       delete nview;
    }
}
//END InsertFilePlugin

//BEGIN InsertFilePluginView
InsertFilePluginView::InsertFilePluginView( KTextEditor::View *view, const char *name )
  : TQObject( view, name ),
    KXMLGUIClient( view )
{
  view->insertChildClient( this );
  setInstance( KGenericFactory<InsertFilePlugin>::instance() );
  _job = 0;
  (void) new TDEAction( i18n("Insert File..."), 0, this, TQ_SLOT(slotInsertFile()), actionCollection(), "tools_insert_file" );
  setXMLFile( "tdetexteditor_insertfileui.rc" );
}

void InsertFilePluginView::slotInsertFile()
{
  KFileDialog dlg("::insertfile", "", (TQWidget*)parent(), "filedialog", true);
  dlg.setOperationMode( KFileDialog::Opening );

  dlg.setCaption(i18n("Choose File to Insert"));
  dlg.okButton()->setText(i18n("&Insert"));
  dlg.setMode( KFile::File );
  dlg.exec();

  _file = dlg.selectedURL().url();
  if ( _file.isEmpty() ) return;

  if ( _file.isLocalFile() ) {
    _tmpfile = _file.path();
    insertFile();
  }
  else {
    KTempFile tempFile( TQString::null );
    _tmpfile = tempFile.name();

    KURL destURL;
    destURL.setPath( _tmpfile );
    _job = TDEIO::file_copy( _file, destURL, 0600, true, false, true );
    connect( _job, TQ_SIGNAL( result( TDEIO::Job * ) ), this, TQ_SLOT( slotFinished ( TDEIO::Job * ) ) );
  }
}

void InsertFilePluginView::slotFinished( TDEIO::Job *job )
{
  assert( job == _job );
  _job = 0;
  if ( job->error() )
    KMessageBox::error( (TQWidget*)parent(), i18n("Failed to load file:\n\n") + job->errorString(), i18n("Insert File Error") );
  else
    insertFile();
}

void InsertFilePluginView::insertFile()
{
  TQString error;
  if ( _tmpfile.isEmpty() )
    return;

  TQFileInfo fi;
  fi.setFile( _tmpfile );
  if (!fi.exists() || !fi.isReadable())
    error = i18n("<p>The file <strong>%1</strong> does not exist or is not readable, aborting.").arg(_file.fileName());

  TQFile f( _tmpfile );
  if ( !f.open(IO_ReadOnly) )
    error = i18n("<p>Unable to open file <strong>%1</strong>, aborting.").arg(_file.fileName());

  if ( ! error.isEmpty() ) {
    KMessageBox::sorry( (TQWidget*)parent(), error, i18n("Insert File Error") );
    return;
  }

  // now grab file contents
  TQTextStream stream(&f);
  TQString str, tmp;
  uint numlines = 0;
  uint len = 0;
  while (!stream.eof()) {
    if ( numlines )
      str += "\n";
    tmp = stream.readLine();
    str += tmp;
    len = tmp.length();
    numlines++;
  }
  f.close();

  if ( str.isEmpty() )
    error = i18n("<p>File <strong>%1</strong> had no contents.").arg(_file.fileName());
  if ( ! error.isEmpty() ) {
    KMessageBox::sorry( (TQWidget*)parent(), error, i18n("Insert File Error") );
    return;
  }

  // insert !!
  KTextEditor::EditInterface *ei;
  KTextEditor::ViewCursorInterface *ci;
  KTextEditor::View *v = (KTextEditor::View*)parent();
  ei = KTextEditor::editInterface( v->document() );
  ci = KTextEditor::viewCursorInterface( v );
  uint line, col;
  ci->cursorPositionReal( &line, &col );
  ei->insertText( line, col, str );

  // move the cursor
  ci->setCursorPositionReal( line + numlines - 1, numlines > 1 ? len : col + len  );

  // clean up
  _file = KURL ();
  _tmpfile.truncate( 0 );
  v = 0;
  ei = 0;
  ci = 0;
}

//END InsertFilePluginView


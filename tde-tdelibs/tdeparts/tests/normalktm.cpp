
#include "normalktm.h"
#include "parts.h"
#include "notepad.h"

#include <tqsplitter.h>
#include <tqcheckbox.h>
#include <tqdir.h>

#include <kiconloader.h>
#include <kstandarddirs.h>
#include <tdeapplication.h>
#include <tdemessagebox.h>
#include <tdeaction.h>
#include <tdelocale.h>

#include <tdemenubar.h>

Shell::Shell()
{
  // We can do this "switch active part" because we have a splitter with
  // two items in it.
  // I wonder what tdevelop uses/will use to embed kedit, BTW.
  m_splitter = new TQSplitter( this );

  m_part1 = new Part1(this, m_splitter);
  m_part2 = new Part2(this, m_splitter);

  TQPopupMenu * pFile = new TQPopupMenu( this );
  menuBar()->insertItem( "File", pFile );
  TQObject * coll = this;
  TDEAction * paLocal = new TDEAction( "&View local file", 0, this, TQ_SLOT( slotFileOpen() ), coll, "open_local_file" );
  // No XML : we need to plug our actions ourselves
  paLocal->plug( pFile );

  TDEAction * paRemote = new TDEAction( "&View remote file", 0, this, TQ_SLOT( slotFileOpenRemote() ), coll, "open_remote_file" );
  paRemote->plug( pFile );

  m_paEditFile = new TDEAction( "&Edit file", 0, this, TQ_SLOT( slotFileEdit() ), coll, "edit_file" );
  m_paEditFile->plug( pFile );

  m_paCloseEditor = new TDEAction( "&Close file editor", 0, this, TQ_SLOT( slotFileCloseEditor() ), coll, "close_editor" );
  m_paCloseEditor->setEnabled(false);
  m_paCloseEditor->plug( pFile );

  TDEAction * paQuit = new TDEAction( "&Quit", 0, this, TQ_SLOT( close() ), coll, "shell_quit" );
  paQuit->setIconSet(TQIconSet(BarIcon("system-log-out")));
  paQuit->plug( pFile );

  setCentralWidget( m_splitter );
  m_splitter->setMinimumSize( 400, 300 );

  m_splitter->show();

  m_editorpart = 0;
}

Shell::~Shell()
{
}

void Shell::slotFileOpen()
{
  if ( ! m_part1->openURL( locate("data", TDEGlobal::instance()->instanceName()+"/tdepartstest_shell.rc" ) ) )
    KMessageBox::error(this,"Couldn't open file !");
}

void Shell::slotFileOpenRemote()
{
  KURL u ( "http://www.kde.org/index.html" );
  if ( ! m_part1->openURL( u ) )
    KMessageBox::error(this,"Couldn't open file !");
}

void Shell::embedEditor()
{
  // replace part2 with the editor part
  delete m_part2;
  m_part2 = 0L;
  m_editorpart = new NotepadPart( m_splitter, "editor", 
                                  this, "NotepadPart" );
  m_editorpart->setReadWrite(); // read-write mode
  ////// m_manager->addPart( m_editorpart );
  m_editorpart->widget()->show(); //// we need to do this in a normal KTM....
  m_paEditFile->setEnabled(false);
  m_paCloseEditor->setEnabled(true);
}

void Shell::slotFileCloseEditor()
{
  delete m_editorpart;
  m_editorpart = 0L;
  m_part2 = new Part2(this, m_splitter);
  ////// m_manager->addPart( m_part2 );
  m_part2->widget()->show(); //// we need to do this in a normal KTM....
  m_paEditFile->setEnabled(true);
  m_paCloseEditor->setEnabled(false);
}

void Shell::slotFileEdit()
{
  if ( !m_editorpart )
    embedEditor();
  // TODO use KFileDialog to allow testing remote files
  if ( ! m_editorpart->openURL( TQDir::current().absPath()+"/tdepartstest_shell.rc" ) )
    KMessageBox::error(this,"Couldn't open file !");
}

int main( int argc, char **argv )
{
  TDEApplication app( argc, argv, "tdepartstest" ); // we cheat and call ourselves tdepartstest for Shell::slotFileOpen()

  Shell *shell = new Shell;

  shell->show();

  app.exec();

  return 0;
}

#include "normalktm.moc"

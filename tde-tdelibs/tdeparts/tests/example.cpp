
#include "example.h"
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

Shell::Shell()
{
  setXMLFile( "tdepartstest_shell.rc" );

  m_manager = new KParts::PartManager( this );

  // When the manager says the active part changes, the builder updates (recreates) the GUI
  connect( m_manager, TQ_SIGNAL( activePartChanged( KParts::Part * ) ),
           this, TQ_SLOT( createGUI( KParts::Part * ) ) );

  // We can do this "switch active part" because we have a splitter with
  // two items in it.
  // I wonder what tdevelop uses/will use to embed kedit, BTW.
  m_splitter = new TQSplitter( this );

  m_part1 = new Part1(this, m_splitter);
  m_part2 = new Part2(this, m_splitter);

  TDEActionCollection *coll = actionCollection();

  (void)new TDEAction( "&View local file", 0, this, TQ_SLOT( slotFileOpen() ), coll, "open_local_file" );
  (void)new TDEAction( "&View remote file", 0, this, TQ_SLOT( slotFileOpenRemote() ), coll, "open_remote_file" );

  m_paEditFile = new TDEAction( "&Edit file", 0, this, TQ_SLOT( slotFileEdit() ), coll, "edit_file" );
  m_paCloseEditor = new TDEAction( "&Close file editor", 0, this, TQ_SLOT( slotFileCloseEditor() ), coll, "close_editor" );
  m_paCloseEditor->setEnabled(false);
  TDEAction * paQuit = new TDEAction( "&Quit", 0, this, TQ_SLOT( close() ), coll, "shell_quit" );
  paQuit->setIconSet(TQIconSet(BarIcon("system-log-out")));

  (void)new TDEAction( "Yet another menu item", 0, coll, "shell_yami" );
  (void)new TDEAction( "Yet another submenu item", 0, coll, "shell_yasmi" );

  setCentralWidget( m_splitter );
  m_splitter->setMinimumSize( 400, 300 );

  m_splitter->show();

  m_manager->addPart( m_part1, true ); // sets part 1 as the active part
  m_manager->addPart( m_part2, false );
  m_editorpart = 0;
}

Shell::~Shell()
{
  disconnect( m_manager, 0, this, 0 );
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
  if ( m_manager->activePart() == m_part2 )
    createGUI( 0L );

  // replace part2 with the editor part
  delete m_part2;
  m_part2 = 0L;
  m_editorpart = new NotepadPart( m_splitter, "editor", 
                                  this, "NotepadPart" );
  m_editorpart->setReadWrite(); // read-write mode
  m_manager->addPart( m_editorpart );
  m_paEditFile->setEnabled(false);
  m_paCloseEditor->setEnabled(true);
}

void Shell::slotFileCloseEditor()
{
  // It is very important to close the url of a read-write part
  // before destroying it. This allows to save the document (if modified)
  // and also to cancel.
  if ( ! m_editorpart->closeURL() )
    return;

  // Is this necessary ? (David)
  if ( m_manager->activePart() == m_editorpart )
    createGUI( 0L );

  delete m_editorpart;
  m_editorpart = 0L;
  m_part2 = new Part2(this, m_splitter);
  m_manager->addPart( m_part2 );
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
  TDEApplication app( argc, argv, "tdepartstest" );

  Shell *shell = new Shell;

  shell->show();

  app.exec();

  return 0;
}

#include "example.moc"

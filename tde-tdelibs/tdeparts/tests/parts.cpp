
#include <tdeparts/event.h>

#include "parts.h"

#include <tqcheckbox.h>
#include <tqfile.h>
#include <tqdir.h>
#include <tqtextstream.h>
#include <tqmultilineedit.h>
#include <tqlineedit.h>
#include <tqvbox.h>

#include <kiconloader.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdemessagebox.h>
#include <tdeaction.h>
#include <tdelocale.h>

Part1::Part1( TQObject *parent, TQWidget * parentWidget )
 : KParts::ReadOnlyPart( parent, "Part1" )
{
  m_instance = new TDEInstance( "tdepartstestpart" );
  setInstance( m_instance );
  m_edit = new TQMultiLineEdit( parentWidget );
  setWidget( m_edit );
  setXMLFile( "tdepartstest_part1.rc" );

  /*TDEAction * paBlah = */ new TDEAction( "Blah", "filemail", 0, actionCollection(), "p1_blah" );
}

Part1::~Part1()
{
  delete m_instance;
}

bool Part1::openFile()
{
  kdDebug() << "Part1: opening " << TQFile::encodeName(m_file) << endl;
  // Hehe this is from a tutorial I did some time ago :)
  TQFile f(m_file);
  TQString s;
  if ( f.open(IO_ReadOnly) ) {
    TQTextStream t( &f );
    while ( !t.eof() ) {
      s += t.readLine() + "\n";
    }
    f.close();
  } else
    return false;
  m_edit->setText(s);

  emit setStatusBarText( m_url.prettyURL() );

  return true;
}

Part2::Part2( TQObject *parent, TQWidget * parentWidget )
 : KParts::Part( parent, "Part2" )
{
  m_instance = new TDEInstance( "part2" );
  setInstance( m_instance );
  TQWidget * w = new TQWidget( parentWidget, "Part2Widget" );
  setWidget( w );

  TQCheckBox * cb = new TQCheckBox( "something", w );

  TQLineEdit * l = new TQLineEdit( "something", widget() );
  l->move(0,50);
  // Since the main widget is a dummy one, we HAVE to set
  // strong focus for it, otherwise we get the
  // the famous activating-file-menu-switches-part bug.
  w->setFocusPolicy( TQWidget::ClickFocus );

  // setXMLFile( ... ); // no actions currently
}

Part2::~Part2()
{
  delete m_instance;
}

void Part2::guiActivateEvent( KParts::GUIActivateEvent * event )
{
  if (event->activated())
    emit setWindowCaption("[part2 activated]");
}

#include "parts.moc"

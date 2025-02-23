
#include "notepad.h"
#include <tdeparts/partmanager.h>
#include <tdeparts/mainwindow.h>

#include <tqsplitter.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqmultilineedit.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeaction.h>
#include <tdelocale.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>

NotepadPart::NotepadPart( TQWidget* parentWidget, const char*,
                          TQObject* parent, const char* name,
                          const TQStringList& )
 : KParts::ReadWritePart( parent, name )
{
  setInstance( NotepadFactory::instance() );

  m_edit = new TQMultiLineEdit( parentWidget, "NotepadPart's multiline edit" );
  setWidget( m_edit );

  (void)new TDEAction( "Search and replace", 0, this, TQ_SLOT( slotSearchReplace() ), actionCollection(), "searchreplace" );
  setXMLFile( "notepadpart.rc" );
  setReadWrite( true );
}

NotepadPart::~NotepadPart()
{
}

void NotepadPart::setReadWrite( bool rw )
{
    m_edit->setReadOnly( !rw );
    if (rw)
        connect( m_edit, TQ_SIGNAL( textChanged() ), this, TQ_SLOT( setModified() ) );
    else
        disconnect( m_edit, TQ_SIGNAL( textChanged() ), this, TQ_SLOT( setModified() ) );

    ReadWritePart::setReadWrite( rw );
}

TDEAboutData* NotepadPart::createAboutData()
{
  return new TDEAboutData( "notepadpart", I18N_NOOP( "Notepad" ), "2.0" );
}

bool NotepadPart::openFile()
{
  kdDebug() << "NotepadPart: opening " << m_file << endl;
  // Hehe this is from a tutorial I did some time ago :)
  TQFile f(m_file);
  TQString s;
  if ( f.open(IO_ReadOnly) ) {
    TQTextStream t( &f );
    while ( !t.eof() ) {
      s += t.readLine() + "\n";
    }
    f.close();
  }
  m_edit->setText(s);

  emit setStatusBarText( m_url.prettyURL() );

  return true;
}

bool NotepadPart::saveFile()
{
  if ( !isReadWrite() )
    return false;
  TQFile f(m_file);
  TQString s;
  if ( f.open(IO_WriteOnly) ) {
    TQTextStream t( &f );
    t << m_edit->text();
    f.close();
    return true;
  } else
    return false;
}

void NotepadPart::slotSearchReplace()
{
  // What's this ? (David)
/*
  TQValueList<KParts::XMLGUIServant *> plugins = KParts::Plugin::pluginServants( this );
  TQValueList<KParts::XMLGUIServant *>::ConstIterator it = plugins.begin();
  TQValueList<KParts::XMLGUIServant *>::ConstIterator end = plugins.end();
  for (; it != end; ++it )
    factory()->removeServant( *it );
*/
}

#include "notepad.moc"

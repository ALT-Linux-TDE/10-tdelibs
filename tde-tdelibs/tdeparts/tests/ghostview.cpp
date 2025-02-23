#include <kiconloader.h>
#include <kstandarddirs.h>
#include <tdeapplication.h>
#include <tdeaction.h>
#include <tdelocale.h>
#include <tdefiledialog.h>
#include <tdemessagebox.h>
#include <tdecmdlineargs.h>
#include <klibloader.h>

#include <tqwidget.h>
#include <tqdir.h>
#include <tqfile.h>

#include <ktrader.h>

#include "ghostview.h"

Shell::Shell()
{
  setXMLFile( "ghostviewtest_shell.rc" );

  TDEAction * paOpen = new TDEAction( "&Open file" , "document-open", 0, this,
    TQ_SLOT( slotFileOpen() ), actionCollection(), "file_open" );

  TDEAction * paQuit = new TDEAction( "&Quit" , "system-log-out", 0, this, TQ_SLOT( close() ), actionCollection(), "file_quit" );

  // Try to find a postscript component first
  TDETrader::OfferList offers = TDETrader::self()->query("application/postscript", "('KParts/ReadOnlyPart' in ServiceTypes) or ('Browser/View' in ServiceTypes)");

  KLibFactory *factory = 0;
  m_gvpart = 0;
  TDETrader::OfferList::Iterator it(offers.begin());
  for( ; it != offers.end(); ++it)
  {
    KService::Ptr ptr = (*it);

    factory = KLibLoader::self()->factory( TQFile::encodeName(ptr->library()) );
    if (factory)
    {
      m_gvpart = static_cast<KParts::ReadOnlyPart *>(factory->create(this, ptr->name().latin1(), "KParts::ReadOnlyPart"));
      setCentralWidget( m_gvpart->widget() );
      // Integrate its GUI
      createGUI( m_gvpart );

      break;
    }
  }

  // if we couldn't find a component with the trader, try the
  // kghostview library directly.  if this ever happens, then something
  // is seriously screwed up, though -- the kghostview component
  // should be picked up by the trader
  if (!m_gvpart)
  {
    factory = KLibLoader::self()->factory( "libkghostview" );
    if (factory)
    {
      // Create the part
      m_gvpart = (KParts::ReadOnlyPart *)factory->create( this, "kgvpart",
                 "KParts::ReadOnlyPart" );
      // Set the main widget
      setCentralWidget( m_gvpart->widget() );
      // Integrate its GUI
      createGUI( m_gvpart );
    }
    else
    {
       KMessageBox::error(this, "No libkghostview found !");
    }
  }
  // Set a reasonable size
  resize( 600, 350 );
}

Shell::~Shell()
{
  delete m_gvpart;
}

void Shell::openURL( const KURL & url )
{
  m_gvpart->openURL( url );
}

void Shell::slotFileOpen()
{
  KURL url = KFileDialog::getOpenURL( TQString::null, "*.ps|Postscript files (*.ps)", 0L, "file dialog" );

  if( !url.isEmpty() )
     openURL( url );
}

static TDECmdLineOptions options[] =
{
 { "+file(s)",          "Files to load", 0 },
 TDECmdLineLastOption
};
static const char version[] = "v0.0.1 2000 (c) David Faure";
static const char description[] = "This is a test shell for the kghostview part.";

int main( int argc, char **argv )
{
  TDECmdLineArgs::init(argc, argv, "ghostviewtest", description, version);
  TDECmdLineArgs::addCmdLineOptions( options ); // Add my own options.
  TDEApplication app;
  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
  Shell *shell = new Shell;
  if ( args->count() == 1 )
  {
    // Allow full paths, but also simple filenames from current dir
    KURL url( TQDir::currentDirPath()+"/", args->arg(0) );
    shell->openURL( url );
  }
  shell->show();
  return app.exec();
}

#include "ghostview.moc"

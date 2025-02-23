/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "helpwindow.h"
#include "richpage.h"
#include <tqstatusbar.h>
#include <tqpixmap.h>
#include <tqpopupmenu.h>
#include <tdemenubar.h>
#include <tdetoolbar.h>
#include <kstatusbar.h>
#include <tqtoolbutton.h>
#include <tqiconset.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqstylesheet.h>
#include <tqmessagebox.h>
#include <tqfiledialog.h>
#include <tqapplication.h>
#include <tqcombobox.h>
#include <tqevent.h>
#include <tqlineedit.h>
#include <tqobjectlist.h>
#include <tqfileinfo.h>
#include <tqdatastream.h>
#include <kprinter.h>
#include <tqsimplerichtext.h>
#include <tqpainter.h>
#include <tqpaintdevicemetrics.h>

#include <ctype.h>

HelpWindow::HelpWindow( const TQString& home_, const TQString& _path,
			TQWidget* parent, const char *name )
    : TDEMainWindow( parent, name, WDestructiveClose ),
      pathCombo( 0 ), selectedURL()
{
    readHistory();
    readBookmarks();

    browser = new TQTextBrowser( this );
    browser->mimeSourceFactory()->setFilePath( _path );
    browser->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
    connect( browser, TQ_SIGNAL( textChanged() ),
	     this, TQ_SLOT( textChanged() ) );

    setCentralWidget( browser );

    if ( !home_.isEmpty() )
	browser->setSource( home_ );

    connect( browser, TQ_SIGNAL( highlighted( const TQString&) ),
	     statusBar(), TQ_SLOT( message( const TQString&)) );

    resize( 640,700 );

    TQPopupMenu* file = new TQPopupMenu( this );
    file->insertItem( "&New Window", this, TQ_SLOT( newWindow() ), ALT | Key_N );
    file->insertItem( "&Open File", this, TQ_SLOT( openFile() ), ALT | Key_O );
    file->insertItem( "&Print", this, TQ_SLOT( print() ), ALT | Key_P );
    file->insertSeparator();
    file->insertItem( "&Close", this, TQ_SLOT( close() ), ALT | Key_Q );
    file->insertItem( "E&xit", tqApp, TQ_SLOT( closeAllWindows() ), ALT | Key_X );

    // The same three icons are used twice each.
    TQIconSet icon_back( TQPixmap("back.xpm") );
    TQIconSet icon_forward( TQPixmap("forward.xpm") );
    TQIconSet icon_home( TQPixmap("home.xpm") );

    TQPopupMenu* go = new TQPopupMenu( this );
    backwardId = go->insertItem( icon_back,
				 "&Backward", browser, TQ_SLOT( backward() ),
				 ALT | Key_Left );
    forwardId = go->insertItem( icon_forward,
				"&Forward", browser, TQ_SLOT( forward() ),
				ALT | Key_Right );
    go->insertItem( icon_home, "&Home", browser, TQ_SLOT( home() ) );

    TQPopupMenu* help = new TQPopupMenu( this );
    help->insertItem( "&About ...", this, TQ_SLOT( about() ) );
    help->insertItem( "About &Qt ...", this, TQ_SLOT( aboutQt() ) );

    hist = new TQPopupMenu( this );
    TQStringList::Iterator it = history.begin();
    for ( ; it != history.end(); ++it )
	mHistory[ hist->insertItem( *it ) ] = *it;
    connect( hist, TQ_SIGNAL( activated( int ) ),
	     this, TQ_SLOT( histChosen( int ) ) );

    bookm = new TQPopupMenu( this );
    bookm->insertItem( tr( "Add Bookmark" ), this, TQ_SLOT( addBookmark() ) );
    bookm->insertSeparator();

    TQStringList::Iterator it2 = bookmarks.begin();
    for ( ; it2 != bookmarks.end(); ++it2 )
	mBookmarks[ bookm->insertItem( *it2 ) ] = *it2;
    connect( bookm, TQ_SIGNAL( activated( int ) ),
	     this, TQ_SLOT( bookmChosen( int ) ) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertItem( tr("&Go"), go );
    menuBar()->insertItem( tr( "History" ), hist );
    menuBar()->insertItem( tr( "Bookmarks" ), bookm );
    menuBar()->insertSeparator();
    menuBar()->insertItem( tr("&Help"), help );

    menuBar()->setItemEnabled( forwardId, false);
    menuBar()->setItemEnabled( backwardId, false);
    connect( browser, TQ_SIGNAL( backwardAvailable( bool ) ),
	     this, TQ_SLOT( setBackwardAvailable( bool ) ) );
    connect( browser, TQ_SIGNAL( forwardAvailable( bool ) ),
	     this, TQ_SLOT( setForwardAvailable( bool ) ) );


    TQToolBar* toolbar = new TQToolBar( this );
    addToolBar( toolbar, "Toolbar");
    TQToolButton* button;

    button = new TQToolButton( icon_back, tr("Backward"), "", browser, TQ_SLOT(backward()), toolbar );
    connect( browser, TQ_SIGNAL( backwardAvailable(bool) ), button, TQ_SLOT( setEnabled(bool) ) );
    button->setEnabled( false );
    button = new TQToolButton( icon_forward, tr("Forward"), "", browser, TQ_SLOT(forward()), toolbar );
    connect( browser, TQ_SIGNAL( forwardAvailable(bool) ), button, TQ_SLOT( setEnabled(bool) ) );
    button->setEnabled( false );
    button = new TQToolButton( icon_home, tr("Home"), "", browser, TQ_SLOT(home()), toolbar );

    toolbar->addSeparator();

    pathCombo = new TQComboBox( true, toolbar );
    connect( pathCombo, TQ_SIGNAL( activated( const TQString & ) ),
	     this, TQ_SLOT( pathSelected( const TQString & ) ) );
    toolbar->setStretchableWidget( pathCombo );
    setRightJustification( true );
    setDockEnabled( DockLeft, false );
    setDockEnabled( DockRight, false );

    pathCombo->insertItem( home_ );

    browser->setFocus();
}


void HelpWindow::setBackwardAvailable( bool b)
{
    menuBar()->setItemEnabled( backwardId, b);
}

void HelpWindow::setForwardAvailable( bool b)
{
    menuBar()->setItemEnabled( forwardId, b);
}


void HelpWindow::textChanged()
{
    if ( browser->documentTitle().isNull() )
	setCaption( browser->context() );
    else
	setCaption( browser->documentTitle() ) ;

    selectedURL = caption();
    if ( !selectedURL.isEmpty() && pathCombo ) {
	bool exists = false;
	int i;
	for ( i = 0; i < pathCombo->count(); ++i ) {
	    if ( pathCombo->text( i ) == selectedURL ) {
		exists = true;
		break;
	    }
	}
	if ( !exists ) {
	    pathCombo->insertItem( selectedURL, 0 );
	    pathCombo->setCurrentItem( 0 );
	    mHistory[ hist->insertItem( selectedURL ) ] = selectedURL;
	} else
	    pathCombo->setCurrentItem( i );
	selectedURL = TQString::null;
    }
}

HelpWindow::~HelpWindow()
{
    history.clear();
    TQMap<int, TQString>::Iterator it = mHistory.begin();
    for ( ; it != mHistory.end(); ++it )
	history.append( *it );

    TQFile f( TQDir::currentDirPath() + "/.history" );
    f.open( IO_WriteOnly );
    TQDataStream s( &f );
    s << history;
    f.close();

    bookmarks.clear();
    TQMap<int, TQString>::Iterator it2 = mBookmarks.begin();
    for ( ; it2 != mBookmarks.end(); ++it2 )
	bookmarks.append( *it2 );

    TQFile f2( TQDir::currentDirPath() + "/.bookmarks" );
    f2.open( IO_WriteOnly );
    TQDataStream s2( &f2 );
    s2 << bookmarks;
    f2.close();
}

void HelpWindow::about()
{
    TQMessageBox::about( this, "HelpViewer Example",
			"<p>This example implements a simple HTML help viewer "
			"using Qt's rich text capabilities</p>"
			"<p>It's just about 100 lines of C++ code, so don't expect too much :-)</p>"
			);
}


void HelpWindow::aboutQt()
{
    TQMessageBox::aboutQt( this, "QBrowser" );
}

void HelpWindow::openFile()
{
#ifndef TQT_NO_FILEDIALOG
    TQString fn = TQFileDialog::getOpenFileName( TQString::null, TQString::null, this );
    if ( !fn.isEmpty() )
	browser->setSource( fn );
#endif
}

void HelpWindow::newWindow()
{
    ( new HelpWindow(browser->source(), "qbrowser") )->show();
}

#define KDE_PRINT
void HelpWindow::print()
{
#ifdef KDE_PRINT
    KPrinter printer;
#else
    TQPrinter printer;
#endif
    printer.setFullPage(true);
    printer.setDocName("Help Viewer");
    printer.setDocFileName("my_document");
#ifdef KDE_PRINT
    printer.addDialogPage(new RichPage());
    printer.addStandardPage(KPrinter::FilesPage);
#endif
    if ( printer.setup(this) ) {
	TQPainter p( &printer );
	TQPaintDeviceMetrics metrics(p.device());
	int dpix = metrics.logicalDpiX();
	int dpiy = metrics.logicalDpiY();
#ifdef KDE_PRINT
	const int margin = printer.option("app-rich-margin").toInt(); // pt
#else
	const int margin = 72; // pt
#endif
	TQRect body(margin*dpix/72, margin*dpiy/72,
		   metrics.width()-margin*dpix/72*2,
		   metrics.height()-margin*dpiy/72*2 );
#ifdef KDE_PRINT
	TQFont font(printer.option("app-rich-fontname"), printer.option("app-rich-fontsize").toInt());
#else
	TQFont font("times",10);
#endif
	TQSimpleRichText richText( browser->text(), font, browser->context(), browser->styleSheet(),
				  browser->mimeSourceFactory(), body.height() );
	richText.setWidth( &p, body.width() );
	TQRect view( body );
	int page = 1;
	do {
	    richText.draw( &p, body.left(), body.top(), view, colorGroup() );
	    view.moveBy( 0, body.height() );
	    p.translate( 0 , -body.height() );
	    p.setFont( font );
	    p.drawText( view.right() - p.fontMetrics().width( TQString::number(page) ),
			view.bottom() + p.fontMetrics().ascent() + 5, TQString::number(page) );
	    if ( view.top()  >= richText.height() )
		break;
	    printer.newPage();
	    page++;
	} while (true);
    }
}

void HelpWindow::pathSelected( const TQString &_path )
{
    browser->setSource( _path );
    TQMap<int, TQString>::Iterator it = mHistory.begin();
    bool exists = false;
    for ( ; it != mHistory.end(); ++it ) {
	if ( *it == _path ) {
	    exists = true;
	    break;
	}
    }
    if ( !exists )
	mHistory[ hist->insertItem( _path ) ] = _path;
}

void HelpWindow::readHistory()
{
    if ( TQFile::exists( TQDir::currentDirPath() + "/.history" ) ) {
	TQFile f( TQDir::currentDirPath() + "/.history" );
	f.open( IO_ReadOnly );
	TQDataStream s( &f );
	s >> history;
	f.close();
	while ( history.count() > 20 )
	    history.remove( history.begin() );
    }
}

void HelpWindow::readBookmarks()
{
    if ( TQFile::exists( TQDir::currentDirPath() + "/.bookmarks" ) ) {
	TQFile f( TQDir::currentDirPath() + "/.bookmarks" );
	f.open( IO_ReadOnly );
	TQDataStream s( &f );
	s >> bookmarks;
	f.close();
    }
}

void HelpWindow::histChosen( int i )
{
    if ( mHistory.contains( i ) )
	browser->setSource( mHistory[ i ] );
}

void HelpWindow::bookmChosen( int i )
{
    if ( mBookmarks.contains( i ) )
	browser->setSource( mBookmarks[ i ] );
}

void HelpWindow::addBookmark()
{
    mBookmarks[ bookm->insertItem( caption() ) ] = caption();
}

#include "helpwindow.moc"

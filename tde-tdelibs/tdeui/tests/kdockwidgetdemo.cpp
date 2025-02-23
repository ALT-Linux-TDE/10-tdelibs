#include "kdockwidgetdemo.h"

#include <tqheader.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>
#include <tqtextview.h>
#include <tqfileinfo.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqmultilineedit.h>
#include <tqevent.h>
#include <tqpopupmenu.h>
#include <tqpushbutton.h>
#include <tqpoint.h>
#include <tqmessagebox.h>
#include <tqmime.h>
#include <tqstrlist.h>
#include <tqpainter.h>

#include <tdeconfig.h>
#include <tdeapplication.h>
//#include <kimgio.h>
#include <stdlib.h>

static const char *dir_tree_xpm[] = {
"16 16 8 1",
"  c Gray0",
". c #000080",
"X c Cyan",
"o c #808000",
"O c Yellow",
"+ c #808080",
"@ c None",
"# c #c1c1c1",
"@@@@@@@@@@@@@@@@",
"@@@+++++@@@@@@@@",
"@@+@O@O@+@@@@@@@",
"@+@O@O@O@++++++@",
"@+@@@@@@@@@@@@o ",
"@+@O@++ +O@O@Oo ",
"@+@@+@X@ +O@O@o ",
"@+@+@X@X@ +O@Oo ",
"@+@+X@X@X o@O@o ",
"@+@+@X@X@ oO@Oo ",
"@+@@+@X@ +ooO@o ",
"@+@O@+  +@..oOo ",
"@+ooooooo.X..oo ",
"@@        .X.. @",
"@@@@@@@@@@@.X.. ",
"@@@@@@@@@@@@.X. "
};

static const char *preview_xpm[] = {
"16 16 6 1",
"  c Gray0",
". c #000080",
"X c Yellow",
"o c #808080",
"O c None",
"+ c Gray100",
"OOOOOOOOOOOOOOOO",
"OOo     oOOOOOOO",
"Oo  oooo  OOOOOO",
"O  OOOOoo oOOOOO",
"O OOOO++oo OOOOO",
"O OOOO++Oo OOOOO",
"O O+OOOOOo OOOOO",
"O OO+OOOOo OOOOO",
"OooOO+OOo  OOOOO",
"OO oOOOo  + OOOO",
"OOOo     .X+ OOO",
"OOOOOOOOO .X+ OO",
"OOOOOOOOOO .X+ O",
"OOOOOOOOOOO .X O",
"OOOOOOOOOOOO  OO",
"OOOOOOOOOOOOOOOO"
};

SFileDialog::SFileDialog( TQString initially, const TQStringList& filter, const char* name )
:TQDialog(0L,name,true)
{
  TDEConfig* config = kapp->config();
  config->setGroup( TQString::fromLatin1("SFileDialogData:") + name );
  if ( initially.isNull() ){
    initially = config->readPathEntry( "InitiallyDir", TQDir::currentDirPath() );
  }

  TQStringList bookmark;
  bookmark = config->readListEntry( "Bookmarks" );

  dockManager = new KDockManager(this);

  d_dirView = new KDockWidget( dockManager, "Dock_DirView", TQPixmap(dir_tree_xpm) );
  d_dirView->setCaption("Tree");

  dirView = new DirectoryView( d_dirView, 0 );
  dirView->addColumn( "" );
  dirView->header()->hide();
  d_dirView->setWidget( dirView );

  Directory* root = new Directory( dirView, "/" );
  root->setOpen(true);

  d_preview = new KDockWidget( dockManager, "Dock_Preview", TQPixmap(preview_xpm) );
  d_preview->setCaption("Preview");
  preview = new Preview( d_preview );
  d_preview->setWidget( preview );

  d_fd = new KDockWidget( dockManager, "Dock_QFileDialog", TQPixmap(), this );
  fd = new CustomFileDialog( d_fd );
  fd->setDir( initially );
  fd->setFilters( filter );
  fd->setBookmark( bookmark );
  fd->reparent(d_fd, TQPoint(0,0));
  d_fd->setWidget( fd );

  connect( dirView, TQ_SIGNAL( folderSelected( const TQString & ) ), fd, TQ_SLOT( setDir2( const TQString & ) ) );
  connect( fd, TQ_SIGNAL( dirEntered( const TQString & ) ), dirView, TQ_SLOT( setDir( const TQString & ) ) );

  d_fd->setDockSite( KDockWidget::DockTop|KDockWidget::DockLeft|KDockWidget::DockRight|KDockWidget::DockBottom );
  d_fd->setEnableDocking(KDockWidget::DockNone);

  d_dirView->setEnableDocking(KDockWidget::DockTop|KDockWidget::DockLeft|KDockWidget::DockRight|KDockWidget::DockBottom|KDockWidget::DockCenter);
  d_preview->setEnableDocking(KDockWidget::DockTop|KDockWidget::DockLeft|KDockWidget::DockRight|KDockWidget::DockBottom|KDockWidget::DockCenter);

  d_dirView->manualDock( d_fd, KDockWidget::DockLeft, 20 );
  d_preview->manualDock( d_fd, KDockWidget::DockBottom, 70 );

  connect(fd, TQ_SIGNAL(fileHighlighted(const TQString&)), preview, TQ_SLOT(showPreview(const TQString&)));
  connect(fd, TQ_SIGNAL(signalDone(int)), this, TQ_SLOT(done(int)));

  connect(fd, TQ_SIGNAL(dirEntered(const TQString&)), this, TQ_SLOT(changeDir(const TQString&)));
  connect(dirView, TQ_SIGNAL(folderSelected(const TQString&)), this, TQ_SLOT(changeDir(const TQString&)));

  b_tree = new TQToolButton( fd );
  TQToolTip::add( b_tree, "Show/Hide Tree" );
  b_tree->setPixmap( TQPixmap( dir_tree_xpm ) );
  connect( b_tree, TQ_SIGNAL(clicked()), d_dirView, TQ_SLOT(changeHideShowState()) );
  b_tree->setToggleButton(true);
  b_tree->setOn(true);
  fd->addToolButton( b_tree, true );

  b_preview = new TQToolButton( fd );
  TQToolTip::add( b_preview, "Show/Hide Preview" );
  b_preview->setPixmap( TQPixmap( preview_xpm ) );
  connect( b_preview, TQ_SIGNAL(clicked()), d_preview, TQ_SLOT(changeHideShowState()) );
  b_preview->setToggleButton(true);
  b_preview->setOn(true);
  fd->addToolButton( b_preview );

  connect( dockManager, TQ_SIGNAL(change()), this, TQ_SLOT(dockChange()));
  connect( dockManager, TQ_SIGNAL(setDockDefaultPos(KDockWidget*)), this, TQ_SLOT(setDockDefaultPos(KDockWidget*)));
  setCaption("Open File");
  resize(550,450);
  tqDebug("read config");
  dockManager->readConfig( 0L , name );
}

void SFileDialog::dockChange()
{
  b_preview->setOn( d_preview->isVisibleToTLW() );
  b_tree->setOn( d_dirView->isVisibleToTLW() );
}

SFileDialog::~SFileDialog()
{
  TDEConfig* config = kapp->config();
  config->setGroup( TQString("SFileDialogData:") + name() );
  config->writeEntry( "Bookmarks", fd->getBookmark() );

  tqDebug("write config");
  dockManager->writeConfig( 0L , name() );
}

void SFileDialog::setDockDefaultPos( KDockWidget* d )
{
  if ( d == d_dirView ){
    d_dirView->manualDock( d_fd, KDockWidget::DockLeft, 20 );
  }

  if ( d == d_preview ){
  d_preview->manualDock( d_fd, KDockWidget::DockBottom, 70 );
  }
}

void SFileDialog::changeDir( const TQString& f )
{
  if ( !f.isEmpty() ){
    TDEConfig* config = kapp->config();
    config->setGroup( TQString("SFileDialogData:") + name() );
    config->writePathEntry( "InitiallyDir", f );
  }
}

TQString SFileDialog::getOpenFileName( TQString initially,
                                      const TQStringList& filter,
                                      const TQString caption, const char* name )
{
  SFileDialog* fd = new SFileDialog( initially, filter, name );
  if ( !caption.isNull() ) fd->setCaption( caption );
  TQString result = ( fd->exec() == TQDialog::Accepted ) ? fd->fd->selectedFile():TQString::null;
  delete fd;

  return result;
}

TQStringList SFileDialog::getOpenFileNames( TQString initially,
                                      const TQStringList& filter,
                                      const TQString caption, const char* name )
{
  SFileDialog* fd = new SFileDialog( initially, filter, name );
  if ( !caption.isNull() ) fd->setCaption( caption );

  fd->fd->setMode( TQFileDialog::ExistingFiles );
  fd->d_preview->undock();
  fd->b_preview->hide();

  fd->exec();
  TQStringList result = fd->fd->selectedFiles();
  delete fd;

  return result;
}

void SFileDialog::showEvent( TQShowEvent *e )
{
  TQDialog::showEvent( e );
  dirView->setDir( fd->dirPath() );
}

/******************************************************************************************************/
PixmapView::PixmapView( TQWidget *parent )
:TQScrollView( parent )
{
//  kimgioRegister();
  viewport()->setBackgroundMode( PaletteBase );
}

void PixmapView::setPixmap( const TQPixmap &pix )
{
    pixmap = pix;
    resizeContents( pixmap.size().width(), pixmap.size().height() );
    viewport()->repaint( true );
}

void PixmapView::drawContents( TQPainter *p, int, int, int, int )
{
    p->drawPixmap( 0, 0, pixmap );
}

Preview::Preview( TQWidget *parent )
:TQWidgetStack( parent )
{
    normalText = new TQMultiLineEdit( this );
    normalText->setReadOnly( true );
    html = new TQTextView( this );
    pixmap = new PixmapView( this );
    raiseWidget( normalText );
}

void Preview::showPreview( const TQString &str )
{
  TQUrl u(str);
  if ( u.isLocalFile() ){
  TQString path = u.path();
  TQFileInfo fi( path );
	if ( fi.isFile() && (int)fi.size() > 400 * 1024 ) {
	    normalText->setText( tr( "The File\n%1\nis too large, so I don't show it!" ).arg( path ) );
	    raiseWidget( normalText );
	    return;
	}
	
	TQPixmap pix( path );
	if ( pix.isNull() ) {
	    if ( fi.isFile() ) {
		TQFile f( path );
		if ( f.open( IO_ReadOnly ) ) {
		    TQTextStream ts( &f );
		    TQString text = ts.read();
		    f.close();
		    if ( fi.extension().lower().contains( "htm" ) ) {
			TQString url = html->mimeSourceFactory()->makeAbsolute( path, html->context() );
			html->setText( text, url ); 	
			raiseWidget( html );
			return;
		    } else {
			normalText->setText( text ); 	
			raiseWidget( normalText );
			return;
		    }
		}
	    }
	    normalText->setText( TQString::null );
	    raiseWidget( normalText );
	} else {
	    pixmap->setPixmap( pix );
	    raiseWidget( pixmap );
	}
    } else {
	normalText->setText( "I only show local files!" );
	raiseWidget( normalText );
    }
}

// ****************************************************************************************************
static const char* homepage_xpm[] = {
"24 24 9 1",
"  c #262660",
". c #383666",
"X c #62639c",
"o c #7e86a5",
"O c #a6a7dd",
"+ c #bbbaed",
"@ c #c4c4f2",
"# c #f8f9f9",
"$ c None",
"$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$o.X$$$$$",
"$$$$$$$$$$oOOOO$O.X$$$$$",
"$$$$$$$$$$oXXXX+O.X$$$$$",
"$$$$$$$$$X#XXXXXO.X$$$$$",
"$$$$$$$$X  #XXXXO.X$$$$$",
"$$$$$$$X XO #XXXO.X$$$$$",
"$$$$$$X XOOO #XXX.XX$$$$",
"$$$$$X XOOOOO #XXXXXX$$$",
"$$$$X XOOOOOOO #XXXXXX$$",
"$$$X XOOOOOOOOO #XXXXXX$",
"$$X XOOOOOOOOOOO #.....$",
"$$$$$OOOOXXXXOOOOX...$$$",
"$$$$$OOOOXXXXOOOOX...$$$",
"$$$$$OOOOXXXXOOOOX..o$$$",
"$$$$$OOOOXXXXOOOOX.oo$$$",
"$$$$$OOOOXXXXOOOOXXoo$$$",
"$$$$$OOOOXXXXOOOOXooo$$$",
"$$$ooOOOOXXXXOOOOXoooo$$",
"$ooooXXXXXXXXXXXXXooooo$",
"$ooooooooooooooooooooooo",
"$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$"
};

static const char* folder_trash[] = {
"16 16 10 1",
"  c Gray0",
". c #222222",
"X c #333366",
"o c #6666cc",
"O c Magenta",
"+ c #808080",
"@ c #9999ff",
"# c #c0c0c0",
"$ c Gray100",
"% c None",
"%%%%%%oo%%%%%%%%",
"%%%%%o$$o%%%%%%%",
"%%%%o$$$$o%%%%%%",
"%%%o$+$+$$o o%%%",
"%%oo$$+$+$$o .%%",
"%%.oo$$+$+$$o.%%",
"%% Xoo$$+$$o +%%",
"%%o Xoo$$oo  o%%",
"%%%X.XX    +X%%%",
"%%%o@X+X#X+X+%%%",
"%%%+.$X#X+X %%%%",
"%%%%.o$o#X+X%%%%",
"%%%%.X@$X+ +%%%%",
"%%%%+Xo.#. %%%%%",
"%%%%%.Xo. o%%%%%",
"%%%%%+.   %%%%%%"
};

static const char* globalbookmark_xpm[]={
"12 16 3 1",
". c None",
"a c #000000",
"# c #008080",
"............",
"............",
"........##..",
".......#####",
"......#####a",
".....#####a.",
"....#####a..",
"...#####a...",
"..#####a....",
".#####a.....",
"aaa##a......",
"...#a.......",
"...a........",
"............",
"............",
"............"};

CustomFileDialog::CustomFileDialog( TQWidget* parent )
: TQFileDialog( parent, 0, false )
{
  TQToolButton *p = new TQToolButton( this );

  p->setPixmap( TQPixmap( globalbookmark_xpm ) );
  TQToolTip::add( p, tr( "Bookmarks" ) );

  bookmarkMenu = new TQPopupMenu( this );
  connect( bookmarkMenu, TQ_SIGNAL( activated( int ) ), this, TQ_SLOT( bookmarkChosen( int ) ) );
  addId = bookmarkMenu->insertItem( "Add bookmark" );
  clearId = bookmarkMenu->insertItem( TQPixmap(folder_trash), "Clear bookmarks" );
  bookmarkMenu->insertSeparator();

  p->setPopup( bookmarkMenu );
  p->setPopupDelay(0);
  addToolButton( p, true );

  TQToolButton *b = new TQToolButton( this );
  TQToolTip::add( b, tr( "Go Home!" ) );

  b->setPixmap( TQPixmap( homepage_xpm ) );
  connect( b, TQ_SIGNAL( clicked() ), this, TQ_SLOT( goHome() ) );

  addToolButton( b );
}


void CustomFileDialog::setBookmark( TQStringList &s )
{
	TQStringList::Iterator it = s.begin();
	for ( ; it != s.end(); ++it ){
    bookmarkList << (*it);

    const char* book_pix[]={
    "12 16 3 1",
    ". c None",
    "a c #000000",
    "# c #008080",
    "............",
    "............",
    "........##..",
    ".......#####",
    "......#####a",
    ".....#####a.",
    "....#####a..",
    "...#####a...",
    "..#####a....",
    ".#####a.....",
    "aaa##a......",
    "...#a.......",
    "...a........",
    "............",
    "............",
    "............"};
    bookmarkMenu->insertItem( TQIconSet( book_pix ), (*it) );
	}
}

CustomFileDialog::~CustomFileDialog()
{
}

void CustomFileDialog::setDir2( const TQString &s )
{
  blockSignals( true );
  setDir( s );
  blockSignals( false );
}

void CustomFileDialog::bookmarkChosen( int i )
{
  if ( i == clearId ){
    bookmarkList.clear();
    bookmarkMenu->clear();
    addId = bookmarkMenu->insertItem( "Add bookmark" );
    clearId = bookmarkMenu->insertItem( "Clear bookmarks" );
    bookmarkMenu->insertSeparator();
    return;
  }

  if ( i == addId ){
    bookmarkList << dirPath();

    const char* book_pix[]={
    "12 16 3 1",
    ". c None",
    "a c #000000",
    "# c #008080",
    "............",
    "............",
    "........##..",
    ".......#####",
    "......#####a",
    ".....#####a.",
    "....#####a..",
    "...#####a...",
    "..#####a....",
    ".#####a.....",
    "aaa##a......",
    "...#a.......",
    "...a........",
    "............",
    "............",
    "............"};
    bookmarkMenu->insertItem( TQIconSet( book_pix ), dirPath() );
    return;
  }

  setDir( bookmarkMenu->text( i ) );
}

void CustomFileDialog::goHome()
{
  if ( getenv( "HOME" ) )
    setDir( getenv( "HOME" ) );
  else
    setDir( "/" );
}

void CustomFileDialog::done( int i )
{
  emit signalDone(i);
}
/******************************************************************************************************/
static const char* folder_closed_xpm[] = {
"16 16 9 1",
"  c Gray0",
". c #222222",
"X c #6666cc",
"o c #9999ff",
"O c #c0c0c0",
"+ c #ccccff",
"@ c #ffffcc",
"# c Gray100",
"$ c None",
"$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$",
"$$XXXXo$$$$$$$$$",
"$X#++o@XXXXXX$$$",
"X#+++++++++++o$$",
"X#o+o+o+o+o+oX$$",
"X#+o+o+o+o+o+X $",
"X#o+o+o+o+o+oX $",
"X#+o+o+o+o+o+X $",
"X#o+o+o+o+o+oX $",
"X#+o+o+o+o+o+X $",
"X+o+o+o+o+o+oX $",
"XXXXXXXXXXXXXX $",
"$ ..           $"
};

static const char* folder_open_xpm[] = {
"16 16 10 1",
"  c Gray0",
". c #222222",
"X c #6666cc",
"o c Magenta",
"O c #9999ff",
"+ c #c0c0c0",
"@ c #ccccff",
"# c #ffffcc",
"$ c Gray100",
"% c None",
"%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%",
"%%XXXX%%%%%%%%%%",
"%X$$$XXXXXXXXX%%",
"%X$O+#######.OX%",
"%X$+#######.O@X%",
"%O$#######.O@OX ",
"XXXXXXXXXX.XO@X ",
"X$$$$$$$$$+ @OX ",
"%XO+O+O+O+O+ @X ",
"%X+O+O+O+O+O. X ",
"%%X+O+O+O+O+O.  ",
"%%XXXXXXXXXXXXX ",
"%%%             "
};

static const char* folder_locked_xpm[] = {
"16 16 8 1",
"  c Gray0",
". c #333366",
"X c #6666cc",
"o c Magenta",
"O c #9999ff",
"+ c #c0c0c0",
"@ c Gray100",
"# c None",
"###O.O##########",
"###...##########",
"###O.O##########",
"##.#X#.#OX######",
"#...X...@@X#####",
"#.......OOOOOOX#",
"#.......@@@@@@X#",
"#@@@@@@@O+O+O+X ",
"#O@O+O+O+O+O+OX ",
"#O@+O+O+O+O+O+X ",
"#O@O+O+O+O+O+OX ",
"#O@+O+O+O+O+O+X ",
"#O@O+O+O+O+O+OX ",
"#OXXXXXXXXXXXXX ",
"##              ",
"################"
};

Directory::Directory( Directory * parent, const TQString& filename )
:TQListViewItem( parent ), f(filename)
{
  p = parent;
  readable = TQDir( fullName() ).isReadable();

  if ( !readable )
    setPixmap( 0, TQPixmap( folder_locked_xpm ) );
  else
    setPixmap( 0, TQPixmap( folder_closed_xpm ) );
}


Directory::Directory( TQListView * parent, const TQString& filename )
:TQListViewItem( parent ), f(filename)
{
  p = 0;
  readable = TQDir( fullName() ).isReadable();
}


void Directory::setOpen( bool o )
{
  if ( o )
    setPixmap( 0, TQPixmap( folder_open_xpm ) );
  else
    setPixmap( 0, TQPixmap( folder_closed_xpm ) );

  if ( o && !childCount() ){
    TQString s( fullName() );
    TQDir thisDir( s );
    if ( !thisDir.isReadable() ) {
    readable = false;
    setExpandable( false );
    return;
  }

  listView()->setUpdatesEnabled( false );
  const QFileInfoList * files = thisDir.entryInfoList();
  if ( files ){
    QFileInfoListIterator it( *files );
    TQFileInfo * f;
    while( (f=it.current()) != 0 ){
      ++it;
      if ( f->fileName() != "." && f->fileName() != ".." && f->isDir() )
        (void)new Directory( this, f->fileName() );
      }
    }
    listView()->setUpdatesEnabled( true );
  }
  TQListViewItem::setOpen( o );
}


void Directory::setup()
{
  setExpandable( true );
  TQListViewItem::setup();
}


TQString Directory::fullName()
{
  TQString s;
  if ( p ) {
    s = p->fullName();
    s.append( f.name() );
    s.append( "/" );
  } else {
    s = f.name();
  }
  return s;
}


TQString Directory::text( int column ) const
{
  if ( column == 0 )
    return f.name();
  else
    if ( readable )
      return "Directory";
    else
      return "Unreadable Directory";
}

DirectoryView::DirectoryView( TQWidget *parent, const char *name )
:TQListView( parent, name )
{
  connect( this, TQ_SIGNAL( clicked( TQListViewItem * ) ),
           this, TQ_SLOT( slotFolderSelected( TQListViewItem * ) ) );
  connect( this, TQ_SIGNAL( doubleClicked( TQListViewItem * ) ),
           this, TQ_SLOT( slotFolderSelected( TQListViewItem * ) ) );
  connect( this, TQ_SIGNAL( returnPressed( TQListViewItem * ) ),
           this, TQ_SLOT( slotFolderSelected( TQListViewItem * ) ) );

  setAcceptDrops( true );
  viewport()->setAcceptDrops( true );
}

void DirectoryView::setOpen( TQListViewItem* i, bool b )
{
  TQListView::setOpen(i,b);
  setCurrentItem(i);
  slotFolderSelected(i);
}

void DirectoryView::slotFolderSelected( TQListViewItem *i )
{
  if ( !i )	return;

  Directory *dir = (Directory*)i;
  emit folderSelected( dir->fullName() );
}

TQString DirectoryView::fullPath(TQListViewItem* item)
{
  TQString fullpath = item->text(0);
  while ( (item=item->parent()) ) {
    if ( item->parent() )
      fullpath = item->text(0) + "/" + fullpath;
    else
      fullpath = item->text(0) + fullpath;
  }
  return fullpath;
}

void DirectoryView::setDir( const TQString &s )
{
  TQListViewItemIterator it( this );
  ++it;
  for ( ; it.current(); ++it ) {
    it.current()->setOpen( false );
  }

  TQStringList lst( TQStringList::split( "/", s ) );
  TQListViewItem *item = firstChild();
  TQStringList::Iterator it2 = lst.begin();
  for ( ; it2 != lst.end(); ++it2 ) {
    while ( item ) {
      if ( item->text( 0 ) == *it2 ) {
        item->setOpen( true );
        break;
      }
      item = item->itemBelow();
    }
  }

  if ( item ){
    setSelected( item, true );
    setCurrentItem( item );
  }
}

TQString DirectoryView::selectedDir()
{
  Directory *dir = (Directory*)currentItem();
  return dir->fullName();
}
/**********************************************************************************************/

int main(int argc, char* argv[]) {
  TDEApplication app(argc,argv,"kdockwidgetdemo");

#if 0
  SFileDialog* openfile = new SFileDialog();
  openfile->exec();
  tqDebug( openfile->fileName() );
#endif

#if 0
  tqDebug ( SFileDialog::getOpenFileName( TQString::null, TQString::fromLatin1("All (*)"),
                                         TQString::fromLatin1("DockWidget Demo"), "dialog1" ) );
#endif

#if 1
  TQStringList s = SFileDialog::getOpenFileNames( TQString::null, TQString::fromLatin1("All (*)"),
                                                TQString::fromLatin1("DockWidget Demo"), "dialog1" );
  TQStringList::Iterator it = s.begin();
  for ( ; it != s.end(); ++it ){
    tqDebug( "%s", (*it).local8Bit().data() );
  }
#endif
  return 0;
}

#include "kdockwidgetdemo.moc"


/* This file is part of the KDE libraries
    Copyright (C) 2001 Klaas Freitag <freitag@suse.de>

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

#include <tqdir.h>

#include <tdeglobal.h>
#include <kiconloader.h>
#include <tdemainwindow.h>
#include <tdeapplication.h>
#include <kurl.h>
#include <kdebug.h>
#include <kstatusbar.h>

#include <tdefiletreeview.h>
#include "tdefiletreeviewtest.h"


#include "tdefiletreeviewtest.moc"

testFrame::testFrame():TDEMainWindow(0,"Test FileTreeView"),
		       dirOnlyMode(false)

{
   treeView = new KFileTreeView( this );
   treeView->setDragEnabled( true );
   treeView->setAcceptDrops( true );
   treeView->setDropVisualizer( true );


   /* Connect to see the status bar */
   KStatusBar* sta = statusBar();
   connect( treeView, TQ_SIGNAL( onItem( const TQString& )),
	    sta, TQ_SLOT( message( const TQString& )));

   connect( treeView, TQ_SIGNAL( dropped( TQWidget*, TQDropEvent*, KURL::List& )),
	    this, TQ_SLOT( urlsDropped( TQWidget*, TQDropEvent*, KURL::List& )));

   connect( treeView, TQ_SIGNAL( dropped( KURL::List&, KURL& )), this,
	    TQ_SLOT( copyURLs( KURL::List&, KURL& )));

   treeView->addColumn( "File" );
   treeView->addColumn( "ChildCount" );
   setCentralWidget( treeView );
   resize( 600, 400 );

   showPath( KURL::fromPathOrURL( TQDir::homeDirPath() ));
}

void testFrame::showPath( const KURL &url )
{
   TQString fname = "TestBranch"; // url.fileName ();
   /* try a user icon */
   TDEIconLoader *loader = TDEGlobal::iconLoader();
   TQPixmap pix = loader->loadIcon( "contents2", TDEIcon::Small );
   TQPixmap pixOpen = loader->loadIcon( "contents", TDEIcon::Small );

   KFileTreeBranch *nb = treeView->addBranch( url, fname, pix );

   if( nb )
   {
      if( dirOnlyMode ) treeView->setDirOnlyMode( nb, true );
      nb->setOpenPixmap( pixOpen );

      connect( nb, TQ_SIGNAL(populateFinished(KFileTreeViewItem*)),
	       this, TQ_SLOT(slotPopulateFinished(KFileTreeViewItem*)));
      connect( nb, TQ_SIGNAL( directoryChildCount( KFileTreeViewItem *, int )),
	       this, TQ_SLOT( slotSetChildCount( KFileTreeViewItem*, int )));
      // nb->setChildRecurse(false );

      nb->setOpen(true);
   }


}

void testFrame::urlsDropped( TQWidget* , TQDropEvent* , KURL::List& list )
{
   KURL::List::ConstIterator it = list.begin();
   for ( ; it != list.end(); ++it ) {
      kdDebug() << "Url dropped: " << (*it).prettyURL() << endl;
   }
}

void testFrame::copyURLs( KURL::List& list, KURL& to )
{
   KURL::List::ConstIterator it = list.begin();
   kdDebug() << "Copy to " << to.prettyURL() << endl;
   for ( ; it != list.end(); ++it ) {
      kdDebug() << "Url: " << (*it).prettyURL() << endl;
   }

}


void testFrame::slotPopulateFinished(KFileTreeViewItem *item )
{
   if( item )
   {
#if 0
      int cc = item->childCount();

      kdDebug() << "setting column 2 of treeview with count " << cc << endl;

      item->setText( 1, TQString::number( cc ));
#endif
   }
   else
   {
      kdDebug() << "slotPopFinished for uninitalised item" << endl;
   }
}

void testFrame::slotSetChildCount( KFileTreeViewItem *item, int c )
{
   if( item )
      item->setText(1, TQString::number( c ));
}

int main(int argc, char **argv)
{
    TDEApplication a(argc, argv, "tdefiletreeviewtest", true);
    TQString name1;
    TQStringList names;

    TQString argv1;
    testFrame *tf;

    tf =  new testFrame();
    a.setMainWidget( tf );

    if (argc > 1)
    {
       for( int i = 1; i < argc; i++ )
       {
	  argv1 = TQString::fromLatin1(argv[i]);
	  kdDebug() << "Opening " << argv1 << endl;
	  if( argv1 == "-d" )
	     tf->setDirOnly();
	  else
	  {
	  KURL u( argv1 );
	  tf->showPath( u );
       }
    }
    }
    tf->show();
    int ret = a.exec();
    return( ret );
}

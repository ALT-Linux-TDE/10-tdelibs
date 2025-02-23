#include <tdelistview.h>
#include <tdeapplication.h>
#include <kdialogbase.h>
#include <tqvbox.h>


int main( int argc, char **argv )
{
	TDEApplication app( argc, argv, "tdelistviewtest" );
	KDialogBase dialog;
	TDEListView *view = new TDEListView( dialog.makeVBoxMainWidget() );
	view->setSelectionModeExt( TDEListView::FileManager );
	view->setDragEnabled( true );
	view->setItemsMovable( false );
	view->setAcceptDrops( true );
	view->addColumn("Column 1");
	view->addColumn("Column 2");
	view->addColumn("Column 3");

	new TDEListViewItem( view, "Item 1");
	new TDEListViewItem( view, "Item 1");
	new TDEListViewItem( view, "Item 1");
	new TDEListViewItem( view, "Item 1");
	new TDEListViewItem( view, "Item 1");
	new TDEListViewItem( view, "Item 1");
	new TDEListViewItem( view, "Item 1");
	new TDEListViewItem( view, "Item 1");
	new TDEListViewItem( view, "Item 1");
	new TDEListViewItem( view, "Item 2", "Some more", "Hi Mom :)" );

	view->restoreLayout( TDEGlobal::config(), "ListView" );

	new TDEListViewItem( view, "Item 3" );

	dialog.exec();
	view->saveLayout( TDEGlobal::config(), "ListView" );

	return 0;
}

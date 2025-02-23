#include <tdeapplication.h>
#include <keditcl.h>
#include <tqpopupmenu.h>

int main( int argc, char **argv )
{
  TDEApplication app( argc, argv, "kedittest" );
  KEdit *edit = new KEdit( 0L );
  TQPopupMenu *pop = new TQPopupMenu( 0L );
  pop->insertItem( "Popupmenu item" );
  edit->installRBPopup( pop );
  edit->show();
  return app.exec();
}

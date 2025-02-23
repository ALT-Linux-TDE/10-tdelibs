#include <tdeapplication.h>
#include <kicondialog.h>

int main( int argc, char **argv )
{
    TDEApplication app( argc, argv, "kicondialogtest", true );

//    TDEIconDialog::getIcon(); 

    TDEIconButton button;
    app.setMainWidget( &button );
    button.show();
 

    return app.exec();
}

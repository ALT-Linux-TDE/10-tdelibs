#include <tdeapplication.h>
#include <knotifydialog.h>

int main( int argc, char **argv )
{
    TDEApplication app( argc, argv, "knotifytest", true );
    KNotifyDialog *dlg = new KNotifyDialog();
    dlg->addApplicationEvents( "twin" );
    return dlg->exec();
}

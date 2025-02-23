#include <tdeapplication.h>
#include <kscan.h>

int main( int argc, char **argv )
{
    TDEApplication app( argc, argv, "kscantest", true, true );
    KScanDialog *dlg = KScanDialog::getScanDialog();
    if ( !dlg ) {
	tqDebug("*** EEK, no Scan-service available, aborting!");
	return EXIT_FAILURE;
    }
    
    dlg->show();

    return app.exec();
}

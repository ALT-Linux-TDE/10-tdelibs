#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdeaboutdata.h>
#include <tdemessagebox.h>
#include <tdecmdlineargs.h>

#include "passdlg.h"

int main ( int argc, char** argv )
{
    TDEAboutData aboutData("kiopassdlgtest", "TDEIO Password Dialog Test", "1.0");
    TDECmdLineArgs::init(argc, argv, &aboutData);
    TDEApplication app;

    TQString usr, pass, comment, label;
    label = "Site:";
    comment = "<b>localhost</b>";
    int res = TDEIO::PasswordDialog::getNameAndPassword( usr, pass, 0L,
                                                       TQString::null, false,
                                                       TQString::null, comment,
                                                       label );
    if ( res == TQDialog::Accepted )
        KMessageBox::information( 0L, TQString("You entered:\n"
					   "  Username: %1\n"
                                           "  Password: %2").arg(usr).arg(pass),
                                	"Test Result");
    else
        KMessageBox::information( 0L, "Password dialog was canceled!",
                                      "Test Result");

    return 0;
}

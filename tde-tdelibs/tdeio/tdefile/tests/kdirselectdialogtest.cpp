#include <tdeapplication.h>
#include <kdirselectdialog.h>
#include <tdemessagebox.h>
#include <kurl.h>

int main( int argc, char **argv )
{
    TDEApplication app(argc, argv, "kdirselectdialogtest", true);

    KURL u = KDirSelectDialog::selectDirectory( (argc >= 1) ? argv[1] : TQString::null );
    if ( u.isValid() )
        KMessageBox::information( 0L,
                                TQString::fromLatin1("You selected the url: %1")
                                .arg( u.prettyURL() ), "Selected URL" );

    return 0;
}

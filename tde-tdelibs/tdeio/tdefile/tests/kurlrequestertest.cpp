#include <tdeapplication.h>
#include <keditlistbox.h>
#include <kurlrequester.h>
#include <kurlrequesterdlg.h>

int main( int argc, char **argv )
{
    TDEApplication app( argc, argv, "kurlrequestertest", true );
    KURL url = KURLRequesterDlg::getURL( "ftp://ftp.kde.org" );
    tqDebug( "Selected url: %s", url.url().latin1());

    KURLRequester *req = new KURLRequester();
    KEditListBox *el = new KEditListBox( TQString::fromLatin1("Test"), req->customEditor() );
    el->show();
    return app.exec();
}

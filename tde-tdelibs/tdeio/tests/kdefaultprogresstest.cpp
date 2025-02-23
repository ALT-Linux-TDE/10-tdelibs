#include <tdeapplication.h>
#include <tdeio/defaultprogress.h>
#include <kurl.h>
#include <kdebug.h>

using namespace TDEIO;

int main(int argc, char **argv)
{
    TDEApplication app(argc, argv, "kdefaultprogresstest",true,true);

    DefaultProgress* dlg = new DefaultProgress();
    KURL src("http://this.host.doesn't.exist/this/dir/neither/andthisfileneither");
    KURL dest("file:/tmp/dest");
    dlg->slotCopying( 0L, src, dest );
    dlg->slotTotalSize( 0L, 12000 );
    dlg->slotTotalFiles( 0L, 12 );
    dlg->slotTotalDirs( 0L, 1 );

    dlg->slotSpeed( 0L, 55 );
    dlg->slotInfoMessage( 0L, TQString::fromLatin1( "Starting..." ) );

    int files = 0;
    for ( int size = 0 ; size < 12000 ; size += 1 )
    {
        dlg->slotProcessedSize( 0L, size );
        dlg->slotPercent( 0L, 100 * size / 12000 );
        if ( size % 1000 == 0 )
        {
            dlg->slotProcessedFiles( 0L, ++files );
        }
        kapp->processEvents();
    }
    dlg->slotInfoMessage( 0L, TQString::fromLatin1( "Done." ) );

    delete dlg;
    return 0;
}


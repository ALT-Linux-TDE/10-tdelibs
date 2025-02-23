#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>

#include "authinfo.h"

void output( const KURL& u )
{
    kdDebug() << "Looking up auto login for: " << u.url() << endl;
    TDEIO::NetRC::AutoLogin l;
    bool result = TDEIO::NetRC::self()->lookup( u, l, true );
    if ( !result )
    {
       kdDebug() << "Either no .netrc and/or .kionetrc file was "
                    "found or there was problem when attempting to "
                    "read from them!  Please make sure either or both "
                    "of the above files exist and have the correct "
                    "permission, i.e. a regular file owned by you with "
                    "with a read/write permission (0600)" << endl;
       return;
    }

    kdDebug() << "Type: " << l.type << endl
              << "Machine: " << l.machine << endl
              << "Login: " << l.login << endl
              << "Password: " << l.password << endl;

    TQMap<TQString,TQStringList>::ConstIterator it = l.macdef.begin();
    for ( ; it != l.macdef.end(); ++it )
    {
        kdDebug() << "Macro: " << it.key() << "= "
                  << it.data().join("   ") << endl;
    }
}

int main(int argc, char **argv)
{
    const char *version = "0.5";
    const char *description = "Unit test for .netrc and kionetrc parser.";
    TDECmdLineOptions options[] =
    {
        { "+command", "[url1,url2 ,...]", 0 },
        TDECmdLineLastOption
    };

    TDECmdLineArgs::init( argc, argv, "kionetrctest", description, version );
    TDECmdLineArgs::addCmdLineOptions( options );
    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
    int count = args->count();
    TDEApplication app;

    if ( !count )
        args->usage();
    else
    {
        KURL u;
        for( int i=0 ; i < count; i++ )
        {
            u = args->arg(i);
            if ( !u.isValid() )
            {
              kdDebug() << u.url() << " is invalid! Ignoring..." << endl;
              continue;
            }
            output( u );
        }
    }
    args->clear();
    return 0;
}

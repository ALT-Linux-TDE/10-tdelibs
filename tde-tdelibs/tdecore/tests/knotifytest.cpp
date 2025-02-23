#include <knotifyclient.h>
#include <tdeapplication.h>

int main( int argc, char **argv )
{
	TDEApplication app( argc, argv, TQCString("knotifytest") );
    KNotifyClient::userEvent( "This is a notification to notify you :)", 
                              KNotifyClient::Messagebox,
                              KNotifyClient::Error );
}

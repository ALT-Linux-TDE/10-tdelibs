#include <sys/times.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>

#include "addressbook.h"
#include "vcardformat.h"
#include "plugins/file/resourcefile.h"

using namespace TDEABC;

int main(int argc,char **argv)
{
  TDEAboutData aboutData("bigwrite","BigWriteKabc","0.1");
  TDECmdLineArgs::init(argc,argv,&aboutData);

  TDEApplication app( false, false );

  AddressBook ab;
  ResourceFile r( "my.tdeabc", "vcard" );
  ab.addResource( &r );
  
  for( int i = 0; i < 5000; ++i ) {
    Addressee a;
    a.setGivenName( "number" + TQString::number( i ) );
    a.setFamilyName( "Name" );
    a.insertEmail( TQString::number( i ) + "@domain" );
    
    ab.insertAddressee( a );
  }
  printf( "\n" );
  
  Ticket *t = ab.requestSaveTicket( &r );
  if ( t ) {
    struct tms start;

    times( &start );

#if 0
    kdDebug() << "utime : " << int( start.tms_utime ) << endl;
    kdDebug() << "stime : " << int( start.tms_stime ) << endl;
    kdDebug() << "cutime: " << int( start.tms_cutime ) << endl;
    kdDebug() << "cstime: " << int( start.tms_cstime ) << endl;
#endif
	    
    if ( !ab.save( t ) ) {
      kdDebug() << "Can't save." << endl;
    }

    struct tms end;

    times( &end );

#if 0
    kdDebug() << "utime : " << int( end.tms_utime ) << endl;
    kdDebug() << "stime : " << int( end.tms_stime ) << endl;
    kdDebug() << "cutime: " << int( end.tms_cutime ) << endl;
    kdDebug() << "cstime: " << int( end.tms_cstime ) << endl;
#endif

    kdDebug() << "UTime: " << int( end.tms_utime ) - int( start.tms_utime ) << endl; 
    kdDebug() << "STime: " << int( end.tms_stime ) - int( start.tms_stime ) << endl; 

  } else {
    kdDebug() << "No ticket for save." << endl;
  }
}

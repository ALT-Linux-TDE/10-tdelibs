#include "ktimezones.h"
#include <tdeapplication.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
  TDEInstance instance("ktimezonestest");

  if ((argc==2) && (strcmp(argv[1], "local")==0))
  {
     KTimezones timezones;

     // Find the local timezone.
     const KTimezone *timezone = timezones.local();
     printf( "Local timezone: %s\n", timezone->name().latin1() );

     // Find the current offset of the UTC timezone.
     timezone = timezones.zone("UTC");
     printf( "UTC timezone offset should be 0: %d\n", timezone->offset(TQDateTime::currentDateTime()) );

     // Find some offsets for Europe/London.
     const char *london = "Europe/London";
     timezone = timezones.zone(london);
     TQDateTime winter(TQDateTime::fromString("2005-01-01T00:00:00", TQt::ISODate));
     TQDateTime summer(TQDateTime::fromString("2005-06-01T00:00:00", TQt::ISODate));
     printf( "%s winter timezone offset should be 0: %d\n", london, timezone->offset(winter) );
     printf( "%s summer timezone offset should be 3600: %d\n", london, timezone->offset(summer) );

     // Try timezone conversions.
     const KTimezone *losAngeles = timezones.zone("America/Los_Angeles");
     const char *bstBeforePdt = "2005-03-28T00:00:00";
     const char *bstAfterPdt = "2005-05-01T00:00:00";
     const char *gmtBeforePst = "2005-10-30T01:00:00";
     const char *gmtAfterPst = "2005-12-01T00:00:00";
     TQString result;
     result = timezone->convert(losAngeles, TQDateTime::fromString(bstBeforePdt, TQt::ISODate)).toString(TQt::ISODate);
     printf( "BST before PDT, %s should be 2005-03-27T15:00:00: %s\n", bstBeforePdt, result.latin1() );
     result = timezone->convert(losAngeles, TQDateTime::fromString(bstAfterPdt, TQt::ISODate)).toString(TQt::ISODate);
     printf( "BST and PDT,    %s should be 2005-04-30T16:00:00: %s\n", bstAfterPdt, result.latin1() );
     result = timezone->convert(losAngeles, TQDateTime::fromString(gmtBeforePst, TQt::ISODate)).toString(TQt::ISODate);
     printf( "GMT before PST, %s should be 2005-10-29T17:00:00: %s\n", gmtBeforePst, result.latin1() );
     result = timezone->convert(losAngeles, TQDateTime::fromString(gmtAfterPst, TQt::ISODate)).toString(TQt::ISODate);
     printf( "GMT and PST,    %s should be 2005-11-30T16:00:00: %s\n", gmtAfterPst, result.latin1() );
     printf( "Latitude 89 should be valid: %svalid\n", KTimezone::isValidLatitude(89.0) ? "" : "in");
     printf( "Latitude 91 should be invalid: %svalid\n", KTimezone::isValidLatitude(91.0) ? "" : "in");
     printf( "Longitude 179 should be valid: %svalid\n", KTimezone::isValidLongitude(179.0) ? "" : "in");
     printf( "Longitude 181 should be valid: %svalid\n", KTimezone::isValidLongitude(181.0) ? "" : "in");
     return 0;
  }

  if ((argc==2) && (strcmp(argv[1], "all")==0))
  {
     KTimezones timezones;
     KTimezones::ZoneMap allZones = timezones.allZones();
     for ( KTimezones::ZoneMap::const_iterator it = allZones.begin(), end = allZones.end(); it != end; ++it )
         printf( "%s\n", it.key().latin1() );
  }

  printf( "Usage: ktimezonestest [local|all]!\n" );
  return 1;
}

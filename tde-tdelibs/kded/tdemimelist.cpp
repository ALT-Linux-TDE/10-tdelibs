#include <ktrader.h>
#include <kservice.h>
#include <kmimetype.h>
#include <assert.h>
#include <kstandarddirs.h>
#include <kservicegroup.h>
#include <kimageio.h>
#include <kuserprofile.h>
#include <kprotocolinfo.h>

#include <tdeapplication.h>

#include <stdio.h>

int main(int argc, char *argv[])
{
   TDEApplication k(argc,argv,"blurb",false);

   KMimeType::List mtl = KMimeType::allMimeTypes( );
   assert( mtl.count() );
   tqDebug( "Found %d mime types.", mtl.count() );
   TQValueListIterator<KMimeType::Ptr> it(mtl.begin());
   KServiceTypeProfile::OfferList ol;

   for (; it != mtl.end(); ++it)
   {
     {
      // Application
      printf( "APP:%s:", (*it)->name().latin1() );
      ol = KServiceTypeProfile::offers((*it)->name(), "Application");
      TQValueListIterator<KServiceOffer> it2(ol.begin());
      for (; it2 != ol.end(); ++it2) {
        if ((*it2).allowAsDefault())
           printf( " %s", (*it2).service()->desktopEntryPath().ascii() );

      }
      printf( "\n" );
     }

     {
      // Embedded
      printf( "PART:%s:", (*it)->name().latin1() );
      ol = KServiceTypeProfile::offers((*it)->name(), "KParts/ReadOnlyPart");
      TQValueListIterator<KServiceOffer> it2(ol.begin());
      for (; it2 != ol.end(); ++it2) {
        if ((*it2).allowAsDefault())
           printf( " %s", (*it2).service()->desktopEntryPath().ascii() );

      }
      printf( "\n" );
     }
   }
}


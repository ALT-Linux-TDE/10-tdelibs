#include <kservice.h>
#include <kmimetype.h>
#include <kservicetype.h>

#include <tdeapplication.h>
#include <kdebug.h>

int main(int argc, char *argv[])
{
   TDEApplication k(argc,argv,"getalltest",false/*nostyle*/,false/*noGUI*/); // KMessageBox needs KApp for makeStdCaption

//for (int i = 0 ; i < 2 ; ++i ) { // test twice to see if they got deleted
   kdDebug() << "All services" << endl;
   KService::List services = KService::allServices();
   kdDebug() << "got " << services.count() << " services" << endl;
   TQValueListIterator<KService::Ptr> s = services.begin();
   for ( ; s != services.end() ; ++s )
   {
     kdDebug() << (*s)->name() << " " << (*s)->desktopEntryPath() << endl;
   }
//}
   
   kdDebug() << "All mimeTypes" << endl;
   KMimeType::List mimeTypes = KMimeType::allMimeTypes();
   kdDebug() << "got " << mimeTypes.count() << " mimeTypes" << endl;
   TQValueListIterator<KMimeType::Ptr> m = mimeTypes.begin();
   for ( ; m != mimeTypes.end() ; ++m )
   {
     kdDebug() << (*m)->name() << endl;
   }
   
   kdDebug() << "All service types" << endl;
   KServiceType::List list = KServiceType::allServiceTypes();
   kdDebug() << "got " << list.count() << " service types" << endl;
   TQValueListIterator<KServiceType::Ptr> st = list.begin();
   for ( ; st != list.end() ; ++st )
   {
     kdDebug() << (*st)->name() << endl;
   }

   kdDebug() << "done" << endl;
   
   return 0;
}

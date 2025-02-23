#include <kiconloader.h>
#include <tqdatetime.h>
#include <stdio.h>
#include <tdeapplication.h>
#include <stdlib.h>
#include <kdebug.h>

int main(int argc, char *argv[])
{
  TDEApplication app(argc,argv,TQCString("kiconloadertest")/*,false,false*/);

  TDEIconLoader * mpLoader = TDEGlobal::iconLoader();
  TDEIcon::Context mContext = TDEIcon::Application;
  TQTime dt;
  dt.start();
  int count = 0;
  for ( int mGroup = 0; mGroup < TDEIcon::LastGroup ; ++mGroup )
  {
      kdDebug() << "queryIcons " << mGroup << "," << mContext << endl;
      TQStringList filelist=mpLoader->queryIcons(mGroup, mContext);
      kdDebug() << " -> found " << filelist.count() << " icons." << endl;
      int i=0;
      for(TQStringList::Iterator it = filelist.begin();
          it != filelist.end() /*&& i<10*/;
          ++it, ++i )
      {
          //kdDebug() << ( i==9 ? "..." : (*it) ) << endl;
          mpLoader->loadIcon( (*it), (TDEIcon::Group)mGroup );
	  ++count;
      }
  }
  kdDebug() << "Loading " << count << " icons took " << (float)(dt.elapsed()) / 1000 << " seconds" << endl;
}


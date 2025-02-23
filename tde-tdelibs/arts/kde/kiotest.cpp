#include <stdio.h>
#include <kmedia2.h>
#include <tdecmdlineargs.h>
#include <connect.h>
#include <tdelocale.h>
#include <tdeapplication.h>
#include <tdeaboutdata.h>
#include "qiomanager.h"
#include "artskde.h"

using namespace std;
using namespace Arts;


static TDECmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP("URL to open"), 0 },
    TDECmdLineLastOption
};

int main(int argc, char **argv)
{
	TDEAboutData aboutData( "kiotest", I18N_NOOP("KIOTest"), I18N_NOOP("0.1"), "", TDEAboutData::License_GPL, "");
							  
	TDECmdLineArgs::init(argc,argv,&aboutData);
	TDECmdLineArgs::addCmdLineOptions(options); 	
	TDEApplication app;
	QIOManager qiomanager;
	Dispatcher dispatcher(&qiomanager);
	TDEIOInputStream stream;
	StdoutWriter writer;

	TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

	if(args->count())
	{
	    if(!stream.openURL(args->arg(0)))
	    {
		printf("can't open url");
		exit(1);
	    }
	}
	else
	    exit(1);
	    
	args->clear();
	
	connect(stream, writer);

	writer.start();
	stream.start();
	
	app.exec();
}

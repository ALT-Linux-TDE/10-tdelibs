#include <stdio.h>

#include <tqfile.h>
#include <tqobject.h>

#include <tdelocale.h>
#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <tdeapplication.h>

#include <flowsystem.h>
#include <kplayobject.h>
#include <kartsdispatcher.h>
#include <kplayobjectfactory.h>
#include <kaudioconverter.h>
#include "kconverttest.moc"

using namespace std;
using namespace Arts;

static TDECmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP("URL to open"), 0 },
    TDECmdLineLastOption
};

KConvertTest::KConvertTest()
{
}

void KConvertTest::slotRawStreamStart()
{
//	cout << "[START]\n\n" << endl;
}

void KConvertTest::slotNewBlockSize(long blockSize)
{
	m_blockSize = blockSize;
}

void KConvertTest::slotNewBlockPointer(long blockPointer)
{
	m_blockPointer = blockPointer;
}

void KConvertTest::slotNewData()
{
	fwrite((void *) m_blockPointer, 1, m_blockSize, stdout);
}

void KConvertTest::slotRawStreamFinished()
{
//	cout << "\n\n[END]" << endl;
}

int main(int argc, char **argv)
{
	TDEAboutData aboutData("kconverttest", I18N_NOOP("KConvertTest"), I18N_NOOP("0.1"), "", TDEAboutData::License_GPL, "");
							  
	TDECmdLineArgs::init(argc, argv, &aboutData);
	TDECmdLineArgs::addCmdLineOptions(options); 	
	TDEApplication app;

	TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

	KURL url;
	
	if(args->count())
		url = args->arg(0);
	else
		exit(1);

	args->clear();

	KConvertTest *get = new KConvertTest();

	KArtsDispatcher dispatcher;
	KAudioConverter converter;

	// FIXME: crashes
	// converter.setup(44100);
	converter.requestPlayObject(url);

	TQObject::connect(&converter, TQ_SIGNAL(rawStreamStart()), get, TQ_SLOT(slotRawStreamStart()));

	TQObject::connect(&converter, TQ_SIGNAL(newBlockSize(long)), get, TQ_SLOT(slotNewBlockSize(long)));
	TQObject::connect(&converter, TQ_SIGNAL(newBlockPointer(long)), get, TQ_SLOT(slotNewBlockPointer(long)));
	TQObject::connect(&converter, TQ_SIGNAL(newData()), get, TQ_SLOT(slotNewData()));
	
	TQObject::connect(&converter, TQ_SIGNAL(rawStreamFinished()), get, TQ_SLOT(slotRawStreamFinished()));

	converter.start();

	app.exec();
}


#include "kmmanager.h"
#include <tdecmdlineargs.h>
#include <tdeapplication.h>
#include <tdemessagebox.h>
#include <tdelocale.h>
#include <tdeglobal.h>

static TDECmdLineOptions options[] =
{
	{ "tdeconfig", I18N_NOOP("Configure TDE Print"), 0 },
	{ "serverconfig", I18N_NOOP("Configure print server"), 0 },
	TDECmdLineLastOption
};

extern "C" TDE_EXPORT int kdemain(int argc, char *argv[])
{
	TDECmdLineArgs::init(argc, argv, "kaddprinterwizard",
			I18N_NOOP("Start the add printer wizard"),
			"0.1");
	TDECmdLineArgs::addCmdLineOptions(options);
	
	TDEGlobal::locale()->setMainCatalogue("tdelibs");

	TDEApplication app;
	TDECmdLineArgs	*args = TDECmdLineArgs::parsedArgs();
	bool	doConfig = args->isSet("tdeconfig");
	bool	doSrvConfig = args->isSet("serverconfig");

	if (doConfig)
		KMManager::self()->invokeOptionsDialog();
	else if (doSrvConfig)
		KMManager::self()->configureServer();
	else if (KMManager::self()->addPrinterWizard() == -1)
	{
		KMessageBox::error(0, KMManager::self()->errorMsg().prepend("<qt>").append("</qt>"));
	}
	
	return 0;
}

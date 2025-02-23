#include <ksystemtray.h>
#include <tdeapplication.h>

int main(int argc, char **argv)
{
	TDEApplication app( argc, argv, "ksystemtraytest" );
	TQLabel *l = new TQLabel("System Tray Main Window", 0L);
	KSystemTray *tray = new KSystemTray( l );
    tray->setText("Test");
	l->show();
	tray->show();

	return app.exec();
}

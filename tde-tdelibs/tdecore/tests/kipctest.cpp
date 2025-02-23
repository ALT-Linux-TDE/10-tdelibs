#include <stdio.h>
#include <stdlib.h>
#include <tqobject.h>
#include <tdeapplication.h>
#include <kipc.h>
#include "kipctest.h"

MyObject::MyObject()
    : TQObject(0L, "testobj")
{
    connect(kapp, TQ_SIGNAL(tdedisplayPaletteChanged()), TQ_SLOT(slotPaletteChanged()));
    connect(kapp, TQ_SIGNAL(tdedisplayFontChanged()), TQ_SLOT(slotFontChanged()));
    connect(kapp, TQ_SIGNAL(tdedisplayStyleChanged()), TQ_SLOT(slotStyleChanged()));
    connect(kapp, TQ_SIGNAL(backgroundChanged(int)), TQ_SLOT(slotBackgroundChanged(int)));
    connect(kapp, TQ_SIGNAL(appearanceChanged()), TQ_SLOT(slotAppearanceChanged()));
    connect(kapp, TQ_SIGNAL(kipcMessage(int,int)), TQ_SLOT(slotMessage(int,int)));
}

int main(int argc, char **argv)
{
    TDEApplication app(argc, argv, TQCString("kipc"));

    if (argc == 3) 
    {
	KIPC::sendMessageAll((KIPC::Message) atoi(argv[1]), atoi(argv[2]));
	return 0;
    }

    MyObject obj;
    return app.exec();
}

#include "kipctest.moc"

#ifndef __blah__h__
#define __blah__h__

#include <tqobject.h>

class MyObject: public TQObject
{
    TQ_OBJECT
public:
    MyObject();

public slots:
    void slotPaletteChanged() { printf("TQ_SIGNAL: Palette changed\n"); }
    void slotStyleChanged() { printf("TQ_SIGNAL: Style changed\n"); }
    void slotFontChanged() { printf("TQ_SIGNAL: Font changed\n"); }
    void slotBackgroundChanged(int i) { printf("TQ_SIGNAL: Background %d changed\n", i); }
    void slotAppearanceChanged() { printf("TQ_SIGNAL: Appearance changed\n"); }
    void slotMessage(int id, int arg) { printf("TQ_SIGNAL: user message: %d,%d\n", id, arg); }
};

#endif

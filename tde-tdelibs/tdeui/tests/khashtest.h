//
// Simple little hack to show off new diagonal gradients.
//
// (C) KDE Artistic Daniel M. Duley <mosfet@kde.org>
//

#ifndef __KHASH_TEST_H
#define __KHASH_TEST_H

#include <tqwidget.h>
#include <kpixmap.h>

class KHashWidget : public TQWidget
{
    TQ_OBJECT

public:
    KHashWidget(TQWidget *parent=0, const char *name=0)
        : TQWidget(parent, name){;}
protected:
    void paintEvent(TQPaintEvent *ev);
private:
    KPixmap pix;
};

#endif

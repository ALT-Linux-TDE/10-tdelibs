//
// Simple little hack to show off new diagonal gradients.
//
// (C) KDE Artistic Daniel M. Duley <mosfet@kde.org>
//

#ifndef __KGRADIENT_TEST_H
#define __KGRADIENT_TEST_H

#include <tqwidget.h>
#include <kpixmap.h>

class KGradientWidget : public TQWidget
{
public:
    KGradientWidget(TQWidget *parent=0, const char *name=0)
        : TQWidget(parent, name){;}
protected:
    void paintEvent(TQPaintEvent *ev);
private:
    KPixmap pix;
};

#endif

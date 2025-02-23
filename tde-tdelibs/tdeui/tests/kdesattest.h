//
// Simple little hack to show off blending effects.
//
// (C) KDE Artistic Cristian Tibirna <tibirna@kde.org>
//

#ifndef __KBLEND_TEST_H
#define __KBLEND_TEST_H

#include <tqwidget.h>
#include <tqimage.h>
#include <knuminput.h>

class KDesatWidget : public TQWidget
{
TQ_OBJECT
public:
    KDesatWidget(TQWidget *parent=0, const char *name=0);

public slots:
    void change(double);

protected:
    void paintEvent(TQPaintEvent *ev);
private:
    float desat_value;
    TQImage image;
    KDoubleNumInput *slide;
};

#endif

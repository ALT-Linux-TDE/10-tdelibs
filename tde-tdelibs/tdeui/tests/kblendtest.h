//
// Simple little hack to show off blending effects.
//
// (C) KDE Artistic Cristian Tibirna <tibirna@kde.org>
//

#ifndef __KBLEND_TEST_H
#define __KBLEND_TEST_H

#include <tqwidget.h>
#include <tqimage.h>

class KBlendWidget : public TQWidget
{
public:
    KBlendWidget(TQWidget *parent=0, const char *name=0);
protected:
    void paintEvent(TQPaintEvent *ev);
private:
    TQImage image, original;
    TQColor bgnd;
};

#endif

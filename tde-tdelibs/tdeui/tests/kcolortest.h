#ifndef __KCOLORTEST_H
#define __KCOLORTEST_H

#include <tqwidget.h>
#include <tqimage.h>
#include <kpixmap.h>

class KColorWidget : public TQWidget
{
public:
    KColorWidget(TQWidget *parent=0, const char *name=0);
    void doIntensityLoop();
protected:
    void paintEvent(TQPaintEvent *ev);
private:
    TQImage image, original;
    KPixmap pixmap;

};

#endif

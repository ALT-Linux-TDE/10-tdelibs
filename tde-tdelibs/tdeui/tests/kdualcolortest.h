#ifndef __KDUALCOLORTEST_H
#define __KDUALCOLORTEST_H

#include <tqlabel.h>
#include <kdualcolorbutton.h>

class KDualColorWidget : public TQWidget
{
    TQ_OBJECT
public:
    KDualColorWidget(TQWidget *parent=0, const char *name=0);
protected slots:
    void slotFgChanged(const TQColor &c);
    void slotBgChanged(const TQColor &c);
    void slotCurrentChanged(KDualColorButton::DualColor current);
protected:
    TQLabel *lbl;
};

#endif

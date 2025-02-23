//
// Simple little hack to show off new diagonal gradients.
//
// (C) KDE Artistic Daniel M. Duley <mosfet@kde.org>
//

#ifndef __KGRADIENT_TEST_H
#define __KGRADIENT_TEST_H

#include <tqwidget.h>
#include <kpixmap.h>
#include <tqslider.h>
#include <tqlabel.h>
#include <tqdatetime.h>

class KGradientWidget : public TQWidget
{
public:
    KGradientWidget(TQWidget *parent=0, const char *name=0);

    void setBalance(int a, int b) { xbalance = a; ybalance = b; }
protected:
    void paintEvent(TQPaintEvent *ev);
private:
    KPixmap pix;
    int xbalance, ybalance;
    TQTime time;
 
};

class myTopWidget: public TQWidget
{
  TQ_OBJECT
public:
  myTopWidget(TQWidget *parent=0, const char *name=0);
  
private:
  TQLabel *bLabel;
  TQSlider *xSlider, *ySlider;
  KGradientWidget *grds;

  TQTime time;

  int itime, otime;

private slots:
  void rebalance();
};
#endif

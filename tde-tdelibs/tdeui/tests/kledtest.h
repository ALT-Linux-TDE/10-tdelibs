#ifndef kledtest_h
#define kledtest_h

#include <tqwidget.h>
#include <tqtimer.h>
#include <stdlib.h>
#include <kled.h>

class KLedTest : public TQWidget
{
  TQ_OBJECT
protected:
  TQTimer timer;
  KLed *leds[/*KLed::NoOfShapes*/2* /*KLed::NoOfLooks*/3* /*KLed::NoOfStates*/2];
  const int LedWidth;
  const int LedHeight;
  const int Grid;
  KLed::Shape shape;
  KLed::Look look;
  KLed::State state;
  int x, y, index;


  TQTimer t_toggle, t_color, t_look;
  //KLed *l;				// create lamp
  //KLed *l;				// create lamp
  KLed *l;				// create lamp
  //KLed *l;				// create lamp
  //KLed *l;				// create lamp
  int ledcolor;
  KLed::Look  ledlook;

  const TQColor red;
  const TQColor blue;
  const TQColor green;
  const TQColor yellow;

public:

  KLedTest(TQWidget* parent=0);
  ~KLedTest();

  bool kled_round;


public slots:
  void timeout();

  void nextColor();
  void nextLook();


};

#endif


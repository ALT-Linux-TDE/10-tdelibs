#ifndef krulertest_h
#define krulertest_h

#include <tdeapplication.h>
#include <tdemainwindow.h>
#include <tqwidget.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqbuttongroup.h>
#include <knuminput.h>

class KRuler;
class TQWidget;
class TQFrame;
class TQGridLayout;
class TQCheckBox;
class TQGroupBox;


class MouseWidget : public TQFrame
{
TQ_OBJECT
public:
MouseWidget( TQWidget *parent=0, const char *name=0, WFlags f=0 );

signals:
  void newXPos(int);
  void newYPos(int);
  void newWidth(int);
  void newHeight(int);

protected:
  virtual void mousePressEvent   ( TQMouseEvent * );
  virtual void mouseReleaseEvent ( TQMouseEvent * );
  virtual void mouseMoveEvent    ( TQMouseEvent * );
  virtual void resizeEvent       ( TQResizeEvent * );
private:
  bool mouseButtonDown;

};


class KRulerTest : public TDEMainWindow
{
TQ_OBJECT
public:
KRulerTest( const char *name = 0L );
~KRulerTest();

private slots:
  void slotNewWidth(int);
  void slotNewHeight(int);

  void slotSetTinyMarks(bool);
  void slotSetLittleMarks(bool);
  void slotSetMediumMarks(bool);
  void slotSetBigMarks(bool);
  void slotSetEndMarks(bool);
  void slotSetRulerPointer(bool);

  void slotSetRulerLength(int);
  void slotFixRulerLength(bool);
  void slotSetMStyle(int);
  void slotUpdateShowMarks();
  void slotCheckLength(bool);

  void slotSetRotate(double);
  void slotSetXTrans(double);
  void slotSetYTrans(double);
  

private:

  KRuler *hruler, *vruler;
  TQGridLayout *layout;
  TQFrame *miniwidget, *bigwidget;
  TQFrame *mainframe;

  TQLabel *mouse_message;
  TQGroupBox *showMarks, *lineEdit, *vertrot;
  TQCheckBox *showTM, *showLM, *showMM, *showBM, *showEM, *showPT, *fixLen;
  KIntNumInput *beginMark, *endMark, *lengthInput;
  KDoubleNumInput *transX, *transY, *rotV;
  TQButtonGroup *metricstyle;
  TQRadioButton *pixelmetric, *inchmetric, *mmmetric, *cmmetric, *mmetric;

};
#endif


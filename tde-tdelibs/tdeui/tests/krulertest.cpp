

#include "krulertest.h"

#include "kruler.h"
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqgroupbox.h>

/*
void
MyCheckBox::mouseReleaseEvent(QMouseEvent *e )
{
  TQButton::mouseReleaseEvent(e);
  if ();
}
*/

MouseWidget::MouseWidget( TQWidget *parent, const char *name, WFlags f )
  : TQFrame(parent, name, f)
{
}

void
MouseWidget::mousePressEvent( TQMouseEvent *e )
{
  mouseButtonDown = true;
  emit newXPos(e->x());
  emit newYPos(e->y());
}

void
MouseWidget::mouseReleaseEvent( TQMouseEvent * )
{ mouseButtonDown = false; }

void
MouseWidget::mouseMoveEvent( TQMouseEvent *e )
{
  if (mouseButtonDown) {
    emit newXPos(e->x());
    emit newYPos(e->y());
  }
}

void
MouseWidget::resizeEvent( TQResizeEvent *r )
{
  emit newWidth(r->size().width());
  emit newHeight(r->size().height());
}


KRulerTest::KRulerTest( const char *name )
  : TDEMainWindow(0, name)
{
  mainframe = new TQFrame(this);

  layout = new TQGridLayout(mainframe, 2, 2);

  miniwidget = new TQFrame(mainframe);
  miniwidget->setFrameStyle(TQFrame::WinPanel | TQFrame::Raised);
  bigwidget = new MouseWidget(mainframe);
  bigwidget->setFrameStyle(TQFrame::WinPanel | TQFrame::Sunken);

  //  TQRect bwrect = bigwidget->frameRect();
  //  tqDebug("big rect: top%i left%i bottom%i right%i",
  //	bwrect.top(), bwrect.left(), bwrect.bottom(), bwrect.right());
  hruler = new KRuler(Horizontal, mainframe);
  //  hruler->setRange( bwrect.left(), bwrect.right() );
  hruler->setRange( 0, 1000 );
  //  hruler->setOffset( bwrect.left() - bigwidget->frameRect().left() );
  hruler->setOffset( 0 );

  vruler = new KRuler(Vertical, mainframe);
  vruler->setFrameStyle(TQFrame::WinPanel | TQFrame::Sunken);
  vruler->setOffset( 0 );
  vruler->setRange( 0, 1000 );

  connect( bigwidget, TQ_SIGNAL(newXPos(int)),
	   hruler, TQ_SLOT(slotNewValue(int)) );
  connect( bigwidget, TQ_SIGNAL(newYPos(int)),
	   vruler, TQ_SLOT(slotNewValue(int)) );
  connect( bigwidget, TQ_SIGNAL(newWidth(int)),
	   TQ_SLOT(slotNewWidth(int)) );
  connect( bigwidget, TQ_SIGNAL(newHeight(int)),
	   TQ_SLOT(slotNewHeight(int)) );

  layout->addWidget(miniwidget, 0, 0);
  layout->addWidget(hruler, 0, 1);
  layout->addWidget(vruler, 1, 0);
  layout->addWidget(bigwidget, 1, 1);

  mouse_message = new TQLabel("Press and hold mouse button\nfor pointer movement", bigwidget);
  mouse_message->adjustSize();
  mouse_message->move(4,4);

  showMarks = new TQGroupBox("Show which marks ?", bigwidget);
  showMarks->setFixedSize(140, 160);
  showMarks->move(330,4);
  showTM = new TQCheckBox("show tiny marks", showMarks);
  showTM->adjustSize();
  showTM->move(5,15);
  showTM->setChecked(true);
  connect(showTM, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotSetTinyMarks(bool)) );
  showLM = new TQCheckBox("show little marks", showMarks);
  showLM->adjustSize();
  showLM->move(5,35);
  showLM->setChecked(true);
  connect(showLM, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotSetLittleMarks(bool)) );
  showMM = new TQCheckBox("show medium marks", showMarks);
  showMM->adjustSize();
  showMM->move(5,55);
  showMM->setChecked(true);
  connect(showMM, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotSetMediumMarks(bool)) );
  showBM = new TQCheckBox("show big marks", showMarks);
  showBM->adjustSize();
  showBM->move(5,75);
  showBM->setChecked(true);
  connect(showBM, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotSetBigMarks(bool)) );
  showEM = new TQCheckBox("show end marks", showMarks);
  showEM->adjustSize();
  showEM->move(5,95);
  showEM->setChecked(true);
  connect(showEM, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotSetEndMarks(bool)) );
  showPT = new TQCheckBox("show ruler pointer", showMarks);
  showPT->adjustSize();
  showPT->move(5,115);
  showPT->setChecked(true);
  connect(showPT, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotSetRulerPointer(bool)) );
  fixLen = new TQCheckBox("fix ruler length", showMarks);
  fixLen->adjustSize();
  fixLen->move(5,135);
  fixLen->setChecked(true);
  connect(fixLen, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotFixRulerLength(bool)) );
  connect(fixLen, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotCheckLength(bool)) );

  lineEdit = new TQGroupBox("Value of begin/end", bigwidget);
  lineEdit->setFixedSize(140, 80);
  lineEdit->move(330,4+160);
  beginMark = new KIntNumInput(0, lineEdit);
  beginMark->setRange(-1000, 1000, 1, false);
  beginMark->move(5, 15);
  beginMark->setFixedSize(beginMark->sizeHint());
  connect(beginMark, TQ_SIGNAL(valueChanged(int)), 
	  hruler, TQ_SLOT(slotNewOffset(int)) );
  connect(beginMark, TQ_SIGNAL(valueChanged(int)), 
	  vruler, TQ_SLOT(slotNewOffset(int)) );
  endMark = new KIntNumInput(0, lineEdit);
  endMark->setRange(-1000, 1000, 1, false);
  endMark->move(5, 35);
  endMark->setFixedSize(endMark->sizeHint());
  connect(endMark, TQ_SIGNAL(valueChanged(int)), 
	  hruler, TQ_SLOT(slotEndOffset(int)) );
  connect(endMark, TQ_SIGNAL(valueChanged(int)), 
	  vruler, TQ_SLOT(slotEndOffset(int)) );
  lengthInput = new KIntNumInput(0, lineEdit);
  lengthInput->setRange(-1000, 1000, 1, false);
  lengthInput->move(5, 55);
  lengthInput->setFixedSize(lengthInput->sizeHint());
  connect(lengthInput, TQ_SIGNAL(valueChanged(int)), 
	  hruler, TQ_SLOT(slotEndOffset(int)) );
  connect(lengthInput, TQ_SIGNAL(valueChanged(int)), 
	  vruler, TQ_SLOT(slotEndOffset(int)) );


  vertrot = new TQGroupBox("Value of rotate translate for Vert.", bigwidget);
  vertrot->setFixedSize(140, 80);
  vertrot->move(330,4+160+80+4);
  transX = new KDoubleNumInput(0.0, vertrot);
  transX->setRange(-1000, 1000, 1, false);
  transX->move(5, 15);
  transX->setFixedSize(transX->sizeHint());
  //transX->setLabel("transx", AlignLeft);
  connect(transX, TQ_SIGNAL(valueChanged(double)), 
	  TQ_SLOT(slotSetXTrans(double)) );
  transY = new KDoubleNumInput(-12.0, vertrot);
  transY->setRange(-1000, 1000, 1, false);
  transY->move(5, 35);
  transY->setFixedSize(transY->sizeHint());
  //transY->setLabel("transy", AlignLeft);
  connect(transY, TQ_SIGNAL(valueChanged(double)), 
	  TQ_SLOT(slotSetYTrans(double)) );
  rotV = new KDoubleNumInput(90.0, vertrot);
  rotV->setRange(-1000, 1000, 1, false);
  rotV->move(5, 55);
  rotV->setFixedSize(rotV->sizeHint());
  //rotV->setLabel("rot", AlignLeft);
  connect(rotV, TQ_SIGNAL(valueChanged(double)), 
	  TQ_SLOT(slotSetRotate(double)) );
  

  metricstyle = new TQButtonGroup("metric styles", bigwidget);
  metricstyle->setFixedSize(100, 120);
  metricstyle->move(330-110,4);
  pixelmetric = new TQRadioButton("pixel", metricstyle);
  pixelmetric->adjustSize();
  pixelmetric->move(5,15);
  metricstyle->insert(pixelmetric, (int)KRuler::Pixel);
  inchmetric = new TQRadioButton("inch", metricstyle);
  inchmetric->adjustSize();
  inchmetric->move(5,35);
  metricstyle->insert(inchmetric, (int)KRuler::Inch);
  mmmetric = new TQRadioButton("millimeter", metricstyle);
  mmmetric->adjustSize();
  mmmetric->move(5,55);
  metricstyle->insert(mmmetric, (int)KRuler::Millimetres);
  cmmetric = new TQRadioButton("centimeter", metricstyle);
  cmmetric->adjustSize();
  cmmetric->move(5,75);
  metricstyle->insert(cmmetric, (int)KRuler::Centimetres);
  mmetric = new TQRadioButton("meter", metricstyle);
  mmetric->adjustSize();
  mmetric->move(5,95);
  metricstyle->insert(mmetric, (int)KRuler::Metres);
  connect ( metricstyle, TQ_SIGNAL(clicked(int)), TQ_SLOT(slotSetMStyle(int)) );

  setCentralWidget(mainframe);

  slotUpdateShowMarks();
}

KRulerTest::~KRulerTest()
{
  delete layout;
  delete hruler;
  delete vruler;
  delete miniwidget;
  delete bigwidget;
  delete mainframe;
}

void
KRulerTest::slotNewWidth(int width)
{
  hruler->setMaxValue(width);
}

void
KRulerTest::slotNewHeight(int height)
{
  vruler->setMaxValue(height);
}

void
KRulerTest::slotSetTinyMarks(bool set)
{
  hruler->setShowTinyMarks(set);
  vruler->setShowTinyMarks(set);
}

void
KRulerTest::slotSetLittleMarks(bool set)
{
  hruler->setShowLittleMarks(set);
  vruler->setShowLittleMarks(set);
}

void
KRulerTest::slotSetMediumMarks(bool set)
{
  hruler->setShowMediumMarks(set);
  vruler->setShowMediumMarks(set);
}

void
KRulerTest::slotSetBigMarks(bool set)
{
  hruler->setShowBigMarks(set);
  vruler->setShowBigMarks(set);
}

void
KRulerTest::slotSetEndMarks(bool set)
{
  hruler->setShowEndMarks(set);
  vruler->setShowEndMarks(set);
}

void 
KRulerTest::slotSetRulerPointer(bool set)
{
  hruler->setShowPointer(set);
  vruler->setShowPointer(set);
}

void 
KRulerTest::slotSetRulerLength(int len)
{
  hruler->setLength(len);
  vruler->setLength(len);
}

void 
KRulerTest::slotFixRulerLength(bool fix)
{
  hruler->setLengthFixed(fix);
  vruler->setLengthFixed(fix);
}

void
KRulerTest::slotSetMStyle(int style)
{
  hruler->setRulerMetricStyle((KRuler::MetricStyle)style);
  vruler->setRulerMetricStyle((KRuler::MetricStyle)style);
  slotUpdateShowMarks();
}


void
KRulerTest::slotUpdateShowMarks()
{
  showTM->setChecked(hruler->showTinyMarks());
  showLM->setChecked(hruler->showLittleMarks());
  showMM->setChecked(hruler->showMediumMarks());
  showBM->setChecked(hruler->showBigMarks());
  showEM->setChecked(hruler->showEndMarks());
}

void 
KRulerTest::slotCheckLength(bool fixlen)
{
  beginMark->setValue(hruler->offset());
  endMark->setValue(hruler->endOffset());
  lengthInput->setValue(hruler->length());
}

void 
KRulerTest::slotSetRotate(double d)
{
#ifdef KRULER_ROTATE_TEST
  vruler->rotate = d;
  vruler->update();
  //debug("rotate %.1f", d);
#endif
}

void 
KRulerTest::slotSetXTrans(double d)
{
#ifdef KRULER_ROTATE_TEST
  vruler->xtrans = d;
  vruler->update();
  //debug("trans x %.1f", d);
#endif
}

void 
KRulerTest::slotSetYTrans(double d)
{
#ifdef KRULER_ROTATE_TEST
  vruler->ytrans = d;
  vruler->update();
  //debug("trans y %.1f", d);
#endif
}


/* --- MAIN -----------------------*/
int main(int argc, char **argv)
{
  TDEApplication *testapp;
  KRulerTest   *window;

  testapp = new TDEApplication(argc, argv,"krulertest");
  testapp->setFont(TQFont("Helvetica",12),true);

  window = new KRulerTest("main");
  testapp->setMainWidget(window);
  window->setCaption("KRulerTest");
  window->show();
  return testapp->exec();
}

#include "krulertest.moc"


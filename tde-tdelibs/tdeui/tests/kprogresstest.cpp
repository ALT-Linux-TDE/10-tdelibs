#include <tdeapplication.h>
#include <tqwidget.h>
#include "kprogress.h"


class MyWidget : public TQWidget {
public:
	MyWidget() : TQWidget()
	{
		setFixedSize(440, 80);
		Progress = new KProgress(this);
		Progress->resize(400, 40);
		Progress->move(20, 20);
		startTimer(50);
	}
	
private:
	KProgress *Progress;
	
	void timerEvent(TQTimerEvent *);
};

void MyWidget::timerEvent(TQTimerEvent *)
{
  static enum { fwd, back } direction = fwd;
  //static KProgress::BarStyle style = KProgress::Solid;
  if (direction == fwd) 
	{
	  if (Progress->value() == Progress->maxValue())
		direction = back;
	  else
		Progress->advance(1);
	} 
  else 
	{
	  if (Progress->value() == 0 /*Progress->minValue()*/) 
		{
				direction = fwd;
				//style = (style == KProgress::Solid)? KProgress::Blocked : KProgress::Solid;
				//Progress->setBarStyle(style);
		} 
	  else
		Progress->advance(-1);
	}
}

int main(int argc, char *argv[])
{
	TDEApplication app(argc, argv, "KProgressTest");
	MyWidget w;
	
	app.setMainWidget(&w);

	w.show();
	
	int ret = app.exec();
	return ret;
}

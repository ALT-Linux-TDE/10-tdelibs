#include "kdebug.h"
#include <tqwidget.h>
#include <kinstance.h>
#include <iostream>
#include <tqapplication.h>
#include <tqpen.h>
#include <tqvariant.h>

class TestWidget : public TQWidget
{

public:
  TestWidget(TQWidget* parent, const char* name)
    : TQWidget(parent, name)
  {
    kdDebug().form("mytest %s", "hello") << endl;
    TQString test = "%20C this is a string";
    kdDebug(150) << test << endl;
    TQCString cstr = test.latin1();
    kdDebug(150) << test << endl;
    TQChar ch = 'a';
    kdDebug() << "TQChar a: " << ch << endl;
    ch = '\r';
    kdDebug() << "TQChar \\r: " << ch << endl;
    kdDebug() << k_lineinfo << "error on this line" << endl;
    kdDebug(2 == 2) << "this is right " << perror << endl;
    kdDebug() << "Before instance creation" << endl;
    kdDebug(1202) << "Before instance creation" << endl;
    TDEInstance i("kdebugtest");
    kdDebug(1) << "kDebugInfo with inexisting area number" << endl;
    kdDebug(1202) << "This number has a value of " << 5 << endl;
    // kdDebug() << "This number should come out as appname " << 5 << " " << "test" << endl;
    kdWarning() << "1+1 = " << 1+1+1 << endl;
    kdError(1+1 != 2) << "there is something really odd!" << endl;
    TQString s = "mystring";
    kdDebug() << s << endl;
    kdError(1202) << "Error !!!" << endl;
    kdError() << "Error with no area" << endl;

    kdDebug() << "Printing a null TQWidget pointer: " << (TQWidget*)0 << endl;

    kdDebug() << "char " << '^' << " " << char(26) << endl;
    TQPoint p(0,9);
    kdDebug() << p << endl;

    TQRect r(9,12,58,234);
    kdDebug() << r << endl;

    TQRegion reg(r);
    reg += TQRect(1,60,200,59);
    kdDebug() << reg << endl;

    TQStringList sl;
    sl << "hi" << "this" << "list" << "is" << "short";
    kdDebug() << sl << endl;

    TQValueList<int> il;
    kdDebug() << "Empty TQValueList<int>: " << il << endl;
    il << 1 << 2 << 3 << 4 << 5;
    kdDebug() << "TQValueList<int> filled: " << il << endl;

    TQ_LLONG big = 65536LL*65536*500;
    kdDebug() << big << endl;

    TQVariant v( 0.12345 );
    kdDebug() << "Variant: " << v << endl;
    v = TQPen( TQt::red );
    kdDebug() << "Variant: " << v << endl;

    TQByteArray data( 6 );
    data[0] = 42;
    data[1] = 'H';
    data[2] = 'e';
    data[3] = 'l';
    data[4] = 'l';
    data[5] = 'o';
    kdDebug() << data << endl;
    data.resize( 80 );
    data.fill( 42 );
    kdDebug() << data << endl;
  }
  void resizeEvent(TQResizeEvent*)
  {
    kdDebug() << this << endl;
  }
};

int main(int argc, char** argv)
{
  TQApplication app(argc, argv);
  TestWidget widget(0, "NoNameWidget");
  widget.setGeometry(45, 54, 120, 80);
  widget.show();
  app.setMainWidget(&widget);
  app.exec();
  return 0;
}


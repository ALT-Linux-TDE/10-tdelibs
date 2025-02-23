#include <tdeapplication.h>
#include <tqwidget.h>
#include <tqcursor.h>
#include "tdepopupmenu.h"

class DemoWidget : public TQWidget {
private:
    TDEPopupMenu *menu;
    
void mousePressEvent(TQMouseEvent *)
{
    menu->popup(TQCursor::pos());
}

void paintEvent(TQPaintEvent *)
{
    drawText(32, 32, "Press a Mouse Button!");
}

public:
    DemoWidget() : TQWidget()
    {
        menu = new TDEPopupMenu("Popup Menu:");
        menu->insertItem("Item1");
        menu->insertItem("Item2");
        menu->insertSeparator();
        menu->insertItem("Quit", tqApp, TQ_SLOT(quit()));
    }       
};

int main(int argc, char **argv)
{
    TDEApplication app(argc, argv, "tdepopupmenutest");
    DemoWidget w;
    app.setMainWidget(&w);
    w.setFont(TQFont("helvetica", 12, TQFont::Bold), true);
    w.show();
    return app.exec();
}
    

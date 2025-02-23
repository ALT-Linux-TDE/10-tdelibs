#ifndef __KPANELMENUTEST_H
#define __KPANELMENUTEST_H

#include <kpanelappmenu.h>
#include <tqlabel.h>

class TestWidget : public TQLabel
{
    TQ_OBJECT
public:
    TestWidget(TQWidget *parent=0, const char *name=0);
    ~TestWidget(){delete testMenu;}
public slots:
    void slotMenuCalled(int id);
    void slotSubMenuCalled(int id);
private:
    void init();
    KPanelAppMenu *testMenu, *subMenu;
};

#endif

#ifndef TDEMAINWINDOWTEST_H
#define TDEMAINWINDOWTEST_H

#include <tdemainwindow.h>

class MainWindow : public TDEMainWindow
{
    TQ_OBJECT
public:
    MainWindow();

private slots:
    void showMessage();
};

#endif // TDEMAINWINDOWTEST_H

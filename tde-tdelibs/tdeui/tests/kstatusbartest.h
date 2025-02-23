#ifndef test_kstatusbar_h
#define test_kstatusbar_h

#include <tdemenubar.h>
#include <tqpopupmenu.h>
#include <kstatusbar.h>
#include <tdemainwindow.h>

class TQMultiLineEdit;

class testWindow  : public TDEMainWindow
{
    TQ_OBJECT

public:
    testWindow (TQWidget *parent=0, const char *name=0);
    ~testWindow ();
    
public slots:
    void slotPress(int i);
    void slotClick(int i);
    void slotMenu(int i);
    
protected:
    TQPopupMenu *fileMenu;
    TQPopupMenu *smenu;
    KMenuBar *menuBar;
    KStatusBar *statusbar;
    bool insert;
    TQMultiLineEdit *widget;
};
#endif


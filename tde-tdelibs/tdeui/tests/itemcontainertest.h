#ifndef _ITEMCONTAINERTEST_H
#define _ITEMCONTAINERTEST_H

#include <tqwidget.h>

class TDEIconView;
class TDEListView;
class TDEListBox;
class TQButtonGroup;
class TQLabel;

class TopLevel : public TQWidget
{
    TQ_OBJECT
public:

    TopLevel( TQWidget *parent=0, const char *name=0 );

    enum ViewID { IconView, ListView, ListBox };
    enum ModeID { NoSelection, Single, Multi, Extended };

public slots:
    //void slotSwitchView( int id );
    void slotSwitchMode( int id ); 

    void slotIconViewExec( TQIconViewItem* item );
    void slotListViewExec( TQListViewItem* item ); 
    void slotListBoxExec( TQListBoxItem* item );
    void slotToggleSingleColumn( bool b );

    void slotClicked( TQIconViewItem* ) { tqDebug("CLICK");}
    void slotDoubleClicked( TQIconViewItem* ) { tqDebug("DOUBLE CLICK");}
protected:
    TDEIconView* m_pIconView;
    TDEListView* m_pListView;
    TDEListBox* m_pListBox;

    TQButtonGroup* m_pbgView;
    TQButtonGroup* m_pbgMode;
    TQLabel* m_plblWidget;
    TQLabel* m_plblSignal;
    TQLabel* m_plblItem;
};

#endif

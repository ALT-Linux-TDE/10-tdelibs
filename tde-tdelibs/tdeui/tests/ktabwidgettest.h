#ifndef KTABWIDGETTEST_H
#define KTABWIDGETTEST_H

#include <kiconloader.h>
#include <tqwidget.h>
#include <ktabwidget.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqpopupmenu.h>
#include <stdlib.h>
#include <tqvbox.h>
#include <time.h>
#include <tqcheckbox.h>
#include <tqtoolbutton.h>

class Test : public TQVBox
{
  TQ_OBJECT
public:
  Test( TQWidget* parent=0, const char *name =0 );

private slots:
  void addTab();
  void removeCurrentTab();
  void toggleLeftButton(bool);
  void toggleRightButton(bool);
  void toggleLeftPopup(bool);
  void toggleRightPopup(bool);
  void toggleTabPosition(bool);
  void toggleTabShape(bool);
  void toggleCloseButtons(bool);
  void toggleLabels(bool);

  void currentChanged(TQWidget*);
  void contextMenu(TQWidget*, const TQPoint&);
  void tabbarContextMenu(const TQPoint&);
  void testCanDecode(const TQDragMoveEvent *, bool & /* result */);
  void receivedDropEvent( TQDropEvent* );
  void initiateDrag( TQWidget * );
  void receivedDropEvent( TQWidget *, TQDropEvent * );
  void mouseDoubleClick(TQWidget*);
  void mouseMiddleClick(TQWidget*);
  void movedTab( int, int );

  void leftPopupActivated(int);
  void rightPopupActivated(int);
  void contextMenuActivated(int);
  void tabbarContextMenuActivated(int);

private:
  KTabWidget*     mWidget;
  int             mChange;

  TQCheckBox *     mLeftButton;
  TQCheckBox *     mRightButton;
  TQCheckBox *     mTabsBottom;

  TQToolButton*    mLeftWidget;
  TQToolButton*    mRightWidget;

  TQPopupMenu*     mLeftPopup;
  TQPopupMenu*     mRightPopup;
  TQPopupMenu*     mTabbarContextPopup;
  TQPopupMenu*     mContextPopup;
  TQWidget*        mContextWidget;
};


#endif

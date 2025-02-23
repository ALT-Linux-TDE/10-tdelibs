#ifndef KDOCKTEST_H
#define KDOCKTEST_H

#include <kdockwidget.h>

class TQWidget;
class DockTest : public KDockArea
{
  TQ_OBJECT
public:
  DockTest( TQWidget* parent=0 );

private:
  KDockWidget* m_blueDock;
  KDockWidget* m_redDock;
  KDockWidget* m_yellowDock;
};

#endif

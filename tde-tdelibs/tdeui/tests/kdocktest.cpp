#include "kdocktest.h"

#include <tdeapplication.h>
#include <kiconloader.h>

#include <tqwidget.h>
#include <tqstring.h>

DockTest::DockTest( TQWidget* parent )
  : KDockArea( parent )
{
  m_blueDock = createDockWidget( "Blue Widget", SmallIcon("mail") );
  //m_blueDock->setDetachable( false );
  m_blueDock->setEnableDocking( KDockWidget::DockFullSite );
  KDockWidgetHeader *header = new KDockWidgetHeader( m_blueDock, "Blue Header" );
  header->forceCloseButtonHidden();
  m_blueDock->setHeader( header );
  m_blueDock->setCaption( "Blue" );
  m_blueDock->setGeometry( 50, 50, 100, 100 );
  TQWidget *l = new TQWidget( m_blueDock );
  l->setBackgroundColor( TQt::blue );
  l->setMinimumSize( 100,100 );
  m_blueDock->setWidget( l );

  setMainDockWidget( m_blueDock );

  m_redDock = createDockWidget( "Red Widget", SmallIcon("news") );
  m_redDock->setEnableDocking( KDockWidget::DockFullSite );
  //m_redDock->setDetachable( false );
  header = new KDockWidgetHeader( m_redDock, "Red kHeader" );
  m_redDock->setHeader( header );
  m_redDock->setCaption( "Red" );
  m_redDock->setGeometry( 50, 50, 100, 100 );
  l = new TQWidget( m_redDock );
  l->setBackgroundColor( TQt::red );
  l->setMinimumSize( 100,100 );
  m_redDock->setWidget( l );
  m_redDock->manualDock( m_blueDock, KDockWidget::DockLeft, 3000 );

  m_yellowDock = createDockWidget( "Yellow Widget", SmallIcon("web") );
  m_yellowDock->setEnableDocking( KDockWidget::DockFullSite );
  //m_yellowDock->setDetachable( false );
//  header = new KDockWidgetHeader( m_yellowDock, "Yellow Header" );
 // m_yellowDock->setHeader( header );
  m_yellowDock->setCaption( "Yellow" );
  m_yellowDock->setGeometry( 50, 50, 100, 100 );
  l = new TQWidget( m_yellowDock );
  l->setBackgroundColor( TQt::yellow );
  l->setMinimumSize( 100,100 );
  m_yellowDock->setWidget( l );
  m_yellowDock->manualDock( m_blueDock, KDockWidget::DockTop, 5000 );
}

int
main( int argc, char** argv )
{
  TDEApplication a( argc,argv, "docktest" );
  DockTest* ap = new DockTest();
  ap->setCaption("DockWidget demo");
  a.setMainWidget( ap );
  ap->show();
  return a.exec();
}

#include "kdocktest.moc"

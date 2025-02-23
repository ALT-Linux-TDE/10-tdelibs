#include <tdeapplication.h>
#include <kjanuswidget.h>

#include <tqstring.h>
#include <tqcheckbox.h>
#include <tqvbox.h>

int main(int argc, char** argv)
{
  TDEApplication app(argc, argv, "JanusWidgetTest");
  // -----
  KJanusWidget* janus = new KJanusWidget( 0, 0, KJanusWidget::IconList );

  TQVBox* page1 = janus->addVBoxPage( TQString( "Page1" ) ); // use i18n in real apps
  TQCheckBox* cb1 = new TQCheckBox( "a", page1 );

  TQVBox* page2 = janus->addVBoxPage( TQString( "Page2" ) );
  TQCheckBox* cb2 = new TQCheckBox( "a", page2 );

  janus->show();
  TQObject::connect( &app, TQ_SIGNAL( lastWindowClosed() ), &app, TQ_SLOT( quit() ) );

  return app.exec();
}


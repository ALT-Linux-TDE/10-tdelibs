#include <tqstring.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqhbox.h>
#include <tqtimer.h>

#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <tdelocale.h>
#include <klineedit.h>
#include <tdeglobalsettings.h>
#include <tdecompletionbox.h>

#include "klineedittest.h"

KLineEditTest::KLineEditTest (TQWidget* widget, const char* name )
              :TQWidget( widget, name )
{
    TQVBoxLayout* layout = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

    TQStringList list;
    list << "Tree" << "Suuupa" << "Stroustrup" << "Stone" << "Slick"
         << "Slashdot" << "Send" << "Peables" << "Mankind" << "Ocean"
         << "Chips" << "Computer" << "Sandworm" << "Sandstorm" << "Chops";
    list.sort();

    m_lineedit = new KLineEdit( this, "klineedittest" );
    m_lineedit->completionObject()->setItems( list );
    m_lineedit->setFixedSize(500,30);
    m_lineedit->setEnableSqueezedText( true );
    connect( m_lineedit, TQ_SIGNAL( returnPressed() ), TQ_SLOT( slotReturnPressed() ) );
    connect( m_lineedit, TQ_SIGNAL( returnPressed(const TQString&) ), 
             TQ_SLOT( slotReturnPressed(const TQString&) ) );

    TQHBox *hbox = new TQHBox (this);
    m_btnExit = new TQPushButton( "E&xit", hbox );
    m_btnExit->setFixedSize(100,30);
    connect( m_btnExit, TQ_SIGNAL( clicked() ), TQ_SLOT( quitApp() ) );
    
    m_btnReadOnly = new TQPushButton( "&Read Only", hbox );
    m_btnReadOnly->setToggleButton (true);
    m_btnReadOnly->setFixedSize(100,30);
    connect( m_btnReadOnly, TQ_SIGNAL( toggled(bool) ), TQ_SLOT( slotReadOnly(bool) ) );
    
    m_btnEnable = new TQPushButton( "Dis&able", hbox );
    m_btnEnable->setToggleButton (true);
    m_btnEnable->setFixedSize(100,30);
    connect( m_btnEnable, TQ_SIGNAL( toggled(bool) ), TQ_SLOT( slotEnable(bool) ) );
    
    m_btnHide = new TQPushButton( "Hi&de", hbox );
     m_btnHide->setFixedSize(100,30);
    connect( m_btnHide, TQ_SIGNAL( clicked() ), TQ_SLOT( slotHide() ) );

    layout->addWidget( m_lineedit );
    layout->addWidget( hbox );
    setCaption( "KLineEdit Unit Test" );
}

KLineEditTest::~KLineEditTest()
{
}

void KLineEditTest::quitApp()
{
    kapp->closeAllWindows();
}

void KLineEditTest::show()
{
  if (m_lineedit->isHidden())
    m_lineedit->show();
  
  m_btnHide->setEnabled( true );
   
  TQWidget::show();
}

void KLineEditTest::slotReturnPressed()
{
    kdDebug() << "Return pressed" << endl;
}

void KLineEditTest::slotReturnPressed( const TQString& text )
{
    kdDebug() << "Return pressed: " << text << endl;
}

void KLineEditTest::resultOutput( const TQString& text )
{
    kdDebug() << "KlineEditTest Debug: " << text << endl;
}

void KLineEditTest::slotReadOnly( bool ro )
{
    m_lineedit->setReadOnly (ro);
    TQString text = (ro) ? "&Read Write" : "&Read Only";
    m_btnReadOnly->setText (text);
}

void KLineEditTest::slotEnable (bool enable)
{
    m_lineedit->setEnabled (!enable);
    TQString text = (enable) ? "En&able":"Dis&able";
    m_btnEnable->setText (text);
}

void KLineEditTest::slotHide()
{
    m_lineedit->hide();
    m_btnHide->setEnabled( false );      
    m_lineedit->setText( "My dog ate the homework, whaaaaaaaaaaaaaaaaaaaaaaa"
                          "aaaaaaaaaaaaaaaaaaaaaaaaa! I want my mommy!" );
    TQTimer::singleShot( 1000, this, TQ_SLOT(show()) );
}

int main ( int argc, char **argv)
{
    TDEAboutData aboutData( "klineedittest", "klineedittest", "1.0" );
    TDECmdLineArgs::init(argc, argv, &aboutData);
    TDEApplication::addCmdLineOptions();
    
    TDEApplication a;    
    KLineEditTest *t = new KLineEditTest();
    //t->lineEdit()->setTrapReturnKey( true );
    //t->lineEdit()->completionBox()->setTabHandling( false );
    t->lineEdit()->setEnableSqueezedText( true );
    t->lineEdit()->setText ("This is a really really really really really really "
                            "really really long line because I am a talkative fool!");
    a.setMainWidget(t);
    t->show();
    return a.exec();
}

#include "klineedittest.moc"

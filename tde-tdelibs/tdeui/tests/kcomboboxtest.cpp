#define protected public // for delegate()
#include <kcombobox.h>
#undef protected

#include "kcomboboxtest.h"

#include <assert.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>

#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqpixmap.h>
#include <tqlabel.h>
#include <tqhbox.h>
#include <tqtimer.h>


KComboBoxTest::KComboBoxTest(TQWidget* widget, const char* name )
              :TQWidget(widget, name)
{
  TQVBoxLayout *vbox = new TQVBoxLayout (this, KDialog::marginHint(), KDialog::spacingHint());
  
  // Test for KCombo's KLineEdit destruction
  KComboBox *testCombo = new KComboBox( true, this ); // rw, with KLineEdit
  testCombo->setEditable( false ); // destroys our KLineEdit
  assert( testCombo->delegate() == 0L );
  delete testCombo; // not needed anymore  
  
  // Qt combobox
  TQHBox* hbox = new TQHBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  TQLabel* lbl = new TQLabel("&QCombobox:", hbox);
  lbl->setSizePolicy (TQSizePolicy::Maximum, TQSizePolicy::Preferred);
  
  m_qc = new TQComboBox(hbox, "QtReadOnlyCombo" );
  lbl->setBuddy (m_qc);  
  TQObject::connect (m_qc, TQ_SIGNAL(activated(int)), TQ_SLOT(slotActivated(int)));
  TQObject::connect (m_qc, TQ_SIGNAL(activated(const TQString&)), TQ_SLOT (slotActivated(const TQString&)));
  vbox->addWidget (hbox);
  
  // Read-only combobox
  hbox = new TQHBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  lbl = new TQLabel("&Read-Only Combo:", hbox);
  lbl->setSizePolicy (TQSizePolicy::Maximum, TQSizePolicy::Preferred);

  m_ro = new KComboBox(hbox, "ReadOnlyCombo" );
  lbl->setBuddy (m_ro);
  m_ro->setCompletionMode( TDEGlobalSettings::CompletionAuto );
  TQObject::connect (m_ro, TQ_SIGNAL(activated(int)), TQ_SLOT(slotActivated(int)));
  TQObject::connect (m_ro, TQ_SIGNAL(activated(const TQString&)), TQ_SLOT (slotActivated(const TQString&)));
  vbox->addWidget (hbox);

  // Read-write combobox
  hbox = new TQHBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  lbl = new TQLabel("&Editable Combo:", hbox);
  lbl->setSizePolicy (TQSizePolicy::Maximum, TQSizePolicy::Preferred);

  m_rw = new KComboBox( true, hbox, "ReadWriteCombo" );
  lbl->setBuddy (m_rw);
  m_rw->setDuplicatesEnabled( true );
  m_rw->setInsertionPolicy( TQComboBox::NoInsertion );
  m_rw->setTrapReturnKey( true );
  TQObject::connect (m_rw, TQ_SIGNAL(activated(int)), TQ_SLOT(slotActivated(int)));
  TQObject::connect (m_rw, TQ_SIGNAL(activated(const TQString&)), TQ_SLOT(slotActivated(const TQString&)));
  TQObject::connect (m_rw, TQ_SIGNAL(returnPressed()), TQ_SLOT(slotReturnPressed()));
  TQObject::connect (m_rw, TQ_SIGNAL(returnPressed(const TQString&)), TQ_SLOT(slotReturnPressed(const TQString&)));
  vbox->addWidget (hbox);

  // History combobox...
  hbox = new TQHBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  lbl = new TQLabel("&History Combo:", hbox);
  lbl->setSizePolicy (TQSizePolicy::Maximum, TQSizePolicy::Preferred);

  m_hc = new KHistoryCombo( true, hbox, "HistoryCombo" );
  lbl->setBuddy (m_hc);
  m_hc->setDuplicatesEnabled( true );
  m_hc->setInsertionPolicy( TQComboBox::NoInsertion );
  TQObject::connect (m_hc, TQ_SIGNAL(activated(int)), TQ_SLOT(slotActivated(int)));
  TQObject::connect (m_hc, TQ_SIGNAL(activated(const TQString&)), TQ_SLOT(slotActivated(const TQString&)));
  TQObject::connect (m_hc, TQ_SIGNAL(returnPressed()), TQ_SLOT(slotReturnPressed()));  
  vbox->addWidget (hbox);
  m_hc->setTrapReturnKey(true);

  // Read-write combobox that is a replica of code in konqueror...
  hbox = new TQHBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  lbl = new TQLabel( "&Konq's Combo:", hbox);
  lbl->setSizePolicy (TQSizePolicy::Maximum, TQSizePolicy::Preferred);

  m_konqc = new KComboBox( true, hbox, "KonqyCombo" );
  lbl->setBuddy (m_konqc);
  m_konqc->setMaxCount( 10 );
  TQObject::connect (m_konqc, TQ_SIGNAL(activated(int)), TQ_SLOT(slotActivated(int)));
  TQObject::connect (m_konqc, TQ_SIGNAL(activated(const TQString&)), TQ_SLOT (slotActivated(const TQString&)));
  TQObject::connect (m_konqc, TQ_SIGNAL(returnPressed()), TQ_SLOT(slotReturnPressed()));
  vbox->addWidget (hbox);

  // Create an exit button
  hbox = new TQHBox (this);
  m_btnExit = new TQPushButton( "E&xit", hbox );
  TQObject::connect( m_btnExit, TQ_SIGNAL(clicked()), TQ_SLOT(quitApp()) );

  // Create a disable button...
  m_btnEnable = new TQPushButton( "Disa&ble", hbox );
  TQObject::connect (m_btnEnable, TQ_SIGNAL(clicked()), TQ_SLOT(slotDisable()));

  vbox->addWidget (hbox);

  // Popuplate the select-only list box
  TQStringList list;
  list << "Stone" << "Tree" << "Peables" << "Ocean" << "Sand" << "Chips"
       << "Computer" << "Mankind";
  list.sort();
  
  // Setup the qcombobox
  m_qc->insertStringList (list);
  
  // Setup read-only combo
  m_ro->insertStringList( list );
  m_ro->completionObject()->setItems( list );

  // Setup read-write combo
  m_rw->insertStringList( list );
  m_rw->completionObject()->setItems( list );

  // Setup read-write combo
  m_hc->insertStringList( list );
  m_hc->completionObject()->setItems( list );

  // Setup konq's combobox
  KSimpleConfig historyConfig( "konq_history" );
  historyConfig.setGroup( "Location Bar" );
  TDECompletion * s_pCompletion = new TDECompletion;
  s_pCompletion->setOrder( TDECompletion::Weighted );
  s_pCompletion->setItems( historyConfig.readListEntry( "ComboContents" ) );
  s_pCompletion->setCompletionMode( TDEGlobalSettings::completionMode() );
  m_konqc->setCompletionObject( s_pCompletion );

  TQPixmap pix = SmallIcon("www");
  m_konqc->insertItem( pix, "http://www.kde.org" );
  m_konqc->setCurrentItem( m_konqc->count()-1 );

  m_timer = new TQTimer (this);
  connect (m_timer, TQ_SIGNAL (timeout()), TQ_SLOT (slotTimeout()));
}

KComboBoxTest::~KComboBoxTest()
{
  if (m_timer)
  {
    delete m_timer;
    m_timer = 0;
  }
}

void KComboBoxTest::slotDisable ()
{
  if (m_timer->isActive())
    return;

  m_btnEnable->setEnabled (!m_btnEnable->isEnabled());

  m_timer->start (5000, true);
}

void KComboBoxTest::slotTimeout ()
{
  bool enabled = m_ro->isEnabled();

  if (enabled)
    m_btnEnable->setText ("Ena&ble");
  else
    m_btnEnable->setText ("Disa&ble");
  
  m_qc->setEnabled (!enabled);  
  m_ro->setEnabled (!enabled);
  m_rw->setEnabled (!enabled);
  m_hc->setEnabled (!enabled);
  m_konqc->setEnabled (!enabled);

  m_btnEnable->setEnabled (!m_btnEnable->isEnabled());
}

void KComboBoxTest::slotActivated( int index )
{
  kdDebug() << "Activated Combo: " << sender()->name() << ", index:" << index << endl;
}

void KComboBoxTest::slotActivated (const TQString& item)
{
  kdDebug() << "Activated Combo: " << sender()->name() << ", item: " << item << endl;
}

void KComboBoxTest::slotReturnPressed ()
{
  kdDebug() << "Return Pressed: " << sender()->name() << endl;
}

void KComboBoxTest::slotReturnPressed(const TQString& item)
{
  kdDebug() << "Return Pressed, value = " << item << endl;
}

void KComboBoxTest::quitApp()
{
  kapp->closeAllWindows();
}

int main ( int argc, char **argv)
{
  TDEApplication a(argc, argv, "kcomboboxtest");
  KComboBoxTest* t= new KComboBoxTest;
  a.setMainWidget (t);
  t->show ();
  return a.exec();
}

#include "kcomboboxtest.moc"

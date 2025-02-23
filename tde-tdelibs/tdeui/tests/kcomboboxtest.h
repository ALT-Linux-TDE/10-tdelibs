#ifndef _KCOMBOBOXTEST_H
#define _KCOMBOBOXTEST_H

#include <tqwidget.h>

class TQTimer;
class TQComboBox;
class TQPushButton;

class KComboBox;

class KComboBoxTest : public TQWidget
{
  TQ_OBJECT
    
public:
  KComboBoxTest ( TQWidget *parent=0, const char *name=0 );
  ~KComboBoxTest();

private slots:
  void quitApp();
  void slotTimeout();
  void slotDisable();
  void slotReturnPressed();
  void slotReturnPressed(const TQString&);
  void slotActivated( int );
  void slotActivated( const TQString& );
   
protected:
  TQComboBox* m_qc;
  
  KComboBox* m_ro;
  KComboBox* m_rw;
  KComboBox* m_hc;
  KComboBox* m_konqc;


  TQPushButton* m_btnExit;
  TQPushButton* m_btnEnable;

  TQTimer* m_timer;
};

#endif

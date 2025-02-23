#ifndef _KLINEEDITTEST_H
#define _KLINEEDITTEST_H

#include <tqwidget.h>
#include <tqguardedptr.h>

class TQString;
class TQPushButton;

class KLineEdit;

class KLineEditTest : public TQWidget
{
    TQ_OBJECT

public:
   KLineEditTest( TQWidget *parent=0, const char *name=0 );
   ~KLineEditTest();
   KLineEdit* lineEdit() const { return m_lineedit; }

public slots:
   virtual void show ();
   
private slots:
   void quitApp();
   void slotHide();   
   void slotEnable( bool );
   void slotReadOnly( bool );   
   void slotReturnPressed();
   void resultOutput( const TQString& );   
   void slotReturnPressed( const TQString& );
   
protected:
   TQGuardedPtr<KLineEdit> m_lineedit;
   TQPushButton* m_btnExit;
   TQPushButton* m_btnReadOnly;
   TQPushButton* m_btnEnable;
   TQPushButton* m_btnHide;
};

#endif

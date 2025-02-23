/****************************************************************************
** Form interface generated from reading ui file './kcompletiontest.ui'
**
** Created: Wed Nov 15 20:12:56 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef FORM1_H
#define FORM1_H

#include <tqstringlist.h>
#include <tqvariant.h>
#include <tqwidget.h>
class TQVBoxLayout;
class TQHBoxLayout;
class TQGridLayout;
class TQGroupBox;
class TQLabel;
class TQListBox;
class TQListBoxItem;
class TQPushButton;

class KHistoryCombo;
class KLineEdit;


class Form1 : public TQWidget
{
    TQ_OBJECT

public:
    Form1( TQWidget* parent = 0, const char* name = 0 );
    ~Form1();

    TQGroupBox* GroupBox1;
    TQLabel* TextLabel1;
    KLineEdit* LineEdit1;
    TQPushButton* PushButton1;
    TQPushButton* PushButton1_4;
    TQListBox* ListBox1;
    TQPushButton* PushButton1_3;
    TQPushButton* PushButton1_2;

    KLineEdit* edit;
    KHistoryCombo *combo;

protected slots:
    void slotList();
    void slotAdd();
    void slotRemove();
    void slotHighlighted( const TQString& );

protected:
    TQStringList defaultItems() const;

    TQVBoxLayout* Form1Layout;
    TQVBoxLayout* GroupBox1Layout;
    TQVBoxLayout* Layout9;
    TQHBoxLayout* Layout1;
    TQHBoxLayout* Layout2;
    TQHBoxLayout* Layout3;
    TQHBoxLayout* Layout8;
    TQVBoxLayout* Layout7;
};

#endif // FORM1_H

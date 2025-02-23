#include <tdelocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './kcompletiontest.ui'
**
** Created: Wed Nov 15 20:15:10 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "kcompletiontest.h"

#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlistbox.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqvariant.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>

#include <tdeapplication.h>
#include <klineedit.h>
#include <kcombobox.h>
/*
 *  Constructs a Form1 which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
Form1::Form1( TQWidget* parent,  const char* name )
    : TQWidget( parent, name, WDestructiveClose )
{
    if ( !name )
	setName( "Form1" );
    resize( 559, 465 );
    setCaption(  "Form1" );
    Form1Layout = new TQVBoxLayout( this );
    Form1Layout->setSpacing( 6 );
    Form1Layout->setMargin( 11 );

    GroupBox1 = new TQGroupBox( this, "GroupBox1" );
    GroupBox1->setTitle( "Completion Test" );
    GroupBox1->setColumnLayout(0, TQt::Vertical );
    GroupBox1->layout()->setSpacing( 0 );
    GroupBox1->layout()->setMargin( 0 );
    GroupBox1Layout = new TQVBoxLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( TQt::AlignTop );
    GroupBox1Layout->setSpacing( 6 );
    GroupBox1Layout->setMargin( 11 );

    Layout9 = new TQVBoxLayout;
    Layout9->setSpacing( 6 );
    Layout9->setMargin( 0 );

    Layout1 = new TQHBoxLayout;
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );

    TextLabel1 = new TQLabel( GroupBox1, "TextLabel1" );
    TextLabel1->setText(  "Completion"  );
    Layout1->addWidget( TextLabel1 );

    edit = new KLineEdit( GroupBox1, "edit" );
    Layout1->addWidget( edit );
    Layout9->addLayout( Layout1 );
    edit->completionObject()->setItems( defaultItems() );
    edit->completionObject()->setIgnoreCase( true );
    edit->setFocus();
    TQToolTip::add( edit, "right-click to change completion mode" );

    Layout2 = new TQHBoxLayout;
    Layout2->setSpacing( 6 );
    Layout2->setMargin( 0 );

    combo = new KHistoryCombo( GroupBox1, "history combo" );
    combo->setCompletionObject( edit->completionObject() );
    // combo->setMaxCount( 5 );
    combo->setHistoryItems( defaultItems(), true );
    connect( combo, TQ_SIGNAL( activated( const TQString& )),
	     combo, TQ_SLOT( addToHistory( const TQString& )));
    TQToolTip::add( combo, "KHistoryCombo" );
    Layout2->addWidget( combo );

    LineEdit1 = new KLineEdit( GroupBox1, "LineEdit1" );
    Layout2->addWidget( LineEdit1 );

    PushButton1 = new TQPushButton( GroupBox1, "PushButton1" );
    PushButton1->setText( "Add" );
    connect( PushButton1, TQ_SIGNAL( clicked() ), TQ_SLOT( slotAdd() ));
    Layout2->addWidget( PushButton1 );
    Layout9->addLayout( Layout2 );

    Layout3 = new TQHBoxLayout;
    Layout3->setSpacing( 6 );
    Layout3->setMargin( 0 );
    TQSpacerItem* spacer = new TQSpacerItem( 20, 20, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
    Layout3->addItem( spacer );

    PushButton1_4 = new TQPushButton( GroupBox1, "PushButton1_4" );
    PushButton1_4->setText( "Remove" );
    connect( PushButton1_4, TQ_SIGNAL( clicked() ), TQ_SLOT( slotRemove() ));
    Layout3->addWidget( PushButton1_4 );
    Layout9->addLayout( Layout3 );

    Layout8 = new TQHBoxLayout;
    Layout8->setSpacing( 6 );
    Layout8->setMargin( 0 );

    ListBox1 = new TQListBox( GroupBox1, "ListBox1" );
    Layout8->addWidget( ListBox1 );
    connect( ListBox1, TQ_SIGNAL( highlighted( const TQString& )),
	     TQ_SLOT( slotHighlighted( const TQString& )));
    TQToolTip::add( ListBox1, "Contains the contents of the completion object.\n:x is the weighting, i.e. how often an item has been inserted");

    Layout7 = new TQVBoxLayout;
    Layout7->setSpacing( 6 );
    Layout7->setMargin( 0 );

    PushButton1_3 = new TQPushButton( GroupBox1, "PushButton1_3" );
    PushButton1_3->setText( "Completion items" );
    connect( PushButton1_3, TQ_SIGNAL( clicked() ), TQ_SLOT( slotList() ));
    Layout7->addWidget( PushButton1_3 );

    PushButton1_2 = new TQPushButton( GroupBox1, "PushButton1_2" );
    PushButton1_2->setText( "Clear" );
    connect( PushButton1_2, TQ_SIGNAL( clicked() ),
	     edit->completionObject(), TQ_SLOT( clear() ));
    Layout7->addWidget( PushButton1_2 );
    Layout8->addLayout( Layout7 );
    Layout9->addLayout( Layout8 );
    GroupBox1Layout->addLayout( Layout9 );
    Form1Layout->addWidget( GroupBox1 );

    slotList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
Form1::~Form1()
{
    // no need to delete child widgets, Qt does it all for us
}

void Form1::slotAdd()
{
    tqDebug("** adding: %s", LineEdit1->text().latin1() );
    edit->completionObject()->addItem( LineEdit1->text() );
    
    TQStringList matches = edit->completionObject()->allMatches("S");
    TQStringList::ConstIterator it = matches.begin();
    for ( ; it != matches.end(); ++it )
        tqDebug("-- %s", (*it).latin1());
}

void Form1::slotRemove()
{
    edit->completionObject()->removeItem( LineEdit1->text() );
}

void Form1::slotList()
{
    ListBox1->clear();
    TQStringList items = edit->completionObject()->items();
    ListBox1->insertStringList( items );
}

void Form1::slotHighlighted( const TQString& text )
{
    // remove any "weighting"
    int index = text.findRev( ':' );
    if ( index > 0 )
	LineEdit1->setText( text.left( index ) );
    else
	LineEdit1->setText( text );
}


TQStringList Form1::defaultItems() const
{
    TQStringList items;
    items << "Super" << "Sushi" << "Samson" << "Sucks" << "Sumo" << "Schumi";
    items << "Slashdot" << "sUpEr" << "SUshi" << "sUshi" << "sUShi";
    items << "sushI" << "SushI";
    return items;
}


int main(int argc, char **argv )
{
    TDEApplication app( argc, argv, "kcompletiontest" );

    Form1 *form = new Form1();
    form->show();

    return app.exec();
}


#include "kcompletiontest.moc"

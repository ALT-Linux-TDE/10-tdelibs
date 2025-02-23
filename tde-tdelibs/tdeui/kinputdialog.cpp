/*
  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqvalidator.h>
#include <tqwhatsthis.h>

#include <klineedit.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <tdelistbox.h>
#include <ktextedit.h>

#include "kinputdialog.h"

class KInputDialogPrivate
{
  public:
    KInputDialogPrivate();

    TQLabel *m_label;
    KLineEdit *m_lineEdit;
    KIntSpinBox *m_intSpinBox;
    KDoubleSpinBox *m_doubleSpinBox;
    KComboBox *m_comboBox;
    TDEListBox *m_listBox;
    KTextEdit *m_textEdit;
};

KInputDialogPrivate::KInputDialogPrivate()
    : m_label( 0L ), m_lineEdit( 0L ), m_intSpinBox( 0L ),
      m_doubleSpinBox( 0L ), m_comboBox( 0L )
{
}

KInputDialog::KInputDialog( const TQString &caption, const TQString &label,
    const TQString &value, TQWidget *parent, const char *name,
    TQValidator *validator, const TQString &mask )
    : KDialogBase( parent, name, true, caption, Ok|Cancel|User1, Ok, true,
    KStdGuiItem::clear() ),
    d( new KInputDialogPrivate() )
{
  TQFrame *frame = makeMainWidget();
  TQVBoxLayout *layout = new TQVBoxLayout( frame, 0, spacingHint() );

  d->m_label = new TQLabel( label, frame );
  layout->addWidget( d->m_label );

  d->m_lineEdit = new KLineEdit( value, frame );
  layout->addWidget( d->m_lineEdit );

  d->m_lineEdit->setFocus();
  d->m_label->setBuddy( d->m_lineEdit );

  layout->addStretch();

  if ( validator )
    d->m_lineEdit->setValidator( validator );

  if ( !mask.isEmpty() )
    d->m_lineEdit->setInputMask( mask );

  connect( d->m_lineEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
      TQ_SLOT( slotEditTextChanged( const TQString & ) ) );
  connect( this, TQ_SIGNAL( user1Clicked() ), d->m_lineEdit, TQ_SLOT( clear() ) );

  slotEditTextChanged( value );
  setMinimumWidth( 350 );
}

KInputDialog::KInputDialog( const TQString &caption, const TQString &label,
    const TQString &value, TQWidget *parent, const char *name )
    : KDialogBase( parent, name, true, caption, Ok|Cancel|User1, Ok, false,
    KStdGuiItem::clear() ),
    d( new KInputDialogPrivate() )
{
  TQFrame *frame = makeMainWidget();
  TQVBoxLayout *layout = new TQVBoxLayout( frame, 0, spacingHint() );

  d->m_label = new TQLabel( label, frame );
  layout->addWidget( d->m_label );

  d->m_textEdit = new KTextEdit( frame );
  d->m_textEdit->setTextFormat( PlainText );
  d->m_textEdit->setText( value );
  layout->addWidget( d->m_textEdit, 10 );

  d->m_textEdit->setFocus();
  d->m_label->setBuddy( d->m_textEdit );

  connect( this, TQ_SIGNAL( user1Clicked() ), d->m_textEdit, TQ_SLOT( clear() ) );

  setMinimumWidth( 400 );
}

KInputDialog::KInputDialog( const TQString &caption, const TQString &label,
    int value, int minValue, int maxValue, int step, int base,
    TQWidget *parent, const char *name )
    : KDialogBase( parent, name, true, caption, Ok|Cancel, Ok, true ),
    d( new KInputDialogPrivate() )
{
  TQFrame *frame = makeMainWidget();
  TQVBoxLayout *layout = new TQVBoxLayout( frame, 0, spacingHint() );

  d->m_label = new TQLabel( label, frame );
  layout->addWidget( d->m_label );

  d->m_intSpinBox = new KIntSpinBox( minValue, maxValue, step, value,
      base, frame );
  layout->addWidget( d->m_intSpinBox );

  layout->addStretch();

  d->m_intSpinBox->setFocus();
  setMinimumWidth( 300 );
}

KInputDialog::KInputDialog( const TQString &caption, const TQString &label,
    double value, double minValue, double maxValue, double step, int decimals,
    TQWidget *parent, const char *name )
    : KDialogBase( parent, name, true, caption, Ok|Cancel, Ok, true ),
    d( new KInputDialogPrivate() )
{
  TQFrame *frame = makeMainWidget();
  TQVBoxLayout *layout = new TQVBoxLayout( frame, 0, spacingHint() );

  d->m_label = new TQLabel( label, frame );
  layout->addWidget( d->m_label );

  d->m_doubleSpinBox = new KDoubleSpinBox( minValue, maxValue, step, value,
      decimals, frame );
  layout->addWidget( d->m_doubleSpinBox );

  layout->addStretch();

  d->m_doubleSpinBox->setFocus();
  setMinimumWidth( 300 );
}

KInputDialog::KInputDialog( const TQString &caption, const TQString &label,
    const TQStringList &list, int current, bool editable, TQWidget *parent,
    const char *name )
    : KDialogBase( parent, name, true, caption, Ok|Cancel|User1, Ok, true,
    KStdGuiItem::clear() ),
    d( new KInputDialogPrivate() )
{
  showButton( User1, editable );

  TQFrame *frame = makeMainWidget();
  TQVBoxLayout *layout = new TQVBoxLayout( frame, 0, spacingHint() );

  d->m_label = new TQLabel( label, frame );
  layout->addWidget( d->m_label );

  if ( editable )
  {
    d->m_comboBox = new KComboBox( editable, frame );
    d->m_comboBox->insertStringList( list );
    d->m_comboBox->setCurrentItem( current );
    layout->addWidget( d->m_comboBox );

    connect( d->m_comboBox, TQ_SIGNAL( textChanged( const TQString & ) ),
      TQ_SLOT( slotUpdateButtons( const TQString & ) ) );
    connect( this, TQ_SIGNAL( user1Clicked() ),
      d->m_comboBox, TQ_SLOT( clearEdit() ) );
    slotUpdateButtons( d->m_comboBox->currentText() );
    d->m_comboBox->setFocus();
  } else {
    d->m_listBox = new TDEListBox( frame );
    d->m_listBox->insertStringList( list );
    d->m_listBox->setSelected( current, true );
    d->m_listBox->ensureCurrentVisible();
    layout->addWidget( d->m_listBox, 10 );
    connect( d->m_listBox, TQ_SIGNAL( doubleClicked( TQListBoxItem * ) ),
      TQ_SLOT( slotOk() ) );
    connect( d->m_listBox, TQ_SIGNAL( returnPressed( TQListBoxItem * ) ),
      TQ_SLOT( slotOk() ) );

    d->m_listBox->setFocus();
  }

  layout->addStretch();

  setMinimumWidth( 320 );
}

KInputDialog::KInputDialog( const TQString &caption, const TQString &label,
    const TQStringList &list, const TQStringList &select, bool multiple,
    TQWidget *parent, const char *name )
    : KDialogBase( parent, name, true, caption, Ok|Cancel, Ok, true ),
    d( new KInputDialogPrivate() )
{
  TQFrame *frame = makeMainWidget();
  TQVBoxLayout *layout = new TQVBoxLayout( frame, 0, spacingHint() );

  d->m_label = new TQLabel( label, frame );
  layout->addWidget( d->m_label );

  d->m_listBox = new TDEListBox( frame );
  d->m_listBox->insertStringList( list );
  layout->addWidget( d->m_listBox );

  TQListBoxItem *item;

  if ( multiple )
  {
    d->m_listBox->setSelectionMode( TQListBox::Extended );

    for ( TQStringList::ConstIterator it=select.begin(); it!=select.end(); ++it )
    {
      item = d->m_listBox->findItem( *it, CaseSensitive|ExactMatch );
      if ( item )
        d->m_listBox->setSelected( item, true );
    }
  }
  else
  {
    connect( d->m_listBox, TQ_SIGNAL( doubleClicked( TQListBoxItem * ) ),
      TQ_SLOT( slotOk() ) );
    connect( d->m_listBox, TQ_SIGNAL( returnPressed( TQListBoxItem * ) ),
      TQ_SLOT( slotOk() ) );

    TQString text = select.first();
    item = d->m_listBox->findItem( text, CaseSensitive|ExactMatch );
    if ( item )
      d->m_listBox->setSelected( item, true );
  }

  d->m_listBox->ensureCurrentVisible();
  d->m_listBox->setFocus();

  layout->addStretch();

  setMinimumWidth( 320 );
}

KInputDialog::~KInputDialog()
{
  delete d;
}

TQString KInputDialog::getText( const TQString &caption, const TQString &label,
    const TQString &value, bool *ok, TQWidget *parent, const char *name,
    TQValidator *validator, const TQString &mask )
{
  return text( caption, label, value, ok, parent, name, validator, mask,
               TQString::null );
}

TQString KInputDialog::text( const TQString &caption,
    const TQString &label, const TQString &value, bool *ok, TQWidget *parent,
    const char *name, TQValidator *validator, const TQString &mask,
    const TQString &whatsThis )
{
  KInputDialog dlg( caption, label, value, parent, name, validator, mask );

  if( !whatsThis.isEmpty() )
    TQWhatsThis::add( dlg.lineEdit(), whatsThis );

  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  TQString result;
  if ( _ok )
    result = dlg.lineEdit()->text();

  // A validator may explicitly allow leading and trailing whitespace
  if ( !validator )
    result = result.stripWhiteSpace();

  return result;
}

TQString KInputDialog::getMultiLineText( const TQString &caption,
    const TQString &label, const TQString &value, bool *ok,
    TQWidget *parent, const char *name )
{
  KInputDialog dlg( caption, label, value, parent, name );

  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  TQString result;
  if ( _ok )
    result = dlg.textEdit()->text();

  return result;
}

int KInputDialog::getInteger( const TQString &caption, const TQString &label,
    int value, int minValue, int maxValue, int step, int base, bool *ok,
    TQWidget *parent, const char *name )
{
  KInputDialog dlg( caption, label, value, minValue,
    maxValue, step, base, parent, name );

  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  int result=0;
  if ( _ok )
    result = dlg.intSpinBox()->value();

  return result;
}

int KInputDialog::getInteger( const TQString &caption, const TQString &label,
    int value, int minValue, int maxValue, int step, bool *ok,
    TQWidget *parent, const char *name )
{
  return getInteger( caption, label, value, minValue, maxValue, step,
    10, ok, parent, name );
}

double KInputDialog::getDouble( const TQString &caption, const TQString &label,
    double value, double minValue, double maxValue, double step, int decimals,
    bool *ok, TQWidget *parent, const char *name )
{
  KInputDialog dlg( caption, label, value, minValue,
    maxValue, step, decimals, parent, name );

  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  double result=0;
  if ( _ok )
    result = dlg.doubleSpinBox()->value();

  return result;
}

double KInputDialog::getDouble( const TQString &caption, const TQString &label,
    double value, double minValue, double maxValue, int decimals,
    bool *ok, TQWidget *parent, const char *name )
{
  return getDouble( caption, label, value, minValue, maxValue, 0.1, decimals,
    ok, parent, name );
}

TQString KInputDialog::getItem( const TQString &caption, const TQString &label,
    const TQStringList &list, int current, bool editable, bool *ok,
    TQWidget *parent, const char *name )
{
  KInputDialog dlg( caption, label, list, current,
    editable, parent, name );
  if ( !editable)
  {
      connect( dlg.listBox(),  TQ_SIGNAL(doubleClicked ( TQListBoxItem *)), &dlg, TQ_SLOT( slotOk()));
  }
  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  TQString result;
  if ( _ok )
    if ( editable )
      result = dlg.comboBox()->currentText();
    else
      result = dlg.listBox()->currentText();

  return result;
}

TQStringList KInputDialog::getItemList( const TQString &caption,
    const TQString &label, const TQStringList &list, const TQStringList &select,
    bool multiple, bool *ok, TQWidget *parent, const char *name )
{
  KInputDialog dlg( caption, label, list, select,
    multiple, parent, name );

  bool _ok = ( dlg.exec() == Accepted );

  if ( ok )
    *ok = _ok;

  TQStringList result;
  if ( _ok )
  {
    for (const TQListBoxItem* i = dlg.listBox()->firstItem(); i != 0; i = i->next() )
      if ( i->isSelected() )
        result.append( i->text() );
  }

  return result;
}

void KInputDialog::slotEditTextChanged( const TQString &text )
{
  bool on;
  if ( lineEdit()->validator() ) {
    TQString str = lineEdit()->text();
    int index = lineEdit()->cursorPosition();
    on = ( lineEdit()->validator()->validate( str, index )
      == TQValidator::Acceptable );
  } else {
    on = !text.stripWhiteSpace().isEmpty();
  }

  enableButton( Ok, on );
  enableButton( User1, !text.isEmpty() );
}

void KInputDialog::slotUpdateButtons( const TQString &text )
{
  enableButton( Ok, !text.isEmpty() );
  enableButton( User1, !text.isEmpty() );
}

KLineEdit *KInputDialog::lineEdit() const
{
  return d->m_lineEdit;
}

KIntSpinBox *KInputDialog::intSpinBox() const
{
  return d->m_intSpinBox;
}

KDoubleSpinBox *KInputDialog::doubleSpinBox() const
{
  return d->m_doubleSpinBox;
}

KComboBox *KInputDialog::comboBox() const
{
  return d->m_comboBox;
}

TDEListBox *KInputDialog::listBox() const
{
  return d->m_listBox;
}

KTextEdit *KInputDialog::textEdit() const
{
  return d->m_textEdit;
}

#include "kinputdialog.moc"

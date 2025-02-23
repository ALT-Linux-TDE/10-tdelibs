/* This file is part of the KDE libraries
   Copyright (C) 1999 Preston Brown <pbrown@kde.org>

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
#include <config.h>

#include <tqvalidator.h>
#include <tqpushbutton.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqlayout.h>
#undef Unsorted // Required for --enable-final (tqdir.h)
#include <tqfiledialog.h>

#include <kbuttonbox.h>
#include <tdelocale.h>
#include <tdeapplication.h>
#include <klineedit.h>
#include <kstdguiitem.h>

#include "klineeditdlg.h"

KLineEditDlg::KLineEditDlg( const TQString&_text, const TQString& _value,
			    TQWidget *parent )
  : KDialogBase( Plain, TQString::null, Ok|Cancel|User1, Ok, parent, 0L, true,
		 true, KStdGuiItem::clear() )
{
  TQVBoxLayout *topLayout = new TQVBoxLayout( plainPage(), 0, spacingHint() );
  TQLabel *label = new TQLabel(_text, plainPage() );
  topLayout->addWidget( label, 1 );

  edit = new KLineEdit( plainPage(), 0L );
  edit->setMinimumWidth(edit->sizeHint().width() * 3);
  label->setBuddy(edit);  // please "scheck" style
  //  connect( edit, TQ_SIGNAL(returnPressed()), TQ_SLOT(accept()) );
  connect( edit, TQ_SIGNAL(textChanged(const TQString&)),
	   TQ_SLOT(slotTextChanged(const TQString&)) );
  topLayout->addWidget( edit, 1 );

  connect( this, TQ_SIGNAL(user1Clicked()), this, TQ_SLOT(slotClear()) );
  edit->setText( _value );
  if ( _value.isEmpty() )
  {
      enableButtonOK( false );
      enableButton(KDialogBase::User1, false);
  }
  edit->setSelection(0, edit->text().length());
  edit->setFocus();
}



#if 0
KLineEditDlg::KLineEditDlg( const TQString&_text, const TQString& _value,
			    TQWidget *parent, bool _file_mode )
    : TQDialog( parent, 0L, true )
{
  TQGridLayout *layout = new TQGridLayout(this, 4, 3, 10);

  TQLabel *label = new TQLabel(_text, this);
  layout->addWidget(label, 0, 0, AlignLeft);

  edit = new KLineEdit( this, 0L );
  edit->setMinimumWidth(edit->sizeHint().width() * 3);
  connect( edit, TQ_SIGNAL(returnPressed()), TQ_SLOT(accept()) );

  if ( _file_mode ) {
    completion = new KURLCompletion();
  	edit->setCompletionObject( completion );
	edit->setAutoDeleteCompletionObject( true );
  } else
    completion = 0L;

  layout->addMultiCellWidget(edit, 1, 1, 0, _file_mode ? 1 : 2);
  layout->setColStretch(1, 1);

  if (_file_mode) {
    TQPushButton *browse = new TQPushButton(i18n("&Browse..."), this);
    layout->addWidget(browse, 1, 2, AlignCenter);
    connect(browse, TQ_SIGNAL(clicked()),
	    TQ_SLOT(slotBrowse()));
  }

  TQFrame *hLine = new TQFrame(this);
  hLine->setFrameStyle(TQFrame::Sunken|TQFrame::HLine);
  layout->addMultiCellWidget(hLine, 2, 2, 0, 2);

  KButtonBox *bBox = new KButtonBox(this);
  layout->addMultiCellWidget(bBox, 3, 3, 0, 2);

  TQPushButton *ok = bBox->addButton(KStdGuiItem::ok());
  ok->setDefault(true);
  connect( ok, TQ_SIGNAL(clicked()), TQ_SLOT(accept()));

  bBox->addStretch(1);

  TQPushButton *clear = bBox->addButton(KStdGuiItem::clear());
  connect( clear, TQ_SIGNAL(clicked()), TQ_SLOT(slotClear()));

  bBox->addStretch(1);

  TQPushButton *cancel = bBox->addButton(KStdGuiItem::cancel());
  connect( cancel, TQ_SIGNAL(clicked()), TQ_SLOT(reject()));

  bBox->layout();

  layout->activate();

  edit->setText( _value );
  edit->setSelection(0, edit->text().length());
  edit->setFocus();
}
#endif


KLineEditDlg::~KLineEditDlg()
{
}

void KLineEditDlg::slotClear()
{
    edit->setText(TQString::null);
}

void KLineEditDlg::slotTextChanged(const TQString &text)
{
  bool on;
  if ( edit->validator() ) {
    TQString str = edit->text();
    int index = edit->cursorPosition();
    on = ( edit->validator()->validate( str, index )
	   == TQValidator::Acceptable );
  } else {
    on = !text.isEmpty();
  }
  enableButtonOK( on );
  enableButton(KDialogBase::User1, text.length());
}

TQString KLineEditDlg::text() const
{
    return edit->text();
}

TQString KLineEditDlg::getText(const TQString &_text, const TQString& _value,
                              bool *ok, TQWidget *parent, TQValidator *_validator )
{
    KLineEditDlg dlg(_text, _value, parent );
    dlg.lineEdit()->setValidator( _validator );
    dlg.slotTextChanged( _value ); // trigger validation

    bool ok_ = dlg.exec() == TQDialog::Accepted;
    if ( ok )
        *ok = ok_;
    if ( ok_ )
        return dlg.text();
    return TQString::null;
}

TQString KLineEditDlg::getText(const TQString &_caption, const TQString &_text,
                              const TQString& _value,
                              bool *ok, TQWidget *parent, TQValidator *_validator )
{
    KLineEditDlg dlg( _text, _value, parent );
    dlg.setCaption( _caption );
    dlg.lineEdit()->setValidator( _validator );
    dlg.slotTextChanged( _value ); // trigger validation

    bool ok_ = dlg.exec() == TQDialog::Accepted;
    if ( ok )
        *ok = ok_;
    if ( ok_ )
        return dlg.text();
    return TQString::null;
}

void KLineEditDlg::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "klineeditdlg.moc"

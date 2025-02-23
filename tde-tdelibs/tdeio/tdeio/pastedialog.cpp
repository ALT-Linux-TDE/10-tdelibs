/* This file is part of the KDE libraries
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "pastedialog.h"

#include <klineedit.h>
#include <kmimetype.h>
#include <tdelocale.h>

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqapplication.h>
#include <tqclipboard.h>

TDEIO::PasteDialog::PasteDialog( const TQString &caption, const TQString &label,
                               const TQString &value, const TQStringList& items,
                               TQWidget *parent,
                               bool clipboard )
    : KDialogBase( parent, 0 /*name*/, true, caption, Ok|Cancel, Ok, true )
{
    TQFrame *frame = makeMainWidget();
    TQVBoxLayout *layout = new TQVBoxLayout( frame, 0, spacingHint() );

    m_label = new TQLabel( label, frame );
    layout->addWidget( m_label );

    m_lineEdit = new KLineEdit( value, frame );
    layout->addWidget( m_lineEdit );

    m_lineEdit->setFocus();
    m_label->setBuddy( m_lineEdit );

    layout->addWidget( new TQLabel( i18n( "Data format:" ), frame ) );
    m_comboBox = new TQComboBox( frame );
    m_comboBox->insertStringList( items );
    layout->addWidget( m_comboBox );

    layout->addStretch();

    //connect( m_lineEdit, TQ_SIGNAL( textChanged( const TQString & ) ),
    //    TQ_SLOT( slotEditTextChanged( const TQString & ) ) );
    //connect( this, TQ_SIGNAL( user1Clicked() ), m_lineEdit, TQ_SLOT( clear() ) );

    //slotEditTextChanged( value );
    setMinimumWidth( 350 );

    m_clipboardChanged = false;
    if ( clipboard )
        connect( TQApplication::clipboard(), TQ_SIGNAL( dataChanged() ),
                 this, TQ_SLOT( slotClipboardDataChanged() ) );
}

void TDEIO::PasteDialog::slotClipboardDataChanged()
{
    m_clipboardChanged = true;
}

TQString TDEIO::PasteDialog::lineEditText() const
{
    return m_lineEdit->text();
}

int TDEIO::PasteDialog::comboItem() const
{
    return m_comboBox->currentItem();
}

#include "pastedialog.moc"

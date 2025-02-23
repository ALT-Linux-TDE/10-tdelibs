/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>, Alexander Neundorf <neundorf@kde.org>
   2000, 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <tqstringlist.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqgroupbox.h>
#include <tqlistbox.h>
#include <tqwhatsthis.h>
#include <tqlabel.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <tdeapplication.h>
#include <knotifyclient.h>

#include "keditlistbox.h"

#include <assert.h>

class KEditListBoxPrivate
{
public:
    bool m_checkAtEntering;
    uint buttons;
};

KEditListBox::KEditListBox(TQWidget *parent, const char *name,
			   bool checkAtEntering, int buttons )
    :TQGroupBox(parent, name ), d(new KEditListBoxPrivate)
{
    init( checkAtEntering, buttons );
}

KEditListBox::KEditListBox(const TQString& title, TQWidget *parent,
			   const char *name, bool checkAtEntering, int buttons)
    :TQGroupBox(title, parent, name ), d(new KEditListBoxPrivate)
{
    init( checkAtEntering, buttons );
}

KEditListBox::KEditListBox(const TQString& title, const CustomEditor& custom,
                           TQWidget *parent, const char *name,
                           bool checkAtEntering, int buttons)
    :TQGroupBox(title, parent, name ), d(new KEditListBoxPrivate)
{
    m_lineEdit = custom.lineEdit();
    init( checkAtEntering, buttons, custom.representationWidget() );
}

KEditListBox::~KEditListBox()
{
    delete d;
}

void KEditListBox::init( bool checkAtEntering, int buttons,
                         TQWidget *representationWidget )
{
    d->m_checkAtEntering = checkAtEntering;

    servNewButton = servRemoveButton = servUpButton = servDownButton = 0L;
    setSizePolicy(TQSizePolicy(TQSizePolicy::MinimumExpanding,
                              TQSizePolicy::MinimumExpanding));

    TQGridLayout * grid = new TQGridLayout(this, 7, 2,
                                         KDialog::marginHint(),
                                         KDialog::spacingHint());
    grid->addRowSpacing(0, fontMetrics().lineSpacing());
    grid->setRowStretch( 6, 1 );

    grid->setMargin(15);

    if ( representationWidget )
        representationWidget->reparent( this, TQPoint(0,0) );
    else
        m_lineEdit=new KLineEdit(this);

    m_listBox = new TQListBox(this);

    TQWidget *editingWidget = representationWidget ?
                             representationWidget : m_lineEdit;
    grid->addMultiCellWidget(editingWidget,1,1,0,1);
    grid->addMultiCellWidget(m_listBox, 2, 6, 0, 0);

    d->buttons = 0;
    setButtons( buttons );

    connect(m_lineEdit,TQ_SIGNAL(textChanged(const TQString&)),this,TQ_SLOT(typedSomething(const TQString&)));
    m_lineEdit->setTrapReturnKey(true);
    connect(m_lineEdit,TQ_SIGNAL(returnPressed()),this,TQ_SLOT(addItem()));
    connect(m_listBox, TQ_SIGNAL(highlighted(int)), TQ_SLOT(enableMoveButtons(int)));

    // maybe supplied lineedit has some text already
    typedSomething( m_lineEdit->text() );
}

void KEditListBox::setButtons( uint buttons )
{
    if ( d->buttons == buttons )
        return;

    TQGridLayout* grid = static_cast<TQGridLayout *>( layout() );
    if ( ( buttons & Add ) && !servNewButton ) {
        servNewButton = new TQPushButton(i18n("&Add"), this);
        servNewButton->setEnabled(false);
        servNewButton->show();
        connect(servNewButton, TQ_SIGNAL(clicked()), TQ_SLOT(addItem()));

        grid->addWidget(servNewButton, 2, 1);
    } else if ( ( buttons & Add ) == 0 && servNewButton ) {
        delete servNewButton;
        servNewButton = 0;
    }

    if ( ( buttons & Remove ) && !servRemoveButton ) {
        servRemoveButton = new TQPushButton(i18n("&Remove"), this);
        servRemoveButton->setEnabled(false);
        servRemoveButton->show();
        connect(servRemoveButton, TQ_SIGNAL(clicked()), TQ_SLOT(removeItem()));

        grid->addWidget(servRemoveButton, 3, 1);
    } else if ( ( buttons & Remove ) == 0 && servRemoveButton ) {
        delete servRemoveButton;
        servRemoveButton = 0;
    }

    if ( ( buttons & UpDown ) && !servUpButton ) {
        servUpButton = new TQPushButton(i18n("Move &Up"), this);
        servUpButton->setEnabled(false);
        servUpButton->show();
        connect(servUpButton, TQ_SIGNAL(clicked()), TQ_SLOT(moveItemUp()));

        servDownButton = new TQPushButton(i18n("Move &Down"), this);
        servDownButton->setEnabled(false);
        servDownButton->show();
        connect(servDownButton, TQ_SIGNAL(clicked()), TQ_SLOT(moveItemDown()));

        grid->addWidget(servUpButton, 4, 1);
        grid->addWidget(servDownButton, 5, 1);
    } else if ( ( buttons & UpDown ) == 0 && servUpButton ) {
        delete servUpButton; servUpButton = 0;
        delete servDownButton; servDownButton = 0;
    }

    d->buttons = buttons;
}

void KEditListBox::typedSomething(const TQString& text)
{
    if(currentItem() >= 0) {
        if(currentText() != m_lineEdit->text())
        {
            // IMHO changeItem() shouldn't do anything with the value
            // of currentItem() ... like changing it or emitting signals ...
            // but TT disagree with me on this one (it's been that way since ages ... grrr)
            bool block = m_listBox->signalsBlocked();
            const TQString& oldText = currentText();
            int item = currentItem();
            m_listBox->blockSignals( true );
            m_listBox->changeItem(text, item);
            m_listBox->blockSignals( block );
            emit changed();
            emit renamed(oldText, text);
            emit renamed(item, oldText, text);
        }
    }

    if ( !servNewButton )
        return;

    if (!d->m_checkAtEntering)
        servNewButton->setEnabled(!text.isEmpty());
    else
    {
        if (text.isEmpty())
        {
            servNewButton->setEnabled(false);
        }
        else
        {
            StringComparisonMode mode = (StringComparisonMode) (ExactMatch | CaseSensitive );
            bool enable = (!m_listBox->findItem( text, mode ));
            servNewButton->setEnabled( enable );
        }
    }
}

void KEditListBox::moveItemUp()
{
    if (!m_listBox->isEnabled())
    {
        KNotifyClient::beep();
        return;
    }

    const unsigned int selIndex = m_listBox->currentItem();
    if (selIndex == 0)
    {
        KNotifyClient::beep();
        return;
    }

    TQListBoxItem *selItem = m_listBox->item(selIndex);
    m_listBox->takeItem(selItem);
    m_listBox->insertItem(selItem, selIndex-1);
    m_listBox->setCurrentItem(selIndex - 1);

    emit changed();
}

void KEditListBox::moveItemDown()
{
    if (!m_listBox->isEnabled())
    {
        KNotifyClient::beep();
        return;
    }

    unsigned int selIndex = m_listBox->currentItem();
    if (selIndex == m_listBox->count() - 1)
    {
        KNotifyClient::beep();
        return;
    }

    TQListBoxItem *selItem = m_listBox->item(selIndex);
    m_listBox->takeItem(selItem);
    m_listBox->insertItem(selItem, selIndex+1);
    m_listBox->setCurrentItem(selIndex + 1);

    emit changed();
}

void KEditListBox::addItem()
{
    // when m_checkAtEntering is true, the add-button is disabled, but this
    // slot can still be called through Key_Return/Key_Enter. So we guard
    // against this.
    if ( !servNewButton || !servNewButton->isEnabled() )
        return;

    const TQString& currentTextLE=m_lineEdit->text();
    bool alreadyInList(false);
    //if we didn't check for dupes at the inserting we have to do it now
    if (!d->m_checkAtEntering)
    {
        // first check current item instead of dumb iterating the entire list
        if ( m_listBox->currentText() == currentTextLE )
            alreadyInList = true;
        else
        {
            StringComparisonMode mode = (StringComparisonMode) (ExactMatch | CaseSensitive );
            alreadyInList =(m_listBox->findItem(currentTextLE, mode) );
        }
    }

    if ( servNewButton )
        servNewButton->setEnabled(false);

    bool block = m_lineEdit->signalsBlocked();
    m_lineEdit->blockSignals(true);
    m_lineEdit->clear();
    m_lineEdit->blockSignals(block);

    int item = currentItem();
    m_listBox->setSelected(item, false);

    if (!alreadyInList)
    {
        block = m_listBox->signalsBlocked();
        m_listBox->blockSignals( true );
        m_listBox->insertItem(currentTextLE);
        m_listBox->blockSignals( block );
        emit changed();
        emit added( currentTextLE );
        emit added( item, currentTextLE );
    }
}

int KEditListBox::currentItem() const
{
    int nr = m_listBox->currentItem();
    if(nr >= 0 && !m_listBox->item(nr)->isSelected()) return -1;
    return nr;
}

void KEditListBox::removeItem()
{
    int item = m_listBox->currentItem();

    if ( item >= 0 )
    {
        TQString removedText = m_listBox->currentText();

        m_listBox->removeItem( item );
        if ( count() > 0 )
            m_listBox->setSelected( TQMIN( item, count() - 1 ), true );

        emit changed();
        emit removed( removedText );
        emit removed( item, removedText );
    }

    if ( servRemoveButton && m_listBox->currentItem() == -1 )
        servRemoveButton->setEnabled(false);
}

void KEditListBox::enableMoveButtons(int index)
{
    // Update the lineEdit when we select a different line.
    if(currentText() != m_lineEdit->text())
        m_lineEdit->setText(currentText());

    bool moveEnabled = servUpButton && servDownButton;

    if (moveEnabled )
    {
        if (m_listBox->count() <= 1)
        {
            servUpButton->setEnabled(false);
            servDownButton->setEnabled(false);
        }
        else if ((uint) index == (m_listBox->count() - 1))
        {
            servUpButton->setEnabled(true);
            servDownButton->setEnabled(false);
        }
        else if (index == 0)
        {
            servUpButton->setEnabled(false);
            servDownButton->setEnabled(true);
        }
        else
        {
            servUpButton->setEnabled(true);
            servDownButton->setEnabled(true);
        }
    }

    if ( servRemoveButton )
        servRemoveButton->setEnabled(true);
}

void KEditListBox::clear()
{
    m_lineEdit->clear();
    m_listBox->clear();
    emit changed();
}

void KEditListBox::insertStringList(const TQStringList& list, int index)
{
    m_listBox->insertStringList(list,index);
}

void KEditListBox::insertStrList(const TQStrList* list, int index)
{
    m_listBox->insertStrList(list,index);
}

void KEditListBox::insertStrList(const TQStrList& list, int index)
{
    m_listBox->insertStrList(list,index);
}

void KEditListBox::insertStrList(const char ** list, int numStrings, int index)
{
    m_listBox->insertStrList(list,numStrings,index);
}

TQStringList KEditListBox::items() const
{
    TQStringList list;
    for (TQListBoxItem const * i = m_listBox->firstItem(); i != 0; i = i->next() )
	list.append( i->text());

    return list;
}

void KEditListBox::setItems(const TQStringList& items)
{
  m_listBox->clear();
  m_listBox->insertStringList(items, 0);
}

int KEditListBox::buttons() const
{
  return d->buttons;
}

void KEditListBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KEditListBox::CustomEditor::CustomEditor( KComboBox *combo )
{
    m_representationWidget = combo;
    m_lineEdit = dynamic_cast<KLineEdit*>( combo->lineEdit() );
    assert( m_lineEdit );
}

#include "keditlistbox.moc"

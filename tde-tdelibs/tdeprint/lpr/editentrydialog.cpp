/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "editentrydialog.h"

#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqspinbox.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqheader.h>
#include <tdelistview.h>
#include <tqlayout.h>
#include <tqwidgetstack.h>
#include <tdelocale.h>
#include <kiconloader.h>

EditEntryDialog::EditEntryDialog(PrintcapEntry *entry, TQWidget *parent, const char *name)
: KDialogBase(parent, name, true, TQString::null, Ok|Cancel)
{
	TQWidget	*w = new TQWidget(this);
	setMainWidget(w);

	TQLabel	*lab0 = new TQLabel(i18n("Aliases:"), w);
	m_aliases = new TQLineEdit(w);
	m_view = new TDEListView(w);
	m_view->addColumn("");
	m_view->header()->hide();
	m_type = new TQComboBox(w);
	m_type->insertItem(i18n("String"));
	m_type->insertItem(i18n("Number"));
	m_type->insertItem(i18n("Boolean"));
	m_stack = new TQWidgetStack(w);
	m_boolean = new TQCheckBox(i18n("Enabled"), m_stack);
	m_string = new TQLineEdit(m_stack);
	m_number = new TQSpinBox(0, 9999, 1, m_stack);
	m_stack->addWidget(m_string, 0);
	m_stack->addWidget(m_boolean, 2);
	m_stack->addWidget(m_number, 1);
	m_name = new TQLineEdit(w);

	TQVBoxLayout	*l0 = new TQVBoxLayout(w, 0, 10);
	TQHBoxLayout	*l1 = new TQHBoxLayout(0, 0, 10);
	TQHBoxLayout	*l2 = new TQHBoxLayout(0, 0, 5);
	l0->addLayout(l1);
	l1->addWidget(lab0);
	l1->addWidget(m_aliases);
	l0->addWidget(m_view);
	l0->addLayout(l2);
	l2->addWidget(m_name, 0);
	l2->addWidget(m_type, 0);
	l2->addWidget(m_stack, 1);

	if (entry)
	{
		setCaption(i18n("Printcap Entry: %1").arg(entry->name));
		m_fields = entry->fields;
		m_aliases->setText(entry->aliases.join("|"));
		TQListViewItem	*root = new TQListViewItem(m_view, entry->name), *item = 0;
		root->setSelectable(false);
		root->setOpen(true);
		root->setPixmap(0, SmallIcon("document-print"));
		for (TQMap<TQString,Field>::ConstIterator it=m_fields.begin(); it!=m_fields.end(); ++it)
			item = new TQListViewItem(root, item, (*it).toString(), it.key());
	}

	m_block = true;
	enableButton(Ok, false);
	slotItemSelected(NULL);
	slotTypeChanged(0);
	m_block = false;

	connect(m_view, TQ_SIGNAL(selectionChanged(TQListViewItem*)), TQ_SLOT(slotItemSelected(TQListViewItem*)));
	connect(m_string, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotChanged()));
	connect(m_boolean, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotChanged()));
	connect(m_number, TQ_SIGNAL(valueChanged(int)), TQ_SLOT(slotChanged()));
	connect(m_type, TQ_SIGNAL(activated(int)), TQ_SLOT(slotTypeChanged(int)));
	connect(m_name, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotChanged()));

	resize(500,400);
}

Field EditEntryDialog::createField()
{
	Field	f;
	f.name = m_name->text();
	f.type = (Field::Type)(m_type->currentItem());
	switch (f.type)
	{
		case Field::String: f.value = m_string->text(); break;
		case Field::Integer: f.value = m_number->cleanText(); break;
		case Field::Boolean: f.value = (m_boolean->isChecked() ? "1" : "0"); break;
	}
	return f;
}

void EditEntryDialog::slotChanged()
{
	if (!m_block && m_view->currentItem())
	{
		Field	f = createField();
		if (f.name != m_current)
			m_fields.remove(m_current);
		m_fields[f.name] = f;
		m_view->currentItem()->setText(0, f.toString());
	}
}

void EditEntryDialog::slotItemSelected(TQListViewItem *item)
{
	m_stack->setEnabled(item);
	m_name->setEnabled(item);
	m_type->setEnabled(item);
	if (item)
	{
		m_block = true;
		m_current = item->text(1);
		Field	f = m_fields[m_current];
		m_name->setText(f.name);
		m_type->setCurrentItem(f.type);
		slotTypeChanged(f.type);
		m_string->setText(f.value);
		m_number->setValue(f.value.toInt());
		m_boolean->setChecked(f.value.toInt() == 1);
		m_block = false;
	}
}

void EditEntryDialog::fillEntry(PrintcapEntry *entry)
{
	entry->aliases = TQStringList::split('|', m_aliases->text(), false);
	entry->fields = m_fields;
}

void EditEntryDialog::slotTypeChanged(int ID)
{
	m_stack->raiseWidget(ID);
	slotChanged();
}

#include "editentrydialog.moc"

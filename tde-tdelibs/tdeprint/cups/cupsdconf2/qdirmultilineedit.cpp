/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <tdeprint@swing.be>
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

#include "qdirmultilineedit.h"

#include <tqlayout.h>
#include <tqheader.h>
#include <tqpushbutton.h>
#include <tdelistview.h>
#include <tdelocale.h>
#include <tdefiledialog.h>
#include <kiconloader.h>

QDirMultiLineEdit::QDirMultiLineEdit(TQWidget *parent, const char *name)
: TQWidget(parent, name)
{
	m_view = new TDEListView(this);
	m_view->header()->hide();
	m_view->addColumn("");
	m_view->setFullWidth(true);
	connect(m_view, TQ_SIGNAL(selectionChanged(TQListViewItem*)), TQ_SLOT(slotSelected(TQListViewItem*)));

	m_add = new TQPushButton(this);
	m_add->setPixmap(SmallIcon("folder-new"));
	connect(m_add, TQ_SIGNAL(clicked()), TQ_SLOT(slotAddClicked()));
	m_remove = new TQPushButton(this);
	m_remove->setPixmap(SmallIcon("edit-delete"));
	connect(m_remove, TQ_SIGNAL(clicked()), TQ_SLOT(slotRemoveClicked()));
	m_remove->setEnabled(false);

	m_view->setFixedHeight(TQMAX(m_view->fontMetrics().lineSpacing()*3+m_view->lineWidth()*2, m_add->sizeHint().height()*2));

	TQHBoxLayout	*l0 = new TQHBoxLayout(this, 0, 3);
	TQVBoxLayout	*l1 = new TQVBoxLayout(0, 0, 0);
	l0->addWidget(m_view);
	l0->addLayout(l1);
	l1->addWidget(m_add);
	l1->addWidget(m_remove);
	l1->addStretch(1);
}

QDirMultiLineEdit::~QDirMultiLineEdit()
{
}

void QDirMultiLineEdit::setURLs(const TQStringList& urls)
{
	m_view->clear();
	for (TQStringList::ConstIterator it=urls.begin(); it!=urls.end(); ++it)
		addURL(*it);
}

TQStringList QDirMultiLineEdit::urls()
{
	TQListViewItem	*item = m_view->firstChild();
	TQStringList	l;
	while (item)
	{
		l << item->text(0);
		item = item->nextSibling();
	}
	return l;
}

void QDirMultiLineEdit::addURL(const TQString& url)
{
	TQListViewItem	*item = new TQListViewItem(m_view, url);
	item->setRenameEnabled(0, true);
}

void QDirMultiLineEdit::slotAddClicked()
{
	TQString	dirname = KFileDialog::getExistingDirectory(TQString::null, this);
	if (!dirname.isEmpty())
		addURL(dirname);
}

void QDirMultiLineEdit::slotRemoveClicked()
{
	TQListViewItem	*item = m_view->currentItem();
	if (item)
	{
		delete item;
		slotSelected(m_view->currentItem());
	}
}

void QDirMultiLineEdit::slotSelected(TQListViewItem *item)
{
	m_remove->setEnabled((item != NULL));
}

#include "qdirmultilineedit.moc"

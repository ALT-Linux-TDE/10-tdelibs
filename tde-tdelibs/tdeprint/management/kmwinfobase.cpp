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

#include "kmwinfobase.h"

#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqlayout.h>

KMWInfoBase::KMWInfoBase(int n, TQWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_labels.setAutoDelete(false);
	m_edits.setAutoDelete(false);

	m_nlines = n;
	TQGridLayout	*lay1 = new TQGridLayout(this, m_nlines+3, 2, 0, 10);
	lay1->addRowSpacing(1,10);
	lay1->setRowStretch(m_nlines+2,1);
	lay1->setColStretch(1,1);

	m_info = new TQLabel(this);
	m_info->setTextFormat(TQt::RichText);
	lay1->addMultiCellWidget(m_info,0,0,0,1);

	for (int i=0;i<m_nlines;i++)
	{
		m_labels.append(new TQLabel(this));
		m_edits.append(new TQLineEdit(this));
		lay1->addWidget(m_labels.last(),i+2,0);
		lay1->addWidget(m_edits.last(),i+2,1);
	}
}

void KMWInfoBase::setInfo(const TQString& s)
{
	m_info->setText(s);
}

void KMWInfoBase::setLabel(int i, const TQString& s)
{
	if (i >= 0 && i < m_nlines)
		m_labels.at(i)->setText(s);
}

void KMWInfoBase::setText(int i, const TQString& s)
{
	if (i >= 0 && i < m_nlines)
		m_edits.at(i)->setText(s);
}

TQString KMWInfoBase::text(int i)
{
	if (i >= 0 && i < m_nlines)
		return m_edits.at(i)->text();
	return TQString::null;
}

void KMWInfoBase::setCurrent(int i)
{
	if (i >= 0 && i < m_nlines)
	{
		m_edits.at(i)->selectAll();
		m_edits.at(i)->setFocus();
	}
}

TQLineEdit* KMWInfoBase::lineEdit( int i )
{
	if ( i >= 0 && i < m_nlines )
		return m_edits.at( i );
	else
		return NULL;
}

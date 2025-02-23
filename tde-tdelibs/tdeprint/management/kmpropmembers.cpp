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

#include "kmpropmembers.h"
#include "kmprinter.h"
#include "kmwizard.h"

#include <tqtextview.h>
#include <tqlayout.h>
#include <tdelocale.h>

KMPropMembers::KMPropMembers(TQWidget *parent, const char *name)
: KMPropWidget(parent,name)
{
	m_members = new TQTextView(this);
	m_members->setPaper(colorGroup().background());
	m_members->setFrameStyle(TQFrame::NoFrame);

	TQVBoxLayout	*main_ = new TQVBoxLayout(this, 10, 0);
	main_->addWidget(m_members);

	m_pixmap = "tdeprint_printer_class";
	m_title = i18n("Members");
	m_header = i18n("Class Members");
}

KMPropMembers::~KMPropMembers()
{
}

void KMPropMembers::setPrinter(KMPrinter *p)
{
	if (p && ((p->isClass(false) && p->isLocal()) || p->isImplicit()))
	{
		TQStringList	l = p->members();
		TQString		txt("<ul>");
		for (TQStringList::ConstIterator it=l.begin(); it!=l.end(); ++it)
			txt.append("<li>" + (*it) + "</li>");
		txt.append("</ul>");
		m_members->setText(txt);
		emit enable(true);
		emit enableChange(!p->isImplicit());
	}
	else
	{
		emit enable(false);
		m_members->setText("");
	}
}

void KMPropMembers::configureWizard(KMWizard *w)
{
	w->configure(KMWizard::Class,KMWizard::Class,true);
}

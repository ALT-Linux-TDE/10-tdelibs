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

#ifndef KMWINFOBASE_H
#define KMWINFOBASE_H

#include "kmwizardpage.h"
#include <tqptrlist.h>

class TQLabel;
class TQLineEdit;

class TDEPRINT_EXPORT KMWInfoBase : public KMWizardPage
{
public:
	KMWInfoBase(int n = 1, TQWidget *parent = 0, const char *name = 0);

	void setInfo(const TQString&);
	void setLabel(int, const TQString&);
	void setText(int, const TQString&);
	void setCurrent(int);

	TQString text(int);

protected:
	TQLineEdit* lineEdit( int );

private:
	TQPtrList<TQLabel>		m_labels;
	TQPtrList<TQLineEdit>	m_edits;
	TQLabel			*m_info;
	int			m_nlines;
};

#endif

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

#ifndef KPHPGL2PAGE_H
#define KPHPGL2PAGE_H

#include "kprintdialogpage.h"

class KIntNumInput;
class TQCheckBox;

class KPHpgl2Page : public KPrintDialogPage
{
public:
	KPHpgl2Page(TQWidget *parent = 0, const char *name = 0);
	~KPHpgl2Page();

	void setOptions(const TQMap<TQString,TQString>& opts);
	void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);

private:
	KIntNumInput	*m_penwidth;
	TQCheckBox		*m_blackplot, *m_fitplot;
};

#endif

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

#ifndef KMWPASSWORD_H
#define KMWPASSWORD_H

#include "kmwizardpage.h"

class TQLineEdit;
class TQButtonGroup;

class KMWPassword : public KMWizardPage
{
public:
	KMWPassword(TQWidget *parent = 0, const char *name = 0);

	bool isValid(TQString&);
	void initPrinter( KMPrinter* );
	void updatePrinter(KMPrinter*);

private:
	TQLineEdit    *m_login;
	TQLineEdit    *m_password;
	TQButtonGroup *m_btngroup;
};

#endif

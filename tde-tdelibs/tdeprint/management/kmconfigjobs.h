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

#ifndef KMCONFIGJOBS_H
#define KMCONFIGJOBS_H

#include "kmconfigpage.h"

class KIntNumInput;

class KMConfigJobs : public KMConfigPage
{
	TQ_OBJECT
public:
	KMConfigJobs(TQWidget *parent = 0, const char *name = 0);

	void loadConfig(TDEConfig*);
	void saveConfig(TDEConfig*);

private:
	KIntNumInput	*m_limit;
};

#endif

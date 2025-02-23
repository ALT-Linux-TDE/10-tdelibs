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

#ifndef KPIPEPROCESS_H
#define KPIPEPROCESS_H

#include <tqfile.h>
#include <stdio.h>

#include <tdelibs_export.h>

class TDEPRINT_EXPORT KPipeProcess : public TQFile
{
public:
	KPipeProcess(const TQString& cmd = TQString::null, int mode = IO_ReadOnly);
	~KPipeProcess();

	bool open(const TQString& cmd, int mode = IO_ReadOnly);
	void close();

private:
	FILE	*m_pipe;
};

#endif

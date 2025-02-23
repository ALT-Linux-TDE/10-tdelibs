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

#ifndef KMVIRTUALMANAGER_H
#define KMVIRTUALMANAGER_H

#include <tqobject.h>
#include <tqptrlist.h>
#include <tqdatetime.h>

#include <tdelibs_export.h>

#include "kpreloadobject.h"

class KMPrinter;
class KMManager;
class TQWidget;

class TDEPRINT_EXPORT KMVirtualManager : public TQObject, public KPReloadObject
{
public:
	KMVirtualManager(KMManager *parent = 0, const char *name = 0);
	~KMVirtualManager();

        void refresh();
	void reset();
	void virtualList(TQPtrList<KMPrinter>& list, const TQString& prname);
	void triggerSave();

	KMPrinter* findPrinter(const TQString& name);
	KMPrinter* findInstance(KMPrinter *p, const TQString& name);
	void setDefault(KMPrinter *p, bool save = true);
	TQString defaultPrinterName();
	bool isDefault(KMPrinter *p, const TQString& name);
	bool testInstance(KMPrinter *p);

	void create(KMPrinter *p, const TQString& name);
	void remove(KMPrinter *p, const TQString& name);
	void copy(KMPrinter *p, const TQString& src, const TQString& name);
	void setAsDefault(KMPrinter *p, const TQString& name, TQWidget *parent = NULL);

protected:
	void loadFile(const TQString& filename);
	void saveFile(const TQString& filename);
	void addPrinter(KMPrinter *p);
	void checkPrinter(KMPrinter*);
	void reload();
	void configChanged();

private:
	KMManager		*m_manager;
	TQDateTime		m_checktime;
        TQString                 m_defaultprinter;
};

inline void KMVirtualManager::reset()
{ m_checktime = TQDateTime(); }

#endif

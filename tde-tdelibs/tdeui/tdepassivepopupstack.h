/* This file is part of the TDE libraries
 * Copyright (C) 2011 - 2015 Timothy Pearson <kb9vqf@pearsoncomputing.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.

 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef TDEPASSIVEPOPUPSTACK_H
#define TDEPASSIVEPOPUPSTACK_H

#include <tqwidget.h>
#include <tqcursor.h>

#include "tdeapplication.h"
#include "tdeglobal.h"
#include "tdeglobalsettings.h"
#include "kiconloader.h"
#include "kpassivepopup.h"

typedef TQMap<KPassivePopup*, TQString> TQStringPopupIDMap;

class TDEUI_EXPORT TDEPassivePopupStackContainer : public TQWidget
{
	TQ_OBJECT

public:
	TDEPassivePopupStackContainer(TQWidget *parent=0, const char *name=0);
	~TDEPassivePopupStackContainer();

	KPassivePopup* displayMessage(TQString title, TQString message, TQString icon, int x, int y, TQString id=TQString::null);
	KPassivePopup* displayMessage(TQString title, TQString message, TQPixmap icon, int x, int y, TQString id=TQString::null);
	void processEvents();

signals:
	void popupClicked(KPassivePopup*, TQPoint, TQString);

private slots:
	void popupClosed(KPassivePopup*);
	void popupClicked(TQPoint);
	void popupDestroyed(TQObject* object);

private:
	TQPtrList<KPassivePopup> mPopupList;
	long mTopOfStack;
	long mRightOfStack;
	TQStringPopupIDMap mPopupIDMap;
};

#endif /* TDEPASSIVEPOPUPSTACK_H */

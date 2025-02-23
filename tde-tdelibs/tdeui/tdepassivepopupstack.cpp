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

#include "tdepassivepopupstack.h"

TDEPassivePopupStackContainer::TDEPassivePopupStackContainer(TQWidget *parent, const char *name) : TQWidget(parent, name) {
	mPopupList.clear();

	// Determine bottom of desktop
	TQPoint cursorPos = TQCursor::pos();
	TQRect r = TDEGlobalSettings::desktopGeometry(cursorPos);
	mTopOfStack = r.height();
	mRightOfStack = r.width();
}

TDEPassivePopupStackContainer::~TDEPassivePopupStackContainer() {
	//
}

KPassivePopup* TDEPassivePopupStackContainer::displayMessage(TQString title, TQString message, TQString icon, int x, int y, TQString id) {
	TQPixmap px;
	TDEIconLoader* il = TDEGlobal::iconLoader();
	px = il->loadIcon(icon, TDEIcon::NoGroup);

	return displayMessage(title, message, px, x, y, id);
}

KPassivePopup* TDEPassivePopupStackContainer::displayMessage(TQString title, TQString message, TQPixmap icon, int x, int y, TQString id) {
	KPassivePopup *popup = new KPassivePopup(KPassivePopup::Boxed, this, "");
	popup->setAutoDelete(true);
	popup->setView(title, message, icon);
	popup->setTimeout(-1);
	TQPoint leftCorner(x, y);
	if (leftCorner.isNull()) {
		if (mPopupList.isEmpty()) {
			// Determine bottom of desktop
			TQPoint cursorPos = TQCursor::pos();
			TQRect r = TDEGlobalSettings::desktopGeometry(cursorPos);
			mTopOfStack = r.height();
			mRightOfStack = r.width();
		}
		TQSize popupSize = popup->sizeHint();
		mTopOfStack = mTopOfStack-popupSize.height();
		if (mTopOfStack < 0) mTopOfStack = 0;
		leftCorner.setX(mRightOfStack-popupSize.width());
		leftCorner.setY(mTopOfStack);
	}
	connect(popup, TQ_SIGNAL(hidden(KPassivePopup*)), this, TQ_SLOT(popupClosed(KPassivePopup*)));
	connect(popup, TQ_SIGNAL(clicked(TQPoint)), this, TQ_SLOT(popupClicked(TQPoint)));
	connect(popup, TQ_SIGNAL(destroyed(TQObject*)), this, TQ_SLOT(popupDestroyed(TQObject*)));
	mPopupList.append(popup);
	mPopupIDMap[popup] = id;
	popup->show(leftCorner);

	return popup;
}

void TDEPassivePopupStackContainer::processEvents() {
	tqApp->processEvents();
}

void TDEPassivePopupStackContainer::popupClosed(KPassivePopup* popup) {
	// Remove the popup from our list of popups
	mPopupList.remove(popup);

	if (mPopupList.isEmpty()) {
		// Determine bottom of desktop
		TQPoint cursorPos = TQCursor::pos();
		TQRect r = TDEGlobalSettings::desktopGeometry(cursorPos);
		mTopOfStack = r.height();
		mRightOfStack = r.width();
	}
}

void TDEPassivePopupStackContainer::popupClicked(TQPoint point) {
	KPassivePopup* popup = dynamic_cast<KPassivePopup*>(const_cast<TQObject*>(TQObject::sender()));
	if (popup) {
		emit(popupClicked(popup, point, mPopupIDMap[popup]));
	}
	else {
		emit(popupClicked(NULL, point, TQString::null));
	}
}

void TDEPassivePopupStackContainer::popupDestroyed(TQObject* object) {
	KPassivePopup* popup = static_cast<KPassivePopup*>(const_cast<TQObject*>(object));
	if (popup) {
		mPopupIDMap.remove(popup);
	}
}

#include "tdepassivepopupstack.moc"

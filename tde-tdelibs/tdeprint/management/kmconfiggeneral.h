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

#ifndef KMCONFIGGENERAL_H
#define KMCONFIGGENERAL_H

#include "kmconfigpage.h"

class KIntNumInput;
class KURLRequester;
class TQCheckBox;
class TQPushButton;
class KMConfigGeneral : public KMConfigPage
{
	TQ_OBJECT
public:
	KMConfigGeneral(TQWidget *parent = 0);

	void loadConfig(TDEConfig*);
	void saveConfig(TDEConfig*);

protected slots:
	void slotTestPagePreview();
        void testPageChanged(const TQString & );
	void setEnabledPreviewButton(bool b);
private:
	KIntNumInput	*m_timer;
	KURLRequester	*m_testpage;
	TQCheckBox	*m_defaulttestpage;
	TQPushButton	*m_preview;
	TQCheckBox	*m_statusmsg, *m_uselast;
};

#endif

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

#ifndef KPIMAGEPAGE_H
#define KPIMAGEPAGE_H

#include "kprintdialogpage.h"

class KIntNumInput;
class TQComboBox;
class TQButtonGroup;
class ImagePreview;
class ImagePosition;

class KPImagePage : public KPrintDialogPage
{
	TQ_OBJECT
public:
	KPImagePage(DrMain *driver = 0, TQWidget *parent = 0, const char *name = 0);
	~KPImagePage();

	void setOptions(const TQMap<TQString,TQString>& opts);
	void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);

protected slots:
	void slotSizeTypeChanged(int);
	void slotPositionChanged();
	void slotImageSettingsChanged();
	void slotDefaultClicked();

private:
	KIntNumInput	*m_brightness, *m_hue, *m_saturation, *m_gamma;
	TQComboBox		*m_sizetype;
	KIntNumInput	*m_size;
	TQButtonGroup	*m_vertgrp, *m_horizgrp;
	ImagePreview	*m_preview;
	ImagePosition	*m_position;
};

#endif

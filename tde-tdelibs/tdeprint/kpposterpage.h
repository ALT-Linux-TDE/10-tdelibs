/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <tdeprint@swing.be>
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

#ifndef KPPOSTERPAGE_H
#define KPPOSTERPAGE_H

#include "kprintdialogpage.h"

class TQComboBox;
class TQCheckBox;
class TQLabel;
class PosterPreview;
class KIntNumInput;
class TQPushButton;
class TQLineEdit;

class KPPosterPage : public KPrintDialogPage
{
	TQ_OBJECT
public:
	KPPosterPage( TQWidget *parent = 0, const char *name = 0 );
	~KPPosterPage();

	void setOptions( const TQMap<TQString,TQString>& opts );
	void getOptions( TQMap<TQString,TQString>& opts, bool incldef = false );
	bool isValid();

protected slots:
	void slotPosterSizeChanged( int );
	void slotMarginChanged( int );
	void slotLockToggled( bool );

private:
	TQComboBox *m_postersize;
	TQComboBox *m_printsize;
	PosterPreview *m_preview;
	TQCheckBox *m_postercheck;
	TQLabel *m_mediasize;
	KIntNumInput *m_cutmargin;
	TQPushButton *m_lockbtn;
	TQLineEdit *m_selection;
};

#endif /* KPPOSTERPAGE_H */

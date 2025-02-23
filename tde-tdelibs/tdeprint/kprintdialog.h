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

#ifndef KPRINTDIALOG_H
#define KPRINTDIALOG_H

#if !defined( _TDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a TDEPrint developer
#endif

#include <kdialog.h>
#include <tqptrlist.h>
#include <tdeprint/kpreloadobject.h>

class KPrinter;
class KPrintDialogPage;

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT KPrintDialog : public KDialog, public KPReloadObject
{
	TQ_OBJECT
public:
	KPrintDialog(TQWidget *parent = 0, const char *name = 0);
	~KPrintDialog();

	void setFlags(int f);
	void setDialogPages(TQPtrList<KPrintDialogPage> *pages);
	void enableDialogPage( int index, bool flag = true );
	KPrinter* printer() const;

	static KPrintDialog* printerDialog(KPrinter*, TQWidget*, const TQString& caption = TQString::null, bool forceExpand = false);

signals:
	void printRequested(KPrinter*);

protected slots:
	void slotPrinterSelected(int);
	void slotProperties();
	void slotSetDefault();
	void slotOptions();
	virtual void done(int);
	void slotWizard();
	void slotExtensionClicked();
	void slotToggleFilter(bool);
	void slotHelp();
	void slotOutputFileSelected(const TQString&);
	void slotUpdatePossible( bool );
	void slotOpenFileDialog();

protected:
	bool checkOutputFile();
	void enableSpecial(bool on);
	void enableOutputFile(bool on);
	void setOutputFileExtension(const TQString&);
	void reload();
	void configChanged();
	void expandDialog(bool on = true);
	void initialize( KPrinter* );
	void init();

protected:
	class KPrintDialogPrivate;
	KPrintDialogPrivate	*d;
};

#endif

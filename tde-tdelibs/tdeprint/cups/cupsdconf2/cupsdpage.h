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

#ifndef CUPSDPAGE_H
#define	CUPSDPAGE_H

#include <tqwidget.h>

struct CupsdConf;

class CupsdPage : public TQWidget
{
	TQ_OBJECT
public:
	CupsdPage(TQWidget *parent = 0, const char *name = 0);
	virtual ~CupsdPage();

	virtual bool loadConfig(CupsdConf *conf, TQString& msg) = 0;
	virtual bool saveConfig(CupsdConf *conf, TQString& msg) = 0;
        virtual void setInfos(CupsdConf*) {}
	
	TQString pageLabel() const	{ return label_; }
	TQString header() const		{ return header_; }
	TQString pixmap() const		{ return pixmap_; }

protected:
	void setPageLabel(const TQString& s)	{ label_ = s; }
	void setHeader(const TQString& s)	{ header_ = s; }
	void setPixmap(const TQString& s)	{ pixmap_ = s; }

protected:
	CupsdConf	*conf_;
	TQString		label_;
	TQString		header_;
	TQString		pixmap_;
};

#endif

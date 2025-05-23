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

#ifndef	MARGINPREVIEW_H
#define	MARGINPREVIEW_H

#include <tqwidget.h>

class MarginPreview : public TQWidget
{
	TQ_OBJECT

public:
	MarginPreview(TQWidget *parent = 0, const char *name = 0);
	~MarginPreview();
	// note : unit -> points (1/72th in)
	void setPageSize(float w, float h);
	void setMargins(float t, float b, float l, float r);
	void setNoPreview(bool on);
	void setSymetric(bool on);
	enum	StateType { Fixed = -1, None = 0, TMoving, BMoving, LMoving, RMoving };

public slots:
	void enableRubberBand(bool on);

signals:
	void marginChanged(int type, float value);

protected:
	void paintEvent(TQPaintEvent *);
	void resizeEvent(TQResizeEvent *);
	void mouseMoveEvent(TQMouseEvent *);
	void mousePressEvent(TQMouseEvent *);
	void mouseReleaseEvent(TQMouseEvent *);
	int locateMouse(const TQPoint& p);
	void drawTempLine(TQPainter*);

private:
	float	width_, height_;
	float	top_, bottom_, left_, right_;
	TQRect	box_, margbox_;
	float	zoom_;
	bool	nopreview_;
	int	state_;
	int	oldpos_;
	bool	symetric_;
};

#endif

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

#include "imageposition.h"

#include <tqpainter.h>
#include <kstandarddirs.h>

static void draw3DPage(TQPainter *p, TQRect r)
{
	// draw white page
	p->fillRect(r,TQt::white);
	// draw 3D border
	p->setPen(TQt::black);
	p->moveTo(r.left(),r.bottom());
	p->lineTo(r.right(),r.bottom());
	p->lineTo(r.right(),r.top());
	p->setPen(TQt::darkGray);
	p->lineTo(r.left(),r.top());
	p->lineTo(r.left(),r.bottom());
	p->setPen(TQt::gray);
	p->moveTo(r.left()+1,r.bottom()-1);
	p->lineTo(r.right()-1,r.bottom()-1);
	p->lineTo(r.right()-1,r.top()+1);
}

ImagePosition::ImagePosition(TQWidget *parent, const char *name)
	: TQWidget(parent,name)
{
	position_ = Center;
	setMinimumSize(sizeHint());
	setSizePolicy(TQSizePolicy(TQSizePolicy::MinimumExpanding, TQSizePolicy::MinimumExpanding));
	pix_.load(locate("data", "tdeprint/preview-mini.png"));
}

ImagePosition::~ImagePosition()
{
}

void ImagePosition::setPosition(const char *type)
{
	int	pos(Center);
	if (strcmp(type,"top-left") == 0) pos = TopLeft;
	else if (strcmp(type,"top") == 0) pos = Top;
	else if (strcmp(type,"top-right") == 0) pos = TopRight;
	else if (strcmp(type,"left") == 0) pos = Left;
	else if (strcmp(type,"center") == 0) pos = Center;
	else if (strcmp(type,"right") == 0) pos = Right;
	else if (strcmp(type,"bottom-left") == 0) pos = BottomLeft;
	else if (strcmp(type,"bottom") == 0) pos = Bottom;
	else if (strcmp(type,"bottom-right") == 0) pos = BottomRight;
	setPosition((PositionType)pos);
}

void ImagePosition::setPosition(PositionType type)
{
	if (position_ != type) {
		position_ = type;
		update();
	}
}

void ImagePosition::setPosition(int horiz, int vert)
{
	int	type = vert*3+horiz;
	setPosition((PositionType)type);
}

TQString ImagePosition::positionString() const
{
	switch (position_) {
	   case TopLeft: return "top-left";
	   case Top: return "top";
	   case TopRight: return "top-right";
	   case Left: return "left";
	   case Center: return "center";
	   case Right: return "right";
	   case BottomLeft: return "bottom-left";
	   case Bottom: return "bottom";
	   case BottomRight: return "bottom-right";
	}
	return "center";
}

void ImagePosition::paintEvent(TQPaintEvent*)
{
	int	horiz, vert, x, y;
	int	margin = 5;
	int	pw(width()), ph(height()), px(0), py(0);

	if (pw > ((ph * 3) / 4))
	{
		pw = (ph * 3) / 4;
		px = (width() - pw) / 2;
	}
	else
	{
		ph = (pw * 4) / 3;
		py = (height() - ph) / 2;
	}
	TQRect	page(px, py, pw, ph), img(0, 0, pix_.width(), pix_.height());

	// compute img position
	horiz = position_%3;
	vert = position_/3;
	switch (horiz) {
	   case 0: x = page.left()+margin; break;
	   default:
	   case 1: x = (page.left()+page.right()-img.width())/2; break;
	   case 2: x = page.right()-margin-img.width(); break;
	}
	switch (vert) {
	   case 0: y = page.top()+margin; break;
	   default:
	   case 1: y = (page.top()+page.bottom()-img.height())/2; break;
	   case 2: y = page.bottom()-margin-img.height(); break;
	}
	img.moveTopLeft(TQPoint(x,y));

	// draw page
	TQPainter	p(this);
	draw3DPage(&p,page);

	// draw img
	/*p.setPen(darkRed);
	p.drawRect(img);
	p.drawLine(img.topLeft(),img.bottomRight());
	p.drawLine(img.topRight(),img.bottomLeft());*/
	p.drawPixmap(x, y, pix_);

	p.end();
}

TQSize ImagePosition::sizeHint() const
{
	return TQSize(60, 80);
}

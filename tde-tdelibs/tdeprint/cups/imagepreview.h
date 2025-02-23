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

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H

#include <tqwidget.h>
#include <tqimage.h>

class ImagePreview : public TQWidget
{
public:
	ImagePreview(TQWidget *parent=0, const char *name=0);
	~ImagePreview();

	void setParameters(int brightness, int hue, int saturation, int gamma);
	void setImage(const TQImage& image);
	void setBlackAndWhite(bool on);
	TQSize minimumSizeHint() const;

private: // Private attributes
	int brightness_;
	int hue_;
	int saturation_;
	int gamma_;
	bool bw_;
	TQImage image_;

protected: // Protected methods
	void paintEvent(TQPaintEvent*);
};

#endif

/*
    Copyright (C) 2002 Nikolas Zimmermann <wildfox@kde.org>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KSVGIconEngine_H
#define KSVGIconEngine_H

#include <tdelibs_export.h>

class KSVGIconPainter;

class TDECORE_EXPORT KSVGIconEngine
{
public:
	KSVGIconEngine();
	~KSVGIconEngine();
	
	bool load(int width, int height, const TQString &path);

	KSVGIconPainter *painter();
	TQImage *image();

	double width();
	double height();

private:
	struct Private;
	Private *d;
};

#endif

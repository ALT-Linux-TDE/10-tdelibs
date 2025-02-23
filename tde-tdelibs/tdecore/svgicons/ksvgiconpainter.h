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
    aint with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KSVGIconPainter_H
#define KSVGIconPainter_H

#include <libart_lgpl/art_render.h>
#include <libart_lgpl/art_render_gradient.h>

class TQImage;
class TQColor;
class TQWMatrix;
class TQDomElement;
class TQPointArray;

class TDECORE_EXPORT KSVGIconPainter
{
public:
	KSVGIconPainter(int width, int height);
	~KSVGIconPainter();

	void setDrawWidth(int dwidth);
	void setDrawHeight(int dheight);

	TQImage *image();

	TQWMatrix *worldMatrix();

	void finish();

	void setUseFill(bool fill);
	void setUseStroke(bool stroke);

	void setStrokeWidth(double width);
	void setStrokeMiterLimit(const TQString &miter);
	void setCapStyle(const TQString &cap);
	void setJoinStyle(const TQString &join);
	void setStrokeColor(const TQString &stroke);
	void setFillColor(const TQString &fill);
	void setFillRule(const TQString &fillRule);
	void setOpacity(const TQString &opacity);
	void setFillOpacity(const TQString &fillOpacity);
	void setStrokeOpacity(const TQString &strokeOpacity);
	void setStrokeDashOffset(const TQString &dashOffset);
	void setStrokeDashArray(const TQString &dashes);

	void setWorldMatrix(TQWMatrix *worldMatrix);
	void setClippingRect(int x, int y, int w, int h);

	void drawRectangle(double x, double y, double w, double h, double rx, double ry);
	void drawEllipse(double cx, double cy, double rx, double ry);
	void drawLine(double x1, double y1, double x2, double y2);
	void drawPolyline(TQPointArray polyArray, int points = -1);
	void drawPolygon(TQPointArray polyArray);
	void drawPath(const TQString &data, bool fill);
	void drawImage(double x, double y, TQImage &image);

	TQColor parseColor(const TQString &param);
	TQ_UINT32 toArtColor(const TQColor &color);
	TQ_UINT32 parseOpacity(const TQString &data);

	double toPixel(const TQString &s, bool hmode);
	double dpi();

	ArtGradientLinear *linearGradient(const TQString &id);
	void addLinearGradient(const TQString &id, ArtGradientLinear *gradient);

	TQDomElement linearGradientElement(ArtGradientLinear *linear);
	void addLinearGradientElement(ArtGradientLinear *gradient, TQDomElement element);

	ArtGradientRadial *radialGradient(const TQString &id);
	void addRadialGradient(const TQString &id, ArtGradientRadial *gradient);

	TQDomElement radialGradientElement(ArtGradientRadial *radial);
	void addRadialGradientElement(ArtGradientRadial *gradient, TQDomElement element);

	TQWMatrix parseTransform(const TQString &transform);

private:
	struct Private;
	Private *d;
};

#endif

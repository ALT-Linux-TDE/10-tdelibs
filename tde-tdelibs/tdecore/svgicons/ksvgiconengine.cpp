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

#include <tqdom.h>
#include <tqfile.h>
#include <tqcolor.h>
#include <tqimage.h>
#include <tqwmatrix.h>
#include <tqregexp.h>

#include <kmdcodec.h>

#include <zlib.h>

#include "ksvgiconpainter.h"
#include "ksvgiconengine.h"

class KSVGIconEngineHelper
{
public:
	KSVGIconEngineHelper(KSVGIconEngine *engine)
	{
		m_engine = engine;
	}

	~KSVGIconEngineHelper()
	{
	}

	double toPixel(const TQString &s, bool hmode)
	{
		return m_engine->painter()->toPixel(s, hmode);
	}

	ArtGradientStop *parseGradientStops(TQDomElement element, int &offsets)
	{
		if (!element.hasChildNodes())
			return 0;

		TQValueList<ArtGradientStop> stopList;

		float oldOffset = -1, newOffset = -1;
		for(TQDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling())
		{
			TQDomElement element = node.toElement();

			oldOffset = newOffset;
			TQString temp = element.attribute("offset");

			if(temp.contains("%"))
			{
				temp = temp.left(temp.length() - 1);
				newOffset = temp.toFloat() / 100.0;
			}
			else
				newOffset = temp.toFloat();

			// Spec  skip double offset specifications
			if(oldOffset == newOffset)
				continue;

			offsets++;
			stopList.append(ArtGradientStop());

			ArtGradientStop &stop = stopList.last();

			stop.offset = newOffset;

			TQString parseOpacity;
			TQString parseColor;

			if(element.hasAttribute("stop-opacity"))
				parseOpacity = element.attribute("stop-opacity");

			if(element.hasAttribute("stop-color"))
				parseColor = element.attribute("stop-color");

			if(parseOpacity.isEmpty() || parseColor.isEmpty())
			{
				TQString style = element.attribute("style");

				TQStringList substyles = TQStringList::split(';', style);
				for(TQStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it)
				{
					TQStringList substyle = TQStringList::split(':', (*it));
					TQString command = substyle[0];
					TQString params = substyle[1];
					command = command.stripWhiteSpace();
					params = params.stripWhiteSpace();

					if(command == "stop-color")
					{
						parseColor = params;

						if(!parseOpacity.isEmpty())
							break;
					}
					else if(command == "stop-opacity")
					{
						parseOpacity = params;

						if(!parseColor.isEmpty())
							break;
					}
				}
			}

			// Parse color using KSVGIconPainter (which uses Qt)
			// Supports all svg-needed color formats
			TQColor qStopColor = m_engine->painter()->parseColor(parseColor);

			// Convert in a libart suitable form
			TQ_UINT32 stopColor = m_engine->painter()->toArtColor(qStopColor);

			int opacity = m_engine->painter()->parseOpacity(parseOpacity);

			TQ_UINT32 rgba = (stopColor << 8) | opacity;
			TQ_UINT32 r, g, b, a;

			// Convert from separated to premultiplied alpha
			a = rgba & 0xff;
			r = (rgba >> 24) * a + 0x80;
			r = (r + (r >> 8)) >> 8;
			g = ((rgba >> 16) & 0xff) * a + 0x80;
			g = (g + (g >> 8)) >> 8;
			b = ((rgba >> 8) & 0xff) * a + 0x80;
			b = (b + (b >> 8)) >> 8;

			stop.color[0] = ART_PIX_MAX_FROM_8(r);
			stop.color[1] = ART_PIX_MAX_FROM_8(g);
			stop.color[2] = ART_PIX_MAX_FROM_8(b);
			stop.color[3] = ART_PIX_MAX_FROM_8(a);
		}

		if (stopList.isEmpty())
			return 0;

		ArtGradientStop *stops = new ArtGradientStop[stopList.count()];

		TQValueList<ArtGradientStop>::iterator it = stopList.begin();
		TQValueList<ArtGradientStop>::iterator end = stopList.end();

		for (int i = 0; it != end; ++i, ++it)
			stops[i] = *it;

		return stops;
	}

	TQPointArray parsePoints(TQString points)
	{
		if(points.isEmpty())
			return TQPointArray();

		points = points.simplifyWhiteSpace();

		if(points.contains(",,") || points.contains(", ,"))
			return TQPointArray();

		points.replace(',', ' ');
		points.replace('\r', TQString());
		points.replace('\n', TQString());

		points = points.simplifyWhiteSpace();

		TQStringList pointList = TQStringList::split(' ', points);

		TQPointArray array(pointList.count() / 2);
		int i = 0;

		for(TQStringList::Iterator it = pointList.begin(); it != pointList.end(); it++)
		{
			float x = (*(it++)).toFloat();
			float y = (*(it)).toFloat();

			array.setPoint(i, static_cast<int>(x), static_cast<int>(y));
			i++;
		}

		return array;
	}

	void parseTransform(const TQString &transform)
	{
		// Combine new and old matrix
		TQWMatrix matrix = m_engine->painter()->parseTransform(transform);

		TQWMatrix *current = m_engine->painter()->worldMatrix();
		*current = matrix * *current;
	}

	void parseCommonAttributes(TQDomNode &node)
	{
		// Set important default attributes
		m_engine->painter()->setFillColor("black");
		m_engine->painter()->setStrokeColor("none");
		m_engine->painter()->setStrokeDashArray("");
		m_engine->painter()->setStrokeWidth(1);
		m_engine->painter()->setJoinStyle("");
		m_engine->painter()->setCapStyle("");
	//	m_engine->painter()->setFillOpacity(255, true);
	//	m_engine->painter()->setStrokeOpacity(255, true);

		// Collect parent node's attributes
		TQPtrList<TQDomNamedNodeMap> applyList;
		applyList.setAutoDelete(true);

		TQDomNode shape = node.parentNode();
		for(; !shape.isNull() ; shape = shape.parentNode())
			applyList.prepend(new TQDomNamedNodeMap(shape.attributes()));

		// Apply parent attributes
		for(TQDomNamedNodeMap *map = applyList.first(); map != 0; map = applyList.next())
		{
			TQDomNamedNodeMap attr = *map;

			for(unsigned int i = 0; i < attr.count(); i++)
			{
				TQString name, value;

				name = attr.item(i).nodeName().lower();
				value = attr.item(i).nodeValue();

				if(name == "transform")
					parseTransform(value);
				else if(name == "style")
					parseStyle(value);
				else
					parsePA(name, value);
			}
		}

		// Apply local attributes
		TQDomNamedNodeMap attr = node.attributes();

		for(unsigned int i = 0; i < attr.count(); i++)
		{
			TQDomNode current = attr.item(i);

			if(current.nodeName().lower() == "transform")
				parseTransform(current.nodeValue());
			else if(current.nodeName().lower() == "style")
				parseStyle(current.nodeValue());
			else
				parsePA(current.nodeName().lower(), current.nodeValue());
		}
	}

	bool handleTags(TQDomElement element, bool paint)
	{
		if(element.attribute("display") == "none")
			return false;
		if(element.tagName() == "linearGradient")
		{
			ArtGradientLinear *gradient = new ArtGradientLinear();

			int offsets = -1;
			gradient->stops = parseGradientStops(element, offsets);
			gradient->n_stops = offsets + 1;

			TQString spread = element.attribute("spreadMethod");
			if(spread == "repeat")
				gradient->spread = ART_GRADIENT_REPEAT;
			else if(spread == "reflect")
				gradient->spread = ART_GRADIENT_REFLECT;
			else
				gradient->spread = ART_GRADIENT_PAD;

			m_engine->painter()->addLinearGradient(element.attribute("id"), gradient);
			m_engine->painter()->addLinearGradientElement(gradient, element);
			return true;
		}
		else if(element.tagName() == "radialGradient")
		{
			ArtGradientRadial *gradient = new ArtGradientRadial();

			int offsets = -1;
			gradient->stops = parseGradientStops(element, offsets);
			gradient->n_stops = offsets + 1;

			m_engine->painter()->addRadialGradient(element.attribute("id"), gradient);
			m_engine->painter()->addRadialGradientElement(gradient, element);
			return true;
		}

		if(!paint)
			return true;

		// TODO: Default attribute values
		if(element.tagName() == "rect")
		{
			double x = toPixel(element.attribute("x"), true);
			double y = toPixel(element.attribute("y"), false);
			double w = toPixel(element.attribute("width"), true);
			double h = toPixel(element.attribute("height"), false);

			double rx = 0.0;
			double ry = 0.0;

			if(element.hasAttribute("rx"))
				rx = toPixel(element.attribute("rx"), true);

			if(element.hasAttribute("ry"))
				ry = toPixel(element.attribute("ry"), false);

			m_engine->painter()->drawRectangle(x, y, w, h, rx, ry);
		}
		else if(element.tagName() == "switch")
		{
			TQDomNode iterate = element.firstChild();

			while(!iterate.isNull())
			{
				// Reset matrix
				m_engine->painter()->setWorldMatrix(new TQWMatrix(m_initialMatrix));

				// Parse common attributes, style / transform
				parseCommonAttributes(iterate);

				if(handleTags(iterate.toElement(), true))
					return true;
				iterate = iterate.nextSibling();
			}
			return true;
		}
		else if(element.tagName() == "g" || element.tagName() == "defs")
		{
			TQDomNode iterate = element.firstChild();

			while(!iterate.isNull())
			{
				// Reset matrix
				m_engine->painter()->setWorldMatrix(new TQWMatrix(m_initialMatrix));

				// Parse common attributes, style / transform
				parseCommonAttributes(iterate);

				handleTags(iterate.toElement(), (element.tagName() == "defs") ? false : true);
				iterate = iterate.nextSibling();
			}
			return true;
		}
		else if(element.tagName() == "line")
		{
			double x1 = toPixel(element.attribute("x1"), true);
			double y1 = toPixel(element.attribute("y1"), false);
			double x2 = toPixel(element.attribute("x2"), true);
			double y2 = toPixel(element.attribute("y2"), false);

			m_engine->painter()->drawLine(x1, y1, x2, y2);
			return true;
		}
		else if(element.tagName() == "circle")
		{
			double cx = toPixel(element.attribute("cx"), true);
			double cy = toPixel(element.attribute("cy"), false);

			double r = toPixel(element.attribute("r"), true); // TODO: horiz correct?

			m_engine->painter()->drawEllipse(cx, cy, r, r);
			return true;
		}
		else if(element.tagName() == "ellipse")
		{
			double cx = toPixel(element.attribute("cx"), true);
			double cy = toPixel(element.attribute("cy"), false);

			double rx = toPixel(element.attribute("rx"), true);
			double ry = toPixel(element.attribute("ry"), false);

			m_engine->painter()->drawEllipse(cx, cy, rx, ry);
			return true;
		}
		else if(element.tagName() == "polyline")
		{
			TQPointArray polyline = parsePoints(element.attribute("points"));
			m_engine->painter()->drawPolyline(polyline);
			return true;
		}
		else if(element.tagName() == "polygon")
		{
			TQPointArray polygon = parsePoints(element.attribute("points"));
			m_engine->painter()->drawPolygon(polygon);
			return true;
		}
		else if(element.tagName() == "path")
		{
			bool filled = true;

			if(element.hasAttribute("fill") && element.attribute("fill").contains("none"))
				filled = false;

			if(element.attribute("style").contains("fill") && element.attribute("style").stripWhiteSpace().contains("fill:none"))
				filled = false;

			m_engine->painter()->drawPath(element.attribute("d"), filled);
			return true;
		}
		else if(element.tagName() == "image")
		{
			double x = toPixel(element.attribute("x"), true);
			double y = toPixel(element.attribute("y"), false);
			double w = toPixel(element.attribute("width"), true);
			double h = toPixel(element.attribute("height"), false);

			TQString href = element.attribute("xlink:href");

			TQImage image;
			if(href.startsWith("data:"))
			{
				// Get input
				TQCString input = TQString(href.remove(TQRegExp("^data:image/.*;base64,"))).utf8();

				// Decode into 'output'
				TQByteArray output;
				KCodecs::base64Decode(input, output);

				// Display
				image.loadFromData(output);
			}
			else
				image.load(href);

			if (!image.isNull())
			{
				// Scale, if needed
				if(image.width() != (int) w || image.height() != (int) h)
					image = image.smoothScale((int) w, (int) h, TQImage::ScaleFree);

				m_engine->painter()->drawImage(x, y, image);
			}

			return true;
		}
		return false;
	}

	void parseStyle(const TQString &style)
	{
		TQStringList substyles = TQStringList::split(';', style);
		for(TQStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it)
		{
			TQStringList substyle = TQStringList::split(':', (*it));
			TQString command = substyle[0];
			TQString params = substyle[1];
			command = command.stripWhiteSpace();
			params = params.stripWhiteSpace();

			parsePA(command, params);
		}
	}

	void parsePA(const TQString &command, const TQString &value)
	{
		if(command == "stroke-width") // TODO: horiz:false correct?
			m_engine->painter()->setStrokeWidth(toPixel(value, false));
		else if(command == "stroke-miterlimit")
			m_engine->painter()->setStrokeMiterLimit(value);
		else if(command == "stroke-linecap")
			m_engine->painter()->setCapStyle(value);
		else if(command == "stroke-linejoin")
			m_engine->painter()->setJoinStyle(value);
		else if(command == "stroke-dashoffset")
			m_engine->painter()->setStrokeDashOffset(value);
		else if(command == "stroke-dasharray" && value != "none")
			m_engine->painter()->setStrokeDashArray(value);
		else if(command == "stroke")
			m_engine->painter()->setStrokeColor(value);
		else if(command == "fill")
			m_engine->painter()->setFillColor(value);
		else if(command == "fill-rule")
			m_engine->painter()->setFillRule(value);
		else if(command == "fill-opacity" || command == "stroke-opacity" || command == "opacity")
		{
			if(command == "fill-opacity")
				m_engine->painter()->setFillOpacity(value);
			else if(command == "stroke-value")
				m_engine->painter()->setStrokeOpacity(value);
			else
			{
				m_engine->painter()->setOpacity(value);
				m_engine->painter()->setFillOpacity(value);
				m_engine->painter()->setStrokeOpacity(value);
			}
		}
	}

private:
	friend class KSVGIconEngine;

	KSVGIconEngine *m_engine;
	TQWMatrix m_initialMatrix;
};

struct KSVGIconEngine::Private
{
	KSVGIconPainter *painter;
	KSVGIconEngineHelper *helper;

	double width;
	double height;
};

KSVGIconEngine::KSVGIconEngine() : d(new Private())
{
	d->painter = 0;
	d->helper = new KSVGIconEngineHelper(this);

	d->width = 0.0;
	d->height = 0.0;
}

KSVGIconEngine::~KSVGIconEngine()
{
	if(d->painter)
		delete d->painter;

	delete d->helper;

	delete d;
}

bool KSVGIconEngine::load(int width, int height, const TQString &path)
{
	if(path.isNull()) return false;

	TQDomDocument svgDocument("svg");
	TQFile file(path);

	if(path.right(3).upper() == "SVG")
	{
		// Open SVG Icon
		if(!file.open(IO_ReadOnly))
			return false;

		svgDocument.setContent(&file);
	}
	else // SVGZ
	{
		gzFile svgz = gzopen(path.latin1(), "ro");
		if(!svgz)
			return false;

		TQString data;
		bool done = false;

		TQCString buffer(1024);
		int length = 0;

		while(!done)
		{
			int ret = gzread(svgz, buffer.data() + length, 1024);
			if(ret == 0)
				done = true;
			else if(ret == -1)
				return false;
			else {
				buffer.resize(buffer.size()+1024);
				length += ret;
			}
		}

		gzclose(svgz);

		svgDocument.setContent(buffer);
	}

	if(svgDocument.isNull())
		return false;

	// Check for root element
	TQDomNode rootNode = svgDocument.namedItem("svg");
	if(rootNode.isNull() || !rootNode.isElement())
		return false;

	// Detect width and height
	TQDomElement rootElement = rootNode.toElement();

	// Create icon painter
	d->painter = new KSVGIconPainter(width, height);

	d->width = width; // this sets default for no width -> 100% case
	if(rootElement.hasAttribute("width"))
		d->width = d->helper->toPixel(rootElement.attribute("width"), true);

	d->height = height; // this sets default for no height -> 100% case
	if(rootElement.hasAttribute("height"))
		d->height = d->helper->toPixel(rootElement.attribute("height"), false);

	// Create icon painter
	d->painter->setDrawWidth(static_cast<int>(d->width));
	d->painter->setDrawHeight(static_cast<int>(d->height));

	// Set viewport clipping rect
	d->painter->setClippingRect(0, 0, width, height);

	// Apply viewbox
	if(rootElement.hasAttribute("viewBox"))
	{
		TQStringList points = TQStringList::split(' ', rootElement.attribute("viewBox").simplifyWhiteSpace());

		float w = points[2].toFloat();
		float h = points[3].toFloat();

		double vratiow = width / w;
		double vratioh = height / h;

		d->width = w;
		d->height = h;

		d->painter->worldMatrix()->scale(vratiow, vratioh);
	}
	else
	{
		// Fit into 'width' and 'height'
		// FIXME: Use an aspect ratio
		double ratiow = width / d->width;
		double ratioh = height / d->height;

		d->painter->worldMatrix()->scale(ratiow, ratioh);
	}

	TQWMatrix initialMatrix = *d->painter->worldMatrix();
	d->helper->m_initialMatrix = initialMatrix;

	// Apply transform
	if(rootElement.hasAttribute("transform"))
		d->helper->parseTransform(rootElement.attribute("transform"));

	// Go through all elements
	TQDomNode svgNode = rootElement.firstChild();
	while(!svgNode.isNull())
	{
		TQDomElement svgChild = svgNode.toElement();
		if(!svgChild.isNull())
		{
			d->helper->parseCommonAttributes(svgNode);
			d->helper->handleTags(svgChild, true);
		}

		svgNode = svgNode.nextSibling();

		// Reset matrix
		d->painter->setWorldMatrix(new TQWMatrix(initialMatrix));
	}

	d->painter->finish();

	return true;
}

KSVGIconPainter *KSVGIconEngine::painter()
{
	return d->painter;
}

TQImage *KSVGIconEngine::image()
{
	return d->painter->image();
}

double KSVGIconEngine::width()
{
	return d->width;
}

double KSVGIconEngine::height()
{
	return d->height;
}

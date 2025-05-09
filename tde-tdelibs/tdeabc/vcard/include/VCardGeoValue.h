/*
    This file is part of libvcard.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef GEOVALUE_H
#define GEOVALUE_H

#include <VCardValue.h>

namespace VCARD
{

class KVCARD_EXPORT GeoValue : public Value
{
	public:
		GeoValue();
		GeoValue(const GeoValue&);
		GeoValue(const TQCString&);
		GeoValue & operator = (GeoValue&);
		GeoValue & operator = (const TQCString&);
		bool operator ==(GeoValue&);
		bool operator !=(GeoValue& x) {return !(*this==x);}
		bool operator ==(const TQCString& s) {GeoValue a(s);return(*this==a);} 
		bool operator != (const TQCString& s) {return !(*this == s);}

		virtual ~GeoValue();
		void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

		void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

		void _parse();
		void _assemble();
		const char * className() const { return "GeoValue"; }

    GeoValue *clone();

		void setLatitude( float lat ) { latitude_ = lat; assembled_ = false; }
		void setLongitude( float lon ) { longitude_ = lon; assembled_ = false; }

		float latitude() { parse(); return latitude_; }
		float longitude() { parse(); return longitude_; }

	private:
		float latitude_;
		float longitude_;
};

}

#endif

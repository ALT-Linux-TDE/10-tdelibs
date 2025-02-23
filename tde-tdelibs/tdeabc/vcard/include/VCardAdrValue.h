/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef  ADRVALUE_H
#define  ADRVALUE_H

#include <tqstrlist.h>
#include <VCardValue.h>

namespace VCARD
{

class KVCARD_EXPORT AdrValue : public Value
{
	public:
		AdrValue();
		AdrValue(const AdrValue&);
		AdrValue(const TQCString&);
		AdrValue & operator = (AdrValue&);
		AdrValue & operator = (const TQCString&);
		bool operator ==(AdrValue&);
		bool operator !=(AdrValue& x) {return !(*this==x);}
		bool operator ==(const TQCString& s) {AdrValue a(s);return(*this==a);} 
		bool operator != (const TQCString& s) {return !(*this == s);}

		virtual ~AdrValue();
		void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

		void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

		void _parse();
		void _assemble();
		const char * className() const { return "AdrValue"; }

		AdrValue *clone();

		void setPOBox(const TQCString & s)
		{ poBox_ = s; assembled_ = false; }

		void setExtAddress(const TQCString & s)
		{ extAddress_ = s; assembled_ = false; }

		void setStreet(const TQCString & s)
		{ street_ = s; assembled_ = false; }

		void setLocality(const TQCString & s)
		{ locality_ = s; assembled_ = false; }

		void setRegion(const TQCString & s)
		{ region_ = s; assembled_ = false; }

		void setPostCode(const TQCString & s)
		{ postCode_ = s; assembled_ = false; }

		void setCountryName(const TQCString & s)
		{ countryName_ = s; assembled_ = false; }

		TQCString poBox()	{ parse(); return poBox_;	}
		TQCString extAddress()	{ parse(); return extAddress_;	}
		TQCString street()	{ parse(); return street_;	}
		TQCString locality()	{ parse(); return locality_;	}
		TQCString region()	{ parse(); return region_;	}
		TQCString postCode()	{ parse(); return postCode_;	}
		TQCString countryName()	{ parse(); return countryName_;	}
	
	private:
		
		TQCString poBox_;
		TQCString extAddress_;
		TQCString street_;
		TQCString locality_;
		TQCString region_;
		TQCString postCode_;
		TQCString countryName_;
};

}

#endif


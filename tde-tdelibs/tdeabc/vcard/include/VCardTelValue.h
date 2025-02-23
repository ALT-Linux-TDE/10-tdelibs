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

#ifndef  TELVALUE_H
#define  TELVALUE_H

#include <tqcstring.h>

#include <VCardValue.h>

namespace VCARD
{

class KVCARD_EXPORT TelValue : public Value
{
	public:
		TelValue();
		TelValue(const TelValue&);
		TelValue(const TQCString&);
		TelValue & operator = (TelValue&);
		TelValue & operator = (const TQCString&);
		bool operator ==(TelValue&);
		bool operator !=(TelValue& x) {return !(*this==x);}
		bool operator ==(const TQCString& s) {TelValue a(s);return(*this==a);} 
		bool operator != (const TQCString& s) {return !(*this == s);}

		virtual ~TelValue();
		void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

		void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

		void _parse();
		void _assemble();
		const char * className() const { return "TelValue"; }
};

}

#endif

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

#ifndef  NVALUE_H
#define  NVALUE_H

#include <VCardValue.h>

namespace VCARD
{

class KVCARD_EXPORT NValue : public Value
{
	public:
		NValue();
		NValue(const NValue&);
		NValue(const TQCString&);
		NValue & operator = (NValue&);
		NValue & operator = (const TQCString&);
		bool operator ==(NValue&);
		bool operator !=(NValue& x) {return !(*this==x);}
		bool operator ==(const TQCString& s) {NValue a(s);return(*this==a);} 
		bool operator != (const TQCString& s) {return !(*this == s);}

		virtual ~NValue();
		void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

		void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

		void _parse();
		void _assemble();
		const char * className() const { return "NValue"; }

		NValue *clone();

		TQCString family()	{ parse(); return family_;	}
		TQCString given()	{ parse(); return given_;	}
		TQCString middle()	{ parse(); return middle_;	}
		TQCString prefix()	{ parse(); return prefix_;	}
		TQCString suffix()	{ parse(); return suffix_;	}

		void setFamily	(const TQCString & s) { family_	= s; assembled_ = false; }
		void setGiven	(const TQCString & s) { given_	= s; assembled_ = false; }
		void setMiddle	(const TQCString & s) { middle_	= s; assembled_ = false; }
		void setPrefix	(const TQCString & s) { prefix_	= s; assembled_ = false; }
		void setSuffix	(const TQCString & s) { suffix_	= s; assembled_ = false; }

	private:

		TQCString family_, given_, middle_, prefix_, suffix_;
};

}

#endif

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

#ifndef  TEXTBINVALUE_H
#define  TEXTBINVALUE_H

#include <tqcstring.h>

#include <VCardValue.h>

namespace VCARD
{

class KVCARD_EXPORT TextBinValue : public Value
{
	public:
		TextBinValue();
		TextBinValue(const TextBinValue&);
		TextBinValue(const TQCString&);
		TextBinValue & operator = (TextBinValue&);
		TextBinValue & operator = (const TQCString&);
		bool operator ==(TextBinValue&);
		bool operator !=(TextBinValue& x) {return !(*this==x);}
		bool operator ==(const TQCString& s) {TextBinValue a(s);return(*this==a);} 
		bool operator != (const TQCString& s) {return !(*this == s);}

		virtual ~TextBinValue();
		void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

		void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

		void _parse();
		void _assemble();
		const char * className() const { return "TextBinValue"; }

		TextBinValue *clone();

		bool isBinary() { parse(); return mIsBinary_; }
		TQByteArray data() { parse(); return mData_; }
		TQString url() { parse(); return mUrl_; }

		void setData( const TQByteArray &data )
		{
			mData_ = data;
			mIsBinary_ = true;
			assembled_ = false;
		}

		void setUrl( const TQString &url )
		{
			mUrl_ = url;
			mIsBinary_ = false;
			assembled_ = false;
		}

	private:
    int mIsBinary_;
    TQByteArray mData_;
    TQString mUrl_;
};

}

#endif

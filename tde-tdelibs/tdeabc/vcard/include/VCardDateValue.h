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

#ifndef  DATEVALUE_H
#define  DATEVALUE_H

#include <tqcstring.h>
#include <tqdatetime.h>

#include <VCardValue.h>

namespace VCARD
{

class KVCARD_EXPORT DateValue : public Value
{
	public:
		DateValue();
		DateValue(const DateValue&);
		DateValue(const TQCString&);
		DateValue & operator = (DateValue&);
		DateValue & operator = (const TQCString&);
		bool operator ==(DateValue&);
		bool operator !=(DateValue& x) {return !(*this==x);}
		bool operator ==(const TQCString& s) {DateValue a(s);return(*this==a);} 
		bool operator != (const TQCString& s) {return !(*this == s);}

		virtual ~DateValue();
		void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

		void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

		void _parse();
		void _assemble();
		const char * className() const { return "DateValue"; }

		DateValue(
				unsigned int	year,
				unsigned int	month,
				unsigned int	day,
				unsigned int	hour = 0,
				unsigned int	minute = 0,
				unsigned int	second = 0,
				double			secFrac = 0,
				bool			zonePositive = true,
				unsigned int	zoneHour = 0,
				unsigned int	zoneMinute = 0);

		DateValue(const TQDate &);
		DateValue(const TQDateTime &);

		DateValue *clone();

		bool hasTime();

		unsigned int	year();
		unsigned int	month();
		unsigned int	day();
		unsigned int	hour();
		unsigned int	minute();
		unsigned int	second();
		double			secondFraction();
		bool			zonePositive();
		unsigned int	zoneHour();
		unsigned int	zoneMinute();

		void setYear			(unsigned int);
		void setMonth			(unsigned int);
		void setDay				(unsigned int);
		void setHour			(unsigned int);
		void setMinute			(unsigned int);
		void setSecond			(unsigned int);
		void setSecondFraction	(double);
		void setZonePositive	(bool);
		void setZoneHour		(unsigned int);
		void setZoneMinute		(unsigned int);

		TQDate qdate();
		TQTime qtime();
		TQDateTime qdt();
	
	private:
		
		unsigned int	year_, month_, day_,
				hour_, minute_, second_,
				zoneHour_, zoneMinute_;
						
		double secFrac_;

		bool zonePositive_;
		
		bool hasTime_;
};

}

#endif

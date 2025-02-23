/**********************************************************************
**
**
** KIntValidator, KFloatValidator:
**   Copyright (C) 1999 Glen Parker <glenebob@nwlink.com>
** KDoubleValidator:
**   Copyright (c) 2002 Marc Mutz <mutz@kde.org>
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the Free
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
*****************************************************************************/

#include <tqwidget.h>
#include <tqstring.h>

#include "knumvalidator.h"
#include <tdelocale.h>
#include <tdeglobal.h>
#include <kdebug.h>

///////////////////////////////////////////////////////////////
//  Implementation of KIntValidator
//

KIntValidator::KIntValidator ( TQWidget * parent, int base, const char * name )
  : TQValidator(parent, name)
{
  _base = base;
  if (_base < 2) _base = 2;
  if (_base > 36) _base = 36;

  _min = _max = 0;
}

KIntValidator::KIntValidator ( int bottom, int top, TQWidget * parent, int base, const char * name )
  : TQValidator(parent, name)
{
  _base = base;
  if (_base > 36) _base = 36;

  _min = bottom;
  _max = top;
}

KIntValidator::~KIntValidator ()
{}

TQValidator::State KIntValidator::validate ( TQString &str, int & ) const
{
  bool ok;
  int  val = 0;
  TQString newStr;

  newStr = str.stripWhiteSpace();
  if (_base > 10)
    newStr = newStr.upper();

  if (newStr == TQString::fromLatin1("-")) // a special case
    if ((_min || _max) && _min >= 0)
      ok = false;
    else
      return TQValidator::Acceptable;
  else if (newStr.length())
    val = newStr.toInt(&ok, _base);
  else {
    val = 0;
    ok = true;
  }

  if (! ok)
    return TQValidator::Invalid;

  if ((! _min && ! _max) || (val >= _min && val <= _max))
    return TQValidator::Acceptable;

  if (_max && _min >= 0 && val < 0)
    return TQValidator::Invalid;

  return TQValidator::Valid;
}

void KIntValidator::fixup ( TQString &str ) const
{
  int                dummy;
  int                val;
  TQValidator::State  state;

  state = validate(str, dummy);

  if (state == TQValidator::Invalid || state == TQValidator::Acceptable)
    return;

  if (! _min && ! _max)
    return;

  val = str.toInt(0, _base);

  if (val < _min) val = _min;
  if (val > _max) val = _max;

  str.setNum(val, _base);
}

void KIntValidator::setRange ( int bottom, int top )
{
  _min = bottom;
  _max = top;

	if (_max < _min)
		_max = _min;
}

void KIntValidator::setBase ( int base )
{
  _base = base;
  if (_base < 2) _base = 2;
}

int KIntValidator::bottom () const
{
  return _min;
}

int KIntValidator::top () const
{
  return _max;
}

int KIntValidator::base () const
{
  return _base;
}


///////////////////////////////////////////////////////////////
//  Implementation of KFloatValidator
//

class KFloatValidatorPrivate
{
public:
    KFloatValidatorPrivate()
    {
    }
    ~KFloatValidatorPrivate()
    {
    }
    bool acceptLocalizedNumbers;
};


KFloatValidator::KFloatValidator ( TQWidget * parent, const char * name )
  : TQValidator(parent, name)
{
    d = new KFloatValidatorPrivate;
    d->acceptLocalizedNumbers=false;
    _min = _max = 0;
}

KFloatValidator::KFloatValidator ( double bottom, double top, TQWidget * parent, const char * name )
  : TQValidator(parent, name)
{
    d = new KFloatValidatorPrivate;
    d->acceptLocalizedNumbers=false;
    _min = bottom;
    _max = top;
}

KFloatValidator::KFloatValidator ( double bottom, double top, bool localeAware, TQWidget * parent, const char * name )
  : TQValidator(parent, name)
{
    d = new KFloatValidatorPrivate;
    d->acceptLocalizedNumbers = localeAware;
    _min = bottom;
    _max = top;
}

KFloatValidator::~KFloatValidator ()
{
     delete d;
}

void KFloatValidator::setAcceptLocalizedNumbers(bool _b)
{
    d->acceptLocalizedNumbers=_b;
}

bool KFloatValidator::acceptLocalizedNumbers() const
{
    return d->acceptLocalizedNumbers;
}

TQValidator::State KFloatValidator::validate ( TQString &str, int & ) const
{
  bool    ok;
  double  val = 0;
  TQString newStr;
  newStr = str.stripWhiteSpace();

  if (newStr == TQString::fromLatin1("-")) // a special case
    if ((_min || _max) && _min >= 0)
      ok = false;
    else
      return TQValidator::Acceptable;
  else if (newStr == TQString::fromLatin1(".") || (d->acceptLocalizedNumbers && newStr==TDEGlobal::locale()->decimalSymbol())) // another special case
    return TQValidator::Acceptable;
  else if (newStr.length())
  {
    val = newStr.toDouble(&ok);
    if(!ok && d->acceptLocalizedNumbers)
       val= TDEGlobal::locale()->readNumber(newStr,&ok);
  }
  else {
    val = 0;
    ok = true;
  }

  if (! ok)
    return TQValidator::Invalid;

  if (( !_min && !_max) || (val >= _min && val <= _max))
    return TQValidator::Acceptable;

  if (_max && _min >= 0 && val < 0)
    return TQValidator::Invalid;

  if ( (_min || _max) && (val < _min || val > _max))
    return TQValidator::Invalid;

  return TQValidator::Valid;
}

void KFloatValidator::fixup ( TQString &str ) const
{
  int                dummy;
  double             val;
  TQValidator::State  state;

  state = validate(str, dummy);

  if (state == TQValidator::Invalid || state == TQValidator::Acceptable)
    return;

  if (! _min && ! _max)
    return;

  val = str.toDouble();

  if (val < _min) val = _min;
  if (val > _max) val = _max;

  str.setNum(val);
}

void KFloatValidator::setRange ( double bottom, double top )
{
  _min = bottom;
  _max = top;

	if (_max < _min)
		_max = _min;
}

double KFloatValidator::bottom () const
{
  return _min;
}

double KFloatValidator::top () const
{
  return _max;
}




///////////////////////////////////////////////////////////////
//  Implementation of KDoubleValidator
//

class KDoubleValidator::Private {
public:
  Private( bool accept=true ) : acceptLocalizedNumbers( accept ) {}

  bool acceptLocalizedNumbers;
};

KDoubleValidator::KDoubleValidator( TQObject * parent, const char * name )
  : TQDoubleValidator( parent, name ), d( 0 )
{
  d = new Private();
}

KDoubleValidator::KDoubleValidator( double bottom, double top, int decimals,
				    TQObject * parent, const char * name )
  : TQDoubleValidator( bottom, top, decimals, parent, name ), d( 0 )
{
  d = new Private();
}

KDoubleValidator::~KDoubleValidator()
{
	delete d;
}

bool KDoubleValidator::acceptLocalizedNumbers() const {
  return d->acceptLocalizedNumbers;
}

void KDoubleValidator::setAcceptLocalizedNumbers( bool accept ) {
  d->acceptLocalizedNumbers = accept;
}

TQValidator::State KDoubleValidator::validate( TQString & input, int & p ) const {
  TQString s = input;
  if ( acceptLocalizedNumbers() ) {
    TDELocale * l = TDEGlobal::locale();
    // ok, we have to re-format the number to have:
    // 1. decimalSymbol == '.'
    // 2. negativeSign  == '-'
    // 3. positiveSign  == <empty>
    // 4. thousandsSeparator() == <empty> (we don't check that there
    //    are exactly three decimals between each separator):
    TQString d = l->decimalSymbol(),
            n = l->negativeSign(),
            p = l->positiveSign(),
            t = l->thousandsSeparator();
    // first, delete p's and t's:
    if ( !p.isEmpty() )
      for ( int idx = s.find( p ) ; idx >= 0 ; idx = s.find( p, idx ) )
	s.remove( idx, p.length() );


    if ( !t.isEmpty() )
      for ( int idx = s.find( t ) ; idx >= 0 ; idx = s.find( t, idx ) )
	s.remove( idx, t.length() );

    // then, replace the d's and n's
    if ( ( !n.isEmpty() && n.find('.') != -1 ) ||
	 ( !d.isEmpty() && d.find('-') != -1 ) ) {
      // make sure we don't replace something twice:
      kdWarning() << "KDoubleValidator: decimal symbol contains '-' or "
		     "negative sign contains '.' -> improve algorithm" << endl;
      return Invalid;
    }

    if ( !d.isEmpty() && d != "." )
      for ( int idx = s.find( d ) ; idx >= 0 ; idx = s.find( d, idx + 1 ) )
	s.replace( idx, d.length(), '.');

    if ( !n.isEmpty() && n != "-" )
      for ( int idx = s.find( n ) ; idx >= 0 ; idx = s.find( n, idx + 1 ) )
	s.replace( idx, n.length(), '-' );
  }

  return base::validate( s, p );
}

#include "knumvalidator.moc"

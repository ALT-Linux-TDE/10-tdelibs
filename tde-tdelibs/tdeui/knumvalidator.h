/**********************************************************************
**
** $Id$
**
** Copyright (C) 1999 Glen Parker <glenebob@nwlink.com>
** Copyright (C) 2002 Marc Mutz <mutz@kde.org>
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

#ifndef __KNUMVALIDATOR_H
#define __KNUMVALIDATOR_H

#include <tqvalidator.h>

#include <tdelibs_export.h>

class TQWidget;
class TQString;

/**
 * TQValidator for integers.

  This can be used by TQLineEdit or subclass to provide validated
  text entry.  Can be provided with a base value (default is 10), to allow
  the proper entry of hexadecimal, octal, or any other base numeric data.

  @author Glen Parker <glenebob@nwlink.com>
  @version 0.0.1
*/
class TDEUI_EXPORT KIntValidator : public TQValidator {

  public:
    /**
      Constuctor.  Also sets the base value.
    */
    KIntValidator ( TQWidget * parent, int base = 10, const char * name = 0 );
    /**
     * Constructor.  Also sets the minimum, maximum, and numeric base values.
     */
    KIntValidator ( int bottom, int top, TQWidget * parent, int base = 10, const char * name = 0 );
    /**
     * Destructs the validator.
     */
    virtual ~KIntValidator ();
    /**
     * Validates the text, and return the result.  Does not modify the parameters.
     */
    virtual State validate ( TQString &, int & ) const;
    /**
     * Fixes the text if possible, providing a valid string.  The parameter may be modified.
     */
    virtual void fixup ( TQString & ) const;
    /**
     * Sets the minimum and maximum values allowed.
     */
    virtual void setRange ( int bottom, int top );
    /**
     * Sets the numeric base value.
     */
    virtual void setBase ( int base );
    /**
     * Returns the current minimum value allowed.
     */
    virtual int bottom () const;
    /**
     * Returns the current maximum value allowed.
     */
    virtual int top () const;
    /**
     * Returns the current numeric base.
     */
    virtual int base () const;

  private:
    int _base;
    int _min;
    int _max;

};

class KFloatValidatorPrivate;

/**
 \brief TQValidator for floating point entry (Obsolete)

  @obsolete Use KDoubleValidator

  Extends the TQValidator class to properly validate double numeric data.
  This can be used by TQLineEdit or subclass to provide validated
  text entry.

  @author Glen Parker <glenebob@nwlink.com>
  @version 0.0.1
*/
class TDEUI_EXPORT KFloatValidator : public TQValidator {

  public:
    /**
     * Constructor.
     */
    KFloatValidator ( TQWidget * parent, const char * name = 0 );
    /**
     * Constructor.  Also sets the minimum and maximum values.
     */
    KFloatValidator ( double bottom, double top, TQWidget * parent, const char * name = 0 );
    /**
     * Constructor.  Sets the validator to be locale aware if @p localeAware is true.
     */
    KFloatValidator ( double bottom, double top, bool localeAware, TQWidget * parent, const char * name = 0 );
    /**
     * Destructs the validator.
     */
    virtual ~KFloatValidator ();
    /**
     * Validates the text, and return the result. Does not modify the parameters.
     */
    virtual State validate ( TQString &, int & ) const;
    /**
     * Fixes the text if possible, providing a valid string. The parameter may be modified.
     */
    virtual void fixup ( TQString & ) const;
    /**
     * Sets the minimum and maximum value allowed.
     */
    virtual void setRange ( double bottom, double top );
    /**
     * Returns the current minimum value allowed.
     */
    virtual double bottom () const;
    /**
     * Returns the current maximum value allowed.
     */
    virtual double top () const;
    /**
     * Sets the validator to be locale aware if @p is true. In this case, the
     * character TDELocale::decimalSymbol() from the global locale is recognized
     * as decimal separator.
     */
    void setAcceptLocalizedNumbers(bool b);
    /**
     * Returns true if the validator is locale aware.
     * @see setAcceptLocalizedNumbers().
     */
    bool acceptLocalizedNumbers() const;

 private:
    double _min;
    double _max;

    KFloatValidatorPrivate *d;
};

/**
   @short A locale-aware QDoubleValidator

   KDoubleValidator extends TQDoubleValidator to be
   locale-aware. That means that - subject to not being disabled -
   TDELocale::decimalSymbol(), TDELocale::thousandsSeparator()
   and TDELocale::positiveSign() and TDELocale::negativeSign()
   are respected.

   @author Marc Mutz <mutz@kde.org>
   @see KIntValidator
   @since 3.1
**/

class TDEUI_EXPORT KDoubleValidator : public TQDoubleValidator {
  TQ_OBJECT
  TQ_PROPERTY( bool acceptLocalizedNumbers READ acceptLocalizedNumbers WRITE setAcceptLocalizedNumbers )
public:
  /** Constuct a locale-aware KDoubleValidator with default range
      (whatever TQDoubleValidator uses for that) and parent @p
      parent */
  KDoubleValidator( TQObject * parent, const char * name=0 );
  /** Constuct a locale-aware KDoubleValidator for range [@p bottom,@p
      top] and a precision of @p decimals decimals after the decimal
      point.  */
  KDoubleValidator( double bottom, double top, int decimals,
		    TQObject * parent, const char * name=0 );
  /** Destructs the validator.
   */
  virtual ~KDoubleValidator();

  /** Overloaded for internal reasons. The API is not affected. */
  virtual TQValidator::State validate( TQString & input, int & pos ) const;

  /** @return whether localized numbers are accepted (default: true) */
  bool acceptLocalizedNumbers() const;
  /** Sets whether to accept localized numbers (default: true) */
  void setAcceptLocalizedNumbers( bool accept );

private:
  typedef TQDoubleValidator base;
  class Private;
  Private * d;
};

#endif

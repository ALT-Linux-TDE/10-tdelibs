/*
 *
 * $Id$
 *
 * Definition of KRestrictedLine
 *
 * Copyright (C) 1997 Michael Wiedmann, <mw@miwie.in-berlin.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef KRESTRICTEDLINE_H
#define KRESTRICTEDLINE_H

#include <klineedit.h>

/**
 * @short A line editor for restricted character sets.
 *
 * The KRestrictedLine widget is a variant of TQLineEdit which
 * accepts only a restricted set of characters as input.
 * All other characters will be discarded and the signal invalidChar()
 * will be emitted for each of them.
 *
 * Valid characters can be passed as a TQString to the constructor
 * or set afterwards via setValidChars().
 * The default key bindings of TQLineEdit are still in effect.
 *
 * @author Michael Wiedmann <mw@miwie.in-berlin.de>
 */
class TDEUI_EXPORT KRestrictedLine : public KLineEdit
{
  TQ_OBJECT
  TQ_PROPERTY( TQString validChars READ validChars WRITE setValidChars )

public:

  /**
   * Constructor: This contructor takes three - optional - arguments.
   *  The first two parameters are simply passed on to TQLineEdit.
   *  @param parent   pointer to the parent widget
   *  @param name     pointer to the name of this widget
   *  @param valid    pointer to set of valid characters
   */
  KRestrictedLine( TQWidget *parent=0, const char *name=0,
		   const TQString& valid = TQString::null);

  /**
   * Destructs the restricted line editor.
   */
  ~KRestrictedLine();

  /**
   * All characters in the string valid are treated as
   * acceptable characters.
   */
  void setValidChars(const TQString& valid);
  /**
   * @return the string of acceptable characters.
   */
  TQString validChars() const;

signals:

  /**
   * Emitted when an invalid character was typed.
   */
  void	invalidChar(int);

protected:
  void	keyPressEvent( TQKeyEvent *e );

private:
  /// TQString of valid characters for this line
  TQString	qsValidChars;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KRestrictedLinePrivate* d;
};

#endif // KRESTRICTEDLINE_H

/*
 *
 *
 * Implementation of KRestrictedLine
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

#include <tqkeycode.h>

#include "krestrictedline.h"

KRestrictedLine::KRestrictedLine( TQWidget *parent,
				  const char *name,
				  const TQString& valid )
  : KLineEdit( parent, name )
{
    qsValidChars = valid;
}

KRestrictedLine::~KRestrictedLine()
{
  ;
}


void KRestrictedLine::keyPressEvent( TQKeyEvent *e )
{
  // let TQLineEdit process "special" keys and return/enter
  // so that we still can use the default key binding
  if (e->key() == Key_Enter || e->key() == Key_Return || e->key() == Key_Delete || e->ascii() < 32)
    {
      TQLineEdit::keyPressEvent(e);
      return;
    }

  // do we have a list of valid chars &&
  // is the pressed key in the list of valid chars?
  if (!qsValidChars.isEmpty() && !qsValidChars.contains(e->ascii()))
    {
      // invalid char, emit signal and return
      emit (invalidChar(e->key()));
      return;
    }
  else
	// valid char: let TQLineEdit process this key as usual
	TQLineEdit::keyPressEvent(e);

  return;
}


void KRestrictedLine::setValidChars( const TQString& valid)
{
  qsValidChars = valid;
}

TQString KRestrictedLine::validChars() const
{
  return qsValidChars;
}

void KRestrictedLine::virtual_hook( int id, void* data )
{ KLineEdit::virtual_hook( id, data ); }

#include "krestrictedline.moc"

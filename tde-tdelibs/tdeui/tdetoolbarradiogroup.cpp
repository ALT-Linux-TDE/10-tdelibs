/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
			  (C) 1999 Chris Schlaeger (cs@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include "tdetoolbarradiogroup.h"
#include "tdetoolbar.h"
#include "tdetoolbarbutton.h"

/*************************************************************************
 *                          TDEToolBarRadioGroup                                  *
 *************************************************************************/


TDEToolBarRadioGroup::TDEToolBarRadioGroup (TDEToolBar *_parent, const char *_name)
: TQObject(_parent, _name)
{
  buttons = new TDEToolBarButtonList();
  tb = _parent;
  connect (tb, TQ_SIGNAL(toggled(int)), this, TQ_SLOT(slotToggled(int)));
}

TDEToolBarRadioGroup::~TDEToolBarRadioGroup()
{
  delete buttons;
}

void TDEToolBarRadioGroup::addButton (int id)
{
    TDEToolBarButton *b = tb->getButton( id );
    b->setRadio( true );
    buttons->insert( id, b );
}

void TDEToolBarRadioGroup::removeButton (int id)
{
  if (!buttons->find(id))
     return;
  buttons->find(id)->setRadio(false);
  buttons->remove(id);
}

void TDEToolBarRadioGroup::slotToggled(int id)
{
  if (buttons->find(id) && buttons->find(id)->isOn())
  {
    TQIntDictIterator<TDEToolBarButton> it(*buttons);
    while (it.current())
    {
      if (it.currentKey() != id)
        it.current()->on(false);
      ++it;
    }
  }
}

#include "tdetoolbarradiogroup.moc"


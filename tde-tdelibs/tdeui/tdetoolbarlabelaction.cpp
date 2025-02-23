/* This file is part of the KDE libraries
    Copyright (C) 2004 Felix Berger <felixberger@beldesign.de>
    
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

#include "tdetoolbarlabelaction.h"

#include <tqlabel.h>
#include <tqapplication.h>

class TDEToolBarLabelAction::TDEToolBarLabelActionPrivate
{
public:
  TDEToolBarLabelActionPrivate()
    : m_label(0)
  {
  }
  TQLabel* m_label;
};


TDEToolBarLabelAction::TDEToolBarLabelAction(const TQString &text,
					 const TDEShortcut &cut,
					 const TQObject *receiver, 
					 const char *slot,
					 TDEActionCollection *parent,
					 const char *name)
  : KWidgetAction(new TQLabel(text, 0, "tde toolbar widget"), text, cut,
		  receiver, slot, parent, name), 
    d(new TDEToolBarLabelActionPrivate)
{
  init();
}

TDEToolBarLabelAction::TDEToolBarLabelAction(TQWidget* buddy, 
					 const TQString &text,
					 const TDEShortcut &cut,
					 const TQObject *receiver, 
					 const char *slot,
 					 TDEActionCollection *parent, 
					 const char *name)
  : KWidgetAction(new TQLabel(buddy, text, 0, "tde toolbar widget"), text, 
		  cut, receiver, slot, parent, name),
    d(new TDEToolBarLabelActionPrivate)
{
  init();
}

TDEToolBarLabelAction::TDEToolBarLabelAction(TQLabel* label, 
					 const TDEShortcut &cut, 
					 const TQObject *receiver,
					 const char *slot,
					 TDEActionCollection* parent, 
					 const char *name)
  : KWidgetAction(label, label->text(), cut, receiver, slot, parent, name),
    d(new TDEToolBarLabelActionPrivate)
{
  Q_ASSERT(TQString::fromLatin1("tde toolbar widget") == label->name());
  init();
}

TDEToolBarLabelAction::~TDEToolBarLabelAction()
{
  delete d;
  d = 0;
}

void TDEToolBarLabelAction::init()
{
  d->m_label = static_cast<TQLabel*>(widget());
  /* these lines were copied from Konqueror's KonqDraggableLabel class in
     konq_misc.cpp */
  d->m_label->setBackgroundMode(TQt::PaletteButton);
  d->m_label->setAlignment((TQApplication::reverseLayout()
			 ? TQt::AlignRight : TQt::AlignLeft) |
 			TQt::AlignVCenter | TQt::ShowPrefix );
  d->m_label->adjustSize();
}

void TDEToolBarLabelAction::setText(const TQString& text)
{
  KWidgetAction::setText(text);
  d->m_label->setText(text);
}

void TDEToolBarLabelAction::setBuddy(TQWidget* buddy)
{
  d->m_label->setBuddy(buddy);
}

TQWidget* TDEToolBarLabelAction::buddy() const
{
  return d->m_label->buddy();
}

TQLabel* TDEToolBarLabelAction::label() const
{
  return d->m_label;
}

void TDEToolBarLabelAction::virtual_hook(int, void*)
{

}

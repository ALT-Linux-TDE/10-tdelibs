/* This file is part of the KDE libraries
    Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)
              (C) 1997,1998, 2000 Sven Radej (radej@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>
#include <kstatusbar.h>
#include <tdeconfig.h>
#include <tdeglobal.h>


KStatusBarLabel::KStatusBarLabel( const TQString& text, int _id,
                                 KStatusBar *parent, const char *name) :
  TQLabel( parent, name)
{
  id = _id;

  setText( text );

  // umm... Mosfet? Can you help here?

  // Warning: TQStatusBar draws shaded rectangle around every item - which
  // IMHO is stupid.
  // So NoFrame|Plain is the best you get. the problem is that only in case of
  // StyledPanel|Something you get TQFrame to call TQStyle::drawPanel().

  setLineWidth  (0);
  setFrameStyle (TQFrame::NoFrame);

  setAlignment( AlignHCenter | AlignVCenter | SingleLine );

  connect (this, TQ_SIGNAL(itemPressed(int)), parent, TQ_SIGNAL(pressed(int)));
  connect (this, TQ_SIGNAL(itemReleased(int)), parent, TQ_SIGNAL(released(int)));
}

void KStatusBarLabel::mousePressEvent (TQMouseEvent *)
{
  emit itemPressed (id);
}

void KStatusBarLabel::mouseReleaseEvent (TQMouseEvent *)
{
  emit itemReleased (id);
}

KStatusBar::KStatusBar( TQWidget *parent, const char *name )
  : TQStatusBar( parent, name )
{
  // make the size grip stuff configurable
  // ...but off by default (sven)
  TDEConfig *config = TDEGlobal::config();
  TQString group(config->group());
  config->setGroup(TQString::fromLatin1("StatusBar style"));
  bool grip_enabled = config->readBoolEntry(TQString::fromLatin1("SizeGripEnabled"), false);
  setSizeGripEnabled(grip_enabled);
  config->setGroup(group);
}

KStatusBar::~KStatusBar ()
{
}

void KStatusBar::insertItem( const TQString& text, int id, int stretch, bool permanent)
{
  if (items[id])
    kdDebug() << "KStatusBar::insertItem: item id " << id << " already exists." << endl;

  KStatusBarLabel *l = new KStatusBarLabel (text, id, this);
  l->setFixedHeight(fontMetrics().height()+2);
  items.insert(id, l);
  addWidget (l, stretch, permanent);
  l->show();
}

void KStatusBar::removeItem (int id)
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    removeWidget (l);
    items.remove(id);
    delete l;
  }
  else
    kdDebug() << "KStatusBar::removeItem: bad item id: " << id << endl;
}

bool KStatusBar::hasItem( int id ) const
{
  KStatusBarLabel *l = items[id];
  if (l)
    return true;
  else
    return false;
}

void KStatusBar::changeItem( const TQString& text, int id )
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    l->setText(text);
    if(l->minimumWidth () != l->maximumWidth ())
    {
      reformat();
    }
  }
  else
    kdDebug() << "KStatusBar::changeItem: bad item id: " << id << endl;
}

void KStatusBar::setItemAlignment (int id, int align)
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    l->setAlignment(align);
  }
  else
    kdDebug() << "KStatusBar::setItemAlignment: bad item id: " << id << endl;
}

void KStatusBar::setItemFixed(int id, int w)
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    if (w==-1)
      w=fontMetrics().boundingRect(l->text()).width()+3;

    l->setFixedWidth(w);
  }
  else
    kdDebug() << "KStatusBar::setItemFixed: bad item id: " << id << endl;
}

#include "kstatusbar.moc"

//Eh!!!
//Eh what ? :)


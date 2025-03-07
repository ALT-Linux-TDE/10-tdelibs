/*****************************************************************

Copyright (c) 2000 Matthias Elter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <tqptrlist.h>

#include "kpanelapplet.h"
#include "kpanelapplet.moc"
#include <tdeapplication.h>
#include <tdeconfig.h>

class KPanelApplet::KPanelAppletPrivate
{
public:
  KPanelAppletPrivate()
    : customMenu(0),
      hasFocus(false)
      {}

  const TQPopupMenu* customMenu;
  TDESharedConfig::Ptr sharedConfig;
  TQPtrList<TQObject> watchedForFocus;
  bool hasFocus;
};

KPanelApplet::KPanelApplet(const TQString& configFile, Type type,
                           int actions, TQWidget *parent, const char *name, WFlags f)
  : TQFrame(parent, name, f)
  , _type(type)
  , _position( pBottom )
  , _alignment( LeftTop )
  , _config(0)
  , _actions(actions)
  , d(new KPanelApplet::KPanelAppletPrivate())
{
  setFrameStyle(NoFrame);
  TQPalette pal(palette());
  if(pal.active().mid() != pal.inactive().mid()){
    pal.setInactive(pal.active());
    setPalette(pal);
  }
  setBackgroundOrigin( AncestorOrigin );

  d->sharedConfig = TDESharedConfig::openConfig(configFile, kapp && kapp->config()->isImmutable());
  _config = d->sharedConfig;
}

KPanelApplet::~KPanelApplet()
{
  d->watchedForFocus.clear();
  needsFocus(false);
  delete d;
}

void KPanelApplet::setPosition( Position p )
{
  if( _position == p ) return;
  _position = p;
  positionChange( p );
}

void KPanelApplet::setAlignment( Alignment a )
{
  if( _alignment == a ) return;
  _alignment = a;
  alignmentChange( a );
}

// FIXME: Remove implementation for KDE 4
void KPanelApplet::positionChange( Position )
{
  orientationChange( orientation() );
  TQResizeEvent e( size(), size() );
  resizeEvent( &e );
  popupDirectionChange( popupDirection() );
}

TQt::Orientation KPanelApplet::orientation() const
{
  if( _position == pTop || _position == pBottom ) {
    return TQt::Horizontal;
  } else {
    return TQt::Vertical;
  }
}

// FIXME: Remove for KDE 4
KPanelApplet::Direction KPanelApplet::popupDirection()
{
    switch( _position ) {
    case pTop:     return Down;
    case pRight:   return Left;
    case pLeft:    return Right;
    default:
    case pBottom:  return Up;
    }
}

void KPanelApplet::action( Action a )
{
    if ( (a & About) )
	about();
    if ( (a & Help) )
	help();
    if ( (a & Preferences) )
	preferences();
    if ( (a & ReportBug) )
    reportBug();
}

const TQPopupMenu* KPanelApplet::customMenu() const
{
    return d->customMenu;
}

void KPanelApplet::setCustomMenu(const TQPopupMenu* menu)
{
    d->customMenu = menu;
}

void KPanelApplet::watchForFocus(TQWidget* widget, bool watch)
{
    if (!widget)
    {
        return;
    }

    if (watch)
    {
        if (d->watchedForFocus.find(widget) == -1)
        {
            d->watchedForFocus.append(widget);
            widget->installEventFilter(this);
        }
    }
    else if (d->watchedForFocus.find(widget) != -1)
    {
        d->watchedForFocus.remove(widget);
        widget->removeEventFilter(this);
    }
}

void KPanelApplet::needsFocus(bool focus)
{
    if (focus == d->hasFocus)
    {
        return;
    }

    d->hasFocus = focus;
    emit requestFocus(focus);
}

bool KPanelApplet::eventFilter(TQObject *o, TQEvent * e)
{
    if (d->watchedForFocus.find(o) != -1)
    {
        if (e->type() == TQEvent::MouseButtonRelease ||
            e->type() == TQEvent::FocusIn)
        {
            needsFocus(true);
        }
        else if (e->type() == TQEvent::FocusOut)
        {
            needsFocus(false);
        }
    }

    return TQFrame::eventFilter(o, e);
}

TDESharedConfig::Ptr KPanelApplet::sharedConfig() const
{
    return d->sharedConfig;
}

void KPanelApplet::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


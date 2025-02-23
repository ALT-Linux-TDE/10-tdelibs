/* This file is part of the KDE libraries
   Copyright (C) 1998 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2000 Peter Putzer <putzer@kde.org>
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

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

#include <tqcolor.h>
#include <tqtimer.h>
#include <tqtooltip.h>
#include <tqpixmap.h>
#include <tqpainter.h>
#include <tqstyle.h>
#include <tqapplication.h>

#include <kcursor.h>
#include <tdeglobalsettings.h>

#include "kurllabel.h"

class KURLLabel::Private
{
public:
  Private (const TQString& url, KURLLabel* label)
    : URL (url),
      LinkColor (TDEGlobalSettings::linkColor()),
      HighlightedLinkColor (TQt::red),
      Tip(url),
      Cursor (0L),
      Underline (true),
      UseTips (false),
      Glow (true),
      Float (false),
      RealUnderline (true),
      MousePressed(false),
      WasInsideRect(false),
      MarginAltered(false),
      Timer (new TQTimer (label))
  {
    connect (Timer, TQ_SIGNAL (timeout ()), label, TQ_SLOT (updateColor ()));
  }

  ~Private ()
  {
      delete Cursor;
  }

  TQString URL;
  TQPixmap AltPixmap;

  TQColor LinkColor;
  TQColor HighlightedLinkColor;

  TQString Tip;
  TQCursor* Cursor;
  bool Underline:1;
  bool UseTips:1;
  bool Glow:1;
  bool Float:1;
  bool RealUnderline:1;
  bool MousePressed:1;
  bool WasInsideRect:1;
  bool MarginAltered:1;
  TQPixmap RealPixmap;

  TQTimer* Timer;
};

KURLLabel::KURLLabel (const TQString& url, const TQString& text,
                        TQWidget* parent, const char* name)
  : TQLabel (!text.isNull() ? text : url, parent, name),
    d (new Private (url, this))
{
  setFont (font());
  setUseCursor (true);
  setLinkColor (d->LinkColor);
  setFocusPolicy( TQWidget::StrongFocus ); //better accessibility
  setMouseTracking (true);
}

KURLLabel::KURLLabel (TQWidget* parent, const char* name)
  : TQLabel (parent, name),
    d (new Private (TQString::null, this))
{
  setFont (font());
  setUseCursor (true);
  setLinkColor (d->LinkColor);
  setFocusPolicy( TQWidget::StrongFocus ); //better accessibility
  setMouseTracking (true);
}

KURLLabel::~KURLLabel ()
{
  delete d;
}

void KURLLabel::mouseReleaseEvent (TQMouseEvent* e)
{
  TQLabel::mouseReleaseEvent (e);
  if (!d->MousePressed)
    return;
  d->MousePressed = false;
  TQRect r( activeRect() );
  if (!r.contains(e->pos()))
    return;

  setLinkColor (d->HighlightedLinkColor);
  d->Timer->start (300);

  switch (e->button())
    {
    case TQt::LeftButton:
      emit leftClickedURL ();
      emit leftClickedURL (d->URL);
      break;

    case TQt::MidButton:
      emit middleClickedURL ();
      emit middleClickedURL (d->URL);
      break;

    case TQt::RightButton:
      emit rightClickedURL ();
      emit rightClickedURL (d->URL);
      break;

    default:
      ; // nothing
    }
}

void KURLLabel::setFont (const TQFont& f)
{
  TQFont newFont = f;
  newFont.setUnderline (d->Underline);

  TQLabel::setFont (newFont);
}

void KURLLabel::setUnderline (bool on)
{
  d->Underline = on;

  setFont (font());
}

void KURLLabel::updateColor ()
{
  d->Timer->stop();

  TQRect r( activeRect() );
  if (!(d->Glow || d->Float) || !r.contains (mapFromGlobal(TQCursor::pos())))
    setLinkColor (d->LinkColor);
}

void KURLLabel::setLinkColor (const TQColor& col)
{
  TQPalette p = palette();
  p.setColor (TQColorGroup::Foreground, col);
  setPalette (p);

  update();
}

void KURLLabel::setURL (const TQString& url)
{
  if ( d->Tip == d->URL ) { // update the tip as well
    d->Tip = url;
    setUseTips( d->UseTips );
  }

  d->URL = url;
}

const TQString& KURLLabel::url () const
{
  return d->URL;
}

void KURLLabel::unsetCursor ()
{
	delete d->Cursor;
	d->Cursor = 0;
}

void KURLLabel::setCursor ( const TQCursor& cursor )
{
	delete d->Cursor;
	d->Cursor = new TQCursor( cursor );
}

void KURLLabel::setUseCursor (bool on, TQCursor* cursor)
{
  if (on)
    {
      if (cursor)
        KURLLabel::setCursor (*cursor);
      else
        KURLLabel::setCursor (KCursor::handCursor());
    }
  else
    KURLLabel::unsetCursor ();
}

bool KURLLabel::useCursor () const
{
  return d->Cursor;
}

void KURLLabel::setUseTips (bool on)
{
  d->UseTips = on;

  if (on) {
    TQToolTip::add (this, activeRect(), d->Tip);
  } else
    TQToolTip::remove (this);
}

void KURLLabel::setTipText (const TQString& tip)
{
  d->Tip = tip;

  setUseTips (d->UseTips);
}

bool KURLLabel::useTips () const
{
  return d->UseTips;
}

const TQString& KURLLabel::tipText () const
{
  return d->Tip;
}

void KURLLabel::setHighlightedColor (const TQColor& highcolor)
{
  d->LinkColor = highcolor;

  if (!d->Timer->isActive())
    setLinkColor (highcolor);
}

void KURLLabel::setHighlightedColor (const TQString& highcolor)
{
  setHighlightedColor (TQColor (highcolor));
}

void KURLLabel::setSelectedColor (const TQColor& selcolor)
{
  d->HighlightedLinkColor = selcolor;

  if (d->Timer->isActive())
    setLinkColor (selcolor);
}

void KURLLabel::setSelectedColor (const TQString& selcolor)
{
  setSelectedColor (TQColor (selcolor));
}

void KURLLabel::setGlow (bool glow)
{
  d->Glow = glow;
}

void KURLLabel::setFloat (bool do_float)
{
  d->Float = do_float;
}

bool KURLLabel::isGlowEnabled () const
{
  return d->Glow;
}

bool KURLLabel::isFloatEnabled () const
{
  return d->Float;
}

void KURLLabel::setAltPixmap (const TQPixmap& altPix)
{
  d->AltPixmap = altPix;
}

const TQPixmap* KURLLabel::altPixmap () const
{
  return &d->AltPixmap;
}

void KURLLabel::enterEvent (TQEvent* e)
{
  TQLabel::enterEvent (e);

  TQRect r( activeRect() );
  if (!r.contains( static_cast<TQMouseEvent*>(e)->pos() ))
    return;

  if (!d->AltPixmap.isNull() && pixmap())
    {
      d->RealPixmap = *pixmap();
      setPixmap (d->AltPixmap);
    }

  if (d->Glow || d->Float)
    {
      d->Timer->stop();

      setLinkColor (d->HighlightedLinkColor);

      d->RealUnderline = d->Underline;

      if (d->Float)
        setUnderline (true);
    }

  emit enteredURL ();
  emit enteredURL (d->URL);
}

void KURLLabel::leaveEvent (TQEvent* e)
{
  TQLabel::leaveEvent (e);

  if (!d->AltPixmap.isNull() && pixmap())
    setPixmap (d->RealPixmap);

  if ((d->Glow || d->Float) && !d->Timer->isActive())
    setLinkColor (d->LinkColor);

  setUnderline (d->RealUnderline);

  emit leftURL ();
  emit leftURL (d->URL);
}

bool KURLLabel::event (TQEvent *e)
{
  if (e && e->type() == TQEvent::ParentPaletteChange)
  {
    // use parentWidget() unless you are a toplevel widget, then try qAapp
    TQPalette p = parentWidget() ? parentWidget()->palette() : tqApp->palette();
    p.setBrush(TQColorGroup::Base, p.brush(TQPalette::Normal, TQColorGroup::Background));
    p.setColor(TQColorGroup::Foreground, palette().active().foreground());
    setPalette(p);
    d->LinkColor = TDEGlobalSettings::linkColor();
    setLinkColor(d->LinkColor);
    return true;
  }
  else if (e->type() == TQEvent::Paint) {
    const bool result = TQLabel::event(e);
    if (result && hasFocus()) {
        TQPainter p(this);
        TQRect r( activeRect() );
        style().drawPrimitive( TQStyle::PE_FocusRect, &p, r, colorGroup() );
    }
    return result;
  }
  else if (e->type() == TQEvent::KeyPress) {
    TQKeyEvent* ke = static_cast<TQKeyEvent*>(e);
    if (ke->key() == TQt::Key_Enter || ke->key() == TQt::Key_Return) {
      setLinkColor (d->HighlightedLinkColor);
      d->Timer->start (300);
      emit leftClickedURL ();
      emit leftClickedURL (d->URL);
      ke->accept();
      return true;
    }
  }
  else if (e->type() == TQEvent::MouseButtonPress) {
    TQRect r( activeRect() );
    d->MousePressed = r.contains(static_cast<TQMouseEvent*>(e)->pos());
  }
  else if (e->type() == TQEvent::MouseMove) {
    if (d->Cursor) {
      TQRect r( activeRect() );
      bool inside = r.contains(static_cast<TQMouseEvent*>(e)->pos());
      if (d->WasInsideRect != inside) {
        if (inside)
          TQLabel::setCursor(*d->Cursor);
        else
          TQLabel::unsetCursor();
        d->WasInsideRect = inside;
      }
    }
  }
  return TQLabel::event(e);
}

TQRect KURLLabel::activeRect() const
{
  TQRect r( contentsRect() );
  if (text().isEmpty() || (!d->MarginAltered && sizePolicy() == TQSizePolicy(TQSizePolicy::Fixed, TQSizePolicy::Fixed)))
      return r; //fixed size is sometimes used with pixmap
  int hAlign = TQApplication::horizontalAlignment( alignment() );
  int indentX = (hAlign && indent()>0) ? indent() : 0;
  TQFontMetrics fm(font());
  r.setWidth( TQMIN(fm.width(text()), r.width()));
  if ( hAlign & AlignLeft )
      r.moveLeft(r.left() + indentX);
  if ( hAlign & AlignCenter )
      r.moveLeft((contentsRect().width()-r.width())/2+margin());
  if ( hAlign & AlignRight )
      r.moveLeft(contentsRect().width()-r.width()-indentX+margin());
  int add = TQMIN(3, margin());
  r = TQRect(r.left()-add, r.top()-add, r.width()+2*add, r.height()+2*add);
  return r;
}

void KURLLabel::setMargin( int margin )
{
  TQLabel::setMargin(margin);
  d->MarginAltered = true;
}

void KURLLabel::setFocusPolicy( TQWidget::FocusPolicy policy )
{
  TQLabel::setFocusPolicy(policy);
  if (!d->MarginAltered) {
      TQLabel::setMargin(policy == TQWidget::NoFocus ? 0 : 3); //better default : better look when focused
  }
}

void KURLLabel::setSizePolicy ( TQSizePolicy policy )
{
  TQLabel::setSizePolicy(policy);
  if (!d->MarginAltered && policy.horData()==TQSizePolicy::Fixed && policy.verData()==TQSizePolicy::Fixed) {
      TQLabel::setMargin(0); //better default : better look when fixed size
  }
}

void KURLLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kurllabel.moc"

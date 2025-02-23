/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
              (C) 1999 Chris Schlaeger (cs@kde.org)
              (C) 1999 Kurt Granroth (granroth@kde.org)

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
#include <string.h>

#include "tdetoolbarbutton.h"
#include "tdetoolbar.h"

#include <tqstyle.h>
#include <tqimage.h>
#include <tqtimer.h>
#include <tqdrawutil.h>
#include <tqtooltip.h>
#include <tqbitmap.h>
#include <tqpopupmenu.h>
#include <tqcursor.h>
#include <tqpainter.h>
#include <tqlayout.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kiconeffect.h>
#include <kiconloader.h>

// needed to get our instance
#include <tdemainwindow.h>

template class TQIntDict<TDEToolBarButton>;

class TDEToolBarButtonPrivate
{
public:
  TDEToolBarButtonPrivate();
  ~TDEToolBarButtonPrivate();

  int     m_id;
  bool    m_buttonDown;
  bool    m_noStyle;
  bool    m_isSeparator;
  bool    m_isRadio;
  bool    m_highlight;
  bool    m_isRaised;
  bool    m_isActive;

  TQString m_iconName;

  TDEToolBar *m_parent;
  TDEToolBar::IconText m_iconText;
  int m_iconSize;
  TQSize size;

  TQPoint m_mousePressPos;

  TDEInstance  *m_instance;
};

TDEToolBarButtonPrivate::TDEToolBarButtonPrivate()
{
    m_buttonDown  = false;

    m_noStyle     = false;
    m_isSeparator = false;
    m_isRadio     = false;
    m_highlight   = false;
    m_isRaised    = false;
    m_isActive    = false;

    m_iconName    = TQString::null;
    m_iconText    = TDEToolBar::IconOnly;
    m_iconSize    = 0;

    m_parent   = 0;
    m_instance = TDEGlobal::instance();
}

TDEToolBarButtonPrivate::~TDEToolBarButtonPrivate()
{
    //
}

// This will construct a separator
TDEToolBarButton::TDEToolBarButton( TQWidget *_parent, const char *_name )
  : TQToolButton( _parent , _name)
{
  d = new TDEToolBarButtonPrivate();

  resize(6,6);
  hide();
  d->m_isSeparator = true;
}

TDEToolBarButton::TDEToolBarButton( const TQString& _icon, int _id,
                                TQWidget *_parent, const char *_name,
                                const TQString &_txt, TDEInstance *_instance )
    : TQToolButton( _parent, _name ), d( 0 )
{
  d = new TDEToolBarButtonPrivate();

  d->m_id     = _id;
  TQToolButton::setTextLabel(_txt);
  d->m_instance = _instance;

  d->m_parent = dynamic_cast<TDEToolBar*>(_parent);
  if (d->m_parent) {
    connect(d->m_parent, TQ_SIGNAL( modechange() ),
            this,         TQ_SLOT( modeChange() ));
  }

  setFocusPolicy( TQWidget::NoFocus );

  // connect all of our slots and start trapping events
  connect(this, TQ_SIGNAL( clicked() ),
          this, TQ_SLOT( slotClicked() ) );
  connect(this, TQ_SIGNAL( pressed() ),
          this, TQ_SLOT( slotPressed() ) );
  connect(this, TQ_SIGNAL( released() ),
          this, TQ_SLOT( slotReleased() ) );
  installEventFilter(this);

  d->m_iconName = _icon;

  // do our initial setup
  modeChange();
}

TDEToolBarButton::TDEToolBarButton( const TQPixmap& pixmap, int _id,
                                TQWidget *_parent, const char *name,
                                const TQString& txt)
    : TQToolButton( _parent, name ), d( 0 )
{
  d = new TDEToolBarButtonPrivate();

  d->m_id       = _id;
  TQToolButton::setTextLabel(txt);

  d->m_parent = dynamic_cast<TDEToolBar*>(_parent);
  if (d->m_parent) {
    connect(d->m_parent, TQ_SIGNAL( modechange() ),
            this,         TQ_SLOT( modeChange() ));
  }

  setFocusPolicy( TQWidget::NoFocus );

  // connect all of our slots and start trapping events
  connect(this, TQ_SIGNAL( clicked() ),
          this, TQ_SLOT( slotClicked() ));
  connect(this, TQ_SIGNAL( pressed() ),
          this, TQ_SLOT( slotPressed() ));
  connect(this, TQ_SIGNAL( released() ),
          this, TQ_SLOT( slotReleased() ));
  installEventFilter(this);

  // set our pixmap and do our initial setup
  setIconSet( TQIconSet( pixmap ));
  modeChange();
}

TDEToolBarButton::~TDEToolBarButton()
{
  delete d; d = 0;
}

void TDEToolBarButton::modeChange()
{
  TQSize mysize;

  // grab a few global variables for use in this function and others
  if (d->m_parent) {
    d->m_highlight = d->m_parent->highlight();
    d->m_iconText  = d->m_parent->iconText();

    d->m_iconSize = d->m_parent->iconSize();
  }
  if (!d->m_iconName.isNull())
    setIcon(d->m_iconName);

  // we'll start with the size of our pixmap
  int pix_width  = d->m_iconSize;
  if ( d->m_iconSize == 0 ) {
      if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
          pix_width = IconSize( TDEIcon::MainToolbar );
      else
          pix_width = IconSize( TDEIcon::Toolbar );
  }
  int pix_height = pix_width;

  int text_height = 0;
  int text_width = 0;

  TQToolTip::remove(this);
  if (d->m_iconText != TDEToolBar::IconOnly)
  {
    // okay, we have to deal with fonts.  let's get our information now
    TQFont tmp_font = TDEGlobalSettings::toolBarFont();

    // now parse out our font sizes from our chosen font
    TQFontMetrics fm(tmp_font);

    text_height = fm.lineSpacing();
    text_width  = fm.width(textLabel());

    // none of the other modes want tooltips
  }
  else
  {
    TQToolTip::add(this, textLabel());
  }

  switch (d->m_iconText)
  {
  case TDEToolBar::IconOnly:
    mysize = TQSize(pix_width, pix_height);
    break;

  case TDEToolBar::IconTextRight:
    mysize = TQSize(pix_width + text_width + 4, pix_height);
    break;

  case TDEToolBar::TextOnly:
    mysize = TQSize(text_width + 4, text_height);
    break;

  case TDEToolBar::IconTextBottom:
    mysize = TQSize((text_width + 4 > pix_width) ? text_width + 4 : pix_width, pix_height + text_height);
    break;

  default:
    break;
  }

  mysize = style().sizeFromContents(TQStyle::CT_ToolButton, this, mysize).
               expandedTo(TQApplication::globalStrut());

  // make sure that this isn't taller then it is wide
  if (mysize.height() > mysize.width())
    mysize.setWidth(mysize.height());

  d->size = mysize;
  updateGeometry();
}

void TDEToolBarButton::setTextLabel( const TQString& text, bool tipToo)
{
  if (text.isNull())
    return;

  TQString txt(text);
  if (txt.endsWith(TQString::fromLatin1("...")))
    txt.truncate(txt.length() - 3);

  TQToolButton::setTextLabel(txt, tipToo);
  update();
}

void TDEToolBarButton::setText( const TQString& text)
{
  setTextLabel(text, true);
  modeChange();
}

void TDEToolBarButton::setIcon( const TQString &icon )
{
  d->m_iconName = icon;
  if (d->m_parent)
    d->m_iconSize = d->m_parent->iconSize();
  // TQObject::name() return "const char *" instead of TQString.
  if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
    TQToolButton::setIconSet( d->m_instance->iconLoader()->loadIconSet(
        d->m_iconName, TDEIcon::MainToolbar, d->m_iconSize ));
  else
    TQToolButton::setIconSet( d->m_instance->iconLoader()->loadIconSet(
        d->m_iconName, TDEIcon::Toolbar, d->m_iconSize ));
}

void TDEToolBarButton::setIconSet( const TQIconSet &iconset )
{
  TQToolButton::setIconSet( iconset );
}

// remove?
void TDEToolBarButton::setPixmap( const TQPixmap &pixmap )
{
  if( pixmap.isNull()) // called by TQToolButton
  {
    TQToolButton::setPixmap( pixmap );
    return;
  }
  TQIconSet set = iconSet();
  set.setPixmap( pixmap, TQIconSet::Automatic, TQIconSet::Active );
  TQToolButton::setIconSet( set );
}

void TDEToolBarButton::setDefaultPixmap( const TQPixmap &pixmap )
{
  TQIconSet set = iconSet();
  set.setPixmap( pixmap, TQIconSet::Automatic, TQIconSet::Normal );
  TQToolButton::setIconSet( set );
}

void TDEToolBarButton::setDisabledPixmap( const TQPixmap &pixmap )
{
  TQIconSet set = iconSet();
  set.setPixmap( pixmap, TQIconSet::Automatic, TQIconSet::Disabled );
  TQToolButton::setIconSet( set );
}

void TDEToolBarButton::setDefaultIcon( const TQString& icon )
{
  TQIconSet set = iconSet();
  TQPixmap pm;
  if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
    pm = d->m_instance->iconLoader()->loadIcon( icon, TDEIcon::MainToolbar,
        d->m_iconSize );
  else
    pm = d->m_instance->iconLoader()->loadIcon( icon, TDEIcon::Toolbar,
        d->m_iconSize );
  set.setPixmap( pm, TQIconSet::Automatic, TQIconSet::Normal );
  TQToolButton::setIconSet( set );
}

void TDEToolBarButton::setDisabledIcon( const TQString& icon )
{
  TQIconSet set = iconSet();
  TQPixmap pm;
  if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
    pm = d->m_instance->iconLoader()->loadIcon( icon, TDEIcon::MainToolbar,
        d->m_iconSize );
  else
    pm = d->m_instance->iconLoader()->loadIcon( icon, TDEIcon::Toolbar,
        d->m_iconSize );
  set.setPixmap( pm, TQIconSet::Automatic, TQIconSet::Disabled );
  TQToolButton::setIconSet( set );
}

TQPopupMenu *TDEToolBarButton::popup()
{
  // obsolete
  // KDE4: remove me
  return TQToolButton::popup();
}

void TDEToolBarButton::setPopup(TQPopupMenu *p, bool)
{
  TQToolButton::setPopup(p);
  TQToolButton::setPopupDelay(-1);
}


void TDEToolBarButton::setDelayedPopup (TQPopupMenu *p, bool)
{
  TQToolButton::setPopup(p);
  TQToolButton::setPopupDelay(TQApplication::startDragTime());
}

void TDEToolBarButton::leaveEvent(TQEvent *)
{
  if( d->m_isRaised || d->m_isActive )
  {
    d->m_isRaised = false;
    d->m_isActive = false;
    repaint(false);
  }

  emit highlighted(d->m_id, false);
}

void TDEToolBarButton::enterEvent(TQEvent *)
{
  if (d->m_highlight)
  {
    if (isEnabled())
    {
      d->m_isActive = true;
      if (!isToggleButton())
        d->m_isRaised = true;
    }
    else
    {
      d->m_isRaised = false;
      d->m_isActive = false;
    }

    repaint(false);
  }
  emit highlighted(d->m_id, true);
}

bool TDEToolBarButton::eventFilter(TQObject *o, TQEvent *ev)
{
  if (o == this)
  {

    // Popup the menu when the left mousebutton is pressed and the mouse
    // is moved by a small distance.
    if (TQToolButton::popup())
    {
      if (ev->type() == TQEvent::MouseButtonPress)
      {
        TQMouseEvent* mev = static_cast<TQMouseEvent*>(ev);
        d->m_mousePressPos = mev->pos();
      }
      else if (ev->type() == TQEvent::MouseMove)
      {
        TQMouseEvent* mev = static_cast<TQMouseEvent*>(ev);
        if ((mev->pos() - d->m_mousePressPos).manhattanLength()
              > TDEGlobalSettings::dndEventDelay())
        {
          openPopup();
          return true;
        }
      }
    }

    if (d->m_isRadio &&
	(ev->type() == TQEvent::MouseButtonPress ||
         ev->type() == TQEvent::MouseButtonRelease ||
         ev->type() == TQEvent::MouseButtonDblClick) && isOn())
      return true;

    // From Kai-Uwe Sattler <kus@iti.CS.Uni-Magdeburg.De>
    if (ev->type() == TQEvent::MouseButtonDblClick)
    {
      emit doubleClicked(d->m_id);
      return false;
    }
  }

  return TQToolButton::eventFilter(o, ev);
}

void TDEToolBarButton::mousePressEvent( TQMouseEvent * e )
{
  d->m_buttonDown = true;

  if ( e->button() == TQt::MidButton )
  {
    // Get TQToolButton to show the button being down while pressed
    TQMouseEvent ev( TQEvent::MouseButtonPress, e->pos(), e->globalPos(), TQt::LeftButton, e->state() );
    TQToolButton::mousePressEvent(&ev);
    return;
  }
  TQToolButton::mousePressEvent(e);
}

void TDEToolBarButton::mouseReleaseEvent( TQMouseEvent * e )
{
  TQt::ButtonState state = TQt::ButtonState(e->button() | (e->state() & KeyButtonMask));
  if ( e->button() == TQt::MidButton )
  {
    TQMouseEvent ev( TQEvent::MouseButtonRelease, e->pos(), e->globalPos(), TQt::LeftButton, e->state() );
    TQToolButton::mouseReleaseEvent(&ev);
  }
  else
    TQToolButton::mouseReleaseEvent(e);

  if ( !d->m_buttonDown )
    return;
  d->m_buttonDown = false;

  if ( hitButton( e->pos() ) )
    emit buttonClicked( d->m_id, state );
}

void TDEToolBarButton::drawButton( TQPainter *_painter )
{
  TQStyle::SFlags flags   = TQStyle::Style_Default;
  TQStyle::SCFlags active = TQStyle::SC_None;

  if (isDown()) {
    flags  |= TQStyle::Style_Down;
    active |= TQStyle::SC_ToolButton;
  }
  if (isEnabled()) 	flags |= TQStyle::Style_Enabled;
  if (isOn()) 		flags |= TQStyle::Style_On;
  if (isEnabled() && hasMouse())	flags |= TQStyle::Style_Raised;
  if (hasFocus())	flags |= TQStyle::Style_HasFocus;

  // Draw a styled toolbutton
  style().drawComplexControl(TQStyle::CC_ToolButton, _painter, this, rect(),
	colorGroup(), flags, TQStyle::SC_ToolButton, active, TQStyleOption());

  int dx, dy;
  TQFont tmp_font(TDEGlobalSettings::toolBarFont());
  TQFontMetrics fm(tmp_font);
  TQRect textRect;
  int textFlags = 0;

  if (d->m_iconText == TDEToolBar::IconOnly) // icon only
  {
    TQPixmap pixmap = iconSet().pixmap( TQIconSet::Automatic,
        isEnabled() ? (d->m_isActive ? TQIconSet::Active : TQIconSet::Normal) :
               TQIconSet::Disabled,
        isOn() ? TQIconSet::On : TQIconSet::Off );
    if( !pixmap.isNull())
    {
      dx = ( width() - pixmap.width() ) / 2;
      dy = ( height() - pixmap.height() ) / 2;
      if ( isDown() && style().styleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, pixmap );
    }
  }
  else if (d->m_iconText == TDEToolBar::IconTextRight) // icon and text (if any)
  {
    TQPixmap pixmap = iconSet().pixmap( TQIconSet::Automatic,
        isEnabled() ? (d->m_isActive ? TQIconSet::Active : TQIconSet::Normal) :
            	TQIconSet::Disabled,
        isOn() ? TQIconSet::On : TQIconSet::Off );
    if( !pixmap.isNull())
    {
      dx = 4;
      dy = ( height() - pixmap.height() ) / 2;
      if ( isDown() && style().styleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, pixmap );
    }

    if (!textLabel().isNull())
    {
      textFlags = AlignVCenter|AlignLeft;
      if (!pixmap.isNull())
        dx = 4 + pixmap.width() + 2;
      else
        dx = 4;
      dy = 0;
      if ( isDown() && style().styleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      textRect = TQRect(dx, dy, width()-dx, height());
    }
  }
  else if (d->m_iconText == TDEToolBar::TextOnly)
  {
    if (!textLabel().isNull())
    {
      textFlags = AlignVCenter|AlignLeft;
      dx = (width() - fm.width(textLabel())) / 2;
      dy = (height() - fm.lineSpacing()) / 2;
      if ( isDown() && style().styleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      textRect = TQRect( dx, dy, fm.width(textLabel()), fm.lineSpacing() );
    }
  }
  else if (d->m_iconText == TDEToolBar::IconTextBottom)
  {
    TQPixmap pixmap = iconSet().pixmap( TQIconSet::Automatic,
        isEnabled() ? (d->m_isActive ? TQIconSet::Active : TQIconSet::Normal) :
            	TQIconSet::Disabled,
        isOn() ? TQIconSet::On : TQIconSet::Off );
    if( !pixmap.isNull())
    {
      dx = (width() - pixmap.width()) / 2;
      dy = (height() - fm.lineSpacing() - pixmap.height()) / 2;
      if ( isDown() && style().styleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, pixmap );
    }

    if (!textLabel().isNull())
    {
      textFlags = AlignBottom|AlignHCenter;
      dx = (width() - fm.width(textLabel())) / 2;
      dy = height() - fm.lineSpacing() - 4;

      if ( isDown() && style().styleHint(TQStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      textRect = TQRect( dx, dy, fm.width(textLabel()), fm.lineSpacing() );
    }
  }

  // Draw the text at the position given by textRect, and using textFlags
  if (!textLabel().isNull() && !textRect.isNull())
  {
      _painter->setFont(TDEGlobalSettings::toolBarFont());
      if (!isEnabled())
        _painter->setPen(palette().disabled().dark());
      else if(d->m_isRaised)
        _painter->setPen(TDEGlobalSettings::toolBarHighlightColor());
      else
	_painter->setPen( colorGroup().buttonText() );
      _painter->drawText(textRect, textFlags, textLabel());
  }

  if (TQToolButton::popup())
  {
    TQStyle::SFlags arrowFlags = TQStyle::Style_Default;

    if (isDown())	arrowFlags |= TQStyle::Style_Down;
    if (isEnabled()) 	arrowFlags |= TQStyle::Style_Enabled;

      style().drawPrimitive(TQStyle::PE_ArrowDown, _painter,
          TQRect(width()-7, height()-7, 7, 7), colorGroup(),
	  arrowFlags, TQStyleOption() );
  }
}

void TDEToolBarButton::paletteChange(const TQPalette &)
{
  if(!d->m_isSeparator)
  {
    modeChange();
    repaint(false); // no need to delete it first therefore only false
  }
}

bool TDEToolBarButton::event(TQEvent *e)
{
  if (e->type() == TQEvent::ParentFontChange || e->type() == TQEvent::ApplicationFontChange)
  {
     //If we use toolbar text, apply the settings again, to relayout...
     if (d->m_iconText != TDEToolBar::IconOnly)
       modeChange();
     return true;
  }

  return TQToolButton::event(e);
}


void TDEToolBarButton::showMenu()
{
  // obsolete
  // KDE4: remove me
}

void TDEToolBarButton::slotDelayTimeout()
{
  // obsolete
  // KDE4: remove me
}

void TDEToolBarButton::slotClicked()
{
  emit clicked( d->m_id );

  // emit buttonClicked when the button was clicked while being in an extension popupmenu
  if ( d->m_parent && !d->m_parent->rect().contains( geometry().center() ) ) {
    ButtonState state = TDEApplication::keyboardMouseState();
    if ( ( state & TQt::MouseButtonMask ) == TQt::NoButton )
      state = ButtonState( TQt::LeftButton | state );
    emit buttonClicked( d->m_id, state ); // Doesn't work with MidButton
  }
}

void TDEToolBarButton::slotPressed()
{
  emit pressed( d->m_id );
}

void TDEToolBarButton::slotReleased()
{
  emit released( d->m_id );
}

void TDEToolBarButton::slotToggled()
{
  emit toggled( d->m_id );
}

void TDEToolBarButton::setNoStyle(bool no_style)
{
    d->m_noStyle = no_style;

    modeChange();
    d->m_iconText = TDEToolBar::IconTextRight;
    repaint(false);
}

void TDEToolBarButton::setRadio (bool f)
{
    if ( d )
	d->m_isRadio = f;
}

void TDEToolBarButton::on(bool flag)
{
  if(isToggleButton())
    setOn(flag);
  else
  {
    setDown(flag);
    leaveEvent((TQEvent *) 0);
  }
  repaint();
}

void TDEToolBarButton::toggle()
{
  setOn(!isOn());
  repaint();
}

void TDEToolBarButton::setToggle(bool flag)
{
  setToggleButton(flag);
  if (flag)
    connect(this, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotToggled()));
  else
    disconnect(this, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotToggled()));
}

TQSize TDEToolBarButton::sizeHint() const
{
   return d->size;
}

TQSize TDEToolBarButton::minimumSizeHint() const
{
   return d->size;
}

TQSize TDEToolBarButton::minimumSize() const
{
   return d->size;
}

bool TDEToolBarButton::isRaised() const
{
    return d->m_isRaised;
}

bool TDEToolBarButton::isActive() const
{
    return d->m_isActive;
}

int TDEToolBarButton::iconTextMode() const
{
    return static_cast<int>( d->m_iconText );
}

int TDEToolBarButton::id() const
{
    return d->m_id;
}

// TDEToolBarButtonList
TDEToolBarButtonList::TDEToolBarButtonList()
{
   setAutoDelete(false);
}

void TDEToolBarButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "tdetoolbarbutton.moc"

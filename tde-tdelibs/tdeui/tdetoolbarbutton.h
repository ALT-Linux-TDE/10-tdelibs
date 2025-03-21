/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
              (C) 2000 Kurt Granroth (granroth@kde.org)

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

#ifndef _TDETOOLBARBUTTON_H
#define _TDETOOLBARBUTTON_H

#include <tqpixmap.h>
#include <tqtoolbutton.h>
#include <tqintdict.h>
#include <tqstring.h>
#include <tdeglobal.h>

class TDEToolBar;
class TDEToolBarButtonPrivate;
class TDEInstance;
class TQEvent;
class TQPopupMenu;
class TQPainter;

/**
 * A toolbar button. This is used internally by TDEToolBar, use the
 * TDEToolBar methods instead.
 * @internal
 */
class TDEUI_EXPORT TDEToolBarButton : public TQToolButton
{
  TQ_OBJECT
  

public:
  /**
   * Construct a button with an icon loaded by the button itself.
   * This will trust the button to load the correct icon with the
   * correct size.
   *
   * @param icon   Name of icon to load (may be absolute or relative)
   * @param id     Id of this button
   * @param parent This button's parent
   * @param name   This button's internal name
   * @param txt    This button's text (in a tooltip or otherwise)
   * @param _instance the instance to use for this button
   */
  TDEToolBarButton(const TQString& icon, int id, TQWidget *parent,
                 const char *name=0L, const TQString &txt=TQString::null,
                 TDEInstance *_instance = TDEGlobal::instance());

  /**
   * Construct a button with an existing pixmap.  It is not
   * recommended that you use this as the internal icon loading code
   * will almost always get it "right".
   *
   * @param pixmap Name of icon to load (may be absolute or relative)
   * @param id     Id of this button
   * @param parent This button's parent
   * @param name   This button's internal name
   * @param txt    This button's text (in a tooltip or otherwise)
   */
  TDEToolBarButton(const TQPixmap& pixmap, int id, TQWidget *parent,
                 const char *name=0L, const TQString &txt=TQString::null);

  /**
   * Construct a separator button
   *
   * @param parent This button's parent
   * @param name   This button's internal name
   */
  TDEToolBarButton(TQWidget *parent=0L, const char *name=0L);

  /**
   * Standard destructor
   */
  ~TDEToolBarButton();

#ifndef KDE_NO_COMPAT
  /**
   * @deprecated
   * Set the pixmap directly for this button.  This pixmap should be
   * the active one... the dimmed and disabled pixmaps are constructed
   * based on this one.  However, don't use this function unless you
   * are positive that you don't want to use setIcon.
   *
   * @param pixmap The active pixmap
   */
  // this one is from TQButton, so #ifdef-ing it out doesn't break BC
  virtual void setPixmap(const TQPixmap &pixmap) TDE_DEPRECATED;

  /**
   * @deprecated
   * Force the button to use this pixmap as the default one rather
   * then generating it using effects.
   *
   * @param pixmap The pixmap to use as the default (normal) one
   */
  void setDefaultPixmap(const TQPixmap& pixmap) TDE_DEPRECATED;

  /**
   * @deprecated
   * Force the button to use this pixmap when disabled one rather then
   * generating it using effects.
   *
   * @param pixmap The pixmap to use when disabled
   */
  void setDisabledPixmap(const TQPixmap& pixmap) TDE_DEPRECATED;
#endif

  /**
   * Set the text for this button.  The text will be either used as a
   * tooltip (IconOnly) or will be along side the icon
   *
   * @param text The button (or tooltip) text
   */
  virtual void setText(const TQString &text);

  /**
   * Set the icon for this button. The icon will be loaded internally
   * with the correct size. This function is preferred over setIconSet
   *
   * @param icon The name of the icon
   */
  virtual void setIcon(const TQString &icon);

  /// @since 3.1
  virtual void setIcon( const TQPixmap &pixmap )
  { TQToolButton::setIcon( pixmap ); }

  /**
   * Set the pixmaps for this toolbar button from a TQIconSet.
   * If you call this you don't need to call any of the other methods
   * that set icons or pixmaps.
   * @param iconset  The iconset to use
   */
  virtual void setIconSet( const TQIconSet &iconset );

#ifndef KDE_NO_COMPAT
  /**
   * @deprecated
   * Set the active icon for this button.  The pixmap itself is loaded
   * internally based on the icon size...  .. the disabled and default
   * pixmaps, however will only be constructed if generate is
   * true.  This function is preferred over setPixmap
   *
   * @param icon     The name of the active icon
   * @param generate If true, then the other icons are automagically
   *                 generated from this one
   */
  TDE_DEPRECATED void setIcon(const TQString &icon, bool generate ) { Q_UNUSED(generate); setIcon( icon ); }

  /**
   * @deprecated
   * Force the button to use this icon as the default one rather
   * then generating it using effects.
   *
   * @param icon The icon to use as the default (normal) one
   */
  void setDefaultIcon(const TQString& icon) TDE_DEPRECATED;

  /**
   * @deprecated
   * Force the button to use this icon when disabled one rather then
   * generating it using effects.
   *
   * @param icon The icon to use when disabled
   */
  void setDisabledIcon(const TQString& icon) TDE_DEPRECATED;
#endif

  /**
   * Turn this button on or off
   *
   * @param flag true or false
   */
  void on(bool flag = true);

  /**
   * Toggle this button
   */
  void toggle();

  /**
   * Turn this button into a toggle button or disable the toggle
   * aspects of it.  This does not toggle the button itself.
   * Use toggle() for that.
   *
   * @param toggle true or false
   */
  void setToggle(bool toggle = true);

  /**
   * Return a pointer to this button's popup menu (if it exists)
   */
  TQPopupMenu *popup();

  /**
   * Returns the button's id.
   * @since 3.2
   */
  int id() const;

  /**
   * Give this button a popup menu.  There will not be a delay when
   * you press the button.  Use setDelayedPopup if you want that
   * behavior.
   *
   * @param p The new popup menu
   * @param unused Has no effect - ignore it.
   */
  void setPopup (TQPopupMenu *p, bool unused = false);

  /**
   * Gives this button a delayed popup menu.
   *
   * This function allows you to add a delayed popup menu to the button.
   * The popup menu is then only displayed when the button is pressed and
   * held down for about half a second.
   *
   * @param p the new popup menu
   * @param unused Has no effect - ignore it.
   */
  void setDelayedPopup(TQPopupMenu *p, bool unused = false);

  /**
   * Turn this button into a radio button
   *
   * @param f true or false
   */
  void setRadio(bool f = true);

  /**
   * Toolbar buttons naturally will assume the global styles
   * concerning icons, icons sizes, etc.  You can use this function to
   * explicitly turn this off, if you like.
   *
   * @param no_style Will disable styles if true
   */
  void setNoStyle(bool no_style = true);

signals:
  /**
   * Emitted when the toolbar button is clicked (with LMB or MMB)
   */
  void clicked(int);
  /**
   * Emitted when the toolbar button is clicked (with any mouse button)
   * @param state makes it possible to find out which button was pressed,
   * and whether any keyboard modifiers were held.
   * @since 3.4
   */
  void buttonClicked(int, TQt::ButtonState state);
  void doubleClicked(int);
  void pressed(int);
  void released(int);
  void toggled(int);
  void highlighted(int, bool);

public slots:
  /**
   * This slot should be called whenever the toolbar mode has
   * potentially changed.  This includes such events as text changing,
   * orientation changing, etc.
   */
   void modeChange();
   virtual void setTextLabel(const TQString&, bool tipToo);

protected:
  bool event(TQEvent *e);
  void paletteChange(const TQPalette &);
  void leaveEvent(TQEvent *e);
  void enterEvent(TQEvent *e);
  void drawButton(TQPainter *p);
  bool eventFilter (TQObject *o, TQEvent *e);
  /// @since 3.4
  void mousePressEvent( TQMouseEvent * );
  /// @since 3.4
  void mouseReleaseEvent( TQMouseEvent * );
  void showMenu();
  TQSize sizeHint() const;
  TQSize minimumSizeHint() const;
  TQSize minimumSize() const;

  /// @since 3.1
  bool isRaised() const;
  /// @since 3.1
  bool isActive() const;
  /// @since 3.1
  int iconTextMode() const;

protected slots:
  void slotClicked();
  void slotPressed();
  void slotReleased();
  void slotToggled();
  void slotDelayTimeout();

protected:
  virtual void virtual_hook( int id, void* data );
private:
  TDEToolBarButtonPrivate *d;
};

/**
* List of TDEToolBarButton objects.
* @internal
* @version $Id$
*/
class TDEUI_EXPORT TDEToolBarButtonList : public TQIntDict<TDEToolBarButton>
{
public:
   TDEToolBarButtonList();
   ~TDEToolBarButtonList() {}
};

#endif

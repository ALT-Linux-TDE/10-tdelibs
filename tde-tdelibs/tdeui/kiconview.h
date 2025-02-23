/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

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
#ifndef KICONVIEW_H
#define KICONVIEW_H

#include <tqcursor.h>
#include <tqiconview.h>

#include <tdelibs_export.h>

/**
 * @short A variant of TQIconView that honors KDE's system-wide settings.
 *
 * This Widget extends the functionality of TQIconView to honor the system
 * wide settings for Single Click/Double Click mode, Auto Selection and
 * Change Cursor over Link.
 *
 * There is a new signal executed(). It gets connected to either
 * TQIconView::clicked() or TQIconView::doubleClicked() depending on the KDE
 * wide Single Click/Double Click settings. It is strongly recommended that
 * you use this signal instead of the above mentioned. This way you don�t
 * need to care about the current settings.
 * If you want to get informed when the user selects something connect to the
 * TQIconView::selectionChanged() signal.
 *
 **/
class TDEUI_EXPORT TDEIconView : public TQIconView
{
  friend class TDEIconViewItem;
  TQ_OBJECT
  TQ_ENUMS( Mode )
  TQ_PROPERTY( Mode mode READ mode WRITE setMode )

public:
  TDEIconView( TQWidget *parent = 0, const char *name = 0, WFlags f = 0 );

  ~TDEIconView();

  /**
   * TDEIconView has two different operating modes. Execute mode is depending
   * on the configuration of single-click or double-click where the signal
   * executed() will be emitted upon click/double-click.
   * In Select mode, this signal will not be emitted.
   *
   * Default is Execute mode.
   */
  enum Mode { Execute, Select };

  /**
   * Sets the mode to Execute or Select.
   * @li In Execute mode, the signal executed()
   * will be emitted when the user clicks/double-clicks an item.
   * @li Select mode is
   * the normal TQIconView mode.
   *
   * Default is Execute.
   */
  void setMode( Mode m );

  /**
   * @returns the current Mode, either Execute or Select.
   */
  Mode mode() const;

  /**
   * Reimplemented for internal purposes
   */
  virtual void setFont( const TQFont & );

   /**
    * Set the maximum number of lines that will be used to display icon text.
    * Setting this value will enable word-wrap, too.
    * @since 3.3
    *
    * @param n Number of lines
    */
  void setIconTextHeight( int n );

   /**
    * @return The height of icon text in lines
    * @since 3.3
    */
  int iconTextHeight() const;

  /**
   * Reimplemented for held() signal behavior internal purposes
   */
  virtual void takeItem( TQIconViewItem * item );

signals:

  /**
   * This signal is emitted whenever the user executes an iconview item.
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed iconview item.
   *
   * Note that you may not delete any TQIconViewItem objects in slots
   * connected to this signal.
   */
  void executed( TQIconViewItem *item );

  /**
   * This signal is emitted whenever the user executes an iconview item.
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed iconview item.
   * @param pos is the position where the user has clicked
   *
   * Note that you may not delete any TQIconViewItem objects in slots
   * connected to this signal.
   */
  void executed( TQIconViewItem *item, const TQPoint &pos );

  /**
   * This signal is emitted whenever the user hold something on an iconview
   * during a drag'n'drop.
   * @param item is the pointer to the iconview item the hold event occur.
   *
   * Note that you may not delete any TQIconViewItem objects in slots
   * connected to this signal.
   */
  void held( TQIconViewItem *item );

  /**
   * This signal gets emitted whenever the user double clicks into the
   * iconview.
   * @param item is the pointer to the clicked iconview item.
   * @param pos is the position where the user has clicked, and
   *
   * Note that you may not delete any TQIconViewItem objects in slots
   * connected to this signal.
   *
   * This signal is more or less here for the sake of completeness.
   * You should normally not need to use this. In most cases it's better
   * to use executed() instead.
   */
  void doubleClicked( TQIconViewItem *item, const TQPoint &pos );

protected slots:
  void slotOnItem( TQIconViewItem *item );
  void slotOnViewport();
  void slotSettingsChanged(int);

  /**
   * Auto selection happend.
   */
  void slotAutoSelect();

protected:
  void emitExecute( TQIconViewItem *item, const TQPoint &pos );
  void updateDragHoldItem( TQDropEvent *e );

  virtual void focusOutEvent( TQFocusEvent *fe );
  virtual void leaveEvent( TQEvent *e );
  virtual void contentsMousePressEvent( TQMouseEvent *e );
  virtual void contentsMouseDoubleClickEvent ( TQMouseEvent * e );
  virtual void contentsMouseReleaseEvent( TQMouseEvent *e );
  virtual void contentsDragEnterEvent( TQDragEnterEvent *e );
  virtual void contentsDragLeaveEvent( TQDragLeaveEvent *e );
  virtual void contentsDragMoveEvent( TQDragMoveEvent *e );
  virtual void contentsDropEvent( TQDropEvent* e );
  virtual void wheelEvent( TQWheelEvent *e );

  /**
   * This method allows to handle correctly cases where a subclass
   * needs the held() signal to not be triggered without calling
   * a TDEIconView::contentsDrag*Event() method (which have side effects
   * because they forward to TQIconView).
   */
  void cancelPendingHeldSignal();
  
private slots:
  void slotMouseButtonClicked( int btn, TQIconViewItem *item, const TQPoint &pos );
  void slotDragHoldTimeout();

private:
  /**
   * @internal. For use by TDEIconViewItem.
   */
  TQFontMetrics *itemFontMetrics() const;
  /**
   * @internal. For use by TDEIconViewItem.
   */
  TQPixmap selectedIconPixmap( TQPixmap *pix, const TQColor &col ) const;

  bool m_bUseSingle;
  bool m_bChangeCursorOverItem;

  TQIconViewItem* m_pCurrentItem;

  TQTimer* m_pAutoSelect;
  int m_autoSelectDelay;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class TDEIconViewPrivate;
  TDEIconViewPrivate *d;
};

class KWordWrap;
/**
 * @short A variant of TQIconViewItem that wraps words better.
 *
 * TDEIconViewItem exists to improve the word-wrap functionality of QIconViewItem
 * Use TDEIconViewItem instead of TQIconViewItem for any iconview item you might have :)
 *
 * @author David Faure <david@mandrakesoft.com>
 */
class TDEUI_EXPORT TDEIconViewItem : public TQIconViewItem
{
public:
    // Need to redefine all the constructors - I want Java !
    TDEIconViewItem( TQIconView *parent )
        : TQIconViewItem( parent ) { init(); } // We need to call it because the parent ctor won't call our reimplementation :(((
    TDEIconViewItem( TQIconView *parent, TQIconViewItem *after )
        : TQIconViewItem( parent, after ) { init(); }
    TDEIconViewItem( TQIconView *parent, const TQString &text )
        : TQIconViewItem( parent, text ) { init(); }
    TDEIconViewItem( TQIconView *parent, TQIconViewItem *after, const TQString &text )
        : TQIconViewItem( parent, after, text ) { init(); }
    TDEIconViewItem( TQIconView *parent, const TQString &text, const TQPixmap &icon )
        : TQIconViewItem( parent, text, icon ) { init(); }
    TDEIconViewItem( TQIconView *parent, TQIconViewItem *after, const TQString &text, const TQPixmap &icon )
        : TQIconViewItem( parent, after, text, icon ) { init(); }
    TDEIconViewItem( TQIconView *parent, const TQString &text, const TQPicture &picture )
        : TQIconViewItem( parent, text, picture ) { init(); }
    TDEIconViewItem( TQIconView *parent, TQIconViewItem *after, const TQString &text, const TQPicture &picture )
        : TQIconViewItem( parent, after, text, picture ) { init(); }
    virtual ~TDEIconViewItem();

   /**
    * Using this function, you can specify a custom size for the pixmap. The
    * geometry of the item will be calculated to let a pixmap of the given size
    * fit in the iconView without needing an update.
    * This may be useful if you want to change the pixmap later without breaking
    * the layout. A possible use of this function is to replace a fileItem icon
    * by a larger pixmap (preview).
    * @since 3.3
    *
    * @param size The size to use
    */
    void setPixmapSize( const TQSize& size );

   /**
    * @return The size set by setPixmapSize() or TQSize( 0, 0 )
    * @since 3.3
    */
    TQSize pixmapSize() const;

protected:
    void init();
    virtual void calcRect( const TQString& text_ = TQString::null );
    virtual void paintItem( TQPainter *p, const TQColorGroup &c );
    KWordWrap *wordWrap();
    void paintPixmap( TQPainter *p, const TQColorGroup &c );
    void paintText( TQPainter *p, const TQColorGroup &c );

private:
    KWordWrap* m_wordWrap;
    struct TDEIconViewItemPrivate;
    TDEIconViewItemPrivate *d;
};

#endif

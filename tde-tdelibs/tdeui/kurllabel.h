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

#ifndef KURLLABEL_H
#define KURLLABEL_H

#include <tqlabel.h>

#include <tdelibs_export.h>

class TQColor;
class TQCursor;
class TQPixmap;

/**
 * @short A drop-in replacement for TQLabel that displays hyperlinks.
 *
 * KURLLabel is a drop-in replacement for TQLabel that handles text
 * in a fashion similar to how an HTML widget handles hyperlinks.  The
 * text can be underlined (or not) and set to different colors.  It
 * can also "glow" (cycle colors) when the mouse passes over it.
 *
 * KURLLabel also provides signals for several events, including
 * the mouse leaving and entering the text area and all forms of
 * mouse clicking.
 *
 * By default KURLLabel accepts focus. When focused, standard 
 * focus rectangle is displayed as in HTML widget. 
 * Pressing Enter key accepts the focused label.
 *
 * A typical usage would be something like so:
 *
 * \code
 * KURLLabel *address = new KURLLabel(this);
 * address->setText("My homepage");
 * address->setURL("http://www.home.com/~me");
 * connect(address, TQ_SIGNAL(leftClickedURL(const TQString&)),
 *                  TQ_SLOT(processMyURL(const TQString&)));
 * \endcode
 *
 * In this example, the text "My homepage" would be displayed
 * as blue, underlined text.  When the mouse passed over it,
 * it would "glow" red.  When the user clicks on the text, the
 * signal leftClickedURL() would be emitted with "http://www.home.com/~me"
 * as its argument.
 *
 * \image html kurllabel.png "KDE URL Label"
 *
 * @author Kurt Granroth <granroth@kde.org> (Interface)
 * @author Peter Putzer <putzer@kde.org> (Rewrite)
 * @version $Id$
 *
 */
class TDEUI_EXPORT KURLLabel : public TQLabel
{
  TQ_OBJECT
  TQ_PROPERTY (TQString url READ url WRITE setURL)
  TQ_PROPERTY (TQString tipText READ tipText WRITE setTipText )
  TQ_PROPERTY (TQPixmap altPixmap READ altPixmap WRITE setAltPixmap)
  TQ_PROPERTY (bool glowEnabled READ isGlowEnabled WRITE setGlow )
  TQ_PROPERTY (bool floatEnabled READ isFloatEnabled WRITE setFloat )
  TQ_PROPERTY (bool useTips READ useTips WRITE setUseTips )
  TQ_PROPERTY (bool useCursor READ useCursor WRITE setUseCursor )

public:
  /**
   * Default constructor.
   *
   * Use setURL() and setText() or TQListView::setPixmap()
   * to set the resp. properties.
   */
  KURLLabel (TQWidget* parent = 0L, const char* name = 0L);

  /**
   * Convenience constructor.
   *
   * @param url is the URL emitted when the label is clicked.
   * @param text is the displayed string. If it's equal to TQString::null
   * the @p url will be used instead.
   * @param parent Passed to lower level constructor
   * @param name Passed to lower level constructor
   *
   * @p parent and @p name are passed to TQLabel, which in turn passes
   * them further down
   */
  KURLLabel (const TQString& url, const TQString& text = TQString::null,
              TQWidget* parent = 0L, const char* name = 0L);

  /**
   * Destructs the label.
   */
  virtual ~KURLLabel ();

  /**
   * Returns the URL.
   */
  const TQString& url () const;

  /**
   * Returns the current tooltip text.
   */
  const TQString& tipText () const;

  /**
   * @return true if a tooltip will be displayed.
   *
   * @see setTipText()
   */
  bool useTips () const;

  /**
   * @return true if the cursor will change while over the URL.
   *
   * @see setUseCursor ()
   */
  bool useCursor () const;

  /**
   * When this is on, the text will switch to the selected
   * color whenever the mouse passes over it.
   */
  bool isGlowEnabled () const;

  /**
   * This feature is very similar to the "glow" feature in that the color of the
   * label switches to the selected color when the cursor passes
   * over it. In addition, underlining is turned on for as
   * long as the mouse is overhead. Note that if "glow" and
   * underlining are both already turned on, this feature
   * will have no visible effect.
   */
  bool isFloatEnabled () const;

  /**
   * @return the alternate pixmap (may be 0L if none was set).
   */
  const TQPixmap* altPixmap () const;

  /**
   * Reimplemented for internal reasons, the API is not affected.
   */
  virtual void setMargin ( int margin );

  /**
   * Reimplemented for internal reasons, the API is not affected.
   */
#ifdef qdoc
#else
  virtual void setFocusPolicy ( TQWidget::FocusPolicy policy );
#endif

  /**
   * Reimplemented for internal reasons, the API is not affected.
   */
  virtual void setSizePolicy ( TQSizePolicy );

public slots:
  /**
   * Turns on or off the underlining.
   *
   *  When this is on, the
   * text will be underlined.  By default, it is @p true.
   */
  void setUnderline (bool on = true);

  /**
   * Sets the URL for this label to @p url.
   *
   * @see url
   */
  void setURL (const TQString& url);

  /**
   * Overridden for internal reasons; the API remains unaffected.
   */
  virtual void setFont (const TQFont&);

  /**
   * Turns on or off the tool tip feature.
   *
   * When this is on, the URL will be displayed as a
   * tooltip whenever the mouse passes passes over it.
   * By default, it is @p false.
   */
  void setUseTips (bool on = true);

  /**
   * Specifies what text to display when tooltips are turned on.
   *
   * If this is not used, the tip will default to the URL.
   *
   * @see setUseTips()
   */
  void setTipText (const TQString& tip);

  /**
   * Sets the highlight color.
   *
   * This is the default foreground
   * color (non-selected).  By default, it is @p blue.
   */
  void setHighlightedColor(const TQColor& highcolor);

  /**
   * This is an overloaded version for convenience.
   *
   * @see setHighlightedColor()
   */
  void setHighlightedColor(const TQString& highcolor);

  /**
   * Sets the selected color.
   *
   * This is the color the text will change
   * to when either a mouse passes over it and "glow" mode is on or
   * when it is selected (clicked).  By default, it is @p red.
   */
  void setSelectedColor(const TQColor& selcolor);

  /**
   * This is an overloaded version for convenience.
   *
   * @see setSelectedColor()
   */
  void setSelectedColor(const TQString& selcolor);

  /**
   * Overridden for internal reasons; the API remains unaffected.
   */
  virtual void setCursor ( const TQCursor& cursor );

  /**
   * Overridden for internal reasons; the API remains unaffected.
   */
  virtual void unsetCursor ();

  /**
   * Turns the custom cursor feature on or off.
   *
   * When this is on, the cursor will change to a custom cursor
   * (default is a "pointing hand") whenever the cursor passes
   * over the label. By default, it is on.
   *
   * @param on whether a custom cursor should be displayed.
   * @param cursor is the custom cursor. @p 0L indicates the default "hand cursor".
   */
  void setUseCursor (bool on, TQCursor* cursor = 0L);

  /**
   * Turns on or off the "glow" feature.
   *
   * When this is on, the text will switch to the
   * selected color whenever the mouse
   * passes over it. By default, it is @p true.
   */
  void setGlow (bool glow = true);

  /**
   * Turns on or off the "float" feature.
   *
   * This feature is very similar to the "glow" feature in
   * that the color of the label switches to the selected
   * color when the cursor passes over it. In addition,
   * underlining is turned on for as long as the mouse is overhead.
   * Note that if "glow" and underlining are both already turned
   * on, this feature will have no visible effect.
   * By default, it is @p false.
   */
  void setFloat (bool do_float = true);

  /**
   * Sets the "alt" pixmap.
   *
   * This pixmap will be displayed when the
   * cursor passes over the label.  The effect is similar to the
   * trick done with 'onMouseOver' in javascript.
   *
   * @see altPixmap()
   */
  void setAltPixmap (const TQPixmap& altPix);

signals:

  /**
   * Emitted when the mouse has passed over the label.
   *
   * @param url The URL for this label.
   */
  void enteredURL (const TQString& url);

  /**
   * Emitted when the mouse has passed over the label.
   */
  void enteredURL ();

  /**
   * Emitted when the mouse is no longer over the label.
   *
   * @param url The URL for this label.
   */
  void leftURL (const TQString& url);

  /**
   * Emitted when the mouse is no longer over the label.
   */
  void leftURL ();

  /**
   * Emitted when the user clicked the left mouse button on this label.
   *
   * @param url The URL for this label.
   */
  void leftClickedURL(const TQString& url);

  /**
   * Emitted when the user clicked the left mouse button on this label.
   */
  void leftClickedURL();

  /**
   * Emitted when the user clicked the right mouse button on this label.
   *
   * @param url The URL for this label.
   */
  void rightClickedURL(const TQString& url);

  /**
   * Emitted when the user clicked the right mouse button on this label.
   */
  void rightClickedURL();

  /**
   * Emitted when the user clicked the middle mouse button on this label.
   *
   * @param url The URL for this label.
   */
  void middleClickedURL(const TQString& url);

  /**
   * Emitted when the user clicked the left mouse button on this label.
   */
  void middleClickedURL();

protected:

  /**
   * Overridden for internal reasons; the API remains unaffected.
   */
  virtual void mouseReleaseEvent ( TQMouseEvent * e );

  /**
   * Overridden for internal reasons; the API remains unaffected.
   */
  virtual void enterEvent (TQEvent*);

  /**
   * Overridden for internal reasons; the API remains unaffected.
   */
  virtual void leaveEvent (TQEvent*);

  /**
   * Catch parent palette changes
   */
  virtual bool event (TQEvent *e);

  /**
   * 
   */
  TQRect activeRect() const;


private slots:
  /**
   * @internal
   * Slot used to reset the link-color to normal (timer-driven).
   */
  void updateColor ();

private:
  /**
   * @internal
   * A private helper function to set the link-color to @p col.
   */
  void setLinkColor (const TQColor& col);

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class Private;
  Private* d;
};

#endif // KURLLABEL_H


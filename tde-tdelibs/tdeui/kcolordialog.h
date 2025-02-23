/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
//----------------------------------------------------------------------
// KDE color selection dialog.

// layout management added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>

#ifndef KDELIBS_KCOLORDIALOG_H
#define KDELIBS_KCOLORDIALOG_H

#ifdef TQ_WS_QWS
// FIXME(E): Do we need the KColorDialog extra functionality in Qt Embedded?
#include <tqcolordialog.h>
#define KColorDialog QColorDialog
#else //UNIX, WIN32
#include <kdialogbase.h>
#include <tqframe.h>
#include <tqpixmap.h>
#include <tqgridview.h>

#include "tdeselect.h"

class TQComboBox;
class TQLineEdit;
class TDEListBox;
class KPalette;
class KColorCells;


/**
 * Widget for Hue/Saturation colour selection.
 *
 * The actual values can be fetched using the inherited xValue and yValue
 * methods.
 *
 * \image html khsselector.png "KDE Hue/Saturation Selection Widget"
 *
 * @see KXYSelector, KValueSelector, KColorDialog
 * @author Martin Jones (mjones@kde.org)
*/
class TDEUI_EXPORT KHSSelector : public KXYSelector
{
  TQ_OBJECT

public:
  /**
   * Constructs a hue/saturation selection widget.
   */
  KHSSelector( TQWidget *parent=0, const char *name=0 );

protected:
  /**
   * Draws the contents of the widget on a pixmap,
   * which is used for buffering.
   */
  virtual void drawPalette( TQPixmap *pixmap );
  virtual void resizeEvent( TQResizeEvent * );

  /**
   * Reimplemented from KXYSelector. This drawing is
   * buffered in a pixmap here. As real drawing
   * routine, drawPalette() is used.
   */
  virtual void drawContents( TQPainter *painter );

private:
  void updateContents();
  TQPixmap pixmap;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KHSSelectorPrivate;
  KHSSelectorPrivate *d;
};


class KValueSelectorPrivate;
/**
 * Widget for color value selection.
 *
 * @see KHSSelector, KColorDialog
 * @author Martin Jones (mjones@kde.org)
 */
class TDEUI_EXPORT KValueSelector : public TDESelector
{
  TQ_OBJECT

public:
  /**
   * Constructs a widget for color selection.
   */
  KValueSelector( TQWidget *parent=0, const char *name=0 );
  /**
   * Constructs a widget for color selection with a given orientation
   */
  KValueSelector( Orientation o, TQWidget *parent = 0, const char *name = 0 );

  int hue() const
        { return _hue; }
  void setHue( int h )
        { _hue = h; }
  int saturation() const
        { return _sat; }
  void setSaturation( int s )
        { _sat = s; }

  void updateContents();
protected:
  /**
   * Draws the contents of the widget on a pixmap,
   * which is used for buffering.
   */
  virtual void drawPalette( TQPixmap *pixmap );
  virtual void resizeEvent( TQResizeEvent * );

  /**
   * Reimplemented from TDESelector. The drawing is
   * buffered in a pixmap here. As real drawing
   * routine, drawPalette() is used.
   */
  virtual void drawContents( TQPainter *painter );

private:
  int _hue;
  int _sat;
  TQPixmap pixmap;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KValueSelectorPrivate;
  KValueSelectorPrivate *d;
};


/**
 * A color class that preserves both RGB and HSV values.
 *
 * This is
 * unlike TQColor which only preserves RGB values and recalculates HSV
 * values. The TQColor behavior leads to an accumulation of rounding
 * errors when working in the HSV color space.
 *
 * @author Waldo Bastian <bastian@kde.org>
 **/
class TDEUI_EXPORT KColor : public TQColor
{
public:
  KColor();
  KColor( const KColor &col);
  KColor( const TQColor &col);

  KColor& operator=( const KColor& col);

  bool operator==( const KColor& col) const;

  void setHsv(int _h, int _s, int _v);
  void setRgb(int _r, int _g, int _b);

  void rgb(int *_r, int *_g, int *_b) const;
  void hsv(int *_h, int *_s, int *_v) const;
protected:
  int h;
  int s;
  int v;
  int r;
  int g;
  int b;

private:
  class KColorPrivate;
  KColorPrivate *d;
};

/**
 * A color palette in table form.
 *
 * @author Waldo Bastian <bastian@kde.org>
 **/
class TDEUI_EXPORT KPaletteTable : public TQWidget
{
  TQ_OBJECT
public:
  KPaletteTable( TQWidget *parent, int minWidth=210, int cols = 16);
  ~KPaletteTable();
  void addToCustomColors( const TQColor &);
  void addToRecentColors( const TQColor &);
  TQString palette() const;
public slots:
  void setPalette(const TQString &paletteName);
signals:
  void colorSelected( const TQColor &, const TQString & );
  void colorDoubleClicked( const TQColor &, const TQString & );

protected slots:
  void slotColorCellSelected( int );
  void slotColorCellDoubleClicked( int );
  void slotColorTextSelected( const TQString &colorText );
  void slotSetPalette( const TQString &_paletteName );
  void slotShowNamedColorReadError( void );

protected:
  void readNamedColor( void );

protected:
  /// \deprecated
  TQString i18n_customColors; /// ### KDE4: remove
  /// \deprecated
  TQString i18n_recentColors; /// ### KDE4: remove
  TQString i18n_namedColors;
  TQComboBox *combo;
  KColorCells *cells;
  TQScrollView *sv;
  TDEListBox *mNamedColorList;
  KPalette *mPalette;
  int mMinWidth;
  int mCols;

private:

  virtual void setPalette(const TQPalette& p) { TQWidget::setPalette(p); }
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KPaletteTablePrivate;
  KPaletteTablePrivate *d;
};


/**
* A table of editable color cells.
*
* @author Martin Jones <mjones@kde.org>
*/
class TDEUI_EXPORT KColorCells : public TQGridView
{
  TQ_OBJECT
public:
  KColorCells( TQWidget *parent, int rows, int cols );
  ~KColorCells();

  void setColor( int colNum, const TQColor &col );
  TQColor color( int indx ) const
  {	return colors[indx]; }
  int numCells() const
  {	return numRows() * numCols(); }

  void setShading(bool _shade) { shade = _shade; }

  void setAcceptDrags(bool _acceptDrags) { acceptDrags = _acceptDrags; }

  int getSelected() const
  {	return selected; }

  signals:
  void colorSelected( int col );
  void colorDoubleClicked( int col );

protected:
  virtual void paintCell( TQPainter *painter, int row, int col );
  virtual void resizeEvent( TQResizeEvent * );
  virtual void mouseReleaseEvent( TQMouseEvent * );
  virtual void mousePressEvent( TQMouseEvent * );
  virtual void mouseMoveEvent( TQMouseEvent * );
  virtual void dragEnterEvent( TQDragEnterEvent *);
  virtual void dropEvent( TQDropEvent *);
  virtual void mouseDoubleClickEvent( TQMouseEvent * );

  int posToCell(const TQPoint &pos, bool ignoreBorders=false);

  TQColor *colors;
  bool inMouse;
  TQPoint mPos;
  int	selected;
  bool shade;
  bool acceptDrags;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KColorCellsPrivate;
  KColorCellsPrivate *d;
};

/**
 * @short A color displayer.
 *
 * The KColorPatch widget is a (usually small) widget showing
 * a selected color e. g. in the KColorDialog. It
 * automatically handles drag and drop from and on the widget.
 *
 */
class TDEUI_EXPORT KColorPatch : public TQFrame
{
  TQ_OBJECT
public:
  KColorPatch( TQWidget *parent );
  virtual ~KColorPatch();

  void setColor( const TQColor &col );

signals:
  void colorChanged( const TQColor&);

protected:
  virtual void drawContents( TQPainter *painter );
  virtual void mouseMoveEvent( TQMouseEvent * );
  virtual void dragEnterEvent( TQDragEnterEvent *);
  virtual void dropEvent( TQDropEvent *);

private:
  TQColor color;
  uint pixel;
  int colContext;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KColorPatchPrivate;
  KColorPatchPrivate *d;
};

/**
 * @short A color selection dialog.
 *
 * <b>Features:</b>\n
 *
 * @li Color selection from a wide range of palettes.
 * @li Color selection from a palette of H vs S and V selectors.
 * @li Direct input of HSV or RGB values.
 * @li Saving of custom colors
 *
 * In most cases, you will want to use the static method KColorDialog::getColor().
 * This pops up the dialog (with an initial selection provided by you), lets the
 * user choose a color, and returns.
 *
 * Example:
 *
 * \code
 * 	TQColor myColor;
 * 	int result = KColorDialog::getColor( myColor );
 *         if ( result == KColorDialog::Accepted )
 *            ...
 * \endcode
 *
 * @image html kcolordialog.png "KDE Color Dialog"
 *
 * The color dialog is really a collection of several widgets which can
 * you can also use separately: the quadratic plane in the top left of
 * the dialog is a KXYSelector. Right next to it is a KHSSelector
 * for choosing hue/saturation.
 *
 * On the right side of the dialog you see a KPaletteTable showing
 * up to 40 colors with a combo box which offers several predefined
 * palettes or a palette configured by the user. The small field showing
 * the currently selected color is a KColorPatch.
 *
 **/
class TDEUI_EXPORT KColorDialog : public KDialogBase
{
  TQ_OBJECT

  public:
    /**
     * Constructs a color selection dialog.
     */
    KColorDialog( TQWidget *parent = 0L, const char *name = 0L,
		  bool modal = false );
    /**
     * Destroys the color selection dialog.
     */
    ~KColorDialog();

    /**
     * Returns the currently selected color.
     **/
    TQColor color() const;

    /**
     * Creates a modal color dialog, let the user choose a
     * color, and returns when the dialog is closed.
     *
     * The selected color is returned in the argument @p theColor.
     *
     * @returns TQDialog::result().
     */
    static int getColor( TQColor &theColor, TQWidget *parent=0L );

    /**
     * Creates a modal color dialog, lets the user choose a
     * color, and returns when the dialog is closed.
     *
     * The selected color is returned in the argument @p theColor.
     *
     * This version takes a @p defaultColor argument, which sets the color
     * selected by the "default color" checkbox. When this checkbox is checked,
     * the invalid color (TQColor()) is returned into @p theColor.
     *
     * @returns TQDialog::result().
     */
    static int getColor( TQColor &theColor, const TQColor& defaultColor, TQWidget *parent=0L );

    /**
     * Gets the color from the pixel at point p on the screen.
     */
    static TQColor grabColor(const TQPoint &p);

    /**
     * Call this to make the dialog show a "Default Color" checkbox.
     * If this checkbox is selected, the dialog will return an "invalid" color (TQColor()).
     * This can be used to mean "the default text color", for instance,
     * the one with the KDE text color on screen, but black when printing.
     */
    void setDefaultColor( const TQColor& defaultCol );

    /**
     * @return the value passed to setDefaultColor
     */
    TQColor defaultColor() const;

  public slots:
    /**
     * Preselects a color.
     */
    void setColor( const TQColor &col );

  signals:
    /**
     * Emitted when a color is selected.
     * Connect to this to monitor the color as it as selected if you are
     * not running modal.
     */
    void colorSelected( const TQColor &col );

  private slots:
    void slotRGBChanged( void );
    void slotHSVChanged( void );
    void slotHtmlChanged( void );
    void slotHSChanged( int, int );
    void slotVChanged( int );
    void slotColorSelected( const TQColor &col );
    void slotColorSelected( const TQColor &col, const TQString &name );
    void slotColorDoubleClicked( const TQColor &col, const TQString &name );
    void slotColorPicker();
    void slotAddToCustomColors();
    void slotDefaultColorClicked();
    /**
     * Write the settings of the dialog to config file.
     **/
    void slotWriteSettings();

  private:
    /**
     * Read the settings for the dialog from config file.
     **/
    void readSettings();

    void setRgbEdit( const KColor &col );
    void setHsvEdit( const KColor &col );
    void setHtmlEdit( const KColor &col );
    void _setColor( const KColor &col, const TQString &name=TQString::null );
    void showColor( const KColor &color, const TQString &name );

  protected:
    virtual void mouseReleaseEvent( TQMouseEvent * );
    virtual void keyPressEvent( TQKeyEvent * );
    virtual bool eventFilter( TQObject *obj, TQEvent *ev );

  protected:
    virtual void virtual_hook( int id, void* data );
  private:
    class KColorDialogPrivate;
    KColorDialogPrivate *d;
};

#endif		// !TQ_WS_QWS
#endif		// KDELIBS_KCOLORDIALOG_H


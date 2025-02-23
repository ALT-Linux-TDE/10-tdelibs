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
//-----------------------------------------------------------------------------
// Selector widgets for KDE Color Selector, but probably useful for other
// stuff also.

#ifndef __TDESELECT_H__
#define __TDESELECT_H__

#include <tqwidget.h>
#include <tqrangecontrol.h>
#include <tqpixmap.h>

#include <tdelibs_export.h>

/**
 * KXYSelector is the base class for other widgets which
 * provides the ability to choose from a two-dimensional
 * range of values. The currently chosen value is indicated
 * by a cross. An example is the KHSSelector which
 * allows to choose from a range of colors, and which is
 * used in KColorDialog.
 *
 * A custom drawing routine for the widget surface has
 * to be provided by the subclass.
 */
class TDEUI_EXPORT KXYSelector : public TQWidget
{
  TQ_OBJECT
  TQ_PROPERTY( int xValue READ xValue WRITE setXValue )
  TQ_PROPERTY( int yValue READ yValue WRITE setYValue )
  
public:
  /**
   * Constructs a two-dimensional selector widget which
   * has a value range of [0..100] in both directions.
   */
  KXYSelector( TQWidget *parent=0, const char *name=0 );
  /**
   * Destructs the widget.
   */
  ~KXYSelector();

  /**
   * Sets the current values in horizontal and
   * vertical direction.
   * @param xPos the horizontal value
   * @param yPos the veritcal value
   */
  void setValues( int xPos, int yPos );
  
  /**
   * Sets the current horizontal value
   * @param xPos the horizontal value
   */
  void setXValue( int xPos );
  
  /**
   * Sets the current vertical value
   * @param yPos the veritcal value
   */
  void setYValue( int yPos );
  
  /**
   * Sets the range of possible values.
   */
  void setRange( int minX, int minY, int maxX, int maxY );

  /**
   * @return the current value in horizontal direction.
   */
  int xValue() const {	return xPos; }
  /**
   * @return the current value in vertical direction.
   */
  int yValue() const {	return yPos; }

  /**
   * @return the rectangle on which subclasses should draw.
   */
  TQRect contentsRect() const;

signals:
  /**
   * This signal is emitted whenever the user chooses a value,
   * e.g. by clicking with the mouse on the widget.
   */
  void valueChanged( int x, int y );

protected:
  /**
   * Override this function to draw the contents of the widget.
   * The default implementation does nothing.
   *
   * Draw within contentsRect() only.
   */
  virtual void drawContents( TQPainter * );
  /**
   * Override this function to draw the cursor which
   * indicates the currently selected value pair.
   */
  virtual void drawCursor( TQPainter *p, int xp, int yp );

  virtual void paintEvent( TQPaintEvent *e );
  virtual void mousePressEvent( TQMouseEvent *e );
  virtual void mouseMoveEvent( TQMouseEvent *e );
  virtual void wheelEvent( TQWheelEvent * );

  /**
   * Converts a pixel position to its corresponding values.
   */
  void valuesFromPosition( int x, int y, int& xVal, int& yVal ) const;

private:
  void setPosition( int xp, int yp );
  int px;
  int py;
  int xPos;
  int yPos;
  int minX;
  int maxX;
  int minY;
  int maxY;
  TQPixmap store;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KXYSelectorPrivate;
  KXYSelectorPrivate *d;
};


/**
 * TDESelector is the base class for other widgets which
 * provides the ability to choose from a one-dimensional
 * range of values. An example is the KGradientSelector
 * which allows to choose from a range of colors.
 *
 * A custom drawing routine for the widget surface has
 * to be provided by the subclass.
 */
class TDEUI_EXPORT TDESelector : public TQWidget, public TQRangeControl
{
  TQ_OBJECT
  TQ_PROPERTY( int value READ value WRITE setValue )
  TQ_PROPERTY( int minValue READ minValue WRITE setMinValue )
  TQ_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )
public:

  /**
   * Constructs a horizontal one-dimensional selection widget.
   */
  TDESelector( TQWidget *parent=0, const char *name=0 );
  /**
   * Constructs a one-dimensional selection widget with
   * a given orientation.
   */
  TDESelector( Orientation o, TQWidget *parent = 0L, const char *name = 0L );
  /*
   * Destructs the widget.
   */
  ~TDESelector();

  /**
   * @return the orientation of the widget.
   */
  Orientation orientation() const
  {	return _orientation; }

  /**
   * @return the rectangle on which subclasses should draw.
   */
  TQRect contentsRect() const;

  /**
   * Sets the indent option of the widget to i.
   * This determines whether a shaded frame is drawn.
   */
  void setIndent( bool i )
  {	_indent = i; }
  /**
   * @return whether the indent option is set.
   */
  bool indent() const
  {	return _indent; }

  /**
   * Sets the value.
   */
  void setValue(int value)
  { TQRangeControl::setValue(value); }

  /**
   * @returns the value.
   */
  int value() const
  { return TQRangeControl::value(); }

  /**
   * Sets the min value.
   */
  void setMinValue(int value)
  { TQRangeControl::setMinValue(value); }

  /**
   * @return the min value.
   */
  int minValue() const
  { return TQRangeControl::minValue(); }

  /**
   * Sets the max value.
   */
  void setMaxValue(int value)
  { TQRangeControl::setMaxValue(value); }

  /**
   * @return the max value.
   */
  int maxValue() const
  { return TQRangeControl::maxValue(); }

signals:
  /**
   * This signal is emitted whenever the user chooses a value,
   * e.g. by clicking with the mouse on the widget.
   */
  void valueChanged( int value );

protected:
  /**
   * Override this function to draw the contents of the control.
   * The default implementation does nothing.
   *
   * Draw only within contentsRect().
   */
  virtual void drawContents( TQPainter * );
  /**
   * Override this function to draw the cursor which
   * indicates the current value. This function is
   * always called twice, once with argument show=false
   * to clear the old cursor, once with argument show=true
   * to draw the new one.
   */
  virtual void drawArrow( TQPainter *painter, bool show, const TQPoint &pos );

  virtual void valueChange();
  virtual void paintEvent( TQPaintEvent * );
  virtual void mousePressEvent( TQMouseEvent *e );
  virtual void mouseMoveEvent( TQMouseEvent *e );
  virtual void wheelEvent( TQWheelEvent * );

private:
  TQPoint calcArrowPos( int val );
  void moveArrow( const TQPoint &pos );

  Orientation _orientation;
  bool _indent;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class TDESelectorPrivate;
  TDESelectorPrivate *d;
};


/**
 * The KGradientSelector widget allows the user to choose
 * from a one-dimensional range of colors which is given as a
 * gradient between two colors provided by the programmer.
 *
 * \image html kgradientselector.png "KDE Gradient Selector Widget"
 *
 **/
class TDEUI_EXPORT KGradientSelector : public TDESelector
{
  TQ_OBJECT

  TQ_PROPERTY( TQColor firstColor READ firstColor WRITE setFirstColor )
  TQ_PROPERTY( TQColor secondColor READ secondColor WRITE setSecondColor )
  TQ_PROPERTY( TQString firstText READ firstText WRITE setFirstText )
  TQ_PROPERTY( TQString secondText READ secondText WRITE setSecondText )

public:
  /**
   * Constructs a horizontal color selector which
   * contains a gradient between white and black.
   */
  KGradientSelector( TQWidget *parent=0, const char *name=0 );
  /**
   * Constructs a colors selector with orientation o which
   * contains a gradient between white and black.
   */
  KGradientSelector( Orientation o, TQWidget *parent=0, const char *name=0 );
  /**
   * Destructs the widget.
   */
  ~KGradientSelector();
  /**
   * Sets the two colors which span the gradient.
   */
  void setColors( const TQColor &col1, const TQColor &col2 )
  {	color1 = col1; color2 = col2; update();}
  void setText( const TQString &t1, const TQString &t2 )
  {	text1 = t1; text2 = t2; update(); }

  /**
   * Set each color on its own.
   */
  void setFirstColor( const TQColor &col )
  { color1 = col; update(); }
  void setSecondColor( const TQColor &col )
  { color2 = col; update(); }

  /**
   * Set each description on its own
   */
  void setFirstText( const TQString &t )
  { text1 = t; update(); }
  void setSecondText( const TQString &t )
  { text2 = t; update(); }

  const TQColor firstColor() const
  { return color1; }
  const TQColor secondColor() const
  { return color2; }

  const TQString firstText() const
  { return text1; }
  const TQString secondText() const
  { return text2; }

protected:

  virtual void drawContents( TQPainter * );
  virtual TQSize minimumSize() const
  { return sizeHint(); }

private:
  void init();
  TQColor color1;
  TQColor color2;
  TQString text1;
  TQString text2;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KGradientSelectorPrivate;
  KGradientSelectorPrivate *d;
};


#endif		// __TDESELECT_H__


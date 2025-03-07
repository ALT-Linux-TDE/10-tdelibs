/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>
   Copyright (C) 2005 Dominik Haumann <dhdev@gmx.de>

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

/*
   IMPORTANT Note: This file compiles also in Qt-only mode by using the NO_KDE2 precompiler definition
*/

#ifndef KDOCKWIDGET_PRIVATE_H
#define KDOCKWIDGET_PRIVATE_H

#include <tqwidget.h>
#include <tqpushbutton.h>

#ifndef NO_KDE2
#include <netwm_def.h>
#endif

class TQFrame;
class KDockContainer;


/**
 * Like TQSplitter but specially designed for dockwidgets stuff.
 * @internal
 *
 * @author Max Judin.
 */
class TDEUI_EXPORT KDockSplitter : public TQWidget
{
  // NOTE: in theory the TDEUI_EXPORT above shouldn't be there, but it's needed for kexi,
  // which copies the whole definition of the class to be able to access separatorPosInPercent etc. 
  // This needs real fixing in KDE4.
  
  TQ_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent widget
   * @param name name
   * @param orient orientation. Either @p Vertical or @p Horizontal
   * @param pos procentual position of the splitter. Must be int [0...100].
   */
  KDockSplitter(TQWidget *parent= 0, const char *name= 0, Orientation orient= TQt::Vertical, int pos= 50);
  virtual ~KDockSplitter(){}

  /**
   * Initialize the splitter. If @p c0 or @p c1 is 0L the child will not
   * be replaced. So if you want to change @p c1 and not change c0, you'd
   * call @p activate(0L,new_widget);
   *
   * @param c0 the widget on top/left
   * @param c1 the widget on borrom/right
   */
  void activate(TQWidget *c0, TQWidget *c1 = 0L);
  /**
   * Disables the splitter.
   */
  void deactivate();

  /**
   * Return the separator position in percent (%), so the range is [0..100]
   * @return separator position in percent
   */
  int separatorPosInPercent();
  /**
   * Set the separator position in percent (%), so the range must be [0..100]
   * @param percent separator position in percent
   */
  void setSeparatorPosInPercent(int percent);

  /**
   * Return the separator position in the range [0..100000]
   * To get the separator position in procent (%), call
   * @p separatorPositionInPercent()!
   *
   * @return high resolution separator position in range [0..100000],
   *         where 100000 is 100%.
   */
  int separatorPos() const;
  /**
   * set separator position.
   * @param pos the separator position in range [0..100000]. 100000 is 100%.
   * @param do_resize if this is true, then a resize event is generated.
   *                  This may cause the size to change.
   */
  void setSeparatorPos(int pos, bool do_resize = true);
  /**
   * For usage from outside.
   * If the splitter is in fixed position when called,
   * the value of @p pos will be saved and used when the splitter
   * is restored.
   * If @p do_resize is true, the size will be changed unless the splitter
   * is in fixed mode.
   */
  // ### please come up with a nicer name
  void setSeparatorPosX(int pos, bool do_resize=false);

  /**
   * The eventfilter installed on the @p divider processes
   * all splitter resizing events.
   */
  virtual bool eventFilter(TQObject *, TQEvent *);
  virtual bool event( TQEvent * );

  /**
   * @return the top/left child widget.
   */
  TQWidget* getFirst() const { return child0; }
  /**
   * @return the bottom/right child widget.
   */
  TQWidget* getLast() const { return child1; }
  /**
   * If @p w is child0, return child1, otherwise child0.
   * @return the other child widget
   */
  TQWidget* getAnother( TQWidget* w ) const;
  void updateName();

  /**
   * Set opaque flag.
   * @param b if true, both child widgets are resized immediately,
   *          if false, the widgets only resize on MouseUpEvent.
   */
  void setOpaqueResize(bool b=true);
  bool opaqueResize() const;

  /**
   * If @p b is true, the splitter will keep its size on resize events.
   * If no @p KDockContainer is around, always the left child0 will be fixed size.
   */
  void setKeepSize(bool b=true);
  bool keepSize() const;


  void setForcedFixedWidth(KDockWidget *dw,int w);
  void setForcedFixedHeight(KDockWidget *dw,int h);
  void restoreFromForcedFixedSize(KDockWidget *dw);

  /**
   * The orientation is either @p Horizontal or @p Vertical.
   */
  Orientation orientation(){return m_orientation;}

protected:
  friend class  KDockContainer;
  /**
   * Make sure the splitter position is not out of bounds.
   * @param position the current position
   * @return a (new) valid splitter position.
   */
  int checkValue(int position) const;
  /**
   * Make sure the splitter position is not out of bounds. It has
   * to honor all child widgets' mimimumSize.
   * @param position current divider position
   * @param child the overlapping child
   * @return the (new) splitter position.
   */
  int checkValueOverlapped(int position, TQWidget* child) const;

  /**
   * The resize event resizes @p child0, @p child1 and the @p divider.
   * The new sizes are dependant of
   *   - whether @p child0 or @p child1 is a KDockContainer
   *   - the current mode which may be
   *     - Closed
   *     - Overlapped (opened)
   *     - Nonoverlap (opened)
   *     .
   *   .
   * So there are 3*2=6 different modes we have to face.
   * @param ev the resize Event. If @p ev=0L the user changed
   *        the mode (for example from overlap to nonoverlap mode).
   */
  virtual void resizeEvent(TQResizeEvent *ev);

/*
protected slots:
  void delayedResize();*/

private:
  /**
   * updates the minimum and maximun sizes for the KDockSplitter.
   * The sizes depend on the minimum and maximum sizes of the two child
   * widgets.
   */
  void setupMinMaxSize();
  /**
   * child0 and child1 contain the embeded widgets. They are always valid
   * so no need to make pointer checks.
   * child[01]->getWidget() may be KDockContainer.
   */
  TQWidget *child0, *child1;
  Orientation m_orientation;
  /**
   * If initialised is true, the divider!=0L. If false, the divider==0L!
   */
  bool initialised;
  /**
   * The splitter controller which is between child0 and child1.
   * Its size is 4 pixel.
   */
  TQFrame* divider;
  /**
   * @p xpos and @p savedXPos represent the current divider position.
   * If the orientation is Horizontal @p xpos actually is "ypos". So
   * do not get confused only because of the 'x'.
   *
   * xpos and savedXPos are internally high resolution. So *not* 0..100%
   * but 0..100000=100%. This fixes rounding bugs. In fact, this should
   * be a double, but due to binary compatibility we can not change this
   * as we would have to change it in all kdockwidgets.
   */
  int xpos, savedXPos;
  bool mOpaqueResize, mKeepSize;
  int fixedWidth0,fixedWidth1;
  int fixedHeight0,fixedHeight1;
  bool m_dontRecalc;
  /**
   * resolution factor, 0 = 0%, 100000=100%
   */
  static const int factor = 100000;
};

/**
 * A mini-button usually placed in the dockpanel.
 * @internal (but used by tdemdi/dockcontainer.cpp)
 *
 * @author Max Judin.
*/
class TDEUI_EXPORT KDockButton_Private : public TQPushButton
{
  TQ_OBJECT
public:
  KDockButton_Private( TQWidget *parent=0, const char *name=0 );
  ~KDockButton_Private();

protected:
  virtual void drawButton( TQPainter * );
  virtual void enterEvent( TQEvent * );
  virtual void leaveEvent( TQEvent * );

private:
  bool moveMouse;
};

/**
 * resizing enum
 **/



/**
 * additional KDockWidget stuff (private)
*/
class KDockWidgetPrivate : public TQObject
{
  TQ_OBJECT
public:
  KDockWidgetPrivate();
  ~KDockWidgetPrivate();

public slots:
  /**
   * Especially used for Tab page docking. Switching the pages requires additional setFocus() for the embedded widget.
   */
  void slotFocusEmbeddedWidget(TQWidget* w = 0L);

public:
 enum KDockWidgetResize
{ResizeLeft,ResizeTop,ResizeRight,ResizeBottom,ResizeBottomLeft,ResizeTopLeft,ResizeBottomRight,ResizeTopRight};

  int index;
  int splitPosInPercent;
  bool pendingFocusInEvent;
  bool blockHasUndockedSignal;
  bool pendingDtor;
  int forcedWidth;
  int forcedHeight;
  bool isContainer;

#ifndef NO_KDE2
  NET::WindowType windowType;
#endif

  TQWidget *_parent;
  bool transient;

  TQGuardedPtr<TQWidget> container;

  TQPoint resizePos;
  bool resizing;
  KDockWidgetResize resizeMode;
};

class KDockWidgetHeaderPrivate
   : public TQObject
{
public:
  KDockWidgetHeaderPrivate( TQObject* parent )
        : TQObject( parent )
  {
    forceCloseButtonHidden=false;
    toDesktopButton = 0;
    showToDesktopButton = true;
    topLevel = false;
    dummy=0;
  }
  KDockButton_Private* toDesktopButton;

  bool showToDesktopButton;
  bool topLevel;
  TQPtrList<KDockButton_Private> btns;
  bool forceCloseButtonHidden;
  TQWidget *dummy;
};

#endif

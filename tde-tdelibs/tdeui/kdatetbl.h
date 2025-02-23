/*
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
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
#ifndef KDATETBL_H
#define KDATETBL_H

// KDE4: rename this file to kdatetable.h

#include <tqvalidator.h>
#include <tqgridview.h>
#include <tqlineedit.h>
#include <tqdatetime.h>
#include <tqcolor.h>

#include <tdelibs_export.h>

class TDEPopupMenu;

/** Week selection widget.
* @internal
* @version $Id$
* @author Stephan Binner
*/
class TDEUI_EXPORT KDateInternalWeekSelector : public TQLineEdit
{
  TQ_OBJECT
protected:
  TQIntValidator *val;
  int result;
public slots:
  void weekEnteredSlot();
  void setMaxWeek(int max);
signals:
  void closeMe(int);
public:
  KDateInternalWeekSelector( TQWidget* parent=0, const char* name=0);
  int getWeek();
  void setWeek(int week);

private:
  class KDateInternalWeekPrivate;
  KDateInternalWeekPrivate *d;
};

/**
* A table containing month names. It is used to pick a month directly.
* @internal
* @version $Id$
* @author Tim Gilman, Mirko Boehm
*/
class TDEUI_EXPORT KDateInternalMonthPicker : public TQGridView
{
  TQ_OBJECT
protected:
  /**
   * Store the month that has been clicked [1..12].
   */
  int result;
  /**
   * the cell under mouse cursor when LBM is pressed
   */
  short int activeCol;
  short int activeRow;
  /**
   * Contains the largest rectangle needed by the month names.
   */
  TQRect max;
signals:
  /**
   * This is send from the mouse click event handler.
   */
  void closeMe(int);
public:
  /**
   * The constructor.
   */
  KDateInternalMonthPicker(const TQDate& date, TQWidget* parent, const char* name=0);
  /**
   * The destructor.
   */
  ~KDateInternalMonthPicker();
  /**
   * The size hint.
   */
  TQSize sizeHint() const;
  /**
   * Return the result. 0 means no selection (reject()), 1..12 are the
   * months.
   */
  int getResult() const;
protected:
  /**
   * Set up the painter.
   */
  void setupPainter(TQPainter *p);
  /**
   * The resize event.
   */
  virtual void viewportResizeEvent(TQResizeEvent*);
  /**
   * Paint a cell. This simply draws the month names in it.
   */
  virtual void paintCell(TQPainter* painter, int row, int col);
  /**
   * Catch mouse click and move events to paint a rectangle around the item.
   */
  virtual void contentsMousePressEvent(TQMouseEvent *e);
  virtual void contentsMouseMoveEvent(TQMouseEvent *e);
  /**
   * Emit monthSelected(int) when a cell has been released.
   */
  virtual void contentsMouseReleaseEvent(TQMouseEvent *e);

private:
  class KDateInternalMonthPrivate;
  KDateInternalMonthPrivate *d;
};

/** Year selection widget.
* @internal
* @version $Id$
* @author Tim Gilman, Mirko Boehm
*/
class TDEUI_EXPORT KDateInternalYearSelector : public TQLineEdit
{
  TQ_OBJECT
protected:
  TQIntValidator *val;
  int result;
public slots:
  void yearEnteredSlot();
signals:
  void closeMe(int);
public:
  KDateInternalYearSelector( TQWidget* parent=0, const char* name=0);
  int getYear();
  void setYear(int year);

private:
  class KDateInternalYearPrivate;
  KDateInternalYearPrivate *d;

};

/**
 * Frame with popup menu behavior.
 * @author Tim Gilman, Mirko Boehm
 * @version $Id$
 */
class TDEUI_EXPORT TDEPopupFrame : public TQFrame
{
  TQ_OBJECT
protected:
  /**
   * The result. It is returned from exec() when the popup window closes.
   */
  int result;
  /**
   * Catch key press events.
   */
  virtual void keyPressEvent(TQKeyEvent* e);
  /**
   * The only subwidget that uses the whole dialog window.
   */
  TQWidget *main;
public slots:
  /**
   * Close the popup window. This is called from the main widget, usually.
   * @p r is the result returned from exec().
   */
  void close(int r);
  /**
   * Hides the widget. Reimplemented from TQWidget
   */
  void hide();

public:
  /**
   * The contructor. Creates a dialog without buttons.
   */
  TDEPopupFrame(TQWidget* parent=0, const char*  name=0);
  /**
   * The destructor.
   */
  ~TDEPopupFrame();
  /**
   * Set the main widget. You cannot set the main widget from the constructor,
   * since it must be a child of the frame itselfes.
   * Be careful: the size is set to the main widgets size. It is up to you to
   * set the main widgets correct size before setting it as the main
   * widget.
   */
  void setMainWidget(TQWidget* m);
  /**
   * The resize event. Simply resizes the main widget to the whole
   * widgets client size.
   */
  virtual void resizeEvent(TQResizeEvent*);
  /**
   * Open the popup window at position pos.
   */
  void popup(const TQPoint &pos);
  /**
   * Execute the popup window.
   */
  int exec(TQPoint p); // KDE4: const TQPoint&
  /**
   * Execute the popup window.
   */
  int exec(int x, int y);

private:

  virtual bool close(bool alsoDelete) { return TQFrame::close(alsoDelete); }
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class TDEPopupFramePrivate;
  TDEPopupFramePrivate *d;
};

/**
* Validates user-entered dates.
*/
class TDEUI_EXPORT KDateValidator : public TQValidator
{
public:
    KDateValidator(TQWidget* parent=0, const char* name=0);
    virtual State validate(TQString&, int&) const;
    virtual void fixup ( TQString & input ) const;
    State date(const TQString&, TQDate&) const;
};

/**
 * Date selection table.
 * This is a support class for the KDatePicker class.  It just
 * draws the calender table without titles, but could theoretically
 * be used as a standalone.
 *
 * When a date is selected by the user, it emits a signal:
 * dateSelected(TQDate)
 *
 * @internal
 * @version $Id$
 * @author Tim Gilman, Mirko Boehm
 */
class TDEUI_EXPORT KDateTable : public TQGridView
{
    TQ_OBJECT
    TQ_PROPERTY( TQDate date READ getDate WRITE setDate )
    TQ_PROPERTY( bool popupMenu READ popupMenuEnabled WRITE setPopupMenuEnabled )

public:
    /**
     * The constructor.
     */
    KDateTable(TQWidget *parent=0, TQDate date=TQDate::currentDate(),
	       const char* name=0, WFlags f=0);

    /**
     * The constructor.
     * @since 3.4
     */
    KDateTable(TQWidget *parent, const char* name, WFlags f=0);

    /**
     * The destructor.
     */
    ~KDateTable();

    /**
     * Returns a recommended size for the widget.
     * To save some time, the size of the largest used cell content is
     * calculated in each paintCell() call, since all calculations have
     * to be done there anyway. The size is stored in maxCell. The
     * sizeHint() simply returns a multiple of maxCell.
     */
    virtual TQSize sizeHint() const;
    /**
     * Set the font size of the date table.
     */
    void setFontSize(int size);
    /**
     * Select and display this date.
     */
    bool setDate(const TQDate&);
    // ### KDE 4.0 rename to date()
    const TQDate& getDate() const;

    /**
     * Enables a popup menu when right clicking on a date.
     *
     * When it's enabled, this object emits a aboutToShowContextMenu signal
     * where you can fill in the menu items.
     *
     * @since 3.2
     */
    void setPopupMenuEnabled( bool enable );

    /**
     * Returns if the popup menu is enabled or not
     */
    bool popupMenuEnabled() const;

    enum BackgroundMode { NoBgMode=0, RectangleMode, CircleMode };

    /**
     * Makes a given date be painted with a given foregroundColor, and background
     * (a rectangle, or a circle/ellipse) in a given color.
     *
     * @since 3.2
     */
    void setCustomDatePainting( const TQDate &date, const TQColor &fgColor, BackgroundMode bgMode=NoBgMode, const TQColor &bgColor=TQColor());

    /**
     * Unsets the custom painting of a date so that the date is painted as usual.
     *
     * @since 3.2
     */
    void unsetCustomDatePainting( const TQDate &date );

protected:
    /**
     * calculate the position of the cell in the matrix for the given date. The result is the 0-based index.
     */
    int posFromDate( const TQDate &date ); // KDE4: make this virtual, so subclasses can reimplement this and use a different default for the start of the matrix
    /**
     * calculate the date that is displayed at a given cell in the matrix. pos is the
     * 0-based index in the matrix. Inverse function to posForDate().
     */
    TQDate dateFromPos( int pos ); // KDE4: make this virtual

    /**
     * Paint a cell.
     */
    virtual void paintCell(TQPainter*, int, int);

    /**
     * Paint the empty area (background).
     */
    virtual void paintEmptyArea(TQPainter*, int, int, int, int);

    /**
     * Handle the resize events.
     */
    virtual void viewportResizeEvent(TQResizeEvent *);
    /**
     * React on mouse clicks that select a date.
     */
    virtual void contentsMousePressEvent(TQMouseEvent *);
    virtual void wheelEvent( TQWheelEvent * e );
    virtual void keyPressEvent( TQKeyEvent *e );
    virtual void focusInEvent( TQFocusEvent *e );
    virtual void focusOutEvent( TQFocusEvent *e );

    // ### KDE 4.0 make the following private and mark as members

    /**
     * The font size of the displayed text.
     */
    int fontsize;
    /**
     * The currently selected date.
     */
    TQDate date;
    /**
     * The day of the first day in the month [1..7].
     */
    int firstday;
    /**
     * The number of days in the current month.
     */
    int numdays;
    /**
     * The number of days in the previous month.
     */
    int numDaysPrevMonth;
    /**
     * unused
     * ### remove in KDE 4.0
     */
    bool unused_hasSelection;
    /**
     * Save the size of the largest used cell content.
     */
    TQRect maxCell;
signals:
    /**
     * The selected date changed.
     */
    // ### KDE 4.0 make parameter a const reference
    void dateChanged(TQDate);
    /**
     * This function behaves essentially like the one above.
     * The selected date changed.
     * @param cur The current date
     * @param old The date before the date was changed
     */
    void dateChanged(const TQDate& cur, const TQDate& old);
    /**
     * A date has been selected by clicking on the table.
     */
    void tableClicked();

    /**
     * A popup menu for a given date is about to be shown (as when the user
     * right clicks on that date and the popup menu is enabled). Connect
     * the slot where you fill the menu to this signal.
     *
     * @since 3.2
     */
    void aboutToShowContextMenu( TDEPopupMenu * menu, const TQDate &date);

private slots:
  void nextMonth();
  void previousMonth();
  void beginningOfMonth();
  void endOfMonth();
  void beginningOfWeek();
  void endOfWeek();

protected:
  virtual void virtual_hook( int id, void* data );
private:
    class KDateTablePrivate;
    KDateTablePrivate *d;
  
  void initAccels();
};

#endif // KDATETBL_H

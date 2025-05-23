/* This file is part of the KDE libraries

   Copyright (c) 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2000 Stefan Schimanski <1Stein@gmx.de>
                 2000,2001,2002,2003,2004 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
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

#ifndef TDECOMPLETIONBOX_H
#define TDECOMPLETIONBOX_H

class TQEvent;
#include <tqstringlist.h>
#include <tdelistbox.h>

/**
 * @short A helper widget for "completion-widgets" (KLineEdit, KComboBox))
 *
 * A little utility class for "completion-widgets", like KLineEdit or
 * KComboBox. TDECompletionBox is a listbox, displayed as a rectangle without
 * any window decoration, usually directly under the lineedit or combobox.
 * It is filled with all possible matches for a completion, so the user
 * can select the one he wants.
 *
 * It is used when TDEGlobalSettings::Completion == CompletionPopup or CompletionPopupAuto.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class TDEUI_EXPORT TDECompletionBox : public TDEListBox
{
    TQ_OBJECT
    TQ_PROPERTY( bool isTabHandling READ isTabHandling WRITE setTabHandling )
    TQ_PROPERTY(TQString cancelledText READ cancelledText WRITE setCancelledText)
    TQ_PROPERTY( bool activateOnSelect READ activateOnSelect WRITE setActivateOnSelect )

public:
    /**
     * Constructs a TDECompletionBox.
     *
     * The parent widget is used to give the focus back when pressing the
     * up-button on the very first item.
     */
    TDECompletionBox( TQWidget *parent, const char *name = 0 );

    /**
     * Destroys the box
     */
    ~TDECompletionBox();

    virtual TQSize sizeHint() const;

    /**
     * @returns true if selecting an item results in the emition of the selected signal.
     *
     * @since 3.4.1
     */
    bool activateOnSelect() const;

public slots:
    /**
     * Returns a list of all items currently in the box.
     */
    TQStringList items() const;

    /**
     * Inserts @p items into the box. Does not clear the items before.
     * @p index determines at which position @p items will be inserted.
     * (defaults to appending them at the end)
     */
    void insertItems( const TQStringList& items, int index = -1 );

    /**
     * Clears the box and inserts @p items.
     */
    void setItems( const TQStringList& items );

    /**
     * Adjusts the size of the box to fit the width of the parent given in the
     * constructor and pops it up at the most appropriate place, relative to
     * the parent.
     *
     * Depending on the screensize and the position of the parent, this may
     * be a different place, however the default is to pop it up and the
     * lower left corner of the parent.
     *
     * Make sure to hide() the box when appropriate.
     */
    virtual void popup();

    /**
     * Makes this widget (when visible) capture Tab-key events to traverse the
     * items in the dropdown list.
     *
     * Default off, as it conflicts with the usual behavior of Tab to traverse
     * widgets. It is useful for cases like Konqueror's Location Bar, though.
     *
     * @see isTabHandling
     */
    void setTabHandling( bool enable );

    /**
     * @returns true if this widget is handling Tab-key events to traverse the
     * items in the dropdown list, otherwise false.
     *
     * Default is false.
     *
     * @see setTabHandling
     */
    bool isTabHandling() const;

    /**
     * Sets the text to be emitted if the user chooses not to
     * pick from the available matches.
     *
     * If the canceled text is not set through this function, the
     * userCancelled signal will not be emitted.
     *
     * @see userCancelled( const TQString& )
     * @param txt  the text to be emitted if the user cancels this box
     */
    void setCancelledText( const TQString& txt);

    /**
     * @returns the text set via setCancelledText() or TQString::null.
     */
    TQString cancelledText() const;

    /**
     * Set whether or not the selected signal should be emitted when an
     * item is selected. By default the selected signal is emitted.
     *
     * @param state false if the signal should not be emitted.
     * @since 3.4.1
     */
    void setActivateOnSelect(bool state);


    /**
     * Moves the selection one line down or select the first item if nothing is selected yet.
     */
    void down();

    /**
     * Moves the selection one line up or select the first item if nothing is selected yet.
     */
    void up();

    /**
     * Moves the selection one page down.
     */
    void pageDown();

    /**
     * Moves the selection one page up.
     */
    void pageUp();

    /**
     * Moves the selection up to the first item.
     */
    void home();

    /**
     * Moves the selection down to the last item.
     */
    void end();

    /**
     * Re-implemented for internal reasons.  API is unaffected.
     */
    virtual void show();

    /**
     * Re-implemented for internal reasons.  API is unaffected.
     */
    virtual void hide();

signals:
    /**
     * Emitted when an item was selected, contains the text of
     * the selected item.
     */
    void activated( const TQString& );

    /**
     * Emitted whenever the user chooses to ignore the available
     * selections and close the this box.
     */
    void userCancelled( const TQString& );

protected:
    /**
     * This calculates the size of the dropdown and the relative position of the top
     * left corner with respect to the parent widget. This matches the geometry and position
     * normally used by K/TQComboBox when used with one.
     */
    TQRect calculateGeometry() const;

    /**
     * This properly sizes and positions the listbox.
     */
    void sizeAndPosition();

    /**
     * Reimplemented from TDEListBox to get events from the viewport (to hide
     * this widget on mouse-click, Escape-presses, etc.
     */
    virtual bool eventFilter( TQObject *, TQEvent * );

protected slots:
    /**
     * Called when an item was activated. Emits
     * activated() with the item.
     */
    virtual void slotActivated( TQListBoxItem * );

private slots:
    void slotSetCurrentItem( TQListBoxItem *i ) { setCurrentItem( i ); } // grrr
    void slotCurrentChanged();
    void canceled();
    void slotItemClicked( TQListBoxItem * );

protected:
    virtual void virtual_hook( int id, void* data );

private:
    class TDECompletionBoxPrivate;
    TDECompletionBoxPrivate* const d;
};


#endif // TDECOMPLETIONBOX_H

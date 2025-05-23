/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 2002 Hamish Rodda <rodda@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */
#ifndef _KTIMERDIALOG_H_
#define _KTIMERDIALOG_H_

#include <kdialogbase.h>

class TQTimer;
class TQHBox;
class TQProgressBar;
class TQLabel;

/**
 * Provides a dialog that is only available for a specified amount
 * of time, and reports the time remaining to the user.
 *
 * The timer is capable of counting up or down, for any number of milliseconds.
 * 
 * The button which is activated upon timeout can be specified, as can the
 * update interval for the dialog box.
 *
 * In addition, this class retains all of the functionality of @see KDialogBase .
 *
 * @short A dialog with a time limit and corresponding UI features.
 * @author Hamish Rodda <rodda@kde.org>
 */
class KTimerDialog : public KDialogBase
{
  TQ_OBJECT

  public:

    /**
     * @li @p CountDown - The timer counts downwards from the seconds given.
     * @li @p CountUp - The timer counts up to the number of seconds given.
     * @li @p Manual - The timer is not invoked; the caller must update the
     * progress.
     */
    enum TimerStyle
    {
        CountDown,
        CountUp,
        Manual
    };

    /**
     * Constructor for the standard mode where you must specify the main
     * widget with @ref setMainWidget() . See @see KDialogBase for further details.
     *
     * For the rest of the arguments, See @see KDialogBase .
     */
    KTimerDialog( int msec, TimerStyle style=CountDown, TQWidget *parent=0,
                 const char *name=0, bool modal=true,
                 const TQString &caption=TQString::null,
                 int buttonMask=Ok|Apply|Cancel, ButtonCode defaultButton=Ok,
                 bool separator=false,
                 const KGuiItem &user1=KGuiItem(),
                 const KGuiItem &user2=KGuiItem(),
                 const KGuiItem &user3=KGuiItem() );

    /**
     * Destructor.
     */
    ~KTimerDialog();

    /**
     * Execute the dialog modelessly - see @see TQDialog .
     */
    virtual void show();

    /**
     * Set the refresh interval for the timer progress. Defaults to one second.
     */
    void setRefreshInterval( int msec );

    /**
     * Retrieves the @ref ButtonCode which will be activated once the timer
     * times out. @see setTimeoutButton
     */
    int timeoutButton() const;

    /**
     * Sets the @ref ButtonCode to determine which button will be activated
     * once the timer times out. @see timeoutButton
     */
    void setTimeoutButton( ButtonCode newButton );

    /**
     * Retrieves the current @ref TimerStyle. @see setTimerStyle
     */
    int timerStyle() const;

    /**
     * Sets the @ref TimerStyle. @see timerStyle
     */
    void setTimerStyle( TimerStyle newStyle );

    /**
     * Overridden function which is used to set the main widget of the dialog.
     * @see KDialogBase::setMainWidget.
     */
    void setMainWidget( TQWidget *widget );

  signals:
    /**
     * Signal which is emitted once the timer has timed out.
     */
    void timerTimeout();

  public slots:
    /**
     * Execute the dialog modally - see @see TQDialog .
     */
    int exec();

  private slots:
    /**
     * Updates the dialog with the current progress levels.
     */
    void slotUpdateTime( bool update = true );

    /**
     * The internal
     */
    void slotInternalTimeout();

  private:
    /**
     * Prepares the layout that manages the widgets of the dialog
     */
    void setupLayout();

    TQTimer *totalTimer;
    TQTimer *updateTimer;
    int msecRemaining, updateInterval, msecTotal;

    ButtonCode buttonOnTimeout;
    TimerStyle tStyle;
    
    TQHBox *timerWidget;
    TQProgressBar *timerProgress;
    TQLabel *timerLabel;
    TQVBox *mainWidget;

    class KTimerDialogPrivate;
    KTimerDialogPrivate *d;
};

#endif




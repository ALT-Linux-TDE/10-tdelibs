/*
  Copyright (C) 2000,2002 Carsten Pfeiffer <pfeiffer@kde.org>
  Copyright (C) 2002 Neil Stevens <neil@qualityassistant.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation;

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library,  If not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KNOTIFYDIALOG_H
#define KNOTIFYDIALOG_H

#include <tdelistview.h>
#include <kdialogbase.h>
#include <kinstance.h>
#include <tdeglobal.h>

#include "knotifywidgetbase.h"

class TQShowEvent;

namespace KNotify
{
    class KNotifyWidget;
}

/**
 * KNotifyDialog presents an interface for configuring an application's
 * KNotify events.
 *
 * Rather than requiring the user to wade through the entire list of
 * applications' events in KControl, your application can make the list
 * of its own notifications available here.
 *
 * Typical usage is calling the static configure() method:
 * \code
 * (void) KNotifyDialog::configure( someParentWidget );
 * \endcode
 *
 * @since 3.1
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class TDEIO_EXPORT KNotifyDialog : public KDialogBase
{
    TQ_OBJECT

public:
    /**
     * If you want a non-modal dialog, you need to instantiate KNotifyDialog
     * yourself instead of using the configure() method.
     *
     * KDE4.0 modal default will be false.
     * 
     * @param parent The parent widget for the dialog
     * @param name The widget name
     * @param modal If true, this will be a modal dialog, otherwise non-modal.
     * @param aboutData A pointer to a TDEAboutData object. TDEAboutData::appName()
     *                  will be used to find the KNotify events (in the eventsrc file).
     *                  Set this to 0L if you want to add all events yourself with
     * addApplicationEvents().
     */
    KNotifyDialog( TQWidget *parent = 0, const char *name = 0,
                   bool modal = true,
                   const TDEAboutData *aboutData =
                   TDEGlobal::instance()->aboutData() );
    /**
     * Destroys the KNotifyDialog
     */
    virtual ~KNotifyDialog();

    /**
     * Convenience method to create exec() a modal KNotifyDialog.
     *
     * @param parent The parent widget for the dialog
     * @param name The widget name
     * @param aboutData A pointer to a TDEAboutData object. TDEAboutData::appName()
     *                  will be used to find the KNotify events (in the eventsrc file).
     * @see exec for the return values.
     * @return The value of TQDialog::exec()
     */
    static int configure( TQWidget *parent = 0, const char *name = 0,
                          const TDEAboutData *aboutData = TDEGlobal::instance()->aboutData() );

    /**
     * With this method, you can add the KNotify events of one eventsrc
     * files to the view.
     * KNotifyDialog can handle events for multiple applications (i.e. eventsrc files).
     * Successive calls with a different @p appName will add them.
     * @param appName The application's name, i.e. the name passed to the 
     *                TDEApplication constructor or TDEAboutData.
     * @see clearApplicationEvents()
     */
    virtual void addApplicationEvents( const char *appName );

    /**
     * With this method, you can add the KNotify events of one eventsrc
     * files to the view.
     * KNotifyDialog can handle events for multiple applications (i.e. eventsrc files).
     * Successive calls with a different @p path will add them.
     * @param path The absolute or relative path to the eventsrc file to be configured.
     *             A relative path would be e.g. "twin/eventsrc".
     * @see clearApplicationEvents()
     */
    virtual void addApplicationEvents( const TQString& path );

    /**
     * Removes all the events added with addApplicationEvents()
     * @see addApplicationEvents()
     */
    virtual void clearApplicationEvents();

private slots:
    void slotDefault();

private:
    enum
    {
        COL_FILENAME = 1
    };

    void updateView();

    KNotify::KNotifyWidget * m_notifyWidget;

    class Private;
    Private *d;
};


namespace KNotify
{
    class Application;
    class Event;
    class ListViewItem;
    typedef TQPtrList<Event> EventList;
    typedef TQPtrListIterator<Application> ApplicationListIterator;
    typedef TQPtrListIterator<Event> EventListIterator;

    /**
     * @internal
     */
    class TDEIO_EXPORT Application
    {
    public:
        Application( const TQString &path );
        ~Application();

        TQString text() const { return m_description; }
        TQString icon() const { return m_icon; }
        const EventList& eventList();
        void reloadEvents( bool revertToDefaults = false );
        void save();

        TQString appName() const { return m_appname; }

    private:
        TQString m_icon;
        TQString m_description;
        TQString m_appname;
        EventList *m_events;

        TDEConfig *kc; // The file that defines the events.
        TDEConfig *config; // The file that contains the settings for the events
    };


    class TDEIO_EXPORT ApplicationList : public TQPtrList<Application>
    {
        virtual int compareItems ( TQPtrCollection::Item item1,
                                   TQPtrCollection::Item item2 )
        {
            return (static_cast<Application*>( item1 )->text() >=
                static_cast<Application*>( item2 )->text()) ? 1 : -1;
        }
    };

    /**
     * @internal
     */
    class TDEIO_EXPORT KNotifyWidget : public KNotifyWidgetBase
    {
        TQ_OBJECT

    public:
        KNotifyWidget( TQWidget* parent = 0, const char* name = 0,
                       bool handleAllApps = false );
        ~KNotifyWidget();

        TDEListView * eventsView() {
            return m_listview;
        }

        void addVisibleApp( Application *app );
        ApplicationList& visibleApps() { return m_visibleApps; }
        ApplicationList& allApps() { return m_allApps; }

        /**
         * Returns 0L if no application events could be found
         * The returned pointer must be freed by the caller (easiest done
         * by putting it into an ApplicationList with setAutoDelete( true )).
         */
        Application * addApplicationEvents( const TQString& path );

        void resetDefaults( bool ask );
        void sort( bool ascending = true );

    public slots:
        /**
         * Clears the view and all the Application events.
         */
        virtual void clear();
        /**
         * Clears only the view and the visible Application events.
         * E.g. useful if you want to set new visible events with
         * addVisibleApp()
         */
        virtual void clearVisible();
        virtual void save();
        virtual void showAdvanced( bool show );
        void toggleAdvanced();


    signals:
        void changed( bool hasChanges );

    protected:
        /**
         * May return 0L, if there is no current event selected.
         */
        Event * currentEvent();
        virtual void showEvent( TQShowEvent * );
        virtual void enableAll( int what, bool enable );

        void reload( bool revertToDefaults = false );

    protected slots:
        void playSound();

    private slots:
        void slotItemClicked( TQListViewItem *item, const TQPoint& point, 
                              int col );
        void slotEventChanged( TQListViewItem * );
        void soundToggled( bool on );
        void loggingToggled( bool on );
        void executeToggled( bool on );
        void messageBoxChanged();
        void stderrToggled( bool on );
        void taskbarToggled( bool on );

        void soundFileChanged( const TQString& text );
        void logfileChanged( const TQString& text );
        void commandlineChanged( const TQString& text );

        void openSoundDialog( KURLRequester * );
        void openLogDialog( KURLRequester * );
        void openExecDialog( KURLRequester * );

        void enableAll();

    private:
        void updateWidgets( ListViewItem *item );
        void updatePixmaps( ListViewItem *item );

        static TQString makeRelative( const TQString& );
        void addToView( const EventList& events );
        void widgetChanged( TQListViewItem *item,
                            int what, bool on, TQWidget *buddy = 0L );
        void selectItem( TQListViewItem *item );

        ApplicationList m_visibleApps;
        ApplicationList m_allApps;

        class Private;
        Private *d;

    };


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


    /**
     * @internal
     */
    class Event
    {
        friend class Application;

    public:
        TQString text() const { return description; }

        int presentation;
        int dontShow;
        TQString logfile;
        TQString soundfile;
        TQString commandline;

        const Application *application() const { return m_app; }

    private:
        Event( const Application *app ) {
            presentation = 0;
            dontShow = 0;
            m_app = app;
        }
        TQString name;
        TQString description;
        TQString configGroup;

        const Application *m_app;
    };

    /**
     * @internal
     */
    class ListViewItem : public TQListViewItem
    {
    public:
        ListViewItem( TQListView *view, Event *event );

        Event& event() { return *m_event; }
        virtual int compare (TQListViewItem * i, int col, bool ascending) const;

    private:
        Event * m_event;
    };

}


#endif

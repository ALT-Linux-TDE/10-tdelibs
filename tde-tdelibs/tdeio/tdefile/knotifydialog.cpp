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

#include <dcopclient.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kaudioplayer.h>
#include <kcombobox.h>
#include <tdeconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <tdefiledialog.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <knotifyclient.h>
#include <knotifydialog.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <tdeio/netaccess.h>

#include <tqcheckbox.h>
#include <tqgroupbox.h>
#include <tqheader.h>
#include <tqlabel.h>
#include <tqlistview.h>
#include <tqlayout.h>
#include <tqptrlist.h>
#include <tqpushbutton.h>
#include <tqstring.h>
#include <tqtooltip.h>
#include <tqtimer.h>
#include <tqvbox.h>
#include <tqwhatsthis.h>

using namespace KNotify;

enum
{
    COL_EXECUTE = 0,
    COL_STDERR  = 1,
    COL_MESSAGE = 2,
    COL_LOGFILE = 3,
    COL_SOUND   = 4,
    COL_TASKBAR = 5,
    COL_EVENT   = 6
};

//
// I don't feel like subclassing KComboBox and find ways to insert that into
// the .ui file...
//
namespace KNotify
{
    class SelectionCombo
    {
    public:
        //
        // Mind the order in fill() and type()
        //
        static void fill( KComboBox *combo )
        {
            combo->insertItem( i18n("Sounds") );
            combo->insertItem( i18n("Logging") );
            combo->insertItem( i18n("Program Execution") );
            combo->insertItem( i18n("Message Windows") );
            combo->insertItem( i18n("Passive Windows") );
            combo->insertItem( i18n("Standard Error Output") );
            combo->insertItem( i18n("Taskbar") );
        }

        static int type( KComboBox *combo )
        {
            switch( combo->currentItem() )
            {
                case 0:
                    return KNotifyClient::Sound;
                case 1:
                    return KNotifyClient::Logfile;
                case 2:
                    return KNotifyClient::Execute;
                case 3:
                    return KNotifyClient::Messagebox;
                case 4:
                    return KNotifyClient::PassivePopup;
                case 5:
                    return KNotifyClient::Stderr;
                case 6:
                    return KNotifyClient::Taskbar;
            }

            return KNotifyClient::None;
        }
    };

    // Needed for displaying tooltips in the listview's QHeader
    class KNotifyToolTip : public TQToolTip
    {
    public:
        KNotifyToolTip( TQHeader *header )
            : TQToolTip( header )
        {
            m_tips[COL_EXECUTE] = i18n("Execute a program");
            m_tips[COL_STDERR]  = i18n("Print to Standard error output");
            m_tips[COL_MESSAGE] = i18n("Display a messagebox");
            m_tips[COL_LOGFILE] = i18n("Log to a file");
            m_tips[COL_SOUND]   = i18n("Play a sound");
            m_tips[COL_TASKBAR] = i18n("Flash the taskbar entry");
        }
        virtual ~KNotifyToolTip() {}

    protected:
        virtual void maybeTip ( const TQPoint& p )
        {
            TQHeader *header = static_cast<TQHeader*>( parentWidget() );
            int section = 0;

            if ( header->orientation() == TQt::Horizontal )
                section= header->sectionAt( p.x() );
            else
                section= header->sectionAt( p.y() );

            if ( ( section < 0 ) || ( static_cast<uint>( section ) >= (sizeof(m_tips) / sizeof(TQString)) ) )
                return;

            tip( header->sectionRect( section ), m_tips[section] );
        }

    private:
        TQString m_tips[6];
    };

}


int KNotifyDialog::configure( TQWidget *parent, const char *name,
                              const TDEAboutData *aboutData )
{
    KNotifyDialog dialog( parent, name, true, aboutData );
    return dialog.exec();
}

KNotifyDialog::KNotifyDialog( TQWidget *parent, const char *name, bool modal,
                              const TDEAboutData *aboutData )
    : KDialogBase(parent, name, modal, i18n("Notification Settings"),
                  Ok | Apply | Cancel | Default, Ok, true )
{
    TQVBox *box = makeVBoxMainWidget();

    m_notifyWidget = new KNotifyWidget( box, "knotify widget" );

    if ( aboutData )
        addApplicationEvents( aboutData->appName() );

    connect( this, TQ_SIGNAL( okClicked() ), m_notifyWidget, TQ_SLOT( save() ));
    connect( this, TQ_SIGNAL( applyClicked() ), m_notifyWidget, TQ_SLOT( save() ));
}

KNotifyDialog::~KNotifyDialog()
{
}

void KNotifyDialog::addApplicationEvents( const char *appName )
{
    addApplicationEvents( TQString::fromUtf8( appName ) +
                          TQString::fromLatin1( "/eventsrc" ) );
}

void KNotifyDialog::addApplicationEvents( const TQString& path )
{
    Application *app = m_notifyWidget->addApplicationEvents( path );
    if ( app )
    {
        m_notifyWidget->addVisibleApp( app );
        m_notifyWidget->sort();
    }
}

void KNotifyDialog::clearApplicationEvents()
{
    m_notifyWidget->clear();
}

void KNotifyDialog::slotDefault()
{
    m_notifyWidget->resetDefaults( true ); // ask user
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


class KNotifyWidget::Private
{
public:
    TQPixmap pixmaps[6];
    KNotifyToolTip *toolTip;
};

// simple access to all knotify-handled applications
KNotifyWidget::KNotifyWidget( TQWidget *parent, const char *name,
                              bool handleAllApps )
    : KNotifyWidgetBase( parent, name ? name : "KNotifyWidget" )
{
    d = new Private;

    m_allApps.setAutoDelete( true );

    if ( !handleAllApps )
    {
        m_affectAllApps->hide();
        m_playerButton->hide();
    }

    SelectionCombo::fill( m_comboEnable );
    SelectionCombo::fill( m_comboDisable );

    m_listview->setFullWidth( true );
    m_listview->setAllColumnsShowFocus( true );

    TQPixmap pexec = SmallIcon("application-x-executable");
    TQPixmap pstderr = SmallIcon("terminal");
    TQPixmap pmessage = SmallIcon("application-vnd.tde.info");
    TQPixmap plogfile = SmallIcon("text-x-log");
    TQPixmap psound = SmallIcon("audio-x-generic");
    TQPixmap ptaskbar = SmallIcon("kicker");

    d->pixmaps[COL_EXECUTE] = pexec;
    d->pixmaps[COL_STDERR]  = pstderr;
    d->pixmaps[COL_MESSAGE] = pmessage;
    d->pixmaps[COL_LOGFILE] = plogfile;
    d->pixmaps[COL_SOUND]   = psound;
    d->pixmaps[COL_TASKBAR] = ptaskbar;

    int w = TDEIcon::SizeSmall + 6;

    TQHeader *header = m_listview->header();
    header->setLabel( COL_EXECUTE, pexec,    TQString::null, w );
    header->setLabel( COL_STDERR,  pstderr,  TQString::null, w );
    header->setLabel( COL_MESSAGE, pmessage, TQString::null, w );
    header->setLabel( COL_LOGFILE, plogfile, TQString::null, w );
    header->setLabel( COL_SOUND,   psound,   TQString::null, w );
    header->setLabel( COL_TASKBAR, ptaskbar, TQString::null, w );

    d->toolTip = new KNotifyToolTip( header );

    m_playButton->setIconSet( SmallIconSet( "media-playback-start" ) );
    connect( m_playButton, TQ_SIGNAL( clicked() ), TQ_SLOT( playSound() ));

    connect( m_listview, TQ_SIGNAL( currentChanged( TQListViewItem * ) ),
             TQ_SLOT( slotEventChanged( TQListViewItem * ) ));
    connect( m_listview, TQ_SIGNAL(clicked( TQListViewItem *, const TQPoint&, int)),
             TQ_SLOT( slotItemClicked( TQListViewItem *, const TQPoint&, int )));

    connect( m_playSound, TQ_SIGNAL( toggled( bool )),
             TQ_SLOT( soundToggled( bool )) );
    connect( m_logToFile, TQ_SIGNAL( toggled( bool )),
             TQ_SLOT( loggingToggled( bool )) );
    connect( m_execute, TQ_SIGNAL( toggled( bool )),
             TQ_SLOT( executeToggled( bool )) );
    connect( m_messageBox, TQ_SIGNAL( toggled( bool )),
             TQ_SLOT( messageBoxChanged() ) );
    connect( m_passivePopup, TQ_SIGNAL( toggled( bool )),
             TQ_SLOT( messageBoxChanged() ) );
    connect( m_stderr, TQ_SIGNAL( toggled( bool )),
             TQ_SLOT( stderrToggled( bool ) ) );
    connect( m_taskbar, TQ_SIGNAL( toggled( bool )),
             TQ_SLOT( taskbarToggled( bool ) ) );

    connect( m_soundPath, TQ_SIGNAL( textChanged( const TQString& )),
             TQ_SLOT( soundFileChanged( const TQString& )));
    connect( m_logfilePath, TQ_SIGNAL( textChanged( const TQString& )),
             TQ_SLOT( logfileChanged( const TQString& ) ));
    connect( m_executePath, TQ_SIGNAL( textChanged( const TQString& )),
             TQ_SLOT( commandlineChanged( const TQString& ) ));

    connect( m_soundPath, TQ_SIGNAL( openFileDialog( KURLRequester * )),
             TQ_SLOT( openSoundDialog( KURLRequester * )));
    connect( m_logfilePath, TQ_SIGNAL( openFileDialog( KURLRequester * )),
             TQ_SLOT( openLogDialog( KURLRequester * )));
    connect( m_executePath, TQ_SIGNAL( openFileDialog( KURLRequester * )),
             TQ_SLOT( openExecDialog( KURLRequester * )));

    connect( m_extension, TQ_SIGNAL( clicked() ),
             TQ_SLOT( toggleAdvanced()) );

    connect( m_buttonEnable, TQ_SIGNAL( clicked() ), TQ_SLOT( enableAll() ));
    connect( m_buttonDisable, TQ_SIGNAL( clicked() ), TQ_SLOT( enableAll() ));

    TQString whatsThis = i18n("<qt>You may use the following macros<br>"
        "in the commandline:<br>"
        "<b>%e</b>: for the event name,<br>"
        "<b>%a</b>: for the name of the application that sent the event,<br>"
        "<b>%s</b>: for the notification message,<br>"
        "<b>%w</b>: for the numeric window ID where the event originated,<br>"
        "<b>%i</b>: for the numeric event ID.");
    TQWhatsThis::add( m_execute, whatsThis );
    TQWhatsThis::add( m_executePath, whatsThis );
    
    showAdvanced( false );

    slotEventChanged( 0L ); // disable widgets by default
}

KNotifyWidget::~KNotifyWidget()
{
    delete d->toolTip;
    delete d;
}

void KNotifyWidget::toggleAdvanced()
{
    showAdvanced( m_logToFile->isHidden() );
}

void KNotifyWidget::showAdvanced( bool show )
{
    if ( show )
    {
        m_extension->setText( i18n("Advanced <<") );
        TQToolTip::add( m_extension, i18n("Hide advanced options") );

        m_logToFile->show();
        m_logfilePath->show();
        m_execute->show();
        m_executePath->show();
        m_messageBox->show();
        m_passivePopup->show();
        m_stderr->show();
        m_taskbar->show();

	m_passivePopup->setEnabled( m_messageBox->isChecked() );
        m_actionsBoxLayout->setSpacing( KDialog::spacingHint() );
    }
    else
    {
        m_extension->setText( i18n("Advanced >>") );
        TQToolTip::add( m_extension, i18n("Show advanced options") );

        m_logToFile->hide();
        m_logfilePath->hide();
        m_execute->hide();
        m_executePath->hide();
        m_messageBox->hide();
        m_passivePopup->hide();
        m_stderr->hide();
        m_taskbar->hide();

        m_actionsBoxLayout->setSpacing( 0 );
    }
}

Application * KNotifyWidget::addApplicationEvents( const TQString& path )
{
    kdDebug() << "**** knotify: adding path: " << path << endl;
    TQString relativePath = path;

    if ( path.at(0) == '/' && TDEStandardDirs::exists( path ) )
        relativePath = makeRelative( path );

    if ( !relativePath.isEmpty() )
    {
        Application *app = new Application( relativePath );
        m_allApps.append( app );
        return app;
    }

    return 0L;
}

void KNotifyWidget::clear()
{
    clearVisible();
    m_allApps.clear();
}

void KNotifyWidget::clearVisible()
{
    m_visibleApps.clear();
    m_listview->clear();
    slotEventChanged( 0L ); // disable widgets
}

void KNotifyWidget::showEvent( TQShowEvent *e )
{
    selectItem( m_listview->firstChild() );
    KNotifyWidgetBase::showEvent( e );
}

void KNotifyWidget::slotEventChanged( TQListViewItem *item )
{
    bool on = (item != 0L);

    m_actionsBox->setEnabled( on );
    m_controlsBox->setEnabled( on );

    if ( !on )
        return;

    ListViewItem *lit = static_cast<ListViewItem*>( item );
    updateWidgets( lit );
}

void KNotifyWidget::updateWidgets( ListViewItem *item )
{
    bool enable;
    bool checked;

    blockSignals( true ); // don't emit changed() signals

    const Event& event = item->event();

    // sound settings
    m_playButton->setEnabled( !event.soundfile.isEmpty() );
    m_soundPath->setURL( event.soundfile );
    enable = (event.dontShow & KNotifyClient::Sound) == 0;
    checked = enable && !event.soundfile.isEmpty() &&
              (event.presentation & KNotifyClient::Sound);
    m_playSound->setEnabled( enable );
    m_playSound->setChecked( checked );
    m_soundPath->setEnabled( checked );


    // logfile settings
    m_logfilePath->setURL( event.logfile );
    enable = (event.dontShow & KNotifyClient::Logfile) == 0;
    checked = enable && !event.logfile.isEmpty()  &&
              (event.presentation & KNotifyClient::Logfile);
    m_logToFile->setEnabled( enable );
    m_logToFile->setChecked( checked );
    m_logfilePath->setEnabled( checked );


    // execute program settings
    m_executePath->setURL( event.commandline );
    enable = (event.dontShow & KNotifyClient::Execute) == 0;
    checked = enable && !event.commandline.isEmpty() &&
              (event.presentation & KNotifyClient::Execute);
    m_execute->setEnabled( enable );
    m_execute->setChecked( checked );
    m_executePath->setEnabled( checked );


    // other settings
    m_messageBox->setChecked(event.presentation & (KNotifyClient::Messagebox | KNotifyClient::PassivePopup));
    enable = (event.dontShow & KNotifyClient::Messagebox) == 0;
    m_messageBox->setEnabled( enable );

    m_passivePopup->setChecked(event.presentation & KNotifyClient::PassivePopup);
    enable = (event.dontShow & KNotifyClient::PassivePopup) == 0;
    m_passivePopup->setEnabled( enable );

    m_stderr->setChecked( event.presentation & KNotifyClient::Stderr );
    enable = (event.dontShow & KNotifyClient::Stderr) == 0;
    m_stderr->setEnabled( enable );

    m_taskbar->setChecked(event.presentation & KNotifyClient::Taskbar);
    enable = (event.dontShow & KNotifyClient::Taskbar) == 0;
    m_taskbar->setEnabled( enable );

    updatePixmaps( item );

    blockSignals( false );
}

void KNotifyWidget::updatePixmaps( ListViewItem *item )
{
    TQPixmap emptyPix;
    Event &event = item->event();

    bool doIt = (event.presentation & KNotifyClient::Execute) &&
                !event.commandline.isEmpty();
    item->setPixmap( COL_EXECUTE, doIt ? d->pixmaps[COL_EXECUTE] : emptyPix );

    doIt = (event.presentation & KNotifyClient::Sound) &&
           !event.soundfile.isEmpty();
    item->setPixmap( COL_SOUND, doIt ? d->pixmaps[COL_SOUND] : emptyPix );

    doIt = (event.presentation & KNotifyClient::Logfile) &&
           !event.logfile.isEmpty();
    item->setPixmap( COL_LOGFILE, doIt ? d->pixmaps[COL_LOGFILE] : emptyPix );

    item->setPixmap( COL_MESSAGE,
                     (event.presentation &
                      (KNotifyClient::Messagebox | KNotifyClient::PassivePopup)) ?
                     d->pixmaps[COL_MESSAGE] : emptyPix );

    item->setPixmap( COL_STDERR,
                     (event.presentation & KNotifyClient::Stderr) ?
                     d->pixmaps[COL_STDERR] : emptyPix );
    item->setPixmap( COL_TASKBAR,
                     (event.presentation & KNotifyClient::Taskbar) ?
                     d->pixmaps[COL_TASKBAR] : emptyPix );
}

void KNotifyWidget::addVisibleApp( Application *app )
{
    if ( !app || (m_visibleApps.findRef( app ) != -1) )
        return;

    m_visibleApps.append( app );
    addToView( app->eventList() );

    TQListViewItem *item = m_listview->selectedItem();
    if ( !item )
        item = m_listview->firstChild();

    selectItem( item );
}

void KNotifyWidget::addToView( const EventList& events )
{
    ListViewItem *item = 0L;

    EventListIterator it( events );

    for ( ; it.current(); ++it )
    {
        Event *event = it.current();
        item = new ListViewItem( m_listview, event );

        if ( (event->presentation & KNotifyClient::Execute) &&
             !event->commandline.isEmpty() )
            item->setPixmap( COL_EXECUTE, d->pixmaps[COL_EXECUTE] );
        if ( (event->presentation & KNotifyClient::Sound) &&
             !event->soundfile.isEmpty() )
            item->setPixmap( COL_SOUND, d->pixmaps[COL_SOUND] );
        if ( (event->presentation & KNotifyClient::Logfile) &&
             !event->logfile.isEmpty() )
            item->setPixmap( COL_LOGFILE, d->pixmaps[COL_LOGFILE] );
        if ( event->presentation & (KNotifyClient::Messagebox|KNotifyClient::PassivePopup) )
            item->setPixmap( COL_MESSAGE, d->pixmaps[COL_MESSAGE] );
        if ( event->presentation & KNotifyClient::Stderr )
            item->setPixmap( COL_STDERR, d->pixmaps[COL_STDERR] );
        if ( event->presentation & KNotifyClient::Taskbar )
            item->setPixmap( COL_TASKBAR, d->pixmaps[COL_TASKBAR] );
    }
}

void KNotifyWidget::widgetChanged( TQListViewItem *item,
                                   int what, bool on, TQWidget *buddy )
{
    if ( signalsBlocked() )
        return;

    if ( buddy )
        buddy->setEnabled( on );

    Event &e = static_cast<ListViewItem*>( item )->event();
    if ( on )
    {
        e.presentation |= what;
        if ( buddy )
            buddy->setFocus();
    }
    else
        e.presentation &= ~what;

    emit changed( true );
}

void KNotifyWidget::soundToggled( bool on )
{
    TQListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;
    bool doIcon = on && !m_soundPath->url().isEmpty();
    item->setPixmap( COL_SOUND, doIcon ? d->pixmaps[COL_SOUND] : TQPixmap() );
    widgetChanged( item, KNotifyClient::Sound, on, m_soundPath );
}

void KNotifyWidget::loggingToggled( bool on )
{
    TQListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;
    bool doIcon = on && !m_logfilePath->url().isEmpty();
    item->setPixmap(COL_LOGFILE, doIcon ? d->pixmaps[COL_LOGFILE] : TQPixmap());
    widgetChanged( item, KNotifyClient::Logfile, on, m_logfilePath );
}

void KNotifyWidget::executeToggled( bool on )
{
    TQListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;
    bool doIcon = on && !m_executePath->url().isEmpty();
    item->setPixmap(COL_EXECUTE, doIcon ? d->pixmaps[COL_EXECUTE] : TQPixmap());
    widgetChanged( item, KNotifyClient::Execute, on, m_executePath );
}

void KNotifyWidget::messageBoxChanged()
{
    if ( signalsBlocked() )
        return;

    m_passivePopup->setEnabled( m_messageBox->isChecked() );

    TQListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;

    bool on = m_passivePopup->isEnabled();
    item->setPixmap( COL_MESSAGE, on ? d->pixmaps[COL_MESSAGE] : TQPixmap() );

    Event &e = static_cast<ListViewItem*>( item )->event();

    if ( m_messageBox->isChecked() ) {
	if ( m_passivePopup->isChecked() ) {
	    e.presentation |= KNotifyClient::PassivePopup;
	    e.presentation &= ~KNotifyClient::Messagebox;
	}
	else {
	    e.presentation &= ~KNotifyClient::PassivePopup;
	    e.presentation |= KNotifyClient::Messagebox;
	}
    }
    else {
        e.presentation &= ~KNotifyClient::Messagebox;
        e.presentation &= ~KNotifyClient::PassivePopup;
    }

    emit changed( true );
}

void KNotifyWidget::stderrToggled( bool on )
{
    TQListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;
    item->setPixmap( COL_STDERR, on ? d->pixmaps[COL_STDERR] : TQPixmap() );
    widgetChanged( item, KNotifyClient::Stderr, on );
}

void KNotifyWidget::taskbarToggled( bool on )
{
    TQListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;
    item->setPixmap( COL_TASKBAR, on ? d->pixmaps[COL_TASKBAR] : TQPixmap() );
    widgetChanged( item, KNotifyClient::Taskbar, on );
}

void KNotifyWidget::soundFileChanged( const TQString& text )
{
    if ( signalsBlocked() )
        return;

    TQListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;

    m_playButton->setEnabled( !text.isEmpty() );

    currentEvent()->soundfile = text;
    bool ok = !text.isEmpty() && m_playSound->isChecked();
    item->setPixmap( COL_SOUND, ok ? d->pixmaps[COL_SOUND] : TQPixmap() );

    emit changed( true );
}

void KNotifyWidget::logfileChanged( const TQString& text )
{
    if ( signalsBlocked() )
        return;

    TQListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;

    currentEvent()->logfile = text;
    bool ok = !text.isEmpty() && m_logToFile->isChecked();
    item->setPixmap( COL_LOGFILE, ok ? d->pixmaps[COL_LOGFILE] : TQPixmap() );

    emit changed( true );
}

void KNotifyWidget::commandlineChanged( const TQString& text )
{
    if ( signalsBlocked() )
        return;

    TQListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;

    currentEvent()->commandline = text;
    bool ok = !text.isEmpty() && m_execute->isChecked();
    item->setPixmap( COL_EXECUTE, ok ? d->pixmaps[COL_EXECUTE] : TQPixmap() );

    emit changed( true );
}

void KNotifyWidget::slotItemClicked( TQListViewItem *item, const TQPoint&,
                                     int col )
{
    if ( !item || !item->isSelected() )
        return;

    Event *event = currentEvent();
    if ( !event )
        return; // very unlikely, but safety first

    bool doShowAdvanced = false;

    switch( col )
    {
        case COL_EXECUTE:
            m_execute->toggle();
            m_executePath->setFocus();
            doShowAdvanced = true;
            break;
        case COL_STDERR:
            m_stderr->toggle();
            break;
        case COL_TASKBAR:
            m_taskbar->toggle();
            break;
        case COL_MESSAGE:
            m_passivePopup->setChecked( true ); // default to passive popups
            m_messageBox->toggle();
            break;
        case COL_LOGFILE:
            m_logToFile->toggle();
            m_logfilePath->setFocus();
            doShowAdvanced = true;
            break;
        case COL_SOUND:
            m_playSound->toggle();
            break;
        default: // do nothing
            break;
    }

    if ( doShowAdvanced && !m_logToFile->isVisible() )
    {
        showAdvanced( true );
        m_listview->ensureItemVisible( m_listview->currentItem() );
    }
}

void KNotifyWidget::sort( bool ascending )
{
    m_listview->setSorting( COL_EVENT, ascending );
    m_listview->sort();
}

void KNotifyWidget::selectItem( TQListViewItem *item )
{
    if ( item )
    {
        m_listview->setCurrentItem( item );
        item->setSelected( true );
        slotEventChanged( item );
    }
}

void KNotifyWidget::resetDefaults( bool ask )
{
    if ( ask )
    {
        if ( KMessageBox::warningContinueCancel(this,
                                   i18n("This will cause the notifications "
                                        "to be reset to their defaults."),
                                                i18n("Are You Sure?"),
                                                i18n("&Reset"))
             != KMessageBox::Continue)
            return;
    }

    reload( true ); // defaults
    emit changed( true );
}

void KNotifyWidget::reload( bool revertToDefaults )
{
    m_listview->clear();
    ApplicationListIterator it( m_visibleApps );
    for ( ; it.current(); ++it )
    {
        it.current()->reloadEvents( revertToDefaults );
        addToView( it.current()->eventList() );
    }

    m_listview->sort();
    selectItem( m_listview->firstChild()  );
}

void KNotifyWidget::save()
{
    kdDebug() << "save\n";

    ApplicationListIterator it( m_allApps );
    while ( it.current() )
    {
        (*it)->save();
        ++it;
    }

    if ( kapp )
    {
        if ( !kapp->dcopClient()->isAttached() )
            kapp->dcopClient()->attach();
        kapp->dcopClient()->send("knotify", "", "reconfigure()", TQString(""));
    }

    emit changed( false );
}

// returns e.g. "twin/eventsrc" from a given path
// "/opt/trinity/share/apps/twin/eventsrc"
TQString KNotifyWidget::makeRelative( const TQString& fullPath )
{
    int slash = fullPath.findRev( '/' ) - 1;
    slash = fullPath.findRev( '/', slash );

    if ( slash < 0 )
        return TQString::null;

    return fullPath.mid( slash+1 );
}

Event * KNotifyWidget::currentEvent()
{
    TQListViewItem *current = m_listview->currentItem();
    if ( !current )
        return 0L;

    return &static_cast<ListViewItem*>( current )->event();
}

void KNotifyWidget::openSoundDialog( KURLRequester *requester )
{
    // only need to init this once
    requester->disconnect( TQ_SIGNAL( openFileDialog( KURLRequester * )),
                           this, TQ_SLOT( openSoundDialog( KURLRequester * )));

    KFileDialog *fileDialog = requester->fileDialog();
    fileDialog->setCaption( i18n("Select Sound File") );
    TQStringList filters;
    filters << "audio/x-wav" << "audio/x-mp3" << "application/ogg"
            << "audio/x-adpcm";
    fileDialog->setMimeFilter( filters );

    // find the first "sound"-resource that contains files
    const Application *app = currentEvent()->application();
    TQStringList soundDirs =
        TDEGlobal::dirs()->findDirs("data", app->appName() + "/sounds");
    soundDirs += TDEGlobal::dirs()->resourceDirs( "sound" );

    if ( !soundDirs.isEmpty() ) {
        KURL soundURL;
        TQDir dir;
        dir.setFilter( TQDir::Files | TQDir::Readable );
        TQStringList::ConstIterator it = soundDirs.begin();
        while ( it != soundDirs.end() ) {
            dir = *it;
            if ( dir.isReadable() && dir.count() > 2 ) {
                soundURL.setPath( *it );
                fileDialog->setURL( soundURL );
                break;
            }
            ++it;
        }
    }
}

void KNotifyWidget::openLogDialog( KURLRequester *requester )
{
    // only need to init this once
    requester->disconnect( TQ_SIGNAL( openFileDialog( KURLRequester * )),
                           this, TQ_SLOT( openLogDialog( KURLRequester * )));

    KFileDialog *fileDialog = requester->fileDialog();
    fileDialog->setCaption( i18n("Select Log File") );
    TQStringList filters;
    filters << "text/x-log" << "text/plain";
    fileDialog->setMimeFilter( filters );
}

void KNotifyWidget::openExecDialog( KURLRequester *requester )
{
    // only need to init this once
    requester->disconnect( TQ_SIGNAL( openFileDialog( KURLRequester * )),
                           this, TQ_SLOT( openExecDialog( KURLRequester * )));


    KFileDialog *fileDialog = requester->fileDialog();
    fileDialog->setCaption( i18n("Select File to Execute") );
    TQStringList filters;
    filters << "application/x-executable" << "application/x-shellscript"
            << "application/x-perl" << "application/x-python";
    fileDialog->setMimeFilter( filters );
}

void KNotifyWidget::playSound()
{
    TQString soundPath = m_soundPath->url();
    if (!TDEIO::NetAccess::exists( m_soundPath->url(), true, 0 )) {        
        bool foundSound=false;

        // find the first "sound"-resource that contains files
        const Application *app = currentEvent()->application();
        TQStringList soundDirs = TDEGlobal::dirs()->findDirs("data", app->appName() + "/sounds");
        soundDirs += TDEGlobal::dirs()->resourceDirs( "sound" );

        if ( !soundDirs.isEmpty() ) {
           TQDir dir;
           dir.setFilter( TQDir::Files | TQDir::Readable );
           TQStringList::ConstIterator it = soundDirs.begin();
           while ( it != soundDirs.end() ) {
               dir = *it;
               if ( dir.isReadable() && dir.count() > 2 && 
	            TDEIO::NetAccess::exists( *it + m_soundPath->url(), true, 0 )) {
                       foundSound=true;
                       soundPath = *it + m_soundPath->url();
                       break;
               }
               ++it;
           }
        }
        if ( !foundSound ) {
            KMessageBox::sorry(this, i18n("The specified file does not exist." ));
            return;
        }
    }
    KAudioPlayer::play( soundPath );
}

void KNotifyWidget::enableAll()
{
    bool enable = (sender() == m_buttonEnable);
    enableAll( SelectionCombo::type(enable ? m_comboEnable : m_comboDisable),
               enable );
}

void KNotifyWidget::enableAll( int what, bool enable )
{
    if ( m_listview->childCount() == 0 )
        return;

    bool affectAll = m_affectAllApps->isChecked(); // multi-apps mode

    ApplicationListIterator appIt( affectAll ? m_allApps : m_visibleApps );
    for ( ; appIt.current(); ++appIt )
    {
        const EventList& events = appIt.current()->eventList();
        EventListIterator it( events );
        for ( ; it.current(); ++it )
        {
            if ( enable )
                it.current()->presentation |= what;
            else
                it.current()->presentation &= ~what;
        }
    }

    // now make the listview reflect the changes
    TQListViewItemIterator it( m_listview->firstChild() );
    for ( ; it.current(); ++it )
    {
        ListViewItem *item = static_cast<ListViewItem*>( it.current() );
        updatePixmaps( item );
    }

    TQListViewItem *item = m_listview->currentItem();
    if ( !item )
        item = m_listview->firstChild();
    selectItem( item );

    emit changed( true );
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//
// path must be "appname/eventsrc", i.e. a relative path
//
Application::Application( const TQString &path )
{
    TQString config_file = path;
    config_file[config_file.find('/')] = '.';
    m_events = 0L;
    config = new TDEConfig(config_file, false, false);
    kc = new TDEConfig(path, true, false, "data");
    kc->setGroup( TQString::fromLatin1("!Global!") );
    m_icon = kc->readEntry(TQString::fromLatin1("IconName"),
                           TQString::fromLatin1("misc"));
    m_description = kc->readEntry( TQString::fromLatin1("Comment"),
                                   i18n("No description available") );

    int index = path.find( '/' );
    if ( index >= 0 )
        m_appname = path.left( index );
    else
        kdDebug() << "Cannot determine application name from path: " << path << endl;
}

Application::~Application()
{
    delete config;
    delete kc;
    delete m_events;
}


const EventList&  Application::eventList()
{
    if ( !m_events ) {
        m_events = new EventList;
        m_events->setAutoDelete( true );
        reloadEvents();
    }

    return *m_events;
}


void Application::save()
{
    if ( !m_events )
        return;

    EventListIterator it( *m_events );
    Event *e;
    while ( (e = it.current()) ) {
        config->setGroup( e->configGroup );
        config->writeEntry( "presentation", e->presentation );
        config->writePathEntry( "soundfile", e->soundfile );
        config->writePathEntry( "logfile", e->logfile );
        config->writePathEntry( "commandline", e->commandline );

        ++it;
    }
    config->sync();
}


void Application::reloadEvents( bool revertToDefaults )
{
    if ( m_events )
        m_events->clear();
    else
    {
        m_events = new EventList;
        m_events->setAutoDelete( true );
    }

    Event *e = 0L;

    TQString global = TQString::fromLatin1("!Global!");
    TQString default_group = TQString::fromLatin1("<default>");
    TQString name = TQString::fromLatin1("Name");
    TQString comment = TQString::fromLatin1("Comment");

    TQStringList conflist = kc->groupList();
    TQStringList::ConstIterator it = conflist.begin();

    while ( it != conflist.end() ) {
        if ( (*it) != global && (*it) != default_group ) { // event group
            kc->setGroup( *it );

            e = new Event( this );
            e->name = kc->readEntry( name );
            e->description = kc->readEntry( comment );
            e->dontShow = kc->readNumEntry("nopresentation", 0 );
            e->configGroup = *it;
            if ( e->name.isEmpty() && e->description.isEmpty() )
                delete e;
            else { // load the event
                if( !e->name.isEmpty() && e->description.isEmpty() )
                        e->description = e->name;
                // default to passive popups over plain messageboxes
                int default_rep = kc->readNumEntry("default_presentation",
                                                   0 | KNotifyClient::PassivePopup);
                TQString default_logfile = kc->readPathEntry("default_logfile");
                TQString default_soundfile = kc->readPathEntry("default_sound");
                TQString default_commandline = kc->readPathEntry("default_commandline");

                config->setGroup(*it);

                if ( revertToDefaults )
                {
                    e->presentation = default_rep;
                    e->logfile = default_logfile;
                    e->soundfile = default_soundfile;
                    e->commandline = default_commandline;
                }

                else
                {
                    e->presentation = config->readNumEntry("presentation",
                                                           default_rep);
                    e->logfile = config->readPathEntry("logfile",
                                                   default_logfile);
                    e->soundfile = config->readPathEntry("soundfile",
                                                     default_soundfile);
                    e->commandline = config->readPathEntry("commandline",
                                                       default_commandline);
                }

                m_events->append( e );
            }
        }

        ++it;
    }

    return;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ListViewItem::ListViewItem( TQListView *view, Event *event )
    : TQListViewItem( view ),
      m_event( event )
{
    setText( COL_EVENT, event->text() );
}

int ListViewItem::compare ( TQListViewItem * i, int col, bool ascending ) const
{
    ListViewItem *item = static_cast<ListViewItem*>( i );
    int myPres = m_event->presentation;
    int otherPres = item->event().presentation;

    int action = 0;

    switch ( col )
    {
        case COL_EVENT: // use default sorting
            return TQListViewItem::compare( i, col, ascending );

        case COL_EXECUTE:
            action = KNotifyClient::Execute;
            break;
        case COL_LOGFILE:
            action = KNotifyClient::Logfile;
            break;
        case COL_MESSAGE:
            action = (KNotifyClient::Messagebox | KNotifyClient::PassivePopup);
            break;
        case COL_SOUND:
            action = KNotifyClient::Sound;
            break;
        case COL_STDERR:
            action = KNotifyClient::Stderr;
            break;
        case COL_TASKBAR:
            action = KNotifyClient::Taskbar;
            break;
    }

    if ( (myPres & action) == (otherPres & action) )
    {
        // default sorting by event
        return TQListViewItem::compare( i, COL_EVENT, true );
    }

    if ( myPres & action )
        return -1;
    if ( otherPres & action )
        return 1;

    return 0;
}

#include "knotifydialog.moc"

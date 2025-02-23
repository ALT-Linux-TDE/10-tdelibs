 /* This file is part of the KDE libraries
     Copyright
     (C) 2000 Reginald Stadlbauer (reggie@kde.org)
     (C) 1997 Stephan Kulow (coolo@kde.org)
     (C) 1997-2000 Sven Radej (radej@kde.org)
     (C) 1997-2000 Matthias Ettrich (ettrich@kde.org)
     (C) 1999 Chris Schlaeger (cs@kde.org)
     (C) 2002 Joseph Wenninger (jowenn@kde.org)

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
#include "config.h"

#include "tdemainwindow.h"
#include "tdemainwindowiface.h"
#include "tdetoolbarhandler.h"
#include "kwhatsthismanager_p.h"
#include <tqsessionmanager.h>
#include <tqobjectlist.h>
#include <tqstyle.h>
#include <tqlayout.h>
#include <tqwidgetlist.h>
#include <tqtimer.h>

#include <tdeaccel.h>
#include <tdeaction.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <tdemenubar.h>
#include <kstatusbar.h>
#include <twin.h>
#include <kedittoolbar.h>
#include <tdemainwindow.h>

#include <tdelocale.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#if defined TQ_WS_X11
#include <netwm.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

class TDEMainWindowPrivate {
public:
    bool showHelpMenu:1;

    bool autoSaveSettings:1;
    bool settingsDirty:1;
    bool autoSaveWindowSize:1;
    bool care_about_geometry:1;
    bool shuttingDown:1;
    bool newStyleRefCounting:1;
    TQString autoSaveGroup;
    TDEAccel * tdeaccel;
    TDEMainWindowInterface *m_interface;
    KDEPrivate::ToolBarHandler *toolBarHandler;
    TQTimer* settingsTimer;
    TDEToggleAction *showStatusBarAction;
    TQRect defaultWindowSize;
    TQPtrList<TQDockWindow> hiddenDockWindows;
};

TQPtrList<TDEMainWindow>* TDEMainWindow::memberList = 0L;
static bool no_query_exit = false;
static KMWSessionManaged* ksm = 0;
static KStaticDeleter<KMWSessionManaged> ksmd;

class KMWSessionManaged : public KSessionManaged
{
public:
    KMWSessionManaged()
    {
    }
    ~KMWSessionManaged()
    {
    }
    bool saveState( TQSessionManager& )
    {
        TDEConfig* config = TDEApplication::kApplication()->sessionConfig();
        if ( TDEMainWindow::memberList->first() ){
            // According to Jochen Wilhelmy <digisnap@cs.tu-berlin.de>, this
            // hook is useful for better document orientation
            TDEMainWindow::memberList->first()->saveGlobalProperties(config);
        }

        TQPtrListIterator<TDEMainWindow> it(*TDEMainWindow::memberList);
        int n = 0;
        for (it.toFirst(); it.current(); ++it){
            n++;
            it.current()->savePropertiesInternal(config, n);
        }
        config->setGroup(TQString::fromLatin1("Number"));
        config->writeEntry(TQString::fromLatin1("NumberOfWindows"), n );
        return true;
    }

    bool commitData( TQSessionManager& sm )
    {
        // not really a fast method but the only compatible one
        if ( sm.allowsInteraction() ) {
            bool canceled = false;
            TQPtrListIterator<TDEMainWindow> it(*TDEMainWindow::memberList);
            ::no_query_exit = true;
            for (it.toFirst(); it.current() && !canceled;){
                TDEMainWindow *window = *it;
                ++it; // Update now, the current window might get deleted
                if ( !window->testWState( TQt::WState_ForceHide ) ) {
                    TQCloseEvent e;
                    TQApplication::sendEvent( window, &e );
                    canceled = !e.isAccepted();
		    /* Don't even think_about deleting widgets with
		     TQt::WDestructiveClose flag set at this point. We
		     are faking a close event, but we are *not*_
		     closing the window. The purpose of the faked
		     close event is to prepare the application so it
		     can safely be quit without the user losing data
		     (possibly showing a message box "do you want to
		     save this or that?"). It is possible that the
		     session manager quits the application later
		     (emitting TQApplication::aboutToQuit() when this
		     happens), but it is also possible that the user
		     cancels the shutdown, so the application will
		     continue to run.
		     */
                }
            }
            ::no_query_exit = false;
            if (canceled)
               return false;

            TDEMainWindow* last = 0;
            for (it.toFirst(); it.current() && !canceled; ++it){
                TDEMainWindow *window = *it;
                if ( !window->testWState( TQt::WState_ForceHide ) ) {
                    last = window;
                }
            }
            if ( last )
                return last->queryExit();
            // else
            return true;
        }

        // the user wants it, the user gets it
        return true;
    }
};

static bool being_first = true;

TDEMainWindow::TDEMainWindow( TQWidget* parent, const char *name, WFlags f )
    : TQMainWindow( parent, name, f ), KXMLGUIBuilder( this ), helpMenu2( 0 ), factory_( 0 )
{
    initTDEMainWindow(name, 0);
}

TDEMainWindow::TDEMainWindow( int cflags, TQWidget* parent, const char *name, WFlags f )
    : TQMainWindow( parent, name, f ), KXMLGUIBuilder( this ), helpMenu2( 0 ), factory_( 0 )
{
    initTDEMainWindow(name, cflags);
}

void TDEMainWindow::initTDEMainWindow(const char *name, int cflags)
{
    KWhatsThisManager::init ();
    setDockMenuEnabled( false );
    mHelpMenu = 0;
    kapp->setTopWidget( this );
    actionCollection()->setWidget( this );
    connect(kapp, TQ_SIGNAL(shutDown()), this, TQ_SLOT(shuttingDown()));
    if( !memberList )
        memberList = new TQPtrList<TDEMainWindow>;

    if ( !ksm )
        ksm = ksmd.setObject(ksm, new KMWSessionManaged());
    // set a unique object name. Required by session management.
    TQCString objname;
    TQCString s;
    int unusedNumber;
    if ( !name )
        { // no name given
        objname = kapp->instanceName() + "-mainwindow#";
        s = objname + '1'; // start adding number immediately
        unusedNumber = 1;
        }
    else if( name[0] != '\0' && name[ strlen( name ) - 1 ] == '#' )
        { // trailing # - always add a number
        objname = name;
        s = objname + '1'; // start adding number immediately
        unusedNumber = 1;
        }
    else
        {
        objname = name;
        s = objname;
        unusedNumber = 0; // add numbers only when needed
        }
    for(;;) {
        TQWidgetList* list = kapp->topLevelWidgets();
        TQWidgetListIt it( *list );
        bool found = false;
        for( TQWidget* w = it.current();
             w != NULL;
             ++it, w = it.current())
            if( w != this && w->name() == s )
                {
                found = true;
                break;
                }
        delete list;
        if( !found )
            break;
        s.setNum( ++unusedNumber );
        s = objname + s;
    }
    setName( s );

    memberList->append( this );

    d = new TDEMainWindowPrivate;
    d->showHelpMenu = true;
    d->settingsDirty = false;
    d->autoSaveSettings = false;
    d->autoSaveWindowSize = true; // for compatibility
    d->tdeaccel = actionCollection()->tdeaccel();
    d->toolBarHandler = 0;
    d->settingsTimer = 0;
    d->showStatusBarAction = NULL;
    d->shuttingDown = false;
    if ((d->care_about_geometry = being_first)) {
        being_first = false;
        if ( kapp->geometryArgument().isNull() ) // if there is no geometry, it doesn't matter
            d->care_about_geometry = false;
        else
            parseGeometry(false);
    }

    setCaption( kapp->caption() );
    if ( cflags & NoDCOPObject)
        d->m_interface = 0;
    else
        d->m_interface = new TDEMainWindowInterface(this);

    if ( cflags & NewRefCountMode ) {
        d->newStyleRefCounting = true;
        kapp->ref();
    }
    else {
        d->newStyleRefCounting = false;
    }

    if (!kapp->authorize("movable_toolbars"))
        setDockWindowsMovable(false);
}

TDEAction *TDEMainWindow::toolBarMenuAction()
{
    if ( !d->toolBarHandler )
	return 0;

    return d->toolBarHandler->toolBarMenuAction();
}


void TDEMainWindow::setupToolbarMenuActions()
{
    if ( d->toolBarHandler )
        d->toolBarHandler->setupActions();
}

void TDEMainWindow::parseGeometry(bool parsewidth)
{
    assert ( !kapp->geometryArgument().isNull() );
    assert ( d->care_about_geometry );

#if defined TQ_WS_X11
    int x, y;
    int w, h;
    int m = XParseGeometry( kapp->geometryArgument().latin1(), &x, &y, (unsigned int*)&w, (unsigned int*)&h);
    if (parsewidth) {
        TQSize minSize = minimumSize();
        TQSize maxSize = maximumSize();
        if ( !(m & WidthValue) )
            w = width();
        if ( !(m & HeightValue) )
            h = height();
         w = TQMIN(w,maxSize.width());
         h = TQMIN(h,maxSize.height());
         w = TQMAX(w,minSize.width());
         h = TQMAX(h,minSize.height());
         resize(w, h);
    } else {
        if ( parsewidth && !(m & XValue) )
            x = geometry().x();
        if ( parsewidth && !(m & YValue) )
            y = geometry().y();
        if ( (m & XNegative) )
            x = TDEApplication::desktop()->width()  + x - w;
        if ( (m & YNegative) )
            y = TDEApplication::desktop()->height() + y - h;
        move(x, y);
    }
#endif
}

TDEMainWindow::~TDEMainWindow()
{
    delete d->settingsTimer;
    TQMenuBar* mb = internalMenuBar();
    delete mb;
    delete d->m_interface;
    delete d;
    memberList->remove( this );
}

TDEPopupMenu* TDEMainWindow::helpMenu( const TQString &aboutAppText, bool showWhatsThis )
{
    if( !mHelpMenu ) {
        if ( aboutAppText.isEmpty() )
            mHelpMenu = new KHelpMenu( this, instance()->aboutData(), showWhatsThis);
        else
            mHelpMenu = new KHelpMenu( this, aboutAppText, showWhatsThis );

        if ( !mHelpMenu )
            return 0;
        connect( mHelpMenu, TQ_SIGNAL( showAboutApplication() ),
                 this, TQ_SLOT( showAboutApplication() ) );
    }

    return mHelpMenu->menu();
}

TDEPopupMenu* TDEMainWindow::customHelpMenu( bool showWhatsThis )
{
    if( !mHelpMenu ) {
        mHelpMenu = new KHelpMenu( this, TQString::null, showWhatsThis );
        connect( mHelpMenu, TQ_SIGNAL( showAboutApplication() ),
                 this, TQ_SLOT( showAboutApplication() ) );
    }

    return mHelpMenu->menu();
}

bool TDEMainWindow::canBeRestored( int number )
{
    if ( !kapp->isRestored() )
        return false;
    TDEConfig *config = kapp->sessionConfig();
    if ( !config )
        return false;
    config->setGroup( TQString::fromLatin1("Number") );
    int n = config->readNumEntry( TQString::fromLatin1("NumberOfWindows") , 1 );
    return number >= 1 && number <= n;
}

const TQString TDEMainWindow::classNameOfToplevel( int number )
{
    if ( !kapp->isRestored() )
        return TQString::null;
    TDEConfig *config = kapp->sessionConfig();
    if ( !config )
        return TQString::null;
    TQString s;
    s.setNum( number );
    s.prepend( TQString::fromLatin1("WindowProperties") );
    config->setGroup( s );
    if ( !config->hasKey( TQString::fromLatin1("ClassName") ) )
        return TQString::null;
    else
        return config->readEntry( TQString::fromLatin1("ClassName") );
}

void TDEMainWindow::show()
{
    TQMainWindow::show();

    for ( TQPtrListIterator<TQDockWindow> it( d->hiddenDockWindows ); it.current(); ++it )
	it.current()->show();

    d->hiddenDockWindows.clear();
}

void TDEMainWindow::hide()
{
    if ( isVisible() ) {

        d->hiddenDockWindows.clear();

        TQObjectList *list = queryList( "TQDockWindow" );
        for( TQObjectListIt it( *list ); it.current(); ++it ) {
            TQDockWindow *dw = (TQDockWindow*)it.current();
            if ( dw->isTopLevel() && dw->isVisible() ) {
                d->hiddenDockWindows.append( dw );
                dw->hide();
            }
        }
        delete list;
    }

    TQWidget::hide();
}

bool TDEMainWindow::restore( int number, bool show )
{
    if ( !canBeRestored( number ) )
        return false;
    TDEConfig *config = kapp->sessionConfig();
    if ( readPropertiesInternal( config, number ) ){
        if ( show )
            TDEMainWindow::show();
        return false;
    }
    return false;
}

KXMLGUIFactory *TDEMainWindow::guiFactory()
{
    if ( !factory_ )
        factory_ = new KXMLGUIFactory( this, this, "guifactory" );
    return factory_;
}

int TDEMainWindow::configureToolbars()
{
    saveMainWindowSettings(TDEGlobal::config());
    KEditToolbar dlg(actionCollection(), xmlFile(), true, this);
    connect(&dlg, TQ_SIGNAL(newToolbarConfig()), TQ_SLOT(saveNewToolbarConfig()));
    return dlg.exec();
}

void TDEMainWindow::saveNewToolbarConfig()
{
    createGUI(xmlFile());
    applyMainWindowSettings( TDEGlobal::config() );
}

void TDEMainWindow::setupGUI( int options, const TQString & xmlfile ) {
    setupGUI(TQSize(), options, xmlfile);
}

void TDEMainWindow::setupGUI( TQSize defaultSize, int options, const TQString & xmlfile ) {
    if( options & Keys ){
        KStdAction::keyBindings(guiFactory(),
                    TQ_SLOT(configureShortcuts()), actionCollection());
    }

    if( (options & StatusBar) && internalStatusBar() ){
        createStandardStatusBarAction();
    }

    if( options & ToolBar ){
        setStandardToolBarMenuEnabled( true );
        KStdAction::configureToolbars(this,
                      TQ_SLOT(configureToolbars() ), actionCollection());
    }

    if( options & Create ){
        createGUI(xmlfile,false);
    }

    if( options & Save ){
        // setupGUI() is typically called in the constructor before show(),
        // so the default window size will be incorrect unless the application
        // hard coded the size which they should try not to do (i.e. use
        // size hints).
        if(initialGeometrySet())
        {
          // Do nothing...
        }
        else if(defaultSize.isValid())
        {
          resize(defaultSize);
        }
        else if(!isShown())
        {
          adjustSize();
        }
        setAutoSaveSettings();
    }

}

void TDEMainWindow::createGUI( const TQString &xmlfile, bool _conserveMemory )
{
    // disabling the updates prevents unnecessary redraws
    setUpdatesEnabled( false );

    // just in case we are rebuilding, let's remove our old client
    guiFactory()->removeClient( this );

    // make sure to have an empty GUI
    TQMenuBar* mb = internalMenuBar();
    if ( mb )
        mb->clear();

    (void)toolBarIterator(); // make sure toolbarList is most-up-to-date
    toolbarList.setAutoDelete( true );
    toolbarList.clear();
    toolbarList.setAutoDelete( false );

    // don't build a help menu unless the user ask for it
    if (d->showHelpMenu) {
        // we always want a help menu
        if (!helpMenu2)
            helpMenu2 = new KHelpMenu(this, instance()->aboutData(), true,
                                      actionCollection());
    }

    // we always want to load in our global standards file
    setXMLFile( locate( "config", "ui/ui_standards.rc", instance() ) );

    // now, merge in our local xml file.  if this is null, then that
    // means that we will be only using the global file
    if ( !xmlfile.isNull() ) {
        setXMLFile( xmlfile, true );
    } else {
        TQString auto_file(instance()->instanceName() + "ui.rc");
        setXMLFile( auto_file, true );
    }

    // make sure we don't have any state saved already
    setXMLGUIBuildDocument( TQDomDocument() );

    // do the actual GUI building
    guiFactory()->addClient( this );

    // try and get back *some* of our memory
    if ( _conserveMemory )
    {
      // before freeing the memory allocated by the DOM document we also
      // free all memory allocated internally in the KXMLGUIFactory for
      // the menubar and the toolbars . This however implies that we
      // have to take care of deleting those widgets ourselves. For
      // destruction this is no problem, but when rebuilding we have
      // to take care of that (and we want to rebuild the GUI when
      // using stuff like the toolbar editor ).
      // In addition we have to take care of not removing containers
      // like popupmenus, defined in the XML document.
      // this code should probably go into a separate method in TDEMainWindow.
      // there's just one problem: I'm bad in finding names ;-) , so
      // I skipped this ;-)

      TQDomDocument doc = domDocument();

      for( TQDomNode n = doc.documentElement().firstChild();
           !n.isNull(); n = n.nextSibling())
      {
          TQDomElement e = n.toElement();

          if ( e.tagName().lower() == "toolbar" )
              factory_->resetContainer( e.attribute( "name" ) );
          else if ( e.tagName().lower() == "menubar" )
              factory_->resetContainer( e.tagName(), true );
      }

      conserveMemory();
    }

    setUpdatesEnabled( true );
    updateGeometry();
}

void TDEMainWindow::setHelpMenuEnabled(bool showHelpMenu)
{
    d->showHelpMenu = showHelpMenu;
}

bool TDEMainWindow::isHelpMenuEnabled()
{
    return d->showHelpMenu;
}

void TDEMainWindow::setCaption( const TQString &caption )
{
    setPlainCaption( kapp->makeStdCaption(caption) );
}

void TDEMainWindow::setCaption( const TQString &caption, bool modified )
{
    setPlainCaption( kapp->makeStdCaption(caption, true, modified) );
}

void TDEMainWindow::setPlainCaption( const TQString &caption )
{
    TQMainWindow::setCaption( caption );
#if defined TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), winId(), tqt_xrootwin(), 0 );
    info.setName( caption.utf8().data() );
#endif
}

void TDEMainWindow::appHelpActivated( void )
{
    if( !mHelpMenu ) {
        mHelpMenu = new KHelpMenu( this );
        if ( !mHelpMenu )
            return;
    }
    mHelpMenu->appHelpActivated();
}

void TDEMainWindow::slotStateChanged(const TQString &newstate)
{
  stateChanged(newstate, KXMLGUIClient::StateNoReverse);
}

/*
 * Get rid of this for KDE 4.0
 */
void TDEMainWindow::slotStateChanged(const TQString &newstate,
                                   KXMLGUIClient::ReverseStateChange reverse)
{
  stateChanged(newstate, reverse);
}

/*
 * Enable this for KDE 4.0
 */
// void TDEMainWindow::slotStateChanged(const TQString &newstate,
//                                    bool reverse)
// {
//   stateChanged(newstate,
//                reverse ? KXMLGUIClient::StateReverse : KXMLGUIClient::StateNoReverse);
// }

void TDEMainWindow::closeEvent ( TQCloseEvent *e )
{
    // Save settings if auto-save is enabled, and settings have changed
    if (d->settingsDirty && d->autoSaveSettings)
        saveAutoSaveSettings();

    if (queryClose()) {
        e->accept();

        int not_withdrawn = 0;
        TQPtrListIterator<TDEMainWindow> it(*TDEMainWindow::memberList);
        for (it.toFirst(); it.current(); ++it){
            if ( !it.current()->isHidden() && it.current()->isTopLevel() && it.current() != this )
                not_withdrawn++;
        }

        if ( !no_query_exit && not_withdrawn <= 0 ) { // last window close accepted?
            if ( queryExit() && !kapp->sessionSaving() && !d->shuttingDown ) { // Yes, Quit app?
                // don't call queryExit() twice
                disconnect(kapp, TQ_SIGNAL(shutDown()), this, TQ_SLOT(shuttingDown()));
                d->shuttingDown = true;
                kapp->deref();             // ...and quit application.
            }  else {
                // cancel closing, it's stupid to end up with no windows at all....
                e->ignore();
            }
        }
    }
}

bool TDEMainWindow::queryExit()
{
    return true;
}

bool TDEMainWindow::queryClose()
{
    return true;
}

void TDEMainWindow::saveGlobalProperties( TDEConfig*  )
{
}

void TDEMainWindow::readGlobalProperties( TDEConfig*  )
{
}

#if defined(KDE_COMPAT)
void TDEMainWindow::updateRects()
{
}
#endif

void TDEMainWindow::showAboutApplication()
{
}

void TDEMainWindow::savePropertiesInternal( TDEConfig *config, int number )
{
    bool oldASWS = d->autoSaveWindowSize;
    d->autoSaveWindowSize = true; // make saveMainWindowSettings save the window size

    TQString s;
    s.setNum(number);
    s.prepend(TQString::fromLatin1("WindowProperties"));
    config->setGroup(s);

    // store objectName, className, Width and Height  for later restoring
    // (Only useful for session management)
    config->writeEntry(TQString::fromLatin1("ObjectName"), name());
    config->writeEntry(TQString::fromLatin1("ClassName"), className());

    saveMainWindowSettings(config); // Menubar, statusbar and Toolbar settings.

    s.setNum(number);
    config->setGroup(s);
    saveProperties(config);

    d->autoSaveWindowSize = oldASWS;
}

void TDEMainWindow::saveMainWindowSettings(TDEConfig *config, const TQString &configGroup)
{
    kdDebug(200) << "TDEMainWindow::saveMainWindowSettings " << configGroup << endl;
    TQString oldGroup;

    if (!configGroup.isEmpty())
    {
       oldGroup = config->group();
       config->setGroup(configGroup);
    }

    // Called by session management - or if we want to save the window size anyway
    if ( d->autoSaveWindowSize )
        saveWindowSize( config );

    TQStatusBar* sb = internalStatusBar();
    if (sb) {
       if(!config->hasDefault("StatusBar") && !sb->isHidden() )
           config->revertToDefault("StatusBar");
       else
           config->writeEntry("StatusBar", sb->isHidden() ? "Disabled" : "Enabled");
    }

    TQMenuBar* mb = internalMenuBar();
    if (mb) {
       TQString MenuBar = TQString::fromLatin1("MenuBar");
       if(!config->hasDefault("MenuBar") && !mb->isHidden() )
           config->revertToDefault("MenuBar");
       else
           config->writeEntry("MenuBar", mb->isHidden() ? "Disabled" : "Enabled");
    }

    int n = 1; // Toolbar counter. toolbars are counted from 1,
    TDEToolBar *toolbar = 0;
    TQPtrListIterator<TDEToolBar> it( toolBarIterator() );
    while ( ( toolbar = it.current() ) ) {
        ++it;
        TQString group;
        if (!configGroup.isEmpty())
        {
           // Give a number to the toolbar, but prefer a name if there is one,
           // because there's no real guarantee on the ordering of toolbars
           group = (!::qstrcmp(toolbar->name(), "unnamed") ? TQString::number(n) : TQString(" ")+toolbar->name());
           group.prepend(" Toolbar");
           group.prepend(configGroup);
        }
        toolbar->saveSettings(config, group);
        n++;
    }
    if (!configGroup.isEmpty())
       config->setGroup(oldGroup);
}

void TDEMainWindow::setStandardToolBarMenuEnabled( bool enable )
{
    if ( enable ) {
        if ( d->toolBarHandler )
            return;

    d->toolBarHandler = new KDEPrivate::ToolBarHandler( this );

    if ( factory() )
        factory()->addClient( d->toolBarHandler );
    } else {
        if ( !d->toolBarHandler )
            return;

        if ( factory() )
            factory()->removeClient( d->toolBarHandler );

        delete d->toolBarHandler;
        d->toolBarHandler = 0;
    }
}

bool TDEMainWindow::isStandardToolBarMenuEnabled() const
{
    return ( d->toolBarHandler );
}

void TDEMainWindow::createStandardStatusBarAction(){
  if(!d->showStatusBarAction){
    d->showStatusBarAction = KStdAction::showStatusbar(this, TQ_SLOT(setSettingsDirty()), actionCollection());
    KStatusBar *sb = statusBar(); // Creates statusbar if it doesn't exist already.
    connect(d->showStatusBarAction, TQ_SIGNAL(toggled(bool)), sb, TQ_SLOT(setShown(bool)));
    d->showStatusBarAction->setChecked(sb->isHidden());
  }
}

bool TDEMainWindow::readPropertiesInternal( TDEConfig *config, int number )
{
    if ( number == 1 )
        readGlobalProperties( config );

    // in order they are in toolbar list
    TQString s;
    s.setNum(number);
    s.prepend(TQString::fromLatin1("WindowProperties"));

    config->setGroup(s);

    // restore the object name (window role)
    if ( config->hasKey(TQString::fromLatin1("ObjectName" )) )
        setName( config->readEntry(TQString::fromLatin1("ObjectName")).latin1()); // latin1 is right here

    applyMainWindowSettings(config); // Menubar, statusbar and toolbar settings.

    s.setNum(number);
    config->setGroup(s);
    readProperties(config);
    return true;
}

void TDEMainWindow::applyMainWindowSettings(TDEConfig *config, const TQString &configGroup)
{
    return applyMainWindowSettings(config,configGroup,false);
}

void TDEMainWindow::applyMainWindowSettings(TDEConfig *config, const TQString &configGroup,bool force)
{
    kdDebug(200) << "TDEMainWindow::applyMainWindowSettings" << endl;

    TDEConfigGroupSaver saver( config, configGroup.isEmpty() ? config->group() : configGroup );

    restoreWindowSize(config);

    TQStatusBar* sb = internalStatusBar();
    if (sb) {
        TQString entry = config->readEntry("StatusBar", "Enabled");
        if ( entry == "Disabled" )
           sb->hide();
        else
           sb->show();
        if(d->showStatusBarAction)
            d->showStatusBarAction->setChecked(!sb->isHidden());
    }

    TQMenuBar* mb = internalMenuBar();
    if (mb) {
        TQString entry = config->readEntry ("MenuBar", "Enabled");
        if ( entry == "Disabled" )
           mb->hide();
        else
           mb->show();
    }

    int n = 1; // Toolbar counter. toolbars are counted from 1,
    TDEToolBar *toolbar;
    TQPtrListIterator<TDEToolBar> it( toolBarIterator() ); // must use own iterator

    for ( ; it.current(); ++it) {
        toolbar= it.current();
        TQString group;
        if (!configGroup.isEmpty())
        {
           // Give a number to the toolbar, but prefer a name if there is one,
           // because there's no real guarantee on the ordering of toolbars
           group = (!::qstrcmp(toolbar->name(), "unnamed") ? TQString::number(n) : TQString(" ")+toolbar->name());
           group.prepend(" Toolbar");
           group.prepend(configGroup);
        }
        toolbar->applySettings(config, group, force);
        n++;
    }

    finalizeGUI( true );
}

void TDEMainWindow::finalizeGUI( bool force )
{
    //kdDebug(200) << "TDEMainWindow::finalizeGUI force=" << force << endl;
    // The whole reason for this is that moveToolBar relies on the indexes
    // of the other toolbars, so in theory it should be called only once per
    // toolbar, but in increasing order of indexes.
    // Since we can't do that immediately, we move them, and _then_
    // we call positionYourself again for each of them, but this time
    // the toolbariterator should give them in the proper order.
    // Both the XMLGUI and applySettings call this, hence "force" for the latter.
    TQPtrListIterator<TDEToolBar> it( toolBarIterator() );
    for ( ; it.current() ; ++it ) {
        it.current()->positionYourself( force );
    }

    d->settingsDirty = false;
}

void TDEMainWindow::saveWindowSize( TDEConfig * config ) const
{
  int scnum = TQApplication::desktop()->screenNumber(parentWidget());
  TQRect desk = TQApplication::desktop()->screenGeometry(scnum);
  int w, h;
#if defined TQ_WS_X11
  // save maximalization as desktop size + 1 in that direction
  KWin::WindowInfo info = KWin::windowInfo( winId(), NET::WMState );
  w = info.state() & NET::MaxHoriz ? desk.width() + 1 : width();
  h = info.state() & NET::MaxVert ? desk.height() + 1 : height();
#else
  if (isMaximized()) {
    w = desk.width() + 1;
    h = desk.height() + 1;
  }
  //TODO: add "Maximized" property instead "+1" hack
#endif
  TQRect size( desk.width(), w, desk.height(), h );
  bool defaultSize = (size == d->defaultWindowSize);
  TQString widthString = TQString::fromLatin1("Width %1").arg(desk.width());
  TQString heightString = TQString::fromLatin1("Height %1").arg(desk.height());
  if (!config->hasDefault(widthString) && defaultSize)
     config->revertToDefault(widthString);
  else
     config->writeEntry(widthString, w );

  if (!config->hasDefault(heightString) && defaultSize)
     config->revertToDefault(heightString);
  else
     config->writeEntry(heightString, h );
}

void TDEMainWindow::restoreWindowSize( TDEConfig * config )
{
    if (d->care_about_geometry) {
        parseGeometry(true);
    } else {
        // restore the size
        int scnum = TQApplication::desktop()->screenNumber(parentWidget());
        TQRect desk = TQApplication::desktop()->screenGeometry(scnum);
        if ( d->defaultWindowSize.isNull() ) // only once
          d->defaultWindowSize = TQRect(desk.width(), width(), desk.height(), height()); // store default values
        TQSize size( config->readNumEntry( TQString::fromLatin1("Width %1").arg(desk.width()), 0 ),
                    config->readNumEntry( TQString::fromLatin1("Height %1").arg(desk.height()), 0 ) );
        if (size.isEmpty()) {
            // try the KDE 2.0 way
            size = TQSize( config->readNumEntry( TQString::fromLatin1("Width"), 0 ),
                          config->readNumEntry( TQString::fromLatin1("Height"), 0 ) );
            if (!size.isEmpty()) {
                // make sure the other resolutions don't get old settings
                config->writeEntry( TQString::fromLatin1("Width"), 0 );
                config->writeEntry( TQString::fromLatin1("Height"), 0 );
            }
        }
        if ( !size.isEmpty() ) {
#ifdef TQ_WS_X11
            int state = 0;
            if (size.width() > desk.width()) {
                state = state | NET::MaxHoriz;
            }
            if (size.height() > desk.height()) {
                state = state | NET::MaxVert;
            }

            if (( state & NET::Max ) == NET::Max ) {
                resize( desk.width(), desk.height());
            }
            else if(( state & NET::MaxHoriz ) == NET::MaxHoriz ) {
                resize( width(), size.height());
            }
            else if(( state & NET::MaxVert ) == NET::MaxVert ) {
                resize( size.width(), height());
            }
            else {
                resize( size );
            }
            // TQWidget::showMaximized() is both insufficient and broken
            KWin::setState( winId(), state );
#else
            if (size.width() > desk.width() || size.height() > desk.height())
              setWindowState( WindowMaximized );
            else
              resize( size );
#endif
        }
    }
}

bool TDEMainWindow::initialGeometrySet() const
{
    return d->care_about_geometry;
}

void TDEMainWindow::ignoreInitialGeometry()
{
    d->care_about_geometry = false;
}

void TDEMainWindow::setSettingsDirty()
{
    //kdDebug(200) << "TDEMainWindow::setSettingsDirty" << endl;
    d->settingsDirty = true;
    if ( d->autoSaveSettings )
    {
        // Use a timer to save "immediately" user-wise, but not too immediately
        // (to compress calls and save only once, in case of multiple changes)
        if ( !d->settingsTimer )
        {
           d->settingsTimer = new TQTimer( this );
           connect( d->settingsTimer, TQ_SIGNAL( timeout() ), TQ_SLOT( saveAutoSaveSettings() ) );
        }
        d->settingsTimer->start( 500, true );
    }
}

bool TDEMainWindow::settingsDirty() const
{
    return d->settingsDirty;
}

TQString TDEMainWindow::settingsGroup() const
{
    return d->autoSaveGroup;
}

void TDEMainWindow::setAutoSaveSettings( const TQString & groupName, bool saveWindowSize )
{
    d->autoSaveSettings = true;
    d->autoSaveGroup = groupName;
    d->autoSaveWindowSize = saveWindowSize;
    // Get notified when the user moves a toolbar around
    disconnect( this, TQ_SIGNAL( dockWindowPositionChanged( TQDockWindow * ) ),
                this, TQ_SLOT( setSettingsDirty() ) );
    connect( this, TQ_SIGNAL( dockWindowPositionChanged( TQDockWindow * ) ),
             this, TQ_SLOT( setSettingsDirty() ) );

    // Now read the previously saved settings
    applyMainWindowSettings( TDEGlobal::config(), groupName );
}

void TDEMainWindow::resetAutoSaveSettings()
{
    d->autoSaveSettings = false;
    if ( d->settingsTimer )
        d->settingsTimer->stop();
}

bool TDEMainWindow::autoSaveSettings() const
{
    return d->autoSaveSettings;
}

TQString TDEMainWindow::autoSaveGroup() const
{
    return d->autoSaveGroup;
}

void TDEMainWindow::saveAutoSaveSettings()
{
    Q_ASSERT( d->autoSaveSettings );
    //kdDebug(200) << "TDEMainWindow::saveAutoSaveSettings -> saving settings" << endl;
    saveMainWindowSettings( TDEGlobal::config(), d->autoSaveGroup );
    TDEGlobal::config()->sync();
    d->settingsDirty = false;
    if ( d->settingsTimer )
        d->settingsTimer->stop();
}

void TDEMainWindow::resizeEvent( TQResizeEvent * )
{
    if ( d->autoSaveWindowSize )
        setSettingsDirty();
}

bool TDEMainWindow::hasMenuBar()
{
    return (internalMenuBar());
}

KMenuBar *TDEMainWindow::menuBar()
{
    KMenuBar * mb = internalMenuBar();
    if ( !mb ) {
        mb = new KMenuBar( this );
        // trigger a re-layout and trigger a call to the private
        // setMenuBar method.
        TQMainWindow::menuBar();
    }
    return mb;
}

KStatusBar *TDEMainWindow::statusBar()
{
    KStatusBar * sb = internalStatusBar();
    if ( !sb ) {
        sb = new KStatusBar( this );
        // trigger a re-layout and trigger a call to the private
        // setStatusBar method.
        TQMainWindow::statusBar();
    }
    return sb;
}

void TDEMainWindow::shuttingDown()
{
    // Needed for Qt <= 3.0.3 at least to prevent reentrancy
    // when queryExit() shows a dialog. Check before removing!
    static bool reentrancy_protection = false;
    if (!reentrancy_protection)
    {
       reentrancy_protection = true;
       // call the virtual queryExit
       queryExit();
       reentrancy_protection = false;
    }

}

KMenuBar *TDEMainWindow::internalMenuBar()
{
    TQObjectList *l = queryList( "KMenuBar", 0, false, false );
    if ( !l || !l->first() ) {
        delete l;
        return 0;
    }

    KMenuBar *m = (KMenuBar*)l->first();
    delete l;
    return m;
}

KStatusBar *TDEMainWindow::internalStatusBar()
{
    TQObjectList *l = queryList( "KStatusBar", 0, false, false );
    if ( !l || !l->first() ) {
        delete l;
        return 0;
    }

    KStatusBar *s = (KStatusBar*)l->first();
    delete l;
    return s;
}

void TDEMainWindow::childEvent( TQChildEvent* e)
{
    TQMainWindow::childEvent( e );
}

TDEToolBar *TDEMainWindow::toolBar( const char * name )
{
    if (!name)
       name = "mainToolBar";
    TDEToolBar *tb = (TDEToolBar*)child( name, "TDEToolBar" );
    if ( tb )
        return tb;
    bool honor_mode = (!strcmp(name, "mainToolBar"));

    if ( builderClient() )
        return new TDEToolBar(this, name, honor_mode); // XMLGUI constructor
    else
        return new TDEToolBar(this, DockTop, false, name, honor_mode ); // non-XMLGUI
}

TQPtrListIterator<TDEToolBar> TDEMainWindow::toolBarIterator()
{
    toolbarList.clear();
    TQPtrList<TQToolBar> lst;
    for ( int i = (int)TQMainWindow::DockUnmanaged; i <= (int)DockMinimized; ++i ) {
        lst = toolBars( (ToolBarDock)i );
        for ( TQToolBar *tb = lst.first(); tb; tb = lst.next() ) {
            if ( !tb->inherits( "TDEToolBar" ) )
                continue;
            toolbarList.append( (TDEToolBar*)tb );
        }
    }
    return TQPtrListIterator<TDEToolBar>( toolbarList );
}

TDEAccel * TDEMainWindow::accel()
{
    if ( !d->tdeaccel )
        d->tdeaccel = new TDEAccel( this, "kmw-tdeaccel" );
    return d->tdeaccel;
}

void TDEMainWindow::paintEvent( TQPaintEvent * pe )
{
    TQMainWindow::paintEvent(pe); //Upcall to handle SH_MainWindow_SpaceBelowMenuBar rendering
}

TQSize TDEMainWindow::sizeForCentralWidgetSize(TQSize size)
{
    TDEToolBar *tb = (TDEToolBar*)child( "mainToolBar", "TDEToolBar" );
    if (tb && !tb->isHidden()) {
        switch( tb->barPos() )
        {
          case TDEToolBar::Top:
          case TDEToolBar::Bottom:
            size += TQSize(0, tb->sizeHint().height());
            break;

          case TDEToolBar::Left:
          case TDEToolBar::Right:
            size += TQSize(toolBar()->sizeHint().width(), 0);
            break;

          case TDEToolBar::Flat:
            size += TQSize(0, 3+kapp->style().pixelMetric( TQStyle::PM_DockWindowHandleExtent ));
            break;

          default:
            break;
        }
    }
    KMenuBar *mb = internalMenuBar();
    if (mb && !mb->isHidden()) {
        size += TQSize(0,mb->heightForWidth(size.width()));
        if (style().styleHint(TQStyle::SH_MainWindow_SpaceBelowMenuBar, this))
           size += TQSize( 0, dockWindowsMovable() ? 1 : 2);
    }
    TQStatusBar *sb = internalStatusBar();
    if( sb && !sb->isHidden() )
       size += TQSize(0, sb->sizeHint().height());

    return size;
}

#if KDE_IS_VERSION( 3, 9, 0 )
#ifdef __GNUC__
#warning Remove, should be in Qt
#endif
#endif
void TDEMainWindow::setIcon( const TQPixmap& p )
{
    TQMainWindow::setIcon( p );
#ifdef TQ_WS_X11 
    // Qt3 doesn't support _NET_WM_ICON, but TDEApplication::setTopWidget(), which
    // is used by TDEMainWindow, sets it
    KWin::setIcons( winId(), p, TQPixmap());
#endif
}

TQPtrList<TDEMainWindow>* TDEMainWindow::getMemberList() { return memberList; }

// why do we support old gcc versions? using KXMLGUIBuilder::finalizeGUI;
// DF: because they compile KDE much faster :)
void TDEMainWindow::finalizeGUI( KXMLGUIClient *client )
{ KXMLGUIBuilder::finalizeGUI( client ); }

void TDEMainWindow::virtual_hook( int id, void* data )
{ KXMLGUIBuilder::virtual_hook( id, data );
  KXMLGUIClient::virtual_hook( id, data ); }



#include "tdemainwindow.moc"


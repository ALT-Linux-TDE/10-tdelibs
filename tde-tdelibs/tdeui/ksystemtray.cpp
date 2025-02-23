/* This file is part of the KDE libraries

    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)

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

#include "config.h"
#include "tdeaction.h"
#include "tdemessagebox.h"
#include "tdeshortcut.h"
#include "ksystemtray.h"
#include "tdepopupmenu.h"
#include "tdeapplication.h"
#include "tdelocale.h"
#include "tdeaboutdata.h"

#ifdef TQ_WS_X11
#include <twin.h>
#include <twinmodule.h>
#include <qxembed.h>
#endif

#include <kimageeffect.h>
#include <kiconloader.h>
#include <tdeconfig.h>

#include <tqapplication.h>
#include <tqbitmap.h>

class KSystemTrayPrivate
{
public:
    KSystemTrayPrivate()
    {
        actionCollection = 0;
    }

    ~KSystemTrayPrivate()
    {
        delete actionCollection;
    }

    TDEActionCollection* actionCollection;
    bool on_all_desktops; // valid only when the parent widget was hidden
};

KSystemTray::KSystemTray( TQWidget* parent, const char* name )
    : TQLabel( parent, name, (WFlags)WType_TopLevel )
{
#ifdef TQ_WS_X11
    QXEmbed::initialize();
#endif

    d = new KSystemTrayPrivate;
    d->actionCollection = new TDEActionCollection(this);

#ifdef TQ_WS_X11
    KWin::setSystemTrayWindowFor( winId(), parent?parent->topLevelWidget()->winId(): tqt_xrootwin() );
#endif
    setBackgroundMode(X11ParentRelative);
    setBackgroundOrigin(WindowOrigin);
    hasQuit = 0;
    menu = new TDEPopupMenu( this );
    menu->insertTitle( kapp->miniIcon(), kapp->caption() );
    move( -1000, -1000 );
    KStdAction::quit(this, TQ_SLOT(maybeQuit()), d->actionCollection);

    if (parentWidget())
    {
        new TDEAction(i18n("Minimize"), "view-restore", TDEShortcut(),
                    this, TQ_SLOT( minimizeRestoreAction() ),
                    d->actionCollection, "minimizeRestore");
#ifdef TQ_WS_X11
	KWin::WindowInfo info = KWin::windowInfo( parentWidget()->winId() );
	d->on_all_desktops = info.onAllDesktops();
#else
	d->on_all_desktops = false;
#endif
    }
    else
    {
        d->on_all_desktops = false;
    }
    setCaption( TDEGlobal::instance()->aboutData()->programName());
    setAlignment( alignment() | TQt::AlignVCenter | TQt::AlignHCenter );

    // Handle the possibility that the requested system tray size is something other than 22x22 pixels, per the Free Desktop specifications
    setScaledContents(true);
}

KSystemTray::~KSystemTray()
{
    delete d;
}


void KSystemTray::showEvent( TQShowEvent * )
{
    if ( !hasQuit ) {
	menu->insertSeparator();
        TDEAction* action = d->actionCollection->action("minimizeRestore");

        if (action)
        {
            action->plug(menu);
        }

        action = d->actionCollection->action(KStdAction::name(KStdAction::Quit));

        if (action)
        {
            action->plug(menu);
        }

	hasQuit = 1;
    }
}

// KDE4 remove
void KSystemTray::enterEvent( TQEvent* e )
{
    TQLabel::enterEvent( e );
}

TDEPopupMenu* KSystemTray::contextMenu() const
{
    return menu;
}


void KSystemTray::mousePressEvent( TQMouseEvent *e )
{
    if ( !rect().contains( e->pos() ) )
	return;

    switch ( e->button() ) {
    case TQt::LeftButton:
        toggleActive();
	break;
    case TQt::MidButton:
	// fall through
    case TQt::RightButton:
	if ( parentWidget() ) {
            TDEAction* action = d->actionCollection->action("minimizeRestore");
	    if ( parentWidget()->isVisible() )
		action->setText( i18n("&Minimize") );
	    else
		action->setText( i18n("&Restore") );
	}
	contextMenuAboutToShow( menu );
	menu->popup( e->globalPos() );
	break;
    default:
	// nothing
	break;
    }
}

void KSystemTray::mouseReleaseEvent( TQMouseEvent * )
{
}


void KSystemTray::contextMenuAboutToShow( TDEPopupMenu* )
{
}

// called from the popup menu - always do what the menu entry says,
// i.e. if the window is shown, no matter if active or not, the menu
// entry is "minimize", otherwise it's "restore"
void KSystemTray::minimizeRestoreAction()
{
    if ( parentWidget() ) {
        bool restore = !( parentWidget()->isVisible() );
	minimizeRestore( restore );
    }
}

void KSystemTray::maybeQuit()
{
    TQString query = i18n("<qt>Are you sure you want to quit <b>%1</b>?</qt>")
                        .arg(kapp->caption());
    if (KMessageBox::warningContinueCancel(this, query,
                                     i18n("Confirm Quit From System Tray"),
                                     KStdGuiItem::quit(),
                                     TQString("systemtrayquit%1")
                                            .arg(kapp->caption())) !=
        KMessageBox::Continue)
    {
        return;
    }

    emit quitSelected();

    // KDE4: stop closing the parent widget? it results in complex application code
    //       instead make applications connect to the quitSelected() signal

    if (parentWidget())
    {
        parentWidget()->close();
    }
    else
    {
        tqApp->closeAllWindows();
    }
}

void KSystemTray::toggleActive()
{
    activateOrHide();
}

void KSystemTray::setActive()
{
    minimizeRestore( true );
}

void KSystemTray::setInactive()
{
    minimizeRestore( false );
}

// called when left-clicking the tray icon
// if the window is not the active one, show it if needed, and activate it
// (just like taskbar); otherwise hide it
void KSystemTray::activateOrHide()
{
    TQWidget *pw = parentWidget();

    if ( !pw )
	return;

#ifdef TQ_WS_X11
    KWin::WindowInfo info1 = KWin::windowInfo( pw->winId(), NET::XAWMState | NET::WMState );
    // mapped = visible (but possibly obscured)
    bool mapped = (info1.mappingState() == NET::Visible) && !info1.isMinimized();
//    - not mapped -> show, raise, focus
//    - mapped
//        - obscured -> raise, focus
//        - not obscured -> hide
    if( !mapped )
        minimizeRestore( true );
    else
    {
        KWinModule module;
        for( TQValueList< WId >::ConstIterator it = module.stackingOrder().fromLast();
             it != module.stackingOrder().end() && (*it) != pw->winId();
             --it )
        {
            KWin::WindowInfo info2 = KWin::windowInfo( *it,
                NET::WMGeometry | NET::XAWMState | NET::WMState | NET::WMWindowType );
            if( info2.mappingState() != NET::Visible )
                continue; // not visible on current desktop -> ignore
            if( !info2.geometry().intersects( pw->geometry()))
                continue; // not obscuring the window -> ignore
            if( !info1.hasState( NET::KeepAbove ) && info2.hasState( NET::KeepAbove ))
                continue; // obscured by window kept above -> ignore
            NET::WindowType type = info2.windowType( NET::NormalMask | NET::DesktopMask
                | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
                | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask );
            if( type == NET::Dock || type == NET::TopMenu )
                continue; // obscured by dock or topmenu -> ignore
            pw->raise();
            KWin::activateWindow( pw->winId());
            return;
        }
        minimizeRestore( false ); // hide
    }
#endif
}

void KSystemTray::minimizeRestore( bool restore )
{
    TQWidget* pw = parentWidget();
    if( !pw )
	return;
#ifdef TQ_WS_X11
    KWin::WindowInfo info = KWin::windowInfo( pw->winId(), NET::WMGeometry | NET::WMDesktop );
    if ( restore )
    {
	if( d->on_all_desktops )
	    KWin::setOnAllDesktops( pw->winId(), true );
	else
	    KWin::setCurrentDesktop( info.desktop() );
        pw->move( info.geometry().topLeft() ); // avoid placement policies
        pw->show();
        pw->raise();
	KWin::activateWindow( pw->winId() );
    } else {
	d->on_all_desktops = info.onAllDesktops();
	pw->hide();
    }
#endif
}

TDEActionCollection* KSystemTray::actionCollection()
{
    return d->actionCollection;
}

TQPixmap KSystemTray::loadIcon( const TQString &icon, TDEInstance *instance )
{
    TDEConfig *appCfg = kapp->config();
    TDEConfigGroupSaver configSaver(appCfg, "System Tray");
    int iconWidth = appCfg->readNumEntry("systrayIconWidth", 22);
    return instance->iconLoader()->loadIcon( icon, TDEIcon::Panel, iconWidth );
}

TQPixmap KSystemTray::loadSizedIcon( const TQString &icon, int iconWidth, TDEInstance *instance )
{
    return instance->iconLoader()->loadIcon( icon, TDEIcon::Panel, iconWidth );
}

void KSystemTray::setPixmap( const TQPixmap& p )
{
	TQPixmap iconPixmapToSet = p;
	if (TQPaintDevice::x11AppDepth() == 32) {
		TQImage correctedImage = KImageEffect::convertToPremultipliedAlpha( iconPixmapToSet.convertToImage() );
		iconPixmapToSet.convertFromImage(correctedImage);
	}
	TQLabel::setPixmap( iconPixmapToSet );
#ifdef TQ_WS_X11
	KWin::setIcons( winId(), iconPixmapToSet, TQPixmap());
#endif
}

void KSystemTray::setCaption( const TQString& s )
{
    TQLabel::setCaption( s );
}

void KSystemTray::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ksystemtray.moc"
#include "kdockwindow.moc"

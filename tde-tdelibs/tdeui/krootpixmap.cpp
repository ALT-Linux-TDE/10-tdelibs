/*
 *
 *
 * This file is part of the KDE project, module tdeui.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * You can Freely distribute this program under the GNU Library
 * General Public License. See the file "COPYING.LIB" for the exact
 * licensing terms.
 */

#include <tqwidget.h>
#include <tqtimer.h>
#include <tqrect.h>
#include <tqimage.h>

#include <tdeapplication.h>
#include <kimageeffect.h>
#include <kpixmapio.h>
#include <twinmodule.h>
#include <twin.h>
#include <kdebug.h>
#include <netwm.h>
#include <dcopclient.h>
#include <dcopref.h>

#include <ksharedpixmap.h>
#include <krootpixmap.h>


static TQString wallpaperForDesktop(int desktop)
{
    return DCOPRef("kdesktop", "KBackgroundIface").call("currentWallpaper", desktop);
}

class KRootPixmapData
{
public:
    TQWidget *toplevel;
#ifdef TQ_WS_X11
    KWinModule *twin;
#endif
};


KRootPixmap::KRootPixmap( TQWidget *widget, const char *name )
    : TQObject(widget, name ? name : "KRootPixmap" ), m_Desk(0), m_pWidget(widget)
{
    init();
}

KRootPixmap::KRootPixmap( TQWidget *widget, TQObject *parent, const char *name )
    : TQObject( parent, name ? name : "KRootPixmap" ), m_Desk(0), m_pWidget(widget)
{
    init();
}

void KRootPixmap::init()
{
    d = new KRootPixmapData;
    m_Fade = 0;
    m_BlurRadius = 0;
    m_BlurSigma = 0;
    m_pPixmap = new TDESharedPixmap; //ordinary KPixmap on win32
    m_pTimer = new TQTimer( this );
    m_bInit = false;
    m_bActive = false;
    m_bCustomPaint = false;

    connect(kapp, TQ_SIGNAL(backgroundChanged(int)), TQ_SLOT(slotBackgroundChanged(int)));
    connect(m_pTimer, TQ_SIGNAL(timeout()), TQ_SLOT(repaint()));
#ifdef TQ_WS_X11
    connect(m_pPixmap, TQ_SIGNAL(done(bool)), TQ_SLOT(slotDone(bool)));

    d->twin = new KWinModule( this );
    connect(d->twin, TQ_SIGNAL(windowChanged(WId, unsigned int)), TQ_SLOT(desktopChanged(WId, unsigned int)));
    connect(d->twin, TQ_SIGNAL(currentDesktopChanged(int)), TQ_SLOT(desktopChanged(int)));
#endif

    d->toplevel = m_pWidget->topLevelWidget();
    d->toplevel->installEventFilter(this);
    m_pWidget->installEventFilter(this);
}

KRootPixmap::~KRootPixmap()
{
    delete m_pPixmap;
    delete d;
}


int KRootPixmap::currentDesktop() const
{
#ifdef TQ_WS_X11
    NETRootInfo rinfo( tqt_xdisplay(), NET::CurrentDesktop );
    rinfo.activate();
    return rinfo.currentDesktop();
#else
    //OK?
    return TQApplication::desktop()->screenNumber(m_pWidget);
#endif
}


void KRootPixmap::start()
{
    if (m_bActive)
	return;

    m_bActive = true;
    if ( !isAvailable() )
    {
	// We will get a KIPC message when the shared pixmap is available.
	enableExports();
	return;
    }
    if (m_bInit)
	repaint(true);
}


void KRootPixmap::stop()
{
    m_bActive = false;
    m_pTimer->stop();
}


void KRootPixmap::setFadeEffect(double fade, const TQColor &color)
{
    if (fade < 0)
	m_Fade = 0;
    else if (fade > 1)
	m_Fade = 1;
    else
	m_Fade = fade;
    m_FadeColor = color;

    if ( m_bActive && m_bInit ) repaint(true);
}

void KRootPixmap::setBlurEffect(double radius, double sigma)
{
    m_BlurRadius = radius;
    m_BlurSigma = sigma;
}

bool KRootPixmap::eventFilter(TQObject *, TQEvent *event)
{
    // Initialise after the first show or paint event on the managed widget.
    if (!m_bInit && ((event->type() == TQEvent::Show) || (event->type() == TQEvent::Paint)))
    {
	m_bInit = true;
	m_Desk = currentDesktop();
    }

    if (!m_bActive)
	return false;

    switch (event->type())
    {
    case TQEvent::Resize:
    case TQEvent::Move:
	m_pTimer->start(100, true);
	break;

    case TQEvent::Paint:
	m_pTimer->start(0, true);
	break;

    case TQEvent::Reparent:
        d->toplevel->removeEventFilter(this);
        d->toplevel = m_pWidget->topLevelWidget();
        d->toplevel->installEventFilter(this);
        break;

    default:
	break;
    }

    return false; // always continue processing
}

void KRootPixmap::desktopChanged(int desktop)
{
    if (wallpaperForDesktop(m_Desk) == wallpaperForDesktop(desktop) &&
	!wallpaperForDesktop(m_Desk).isNull())
	return;

#ifdef TQ_WS_X11
    if (KWin::windowInfo(m_pWidget->topLevelWidget()->winId()).desktop() == NET::OnAllDesktops &&
	pixmapName(m_Desk) != pixmapName(desktop))
#endif
	repaint(true);
}

void KRootPixmap::desktopChanged( WId window, unsigned int properties )
{
#ifdef TQ_WS_X11
    if( !(properties & NET::WMDesktop) ||
	(window != m_pWidget->topLevelWidget()->winId()))
	return;
#endif

    kdDebug() << k_funcinfo << endl;
    repaint(true);
}

void KRootPixmap::repaint()
{
    repaint(false);
}


void KRootPixmap::repaint(bool force)
{
    TQPoint p1 = m_pWidget->mapToGlobal(m_pWidget->rect().topLeft());
    TQPoint p2 = m_pWidget->mapToGlobal(m_pWidget->rect().bottomRight());
    if (!force && (m_Rect == TQRect(p1, p2)))
	return;

    // Due to northwest bit gravity, we don't need to do anything if the
    // bottom right corner of the widget is moved inward.
    // That said, konsole clears the background when it is resized, so
    // we have to reset the background pixmap.
    if ((p1 == m_Rect.topLeft()) && (m_pWidget->width() < m_Rect.width()) &&
	(m_pWidget->height() < m_Rect.height())
       )
    {
        m_Rect = TQRect(p1, p2);
 	updateBackground( m_pPixmap );
	return;
    }
    m_Rect = TQRect(p1, p2);
#ifdef TQ_WS_X11
    m_Desk = KWin::windowInfo(m_pWidget->topLevelWidget()->winId()).desktop();
    if ((m_Desk == NET::OnAllDesktops) || (m_Desk == 0)) {
	m_Desk = currentDesktop();
    }

    // TDESharedPixmap will correctly generate a tile for us.
    m_pPixmap->loadFromShared(pixmapName(m_Desk), m_Rect);
#else
	m_Desk = currentDesktop();
    // !x11 note: tile is not generated!
    // TODO: pixmapName() is a nonsense now!
    m_pPixmap->load( pixmapName(m_Desk) );
    if (!m_pPixmap->isNull()) {
        m_pPixmap->resize( m_Rect.size() );
        slotDone(true);
    }
#endif
}

bool KRootPixmap::isAvailable() const
{
#ifdef TQ_WS_X11
    return m_pPixmap->isAvailable(pixmapName(m_Desk));
#else
    return m_pPixmap->isNull();
#endif
}

TQString KRootPixmap::pixmapName(int desk) {
    TQString pattern = TQString("DESKTOP%1");
#ifdef TQ_WS_X11
    int screen_number = DefaultScreen(tqt_xdisplay());
    if (screen_number) {
        pattern = TQString("SCREEN%1-DESKTOP").arg(screen_number) + "%1";
    }
#endif
    return pattern.arg( desk );
}


void KRootPixmap::enableExports()
{
#ifdef TQ_WS_X11
    kdDebug(270) << k_lineinfo << "activating background exports.\n";
    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
	client->attach();
    TQByteArray data;
    TQDataStream args( data, IO_WriteOnly );
    args << 1;

    TQCString appname( "kdesktop" );
    int screen_number = DefaultScreen(tqt_xdisplay());
    if ( screen_number )
        appname.sprintf("kdesktop-screen-%d", screen_number );

    client->send( appname, "KBackgroundIface", "setExport(int)", data );
#endif
}


void KRootPixmap::slotDone(bool success)
{
    if (!success)
    {
	kdWarning(270) << k_lineinfo << "loading of desktop background failed.\n";
	return;
    }

    // We need to test active as the pixmap might become available
    // after the widget has been destroyed.
    if ( m_bActive )
	updateBackground( m_pPixmap );
}

void KRootPixmap::updateBackground( TDESharedPixmap *spm )
{
    TQPixmap pm = *spm;

    if (m_Fade > 1e-6)
    {
	KPixmapIO io;
	TQImage img = io.convertToImage(pm);
	img = KImageEffect::fade(img, m_Fade, m_FadeColor);
	pm = io.convertToPixmap(img);
    }

    if ((m_BlurRadius > 1e-6) || (m_BlurSigma > 1e-6))
    {
        KPixmapIO io;
        TQImage img = io.convertToImage(pm);
        img = KImageEffect::blur(img, m_BlurRadius, m_BlurSigma);
        pm = io.convertToPixmap(img);
    }

    if ( !m_bCustomPaint )
	m_pWidget->setBackgroundPixmap( pm );
    else {
	emit backgroundUpdated( pm );
    }
}


void KRootPixmap::slotBackgroundChanged(int desk)
{
    if (!m_bInit || !m_bActive)
	return;

    if (desk == m_Desk)
	repaint(true);
}

#include "krootpixmap.moc"

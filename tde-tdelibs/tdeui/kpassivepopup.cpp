/*
 *   copyright            : (C) 2001-2002 by Richard Moore
 *   copyright            : (C) 2004-2005 by Sascha Cunz
 *   License              : This file is released under the terms of the LGPL, version 2.
 *   email                : rich@kde.org
 *   email                : sascha.cunz@tiscali.de
 */

#include <tdeconfig.h>

#include <tqapplication.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqvbox.h>
#include <tqpainter.h>
#include <tqtooltip.h>
#include <tqbitmap.h>
#include <tqpointarray.h>

#include <kdebug.h>
#include <kdialog.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <tdeglobalsettings.h>

#include "config.h"
#ifdef TQ_WS_X11
#include <netwm.h>
#endif

#include "kpassivepopup.h"
#include "kpassivepopup.moc"

class KPassivePopup::Private
{
public:
  int popupStyle;
  TQPointArray               surround;
  TQPoint                    anchor;
  TQPoint                    fixedPosition;
};

static const int DEFAULT_POPUP_TYPE = KPassivePopup::Boxed;
static const int DEFAULT_POPUP_TIME = 6*1000;
static const int POPUP_FLAGS = TQt::WStyle_Customize | TQt::WDestructiveClose | TQt::WX11BypassWM
                             | TQt::WStyle_StaysOnTop | TQt::WStyle_Tool | TQt::WStyle_NoBorder;

KPassivePopup::KPassivePopup( TQWidget *parent, const char *name, WFlags f )
    : TQFrame( 0, name, (WFlags)(f ? (int)f : POPUP_FLAGS) ),
      window( parent ? parent->winId() : 0L ), msgView( 0 ), topLayout( 0 ),
      hideDelay( DEFAULT_POPUP_TIME ), hideTimer( new TQTimer( this, "hide_timer" ) ),
      m_autoDelete( false )
{
    init( DEFAULT_POPUP_TYPE );
}

KPassivePopup::KPassivePopup( WId win, const char *name, WFlags f )
    : TQFrame( 0, name, (WFlags)(f ? (int)f : POPUP_FLAGS) ),
      window( win ), msgView( 0 ), topLayout( 0 ),
      hideDelay( DEFAULT_POPUP_TIME ), hideTimer( new TQTimer( this, "hide_timer" ) ),
      m_autoDelete( false )
{
    init( DEFAULT_POPUP_TYPE );
}

KPassivePopup::KPassivePopup( int popupStyle, TQWidget *parent, const char *name, WFlags f )
    : TQFrame( 0, name, (WFlags)(f ? (int)f : POPUP_FLAGS) ),
      window( parent ? parent->winId() : 0L ), msgView( 0 ), topLayout( 0 ),
      hideDelay( DEFAULT_POPUP_TIME ), hideTimer( new TQTimer( this, "hide_timer" ) ),
      m_autoDelete( false )
{
    init( popupStyle );
}

KPassivePopup::KPassivePopup( int popupStyle, WId win, const char *name, WFlags f )
    : TQFrame( 0, name, (WFlags)(f ? (int)f : POPUP_FLAGS) ),
      window( win ), msgView( 0 ), topLayout( 0 ),
      hideDelay( DEFAULT_POPUP_TIME ), hideTimer( new TQTimer( this, "hide_timer" ) ),
      m_autoDelete( false )
{
    init( popupStyle );
}

void KPassivePopup::init( int popupStyle )
{
    d = new Private;
    d->popupStyle = popupStyle;
    if( popupStyle == Boxed )
    {
        setFrameStyle( TQFrame::Box| TQFrame::Plain );
        setLineWidth( 2 );
    }
    else if( popupStyle == Balloon )
    {
        setPalette(TQToolTip::palette());
        setAutoMask(TRUE);
    }
    connect( hideTimer, TQ_SIGNAL( timeout() ), TQ_SLOT( hide() ) );
    connect( this, TQ_SIGNAL( clicked() ), TQ_SLOT( hide() ) );
}

KPassivePopup::~KPassivePopup()
{
    delete d;
}

void KPassivePopup::setView( TQWidget *child )
{
    delete msgView;
    msgView = child;

    delete topLayout;
    topLayout = new TQVBoxLayout( this, d->popupStyle == Balloon ? 22 : KDialog::marginHint(), KDialog::spacingHint() );
    topLayout->addWidget( msgView );
    topLayout->activate();
}

void KPassivePopup::setView( const TQString &caption, const TQString &text,
                             const TQPixmap &icon )
{
    // kdDebug() << "KPassivePopup::setView " << caption << ", " << text << endl;
    setView( standardView( caption, text, icon, this ) );
}

static void truncateStringToFit(TQString &string, TQFont font, int max_width) {
	bool truncated = false;
	TQFontMetrics fm(font);
	while (fm.width(string) > max_width) {
		string.truncate(string.length() - 1);
		truncated = true;
	}
	if (truncated) {
		string += " ...";
	}
}

TQVBox * KPassivePopup::standardView(const TQString& caption,
                                     const TQString& text,
                                     const TQPixmap& icon,
                                     TQWidget *parent)
{
	TQString sizedCaption = caption;
	TQString sizedText = text;

#ifdef TQ_WS_X11
	int max_width;

	NETRootInfo info( tqt_xdisplay(),
			NET::NumberOfDesktops |
			NET::CurrentDesktop |
			NET::WorkArea,
			-1, false );
	info.activate();
	NETRect workArea = info.workArea(info.currentDesktop());
	max_width = workArea.size.width / 3;
#endif

	TQVBox *vb = new TQVBox( parent ? parent : this );
	vb->setSpacing( KDialog::spacingHint() );
	
	TQHBox *hb=0;
	if ( !icon.isNull() ) {
		hb = new TQHBox( vb );
		hb->setMargin( 0 );
		hb->setSpacing( KDialog::spacingHint() );
		ttlIcon = new TQLabel( hb, "title_icon" );
		ttlIcon->setPixmap( icon );
		ttlIcon->setAlignment( AlignLeft );
	}

	if ( !sizedCaption.isEmpty() ) {
		ttl = new TQLabel( sizedCaption, hb ? hb : vb, "title_label" );
		TQFont fnt = ttl->font();
#ifdef TQ_WS_X11
		truncateStringToFit(sizedCaption, fnt, max_width);
		ttl->setText(sizedCaption);
#endif
		fnt.setBold( true );
		ttl->setFont( fnt );
		ttl->setAlignment( TQt::AlignHCenter );
		if ( hb ) {
			hb->setStretchFactor( ttl, 10 ); // enforce centering
		}
	}

	if ( !sizedText.isEmpty() ) {
		msg = new TQLabel( sizedText, vb, "msg_label" );
#ifdef TQ_WS_X11
		TQStringList textLines = TQStringList::split("\n", sizedText, true);
		for (TQStringList::Iterator it = textLines.begin(); it != textLines.end(); ++it) {
			truncateStringToFit(*it, msg->font(), max_width);
		}

		// Limit message to 5 lines of text
		if (textLines.count() > 5) {
			int count = 3;
			TQStringList truncatedLines;
			for (TQStringList::Iterator it = textLines.begin(); it != textLines.end(); ++it) {
				truncatedLines.append(*it);
				if (count > 5) {
					truncatedLines.append("...");
					break;
				}
				count++;
			}
			textLines = truncatedLines;
		}
		sizedText = textLines.join("\n");
		msg->setText(sizedText);
#endif
		msg->setAlignment( AlignLeft );
	}

	return vb;
}

void KPassivePopup::setView( const TQString &caption, const TQString &text )
{
    setView( caption, text, TQPixmap() );
}

void KPassivePopup::setTimeout( int delay )
{
    hideDelay = delay;
    if( hideTimer->isActive() )
    {
        if( delay ) {
            hideTimer->changeInterval( delay );
        } else {
            hideTimer->stop();
        }
    }
}

void KPassivePopup::setAutoDelete( bool autoDelete )
{
    m_autoDelete = autoDelete;
}

void KPassivePopup::mouseReleaseEvent( TQMouseEvent *e )
{
    emit clicked();
    emit clicked( e->pos() );
}

//
// Main Implementation
//

void KPassivePopup::show()
{
	TQSize desiredSize = sizeHint();

	if (size() != desiredSize) {
		resize(desiredSize);
	}

	if (d->fixedPosition.isNull()) {
		positionSelf();
	}
	else {
		if( d->popupStyle == Balloon ) {
			setAnchor(d->fixedPosition);
		}
		else {
			move(d->fixedPosition);
		}
	}
	TQFrame::show();

	int delay = hideDelay;
	if ( delay < 0 ) {
		delay = DEFAULT_POPUP_TIME;
	}

	if ( delay > 0 ) {
		hideTimer->start( delay );
	}
}

void KPassivePopup::show(const TQPoint &p)
{
    d->fixedPosition = p;
    show();
}

void KPassivePopup::hideEvent( TQHideEvent * )
{
    hideTimer->stop();
    emit( hidden( this ) );
    if ( m_autoDelete )
        deleteLater();
}

TQRect KPassivePopup::defaultArea() const
{
#ifdef TQ_WS_X11
    NETRootInfo info( tqt_xdisplay(),
                      NET::NumberOfDesktops |
                      NET::CurrentDesktop |
                      NET::WorkArea,
                      -1, false );
    info.activate();
    NETRect workArea = info.workArea( info.currentDesktop() );
    TQRect r;
    r.setRect( workArea.pos.x, workArea.pos.y, 0, 0 ); // top left
#else
    // FIX IT
    TQRect r;
    r.setRect( 100, 100, 200, 200 ); // top left
#endif
    return r;
}

void KPassivePopup::positionSelf()
{
    TQRect target;

#ifdef TQ_WS_X11
    if ( !window ) {
        target = defaultArea();
    }

    else {
        NETWinInfo ni( tqt_xdisplay(), window, tqt_xrootwin(),
                       NET::WMIconGeometry | NET::WMKDESystemTrayWinFor );

        // Figure out where to put the popup. Note that we must handle
        // windows that skip the taskbar cleanly
        if ( ni.kdeSystemTrayWinFor() ) {
            NETRect frame, win;
            ni.kdeGeometry( frame, win );
            target.setRect( win.pos.x, win.pos.y,
                            win.size.width, win.size.height );
        }
        else if ( ni.state() & NET::SkipTaskbar ) {
            target = defaultArea();
        }
        else {
            NETRect r = ni.iconGeometry();
            target.setRect( r.pos.x, r.pos.y, r.size.width, r.size.height );
                if ( target.isNull() ) { // bogus value, use the exact position
                    NETRect dummy;
                    ni.kdeGeometry( dummy, r );
                    target.setRect( r.pos.x, r.pos.y, 
                                    r.size.width, r.size.height);
                }
        }
    }
#else
        target = defaultArea();
#endif
    moveNear( target );
}

void KPassivePopup::moveNear( TQRect target )
{
    TQPoint pos = target.topLeft();
    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

    TQRect r = TDEGlobalSettings::desktopGeometry(TQPoint(x+w/2,y+h/2));

    if( d->popupStyle == Balloon )
    {
        // find a point to anchor to
        if( x + w > r.width() ){
            x = x + target.width();
        }

        if( y + h > r.height() ){
            y = y + target.height();
        }
    } else
    {
        if ( x < r.center().x() )
            x = x + target.width();
        else
            x = x - w;

        // It's apparently trying to go off screen, so display it ALL at the bottom.
        if ( (y + h) > r.bottom() )
            y = r.bottom() - h;

        if ( (x + w) > r.right() )
            x = r.right() - w;
    }
    if ( y < r.top() )
        y = r.top();

    if ( x < r.left() )
        x = r.left();

    if( d->popupStyle == Balloon )
        setAnchor( TQPoint( x, y ) );
    else
        move( x, y );
}

void KPassivePopup::setAnchor(const TQPoint &anchor)
{
    d->anchor = anchor;
    updateMask();
}

void KPassivePopup::paintEvent( TQPaintEvent* pe )
{
	if( d->popupStyle == Balloon ) {
		TQPainter p;
		p.begin( this );
		p.drawPolygon( d->surround );
	}
	else {
		TQFrame::paintEvent( pe );
	}
}

void KPassivePopup::updateMask()
{
    // get screen-geometry for screen our anchor is on
    // (geometry can differ from screen to screen!
    TQRect deskRect = TDEGlobalSettings::desktopGeometry(d->anchor);

    int xh = 70, xl = 40;
    if( width() < 80 )
        xh = xl = 40;
    else if( width() < 110 )
        xh = width() - 40;

    bool bottom = (d->anchor.y() + height()) > ((deskRect.y() + deskRect.height()-48));
    bool right = (d->anchor.x() + width()) > ((deskRect.x() + deskRect.width()-48));

    TQPoint corners[4] = {
        TQPoint( width() - 50, 10 ),
        TQPoint( 10, 10 ),
        TQPoint( 10, height() - 50 ),
        TQPoint( width() - 50, height() - 50 )
    };

    TQBitmap mask( width(), height(), true );
    TQPainter p( &mask );
    TQBrush brush( TQt::white, TQt::SolidPattern );
    p.setBrush( brush );

    int i = 0, z = 0;
    for (; i < 4; ++i) {
        TQPointArray corner;
        corner.makeArc(corners[i].x(), corners[i].y(), 40, 40, i * 16 * 90, 16 * 90);

        d->surround.resize( z + corner.count() );
        for (unsigned int s = 0; s < corner.count() - 1; s++) {
            d->surround.setPoint( z++, corner[s] );
        }
		
        if (bottom && i == 2) {
            if (right) {
                d->surround.resize( z + 3 );
                d->surround.setPoint( z++, TQPoint( width() - xh, height() - 11 ) );
                d->surround.setPoint( z++, TQPoint( width() - 20, height() ) );
                d->surround.setPoint( z++, TQPoint( width() - xl, height() - 11 ) );
            } else {
                d->surround.resize( z + 3 );
                d->surround.setPoint( z++, TQPoint( xl, height() - 11 ) );
                d->surround.setPoint( z++, TQPoint( 20, height() ) );
                d->surround.setPoint( z++, TQPoint( xh, height() - 11 ) );
            }
        } else if (!bottom && i == 0) {
            if (right) {
                d->surround.resize( z + 3 );
                d->surround.setPoint( z++, TQPoint( width() - xl, 10 ) );
                d->surround.setPoint( z++, TQPoint( width() - 20, 0 ) );
                d->surround.setPoint( z++, TQPoint( width() - xh, 10 ) );
            } else {
                d->surround.resize( z + 3 );
                d->surround.setPoint( z++, TQPoint( xh, 10 ) );
                d->surround.setPoint( z++, TQPoint( 20, 0 ) );
                d->surround.setPoint( z++, TQPoint( xl, 10 ) );
            }
        }
    }

    d->surround.resize( z + 1 );
    d->surround.setPoint( z, d->surround[0] );
    p.drawPolygon( d->surround );
    setMask(mask);

    move( right ? d->anchor.x() - width() + 20 : ( d->anchor.x() < 11 ? 11 : d->anchor.x() - 20 ),
          bottom ? d->anchor.y() - height() : ( d->anchor.y() < 11 ? 11 : d->anchor.y() ) );

    update();
}

//
// Convenience Methods
//

KPassivePopup *KPassivePopup::message( const TQString &caption, const TQString &text,
				       const TQPixmap &icon,
				       TQWidget *parent, const char *name, int timeout )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, icon, parent, name, timeout );
}

KPassivePopup *KPassivePopup::message( const TQString &text, TQWidget *parent, const char *name )
{
    return message( DEFAULT_POPUP_TYPE, TQString::null, text, TQPixmap(), parent, name );
}

KPassivePopup *KPassivePopup::message( const TQString &caption, const TQString &text,
				       TQWidget *parent, const char *name )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, TQPixmap(), parent, name );
}

KPassivePopup *KPassivePopup::message( const TQString &caption, const TQString &text,
				       const TQPixmap &icon, WId parent, const char *name, int timeout )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, icon, parent, name, timeout );
}

KPassivePopup *KPassivePopup::message( int popupStyle, const TQString &caption, const TQString &text,
				       const TQPixmap &icon,
				       TQWidget *parent, const char *name, int timeout )
{
    KPassivePopup *pop = new KPassivePopup( popupStyle, parent, name );
    pop->setAutoDelete( true );
    pop->setView( caption, text, icon );
    pop->hideDelay = timeout;
    pop->show();

    return pop;
}

KPassivePopup *KPassivePopup::message( int popupStyle, const TQString &text, TQWidget *parent, const char *name )
{
    return message( popupStyle, TQString::null, text, TQPixmap(), parent, name );
}

KPassivePopup *KPassivePopup::message( int popupStyle, const TQString &caption, const TQString &text,
				       TQWidget *parent, const char *name )
{
    return message( popupStyle, caption, text, TQPixmap(), parent, name );
}

KPassivePopup *KPassivePopup::message( int popupStyle, const TQString &caption, const TQString &text,
				       const TQPixmap &icon, WId parent, const char *name, int timeout )
{
    KPassivePopup *pop = new KPassivePopup( popupStyle, parent, name );
    pop->setAutoDelete( true );
    pop->setView( caption, text, icon );
    pop->hideDelay = timeout;
    pop->show();

    return pop;
}

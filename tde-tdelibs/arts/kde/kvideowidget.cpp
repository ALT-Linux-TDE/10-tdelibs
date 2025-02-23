/*
   This file is part of KDE/aRts (Noatun) - xine integration
   Copyright (C) 2002 Ewald Snel <ewald@rambo.its.tudelft.nl>
   Copyright (C) 2002 Neil Stevens <neil@qualityassistant.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#ifdef HAVE_USLEEP
#include <unistd.h>
#endif
#include <tqaccel.h>
#include <tqcursor.h>

#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#else
#define XEvent void
#endif

#include <tdeaction.h>
#include <tdelocale.h>
#include "kvideowidget.h"


class KFullscreenVideoWidget : public KVideoWidget
{
public:
    KFullscreenVideoWidget( KVideoWidget *parent = 0, const char *name = 0 );

protected:
    virtual void windowActivationChange( bool );
    virtual bool x11Event( XEvent *event );

private:
    KVideoWidget *videoWidget;
};

KFullscreenVideoWidget::KFullscreenVideoWidget( KVideoWidget *parent, const char *name )
    : KVideoWidget( 0, name )
{
    this->videoWidget = parent;
    setEraseColor( black );
    setCursor(TQCursor(TQt::BlankCursor));
}

void KFullscreenVideoWidget::windowActivationChange( bool )
{
    if (!isActiveWindow())
    {
	videoWidget->setWindowed();
    }
}

bool KFullscreenVideoWidget::x11Event( XEvent *event )
{
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
    if (event->type == ClientMessage &&
	event->xclient.message_type ==
		XInternAtom( tqt_xdisplay(), "VPO_RESIZE_NOTIFY", False ))
    {
	videoWidget->resizeNotify( event->xclient.data.l[0], event->xclient.data.l[1] );
    }
#endif
    return false;
}

KVideoWidget::KVideoWidget( KXMLGUIClient *clientParent, TQWidget *parent, const char *name, WFlags f )
    : KXMLGUIClient( clientParent ),
    TQWidget( parent, name, f )
{
	init();
	// ???
	TQString toolbarName = i18n("Video Toolbar");
	setXML(TQString("<!DOCTYPE kpartgui>\n<kpartgui name=\"kvideowidget\" version=\"1\"><MenuBar><Menu name=\"edit\"><Separator/><Action name=\"double_size\"/><Action name=\"normal_size\"/><Action name=\"half_size\"/><Separator/><Action name=\"fullscreen_mode\"/></Menu></MenuBar><Toolbar name=\"VideoToolbar\"><text>Video Toolbar</text><Action name=\"fullscreen_mode\"/></Toolbar></kpartgui>"), true);
}

KVideoWidget::KVideoWidget( TQWidget *parent, const char *name, WFlags f )
    : TQWidget( parent, name, f )
{
	init();
}

void KVideoWidget::init(void)
{
    setMinimumSize(0, 0);
    setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Expanding ) );
    setFocusPolicy( TQWidget::ClickFocus );

    fullscreenWidget = 0;
    poVideo	     = Arts::VideoPlayObject::null();
    videoWidth	     = 0;
    videoHeight	     = 0;

    // Setup actions
    new TDEToggleAction( i18n("Fullscreen &Mode"), "view-fullscreen",
		       CTRL+SHIFT+Key_F, this, TQ_SLOT(fullscreenActivated()),
		       actionCollection(), "fullscreen_mode" );
    new TDERadioAction( i18n("&Half Size"), ALT+Key_0,
		      this, TQ_SLOT(halfSizeActivated()),
		      actionCollection(), "half_size" );
    new TDERadioAction( i18n("&Normal Size"), ALT+Key_1,
		      this, TQ_SLOT(normalSizeActivated()),
		      actionCollection(), "normal_size" );
    new TDERadioAction( i18n("&Double Size"), ALT+Key_2,
		      this, TQ_SLOT(doubleSizeActivated()),
		      actionCollection(), "double_size" );

    ((TDEToggleAction *)action( "half_size" ))->setExclusiveGroup( "KVideoWidget::zoom" );
    ((TDEToggleAction *)action( "normal_size" ))->setExclusiveGroup( "KVideoWidget::zoom" );
    ((TDEToggleAction *)action( "double_size" ))->setExclusiveGroup( "KVideoWidget::zoom" );

    action("double_size")->setEnabled(false);
    action("half_size")->setEnabled(false);
    action("normal_size")->setEnabled(false);
    action("fullscreen_mode")->setEnabled(false);
}

KVideoWidget::~KVideoWidget()
{
    if (isEmbedded())
    {
	poVideo.x11WindowId( -1 );
	poVideo = Arts::VideoPlayObject::null();
    }

	delete fullscreenWidget;
}

void KVideoWidget::embed( Arts::VideoPlayObject vpo )
{
    bool enable;
    if (vpo.isNull())
    {
	if (isEmbedded())
	{
	    poVideo.x11WindowId( -1 );
	    poVideo = Arts::VideoPlayObject::null();
	}

	setBackgroundMode( PaletteBackground );
	repaint();

	// Resize GUI
	videoWidth  = 0;
	videoHeight = 0;

	if (isHalfSize() || isNormalSize() || isDoubleSize())
	    emit adaptSize( 0, 0 );

        enable = false;
	updateGeometry();
    }
    else
    {
	if (isEmbedded())
	{
	    poVideo.x11WindowId( -1 );
	}

	poVideo = vpo;

	// Don't reset fullscreen mode for video playlists
	if (fullscreenWidget)
	{
	    poVideo.x11WindowId( fullscreenWidget->winId() );
	    fullscreenWidget->setBackgroundMode( NoBackground );

	    setEraseColor( black );
	}
	else
	{
	    poVideo.x11WindowId( winId() );
	    setBackgroundMode( NoBackground );
	}
        enable = true;
    }
    action("double_size")->setEnabled(enable);
    action("half_size")->setEnabled(enable);
    action("normal_size")->setEnabled(enable);
    action("fullscreen_mode")->setEnabled(enable);
}

TQImage KVideoWidget::snapshot( Arts::VideoPlayObject vpo )
{
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
    Window root;
    Pixmap pixmap;
    XImage *xImage;
    unsigned int width, height, border, depth;
    int x, y;

    if (vpo.isNull() || (long)(pixmap = vpo.x11Snapshot()) == -1)
	return TQImage();

    // Get 32bit RGBA image data (stored in 1bpp pixmap)
    XGetGeometry( tqt_xdisplay(), pixmap, &root, &x, &y, &width, &height, &border, &depth );

    xImage = XGetImage( tqt_xdisplay(), pixmap, 0, 0, width, height, 1, XYPixmap );

    if (xImage == 0)
    {
	XFreePixmap( tqt_xdisplay(), pixmap );
	return TQImage();
    }

    // Convert 32bit RGBA image data into Qt image
    TQImage qImage = TQImage( (uchar *)xImage->data, width/32, height, 32, (TQRgb *)0, 0, TQImage::IgnoreEndian ).copy();

    // Free X11 resources and return Qt image
    XDestroyImage( xImage );
    XFreePixmap( tqt_xdisplay(), pixmap );

    return qImage;
#else
    return 0;
#endif
}

bool KVideoWidget::isEmbedded()
{
    return !poVideo.isNull();
}

bool KVideoWidget::isFullscreen()
{
    return ((TDEToggleAction *)action( "fullscreen_mode" ))->isChecked();
}

bool KVideoWidget::isHalfSize()
{
    return ((TDEToggleAction *)action( "half_size" ))->isChecked();
}

bool KVideoWidget::isNormalSize()
{
    return ((TDEToggleAction *)action( "normal_size" ))->isChecked();
}

bool KVideoWidget::isDoubleSize()
{
    return ((TDEToggleAction *)action( "double_size" ))->isChecked();
}

void KVideoWidget::setFullscreen()
{
    if (!isFullscreen())
    {
	((TDEToggleAction *)action( "fullscreen_mode" ))->setChecked( true );
	fullscreenActivated();
    }
}

void KVideoWidget::setWindowed()
{
    if (isFullscreen())
    {
	((TDEToggleAction *)action( "fullscreen_mode" ))->setChecked( false );
	fullscreenActivated();
    }
}

void KVideoWidget::setHalfSize()
{
    ((TDEToggleAction *)action( "half_size" ))->setChecked( true );
    halfSizeActivated();
}

void KVideoWidget::setNormalSize()
{
    ((TDEToggleAction *)action( "normal_size" ))->setChecked( true );
    normalSizeActivated();
}

void KVideoWidget::setDoubleSize()
{
    ((TDEToggleAction *)action( "double_size" ))->setChecked( true );
    doubleSizeActivated();
}

TQSize KVideoWidget::sizeHint() const
{
    return TQSize( videoWidth, videoHeight );
}

int KVideoWidget::heightForWidth( int w ) const
{
	if(videoWidth == 0)
		return 0;
	else
		return int( double(w)*double(videoHeight)/double(videoWidth) );
}

void KVideoWidget::mousePressEvent( TQMouseEvent *event )
{
	TQPoint pos = mapToGlobal( event->pos() );

	emit mouseButtonPressed( event->button(), pos, event->state() );

	// ### Remove in KDE4
	if ( event->button() == TQt::RightButton )
		emit rightButtonPressed( pos );
}

void KVideoWidget::mouseDoubleClickEvent( TQMouseEvent *event )
{
	emit mouseButtonDoubleClick( mapToGlobal( event->pos() ), event->state() );
}

void KVideoWidget::resizeEvent( TQResizeEvent *event )
{
    TQWidget::resizeEvent( event );

    if (width() > minimumWidth() || height() > minimumHeight())
    {
	if (width() == TQMAX( (videoWidth / 2), minimumWidth() ) &&
	         height() == TQMAX( (videoHeight / 2), minimumHeight() ))
	    ((TDEToggleAction *)action( "half_size" ))->setChecked( true );
	else if (width() == TQMAX( videoWidth, minimumWidth() ) &&
		 height() == TQMAX( videoHeight, minimumHeight() ))
	    ((TDEToggleAction *)action( "normal_size" ))->setChecked( true );
	else if (width() == TQMAX( (2 * videoWidth), minimumWidth() ) &&
		 height() == TQMAX( (2 * videoHeight), minimumHeight() ))
	    ((TDEToggleAction *)action( "double_size" ))->setChecked( true );
	else
	{
	    ((TDEToggleAction *)action( "half_size" ))->setChecked( false );
	    ((TDEToggleAction *)action( "normal_size" ))->setChecked( false );
	    ((TDEToggleAction *)action( "double_size" ))->setChecked( false );
	}
    }
}

void KVideoWidget::resizeNotify( int width, int height )
{
    if(!isEmbedded()) return;

    videoWidth = width;
    videoHeight = height;

    if (isHalfSize())
	emit adaptSize( (videoWidth / 2), (videoHeight / 2) );
    else if (isNormalSize())
	emit adaptSize( videoWidth, videoHeight );
    else if (isDoubleSize())
	emit adaptSize( (2 * videoWidth), (2 * videoHeight) );

    updateGeometry();
}

bool KVideoWidget::x11Event( XEvent *event )
{
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
    if (event->type == ClientMessage &&
	event->xclient.message_type ==
		XInternAtom( tqt_xdisplay(), "VPO_RESIZE_NOTIFY", False ))
    {
	resizeNotify( event->xclient.data.l[0], event->xclient.data.l[1] );
    }
#endif
    return false;
}

void KVideoWidget::fullscreenActivated()
{
    if (isFullscreen() == (fullscreenWidget != 0))
	return;

    if (isFullscreen())
    {
	fullscreenWidget = new KFullscreenVideoWidget( this );

	// Interconnect mouse button signals
	connect( fullscreenWidget, TQ_SIGNAL(mouseButtonPressed( int, const TQPoint &, int )),
		 this, TQ_SIGNAL(mouseButtonPressed( int, const TQPoint &, int)) );

	connect( fullscreenWidget, TQ_SIGNAL(mouseButtonDoubleClick( const TQPoint &, int )),
		 this, TQ_SIGNAL(mouseButtonDoubleClick( const TQPoint &, int )) );

	// ### Remove in KDE4
	 connect( fullscreenWidget, TQ_SIGNAL(rightButtonPressed(const TQPoint &)),
		this, TQ_SIGNAL(rightButtonPressed(const TQPoint &)) );
		 
	// Leave fullscreen mode with <Escape> key
	TQAccel *a = new TQAccel( fullscreenWidget );
	a->connectItem( a->insertItem( Key_Escape ),
			this, TQ_SLOT(setWindowed()) );

	fullscreenWidget->setFocus();
	fullscreenWidget->showFullScreen();

	if (isEmbedded())
	{
	    poVideo.x11WindowId( fullscreenWidget->winId() );
	    fullscreenWidget->setBackgroundMode( NoBackground );
	}
    }
    else
    {
	if (isEmbedded())
	{
	    poVideo.x11WindowId( winId() );
	    setBackgroundMode( NoBackground );
	}

	delete fullscreenWidget;
	fullscreenWidget = 0;
    }
}

void KVideoWidget::halfSizeActivated()
{
    if (isHalfSize())
    {
	if(isEmbedded()) emit adaptSize( (videoWidth / 2), (videoHeight / 2) );
	setWindowed();
    }
}

void KVideoWidget::normalSizeActivated()
{
    if (isNormalSize())
    {
	if(isEmbedded()) emit adaptSize( videoWidth, videoHeight );
	setWindowed();
    }
}

void KVideoWidget::doubleSizeActivated()
{
    if (isDoubleSize())
    {
	if(isEmbedded()) emit adaptSize( (2 * videoWidth), (2 * videoHeight) );
	setWindowed();
    }
}

#include "kvideowidget.moc"

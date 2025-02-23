/*
   This file is part of KDE/aRts (Noatun) - xine integration
   Copyright (C) 2002 Ewald Snel <ewald@rambo.its.tudelft.nl>
   Copyright (C) 2002 Neil Stevens <neil@qualityassistant.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.
*/

#ifndef ARTSKDE_KVIDEOWIDGET_H
#define ARTSKDE_KVIDEOWIDGET_H

#include <tqevent.h>
#include <tqimage.h>
#include <tqwidget.h>
#include <kmedia2.h>
#include <kxmlguiclient.h>


class KDE_ARTS_EXPORT KVideoWidget : public TQWidget, virtual public KXMLGUIClient
{
TQ_OBJECT

public:
    KVideoWidget( KXMLGUIClient *clientParent, TQWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    KVideoWidget( TQWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    virtual ~KVideoWidget();

    void embed( Arts::VideoPlayObject vpo );
    static TQImage snapshot( Arts::VideoPlayObject vpo );

    bool isEmbedded();
    bool isFullscreen();
    bool isHalfSize();
    bool isNormalSize();
    bool isDoubleSize();

    TQSize sizeHint() const;

    virtual int heightForWidth ( int w ) const;

protected:
    virtual void mousePressEvent( TQMouseEvent *event );
    virtual void mouseDoubleClickEvent( TQMouseEvent *event);
    virtual void resizeEvent( TQResizeEvent *event );
    virtual bool x11Event( XEvent *event );

public slots:
    void setFullscreen();
    void setWindowed();
    void setHalfSize();
    void setNormalSize();
    void setDoubleSize();

    void resizeNotify( int width, int height );

protected slots:
    void fullscreenActivated();
    void halfSizeActivated();
    void normalSizeActivated();
    void doubleSizeActivated();

signals:
    void adaptSize( int width, int height );
    void mouseButtonPressed( int type, const TQPoint &, int state );
    void mouseButtonDoubleClick( const TQPoint &, int state );
    /**
     * @deprecated
     * use mouseButtonPressed( int type, const TQPoint & ) instead.
     */
    void rightButtonPressed( const TQPoint & );

private:
    void init(void);
    TQWidget *fullscreenWidget;
    int videoWidth;
    int videoHeight;
    Arts::VideoPlayObject poVideo;
};

#endif

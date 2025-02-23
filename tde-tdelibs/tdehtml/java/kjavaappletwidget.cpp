/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Richard Moore <rich@kde.org>
 *               2000 Wynn Wilkes <wynnw@caldera.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kjavaappletwidget.h"
#include "kjavaappletserver.h"

#include <twin.h>
#include <kdebug.h>
#include <tdelocale.h>

#include <tqlabel.h>


// For future expansion
class KJavaAppletWidgetPrivate
{
friend class KJavaAppletWidget;
private:
    TQLabel* tmplabel;
};

int KJavaAppletWidget::appletCount = 0;

KJavaAppletWidget::KJavaAppletWidget( TQWidget* parent, const char* name )
   : QXEmbed ( parent, name)
{
    setProtocol(QXEmbed::XPLAIN);

    m_applet = new KJavaApplet( this );
    d        = new KJavaAppletWidgetPrivate;
    m_kwm    = new KWinModule( this );

    d->tmplabel = new TQLabel( this );
    d->tmplabel->setText( KJavaAppletServer::getAppletLabel() );
    d->tmplabel->setAlignment( TQt::AlignCenter | TQt::WordBreak );
    d->tmplabel->setFrameStyle( TQFrame::StyledPanel | TQFrame::Sunken );
    d->tmplabel->show();

    m_swallowTitle.sprintf( "KJAS Applet - Ticket number %u", appletCount++ );
    m_applet->setWindowName( m_swallowTitle );
}

KJavaAppletWidget::~KJavaAppletWidget()
{
    delete m_applet;
    delete d;
}

void KJavaAppletWidget::showApplet()
{
    connect( m_kwm, TQ_SIGNAL( windowAdded( WId ) ),
	         this,  TQ_SLOT( setWindow( WId ) ) );

    m_kwm->doNotManage( m_swallowTitle );

    //Now we send applet info to the applet server
    if ( !m_applet->isCreated() )
        m_applet->create();
}

void KJavaAppletWidget::setWindow( WId w )
{
    //make sure that this window has the right name, if so, embed it...
    KWin::WindowInfo w_info = KWin::windowInfo( w );
    if ( m_swallowTitle == w_info.name() ||
         m_swallowTitle == w_info.visibleName() )
    {
        kdDebug(6100) << "swallowing our window: " << m_swallowTitle
                      << ", window id = " << w << endl;
        delete d->tmplabel;
        d->tmplabel = 0;

        // disconnect from KWM events
        disconnect( m_kwm, TQ_SIGNAL( windowAdded( WId ) ),
                    this,  TQ_SLOT( setWindow( WId ) ) );


        embed( w );
        setFocus();
    }
}

TQSize KJavaAppletWidget::sizeHint() const
{
    kdDebug(6100) << "KJavaAppletWidget::sizeHint()" << endl;
    TQSize rval = QXEmbed::sizeHint();

    if( rval.width() == 0 || rval.height() == 0 )
    {
        if( width() != 0 && height() != 0 )
        {
            rval = TQSize( width(), height() );
        }
    }

    kdDebug(6100) << "returning: (" << rval.width() << ", " << rval.height() << ")" << endl;

    return rval;
}

void KJavaAppletWidget::resize( int w, int h )
{
    if( d->tmplabel )
    {
        d->tmplabel->resize( w, h );
        m_applet->setSize( TQSize( w, h ) );
    }

    QXEmbed::resize( w, h );
}

void KJavaAppletWidget::showEvent (TQShowEvent * e) {
    QXEmbed::showEvent(e);
    if (!applet()->isCreated() && !applet()->appletClass().isEmpty()) {
        // delayed showApplet
        if (applet()->size().width() <= 0)
            applet()->setSize (sizeHint());
        showApplet();
    }
}

#include "kjavaappletwidget.moc"

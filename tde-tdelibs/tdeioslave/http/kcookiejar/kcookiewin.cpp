/*
This file is part of KDE

  Copyright (C) 2000- Waldo Bastian <bastian@kde.org>
  Copyright (C) 2000- Dawit Alemayehu <adawit@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//----------------------------------------------------------------------------
//
// KDE File Manager -- HTTP Cookie Dialogs
// $Id$

// The purpose of the TQT_NO_TOOLTIP and TQT_NO_WHATSTHIS ifdefs is because
// this file is also used in Konqueror/Embedded. One of the aims of
// Konqueror/Embedded is to be a small as possible to fit on embedded
// devices. For this it's also useful to strip out unneeded features of
// Qt, like for example TQToolTip or TQWhatsThis. The availability (or the
// lack thereof) can be determined using these preprocessor defines.
// The same applies to the TQT_NO_ACCEL ifdef below. I hope it doesn't make
// too much trouble... (Simon)

#include <tqhbox.h>
#include <tqvbox.h>
#include <tqaccel.h>
#include <tqlabel.h>
#include <tqwidget.h>
#include <tqlayout.h>
#include <tqgroupbox.h>
#include <tqdatetime.h>
#include <tqmessagebox.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqvbuttongroup.h>

#ifndef TQT_NO_TOOLTIP
#include <tqtooltip.h>
#endif

#ifndef TQT_NO_WHATSTHIS
#include <tqwhatsthis.h>
#endif

#include <kidna.h>
#include <twin.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <kurllabel.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <tdeapplication.h>

#ifdef TQ_WS_X11
#include <X11/Xlib.h>
#endif

#include "kcookiejar.h"
#include "kcookiewin.h"

KCookieWin::KCookieWin( TQWidget *parent, KHttpCookieList cookieList,
                        int defaultButton, bool showDetails )
           :KDialog( parent, "cookiealert", true )
{
#ifndef TQ_WS_QWS //FIXME(E): Implement for Qt Embedded
    setCaption( i18n("Cookie Alert") );
    setIcon( SmallIcon("cookie") );
    // all cookies in the list should have the same window at this time, so let's take the first
# ifdef TQ_WS_X11
    if( cookieList.first()->windowIds().count() > 0 )
    {
        XSetTransientForHint( tqt_xdisplay(), winId(), cookieList.first()->windowIds().first());
    }
    else
    {
        // No window associated... make sure the user notices our dialog.
        KWin::setState( winId(), NET::KeepAbove );
        kapp->updateUserTimestamp();
    }
# endif
#endif
    // Main widget's layout manager...
    TQVBoxLayout* vlayout = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );
    vlayout->setResizeMode( TQLayout::Fixed );

    // Cookie image and message to user
    TQHBox* hBox = new TQHBox( this );
    hBox->setSpacing( KDialog::spacingHint() );
    TQLabel* icon = new TQLabel( hBox );
    icon->setPixmap( TQMessageBox::standardIcon(TQMessageBox::Warning) );
    icon->setAlignment( TQt::AlignCenter );
    icon->setFixedSize( 2*icon->sizeHint() );

    int count = cookieList.count();

    TQVBox* vBox = new TQVBox( hBox );
    TQString txt = i18n("You received a cookie from",
                       "You received %n cookies from", count);
    TQLabel* lbl = new TQLabel( txt, vBox );
    lbl->setAlignment( TQt::AlignCenter );
    KHttpCookiePtr cookie = cookieList.first();

    TQString host (cookie->host());
    int pos = host.find(':');
    if ( pos > 0 )
    {
      TQString portNum = host.left(pos);
      host.remove(0, pos+1);
      host += ':';
      host += portNum;
    }

    txt = TQString("<b>%1</b>").arg( KIDNA::toUnicode(host) );
    if (cookie->isCrossDomain())
       txt += i18n(" <b>[Cross Domain!]</b>");
    lbl = new TQLabel( txt, vBox );
    lbl->setAlignment( TQt::AlignCenter );
    lbl = new TQLabel( i18n("Do you want to accept or reject?"), vBox );
    lbl->setAlignment( TQt::AlignCenter );
    vlayout->addWidget( hBox, 0, TQt::AlignLeft );

    // Cookie Details dialog...
    m_detailView = new KCookieDetail( cookieList, count, this );
    vlayout->addWidget( m_detailView );
    m_showDetails = showDetails;
    m_showDetails ? m_detailView->show():m_detailView->hide();

    // Cookie policy choice...
    m_btnGrp = new TQVButtonGroup( i18n("Apply Choice To"), this );
    m_btnGrp->setRadioButtonExclusive( true );

    txt = (count == 1)? i18n("&Only this cookie") : i18n("&Only these cookies");
    TQRadioButton* rb = new TQRadioButton( txt, m_btnGrp );
#ifndef TQT_NO_WHATSTHIS
    TQWhatsThis::add( rb, i18n("Select this option to accept/reject only this cookie. "
                              "You will be prompted if another cookie is received. "
                              "<em>(see WebBrowsing/Cookies in the Control Center)</em>." ) );
#endif
    m_btnGrp->insert( rb );
    rb = new TQRadioButton( i18n("All cookies from this do&main"), m_btnGrp );
#ifndef TQT_NO_WHATSTHIS
    TQWhatsThis::add( rb, i18n("Select this option to accept/reject all cookies from "
                              "this site. Choosing this option will add a new policy for "
                              "the site this cookie originated from. This policy will be "
                              "permanent until you manually change it from the Control Center "
                              "<em>(see WebBrowsing/Cookies in the Control Center)</em>.") );
#endif
    m_btnGrp->insert( rb );
    rb = new TQRadioButton( i18n("All &cookies"), m_btnGrp );
#ifndef TQT_NO_WHATSTHIS
    TQWhatsThis::add( rb, i18n("Select this option to accept/reject all cookies from "
                              "anywhere. Choosing this option will change the global "
                              "cookie policy set in the Control Center for all cookies "
                              "<em>(see WebBrowsing/Cookies in the Control Center)</em>.") );
#endif
    m_btnGrp->insert( rb );
    vlayout->addWidget( m_btnGrp );

    if ( defaultButton > -1 && defaultButton < 3 )
        m_btnGrp->setButton( defaultButton );
    else
        m_btnGrp->setButton( 1 );

    // Accept/Reject buttons
    TQWidget* bbox = new TQWidget( this );
    TQBoxLayout* bbLay = new TQHBoxLayout( bbox );
    bbLay->setSpacing( KDialog::spacingHint() );
    TQPushButton* btn = new TQPushButton( i18n("&Accept"), bbox );
    btn->setDefault( true );
    btn->setFocus();
    connect( btn, TQ_SIGNAL(clicked()), TQ_SLOT(accept()) );
    bbLay->addWidget( btn );
    btn = new TQPushButton( i18n("&Reject"), bbox );
    connect( btn, TQ_SIGNAL(clicked()), TQ_SLOT(reject()) );
    bbLay->addWidget( btn );
    bbLay->addStretch( 1 );
#ifndef TQT_NO_ACCEL
    TQAccel* a = new TQAccel( this );
    a->connectItem( a->insertItem(TQt::Key_Escape), btn, TQ_SLOT(animateClick()) );
#endif

    m_button = new TQPushButton( bbox );
    m_button->setText( m_showDetails ? i18n("&Details <<"):i18n("&Details >>") );
    connect( m_button, TQ_SIGNAL(clicked()), TQ_SLOT(slotCookieDetails()) );
    bbLay->addWidget( m_button );
#ifndef TQT_NO_WHATSTHIS
    TQWhatsThis::add( m_button, i18n("See or modify the cookie information") );
#endif


    vlayout->addWidget( bbox );
    setFixedSize( sizeHint() );
}

KCookieWin::~KCookieWin()
{
}

void KCookieWin::slotCookieDetails()
{
    if ( m_detailView->isVisible() )
    {
        m_detailView->setMaximumSize( 0, 0 );
        m_detailView->adjustSize();
        m_detailView->hide();
        m_button->setText( i18n( "&Details >>" ) );
        m_showDetails = false;
    }
    else
    {
        m_detailView->setMaximumSize( 1000, 1000 );
        m_detailView->adjustSize();
        m_detailView->show();
        m_button->setText( i18n( "&Details <<" ) );
        m_showDetails = true;
    }
}

KCookieAdvice KCookieWin::advice( KCookieJar *cookiejar, KHttpCookie* cookie )
{
    int result = exec();
    
    cookiejar->setShowCookieDetails ( m_showDetails );
    
    KCookieAdvice advice = (result==TQDialog::Accepted) ? KCookieAccept:KCookieReject;
    
    int preferredPolicy = m_btnGrp->id( m_btnGrp->selected() );
    cookiejar->setPreferredDefaultPolicy( preferredPolicy );
    
    switch ( preferredPolicy )
    {
        case 2:
            cookiejar->setGlobalAdvice( advice );
            break;
        case 1:
            cookiejar->setDomainAdvice( cookie, advice );
            break;
        case 0:
        default:
            break;
    }
    return advice;
}

KCookieDetail::KCookieDetail( KHttpCookieList cookieList, int cookieCount,
                              TQWidget* parent, const char* name )
              :TQGroupBox( parent, name )
{
    setTitle( i18n("Cookie Details") );
    TQGridLayout* grid = new TQGridLayout( this, 9, 2,
                                         KDialog::spacingHint(),
                                         KDialog::marginHint() );
    grid->addRowSpacing( 0, fontMetrics().lineSpacing() );
    grid->setColStretch( 1, 3 );

    TQLabel* label = new TQLabel( i18n("Name:"), this );
    grid->addWidget( label, 1, 0 );
    m_name = new KLineEdit( this );
    m_name->setReadOnly( true );
    m_name->setMaximumWidth( fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_name, 1 ,1 );

    //Add the value
    label = new TQLabel( i18n("Value:"), this );
    grid->addWidget( label, 2, 0 );
    m_value = new KLineEdit( this );
    m_value->setReadOnly( true );
    m_value->setMaximumWidth( fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_value, 2, 1);

    label = new TQLabel( i18n("Expires:"), this );
    grid->addWidget( label, 3, 0 );
    m_expires = new KLineEdit( this );
    m_expires->setReadOnly( true );
    m_expires->setMaximumWidth(fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_expires, 3, 1);

    label = new TQLabel( i18n("Path:"), this );
    grid->addWidget( label, 4, 0 );
    m_path = new KLineEdit( this );
    m_path->setReadOnly( true );
    m_path->setMaximumWidth( fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_path, 4, 1);

    label = new TQLabel( i18n("Domain:"), this );
    grid->addWidget( label, 5, 0 );
    m_domain = new KLineEdit( this );
    m_domain->setReadOnly( true );
    m_domain->setMaximumWidth( fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_domain, 5, 1);

    label = new TQLabel( i18n("Exposure:"), this );
    grid->addWidget( label, 6, 0 );
    m_secure = new KLineEdit( this );
    m_secure->setReadOnly( true );
    m_secure->setMaximumWidth( fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_secure, 6, 1 );

    if ( cookieCount > 1 )
    {
        TQPushButton* btnNext = new TQPushButton( i18n("Next cookie","&Next >>"), this );
        btnNext->setFixedSize( btnNext->sizeHint() );
        grid->addMultiCellWidget( btnNext, 8, 8, 0, 1 );
        connect( btnNext, TQ_SIGNAL(clicked()), TQ_SLOT(slotNextCookie()) );
#ifndef TQT_NO_TOOLTIP
        TQToolTip::add( btnNext, i18n("Show details of the next cookie") );
#endif
    }
    m_cookieList = cookieList;
    m_cookie = 0;
    slotNextCookie();
}

KCookieDetail::~KCookieDetail()
{
}

void KCookieDetail::slotNextCookie()
{
    KHttpCookiePtr cookie = m_cookieList.first();
    if (m_cookie) while(cookie)
    {
       if (cookie == m_cookie)
       {
          cookie = m_cookieList.next();
          break;
       }
       cookie = m_cookieList.next();
    }
    m_cookie = cookie;
    if (!m_cookie)
        m_cookie = m_cookieList.first();

    if ( m_cookie )
    {
        m_name->setText( m_cookie->name() );
        m_value->setText( ( m_cookie->value() ) );
        if ( m_cookie->domain().isEmpty() )
          m_domain->setText( i18n("Not specified") );
        else
          m_domain->setText( m_cookie->domain() );
        m_path->setText( m_cookie->path() );
        TQDateTime cookiedate;
        cookiedate.setTime_t( m_cookie->expireDate() );
        if ( m_cookie->expireDate() )
          m_expires->setText( TDEGlobal::locale()->formatDateTime(cookiedate) );
        else
          m_expires->setText( i18n("End of Session") );
        TQString sec;
        if (m_cookie->isSecure())
        {
          if (m_cookie->isHttpOnly())
            sec = i18n("Secure servers only");
          else
            sec = i18n("Secure servers, page scripts");
        }
        else
        {
          if (m_cookie->isHttpOnly())
            sec = i18n("Servers");
          else
            sec = i18n("Servers, page scripts");
        }
        m_secure->setText( sec );
    }
}

#include "kcookiewin.moc"

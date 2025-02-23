/* This file is part of the KDE libraries
    Copyright (C) 2000 Wilco Greven <greven@kde.org>

    library is free software; you can redistribute it and/or
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


#include <sys/stat.h>
#include <unistd.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqstring.h>
#include <tqtoolbutton.h>

#include <tdeaccel.h>
#include <tdefiledialog.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <tderecentdocument.h>
#include <kurl.h>
#include <kurlrequester.h>

#include "kurlrequesterdlg.h"


KURLRequesterDlg::KURLRequesterDlg( const TQString& urlName, TQWidget *parent,
        const char *name, bool modal )
    :   KDialogBase( Plain, TQString::null, Ok|Cancel|User1, Ok, parent, name,
                modal, true, KStdGuiItem::clear() )
{
  initDialog(i18n( "Location:" ), urlName);
}

KURLRequesterDlg::KURLRequesterDlg( const TQString& urlName, const TQString& _text, TQWidget *parent, const char *name, bool modal )
    :   KDialogBase( Plain, TQString::null, Ok|Cancel|User1, Ok, parent, name,
                modal, true, KStdGuiItem::clear() )
{
  initDialog(_text, urlName);
}

KURLRequesterDlg::~KURLRequesterDlg()
{
}

void KURLRequesterDlg::initDialog(const TQString &text,const TQString &urlName)
{
   TQVBoxLayout * topLayout = new TQVBoxLayout( plainPage(), 0,
            spacingHint() );

    TQLabel * label = new TQLabel( text , plainPage() );
    topLayout->addWidget( label );

    urlRequester_ = new KURLRequester( urlName, plainPage(), "urlRequester" );
    urlRequester_->setMinimumWidth( urlRequester_->sizeHint().width() * 3 );
    topLayout->addWidget( urlRequester_ );
    urlRequester_->setFocus();
    connect( urlRequester_->lineEdit(), TQ_SIGNAL(textChanged(const TQString&)),
             TQ_SLOT(slotTextChanged(const TQString&)) );
    bool state = !urlName.isEmpty();
    enableButtonOK( state );
    enableButton( KDialogBase::User1, state );
    /*
    KFile::Mode mode = static_cast<KFile::Mode>( KFile::File |
            KFile::ExistingOnly );
	urlRequester_->setMode( mode );
    */
    connect( this, TQ_SIGNAL( user1Clicked() ), TQ_SLOT( slotClear() ) );
}

void KURLRequesterDlg::slotTextChanged(const TQString & text)
{
    bool state = !text.stripWhiteSpace().isEmpty();
    enableButtonOK( state );
    enableButton( KDialogBase::User1, state );
}

void KURLRequesterDlg::slotClear()
{
    urlRequester_->clear();
}

KURL KURLRequesterDlg::selectedURL() const
{
    if ( result() == TQDialog::Accepted )
        return KURL::fromPathOrURL( urlRequester_->url() );
    else
        return KURL();
}


KURL KURLRequesterDlg::getURL(const TQString& dir, TQWidget *parent,
        const TQString& caption)
{
    KURLRequesterDlg dlg(dir, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);

    dlg.exec();

    const KURL& url = dlg.selectedURL();
    if (url.isValid())
        TDERecentDocument::add(url);

    return url;
}

KFileDialog * KURLRequesterDlg::fileDialog()
{
    return urlRequester_->fileDialog();
}

KURLRequester * KURLRequesterDlg::urlRequester()
{
    return urlRequester_;
}

#include "kurlrequesterdlg.moc"

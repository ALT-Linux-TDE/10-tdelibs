/*
 * This file is part of the KDE project
 * Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
 *               2001 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqcheckbox.h>
#include <tqwhatsthis.h>
#include <tqtimer.h>

#include <tdeapplication.h>
#include <tdeconfig.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdefiledialog.h>
#include <tdefileitem.h>
#include <tdeio/previewjob.h>

#include "kimagefilepreview.h"
#include "config-tdefile.h"

/**** KImageFilePreview ****/

KImageFilePreview::KImageFilePreview( TQWidget *parent )
    : KPreviewWidgetBase( parent ),
      m_job( 0L )
{
    TDEConfig *config = TDEGlobal::config();
    TDEConfigGroupSaver cs( config, ConfigGroup );
    autoMode = config->readBoolEntry( "Automatic Preview", true );

    TQVBoxLayout *vb = new TQVBoxLayout( this, 0, KDialog::spacingHint() );

    imageLabel = new TQLabel( this );
    imageLabel->setFrameStyle( TQFrame::NoFrame );
    imageLabel->setAlignment( TQt::AlignHCenter | TQt::AlignVCenter );
    imageLabel->setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Expanding) );
    vb->addWidget( imageLabel );

    TQHBoxLayout *hb = new TQHBoxLayout( 0 );
    vb->addLayout( hb );

    autoPreview = new TQCheckBox( i18n("&Automatic preview"), this );
    autoPreview->setChecked( autoMode );
    hb->addWidget( autoPreview );
    connect( autoPreview, TQ_SIGNAL(toggled(bool)), TQ_SLOT(toggleAuto(bool)) );

    previewButton = new KPushButton( SmallIconSet("thumbnail"), i18n("&Preview"), this );
    hb->addWidget( previewButton );
    connect( previewButton, TQ_SIGNAL(clicked()), TQ_SLOT(showPreview()) );

    timer = new TQTimer( this );
    connect( timer, TQ_SIGNAL(timeout()), TQ_SLOT(showPreview()) );

    setSupportedMimeTypes( TDEIO::PreviewJob::supportedMimeTypes() );
}

KImageFilePreview::~KImageFilePreview()
{
    if ( m_job )
        m_job->kill();

    TDEConfig *config = TDEGlobal::config();
    TDEConfigGroupSaver cs( config, ConfigGroup );
    config->writeEntry( "Automatic Preview", autoPreview->isChecked() );
}

void KImageFilePreview::showPreview()
{
    // Pass a copy since clearPreview() will clear currentURL
    KURL url = currentURL;
    showPreview( url, true );
}

// called via KPreviewWidgetBase interface
void KImageFilePreview::showPreview( const KURL& url )
{
    showPreview( url, false );
}

void KImageFilePreview::showPreview( const KURL &url, bool force )
{
    if ( !url.isValid() ) {
        clearPreview();
        return;
    }

    if ( url != currentURL || force )
    {
        clearPreview();
	currentURL = url;

	if ( autoMode || force )
	{
            int w = imageLabel->contentsRect().width() - 4;
            int h = imageLabel->contentsRect().height() - 4;

            m_job =  createJob( url, w, h );
            if ( force ) // explicitly requested previews shall always be generated!
                m_job->setIgnoreMaximumSize( true );
            
            connect( m_job, TQ_SIGNAL( result( TDEIO::Job * )),
                     this, TQ_SLOT( slotResult( TDEIO::Job * )));
            connect( m_job, TQ_SIGNAL( gotPreview( const KFileItem*,
                                                const TQPixmap& )),
                     TQ_SLOT( gotPreview( const KFileItem*, const TQPixmap& ) ));

            connect( m_job, TQ_SIGNAL( failed( const KFileItem* )),
                     this, TQ_SLOT( slotFailed( const KFileItem* ) ));
	}
    }
}

void KImageFilePreview::toggleAuto( bool a )
{
    autoMode = a;
    if ( autoMode )
    {
        // Pass a copy since clearPreview() will clear currentURL
        KURL url = currentURL;
        showPreview( url, true );
    }
}

void KImageFilePreview::resizeEvent( TQResizeEvent * )
{
    timer->start( 100, true ); // forces a new preview
}

TQSize KImageFilePreview::sizeHint() const
{
    return TQSize( 20, 200 ); // otherwise it ends up huge???
}

TDEIO::PreviewJob * KImageFilePreview::createJob( const KURL& url, int w, int h )
{
    KURL::List urls;
    urls.append( url );
    return TDEIO::filePreview( urls, w, h, 0, 0, true, false );
}

void KImageFilePreview::gotPreview( const KFileItem* item, const TQPixmap& pm )
{
    if ( item->url() == currentURL ) // should always be the case
        imageLabel->setPixmap( pm );
}

void KImageFilePreview::slotFailed( const KFileItem* item )
{
    if ( item->isDir() )
        imageLabel->clear();
    else if ( item->url() == currentURL ) // should always be the case
        imageLabel->setPixmap( SmallIcon( "file_broken", TDEIcon::SizeLarge,
                                          TDEIcon::DisabledState ));
}

void KImageFilePreview::slotResult( TDEIO::Job *job )
{
    if ( job == m_job )
        m_job = 0L;
}

void KImageFilePreview::clearPreview()
{
    if ( m_job ) {
        m_job->kill();
        m_job = 0L;
    }

    imageLabel->clear();
    currentURL = KURL();
}

void KImageFilePreview::virtual_hook( int id, void* data )
{ KPreviewWidgetBase::virtual_hook( id, data ); }

#include "kimagefilepreview.moc"

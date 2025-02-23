#include "tdefileaudiopreview.h"

#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqlayout.h>
#include <tqvgroupbox.h>

#include <tdeglobal.h>
#include <tdeconfig.h>
#include <klibloader.h>
#include <tdelocale.h>
#include <tdemediaplayer/player.h>
#include <kmimetype.h>
#include <tdeparts/componentfactory.h>

#include <kplayobjectfactory.h>

#include <config-tdefile.h>

class KFileAudioPreviewFactory : public KLibFactory
{
protected:
    virtual TQObject *createObject( TQObject *parent, const char *name,
                           const char *className, const TQStringList & args)
    {
        Q_UNUSED(className);
        Q_UNUSED(args);
        return new KFileAudioPreview( dynamic_cast<TQWidget*>( parent ), name );
    }
};

K_EXPORT_COMPONENT_FACTORY( tdefileaudiopreview, KFileAudioPreviewFactory )


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


class KFileAudioPreview::KFileAudioPreviewPrivate
{
public:
    KFileAudioPreviewPrivate( TQWidget *parent )
    {
        player = KParts::ComponentFactory::createInstanceFromQuery<KMediaPlayer::Player>( "KMediaPlayer/Player", TQString(), parent );
    }

    ~KFileAudioPreviewPrivate()
    {
        delete player;
    }

    KMediaPlayer::Player *player;
};


KFileAudioPreview::KFileAudioPreview( TQWidget *parent, const char *name )
    : KPreviewWidgetBase( parent, name )
{
    TDEGlobal::locale()->insertCatalogue("tdefileaudiopreview");    

    TQStringList formats = KDE::PlayObjectFactory::mimeTypes();
    // ###
    TQStringList::ConstIterator it = formats.begin();
    for ( ; it != formats.end(); ++it )
        m_supportedFormats.insert( *it, (void*) 1 );

    TQVGroupBox *box = new TQVGroupBox( i18n("Media Player"), this );
    TQVBoxLayout *layout = new TQVBoxLayout( this );
    layout->addWidget( box );

    (void) new TQWidget( box ); // spacer

    d = new KFileAudioPreviewPrivate( 0L ); // not box -- being reparented anyway
    if ( d->player ) // only if there actually is a component...
    {
        setSupportedMimeTypes( formats );
        KMediaPlayer::View *view = d->player->view();
        view->setEnabled( false );

        // if we have access to the video widget, show it above the player
        // So, reparent first the video widget, then the view.
        if ( view->videoWidget() )
        {
            TQHBox *frame = new TQHBox( box );
            frame->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
            frame->setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Expanding ) );
            view->videoWidget()->reparent( frame, TQPoint(0,0) );
        }

        view->reparent( box, TQPoint(0,0) );
    }

    m_autoPlay = new TQCheckBox( i18n("Play &automatically"), box );
    TDEConfigGroup config( TDEGlobal::config(), ConfigGroup );
    m_autoPlay->setChecked( config.readBoolEntry( "Autoplay sounds", true ) );
    connect( m_autoPlay, TQ_SIGNAL(toggled(bool)), TQ_SLOT(toggleAuto(bool)) );
}

KFileAudioPreview::~KFileAudioPreview()
{
    TDEConfigGroup config( TDEGlobal::config(), ConfigGroup );
    config.writeEntry( "Autoplay sounds", m_autoPlay->isChecked() );

    delete d;
}

void KFileAudioPreview::showPreview( const KURL &url )
{
    if ( !d->player || !url.isValid() )
        return;

    KMimeType::Ptr mt = KMimeType::findByURL( url );
    bool supported = m_supportedFormats.find( mt->name() );
    d->player->view()->setEnabled( supported );
    if ( !supported )
        return;

    static_cast<KParts::ReadOnlyPart*>(d->player)->openURL( url );
    if ( m_autoPlay->isChecked() )
        d->player->play();
}

void KFileAudioPreview::clearPreview()
{
    if ( d->player )
    {
        d->player->stop();
        d->player->closeURL();
    }
}

void KFileAudioPreview::toggleAuto( bool on )
{
    if ( !d->player )
        return;

    if ( on && m_currentURL.isValid() && d->player->view()->isEnabled() )
        d->player->play();
    else
        d->player->stop();
}

void KFileAudioPreview::virtual_hook( int, void* )
{}

#include "tdefileaudiopreview.moc"

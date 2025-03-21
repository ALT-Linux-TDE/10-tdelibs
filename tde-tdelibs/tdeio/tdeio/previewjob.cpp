/*  This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>
                  2000 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Malte Starostik <malte.starostik@t-online.de>

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

#include "previewjob.h"

#include <sys/stat.h>
#ifdef __FreeBSD__
    #include <machine/param.h>
#endif
#include <sys/types.h>

#ifdef Q_OS_UNIX
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include <tqdir.h>
#include <tqfile.h>
#include <tqimage.h>
#include <tqtimer.h>
#include <tqregexp.h>

#include <kdatastream.h> // Do not remove, needed for correct bool serialization
#include <tdefileitem.h>
#include <tdeapplication.h>
#include <tdetempfile.h>
#include <ktrader.h>
#include <kmdcodec.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>

#include <tdeio/kservice.h>

#include "previewjob.moc"

namespace TDEIO { struct PreviewItem; }
using namespace TDEIO;

struct TDEIO::PreviewItem
{
    KFileItem *item;
    KService::Ptr plugin;
};

struct TDEIO::PreviewJobPrivate
{
    enum { STATE_STATORIG, // if the thumbnail exists
           STATE_GETORIG, // if we create it
           STATE_CREATETHUMB // thumbnail:/ slave
    } state;
    KFileItemList initialItems;
    const TQStringList *enabledPlugins;
    // Our todo list :)
    TQValueList<PreviewItem> items;
    // The current item
    PreviewItem currentItem;
    // The modification time of that URL
    time_t tOrig;
    // Path to thumbnail cache for the current size
    TQString thumbPath;
    // Original URL of current item in TMS format
    // (file:///path/to/file instead of file:/path/to/file)
    TQString origName;
    // Thumbnail file name for current item
    TQString thumbName;
    // Size of thumbnail
    int width;
    int height;
    // Unscaled size of thumbnail (128 or 256 if cache is enabled)
    int cacheWidth;
    int cacheHeight;
    // Whether the thumbnail should be scaled
    bool bScale;
    // Whether we should save the thumbnail
    bool bSave;
    // If the file to create a thumb for was a temp file, this is its name
    TQString tempName;
    // Over that, it's too much
    unsigned long maximumSize;
    // the size for the icon overlay
    int iconSize;
    // the transparency of the blended mimetype icon
    int iconAlpha;
	// Shared memory segment Id. The segment is allocated to a size
	// of extent x extent x 4 (32 bit image) on first need.
	int shmid;
	// And the data area
	uchar *shmaddr;
    // Delete the KFileItems when done?
    bool deleteItems;
    bool succeeded;
    // Root of thumbnail cache
    TQString thumbRoot;
    bool ignoreMaximumSize;
    TQTimer startPreviewTimer;
};

PreviewJob::PreviewJob( const KFileItemList &items, int width, int height,
    int iconSize, int iconAlpha, bool scale, bool save,
    const TQStringList *enabledPlugins, bool deleteItems )
    : TDEIO::Job( false /* no GUI */ )
{
    d = new PreviewJobPrivate;
    d->tOrig = 0;
    d->shmid = -1;
    d->shmaddr = 0;
    d->initialItems = items;
    d->enabledPlugins = enabledPlugins;
    d->width = width;
    d->height = height ? height : width;
    d->cacheWidth = d->width;
    d->cacheHeight = d->height;
    d->iconSize = iconSize;
    d->iconAlpha = iconAlpha;
    d->deleteItems = deleteItems;
    d->bScale = scale;
    d->bSave = save && scale;
    d->succeeded = false;
    d->currentItem.item = 0;
    d->thumbRoot = TQDir::homeDirPath() + "/.thumbnails/";
    d->ignoreMaximumSize = false;

    // Return to event loop first, determineNextFile() might delete this;
    connect(&d->startPreviewTimer, TQ_SIGNAL(timeout()), TQ_SLOT(startPreview()) );
    d->startPreviewTimer.start(0, true);
}

PreviewJob::~PreviewJob()
{
#ifdef Q_OS_UNIX
    if (d->shmaddr) {
        shmdt((char*)d->shmaddr);
        shmctl(d->shmid, IPC_RMID, 0);
    }
#endif
    delete d;
}

void PreviewJob::startPreview()
{
    // Load the list of plugins to determine which mimetypes are supported
    TDETrader::OfferList plugins = TDETrader::self()->query("ThumbCreator");
    TQMap<TQString, KService::Ptr> mimeMap;

    for (TDETrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        if (!d->enabledPlugins || d->enabledPlugins->contains((*it)->desktopEntryName()))
    {
        TQStringList mimeTypes = (*it)->property("MimeTypes").toStringList();
        for (TQStringList::ConstIterator mt = mimeTypes.begin(); mt != mimeTypes.end(); ++mt)
            mimeMap.insert(*mt, *it);
    }

    // Look for images and store the items in our todo list :)
    bool bNeedCache = false;
    for (KFileItemListIterator it(d->initialItems); it.current(); ++it )
    {
        PreviewItem item;
        item.item = it.current();
        TQMap<TQString, KService::Ptr>::ConstIterator plugin = mimeMap.find(it.current()->mimetype());
        if (plugin == mimeMap.end()
            && (it.current()->mimetype() != "application/x-desktop")
            && (it.current()->mimetype() != "media/builtin-mydocuments")
            && (it.current()->mimetype() != "media/builtin-mycomputer")
            && (it.current()->mimetype() != "media/builtin-mynetworkplaces")
            && (it.current()->mimetype() != "media/builtin-printers")
            && (it.current()->mimetype() != "media/builtin-trash")
            && (it.current()->mimetype() != "media/builtin-webbrowser"))
        {
            TQString mimeType = it.current()->mimetype();
            plugin = mimeMap.find(mimeType.replace(TQRegExp("/.*"), "/*"));

            if (plugin == mimeMap.end())
            {
                // check mime type inheritance
                KMimeType::Ptr mimeInfo = KMimeType::mimeType(it.current()->mimetype());
                TQString parentMimeType = mimeInfo->parentMimeType();
                while (!parentMimeType.isEmpty())
                {
                    plugin = mimeMap.find(parentMimeType);
                    if (plugin != mimeMap.end()) break;

                    KMimeType::Ptr parentMimeInfo = KMimeType::mimeType(parentMimeType);
                    if (!parentMimeInfo) break;

                    parentMimeType = parentMimeInfo->parentMimeType();
                }
            }

            if (plugin == mimeMap.end())
            {
                // check X-TDE-Text property
                KMimeType::Ptr mimeInfo = KMimeType::mimeType(it.current()->mimetype());
                TQVariant textProperty = mimeInfo->property("X-TDE-text");
                if (textProperty.isValid() && textProperty.type() == TQVariant::Bool)
                {
                    if (textProperty.toBool())
                    {
                        plugin = mimeMap.find("text/plain");
                        if (plugin == mimeMap.end())
                        {
                            plugin = mimeMap.find( "text/*" );
                        }
                    }
                }
            }
        }

        if (plugin != mimeMap.end())
        {
            item.plugin = *plugin;
            d->items.append(item);
            if (!bNeedCache && d->bSave &&
                (it.current()->url().protocol() != "file" ||
                 !it.current()->url().directory( false ).startsWith(d->thumbRoot)) &&
                (*plugin)->property("CacheThumbnail").toBool())
                bNeedCache = true;
        }
        else
        {
            emitFailed(it.current());
            if (d->deleteItems)
                delete it.current();
        }
    }

  // Read configuration value for the maximum allowed size
    TDEConfig * config = TDEGlobal::config();
    TDEConfigGroupSaver cgs( config, "PreviewSettings" );
    d->maximumSize = config->readNumEntry( "MaximumSize", 1024*1024 /* 1MB */ );

    if (bNeedCache)
    {
        if (d->width <= 128 && d->height <= 128) d->cacheWidth = d->cacheHeight = 128;
        else d->cacheWidth = d->cacheHeight = 256;
        d->thumbPath = d->thumbRoot + (d->cacheWidth == 128 ? "normal/" : "large/");
        TDEStandardDirs::makeDir(d->thumbPath, 0700);
    }
    else
        d->bSave = false;
    determineNextFile();
}

void PreviewJob::removeItem( const KFileItem *item )
{
    for (TQValueList<PreviewItem>::Iterator it = d->items.begin(); it != d->items.end(); ++it)
        if ((*it).item == item)
        {
            d->items.remove(it);
            break;
        }

    if (d->currentItem.item == item)
    {
        subjobs.first()->kill();
        subjobs.removeFirst();
        determineNextFile();
    }
}

void PreviewJob::setIgnoreMaximumSize(bool ignoreSize)
{
    d->ignoreMaximumSize = ignoreSize;
}

void PreviewJob::determineNextFile()
{
    if (d->currentItem.item)
    {
        if (!d->succeeded)
            emitFailed();
        if (d->deleteItems) {
            delete d->currentItem.item;
            d->currentItem.item = 0L;
        }
    }
    // No more items ?
    if ( d->items.isEmpty() )
    {
        emitResult();
        return;
    }
    else
    {
        // First, stat the orig file
        d->state = PreviewJobPrivate::STATE_STATORIG;
        d->currentItem = d->items.first();
        d->succeeded = false;
        d->items.remove(d->items.begin());
        TDEIO::Job *job = TDEIO::stat( d->currentItem.item->url(), false );
        job->addMetaData( "no-auth-prompt", "true" );
        addSubjob(job);
    }
}

void PreviewJob::slotResult( TDEIO::Job *job )
{
    subjobs.remove( job );
    Q_ASSERT ( subjobs.isEmpty() ); // We should have only one job at a time ...
    switch ( d->state )
    {
        case PreviewJobPrivate::STATE_STATORIG:
        {
            if (job->error()) // that's no good news...
            {
                // Drop this one and move on to the next one
                determineNextFile();
                return;
            }
            TDEIO::UDSEntry entry = ((TDEIO::StatJob*)job)->statResult();
            TDEIO::UDSEntry::ConstIterator it = entry.begin();
            d->tOrig = 0;
            int found = 0;
            for( ; it != entry.end() && found < 2; it++ )
            {
                if ( (*it).m_uds == TDEIO::UDS_MODIFICATION_TIME )
                {
                    d->tOrig = (time_t)((*it).m_long);
                    found++;
                }
                else if ( (*it).m_uds == TDEIO::UDS_SIZE )
                    {
                    if ( filesize_t((*it).m_long) > d->maximumSize &&
                         !d->ignoreMaximumSize &&
                         !d->currentItem.plugin->property("IgnoreMaximumSize").toBool() )
                    {
                        determineNextFile();
                        return;
                    }
                    found++;
                }
            }

            if ( !d->currentItem.plugin->property( "CacheThumbnail" ).toBool() )
            {
                // This preview will not be cached, no need to look for a saved thumbnail
                // Just create it, and be done
                getOrCreateThumbnail();
                return;
            }

            if ( statResultThumbnail() )
                return;

            getOrCreateThumbnail();
            return;
        }
        case PreviewJobPrivate::STATE_GETORIG:
        {
            if (job->error())
            {
                determineNextFile();
                return;
            }

            createThumbnail( static_cast<TDEIO::FileCopyJob*>(job)->destURL().path() );
            return;
        }
        case PreviewJobPrivate::STATE_CREATETHUMB:
        {
            if (!d->tempName.isEmpty())
            {
                TQFile::remove(d->tempName);
                d->tempName = TQString::null;
            }
            determineNextFile();
            return;
        }
    }
}

bool PreviewJob::statResultThumbnail()
{
    if ( d->thumbPath.isEmpty() )
        return false;

    KURL url = d->currentItem.item->url();
    // Don't include the password if any
    url.setPass(TQString::null);
    // The TMS defines local files as file:///path/to/file instead of KDE's
    // way (file:/path/to/file)
#ifdef KURL_TRIPLE_SLASH_FILE_PROT
    d->origName = url.url();
#else
    if (url.protocol() == "file") d->origName = "file://" + url.path();
    else d->origName = url.url();
#endif

    KMD5 md5( TQFile::encodeName( d->origName ).data() );
    d->thumbName = TQFile::encodeName( md5.hexDigest() ) + ".png";

    TQImage thumb;
    if ( !thumb.load( d->thumbPath + d->thumbName ) ) return false;

    if ( thumb.text( "Thumb::URI", 0 ) != d->origName ||
         thumb.text( "Thumb::MTime", 0 ).toInt() != d->tOrig ) return false;

    // Found it, use it
    emitPreview( thumb );
    d->succeeded = true;
    determineNextFile();
    return true;
}


void PreviewJob::getOrCreateThumbnail()
{
    // We still need to load the orig file ! (This is getting tedious) :)
    const KFileItem* item = d->currentItem.item;
    const TQString localPath = item->localPath();
    if ( !localPath.isEmpty() )
        createThumbnail( localPath );
    else
    {
        d->state = PreviewJobPrivate::STATE_GETORIG;
        KTempFile localFile;
        KURL localURL;
        localURL.setPath( d->tempName = localFile.name() );
        const KURL currentURL = item->url();
        TDEIO::Job * job = TDEIO::file_copy( currentURL, localURL, -1, true,
                                         false, false /* No GUI */ );
        job->addMetaData("thumbnail","1");
        addSubjob(job);
    }
}

// KDE 4: Make it const TQString &
void PreviewJob::createThumbnail( TQString pixPath )
{
    d->state = PreviewJobPrivate::STATE_CREATETHUMB;
    KURL thumbURL;
    thumbURL.setProtocol("thumbnail");
    thumbURL.setPath(pixPath);
    TDEIO::TransferJob *job = TDEIO::get(thumbURL, false, false);
    addSubjob(job);
    connect(job, TQ_SIGNAL(data(TDEIO::Job *, const TQByteArray &)), TQ_SLOT(slotThumbData(TDEIO::Job *, const TQByteArray &)));
    bool save = d->bSave && d->currentItem.plugin->property("CacheThumbnail").toBool();
    job->addMetaData("mimeType", d->currentItem.item->mimetype());
    job->addMetaData("width", TQString().setNum(save ? d->cacheWidth : d->width));
    job->addMetaData("height", TQString().setNum(save ? d->cacheHeight : d->height));
    job->addMetaData("iconSize", TQString().setNum(save ? 64 : d->iconSize));
    job->addMetaData("iconAlpha", TQString().setNum(d->iconAlpha));
    job->addMetaData("plugin", d->currentItem.plugin->library());
#ifdef Q_OS_UNIX
    if (d->shmid == -1)
    {
        if (d->shmaddr) {
            shmdt((char*)d->shmaddr);
            shmctl(d->shmid, IPC_RMID, 0);
        }
        d->shmid = shmget(IPC_PRIVATE, d->cacheWidth * d->cacheHeight * 4, IPC_CREAT|0600);
        if (d->shmid != -1)
        {
            d->shmaddr = (uchar *)(shmat(d->shmid, 0, SHM_RDONLY));
            if (d->shmaddr == (uchar *)-1)
            {
                shmctl(d->shmid, IPC_RMID, 0);
                d->shmaddr = 0;
                d->shmid = -1;
            }
        }
        else
            d->shmaddr = 0;
    }
    if (d->shmid != -1)
        job->addMetaData("shmid", TQString().setNum(d->shmid));
#endif
}

void PreviewJob::slotThumbData(TDEIO::Job *, const TQByteArray &data)
{
    bool save = d->bSave &&
                d->currentItem.plugin->property("CacheThumbnail").toBool() &&
                (d->currentItem.item->url().protocol() != "file" ||
                 !d->currentItem.item->url().directory( false ).startsWith(d->thumbRoot));
    TQImage thumb;
#ifdef Q_OS_UNIX
    if (d->shmaddr)
    {
        TQDataStream str(data, IO_ReadOnly);
        int width, height, depth;
        bool alpha;
        str >> width >> height >> depth >> alpha;
        thumb = TQImage(d->shmaddr, width, height, depth, 0, 0, TQImage::IgnoreEndian);
        thumb.setAlphaBuffer(alpha);
    }
    else
#endif
        thumb.loadFromData(data);

    if (save)
    {
        thumb.setText("Thumb::URI", 0, d->origName);
        thumb.setText("Thumb::MTime", 0, TQString::number(d->tOrig));
        thumb.setText("Thumb::Size", 0, number(d->currentItem.item->size()));
        thumb.setText("Thumb::Mimetype", 0, d->currentItem.item->mimetype());
        thumb.setText("Software", 0, "KDE Thumbnail Generator");
        KTempFile temp(d->thumbPath + "kde-tmp-", ".png");
        if (temp.status() == 0) //Only try to write out the thumbnail if we
        {                       //actually created the temp file.
            thumb.save(temp.name(), "PNG");
            rename(TQFile::encodeName(temp.name()), TQFile::encodeName(d->thumbPath + d->thumbName));
        }
    }
    emitPreview( thumb );
    d->succeeded = true;
}

void PreviewJob::emitPreview(const TQImage &thumb)
{
    TQPixmap pix;
    if (thumb.width() > d->width || thumb.height() > d->height)
    {
        double imgRatio = (double)thumb.height() / (double)thumb.width();
        if (imgRatio > (double)d->height / (double)d->width)
            pix.convertFromImage(
                thumb.smoothScale((int)TQMAX((double)d->height / imgRatio, 1), d->height));
        else pix.convertFromImage(
            thumb.smoothScale(d->width, (int)TQMAX((double)d->width * imgRatio, 1)));
    }
    else pix.convertFromImage(thumb);
    emit gotPreview(d->currentItem.item, pix);
}

void PreviewJob::emitFailed(const KFileItem *item)
{
    if (!item)
        item = d->currentItem.item;
    emit failed(item);
}

TQStringList PreviewJob::availablePlugins()
{
    TQStringList result;
    TDETrader::OfferList plugins = TDETrader::self()->query("ThumbCreator");
    for (TDETrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        if (!result.contains((*it)->desktopEntryName()))
            result.append((*it)->desktopEntryName());
    return result;
}

TQStringList PreviewJob::supportedMimeTypes()
{
    TQStringList result;
    TDETrader::OfferList plugins = TDETrader::self()->query("ThumbCreator");
    for (TDETrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        result += (*it)->property("MimeTypes").toStringList();
    return result;
}

void PreviewJob::kill( bool quietly )
{
    d->startPreviewTimer.stop();
    Job::kill( quietly );
}

PreviewJob *TDEIO::filePreview( const KFileItemList &items, int width, int height,
    int iconSize, int iconAlpha, bool scale, bool save,
    const TQStringList *enabledPlugins )
{
    return new PreviewJob(items, width, height, iconSize, iconAlpha,
                          scale, save, enabledPlugins);
}

PreviewJob *TDEIO::filePreview( const KURL::List &items, int width, int height,
    int iconSize, int iconAlpha, bool scale, bool save,
    const TQStringList *enabledPlugins )
{
    KFileItemList fileItems;
    for (KURL::List::ConstIterator it = items.begin(); it != items.end(); ++it)
        fileItems.append(new KFileItem(KFileItem::Unknown, KFileItem::Unknown, *it, true));
    return new PreviewJob(fileItems, width, height, iconSize, iconAlpha,
                          scale, save, enabledPlugins, true);
}

void PreviewJob::virtual_hook( int id, void* data )
{ TDEIO::Job::virtual_hook( id, data ); }


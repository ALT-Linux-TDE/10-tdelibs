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

#ifndef __tdeio_previewjob_h__
#define __tdeio_previewjob_h__

#include <tdefileitem.h>
#include <tdeio/job.h>

class TQPixmap;

namespace TDEIO {
    /*!
     * This class catches a preview (thumbnail) for files.
     * @short TDEIO Job to get a thumbnail picture
     */
    class TDEIO_EXPORT PreviewJob : public TDEIO::Job
    {
        TQ_OBJECT
    public:
	/**
	 * Creates a new PreviewJob.
	 * @param items a list of files to create previews for
	 * @param width the desired width
	 * @param height the desired height, 0 to use the @p width
	 * @param iconSize the size of the mimetype icon to overlay over the
	 * preview or zero to not overlay an icon. This has no effect if the
	 * preview plugin that will be used doesn't use icon overlays.
	 * @param iconAlpha transparency to use for the icon overlay
	 * @param scale if the image is to be scaled to the requested size or
	 * returned in its original size
	 * @param save if the image should be cached for later use
	 * @param enabledPlugins if non-zero, this points to a list containing
	 * the names of the plugins that may be used.
	 * @param deleteItems true to delete the items when done
	 */
        PreviewJob( const KFileItemList &items, int width, int height,
            int iconSize, int iconAlpha, bool scale, bool save,
            const TQStringList *enabledPlugins, bool deleteItems = false );
        virtual ~PreviewJob();

        /**
         * Removes an item from preview processing. Use this if you passed
         * an item to filePreview and want to delete it now.
         *
         * @param item the item that should be removed from the preview queue
         */
        void removeItem( const KFileItem *item );

        /**
         * If @p ignoreSize is true, then the preview is always 
         * generated regardless of the settings
         *
         * @since KDE 3.4
         **/
        void setIgnoreMaximumSize(bool ignoreSize = true);

        /**
         * Returns a list of all available preview plugins. The list
         * contains the basenames of the plugins' .desktop files (no path,
         * no .desktop).
	 * @return the list of plugins
         */
        static TQStringList availablePlugins();

        /**
         * Returns a list of all supported MIME types. The list can
         * contain entries like text/ * (without the space).
	 * @return the list of mime types
         */
        static TQStringList supportedMimeTypes();

        /**
         * Reimplemented for internal reasons
         */
        virtual void kill( bool quietly = true );

    signals:
        /**
         * Emitted when a thumbnail picture for @p item has been successfully
         * retrieved.
	 * @param item the file of the preview
	 * @param preview the preview image
         */
        void gotPreview( const KFileItem *item, const TQPixmap &preview );
        /**
         * Emitted when a thumbnail for @p item could not be created,
         * either because a ThumbCreator for its MIME type does not
         * exist, or because something went wrong.
	 * @param item the file that failed
         */
        void failed( const KFileItem *item );

    protected:
        void getOrCreateThumbnail();
        bool statResultThumbnail();
        void createThumbnail( TQString );

    protected slots:
        virtual void slotResult( TDEIO::Job *job );

    private slots:
        void startPreview();
        void slotThumbData(TDEIO::Job *, const TQByteArray &);

    private:
        void determineNextFile();
        void emitPreview(const TQImage &thumb);
        void emitFailed(const KFileItem *item = 0);

    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        struct PreviewJobPrivate *d;
    };

    /**
     * Creates a PreviewJob to generate or retrieve a preview image 
     * for the given URL.
     *
     * @param items files to get previews for
     * @param width the maximum width to use
     * @param height the maximum height to use, if this is 0, the same
     * value as width is used.
     * @param iconSize the size of the mimetype icon to overlay over the
     * preview or zero to not overlay an icon. This has no effect if the
     * preview plugin that will be used doesn't use icon overlays.
     * @param iconAlpha transparency to use for the icon overlay
     * @param scale if the image is to be scaled to the requested size or
     * returned in its original size
     * @param save if the image should be cached for later use
     * @param enabledPlugins if non-zero, this points to a list containing
     * the names of the plugins that may be used.
     * @return the new PreviewJob
     * @see PreviewJob::availablePlugins()
     */
    TDEIO_EXPORT PreviewJob *filePreview( const KFileItemList &items, int width, int height = 0, int iconSize = 0, int iconAlpha = 70, bool scale = true, bool save = true, const TQStringList *enabledPlugins = 0 );

    /**
     * Creates a PreviewJob to generate or retrieve a preview image 
     * for the given URL.
     *
     * @param items files to get previews for
     * @param width the maximum width to use
     * @param height the maximum height to use, if this is 0, the same
     * value as width is used.
     * @param iconSize the size of the mimetype icon to overlay over the
     * preview or zero to not overlay an icon. This has no effect if the
     * preview plugin that will be used doesn't use icon overlays.
     * @param iconAlpha transparency to use for the icon overlay
     * @param scale if the image is to be scaled to the requested size or
     * returned in its original size
     * @param save if the image should be cached for later use
     * @param enabledPlugins if non-zero, this points to a list containing
     * the names of the plugins that may be used.
     * @return the new PreviewJob
     * @see PreviewJob::availablePlugins()
     */
    TDEIO_EXPORT PreviewJob *filePreview( const KURL::List &items, int width, int height = 0, int iconSize = 0, int iconAlpha = 70, bool scale = true, bool save = true, const TQStringList *enabledPlugins = 0 );
}

#endif

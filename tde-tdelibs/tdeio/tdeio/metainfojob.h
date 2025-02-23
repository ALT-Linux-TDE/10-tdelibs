/*  This file is part of the KDE libraries
    Copyright (C) 2001 Rolf Magnus <ramagnus@kde.org>
    parts of this taken from previewjob.h

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation version 2.0.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __tdeio_metainfojob_h__
#define __tdeio_metainfojob_h__

#include <tdeio/job.h>
#include <tdefileitem.h>

namespace TDEIO {
    /**
     * MetaInfoJob is a TDEIO Job to retrieve meta information from files.
     * 
     * @short TDEIO Job to retrieve meta information from files.
     * @since 3.1
     */
    class TDEIO_EXPORT MetaInfoJob : public TDEIO::Job
    {
        TQ_OBJECT
    public:
        /**
         * Creates a new MetaInfoJob.
         *  @param items   A list of KFileItems to get the metainfo for
         *  @param deleteItems If true, the finished KFileItems are deleted
         */
        MetaInfoJob(const KFileItemList &items, bool deleteItems = false);
        virtual ~MetaInfoJob();

        /**
         * Removes an item from metainfo extraction.
         *
         * @param item the item that should be removed from the queue
         */
        void removeItem( const KFileItem *item );

        /**
         * Returns a list of all available metainfo plugins. The list
         * contains the basenames of the plugins' .desktop files (no path,
         * no .desktop).
	 * @return the list of available meta info plugins
         */
        static TQStringList availablePlugins();

        /**
         * Returns a list of all supported MIME types. The list can
         * contain entries like text/ * (without the space).
	 * @return the list of MIME types that are supported
         */
        static TQStringList supportedMimeTypes();

    signals:
        /**
         * Emitted when the meta info for @p item has been successfully
         * retrieved.
	 * @param item the KFileItem describing the fetched item
         */
        void gotMetaInfo( const KFileItem *item );
        /**
         * Emitted when metainfo for @p item could not be extracted,
         * either because a plugin for its MIME type does not
         * exist, or because something went wrong.
	 * @param item the KFileItem of the file that failed
         */
        void failed( const KFileItem *item );

    protected:
        void getMetaInfo();

    protected slots:
        virtual void slotResult( TDEIO::Job *job );

    private slots:
        void start();
        void slotMetaInfo(TDEIO::Job *, const TQByteArray &);

    private:
        void determineNextFile();
//        void saveMetaInfo(const TQByteArray info);

    private:
        struct MetaInfoJobPrivate *d;
    };

    /**
     * Retrieves meta information for the given items.
     *
     * @param items files to get metainfo for
     * @return the MetaInfoJob to retrieve the items
     */
    TDEIO_EXPORT MetaInfoJob* fileMetaInfo(const KFileItemList& items);

    /**
     * Retrieves meta information for the given items.
     *
     * @param items files to get metainfo for
     * @return the MetaInfoJob to retrieve the items
     */
    TDEIO_EXPORT MetaInfoJob* fileMetaInfo(const KURL::List& items);
}

#endif

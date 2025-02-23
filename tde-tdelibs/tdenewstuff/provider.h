/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>

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
#ifndef KNEWSTUFF_PROVIDER_H
#define KNEWSTUFF_PROVIDER_H

#include <tqcstring.h>
#include <tqdom.h>
#include <tqobject.h>
#include <tqptrlist.h>
#include <tqstring.h>

#include <kurl.h>

namespace TDEIO { class Job; }

namespace KNS {

/**
 * @short TDENewStuff provider container.
 *
 * This class provides accessors for the provider object.
 * as used by TDENewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Timothy Pearson (kb9vqf@pearsoncomputing.net)
 */
class TDE_EXPORT Provider : public TQObject
{
    TQ_OBJECT
  public:
    typedef TQPtrList<Provider> List;

    /**
     * Constructor.
     */
    Provider( TQString type = TQString::null, TQWidget* parent = 0 );

    /**
     * Constructor with XML feed.
     */
    Provider( const TQDomElement &, TQString type = TQString::null, TQWidget* parent = 0 );

    /**
     * Destructor.
     */
    virtual ~Provider();

    /**
     * @return provider load status
     */
    bool loaded();

    /**
     * Sets the common name of the provider.
     */
    void setName( const TQString & );

    /**
     * Retrieves the common name of the provider.
     *
     * @return provider name
     */
    TQString name() const;

    /**
     * Sets the download URL.
     */
    void setDownloadUrl( const KURL & );

    /**
     * Retrieves the download URL.
     *
     * @return download URL
     */
    KURL downloadUrl() const;

    /**
     * Variant to retrieve 'tagged' download URLs.
     * Variant can be one of 'score', 'downloads', 'latest'.
     *
     * @return download specific URL
     */
    KURL downloadUrlVariant( TQString variant ) const;

    /**
     * Sets the upload URL.
     */
    void setUploadUrl( const KURL & );

    /**
     * Retrieves the upload URL.
     *
     * @return upload URL
     */
    KURL uploadUrl() const;

    /**
     * Sets the URL where a user is led if the provider does not support
     * uploads.
     *
     * @see setNoUpload
     */
    void setNoUploadUrl( const KURL & );

    /**
     * Retrieves the URL where a user is led if the provider does not
     * support uploads.
     *
     * @return website URL
     */
    KURL noUploadUrl() const;

    /**
     * Indicate whether provider supports uploads.
     */
    void setNoUpload( bool );

    /**
     * Query whether provider supports uploads.
     *
     * @return upload support status
     */
    bool noUpload() const;

    /**
     * Sets the URL for an icon for this provider.
     * The icon should be in 32x32 format. If not set, the default icon
     * of KDialogBase is used.
     */
    void setIcon( const KURL & );

    /**
     * Retrieves the icon URL for this provider.
     *
     * @return icon URL
     */
    KURL icon() const;

  protected:
    void parseDomElement( const TQDomElement & );

    TQDomElement createDomElement( TQDomDocument &, TQDomElement &parent );

  protected slots:
    void slotJobData( TDEIO::Job *, const TQByteArray & );
    void slotJobResult( TDEIO::Job * );

  signals:
    void providerLoaded();

  private:
    TQString mName;
    KURL mDownloadUrl;
    KURL mUploadUrl;
    KURL mNoUploadUrl;
    KURL mIcon;
    bool mNoUpload;
    TQString mJobData;
    TQString mBaseURL;
    TQWidget* mParent;
    bool mLoaded;
    TQString mContentType;
};

/**
 * TDENewStuff provider loader.
 * This class sets up a list of all possible providers by querying
 * the main provider database for this specific application.
 * It should probably not be used directly by the application.
 */
class TDE_EXPORT ProviderLoader : public TQObject
{
    TQ_OBJECT
  public:
    /**
     * Constructor.
     *
     * @param parentWidget the parent widget
     */
    ProviderLoader( TQWidget *parentWidget );

    /**
     * Starts asynchronously loading the list of providers of the
     * specified type.
     *
     * @param type data type such as 'kdesktop/wallpaper'.
     * @param providerList the URl to the list of providers; if empty
     *    we first try the ProvidersUrl from TDEGlobal::config, then we
     *    fall back to a hardcoded value.
     */
    void load( const TQString &type, const TQString &providerList = TQString::null );

  signals:
    /**
     * Indicates that the list of providers has been successfully loaded.
     */
    void providersLoaded( Provider::List * );
    void percent(TDEIO::Job *job, unsigned long percent);
    void error();

  protected slots:
    void slotJobData( TDEIO::Job *, const TQByteArray & );
    void slotJobResult( TDEIO::Job * );
    void providerLoaded();

  private:
    TQWidget *mParentWidget;

    TQString mJobData;
    TQString mContentType;

    Provider::List mProviders;
};

}

#endif

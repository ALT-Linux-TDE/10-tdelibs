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
#ifndef KNEWSTUFF_ENTRY_H
#define KNEWSTUFF_ENTRY_H

#include <tqdatetime.h>
#include <tqdom.h>
#include <tqmap.h>
#include <tqstring.h>
#include <tqstringlist.h>

#include <kurl.h>

namespace KNS {

/**
 * @short TDENewStuff data entry container.
 *
 * This class provides accessor methods to the data objects
 * as used by TDENewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class TDE_EXPORT Entry
{
  public:
    Entry();
    /**
     * Constructor.
     */
    Entry( const TQDomElement & );

    /**
     * Destructor.
     */
    ~Entry();

    /**
     * Sets the (unique) name for this data object.
     */
    void setName( const TQString & );

    /**
     * Sets the (internationalised) name for this data object.
     */
    void setName( const TQString &, const TQString & );

    /**
     * Retrieve the name of the data object.
     *
     * @return object name
     */
    TQString name() const;

    /**
     * Retrieve the internationalised name of the data object.
     *
     * @return object name (potentially translated)
     */
    TQString name( const TQString &lang ) const;

    /**
     * Sets the application type, e.g. 'kdesktop/wallpaper'.
     */
    void setType( const TQString & );

    /**
     * Retrieve the type of the data object.
     *
     * @return object type
     */
    TQString type() const;

    /**
     * Sets the full name of the object's author.
     */
    void setAuthor( const TQString & );

    /**
     * Retrieve the author's name of the object.
     *
     * @return object author
     */
    TQString author() const;

    /**
     * Sets the email address of the object's author.
     */
    void setAuthorEmail( const TQString & );

    /**
     * Retrieve the author's email address of the object.
     *
     * @return object author email address
     */
    TQString authorEmail() const;

    /**
     * Sets the license (abbreviation) applicable to the object.
     */
    void setLicence( const TQString & );

    /**
     * Retrieve the license name of the object.
     *
     * @return object license
     */
    TQString license() const;

    /**
     * Sets a short description on what the object is all about.
     */
    void setSummary( const TQString &, const TQString &lang = TQString::null );

    /**
     * Retrieve a short description about the object.
     *
     * @param lang preferred language, or TQString::null for KDE default
     * @return object description
     */
    TQString summary( const TQString &lang = TQString::null ) const;

    /**
     * Sets the version number.
     */
    void setVersion( const TQString & );

    /**
     * Retrieve the version string of the object.
     *
     * @return object version
     */
    TQString version() const;

    /**
     * Sets the release number, which is increased for feature-equal objects
     * with the same version number, but slightly updated contents.
     */
    void setRelease( int );

    /**
     * Retrieve the release number of the object
     *
     * @return object release
     */
    int release() const;

    /**
     * Sets the release date.
     */
    void setReleaseDate( const TQDate & );

    /**
     * Retrieve the date of the object's publication.
     *
     * @return object release date
     */
    TQDate releaseDate() const;

    /**
     * Sets the object's file.
     */
    void setPayload( const KURL &, const TQString &lang = TQString::null );

    /**
     * Retrieve the file name of the object.
     *
     * @param lang preferred language, or TQString::null for KDE default
     * @return object filename
     */
    KURL payload( const TQString &lang = TQString::null ) const;

    /**
     * Sets the object's preview file, if available. This should be a
     * picture file.
     */
    void setPreview( const KURL &, const TQString &lang = TQString::null );

    /**
     * Retrieve the file name of an image containing a preview of the object.
     *
     * @param lang preferred language, or TQString::null for KDE default
     * @return object preview filename
     */
    KURL preview( const TQString &lang = TQString::null ) const;

    /**
     * Sets the rating between 0 (worst) and 10 (best).
     *
     * @internal
     */
    void setRating( int );

    /**
     * Retrieve the rating for the object, which has been determined by its
     * users and thus might change over time.
     *
     * @return object rating
     */
    int rating();

    /**
     * Sets the number of downloads.
     * 
     * @internal
     */
    void setDownloads( int );

    /**
     * Retrieve the download count for the object, which has been determined
     * by its hosting sites and thus might change over time.
     *
     * @return object download count
     */
    int downloads();

    /**
     * Return the full name for the meta information. It is constructed as
     * name-version-release.
     */
    TQString fullName();

    /**
     * Return the list of languages this object supports.
     */
    TQStringList langs();

    /**
     * @internal
     */
    void parseDomElement( const TQDomElement & );

    /**
     * @internal
     */
    TQDomElement createDomElement( TQDomDocument &, TQDomElement &parent );

  protected:
    TQDomElement addElement( TQDomDocument &doc, TQDomElement &parent,
                            const TQString &tag, const TQString &value );

  private:
    TQString mName;
    TQString mType;
    TQString mAuthor;
    TQString mLicence;
    TQMap<TQString,TQString> mSummaryMap;
    TQString mVersion;
    int mRelease;
    TQDate mReleaseDate;
    TQMap<TQString,KURL> mPayloadMap;
    TQMap<TQString,KURL> mPreviewMap;
    int mRating;
    int mDownloads;

    TQStringList mLangs;
};

}

#endif

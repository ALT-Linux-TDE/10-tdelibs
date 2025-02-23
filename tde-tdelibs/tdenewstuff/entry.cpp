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

#include "entry.h"

#include <tqptrdict.h>
#include <tqwindowdefs.h>

#include <tdeglobal.h>
#include <tdelocale.h>

using namespace KNS;

// BCI for KDE 3.5 only

class EntryPrivate
{
  public:
  EntryPrivate(){}
  TQString mEmail;
  TQMap<TQString,TQString> mNameMap;
};

static TQPtrDict<EntryPrivate> *d_ptr = 0;

static EntryPrivate *d(const Entry *e)
{
  if(!d_ptr)
  {
    d_ptr = new TQPtrDict<EntryPrivate>();
    d_ptr->setAutoDelete(true);
  }
  EntryPrivate *ret = d_ptr->find((void*)e);
  if(!ret)
  {
    ret = new EntryPrivate();
    d_ptr->replace((void*)e, ret);
  }
  return ret;
}

TQString Entry::authorEmail() const
{
  return d(this)->mEmail;
}

void Entry::setAuthorEmail( const TQString& email )
{
  d(this)->mEmail = email;
}

TQString Entry::name( const TQString &lang ) const
{
  if ( d(this)->mNameMap.isEmpty() ) return TQString::null;

  if ( !d(this)->mNameMap[ lang ].isEmpty() ) return d(this)->mNameMap[ lang ];
  else {
    TQStringList langs = TDEGlobal::locale()->languageList();
    for(TQStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
      if( !d(this)->mNameMap[ *it ].isEmpty() ) return d(this)->mNameMap[ *it ];
  }
  if ( !d(this)->mNameMap[ TQString::null ].isEmpty() ) return d(this)->mNameMap[ TQString::null ];
  else return *(mSummaryMap.begin());
}

void Entry::setName( const TQString &name, const TQString &lang )
{
  d(this)->mNameMap.insert( lang, name );

  if ( mLangs.find( lang ) == mLangs.end() ) mLangs.append( lang );
}

// BCI part ends here

Entry::Entry() :
  mRelease( 0 ), mReleaseDate( TQDate::currentDate() ), mRating( 0 ),
  mDownloads( 0 )
{
}

Entry::Entry( const TQDomElement &e ) :
  mRelease( 0 ), mRating( 0 ), mDownloads( 0 )
{
  parseDomElement( e );
}

Entry::~Entry()
{
    if (d_ptr)
    {
        EntryPrivate *p = d_ptr->find(this);
        if (p)
            d_ptr->remove(p);

        if (d_ptr->isEmpty())
        {
            delete d_ptr;
            d_ptr = 0L;
        }
    }
}


void Entry::setName( const TQString &name )
{
  mName = name;
}

TQString Entry::name() const
{
  return mName;
}


void Entry::setType( const TQString &type )
{
  mType = type;
}

TQString Entry::type() const
{
  return mType;
}


void Entry::setAuthor( const TQString &author )
{
  mAuthor = author;
}

TQString Entry::author() const
{
  return mAuthor;
}


void Entry::setLicence( const TQString &license )
{
  mLicence = license;
}

TQString Entry::license() const
{
  return mLicence;
}


void Entry::setSummary( const TQString &text, const TQString &lang )
{
  mSummaryMap.insert( lang, text );

  if ( mLangs.find( lang ) == mLangs.end() ) mLangs.append( lang );
}

TQString Entry::summary( const TQString &lang ) const
{
  if ( mSummaryMap.isEmpty() ) return TQString::null;

  if ( !mSummaryMap[ lang ].isEmpty() ) return mSummaryMap[ lang ];
  else {
    TQStringList langs = TDEGlobal::locale()->languageList();
    for(TQStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
      if( !mSummaryMap[ *it ].isEmpty() ) return mSummaryMap[ *it ];
  }
  if ( !mSummaryMap[ TQString::null ].isEmpty() ) return mSummaryMap[ TQString::null ];
  else return *(mSummaryMap.begin());
}


void Entry::setVersion( const TQString &version )
{
  mVersion = version;
}

TQString Entry::version() const
{
  return mVersion;
}


void Entry::setRelease( int release )
{
  mRelease = release;
}

int Entry::release() const
{
  return mRelease;
}


void Entry::setReleaseDate( const TQDate &d )
{
  mReleaseDate = d;
}

TQDate Entry::releaseDate() const
{
  return mReleaseDate;
}


void Entry::setPayload( const KURL &url, const TQString &lang )
{
  mPayloadMap.insert( lang, url );

  if ( mLangs.find( lang ) == mLangs.end() ) mLangs.append( lang );
}

KURL Entry::payload( const TQString &lang ) const
{
  KURL payload = mPayloadMap[ lang ];
  if ( payload.isEmpty() ) {
    TQStringList langs = TDEGlobal::locale()->languageList();
    for(TQStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
      if( !mPayloadMap[ *it ].isEmpty() ) return mPayloadMap[ *it ];
  }
  if ( payload.isEmpty() ) payload = mPayloadMap [ TQString::null ];
  if ( payload.isEmpty() && !mPayloadMap.isEmpty() ) {
    payload = *(mPayloadMap.begin());
  }
  return payload;
}


void Entry::setPreview( const KURL &url, const TQString &lang )
{
  mPreviewMap.insert( lang, url );
  
  if ( mLangs.find( lang ) == mLangs.end() ) mLangs.append( lang );
}

KURL Entry::preview( const TQString &lang ) const
{
  KURL preview = mPreviewMap[ lang ];
  if ( preview.isEmpty() ) {
    TQStringList langs = TDEGlobal::locale()->languageList();
    for(TQStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
      if( !mPreviewMap[ *it ].isEmpty() ) return mPreviewMap[ *it ];
  }
  if ( preview.isEmpty() ) preview = mPreviewMap [ TQString::null ];
  if ( preview.isEmpty() && !mPreviewMap.isEmpty() ) {
    preview = *(mPreviewMap.begin());
  }
  return preview;
}


void Entry::setRating( int rating )
{
  mRating = rating;
}

int Entry::rating()
{
  return mRating;
}


void Entry::setDownloads( int downloads )
{
  mDownloads = downloads;
}

int Entry::downloads()
{
  return mDownloads;
}

TQString Entry::fullName()
{
  if ( version().isEmpty() )
    return name();
  else
    return name() + "-" + version() + "-" + TQString::number( release() );
}

TQStringList Entry::langs()
{
  return mLangs;
}

// FIXME
// It appears that OCS has removed the ability to retrieve author EMail;
// further confirmation is needed before removing EMail-related code
// NOTE
// OCS also removed the ability to have individually localized names and summaries for a single item
// As this would be a useful feature to add to the OCS system I'm keeping the lang code skeleton in at this time
// Note that the "language" XML tag refers to the intended language of the content, not the language of the entry!
void Entry::parseDomElement( const TQDomElement &element )
{
  if ( element.tagName() != "content" ) return;
  mType = element.attribute("type");

  TQDomNode n;
  TQString lang;
  for( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement e = n.toElement();
    if ( e.tagName() == "name" )
    {
      setName( e.text().stripWhiteSpace(), lang );
      setName( e.text().stripWhiteSpace() ); /* primary key - no i18n */
    }
    if ( e.tagName() == "personid" ) {
      setAuthor( e.text().stripWhiteSpace() );
//       TQString email = e.attribute( "email" );
//       setAuthorEmail( email );
    }
//     if ( e.tagName() == "email" ) setAuthorEmail( e.text().stripWhiteSpace() ); /* kde-look; change on server! */
    if ( e.tagName() == "licence" ) setLicence( e.text().stripWhiteSpace() );
    if ( e.tagName() == "description" ) {
      setSummary( e.text().stripWhiteSpace(), lang );
    }
    if ( e.tagName() == "version" ) setVersion( e.text().stripWhiteSpace() );
//     if ( e.tagName() == "release" ) setRelease( e.text().toInt() );
    if ( e.tagName() == "created" ) {
      TQDate date = TQDate::fromString( e.text().stripWhiteSpace(), TQt::ISODate );
      setReleaseDate( date );
    }
    if ( e.tagName() == "smallpreviewpic1" ) {
      setPreview( KURL( e.text().stripWhiteSpace() ), lang );
    }
    if ( e.tagName() == "downloadlink1" ) {
      setPayload( KURL( e.text().stripWhiteSpace() ), lang );
    }
    if ( e.tagName() == "score" ) setRating( e.text().toInt() );
    if ( e.tagName() == "downloads" ) setDownloads( e.text().toInt() );
//     if ( e.tagName() == "typename" ) setType( e.text() );
  }
}

TQDomElement Entry::createDomElement( TQDomDocument &doc,
                                              TQDomElement &parent )
{
  TQDomElement entry = doc.createElement( "content" );
  entry.setAttribute("type", mType);
  parent.appendChild( entry );

  addElement( doc, entry, "language", langs().first() );

  addElement( doc, entry, "name", name() );
  addElement( doc, entry, "personid", author() );
//   addElement( doc, entry, "email", authorEmail() );
  addElement( doc, entry, "licence", license() );
  addElement( doc, entry, "version", version() );
//   addElement( doc, entry, "release", TQString::number( release() ) );
  addElement( doc, entry, "score", TQString::number( rating() ) );
  addElement( doc, entry, "downloads", TQString::number( downloads() ) );

  addElement( doc, entry, "created",
              releaseDate().toString( TQt::ISODate ) );

  addElement( doc, entry, "description", summary() );
  addElement( doc, entry, "preview", preview().url() );
  addElement( doc, entry, "payload", payload().url() );

  return entry;
}

TQDomElement Entry::addElement( TQDomDocument &doc, TQDomElement &parent,
                               const TQString &tag, const TQString &value )
{
  TQDomElement n = doc.createElement( tag );
  n.appendChild( doc.createTextNode( value ) );
  parent.appendChild( n );

  return n;
}

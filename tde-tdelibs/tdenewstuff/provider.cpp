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

#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeio/job.h>
#include <tdeglobal.h>
#include <tdemessagebox.h>
#include <tdelocale.h>

#include <tqptrdict.h>
#include <tqwindowdefs.h>

#include "provider.h"
#include "provider.moc"

using namespace KNS;

// BCI for KDE 3.5 only

class ProviderPrivate
{
  public:
  ProviderPrivate(){}
  KURL mDownloadUrlLatest;
  KURL mDownloadUrlScore;
  KURL mDownloadUrlDownloads;
};

static TQPtrDict<ProviderPrivate> *d_ptr_prov = 0;

static ProviderPrivate *d_prov(const Provider *p)
{
  if(!d_ptr_prov)
  {
    d_ptr_prov = new TQPtrDict<ProviderPrivate>();
    d_ptr_prov->setAutoDelete(true);
  }
  ProviderPrivate *ret = d_ptr_prov->find((void*)p);
  if(!ret)
  {
    ret = new ProviderPrivate();
    d_ptr_prov->replace((void*)p, ret);
  }
  return ret;
}

KURL Provider::downloadUrlVariant( TQString variant ) const
{
  if((variant == "latest") && (d_prov(this)->mDownloadUrlLatest.isValid()))
	return d_prov(this)->mDownloadUrlLatest;
  if((variant == "score") && (d_prov(this)->mDownloadUrlScore.isValid()))
	return d_prov(this)->mDownloadUrlScore;
  if((variant == "downloads") && (d_prov(this)->mDownloadUrlDownloads.isValid()))
	return d_prov(this)->mDownloadUrlDownloads;

  return mDownloadUrl;
}

// BCI part ends here

Provider::Provider( TQString type, TQWidget* parent ) : mNoUpload( false ), mParent( parent ), mLoaded( false ), mContentType( type )
{
}

Provider::Provider( const TQDomElement &e, TQString type, TQWidget* parent ) : mNoUpload( false ), mParent( parent ), mLoaded( false ), mContentType( type )
{
  parseDomElement( e );
}

Provider::~Provider()
{
    if (d_ptr_prov)
    {
        ProviderPrivate *p = d_ptr_prov->find(this);
        if (p)
            d_ptr_prov->remove(p);

        if (d_ptr_prov->isEmpty())
        {
            delete d_ptr_prov;
            d_ptr_prov = 0L;
        }
    }
}

bool Provider::loaded()
{
  return mLoaded;
}

void Provider::setName( const TQString &name )
{
  mName = name;
}

TQString Provider::name() const
{
  return mName;
}


void Provider::setIcon( const KURL &url )
{
  mIcon = url;
}

KURL Provider::icon() const
{
  return mIcon;
}


void Provider::setDownloadUrl( const KURL &url )
{
  mDownloadUrl = url;
}

KURL Provider::downloadUrl() const
{
  return mDownloadUrl;
}


void Provider::setUploadUrl( const KURL &url )
{
  mUploadUrl = url;
}

KURL Provider::uploadUrl() const
{
  return mUploadUrl;
}


void Provider::setNoUploadUrl( const KURL &url )
{
  mNoUploadUrl = url;
}

KURL Provider::noUploadUrl() const
{
  return mNoUploadUrl;
}


void Provider::setNoUpload( bool enabled )
{
  mNoUpload = enabled;
}

bool Provider::noUpload() const
{
  return mNoUpload;
}

void Provider::parseDomElement( const TQDomElement &element )
{
  bool contentAvailable = false;

  if ( element.tagName() != "provider" ) return;

  TQDomNode n;
  for ( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement p = n.toElement();

    if ( p.tagName() == "location" ) mBaseURL = p.text();
    if ( p.tagName() == "icon" ) {
      KURL iconurl( p.text() );
      if(!iconurl.isValid()) iconurl.setPath( p.text() );
      setIcon( iconurl );
    }

    if ( p.tagName() == "noupload" ) setNoUpload( true );
    if ( p.tagName() == "name" ) setName( p.text().stripWhiteSpace() );

    if ( p.tagName() == "services" ) {
      TQDomNode n2;
      for ( n2 = p.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
        TQDomElement p = n2.toElement();

        if ( p.tagName() == "content" ) contentAvailable = true;
      }
    }
  }

  if (!mBaseURL.endsWith("/")) {
    mBaseURL.append("/");
  }

  if (contentAvailable) {
    // Load content type list
    KURL contentTypeUrl( mBaseURL + "content/categories" );

    kdDebug() << "Provider::parseDomElement(): contentTypeUrl: " << contentTypeUrl << endl;

    TDEIO::TransferJob *job = TDEIO::get( KURL( contentTypeUrl ), false, false );
    connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ),
             TQ_SLOT( slotJobResult( TDEIO::Job * ) ) );
    connect( job, TQ_SIGNAL( data( TDEIO::Job *, const TQByteArray & ) ),
             TQ_SLOT( slotJobData( TDEIO::Job *, const TQByteArray & ) ) );
  }
}

TQDomElement Provider::createDomElement( TQDomDocument &doc, TQDomElement &parent )
{
  TQDomElement entry = doc.createElement( "stuff" );
  parent.appendChild( entry );

  TQDomElement n = doc.createElement( "name" );
  n.appendChild( doc.createTextNode( name() ) );
  entry.appendChild( n );
  
  return entry;
}

void Provider::slotJobData( TDEIO::Job *, const TQByteArray &data )
{
  kdDebug() << "Provider::slotJobData()" << endl;

  if ( data.size() == 0 ) return;

  TQCString str( data, data.size() + 1 );

  mJobData.append( TQString::fromUtf8( str ) );
}

void Provider::slotJobResult( TDEIO::Job *job )
{
  if ( job->error() ) {
    if (mParent) {
      job->showErrorDialog( mParent );
    }
    return;
  }

  kdDebug() << "--CONTENT-START--" << endl << mJobData << "--CONT_END--"
            << endl;

  TQDomDocument doc;
  if ( !doc.setContent( mJobData ) ) {
    if (mParent) {
      KMessageBox::error( mParent, i18n("Error parsing category list.") );
    }
    return;
  }

  TQDomElement categories = doc.documentElement();

  if ( categories.isNull() ) {
    kdDebug() << "No document in Content.xml." << endl;
  }

  TQStringList desiredCategoryList;
  TQString desiredCategories;

  TQDomNode n;
  for ( n = categories.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement p = n.toElement();

    if ( p.tagName() == "data" ) {
      TQDomNode n2;
      for ( n2 = p.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
        TQDomElement p = n2.toElement();

        if ( p.tagName() == "category" ) {
          TQDomNode n3;
          TQString id;
          TQString name;
          for ( n3 = p.firstChild(); !n3.isNull(); n3 = n3.nextSibling() ) {
            TQDomElement p = n3.toElement();

            if ( p.tagName() == "id" ) {
              id = p.text();
            }

            if ( p.tagName() == "name" ) {
              name = p.text();
            }
          }

          if (mContentType == "") {
            desiredCategoryList.append(id);
          }
          else {
            if (name.lower().contains(mContentType.lower())) {
              desiredCategoryList.append(id);
            }
          }
        }
      }
    }
  }

  desiredCategories = desiredCategoryList.join("x");

  // int maxEntries = 10;
  //int maxEntries = 50;
  int maxEntries = 100;

  // FIXME
  // The download URL assumes no more than 100 entries will be present on the remote server
  // Any more and assembly of the list from multiple fetched pages will be required; the TDENewStuff Engine class does not have support for paging at this time
  setDownloadUrl( KURL( mBaseURL + "content/data?categories=" + desiredCategories + "&search=&sortmode=alpha&page=0&pagesize=" + TQString("%1").arg(maxEntries) ) );
  // FIXME
  // Uploading is not implemented at this time
  setUploadUrl( KURL( mBaseURL ) );
  setNoUploadUrl( KURL( mBaseURL ) );

  d_prov(this)->mDownloadUrlLatest = KURL( mBaseURL + "content/data?categories=" + desiredCategories + "&search=&sortmode=new&page=0&pagesize=" + TQString("%1").arg(maxEntries) );
  d_prov(this)->mDownloadUrlScore = KURL( mBaseURL + "content/data?categories=" + desiredCategories + "&search=&sortmode=high&page=0&pagesize=" + TQString("%1").arg(maxEntries) );
  d_prov(this)->mDownloadUrlDownloads = KURL( mBaseURL + "content/data?categories=" + desiredCategories + "&search=&sortmode=down&page=0&pagesize=" + TQString("%1").arg(maxEntries) );

  mLoaded = true;
  emit providerLoaded();
}


ProviderLoader::ProviderLoader( TQWidget *parentWidget ) :
  TQObject( parentWidget )
{
  mProviders.setAutoDelete( true );
}

void ProviderLoader::load( const TQString &type, const TQString &providersList )
{
  kdDebug() << "ProviderLoader::load()" << endl;

  mProviders.clear();
  mJobData = "";
  mContentType = type;

  TDEConfig *cfg = TDEGlobal::config();
  cfg->setGroup("TDENewStuff");

  TQString providersUrl = providersList;
  if( providersUrl.isEmpty() ) {
    providersUrl = cfg->readEntry( "ProvidersUrl" );
  }

  if ( providersUrl.isEmpty() ) {
    TQString server = cfg->readEntry( "MasterServer",
                                     "https://www.trinitydesktop.org" );
    providersUrl = server + "/ocs/providers.xml";
  }

  kdDebug() << "ProviderLoader::load(): providersUrl: " << providersUrl << endl;

  TDEIO::TransferJob *job = TDEIO::get( KURL( providersUrl ), false, false );
  connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ),
           TQ_SLOT( slotJobResult( TDEIO::Job * ) ) );
  connect( job, TQ_SIGNAL( data( TDEIO::Job *, const TQByteArray & ) ),
           TQ_SLOT( slotJobData( TDEIO::Job *, const TQByteArray & ) ) );
  connect( job, TQ_SIGNAL( percent (TDEIO::Job *, unsigned long) ),
           TQ_SIGNAL( percent (TDEIO::Job *, unsigned long) ) );

//  job->dumpObjectInfo();
}

void ProviderLoader::slotJobData( TDEIO::Job *, const TQByteArray &data )
{
  kdDebug() << "ProviderLoader::slotJobData()" << endl;

  if ( data.size() == 0 ) return;

  TQCString str( data, data.size() + 1 );

  mJobData.append( TQString::fromUtf8( str ) );
}

void ProviderLoader::slotJobResult( TDEIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( static_cast<TQWidget*>(parent()) );
    emit error();
    return;
  }

  kdDebug() << "--PROVIDERS-START--" << endl << mJobData << "--PROV_END--"
            << endl;

  TQDomDocument doc;
  if ( !doc.setContent( mJobData ) ) {
    KMessageBox::error( static_cast<TQWidget*>(parent()), i18n("Error parsing providers list.") );
    emit error();
    return;
  }

  TQDomElement providers = doc.documentElement();

  if ( providers.isNull() ) {
    kdDebug() << "No document in Providers.xml." << endl;
    emit error();
    return;
  }

  TQDomNode n;
  for ( n = providers.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement p = n.toElement();
 
    if ( p.tagName() == "provider" ) {
      Provider* prov = new Provider( p, mContentType, static_cast<TQWidget*>(parent()) );
      mProviders.append( prov );
      connect( prov, TQ_SIGNAL( providerLoaded() ), this, TQ_SLOT( providerLoaded() ) );
    }
  }
}

void ProviderLoader::providerLoaded() {
  Provider* prov = NULL;
  bool allLoaded = true;
  for ( prov = mProviders.first(); prov; prov = mProviders.next() ) {
    if (!prov->loaded()) {
      allLoaded = false;
      break;
    }
  }

  if (allLoaded) {
    emit providersLoaded( &mProviders );
  }
}
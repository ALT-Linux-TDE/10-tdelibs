/*
    This file is part of libtdeabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

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

#ifndef KABC_RESOURCELDAP_H
#define KABC_RESOURCELDAP_H

#include <tdeabc/resource.h>
#include <tdeabc/ldif.h>
#include <tdeio/job.h>

class TDEConfig;

namespace TDEABC {

class KABC_EXPORT ResourceLDAPTDEIO : public Resource
{
  TQ_OBJECT

  public:
    enum CachePolicy{ Cache_No, Cache_NoConnection, Cache_Always };

    ResourceLDAPTDEIO( const TDEConfig* );
    virtual ~ResourceLDAPTDEIO();
    /**
     *  Call this after you used one of the set... methods 
     */
    virtual void init();
    
    virtual void writeConfig( TDEConfig* );

    virtual bool doOpen();
    virtual void doClose();

    virtual Ticket *requestSaveTicket();
    virtual void releaseSaveTicket( Ticket* );

    virtual bool readOnly() const { return Resource::readOnly(); }
    virtual void setReadOnly( bool value );

    virtual bool load();
    virtual bool asyncLoad();
    virtual bool save( Ticket * ticket );
    virtual bool asyncSave( Ticket * ticket );

    virtual void removeAddressee( const Addressee& addr );

    void setUser( const TQString &user );
    TQString user() const;

    void setPassword( const TQString &password );
    TQString password() const;
    
    void setRealm( const TQString &realm );
    TQString realm() const;
    
    void setBindDN( const TQString &binddn );
    TQString bindDN() const;

    void setDn( const TQString &dn );
    TQString dn() const;

    void setHost( const TQString &host );
    TQString host() const;

    void setPort( int port );
    int port() const;

    void setVer( int ver );
    int ver() const;
    
    void setSizeLimit( int sizelimit );
    int sizeLimit();
    
    void setTimeLimit( int timelimit );
    int timeLimit();

    void setFilter( const TQString &filter );
    TQString filter() const;

    void setIsAnonymous( bool value );
    bool isAnonymous() const;

    void setAttributes( const TQMap<TQString, TQString> &attributes );
    TQMap<TQString, TQString> attributes() const;
    
    void setRDNPrefix( int value );
    int RDNPrefix() const;

    void setIsTLS( bool value );
    bool isTLS() const ;
    
    void setIsSSL( bool value );
    bool isSSL() const;
    
    void setIsSubTree( bool value );
    bool isSubTree() const ;

    void setIsSASL( bool value );
    bool isSASL() const ;

    void setMech( const TQString &mech );
    TQString mech() const;

    void setCachePolicy( int pol );
    int cachePolicy() const;

    void setAutoCache( bool value );
    bool autoCache();
    
    TQString cacheDst() const;
    
protected slots:
    void entries( TDEIO::Job*, const TDEIO::UDSEntryList& );
    void data( TDEIO::Job*, const TQByteArray& );
    void result( TDEIO::Job* );
    void listResult( TDEIO::Job* );
    void syncLoadSaveResult( TDEIO::Job* );
    void saveResult( TDEIO::Job* );
    void saveData( TDEIO::Job*, TQByteArray& );
    void loadCacheResult( TDEIO::Job* );
  
  private:
    TQString mUser;
    TQString mPassword;
    TQString mDn;
    TQString mHost;
    TQString mFilter;
    int mPort;
    bool mAnonymous;
    TQMap<TQString, TQString> mAttributes;

    KURL mLDAPUrl;
    int mGetCounter; //KDE 4: remove
    bool mErrorOccured; //KDE 4: remove
    TQString mErrorMsg;
    TQMap<TDEIO::Job*, TQByteArray> mJobMap; //KDE 4: remove

    TDEIO::Job *loadFromCache();
    void createCache();
    void activateCache();
    void enter_loop();
    TQCString addEntry( const TQString &attr, const TQString &value, bool mod );
    TQString findUid( const TQString &uid );
    bool AddresseeToLDIF( TQByteArray &ldif, const Addressee &addr, 
      const TQString &olddn );
    
    class ResourceLDAPTDEIOPrivate;
    ResourceLDAPTDEIOPrivate *d;
};

}

#endif

/* kldapclient.h - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


#ifndef KABC_LDAPCLIENT_H
#define KABC_LDAPCLIENT_H


#include <tqobject.h>
#include <tqstring.h>
#include <tqcstring.h>
#include <tqstringlist.h>
#include <tqmemarray.h>
#include <tqguardedptr.h>
#include <tqtimer.h>

#include <tdeio/job.h>

namespace TDEABC {

class LdapClient;
typedef TQValueList<TQByteArray> LdapAttrValue;
typedef TQMap<TQString,LdapAttrValue > LdapAttrMap;

/**
  * This class is internal. Binary compatibiliy might be broken any time
  * without notification. Do not use it.
  *
  * We mean it!
  *
  */
class KABC_EXPORT LdapObject
{
  public:
    LdapObject()
      : dn( TQString::null ), client( 0 ) {}
    explicit LdapObject( const TQString& _dn, LdapClient* _cl ) : dn( _dn ), client( _cl ) {}
    LdapObject( const LdapObject& that ) { assign( that ); }

    LdapObject& operator=( const LdapObject& that )
    {
      assign( that );
      return *this;
    }

    TQString toString() const;

    void clear();

    TQString dn;
    LdapAttrMap attrs;
    LdapClient* client;

  protected:
    void assign( const LdapObject& that );

  private:
    //class LdapObjectPrivate* d;
};

/**
  * This class is internal. Binary compatibiliy might be broken any time
  * without notification. Do not use it.
  *
  * We mean it!
  *
  */
class KABC_EXPORT LdapClient : public TQObject
{
  TQ_OBJECT

  public:
    LdapClient( TQObject* parent = 0, const char* name = 0 );
    virtual ~LdapClient();

    /*! returns true if there is a query running */
    bool isActive() const { return mActive; }

  signals:
    /*! Emitted when the query is done */
    void done();

    /*! Emitted in case of error */
    void error( const TQString& );

    /*! Emitted once for each object returned
     * from the query
     */
    void result( const TDEABC::LdapObject& );

  public slots:
    /*!
     * Set the name or IP of the LDAP server
     */
    void setHost( const TQString& host );
    TQString host() const { return mHost; }

    /*!
     * Set the port of the LDAP server
     * if using a nonstandard port
     */
    void setPort( const TQString& port );
    TQString port() const { return mPort; }

    /*!
     * Set the base DN
     */
    void setBase( const TQString& base );
    TQString base() const { return mBase; }

    /*!
     * Set the bind DN
     */
    void setBindDN( const TQString& bindDN );
    TQString bindDN() const;

    /*!
     * Set the bind password DN
     */
    void setPwdBindDN( const TQString& pwdBindDN );
    TQString pwdBindDN() const;

    /*! Set the attributes that should be
     * returned, or an empty list if
     * all attributes are wanted
     */
    void setAttrs( const TQStringList& attrs );
    TQStringList attrs() const { return mAttrs; }

    void setScope( const TQString scope ) { mScope = scope; }

    /*!
     * Start the query with filter filter
     */
    void startQuery( const TQString& filter );

    /*!
     * Abort a running query
     */
    void cancelQuery();

  protected slots:
    void slotData( TDEIO::Job*, const TQByteArray &data );
    void slotInfoMessage( TDEIO::Job*, const TQString &info );
    void slotDone();

  protected:
    void startParseLDIF();
    void parseLDIF( const TQByteArray& data );
    void endParseLDIF();

    TQString mHost;
    TQString mPort;
    TQString mBase;
    TQString mScope;
    TQStringList mAttrs;

    TQGuardedPtr<TDEIO::SimpleJob> mJob;
    bool mActive;

    LdapObject mCurrentObject;
    TQCString mBuf;
    TQCString mLastAttrName;
    TQCString mLastAttrValue;
    bool mIsBase64;

  private:
    class LdapClientPrivate;
    LdapClientPrivate* d;
};

/**
 * Structure describing one result returned by a LDAP query
 */
struct LdapResult {
  TQString name;     ///< full name
  TQString email;    ///< email
  int clientNumber; ///< for sorting
};
typedef TQValueList<LdapResult> LdapResultList;


/**
  * This class is internal. Binary compatibiliy might be broken any time
  * without notification. Do not use it.
  *
  * We mean it!
  *
  */
class KABC_EXPORT LdapSearch : public TQObject
{
  TQ_OBJECT

  public:
    LdapSearch();

    void startSearch( const TQString& txt );
    void cancelSearch();
    bool isAvailable() const;

  signals:
    /// Results, assembled as "Full Name <email>"
    /// (This signal can be emitted many times)
    void searchData( const TQStringList& );
    /// Another form for the results, with separate fields
    /// (This signal can be emitted many times)
    void searchData( const TDEABC::LdapResultList& );
    void searchDone();

  private slots:
    void slotLDAPResult( const TDEABC::LdapObject& );
    void slotLDAPError( const TQString& );
    void slotLDAPDone();
    void slotDataTimer();

  private:
    void finish();
    void makeSearchData( TQStringList& ret, LdapResultList& resList );
    TQValueList< LdapClient* > mClients;
    TQString mSearchText;
    TQTimer mDataTimer;
    int mActiveClients;
    bool mNoLDAPLookup;
    TQValueList< LdapObject > mResults;

  private:
    class LdapSearchPrivate* d;
};

}
#endif // KABC_LDAPCLIENT_H

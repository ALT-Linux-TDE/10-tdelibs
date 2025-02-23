/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#ifndef __tdeio_slaveinterface_h
#define __tdeio_slaveinterface_h

#include <unistd.h>
#include <sys/types.h>

#include <tqobject.h>

#include <kurl.h>
#include <tdeio/global.h>
#include <tdeio/authinfo.h>
#include <kdatastream.h>

namespace TDEIO {

class Connection;
// better there is one ...
class SlaveInterfacePrivate;

  // Definition of enum Command has been moved to global.h

 /**
  * Identifiers for TDEIO informational messages.
  */
 enum Info {
   INF_TOTAL_SIZE = 10,
   INF_PROCESSED_SIZE = 11,
   INF_SPEED,
   INF_REDIRECTION = 20,
   INF_MIME_TYPE = 21,
   INF_ERROR_PAGE = 22,
   INF_WARNING = 23,
   INF_GETTING_FILE, // Deprecated
   INF_NEED_PASSWD = 25,
   INF_INFOMESSAGE,
   INF_META_DATA,
   INF_NETWORK_STATUS,
   INF_MESSAGEBOX,
   INF_LOCALURL
   // add new ones here once a release is done, to avoid breaking binary compatibility
 };

 /**
  * Identifiers for TDEIO data messages.
  */
 enum Message {
   MSG_DATA = 100,
   MSG_DATA_REQ,
   MSG_ERROR,
   MSG_CONNECTED,
   MSG_FINISHED,
   MSG_STAT_ENTRY,
   MSG_LIST_ENTRIES,
   MSG_RENAMED, // unused
   MSG_RESUME,
   MSG_SLAVE_STATUS,
   MSG_SLAVE_ACK,
   MSG_NET_REQUEST,
   MSG_NET_DROP,
   MSG_NEED_SUBURL_DATA,
   MSG_CANRESUME,
   MSG_AUTH_KEY, // deprecated.
   MSG_DEL_AUTH_KEY // deprecated.
   // add new ones here once a release is done, to avoid breaking binary compatibility
 };

/**
 * There are two classes that specifies the protocol between application
 * (TDEIO::Job) and tdeioslave. SlaveInterface is the class to use on the application
 * end, SlaveBase is the one to use on the slave end.
 *
 * A call to foo() results in a call to slotFoo() on the other end.
 */
class TDEIO_EXPORT SlaveInterface : public TQObject
{
    TQ_OBJECT

public:
    SlaveInterface( Connection *connection );
    virtual ~SlaveInterface();

    void setConnection( Connection* connection ) { m_pConnection = connection; }
    Connection *connection() const { return m_pConnection; }

    void setProgressId( int id ) { m_progressId = id; }
    int progressId() const { return m_progressId; }

    /** Send our answer to the MSG_RESUME (canResume) request
     * (to tell the "put" job whether to resume or not)
     */
    void sendResumeAnswer( bool resume );

    void setOffset( TDEIO::filesize_t offset );
    TDEIO::filesize_t offset() const;

signals:
    ///////////
    // Messages sent by the slave
    ///////////

    void data( const TQByteArray & );
    void dataReq( );
    void error( int , const TQString & );
    void connected();
    void finished();
    void slaveStatus(pid_t, const TQCString &, const TQString &, bool);
    void listEntries( const TDEIO::UDSEntryList& );
    void statEntry( const TDEIO::UDSEntry& );
    void needSubURLData();
    void needProgressId();

    void canResume( TDEIO::filesize_t ) ;

    ///////////
    // Info sent by the slave
    //////////
    void metaData( const TDEIO::MetaData & );
    void totalSize( TDEIO::filesize_t ) ;
    void processedSize( TDEIO::filesize_t ) ;
    void redirection( const KURL& ) ;
    void localURL( const KURL&, bool ) ;

    void speed( unsigned long ) ;
    void errorPage() ;
    void mimeType( const TQString & ) ;
    void warning( const TQString & ) ;
    void infoMessage( const TQString & ) ;
    void connectFinished();

    /**
     * @deprecated. Obsolete as of 3.1. Replaced by kpassword, a kded module.
     */
    void authorizationKey( const TQCString&, const TQCString&, bool );

    /**
     * @deprecated. Obsolete as of 3.1. Replaced by kpassword, a kded module.
     */
    void delAuthorization( const TQCString& grpkey );

protected:
    /////////////////
    // Dispatching
    ////////////////

    virtual bool dispatch();
    virtual bool dispatch( int _cmd, const TQByteArray &data );

   /**
    * Prompt the user for authrization info (login & password).
    *
    * Use this function to request authorization info from the
    * the end user. For example to open an empty password dialog
    * using default values:
    *
    * \code
    * TDEIO::AuthInfo authInfo;
    * bool result = openPassDlg( authInfo );
    * if ( result )
    * {
    *    printf( "Username: %s", result.username.latin1() );
    *    printf( "Username: %s", result.username.latin1() );
    * }
    * \endcode
    *
    * You can also pre-set some values like the username before hand
    * if it is known as well as the comment and caption to be displayed:
    *
    * \code
    * authInfo.comment= "Enter username and password to access acmeone";
    * authInfo.caption= "Acme Password Dialog";
    * authInfo.username= "Wily E. kaiody";
    * bool result = openPassDlg( authInfo );
    * if ( result )
    * {
    *    printf( "Username: %s", result.username.latin1() );
    *    printf( "Username: %s", result.username.latin1() );
    * }
    * \endcode
    *
    * NOTE: A call to this function can also fail and result
    * in a return value of @p false, if the UIServer could not
    * be started for whatever reason.
     *
     * @param       info See AuthInfo.
     * @return      true if user clicks on "OK", false otherwsie.
     */
    void openPassDlg( TDEIO::AuthInfo& info );

   /**
    * @deprecated. Use openPassDlg( AuthInfo& ) instead.
    */
    void openPassDlg( const TQString& prompt, const TQString& user,
                      const TQString& caption, const TQString& comment,
                      const TQString& label, bool readOnly ) TDE_DEPRECATED;

   /**
    * @deprecated. Use openPassDlg( AuthInfo& ) instead.
    */
    void openPassDlg( const TQString& prompt, const TQString& user, bool readOnly ) TDE_DEPRECATED;

    void messageBox( int type, const TQString &text, const TQString &caption,
                     const TQString &buttonYes, const TQString &buttonNo );

   /**
    * @since 3.3
    */
    void messageBox( int type, const TQString &text, const TQString &caption,
                     const TQString &buttonYes, const TQString &buttonNo, const TQString &dontAskAgainName );

    // I need to identify the slaves
    void requestNetwork( const TQString &, const TQString &);
    void dropNetwork( const TQString &, const TQString &);

    /**
     * @internal
     * KDE 4.0: Remove
     */
    static void sigpipe_handler(int);

protected slots:
    void calcSpeed();

protected:
    Connection * m_pConnection;

private:
    int m_progressId;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    SlaveInterfacePrivate *d;
};

}

inline TQDataStream &operator >>(TQDataStream &s, TDEIO::UDSAtom &a )
{
    TQ_INT32 l;
    s >> a.m_uds;

    if ( a.m_uds & TDEIO::UDS_LONG ) {
        s >> l;
        a.m_long = l;
        a.m_str = TQString::null;
    } else if ( a.m_uds & TDEIO::UDS_STRING ) {
        s >> a.m_str;
        a.m_long = 0;
    } else {} // DIE!
    //    assert( 0 );

    return s;
}

inline TQDataStream &operator <<(TQDataStream &s, const TDEIO::UDSAtom &a )
{
    s << a.m_uds;

    if ( a.m_uds & TDEIO::UDS_LONG )
        s << (TQ_INT32) a.m_long;
    else if ( a.m_uds & TDEIO::UDS_STRING )
        s << a.m_str;
    else {} // DIE!
    //    assert( 0 );

    return s;
}

TDEIO_EXPORT TQDataStream &operator <<(TQDataStream &s, const TDEIO::UDSEntry &e );
TDEIO_EXPORT TQDataStream &operator >>(TQDataStream &s, TDEIO::UDSEntry &e );

#endif

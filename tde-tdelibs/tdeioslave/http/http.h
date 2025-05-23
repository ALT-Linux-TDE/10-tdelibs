/*
   Copyright (C) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2000,2001 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
   Copyright (C) 2001,2002 Hamish Rodda <rodda@kde.org>

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

#ifndef HTTP_H_
#define HTTP_H_


#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <tqptrlist.h>
#include <tqstrlist.h>
#include <tqstringlist.h>

#include <kurl.h>
#include "tdeio/tcpslavebase.h"
#include "tdeio/http.h"

class DCOPClient;
class TQDomElement;
class TQDomNodeList;

namespace TDEIO {
    class AuthInfo;
}

class HTTPProtocol : public TQObject, public TDEIO::TCPSlaveBase
{
  TQ_OBJECT
public:
  HTTPProtocol( const TQCString &protocol, const TQCString &pool,
                const TQCString &app );
  virtual ~HTTPProtocol();

  /** HTTP version **/
  enum HTTP_REV    {HTTP_None, HTTP_Unknown, HTTP_10, HTTP_11, SHOUTCAST};

  /** Authorization method used **/
  enum HTTP_AUTH   {AUTH_None, AUTH_Basic, AUTH_NTLM, AUTH_Digest, AUTH_Negotiate};

  /** HTTP / DAV method **/
  // Removed to interfaces/tdeio/http.h
  //enum HTTP_METHOD {HTTP_GET, HTTP_PUT, HTTP_POST, HTTP_HEAD, HTTP_DELETE,
  //                  HTTP_OPTIONS, DAV_PROPFIND, DAV_PROPPATCH, DAV_MKCOL,
  //                  DAV_COPY, DAV_MOVE, DAV_LOCK, DAV_UNLOCK, DAV_SEARCH };

  /** State of the current Connection **/
  struct HTTPState
  {
    HTTPState ()
    {
      port = 0;
      doProxy = false;
    }

    TQString hostname;
    TQString encoded_hostname;
    short unsigned int port;
    TQString user;
    TQString passwd;
    bool  doProxy;
  };

  /** DAV-specific request elements for the current connection **/
  struct DAVRequest
  {
    DAVRequest ()
    {
      overwrite = false;
      depth = 0;
    }

    TQString desturl;
    bool overwrite;
    int depth;
  };

  /** The request for the current connection **/
  struct HTTPRequest
  {
    HTTPRequest ()
    {
      port = 0;
      method = TDEIO::HTTP_UNKNOWN;
      offset = 0;
      doProxy = false;
      allowCompressedPage = false;
      disablePassDlg = false;
      bNoAuth = false;
      bUseCache = false;
      bCachedRead = false;
      bCachedWrite = false;
      fcache = 0;
      bMustRevalidate = false;
      cacheExpireDateOffset = 0;
      bErrorPage = false;
      bUseCookiejar = false;
      expireDate = 0;
      creationDate = 0;
    }

    TQString hostname;
    TQString encoded_hostname;
    short unsigned int port;
    TQString user;
    TQString passwd;
    TQString path;
    TQString query;
    TDEIO::HTTP_METHOD method;
    TDEIO::CacheControl cache;
    TDEIO::filesize_t offset;
    bool doProxy;
    KURL url;
    TQString window;                 // Window Id this request is related to.
    TQString referrer;
    TQString charsets;
    TQString languages;
    bool allowCompressedPage;
    bool disablePassDlg;
    TQString userAgent;
    TQString id;
    DAVRequest davData;

    bool bNoAuth; // Do not authenticate

    // Cache related
    TQString cef; // Cache Entry File belonging to this URL.
    bool bUseCache; // Whether the cache is active
    bool bCachedRead; // Whether the file is to be read from m_fcache.
    bool bCachedWrite; // Whether the file is to be written to m_fcache.
    FILE* fcache; // File stream of a cache entry
    TQString etag; // ETag header.
    TQString lastModified; // Last modified.
    bool bMustRevalidate; // Cache entry is expired.
    long cacheExpireDateOffset; // Position in the cache entry where the
                                  // 16 byte expire date is stored.
    time_t expireDate; // Date when the cache entry will expire
    time_t creationDate; // Date when the cache entry was created
    TQString strCharset; // Charset

    // Indicates whether an error-page or error-msg should is preferred.
    bool bErrorPage;

    // Cookie flags
    bool bUseCookiejar;
    enum { CookiesAuto, CookiesManual, CookiesNone } cookieMode;
  };

  struct DigestAuthInfo
  {
    TQCString nc;
    TQCString qop;
    TQCString realm;
    TQCString nonce;
    TQCString method;
    TQCString cnonce;
    TQCString username;
    TQCString password;
    TQStrList digestURI;
    TQCString algorithm;
    TQCString entityBody;
  };

//---------------------- Re-implemented methods ----------------
  virtual void setHost(const TQString& host, int port, const TQString& user,
                       const TQString& pass);

  virtual void slave_status();

  virtual void get( const KURL& url );
  virtual void put( const KURL& url, int permissions, bool overwrite,
                    bool resume );

//----------------- Re-implemented methods for WebDAV -----------
  virtual void listDir( const KURL& url );
  virtual void mkdir( const KURL& url, int permissions );

  virtual void rename( const KURL& src, const KURL& dest, bool overwrite );
  virtual void copy( const KURL& src, const KURL& dest, int permissions, bool overwrite );
  virtual void del( const KURL& url, bool isfile );

  // ask the host whether it supports WebDAV & cache this info
  bool davHostOk();

  // send generic DAV request
  void davGeneric( const KURL& url, TDEIO::HTTP_METHOD method );

  // Send requests to lock and unlock resources
  void davLock( const KURL& url, const TQString& scope,
                const TQString& type, const TQString& owner );
  void davUnlock( const KURL& url );

  // Calls httpClose() and finished()
  void davFinished();

  // Handle error conditions
  TQString davError( int code = -1, TQString url = TQString::null );
//---------------------------- End WebDAV -----------------------

  /**
   * Special commands supported by this slave :
   * 1 - HTTP POST
   * 2 - Cache has been updated
   * 3 - SSL Certificate Cache has been updated
   * 4 - HTTP multi get
   * 5 - DAV LOCK     (see
   * 6 - DAV UNLOCK     README.webdav)
   */
  virtual void special( const TQByteArray &data );

  virtual void mimetype( const KURL& url);

  virtual void stat( const KURL& url );

  virtual void reparseConfiguration();

  virtual void closeConnection(); // Forced close of connection

  void post( const KURL& url );
  void multiGet(const TQByteArray &data);
  bool checkRequestURL( const KURL& );
  void cacheUpdate( const KURL &url, bool nocache, time_t expireDate);

  void httpError(); // Generate error message based on response code

  bool isOffline(const KURL &url); // Check network status

protected slots:
  void slotData(const TQByteArray &);
  void error( int _errid, const TQString &_text );

protected:
  int readChunked();    // Read a chunk
  int readLimited();    // Read maximum m_iSize bytes.
  int readUnlimited();  // Read as much as possible.

  /**
    * A "smart" wrapper around write that will use SSL_write or
    * write(2) depending on whether you've got an SSL connection or not.
    * The only shortcomming is that it uses the "global" file handles and
    * soforth.  So you can't really use this on individual files/sockets.
    */
  ssize_t write(const void *buf, size_t nbytes);

  /**
    * Another "smart" wrapper, this time around read that will
    * use SSL_read or read(2) depending on whether you've got an
    * SSL connection or not.
    */
  ssize_t read (void *b, size_t nbytes);

  char *gets (char *str, int size);

  void setRewindMarker();
  void rewind();

  /**
    * Add an encoding on to the appropriate stack this
    * is nececesary because transfer encodings and
    * content encodings must be handled separately.
    */
  void addEncoding(TQString, TQStringList &);

  void configAuth( char *, bool );

  bool httpOpen();             // Open transfer
  void httpClose(bool keepAlive);  // Close transfer

  bool httpOpenConnection();   // Open connection
  void httpCloseConnection();  // Close connection
  void httpCheckConnection();  // Check whether to keep connection.

  void forwardHttpResponseHeader();

  bool readHeader();

  bool sendBody();

  // where dataInternal == true, the content is to be made available
  // to an internal function.
  bool readBody( bool dataInternal = false );

  /**
   * Performs a WebDAV stat or list
   */
  void davSetRequest( const TQCString& requestXML );
  void davStatList( const KURL& url, bool stat = true );
  void davParsePropstats( const TQDomNodeList& propstats, TDEIO::UDSEntry& entry );
  void davParseActiveLocks( const TQDomNodeList& activeLocks,
                            uint& lockCount );

  /**
   * Parses a date & time string
   */
  long parseDateTime( const TQString& input, const TQString& type );

  /**
   * Returns the error code from a "HTTP/1.1 code Code Name" string
   */
  int codeFromResponse( const TQString& response );

  /**
   * Extracts locks from metadata
   * Returns the appropriate If: header
   */
  TQString davProcessLocks();

  /**
   * Send a cookie to the cookiejar
   */
  void addCookies( const TQString &url, const TQCString &cookieHeader);

  /**
   * Look for cookies in the cookiejar
   */
  TQString findCookies( const TQString &url);

  /**
   * Do a cache lookup for the current url. (m_state.url)
   *
   * @param readWrite If true, file is opened read/write.
   *                  If false, file is opened read-only.
   *
   * @return a file stream open for reading and at the start of
   *         the header section when the Cache entry exists and is valid.
   *         0 if no cache entry could be found, or if the entry is not
   *         valid (any more).
   */
  FILE *checkCacheEntry(bool readWrite = false);

  /**
   * Create a cache entry for the current url. (m_state.url)
   *
   * Set the contents type of the cache entry to 'mimetype'.
   */
  void createCacheEntry(const TQString &mimetype, time_t expireDate);

  /**
   * Write data to cache.
   *
   * Write 'nbytes' from 'buffer' to the Cache Entry File
   */
  void writeCacheEntry( const char *buffer, int nbytes);

  /**
   * Close cache entry
   */
  void closeCacheEntry();

  /**
   * Update expire time of current cache entry.
   */
  void updateExpireDate(time_t expireDate, bool updateCreationDate=false);

  /**
   * Quick check whether the cache needs cleaning.
   */
  void cleanCache();

  /**
   * Performs a GET HTTP request.
   */
  // where dataInternal == true, the content is to be made available
  // to an internal function.
  void retrieveContent( bool dataInternal = false );

  /**
   * Performs a HEAD HTTP request.
   */
  bool retrieveHeader(bool close_connection = true);

  /**
   * Resets any per session settings.
   */
  void resetSessionSettings();

  /**
   * Resets settings related to parsing a response.
   */
  void resetResponseSettings();

  /**
   * Resets any per connection settings.  These are different from
   * per-session settings in that they must be invalidates every time
   * a request is made, e.g. a retry to re-send the header to the
   * server, as compared to only when a new request arrives.
   */
  void resetConnectionSettings();

  /**
   * Returns any pre-cached proxy authentication info
   * info in HTTP header format.
   */
  TQString proxyAuthenticationHeader();

  /**
   * Retrieves authorization info from cache or user.
   */
  bool getAuthorization();

  /**
   * Saves valid authorization info in the cache daemon.
   */
  void saveAuthorization();

  /**
   * Creates the entity-header for Basic authentication.
   */
  TQString createBasicAuth( bool isForProxy = false );

  /**
   * Creates the entity-header for Digest authentication.
   */
  TQString createDigestAuth( bool isForProxy = false );

  /**
   * Creates the entity-header for NTLM authentication.
   */
  TQString createNTLMAuth( bool isForProxy = false );

  /**
   * Creates the entity-header for Negotiate authentication.
   */
  TQString createNegotiateAuth();

  /**
   * create GSS error string
   */
  TQCString gssError( int major_status, int minor_status );

  /**
   * Calcualtes the message digest response based on RFC 2617.
   */
  void calculateResponse( DigestAuthInfo &info, TQCString &Response );

  /**
   * Prompts the user for authorization retry.
   */
  bool retryPrompt();

  /**
   * Creates authorization prompt info.
   */
  void promptInfo( TDEIO::AuthInfo& info );

protected:
  HTTPState m_state;
  HTTPRequest m_request;
  TQPtrList<HTTPRequest> m_requestQueue;

  bool m_bBusy; // Busy handling request queue.
  bool m_bEOF;
  bool m_bEOD;

//--- Settings related to a single response only
  TQStringList m_responseHeader; // All headers
  KURL m_redirectLocation;
  bool m_bRedirect; // Indicates current request is a redirection

  // Processing related
  bool m_bChunked; // Chunked tranfer encoding
  TDEIO::filesize_t m_iSize; // Expected size of message
  TDEIO::filesize_t m_iBytesLeft; // # of bytes left to receive in this message.
  TDEIO::filesize_t m_iContentLeft; // # of content bytes left
  TQByteArray m_bufReceive; // Receive buffer
  bool m_dataInternal; // Data is for internal consumption
  char m_lineBuf[1024];
  char m_rewindBuf[8192];
  size_t m_rewindCount;
  char *m_linePtr;
  size_t m_lineCount;
  char *m_lineBufUnget;
  char *m_linePtrUnget;
  size_t m_lineCountUnget;

  // Mimetype determination
  bool m_cpMimeBuffer;
  TQByteArray m_mimeTypeBuffer;

  // Language/Encoding related
  TQStringList m_qTransferEncodings;
  TQStringList m_qContentEncodings;
  TQString m_sContentMD5;
  TQString m_strMimeType;


//--- WebDAV
  // Data structure to hold data which will be passed to an internal func.
  TQByteArray m_bufWebDavData;
  TQStringList m_davCapabilities;

  bool m_davHostOk;
  bool m_davHostUnsupported;
//----------

  // Holds the POST data so it won't get lost on if we
  // happend to get a 401/407 response when submitting,
  // a form.
  TQByteArray m_bufPOST;

  // Cache related
  int m_maxCacheAge; // Maximum age of a cache entry.
  long m_maxCacheSize; // Maximum cache size in Kb.
  TQString m_strCacheDir; // Location of the cache.



//--- Proxy related members
  bool m_bUseProxy;
  bool m_bNeedTunnel; // Whether we need to make a SSL tunnel
  bool m_bIsTunneled; // Whether we have an active SSL tunnel
  bool m_bProxyAuthValid;
  int m_iProxyPort;
  KURL m_proxyURL;
  TQString m_strProxyRealm;

  // Operation mode
  TQCString m_protocol;

  // Authentication
  TQString m_strRealm;
  TQString m_strAuthorization;
  TQString m_strProxyAuthorization;
  HTTP_AUTH Authentication;
  HTTP_AUTH ProxyAuthentication;
  bool m_bUnauthorized;
  short unsigned int m_iProxyAuthCount;
  short unsigned int m_iWWWAuthCount;

  // First request on a connection
  bool m_bFirstRequest;

  // Persistent connections
  bool m_bKeepAlive;
  int m_keepAliveTimeout; // Timeout in seconds.

  // Persistent proxy connections
  bool m_bPersistentProxyConnection;


  // Indicates whether there was some connection error.
  bool m_bError;

  // Previous and current response codes
  unsigned int m_responseCode;
  unsigned int m_prevResponseCode;

  // Values that determine the remote connection timeouts.
  int m_proxyConnTimeout;
  int m_remoteConnTimeout;
  int m_remoteRespTimeout;

  int m_pid;
};
#endif

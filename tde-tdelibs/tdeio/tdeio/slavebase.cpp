/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *  Copyright (c) 2000 David Faure <faure@kde.org>
 *  Copyright (c) 2000 Stephan Kulow <coolo@kde.org>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 **/

#include "slavebase.h"

#include <config.h>

#include <sys/time.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		// Needed on some systems.
#endif

#include <assert.h>
#include <kdebug.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <tqfile.h>

#include <dcopclient.h>

#include <tdeapplication.h>
#include <ksock.h>
#include <kcrash.h>
#include <tdesu/client.h>
#include <tdelocale.h>
#include <ksocks.h>

#include "kremoteencoding.h"

#include "tdeio/slavebase.h"
#include "tdeio/connection.h"
#include "tdeio/ioslave_defaults.h"
#include "tdeio/slaveinterface.h"

#include "uiserver_stub.h"

using namespace TDEIO;

template class TQPtrList<TQValueList<UDSAtom> >;
typedef TQValueList<TQCString> AuthKeysList;
typedef TQMap<TQString,TQCString> AuthKeysMap;
#define TDEIO_DATA TQByteArray data; TQDataStream stream( data, IO_WriteOnly ); stream
#define TDEIO_FILESIZE_T(x) (unsigned long)(x & 0xffffffff) << (unsigned long)(x >> 32)

namespace TDEIO {

class SlaveBaseConfig : public TDEConfigBase
{
public:
   SlaveBaseConfig(SlaveBase *_slave)
	: slave(_slave) { }

   bool internalHasGroup(const TQCString &) const { tqWarning("hasGroup(const TQCString &)");
return false; }

   TQStringList groupList() const { return TQStringList(); }

   TQMap<TQString,TQString> entryMap(const TQString &group) const
      { Q_UNUSED(group); return TQMap<TQString,TQString>(); }

   void reparseConfiguration() { }

   KEntryMap internalEntryMap( const TQString &pGroup) const { Q_UNUSED(pGroup); return KEntryMap(); }

   KEntryMap internalEntryMap() const { return KEntryMap(); }

   void putData(const KEntryKey &_key, const KEntry&_data, bool _checkGroup) 
   { Q_UNUSED(_key); Q_UNUSED(_data); Q_UNUSED(_checkGroup); }

   KEntry lookupData(const KEntryKey &_key) const
   {
     KEntry entry;
     TQString value = slave->metaData(_key.c_key);
     if (!value.isNull())
        entry.mValue = value.utf8();
     return entry;
   }
protected:
   SlaveBase *slave;
};


class SlaveBasePrivate {
public:
    TQString slaveid;
    bool resume:1;
    bool needSendCanResume:1;
    bool onHold:1;
    bool wasKilled:1;
    MetaData configData;
    SlaveBaseConfig *config;
    KURL onHoldUrl;

    struct timeval last_tv;
    TDEIO::filesize_t totalSize;
    TDEIO::filesize_t sentListEntries;
    DCOPClient *dcopClient;
    KRemoteEncoding *remotefile;
    time_t timeout;
    TQByteArray timeoutData;
};

}

static SlaveBase *globalSlave;
long SlaveBase::s_seqNr;

static volatile bool slaveWriteError = false;

static const char *s_protocol;

#ifdef Q_OS_UNIX
static void genericsig_handler(int sigNumber)
{
   signal(sigNumber,SIG_IGN);
   //WABA: Don't do anything that requires malloc, we can deadlock on it since
   //a SIGTERM signal can come in while we are in malloc/free.
   //kdDebug()<<"tdeioslave : exiting due to signal "<<sigNumber<<endl;
   //set the flag which will be checked in dispatchLoop() and which *should* be checked
   //in lengthy operations in the various slaves
   if (globalSlave!=0)
      globalSlave->setKillFlag();
   signal(SIGALRM,SIG_DFL);
   alarm(5);  //generate an alarm signal in 5 seconds, in this time the slave has to exit
}
#endif

//////////////

SlaveBase::SlaveBase( const TQCString &protocol,
                      const TQCString &pool_socket,
                      const TQCString &app_socket )
    : mProtocol(protocol), m_pConnection(0),
      mPoolSocket( TQFile::decodeName(pool_socket)),
      mAppSocket( TQFile::decodeName(app_socket))
{
    s_protocol = protocol.data();
#ifdef Q_OS_UNIX
    if (!getenv("TDE_DEBUG"))
    {
        TDECrash::setCrashHandler( sigsegv_handler );
        signal(SIGILL,&sigsegv_handler);
        signal(SIGTRAP,&sigsegv_handler);
        signal(SIGABRT,&sigsegv_handler);
        signal(SIGBUS,&sigsegv_handler);
        signal(SIGALRM,&sigsegv_handler);
        signal(SIGFPE,&sigsegv_handler);
#ifdef SIGPOLL
        signal(SIGPOLL, &sigsegv_handler);
#endif
#ifdef SIGSYS
        signal(SIGSYS, &sigsegv_handler);
#endif
#ifdef SIGVTALRM
        signal(SIGVTALRM, &sigsegv_handler);
#endif
#ifdef SIGXCPU
        signal(SIGXCPU, &sigsegv_handler);
#endif
#ifdef SIGXFSZ
        signal(SIGXFSZ, &sigsegv_handler);
#endif
    }

    struct sigaction act;
    act.sa_handler = sigpipe_handler;
    sigemptyset( &act.sa_mask );
    act.sa_flags = 0;
    sigaction( SIGPIPE, &act, 0 );

    signal(SIGINT,&genericsig_handler);
    signal(SIGQUIT,&genericsig_handler);
    signal(SIGTERM,&genericsig_handler);
#endif

    globalSlave=this;

    appconn = new Connection();
    listEntryCurrentSize = 100;
    struct timeval tp;
    gettimeofday(&tp, 0);
    listEntry_sec = tp.tv_sec;
    listEntry_usec = tp.tv_usec;
    mConnectedToApp = true;

    d = new SlaveBasePrivate;
    // by kahl for netmgr (need a way to identify slaves)
    d->slaveid = protocol;
    d->slaveid += TQString::number(getpid());
    d->resume = false;
    d->needSendCanResume = false;
    d->config = new SlaveBaseConfig(this);
    d->onHold = false;
    d->wasKilled=false;
    d->last_tv.tv_sec = 0;
    d->last_tv.tv_usec = 0;
//    d->processed_size = 0;
    d->totalSize=0;
    d->sentListEntries=0;
    d->timeout = 0;
    connectSlave(mAppSocket);

    d->dcopClient = 0;
    d->remotefile = 0;
}

SlaveBase::~SlaveBase()
{
    delete d;
    s_protocol = "";
}

DCOPClient *SlaveBase::dcopClient()
{
    if (!d->dcopClient)
    {
       d->dcopClient = TDEApplication::dcopClient();
       if (!d->dcopClient->isAttached())
          d->dcopClient->attach();
       d->dcopClient->setDaemonMode( true );
    }
    return d->dcopClient;
}

void SlaveBase::dispatchLoop()
{
#ifdef Q_OS_UNIX //TODO: WIN32
    fd_set rfds;
    int retval;

    while (true)
    {
       if (d->timeout && (d->timeout < time(0)))
       {
          TQByteArray data = d->timeoutData;
          d->timeout = 0;
          d->timeoutData = TQByteArray();
          special(data);
       }
       FD_ZERO(&rfds);

       assert(appconn->inited());
       int maxfd = appconn->fd_from();
       FD_SET(appconn->fd_from(), &rfds);
       if( d->dcopClient )
       {
           FD_SET( d->dcopClient->socket(), &rfds );
           if( d->dcopClient->socket() > maxfd )
               maxfd = d->dcopClient->socket();
       }

       if (!d->timeout) // we can wait forever
       {
          retval = select( maxfd + 1, &rfds, NULL, NULL, NULL);
       }
       else
       {
          struct timeval tv;
          tv.tv_sec = kMax(d->timeout-time(0),(time_t) 1);
          tv.tv_usec = 0;
          retval = select( maxfd + 1, &rfds, NULL, NULL, &tv);
       }
       if ((retval>0) && FD_ISSET(appconn->fd_from(), &rfds))
       { // dispatch application messages
          int cmd;
          TQByteArray data;
          if ( appconn->read(&cmd, data) != -1 )
          {
             dispatch(cmd, data);
          }
          else // some error occurred, perhaps no more application
          {
             // When the app exits, should the slave be put back in the pool ?
             if (mConnectedToApp && !mPoolSocket.isEmpty())
             {
                disconnectSlave();
                mConnectedToApp = false;
                closeConnection();
                connectSlave(mPoolSocket);
             }
             else
             {
                return;
             }
          }
       }
       if( retval > 0 && d->dcopClient && FD_ISSET( d->dcopClient->socket(), &rfds ))
       {
           d->dcopClient->processSocketData( d->dcopClient->socket());
       }
       if ((retval<0) && (errno != EINTR))
       {
          kdDebug(7019) << "dispatchLoop(): select returned " << retval << " "
            << (errno==EBADF?"EBADF":errno==EINTR?"EINTR":errno==EINVAL?"EINVAL":errno==ENOMEM?"ENOMEM":"unknown")
            << " (" << errno << ")" << endl;
          return;
       }
       //I think we get here when we were killed in dispatch() and not in select()
       if (wasKilled())
       {
          kdDebug(7019)<<" dispatchLoop() slave was killed, returning"<<endl;
          return;
       }
    }
#else
#error The TDEIO slave system only works under UNIX
#endif
}

void SlaveBase::connectSlave(const TQString& path)
{
#ifdef Q_OS_UNIX //TODO: TDESocket not yet available on WIN32
    appconn->init(new TDESocket(TQFile::encodeName(path).data()));
    if (!appconn->inited())
    {
        kdDebug(7019) << "SlaveBase: failed to connect to " << path << endl;
        exit();
    }

    setConnection(appconn);
#endif
}

void SlaveBase::disconnectSlave()
{
    appconn->close();
}

void SlaveBase::setMetaData(const TQString &key, const TQString &value)
{
   mOutgoingMetaData.replace(key, value);
}

TQString SlaveBase::metaData(const TQString &key) const
{
   if (mIncomingMetaData.contains(key))
      return mIncomingMetaData[key];
   if (d->configData.contains(key))
      return d->configData[key];
   return TQString::null;
}

bool SlaveBase::hasMetaData(const TQString &key) const
{
   if (mIncomingMetaData.contains(key))
      return true;
   if (d->configData.contains(key))
      return true;
   return false;
}

// ### remove the next two methods for KDE4 (they miss the const)
TQString SlaveBase::metaData(const TQString &key) {
   return const_cast<const SlaveBase*>(this)->metaData( key );
}
bool SlaveBase::hasMetaData(const TQString &key) {
   return const_cast<const SlaveBase*>(this)->hasMetaData( key );
}

TDEConfigBase *SlaveBase::config()
{
   return d->config;
}

void SlaveBase::sendMetaData()
{
   TDEIO_DATA << mOutgoingMetaData;

   slaveWriteError = false;
   m_pConnection->send( INF_META_DATA, data );
   if (slaveWriteError) exit();
   mOutgoingMetaData.clear(); // Clear
}

KRemoteEncoding *SlaveBase::remoteEncoding()
{
   if (d->remotefile != 0)
      return d->remotefile;

   return d->remotefile = new KRemoteEncoding(metaData("Charset").latin1());
}

void SlaveBase::data( const TQByteArray &data )
{
   if (!mOutgoingMetaData.isEmpty())
      sendMetaData();
   slaveWriteError = false;
   m_pConnection->send( MSG_DATA, data );
   if (slaveWriteError) exit();
}

void SlaveBase::dataReq( )
{
/*
   if (!mOutgoingMetaData.isEmpty())
      sendMetaData();
*/
   if (d->needSendCanResume)
      canResume(0);
   m_pConnection->send( MSG_DATA_REQ );
}

void SlaveBase::error( int _errid, const TQString &_text )
{
    mIncomingMetaData.clear(); // Clear meta data
    mOutgoingMetaData.clear();
    TDEIO_DATA << (TQ_INT32) _errid << _text;

    m_pConnection->send( MSG_ERROR, data );
    //reset
    listEntryCurrentSize = 100;
    d->sentListEntries=0;
    d->totalSize=0;
}

void SlaveBase::connected()
{
    slaveWriteError = false;
    m_pConnection->send( MSG_CONNECTED );
    if (slaveWriteError) exit();
}

void SlaveBase::finished()
{
    mIncomingMetaData.clear(); // Clear meta data
    if (!mOutgoingMetaData.isEmpty())
       sendMetaData();
    m_pConnection->send( MSG_FINISHED );

    // reset
    listEntryCurrentSize = 100;
    d->sentListEntries=0;
    d->totalSize=0;
}

void SlaveBase::needSubURLData()
{
    m_pConnection->send( MSG_NEED_SUBURL_DATA );
}

void SlaveBase::slaveStatus( const TQString &host, bool connected )
{
    pid_t pid = getpid();
    TQ_INT8 b = connected ? 1 : 0;
    TDEIO_DATA << pid << mProtocol << host << b;
    if (d->onHold)
       stream << d->onHoldUrl;
    m_pConnection->send( MSG_SLAVE_STATUS, data );
}

void SlaveBase::canResume()
{
    m_pConnection->send( MSG_CANRESUME );
}

void SlaveBase::totalSize( TDEIO::filesize_t _bytes )
{
    TDEIO_DATA << TDEIO_FILESIZE_T(_bytes);
    slaveWriteError = false;
    m_pConnection->send( INF_TOTAL_SIZE, data );
    if (slaveWriteError) exit();

    //this one is usually called before the first item is listed in listDir()
    struct timeval tp;
    gettimeofday(&tp, 0);
    listEntry_sec = tp.tv_sec;
    listEntry_usec = tp.tv_usec;
    d->totalSize=_bytes;
    d->sentListEntries=0;
}

void SlaveBase::processedSize( TDEIO::filesize_t _bytes )
{
    bool           emitSignal=false;
    struct timeval tv;
    int            gettimeofday_res=gettimeofday( &tv, 0L );

    if( _bytes == d->totalSize )
        emitSignal=true;
    else if ( gettimeofday_res == 0 ) {
        time_t msecdiff = 2000;
        if (d->last_tv.tv_sec) {
            // Compute difference, in ms
            msecdiff = 1000 * ( tv.tv_sec - d->last_tv.tv_sec );
            time_t usecdiff = tv.tv_usec - d->last_tv.tv_usec;
            if ( usecdiff < 0 ) {
                msecdiff--;
                msecdiff += 1000;
            }
            msecdiff += usecdiff / 1000;
        }
        emitSignal=msecdiff >= 100; // emit size 10 times a second
    }

    if( emitSignal ) {
        TDEIO_DATA << TDEIO_FILESIZE_T(_bytes);
        slaveWriteError = false;
        m_pConnection->send( INF_PROCESSED_SIZE, data );
            if (slaveWriteError) exit();
        if ( gettimeofday_res == 0 ) {
            d->last_tv.tv_sec = tv.tv_sec;
            d->last_tv.tv_usec = tv.tv_usec;
        }
    }
//    d->processed_size = _bytes;
}

void SlaveBase::processedPercent( float /* percent */ )
{
  kdDebug(7019) << "SlaveBase::processedPercent: STUB" << endl;
}


void SlaveBase::speed( unsigned long _bytes_per_second )
{
    TDEIO_DATA << (TQ_UINT32) _bytes_per_second;
    slaveWriteError = false;
    m_pConnection->send( INF_SPEED, data );
    if (slaveWriteError) exit();
}

void SlaveBase::redirection( const KURL& _url )
{
    TDEIO_DATA << _url;
    m_pConnection->send( INF_REDIRECTION, data );
}

void SlaveBase::errorPage()
{
    m_pConnection->send( INF_ERROR_PAGE );
}

static bool isSubCommand(int cmd)
{
   return ( (cmd == CMD_REPARSECONFIGURATION) ||
            (cmd == CMD_META_DATA) ||
            (cmd == CMD_CONFIG) ||
            (cmd == CMD_SUBURL) ||
            (cmd == CMD_SLAVE_STATUS) ||
            (cmd == CMD_SLAVE_CONNECT) ||
            (cmd == CMD_SLAVE_HOLD) ||
            (cmd == CMD_MULTI_GET));
}

void SlaveBase::mimeType( const TQString &_type)
{
  // kdDebug(7019) << "(" << getpid() << ") SlaveBase::mimeType '" << _type << "'" << endl;
  int cmd;
  do
  {
    // Send the meta-data each time we send the mime-type.
    if (!mOutgoingMetaData.isEmpty())
    {
      // kdDebug(7019) << "(" << getpid() << ") mimeType: emitting meta data" << endl;
      TDEIO_DATA << mOutgoingMetaData;
      m_pConnection->send( INF_META_DATA, data );
    }
    TDEIO_DATA << _type;
    m_pConnection->send( INF_MIME_TYPE, data );
    while(true)
    {
       cmd = 0;
       if ( m_pConnection->read( &cmd, data ) == -1 ) {
           kdDebug(7019) << "SlaveBase: mimetype: read error" << endl;
           exit();
       }
       // kdDebug(7019) << "(" << getpid() << ") Slavebase: mimetype got " << cmd << endl;
       if ( cmd == CMD_HOST) // Ignore.
          continue;
       if ( isSubCommand(cmd) )
       {
          dispatch( cmd, data );
          continue; // Disguised goto
       }
       break;
    }
  }
  while (cmd != CMD_NONE);
  mOutgoingMetaData.clear();
}

void SlaveBase::exit()
{
    this->~SlaveBase();
    ::exit(255);
}

void SlaveBase::warning( const TQString &_msg)
{
    TDEIO_DATA << _msg;
    m_pConnection->send( INF_WARNING, data );
}

void SlaveBase::infoMessage( const TQString &_msg)
{
    TDEIO_DATA << _msg;
    m_pConnection->send( INF_INFOMESSAGE, data );
}

bool SlaveBase::requestNetwork(const TQString& host)
{
    TDEIO_DATA << host << d->slaveid;
    m_pConnection->send( MSG_NET_REQUEST, data );

    if ( waitForAnswer( INF_NETWORK_STATUS, 0, data ) != -1 )
    {
        bool status;
        TQDataStream stream( data, IO_ReadOnly );
        stream >> status;
        return status;
    } else
        return false;
}

void SlaveBase::dropNetwork(const TQString& host)
{
    TDEIO_DATA << host << d->slaveid;
    m_pConnection->send( MSG_NET_DROP, data );
}

void SlaveBase::statEntry( const UDSEntry& entry )
{
    TDEIO_DATA << entry;
    slaveWriteError = false;
    m_pConnection->send( MSG_STAT_ENTRY, data );
    if (slaveWriteError) exit();
}

void SlaveBase::listEntry( const UDSEntry& entry, bool _ready )
{
   static struct timeval tp;
   static const int maximum_updatetime = 300;
   static const int minimum_updatetime = 100;

   if (!_ready) {
      pendingListEntries.append(entry);

      if (pendingListEntries.count() > listEntryCurrentSize) {
         gettimeofday(&tp, 0);

         long diff = ((tp.tv_sec - listEntry_sec) * 1000000 +
                      tp.tv_usec - listEntry_usec) / 1000;
         if (diff==0) diff=1;

         if (diff > maximum_updatetime) {
            listEntryCurrentSize = listEntryCurrentSize * 3 / 4;
            _ready = true;
         }
//if we can send all list entries of this dir which have not yet been sent
//within maximum_updatetime, then make listEntryCurrentSize big enough for all of them
         else if (((pendingListEntries.count()*maximum_updatetime)/diff) > (d->totalSize-d->sentListEntries))
            listEntryCurrentSize=d->totalSize-d->sentListEntries+1;
//if we are below minimum_updatetime, estimate how much we will get within
//maximum_updatetime
         else if (diff < minimum_updatetime)
            listEntryCurrentSize = (pendingListEntries.count() * maximum_updatetime) / diff;
         else
            _ready=true;
      }
   }
   if (_ready) { // may happen when we started with !ready
      listEntries( pendingListEntries );
      pendingListEntries.clear();

      gettimeofday(&tp, 0);
      listEntry_sec = tp.tv_sec;
      listEntry_usec = tp.tv_usec;
   }
}

void SlaveBase::listEntries( const UDSEntryList& list )
{
    TDEIO_DATA << (TQ_UINT32)list.count();
    UDSEntryListConstIterator it = list.begin();
    UDSEntryListConstIterator end = list.end();
    for (; it != end; ++it)
      stream << *it;
    slaveWriteError = false;
    m_pConnection->send( MSG_LIST_ENTRIES, data);
    if (slaveWriteError) exit();
    d->sentListEntries+=(uint)list.count();
}

void SlaveBase::sendAuthenticationKey( const TQCString& key,
                                       const TQCString& group,
                                       bool keepPass )
{
    TDEIO_DATA << key << group << keepPass;
    m_pConnection->send( MSG_AUTH_KEY, data );
}

void SlaveBase::delCachedAuthentication( const TQString& key )
{
    TDEIO_DATA << key.utf8() ;
    m_pConnection->send( MSG_DEL_AUTH_KEY, data );
}

void SlaveBase::sigsegv_handler(int sig)
{
#ifdef Q_OS_UNIX
    signal(sig,SIG_DFL); // Next one kills

    //Kill us if we deadlock
    signal(SIGALRM,SIG_DFL);
    alarm(5);  //generate an alarm signal in 5 seconds, in this time the slave has to exit

    // Debug and printf should be avoided because they might
    // call malloc.. and get in a nice recursive malloc loop
    char buffer[120];
    snprintf(buffer, sizeof(buffer), "tdeioslave: ####### CRASH ###### protocol = %s pid = %d signal = %d\n", s_protocol, getpid(), sig);
    if (write(2, buffer, strlen(buffer)) >= 0) {
#ifdef SECURE_DEBUG
	kdBacktraceFD();
#else // SECURE_DEBUG
	// Screw the malloc issue! We want nice demangled backtraces!
	// Anyway we are not supposed to go into infinite loop because next signal
	// will kill us. If you are unlucky and  there is a second crash during
	// backtrase in your system, you can define SECURE_DEBUG to avoid it

	// Extra sync here so we are sure even if the backtrace will fail
	// we will pass at least some crash message.
	fsync(2);
	TQString backtrace = kdBacktrace();
	if (write(2, backtrace.ascii(), backtrace.length()) < 0) {
		// FIXME
		// Could not write crash information
	}
#endif // SECURE_DEBUG
    }
    ::exit(1);
#endif
}

void SlaveBase::sigpipe_handler (int)
{
    // We ignore a SIGPIPE in slaves.
    // A SIGPIPE can happen in two cases:
    // 1) Communication error with application.
    // 2) Communication error with network.
    slaveWriteError = true;

    // Don't add anything else here, especially no debug output
}

void SlaveBase::setHost(TQString const &, int, TQString const &, TQString const &)
{
}

void SlaveBase::openConnection(void)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_CONNECT)); }
void SlaveBase::closeConnection(void)
{ } // No response!
void SlaveBase::stat(KURL const &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_STAT)); }
void SlaveBase::put(KURL const &, int, bool, bool)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_PUT)); }
void SlaveBase::special(const TQByteArray &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SPECIAL)); }
void SlaveBase::listDir(KURL const &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_LISTDIR)); }
void SlaveBase::get(KURL const & )
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_GET)); }
void SlaveBase::mimetype(KURL const &url)
{ get(url); }
void SlaveBase::rename(KURL const &, KURL const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_RENAME)); }
void SlaveBase::symlink(TQString const &, KURL const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SYMLINK)); }
void SlaveBase::copy(KURL const &, KURL const &, int, bool)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_COPY)); }
void SlaveBase::del(KURL const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_DEL)); }
void SlaveBase::mkdir(KURL const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_MKDIR)); }
void SlaveBase::chmod(KURL const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_CHMOD)); }
void SlaveBase::setSubURL(KURL const &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SUBURL)); }
void SlaveBase::multiGet(const TQByteArray &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_MULTI_GET)); }


void SlaveBase::slave_status()
{ slaveStatus( TQString::null, false ); }

void SlaveBase::reparseConfiguration()
{
}

void SlaveBase::localURL(const KURL& remoteURL)
{
    bool local = remoteURL.isLocalFile();
    TQ_INT8 islocal;
    KURL retURL;
    if (local) {
        islocal = true;
        retURL = remoteURL;
    }
    else {
        islocal = false;
        retURL = remoteURL;
    }
    TDEIO_DATA << islocal << retURL;
    m_pConnection->send( INF_LOCALURL, data );
}

bool SlaveBase::dispatch()
{
    assert( m_pConnection );

    int cmd;
    TQByteArray data;
    if ( m_pConnection->read( &cmd, data ) == -1 )
    {
        kdDebug(7019) << "SlaveBase::dispatch() has read error." << endl;
        return false;
    }

    dispatch( cmd, data );
    return true;
}

bool SlaveBase::openPassDlg( AuthInfo& info )
{
    return openPassDlg(info, TQString::null);
}

bool SlaveBase::openPassDlg( AuthInfo& info, const TQString &errorMsg )
{
    TQCString replyType;
    TQByteArray params;
    TQByteArray reply;
    AuthInfo authResult;
    long windowId = metaData("window-id").toLong();
    long progressId = metaData("progress-id").toLong();
    unsigned long userTimestamp = metaData("user-timestamp").toULong();

    kdDebug(7019) << "SlaveBase::openPassDlg window-id=" << windowId << " progress-id=" << progressId << endl;

    (void) dcopClient(); // Make sure to have a dcop client.

    UIServer_stub uiserver( "tdeio_uiserver", "UIServer" );
    if (progressId)
      uiserver.setJobVisible( progressId, false );

    TQDataStream stream(params, IO_WriteOnly);

    if (metaData("no-auth-prompt").lower() == "true")
       stream << info << TQString("<NoAuthPrompt>") << windowId << s_seqNr << userTimestamp;
    else
       stream << info << errorMsg << windowId << s_seqNr << userTimestamp;

    bool callOK = d->dcopClient->call( "kded", "kpasswdserver", "queryAuthInfo(TDEIO::AuthInfo, TQString, long int, long int, unsigned long int)",
                                        params, replyType, reply );

    if (progressId)
      uiserver.setJobVisible( progressId, true );

    if (!callOK)
    {
       kdWarning(7019) << "Can't communicate with kded_kpasswdserver (openPassDlg)!" << endl;
       return false;
    }

    if ( replyType == "TDEIO::AuthInfo" )
    {
       TQDataStream stream2( reply, IO_ReadOnly );
       stream2 >> authResult >> s_seqNr;
    }
    else
    {
       kdError(7019) << "DCOP function queryAuthInfo(...) returns "
                     << replyType << ", expected TDEIO::AuthInfo" << endl;
       return false;
    }

    if (!authResult.isModified())
       return false;

    info = authResult;

    kdDebug(7019) << "SlaveBase::openPassDlg: username=" << info.username << endl;
    kdDebug(7019) << "SlaveBase::openPassDlg: password=[hidden]" << endl;

    return true;
}

int SlaveBase::messageBox( MessageBoxType type, const TQString &text, const TQString &caption,
                           const TQString &buttonYes, const TQString &buttonNo )
{
    return messageBox( text, type, caption, buttonYes, buttonNo, TQString::null );
}

int SlaveBase::messageBox( const TQString &text, MessageBoxType type, const TQString &caption,
                           const TQString &buttonYes, const TQString &buttonNo, const TQString &dontAskAgainName )
{
    kdDebug(7019) << "messageBox " << type << " " << text << " - " << caption << buttonYes << buttonNo << endl;
    TDEIO_DATA << (TQ_INT32)type << text << caption << buttonYes << buttonNo << dontAskAgainName;
    m_pConnection->send( INF_MESSAGEBOX, data );
    if ( waitForAnswer( CMD_MESSAGEBOXANSWER, 0, data ) != -1 )
    {
        TQDataStream stream( data, IO_ReadOnly );
        int answer;
        stream >> answer;
        kdDebug(7019) << "got messagebox answer" << answer << endl;
        return answer;
    } else
        return 0; // communication failure
}

bool SlaveBase::canResume( TDEIO::filesize_t offset )
{
    kdDebug(7019) << "SlaveBase::canResume offset=" << TDEIO::number(offset) << endl;
    d->needSendCanResume = false;
    TDEIO_DATA << TDEIO_FILESIZE_T(offset);
    m_pConnection->send( MSG_RESUME, data );
    if ( offset )
    {
        int cmd;
        if ( waitForAnswer( CMD_RESUMEANSWER, CMD_NONE, data, &cmd ) != -1 )
        {
            kdDebug(7019) << "SlaveBase::canResume returning " << (cmd == CMD_RESUMEANSWER) << endl;
            return cmd == CMD_RESUMEANSWER;
        } else
            return false;
    }
    else // No resuming possible -> no answer to wait for
        return true;
}



int SlaveBase::waitForAnswer( int expected1, int expected2, TQByteArray & data, int *pCmd )
{
    int cmd, result;
    for (;;)
    {
        result = m_pConnection->read( &cmd, data );
        if ( result == -1 )
        {
            kdDebug(7019) << "SlaveBase::waitForAnswer has read error." << endl;
            return -1;
        }
        if ( cmd == expected1 || cmd == expected2 )
        {
            if ( pCmd ) *pCmd = cmd;
            return result;
        }
        if ( isSubCommand(cmd) )
        {
            dispatch( cmd, data );
        }
        else
        {
            kdWarning() << "Got cmd " << cmd << " while waiting for an answer!" << endl;
        }
    }
}


int SlaveBase::readData( TQByteArray &buffer)
{
   int result = waitForAnswer( MSG_DATA, 0, buffer );
   //kdDebug(7019) << "readData: length = " << result << " " << endl;
   return result;
}

void SlaveBase::setTimeoutSpecialCommand(int timeout, const TQByteArray &data)
{
   if (timeout > 0)
      d->timeout = time(0)+(time_t)timeout;
   else if (timeout == 0)
      d->timeout = 1; // Immediate timeout
   else
      d->timeout = 0; // Canceled

   d->timeoutData = data;
}

void SlaveBase::dispatch( int command, const TQByteArray &data )
{
    TQDataStream stream( data, IO_ReadOnly );

    KURL url;
    int i;

    switch( command ) {
    case CMD_HOST: {
        // Reset s_seqNr, see kpasswdserver/DESIGN
        s_seqNr = 0;
        TQString passwd;
        TQString host, user;
        stream >> host >> i >> user >> passwd;
        setHost( host, i, user, passwd );
    }
    break;
    case CMD_CONNECT:
        openConnection( );
        break;
    case CMD_DISCONNECT:
        closeConnection( );
        break;
    case CMD_SLAVE_STATUS:
        slave_status();
        break;
    case CMD_SLAVE_CONNECT:
    {
        d->onHold = false;
        TQString app_socket;
        TQDataStream stream( data, IO_ReadOnly);
        stream >> app_socket;
        appconn->send( MSG_SLAVE_ACK );
        disconnectSlave();
        mConnectedToApp = true;
        connectSlave(app_socket);
    } break;
    case CMD_SLAVE_HOLD:
    {
        KURL url;
        TQDataStream stream( data, IO_ReadOnly);
        stream >> url;
        d->onHoldUrl = url;
        d->onHold = true;
        disconnectSlave();
        mConnectedToApp = false;
        // Do not close connection!
        connectSlave(mPoolSocket);
    } break;
    case CMD_REPARSECONFIGURATION:
        reparseConfiguration();
        break;
    case CMD_CONFIG:
        stream >> d->configData;
#ifdef Q_OS_UNIX //TODO: not yet available on WIN32
        KSocks::setConfig(d->config);
#endif
	delete d->remotefile;
	d->remotefile = 0;
        break;
    case CMD_GET:
    {
        stream >> url;
        get( url );
    } break;
    case CMD_PUT:
    {
        int permissions;
        TQ_INT8 iOverwrite, iResume;
        stream >> url >> iOverwrite >> iResume >> permissions;
        bool overwrite = ( iOverwrite != 0 );
        bool resume = ( iResume != 0 );

        // Remember that we need to send canResume(), TransferJob is expecting
        // it. Well, in theory this shouldn't be done if resume is true.
        //   (the resume bool is currently unused)
        d->needSendCanResume = true   /* !resume */;

        put( url, permissions, overwrite, resume);
    } break;
    case CMD_STAT:
        stream >> url;
        stat( url );
        break;
    case CMD_MIMETYPE:
        stream >> url;
        mimetype( url );
        break;
    case CMD_LISTDIR:
        stream >> url;
        listDir( url );
        break;
    case CMD_MKDIR:
        stream >> url >> i;
        mkdir( url, i );
        break;
    case CMD_RENAME:
    {
        TQ_INT8 iOverwrite;
        KURL url2;
        stream >> url >> url2 >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        rename( url, url2, overwrite );
    } break;
    case CMD_SYMLINK:
    {
        TQ_INT8 iOverwrite;
        TQString target;
        stream >> target >> url >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        symlink( target, url, overwrite );
    } break;
    case CMD_COPY:
    {
        int permissions;
        TQ_INT8 iOverwrite;
        KURL url2;
        stream >> url >> url2 >> permissions >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        copy( url, url2, permissions, overwrite );
    } break;
    case CMD_DEL:
    {
        TQ_INT8 isFile;
        stream >> url >> isFile;
        del( url, isFile != 0);
    } break;
    case CMD_CHMOD:
        stream >> url >> i;
        chmod( url, i);
        break;
    case CMD_SPECIAL:
        special( data );
        break;
    case CMD_META_DATA:
        //kdDebug(7019) << "(" << getpid() << ") Incoming meta-data..." << endl;
        stream >> mIncomingMetaData;
        break;
    case CMD_SUBURL:
        stream >> url;
        setSubURL(url);
        break;
    case CMD_NONE:
        fprintf(stderr, "Got unexpected CMD_NONE!\n");
        break;
    case CMD_MULTI_GET:
        multiGet( data );
        break;
    case CMD_LOCALURL:
    {
        stream >> url;
        localURL( url );
    } break;
    default:
        // Some command we don't understand.
        // Just ignore it, it may come from some future version of KDE.
        break;
    }
}

TQString SlaveBase::createAuthCacheKey( const KURL& url )
{
    if( !url.isValid() )
        return TQString::null;

    // Generate the basic key sequence.
    TQString key = url.protocol();
    key += '-';
    key += url.host();
    int port = url.port();
    if( port )
    {
      key += ':';
      key += TQString::number(port);
    }

    return key;
}

bool SlaveBase::pingCacheDaemon() const
{
#ifdef Q_OS_UNIX
    // TODO: Ping kded / kpasswdserver
    TDEsuClient client;
    int success = client.ping();
    if( success == -1 )
    {
        success = client.startServer();
        if( success == -1 )
        {
            kdDebug(7019) << "Cannot start a new deamon!!" << endl;
            return false;
        }
        kdDebug(7019) << "Sucessfully started new cache deamon!!" << endl;
    }
    return true;
#else
    return false;
#endif
}

bool SlaveBase::checkCachedAuthentication( AuthInfo& info )
{
    TQCString replyType;
    TQByteArray params;
    TQByteArray reply;
    AuthInfo authResult;
    long windowId = metaData("window-id").toLong();
    unsigned long userTimestamp = metaData("user-timestamp").toULong();

    kdDebug(7019) << "SlaveBase::checkCachedAuthInfo window = " << windowId << " url = " << info.url.url() << endl;

    (void) dcopClient(); // Make sure to have a dcop client.

    TQDataStream stream(params, IO_WriteOnly);
    stream << info << windowId << userTimestamp;

    if ( !d->dcopClient->call( "kded", "kpasswdserver", "checkAuthInfo(TDEIO::AuthInfo, long int, unsigned long int)",
                               params, replyType, reply ) )
    {
       kdWarning(7019) << "Can't communicate with kded_kpasswdserver (checkCachedAuthentication)!" << endl;
       return false;
    }

    if ( replyType == "TDEIO::AuthInfo" )
    {
       TQDataStream stream2( reply, IO_ReadOnly );
       stream2 >> authResult;
    }
    else
    {
       kdError(7019) << "DCOP function checkAuthInfo(...) returns "
                     << replyType << ", expected TDEIO::AuthInfo" << endl;
       return false;
    }
    if (!authResult.isModified())
    {
       return false;
    }

    info = authResult;
    return true;
}

bool SlaveBase::cacheAuthentication( const AuthInfo& info )
{
    TQByteArray params;
    long windowId = metaData("window-id").toLong();

    (void) dcopClient(); // Make sure to have a dcop client.

    TQDataStream stream(params, IO_WriteOnly);
    stream << info << windowId;

    d->dcopClient->send( "kded", "kpasswdserver", "addAuthInfo(TDEIO::AuthInfo, long int)", params );

    return true;
}

int SlaveBase::connectTimeout()
{
    bool ok;
    TQString tmp = metaData("ConnectTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_CONNECT_TIMEOUT;
}

int SlaveBase::proxyConnectTimeout()
{
    bool ok;
    TQString tmp = metaData("ProxyConnectTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_PROXY_CONNECT_TIMEOUT;
}


int SlaveBase::responseTimeout()
{
    bool ok;
    TQString tmp = metaData("ResponseTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_RESPONSE_TIMEOUT;
}


int SlaveBase::readTimeout()
{
    bool ok;
    TQString tmp = metaData("ReadTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_READ_TIMEOUT;
}

bool SlaveBase::wasKilled() const
{
   return d->wasKilled;
}

void SlaveBase::setKillFlag()
{
   d->wasKilled=true;
}

void SlaveBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


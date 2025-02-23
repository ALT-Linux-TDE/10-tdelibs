/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "tdeio/slaveinterface.h"
#include "tdeio/slavebase.h"
#include "tdeio/connection.h"
#include <errno.h>
#include <assert.h>
#include <kdebug.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <tdeio/observer.h>
#include <tdeapplication.h>
#include <dcopclient.h>
#include <time.h>
#include <tqtimer.h>

using namespace TDEIO;


TQDataStream &operator <<(TQDataStream &s, const TDEIO::UDSEntry &e )
{
    // On 32-bit platforms we send UDS_SIZE with UDS_SIZE_LARGE in front
    // of it to carry the 32 msb. We can't send a 64 bit UDS_SIZE because
    // that would break the compatibility of the wire-protocol with KDE 2.
    // We do the same on 64-bit platforms in case we run in a mixed 32/64bit
    // environment.

    TQ_UINT32 size = 0;
    TDEIO::UDSEntry::ConstIterator it = e.begin();
    for( ; it != e.end(); ++it )
    {
       size++;
       if ((*it).m_uds == TDEIO::UDS_SIZE)
          size++;
    }
    s << size;
    it = e.begin();
    for( ; it != e.end(); ++it )
    {
       if ((*it).m_uds == TDEIO::UDS_SIZE)
       {
          TDEIO::UDSAtom a;
          a.m_uds = TDEIO::UDS_SIZE_LARGE;
          a.m_long = (*it).m_long >> 32;
          s << a;
       }
       s << *it;
    }
    return s;
}

TQDataStream &operator >>(TQDataStream &s, TDEIO::UDSEntry &e )
{
    e.clear();
    TQ_UINT32 size;
    s >> size;

    // On 32-bit platforms we send UDS_SIZE with UDS_SIZE_LARGE in front
    // of it to carry the 32 msb. We can't send a 64 bit UDS_SIZE because
    // that would break the compatibility of the wire-protocol with KDE 2.
    // We do the same on 64-bit platforms in case we run in a mixed 32/64bit
    // environment.
    TQ_LLONG msb = 0;
    for(TQ_UINT32 i = 0; i < size; i++)
    {
       TDEIO::UDSAtom a;
       s >> a;
       if (a.m_uds == TDEIO::UDS_SIZE_LARGE)
       {
          msb = a.m_long;
       }
       else
       {
          if (a.m_uds == TDEIO::UDS_SIZE)
          {
             if (a.m_long < 0)
                a.m_long += (TQ_LLONG) 1 << 32;
             a.m_long += msb << 32;
          }
          e.append(a);
          msb = 0;
       }
    }
    return s;
}

static const unsigned int max_nums = 8;

class TDEIO::SlaveInterfacePrivate
{
public:
  SlaveInterfacePrivate() {
    slave_calcs_speed = false;
    start_time.tv_sec = 0;
    start_time.tv_usec = 0;
    last_time = 0;
    nums = 0;
    filesize = 0;
    offset = 0;
  }
  bool slave_calcs_speed;
  struct timeval start_time;
  uint nums;
  long times[max_nums];
  TDEIO::filesize_t sizes[max_nums];
  size_t last_time;
  TDEIO::filesize_t filesize, offset;

  TQTimer speed_timer;
};

//////////////

SlaveInterface::SlaveInterface( Connection * connection )
{
    m_pConnection = connection;
    m_progressId = 0;

    d = new SlaveInterfacePrivate;
    connect(&d->speed_timer, TQ_SIGNAL(timeout()), TQ_SLOT(calcSpeed()));
}

SlaveInterface::~SlaveInterface()
{
    // Note: no kdDebug() here (scheduler is deleted very late)
    m_pConnection = 0; // a bit like the "wasDeleted" of TQObject...

    delete d;
}

static TDEIO::filesize_t readFilesize_t(TQDataStream &stream)
{
   TDEIO::filesize_t result;
   unsigned long ul;
   stream >> ul;
   result = ul;
   if (stream.atEnd())
      return result;
   stream >> ul;
   result += ((TDEIO::filesize_t)ul) << 32;
   return result;
}


bool SlaveInterface::dispatch()
{
    assert( m_pConnection );

    int cmd;
    TQByteArray data;

    if (m_pConnection->read( &cmd, data ) == -1)
      return false;

    return dispatch( cmd, data );
}

void SlaveInterface::calcSpeed()
{
  if (d->slave_calcs_speed) {
    d->speed_timer.stop();
    return;
  }

  struct timeval tv;
  gettimeofday(&tv, 0);

  long diff = ((tv.tv_sec - d->start_time.tv_sec) * 1000000 +
	       tv.tv_usec - d->start_time.tv_usec) / 1000;
  if (diff - d->last_time >= 900) {
    d->last_time = diff;
    if (d->nums == max_nums) {
      // let's hope gcc can optimize that well enough
      // otherwise I'd try memcpy :)
      for (unsigned int i = 1; i < max_nums; ++i) {
	d->times[i-1] = d->times[i];
	d->sizes[i-1] = d->sizes[i];
      }
      d->nums--;
    }
    d->times[d->nums] = diff;
    d->sizes[d->nums++] = d->filesize - d->offset;

    TDEIO::filesize_t lspeed = 1000 * (d->sizes[d->nums-1] - d->sizes[0]) / (d->times[d->nums-1] - d->times[0]);

//     kdDebug() << "proceeed " << (long)d->filesize << " " << diff << " " 
// 	      << long(d->sizes[d->nums-1] - d->sizes[0]) << " " 
// 	      <<  d->times[d->nums-1] - d->times[0] << " " 
// 	      << long(lspeed) << " " << double(d->filesize) / diff 
// 	      << " " << convertSize(lspeed) << " " 
// 	      << convertSize(long(double(d->filesize) / diff) * 1000) << " " 
// 	      <<  endl ;

    if (!lspeed) {
      d->nums = 1;
      d->times[0] = diff;
      d->sizes[0] = d->filesize - d->offset;
    }
    emit speed(lspeed);
  }
}

bool SlaveInterface::dispatch( int _cmd, const TQByteArray &rawdata )
{
    //kdDebug(7007) << "dispatch " << _cmd << endl;

    TQDataStream stream( rawdata, IO_ReadOnly );

    TQString str1;
    TQ_INT32 i;
    TQ_INT8 b;
    TQ_UINT32 ul;

    switch( _cmd ) {
    case MSG_DATA:
	emit data( rawdata );
	break;
    case MSG_DATA_REQ:
        emit dataReq();
	break;
    case MSG_FINISHED:
	//kdDebug(7007) << "Finished [this = " << this << "]" << endl;
        d->offset = 0;
        d->speed_timer.stop();
	emit finished();
	break;
    case MSG_STAT_ENTRY:
	{
	    UDSEntry entry;
	    stream >> entry;
	    emit statEntry(entry);
	}
	break;
    case MSG_LIST_ENTRIES:
	{
	    TQ_UINT32 count;
	    stream >> count;

	    UDSEntryList list;
	    UDSEntry entry;
	    for (uint i = 0; i < count; i++) {
		stream >> entry;
		list.append(entry);
	    }
	    emit listEntries(list);

	}
	break;
    case MSG_RESUME: // From the put job
	{
	    d->offset = readFilesize_t(stream);
	    emit canResume( d->offset );
	}
	break;
    case MSG_CANRESUME: // From the get job
        d->filesize = d->offset;
        emit canResume(0); // the arg doesn't matter
        break;
    case MSG_ERROR:
	stream >> i >> str1;
	kdDebug(7007) << "error " << i << " " << str1 << endl;
	emit error( i, str1 );
	break;
    case MSG_SLAVE_STATUS:
        {
           pid_t pid;
           TQCString protocol;
           stream >> pid >> protocol >> str1 >> b;
           emit slaveStatus(pid, protocol, str1, (b != 0));
        }
        break;
    case MSG_CONNECTED:
	emit connected();
	break;

    case INF_TOTAL_SIZE:
	{
	    TDEIO::filesize_t size = readFilesize_t(stream);
	    gettimeofday(&d->start_time, 0);
	    d->last_time = 0;
	    d->filesize = d->offset;
	    d->sizes[0] = d->filesize - d->offset;
	    d->times[0] = 0;
	    d->nums = 1;
	    d->speed_timer.start(1000);
	    d->slave_calcs_speed = false;
	    emit totalSize( size );
	}
	break;
    case INF_PROCESSED_SIZE:
	{
	    TDEIO::filesize_t size = readFilesize_t(stream);
	    emit processedSize( size );
	    d->filesize = size;
	}
	break;
    case INF_SPEED:
	stream >> ul;
	d->slave_calcs_speed = true;
	d->speed_timer.stop();

	emit speed( ul );
	break;
    case INF_GETTING_FILE:
	break;
    case INF_ERROR_PAGE:
	emit errorPage();
	break;
    case INF_REDIRECTION:
      {
	KURL url;
	stream >> url;

	emit redirection( url );
      }
      break;
    case INF_MIME_TYPE:
	stream >> str1;

	emit mimeType( str1 );
        if (!m_pConnection->suspended())
            m_pConnection->sendnow( CMD_NONE, TQByteArray() );
	break;
    case INF_WARNING:
	stream >> str1;

	emit warning( str1 );
	break;
    case INF_NEED_PASSWD: {
        AuthInfo info;
       	stream >> info;
	openPassDlg( info );
	break;
    }
    case INF_MESSAGEBOX: {
	kdDebug(7007) << "needs a msg box" << endl;
	TQString text, caption, buttonYes, buttonNo, dontAskAgainName;
        int type;
	stream >> type >> text >> caption >> buttonYes >> buttonNo;
	if (stream.atEnd())
	messageBox(type, text, caption, buttonYes, buttonNo);
	else {
	    stream >> dontAskAgainName;
	    messageBox(type, text, caption, buttonYes, buttonNo, dontAskAgainName);
	}
	break;
    }
    case INF_INFOMESSAGE: {
        TQString msg;
        stream >> msg;
        infoMessage(msg);
        break;
    }
    case INF_META_DATA: {
        MetaData meta_data;
        stream >> meta_data;
        metaData(meta_data);
        break;
    }
    case INF_LOCALURL: {
        TQ_INT8 islocal;
        KURL url;
        stream >> islocal >> url;
        emit localURL( url, islocal );
        break;
    }
    case MSG_NET_REQUEST: {
        TQString host;
	TQString slaveid;
        stream >> host >> slaveid;
        requestNetwork(host, slaveid);
        break;
    }
    case MSG_NET_DROP: {
        TQString host;
	TQString slaveid;
        stream >> host >> slaveid;
        dropNetwork(host, slaveid);
        break;
    }
    case MSG_NEED_SUBURL_DATA: {
        emit needSubURLData();
        break;
    }
    case MSG_AUTH_KEY: {
        bool keep;
        TQCString key, group;
        stream >> key >> group >> keep;
        kdDebug(7007) << "Got auth-key:      " << key << endl
                      << "    group-key:     " << group << endl
                      << "    keep password: " << keep << endl;
        emit authorizationKey( key, group, keep );
        break;
    }
    case MSG_DEL_AUTH_KEY: {
        TQCString key;
        stream >> key;
        kdDebug(7007) << "Delete auth-key: " << key << endl;
        emit delAuthorization( key );
    }
    default:
        kdWarning(7007) << "Slave sends unknown command (" << _cmd << "), dropping slave" << endl;
	return false;
    }
    return true;
}

void SlaveInterface::setOffset( TDEIO::filesize_t o)
{
    d->offset = o;
}

TDEIO::filesize_t SlaveInterface::offset() const { return d->offset; }

void SlaveInterface::requestNetwork(const TQString &host, const TQString &slaveid)
{
    kdDebug(7007) << "requestNetwork " << host << slaveid << endl;
    TQByteArray packedArgs;
    TQDataStream stream( packedArgs, IO_WriteOnly );
    stream << true;
    m_pConnection->sendnow( INF_NETWORK_STATUS, packedArgs );
}

void SlaveInterface::dropNetwork(const TQString &host, const TQString &slaveid)
{
    kdDebug(7007) << "dropNetwork " << host << slaveid << endl;
}

void SlaveInterface::sendResumeAnswer( bool resume )
{
    kdDebug(7007) << "SlaveInterface::sendResumeAnswer ok for resuming :" << resume << endl;
    m_pConnection->sendnow( resume ? CMD_RESUMEANSWER : CMD_NONE, TQByteArray() );
}

void SlaveInterface::openPassDlg( const TQString& prompt, const TQString& user, bool readOnly )
{
    AuthInfo info;
    info.prompt = prompt;
    info.username = user;
    info.readOnly = readOnly;
    openPassDlg( info );
}

void SlaveInterface::openPassDlg( const TQString& prompt, const TQString& user,
                                  const TQString& caption, const TQString& comment,
                                  const TQString& label, bool readOnly )
{
    AuthInfo info;
    info.prompt = prompt;
    info.username = user;
    info.caption = caption;
    info.comment = comment;
    info.commentLabel = label;
    info.readOnly = readOnly;
    openPassDlg( info );
}

void SlaveInterface::openPassDlg( AuthInfo& info )
{
    kdDebug(7007) << "SlaveInterface::openPassDlg: "
                  << "User= " << info.username
                  << ", Message= " << info.prompt << endl;
    bool result = Observer::self()->openPassDlg( info );
    if ( m_pConnection )
    {
        TQByteArray data;
        TQDataStream stream( data, IO_WriteOnly );
        if ( result )
        {
            stream << info;
            kdDebug(7007) << "SlaveInterface:::openPassDlg got: "
                          << "User= " << info.username
                          << ", Password= [hidden]" << endl;
            m_pConnection->sendnow( CMD_USERPASS, data );
        }
        else
            m_pConnection->sendnow( CMD_NONE, data );
    }
}

void SlaveInterface::messageBox( int type, const TQString &text, const TQString &_caption,
                                 const TQString &buttonYes, const TQString &buttonNo )
{
    messageBox( type, text, _caption, buttonYes, buttonNo, TQString::null );
}

void SlaveInterface::messageBox( int type, const TQString &text, const TQString &_caption,
                                 const TQString &buttonYes, const TQString &buttonNo, const TQString &dontAskAgainName )
{
    kdDebug(7007) << "messageBox " << type << " " << text << " - " << _caption << " " << dontAskAgainName << endl;
    TQByteArray packedArgs;
    TQDataStream stream( packedArgs, IO_WriteOnly );

    TQString caption( _caption );
    if ( type == TDEIO::SlaveBase::SSLMessageBox )
        caption = TQString::fromUtf8(kapp->dcopClient()->appId()); // hack, see observer.cpp

    emit needProgressId();
    kdDebug(7007) << "SlaveInterface::messageBox m_progressId=" << m_progressId << endl;
    TQGuardedPtr<SlaveInterface> me = this;
    m_pConnection->suspend();
    int result = Observer::/*self()->*/messageBox( m_progressId, type, text, caption, buttonYes, buttonNo, dontAskAgainName );
    if ( me && m_pConnection ) // Don't do anything if deleted meanwhile
    {
        m_pConnection->resume();
        kdDebug(7007) << this << " SlaveInterface result=" << result << endl;
        stream << result;
        m_pConnection->sendnow( CMD_MESSAGEBOXANSWER, packedArgs );
    }
}

// No longer used.
// Remove in KDE 4.0
void SlaveInterface::sigpipe_handler(int)
{
    int saved_errno = errno;
    // Using kdDebug from a signal handler is not a good idea.
#ifndef NDEBUG
    char msg[1000];
    sprintf(msg, "*** SIGPIPE *** (ignored, pid = %ld)\n", (long) getpid());
    if (write(2, msg, strlen(msg)) < 0) {
        // FIXME
        // Could not write error message
        // Triple fault? ;-)
    }
#endif

    // Do nothing.
    // dispatch will return false and that will trigger ERR_SLAVE_DIED in slave.cpp
    errno = saved_errno;
}

void SlaveInterface::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "slaveinterface.moc"

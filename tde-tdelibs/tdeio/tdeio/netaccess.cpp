/*  $Id$

    This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999 David Faure (faure@kde.org)

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

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <cstring>

#include <tqstring.h>
#include <tqapplication.h>
#include <tqfile.h>
#include <tqmetaobject.h>

#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdetempfile.h>
#include <kdebug.h>
#include <kurl.h>
#include <tdeio/job.h>
#include <tdeio/scheduler.h>

#include "tdeio/netaccess.h"

using namespace TDEIO;

TQString * NetAccess::lastErrorMsg;
int NetAccess::lastErrorCode = 0;
TQStringList* NetAccess::tmpfiles;

bool NetAccess::download(const KURL& u, TQString & target)
{
  return NetAccess::download (u, target, 0);
}

bool NetAccess::download(const KURL& u, TQString & target, TQWidget* window)
{
  if (u.isLocalFile()) {
    // file protocol. We do not need the network
    target = u.path();
    bool accessible = checkAccess(target, R_OK);
    if(!accessible)
    {
        if(!lastErrorMsg)
            lastErrorMsg = new TQString;
        *lastErrorMsg = i18n("File '%1' is not readable").arg(target);
        lastErrorCode = ERR_COULD_NOT_READ;
    }
    return accessible;
  }

  if (target.isEmpty())
  {
      KTempFile tmpFile;
      target = tmpFile.name();
      if (!tmpfiles)
          tmpfiles = new TQStringList;
      tmpfiles->append(target);
  }

  NetAccess kioNet;
  KURL dest;
  dest.setPath( target );
  return kioNet.filecopyInternal( u, dest, -1, true /*overwrite*/,
                                  false, window, false /*copy*/);
}

bool NetAccess::upload(const TQString& src, const KURL& target)
{
  return NetAccess::upload(src, target, 0);
}

bool NetAccess::upload(const TQString& src, const KURL& target, TQWidget* window)
{
  if (target.isEmpty())
    return false;

  // If target is local... well, just copy. This can be useful
  // when the client code uses a temp file no matter what.
  // Let's make sure it's not the exact same file though
  if (target.isLocalFile() && target.path() == src)
    return true;

  NetAccess kioNet;
  KURL s;
  s.setPath(src);
  return kioNet.filecopyInternal( s, target, -1, true /*overwrite*/,
                                  false, window, false /*copy*/ );
}

bool NetAccess::copy( const KURL & src, const KURL & target )
{
  return NetAccess::file_copy( src, target, -1, false /*not overwrite*/, false, 0L );
}

bool NetAccess::copy( const KURL & src, const KURL & target, TQWidget* window )
{
  return NetAccess::file_copy( src, target, -1, false /*not overwrite*/, false, window );
}

bool NetAccess::file_copy( const KURL& src, const KURL& target, int permissions,
                           bool overwrite, bool resume, TQWidget* window )
{
  NetAccess kioNet;
  return kioNet.filecopyInternal( src, target, permissions, overwrite, resume,
                                  window, false /*copy*/ );
}


bool NetAccess::file_move( const KURL& src, const KURL& target, int permissions,
                           bool overwrite, bool resume, TQWidget* window )
{
  NetAccess kioNet;
  return kioNet.filecopyInternal( src, target, permissions, overwrite, resume,
                                  window, true /*move*/ );
}

bool NetAccess::dircopy( const KURL & src, const KURL & target )
{
  return NetAccess::dircopy( src, target, 0 );
}

bool NetAccess::dircopy( const KURL & src, const KURL & target, TQWidget* window )
{
  KURL::List srcList;
  srcList.append( src );
  return NetAccess::dircopy( srcList, target, window );
}

bool NetAccess::dircopy( const KURL::List & srcList, const KURL & target, TQWidget* window )
{
  NetAccess kioNet;
  return kioNet.dircopyInternal( srcList, target, window, false /*copy*/ );
}

bool NetAccess::move( const KURL& src, const KURL& target, TQWidget* window )
{
  KURL::List srcList;
  srcList.append( src );
  return NetAccess::move( srcList, target, window );
}

bool NetAccess::move( const KURL::List& srcList, const KURL& target, TQWidget* window )
{
  NetAccess kioNet;
  return kioNet.dircopyInternal( srcList, target, window, true /*move*/ );
}

bool NetAccess::exists( const KURL & url )
{
  return NetAccess::exists( url, false, 0 );
}

bool NetAccess::exists( const KURL & url, TQWidget* window )
{
  return NetAccess::exists( url, false, window );
}

bool NetAccess::exists( const KURL & url, bool source )
{
  return NetAccess::exists( url, source, 0 );
}

bool NetAccess::exists( const KURL & url, bool source, TQWidget* window )
{
  if ( url.isLocalFile() )
    return TQFile::exists( url.path() );
  NetAccess kioNet;
  return kioNet.statInternal( url, 0 /*no details*/, source, window );
}

KURL NetAccess::localURL(const KURL& url, TQWidget* window)
{
  NetAccess kioNet;
  return kioNet.localURLInternal( url, window );
}

bool NetAccess::stat( const KURL & url, TDEIO::UDSEntry & entry )
{
  return NetAccess::stat( url, entry, 0 );
}

bool NetAccess::stat( const KURL & url, TDEIO::UDSEntry & entry, TQWidget* window )
{
  NetAccess kioNet;
  bool ret = kioNet.statInternal( url, 2 /*all details*/, true /*source*/, window );
  if (ret)
    entry = kioNet.m_entry;
  return ret;
}

KURL NetAccess::mostLocalURL(const KURL & url, TQWidget* window)
{
  if ( url.isLocalFile() )
  {
    return url;
  }

  TDEIO::UDSEntry entry;
  if (!stat(url, entry, window))
  {
    return url;
  }

  TQString path;

  // Extract the local path from the TDEIO::UDSEntry
  TDEIO::UDSEntry::ConstIterator it = entry.begin();
  const TDEIO::UDSEntry::ConstIterator end = entry.end();
  for ( ; it != end; ++it )
  {
    if ( (*it).m_uds == TDEIO::UDS_LOCAL_PATH )
    {
      path = (*it).m_str;
      break;
    }
  }

  if ( !path.isEmpty() )
  {
    KURL new_url;
    new_url.setPath(path);
    return new_url;
  }

  return url;
}


bool NetAccess::del( const KURL & url )
{
  return NetAccess::del( url, 0 );
}

bool NetAccess::del( const KURL & url, TQWidget* window )
{
  NetAccess kioNet;
  return kioNet.delInternal( url, window );
}

bool NetAccess::mkdir( const KURL & url, int permissions )
{
  return NetAccess::mkdir( url, 0, permissions );
}

bool NetAccess::mkdir( const KURL & url, TQWidget* window, int permissions )
{
  NetAccess kioNet;
  return kioNet.mkdirInternal( url, permissions, window );
}

TQString NetAccess::fish_execute( const KURL & url, const TQString command, TQWidget* window )
{
  NetAccess kioNet;
  return kioNet.fish_executeInternal( url, command, window );
}

bool NetAccess::synchronousRun( Job* job, TQWidget* window, TQByteArray* data,
                                KURL* finalURL, TQMap<TQString, TQString>* metaData )
{
  NetAccess kioNet;
  return kioNet.synchronousRunInternal( job, window, data, finalURL, metaData );
}

TQString NetAccess::mimetype( const KURL& url )
{
  NetAccess kioNet;
  return kioNet.mimetypeInternal( url, 0 );
}

TQString NetAccess::mimetype( const KURL& url, TQWidget* window )
{
  NetAccess kioNet;
  return kioNet.mimetypeInternal( url, window );
}

void NetAccess::removeTempFile(const TQString& name)
{
  if (!tmpfiles)
    return;
  if (tmpfiles->contains(name))
  {
    unlink(TQFile::encodeName(name));
    tmpfiles->remove(name);
  }
}

bool NetAccess::filecopyInternal(const KURL& src, const KURL& target, int permissions,
                                 bool overwrite, bool resume, TQWidget* window, bool move)
{
  bJobOK = true; // success unless further error occurs

  TDEIO::Scheduler::checkSlaveOnHold(true);
  TDEIO::Job * job = move
                   ? TDEIO::file_move( src, target, permissions, overwrite, resume )
                   : TDEIO::file_copy( src, target, permissions, overwrite, resume );
  job->setWindow (window);
  connect( job, TQ_SIGNAL( result (TDEIO::Job *) ),
           this, TQ_SLOT( slotResult (TDEIO::Job *) ) );

  enter_loop();
  return bJobOK;
}

bool NetAccess::dircopyInternal(const KURL::List& src, const KURL& target,
                                TQWidget* window, bool move)
{
  bJobOK = true; // success unless further error occurs

  TDEIO::Job * job = move
                   ? TDEIO::move( src, target )
                   : TDEIO::copy( src, target );
  job->setWindow (window);
  connect( job, TQ_SIGNAL( result (TDEIO::Job *) ),
           this, TQ_SLOT( slotResult (TDEIO::Job *) ) );

  enter_loop();
  return bJobOK;
}

bool NetAccess::statInternal( const KURL & url, int details, bool source,
                              TQWidget* window )
{
  bJobOK = true; // success unless further error occurs
  TDEIO::StatJob * job = TDEIO::stat( url, !url.isLocalFile() );
  job->setWindow (window);
  job->setDetails( details );
  job->setSide( source );
  connect( job, TQ_SIGNAL( result (TDEIO::Job *) ),
           this, TQ_SLOT( slotResult (TDEIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

KURL NetAccess::localURLInternal( const KURL & url, TQWidget* window )
{
  m_localURL = url;
  TDEIO::LocalURLJob* job = TDEIO::localURL(url);
  job->setWindow (window);
  connect(job, TQ_SIGNAL( localURL(TDEIO::Job*, const KURL&, bool) ),
           this, TQ_SLOT( slotLocalURL(TDEIO::Job*, const KURL&, bool) ));
  enter_loop();
  return m_localURL;
}

bool NetAccess::delInternal( const KURL & url, TQWidget* window )
{
  bJobOK = true; // success unless further error occurs
  TDEIO::Job * job = TDEIO::del( url );
  job->setWindow (window);
  connect( job, TQ_SIGNAL( result (TDEIO::Job *) ),
           this, TQ_SLOT( slotResult (TDEIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

bool NetAccess::mkdirInternal( const KURL & url, int permissions,
                               TQWidget* window )
{
  bJobOK = true; // success unless further error occurs
  TDEIO::Job * job = TDEIO::mkdir( url, permissions );
  job->setWindow (window);
  connect( job, TQ_SIGNAL( result (TDEIO::Job *) ),
           this, TQ_SLOT( slotResult (TDEIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

TQString NetAccess::mimetypeInternal( const KURL & url, TQWidget* window )
{
  bJobOK = true; // success unless further error occurs
  m_mimetype = TQString::fromLatin1("unknown");
  TDEIO::Job * job = TDEIO::mimetype( url );
  job->setWindow (window);
  connect( job, TQ_SIGNAL( result (TDEIO::Job *) ),
           this, TQ_SLOT( slotResult (TDEIO::Job *) ) );
  connect( job, TQ_SIGNAL( mimetype (TDEIO::Job *, const TQString &) ),
           this, TQ_SLOT( slotMimetype (TDEIO::Job *, const TQString &) ) );
  enter_loop();
  return m_mimetype;
}

void NetAccess::slotMimetype( TDEIO::Job *, const TQString & type  )
{
  m_mimetype = type;
}

void NetAccess::slotLocalURL(TDEIO::Job*, const KURL & url, bool)
{
  m_localURL = url;
  tqApp->exit_loop();
}

TQString NetAccess::fish_executeInternal(const KURL & url, const TQString command, TQWidget* window)
{
  TQString target, remoteTempFileName, resultData;
  KURL tempPathUrl;
  KTempFile tmpFile;
  tmpFile.setAutoDelete( true );

  if( url.protocol() == "fish" )
  {
    // construct remote temp filename
    tempPathUrl = url;
    remoteTempFileName = tmpFile.name();
    // only need the filename KTempFile adds some KDE specific dirs
    // that probably does not exist on the remote side
    int pos = remoteTempFileName.findRev('/');
    remoteTempFileName = "/tmp/fishexec_" + remoteTempFileName.mid(pos + 1);
    tempPathUrl.setPath( remoteTempFileName );
    bJobOK = true; // success unless further error occurs
    TQByteArray packedArgs;
    TQDataStream stream( packedArgs, IO_WriteOnly );

    stream << int('X') << tempPathUrl << command;

    TDEIO::Job * job = TDEIO::special( tempPathUrl, packedArgs, true );
    job->setWindow( window );
    connect( job, TQ_SIGNAL( result (TDEIO::Job *) ),
             this, TQ_SLOT( slotResult (TDEIO::Job *) ) );
    enter_loop();

    // since the TDEIO::special does not provide feedback we need to download the result
    if( NetAccess::download( tempPathUrl, target, window ) )
    {
      TQFile resultFile( target );

      if (resultFile.open( IO_ReadOnly ))
      {
        TQTextStream ts( &resultFile );
        ts.setEncoding( TQTextStream::Locale ); // Locale??
        resultData = ts.read();
        resultFile.close();
        NetAccess::del( tempPathUrl, window );
      }
    }
  }
  else
  {
    resultData = i18n( "ERROR: Unknown protocol '%1'" ).arg( url.protocol() );
  }
  return resultData;
}

bool NetAccess::synchronousRunInternal( Job* job, TQWidget* window, TQByteArray* data,
                                        KURL* finalURL, TQMap<TQString,TQString>* metaData )
{
  job->setWindow( window );

  m_metaData = metaData;
  if ( m_metaData ) {
      for ( TQMap<TQString, TQString>::iterator it = m_metaData->begin(); it != m_metaData->end(); ++it ) {
          job->addMetaData( it.key(), it.data() );
      }
  }

  if ( finalURL ) {
      SimpleJob *sj = dynamic_cast<SimpleJob*>( job );
      if ( sj ) {
          m_url = sj->url();
      }
  }

  connect( job, TQ_SIGNAL( result (TDEIO::Job *) ),
           this, TQ_SLOT( slotResult (TDEIO::Job *) ) );

  TQMetaObject *meta = job->metaObject();

  static const char dataSignal[] = "data(TDEIO::Job*,const " "TQByteArray" "&)";
  if ( meta->findSignal( dataSignal ) != -1 ) {
      connect( job, TQ_SIGNAL(data(TDEIO::Job*,const TQByteArray&)),
               this, TQ_SLOT(slotData(TDEIO::Job*,const TQByteArray&)) );
  }

  static const char redirSignal[] = "redirection(TDEIO::Job*,const KURL&)";
  if ( meta->findSignal( redirSignal ) != -1 ) {
      connect( job, TQ_SIGNAL(redirection(TDEIO::Job*,const KURL&)),
               this, TQ_SLOT(slotRedirection(TDEIO::Job*, const KURL&)) );
  }

  enter_loop();

  if ( finalURL )
      *finalURL = m_url;
  if ( data )
      *data = m_data;

  return bJobOK;
}

// If a troll sees this, he kills me
void tqt_enter_modal( TQWidget *widget );
void tqt_leave_modal( TQWidget *widget );

void NetAccess::enter_loop()
{
  TQWidget dummy(0,0,(WFlags)(WType_Dialog | WShowModal));
  dummy.setFocusPolicy( TQWidget::NoFocus );
  tqt_enter_modal(&dummy);
  tqApp->enter_loop();
  tqt_leave_modal(&dummy);
}

void NetAccess::slotResult( TDEIO::Job * job )
{
  lastErrorCode = job->error();
  bJobOK = !job->error();
  if ( !bJobOK ) {
    if ( !lastErrorMsg ) {
      lastErrorMsg = new TQString;
    }
    *lastErrorMsg = job->errorString();
  }
  if ( job->isA("TDEIO::StatJob") ) {
    m_entry = static_cast<TDEIO::StatJob *>(job)->statResult();
  }

  if ( m_metaData ) {
    *m_metaData = job->metaData();
  }

  tqApp->exit_loop();
}

void NetAccess::slotData( TDEIO::Job*, const TQByteArray& data )
{
  if ( data.isEmpty() ) {
    return;
  }

  unsigned offset = m_data.size();
  m_data.resize( offset + data.size() );
  std::memcpy( m_data.data() + offset, data.data(), data.size() );
}

void NetAccess::slotRedirection( TDEIO::Job*, const KURL& url )
{
  m_url = url;
}

#include "netaccess.moc"

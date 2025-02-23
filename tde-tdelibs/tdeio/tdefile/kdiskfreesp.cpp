/*
 * kdiskfreesp.cpp
 *
 * Copyright (c) 1999 Michael Kropfberger <michael.kropfberger@gmx.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
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
 */

#include "kdiskfreesp.h"
#include <tqfile.h>
#include <tqtextstream.h>

#include <kdebug.h>
#include <kprocess.h>
#include <tdeio/global.h>
#include <config-tdefile.h>

#include "kdiskfreesp.moc"

#define DF_COMMAND    "df"
#define DF_ARGS       "-k"
#define NO_FS_TYPE    true

#define BLANK ' '
#define FULL_PERCENT 95.0

/***************************************************************************
  * constructor
**/
KDiskFreeSp::KDiskFreeSp(TQObject *parent, const char *name)
    : TQObject(parent,name)
{
    dfProc = new TDEProcess(); TQ_CHECK_PTR(dfProc);
    dfProc->setEnvironment("LANGUAGE", "C");
    connect( dfProc, TQ_SIGNAL(receivedStdout(TDEProcess *, char *, int) ),
             this, TQ_SLOT (receivedDFStdErrOut(TDEProcess *, char *, int)) );
    connect(dfProc,TQ_SIGNAL(processExited(TDEProcess *) ),
            this, TQ_SLOT(dfDone() ) );

    readingDFStdErrOut=false;
}


/***************************************************************************
  * destructor
**/
KDiskFreeSp::~KDiskFreeSp()
{
    delete dfProc;
}

/***************************************************************************
  * is called, when the df-command writes on StdOut
**/
void KDiskFreeSp::receivedDFStdErrOut(TDEProcess *, char *data, int len)
{
  TQCString tmp(data,len+1);  // adds a zero-byte
  dfStringErrOut.append(tmp);
}

/***************************************************************************
  * reads the df-commands results
**/
int KDiskFreeSp::readDF( const TQString & mountPoint )
{
  if (readingDFStdErrOut || dfProc->isRunning())
    return -1;
  m_mountPoint = mountPoint;
  dfStringErrOut=""; // yet no data received
  dfProc->clearArguments();
  (*dfProc) << TQString::fromLocal8Bit(DF_COMMAND) << TQString::fromLocal8Bit(DF_ARGS);
  if (!dfProc->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ))
     kdError() << "could not execute ["<< DF_COMMAND << "]" << endl;
  return 1;
}


/***************************************************************************
  * is called, when the df-command has finished
**/
void KDiskFreeSp::dfDone()
{
  readingDFStdErrOut=true;

  TQTextStream t (dfStringErrOut, IO_ReadOnly);
  t.setEncoding(TQTextStream::Locale);
  TQString s=t.readLine();
  if ( (s.isEmpty()) || ( s.left(10) != TQString::fromLatin1("Filesystem") ) )
    kdError() << "Error running df command... got [" << s << "]" << endl;
  while ( !t.eof() ) {
    TQString u,v;
    s=t.readLine();
    s=s.simplifyWhiteSpace();
    if ( !s.isEmpty() ) {
      //kdDebug(tdefile_area) << "GOT: [" << s << "]" << endl;

      if (s.find(BLANK)<0)      // devicename was too long, rest in next line
	if ( !t.eof() ) {       // just appends the next line
            v=t.readLine();
            s=s.append(v);
            s=s.simplifyWhiteSpace();
            //kdDebug(tdefile_area) << "SPECIAL GOT: [" << s << "]" << endl;
	 }//if silly linefeed

      //kdDebug(tdefile_area) << "[" << s << "]" << endl;

      //TQString deviceName = s.left(s.find(BLANK));
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug(tdefile_area) << "    DeviceName:    [" << deviceName << "]" << endl;

      if (!NO_FS_TYPE)
          s=s.remove(0,s.find(BLANK)+1 ); // eat fs type

      u=s.left(s.find(BLANK));
      unsigned long kBSize = u.toULong();
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug(tdefile_area) << "    Size:       [" << kBSize << "]" << endl;

      u=s.left(s.find(BLANK));
      unsigned long kBUsed = u.toULong();
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug(tdefile_area) << "    Used:       [" << kBUsed << "]" << endl;

      u=s.left(s.find(BLANK));
      unsigned long kBAvail = u.toULong();
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug(tdefile_area) << "    Avail:       [" << kBAvail << "]" << endl;


      s=s.remove(0,s.find(BLANK)+1 );  // delete the capacity 94%
      TQString mountPoint = s.stripWhiteSpace();
      //kdDebug(tdefile_area) << "    MountPoint:       [" << mountPoint << "]" << endl;

      if ( mountPoint == m_mountPoint )
      {
        //kdDebug(tdefile_area) << "Found mount point. Emitting" << endl;
        emit foundMountPoint( mountPoint, kBSize, kBUsed, kBAvail );
        emit foundMountPoint( kBSize, kBUsed, kBAvail, mountPoint ); // sic!
      }
    }//if not header
  }//while further lines available

  readingDFStdErrOut=false;
  emit done();
  delete this;
}

KDiskFreeSp * KDiskFreeSp::findUsageInfo( const TQString & path )
{
    KDiskFreeSp * job = new KDiskFreeSp;
    TQString mountPoint = TDEIO::findPathMountPoint( path );
    job->readDF( mountPoint );
    return job;
}

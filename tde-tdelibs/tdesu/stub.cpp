/*
 *
 * $Id$
 *
 * This file is part of the KDE project, module tdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * This is free software; you can use this library under the GNU Library 
 * General Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 *
 * stub.cpp: Conversation with tdesu_stub.
 */

#include <config.h>
#include <stdlib.h>
#include <unistd.h>

#include <tqglobal.h>
#include <tqcstring.h>
#include <kdatastream.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <dcopclient.h>

#include "stub.h"
#include "kcookie.h"


StubProcess::StubProcess()
{
    m_User = "root";
    m_Scheduler = SchedNormal;
    m_Priority = 50;
    m_pCookie = new KCookie;
    m_bXOnly = true;
    m_bDCOPForwarding = false;
}


StubProcess::~StubProcess()
{
    delete m_pCookie;
}


void StubProcess::setPriority(int prio)
{
    if (prio > 100)
	m_Priority = 100;
    else if (prio < 0)
	m_Priority = 0;
    else
	m_Priority = prio;
}


TQCString StubProcess::commaSeparatedList(QCStringList lst)
{
    if (lst.count() == 0)
	return TQCString("");

    QCStringList::Iterator it = lst.begin();
    TQCString str = *it;
    for (it++; it!=lst.end(); it++) 
    {
	str += ',';
	str += *it;
    }
    return str;
}
    
/*
 * Conversation with tdesu_stub. This is how we pass the authentication
 * tokens (X11, DCOP) and other stuff to tdesu_stub.
 * return values: -1 = error, 0 = ok, 1 = kill me
 */

int StubProcess::ConverseStub(int check)
{
    TQCString line, tmp;
    while (1) 
    {
	line = readLine();
	if (line.isNull())
	    return -1;

	if (line == "tdesu_stub") 
	{
	    // This makes parsing a lot easier.
	    enableLocalEcho(false);
	    if (check) writeLine("stop");
	    else writeLine("ok");
	} else if (line == "display") {
	    writeLine(display());
	} else if (line == "display_auth") {
#ifdef TQ_WS_X11
	    writeLine(displayAuth());
#else
	    writeLine("");
#endif
	} else if (line == "dcopserver") {
	    if (m_bDCOPForwarding)
	       writeLine(dcopServer());
	    else
	       writeLine("no");
	} else if (line == "dcop_auth") {
	    if (m_bDCOPForwarding)
	       writeLine(dcopAuth());
	    else
	       writeLine("no");
	} else if (line == "ice_auth") {
	    if (m_bDCOPForwarding)
	       writeLine(iceAuth());
	    else
	       writeLine("no");
	} else if (line == "command") {
	    writeLine(m_Command);
	} else if (line == "path") {
	    TQCString path = getenv("PATH");
            if (!path.isEmpty() && path[0] == ':')
                path = path.mid(1);
	    if (m_User == "root")
	       if (!path.isEmpty())
	          path = "/usr/local/sbin:/usr/sbin:/sbin:" + path;
	       else
                  if (strcmp(__TDE_BINDIR, "/usr/bin") == 0) {
		          path = "/usr/local/sbin:/usr/sbin:/sbin:/usr/local/bin:/usr/bin:/bin";
		  }
		  else {
			  path = "/usr/local/sbin:/usr/sbin:/sbin:/usr/local/bin:" __TDE_BINDIR ":/usr/bin:/bin";
		  }
	    writeLine(path);
	} else if (line == "user") {
	    writeLine(m_User);
	} else if (line == "priority") {
	    tmp.setNum(m_Priority);
	    writeLine(tmp);
	} else if (line == "scheduler") {
	    if (m_Scheduler == SchedRealtime) writeLine("realtime");
	    else writeLine("normal");
	} else if (line == "xwindows_only") {
	    if (m_bXOnly) writeLine("no");
	    else writeLine("yes");
	} else if (line == "app_startup_id") {
	    QCStringList env = environment();
	    TQCString tmp;
	    for( QCStringList::ConstIterator it = env.begin();
		 it != env.end();
		 ++it )
	    {
		if( (*it).find( "DESKTOP_STARTUP_ID=" ) == 0 )
		    tmp = (*it).mid( strlen( "DESKTOP_STARTUP_ID=" ));
	    }
	    if( tmp.isEmpty())
		tmp = "0";
	    writeLine(tmp);
	} else if (line == "app_start_pid") { // obsolete
	    tmp.setNum(getpid());
	    writeLine(tmp);
	} else if (line == "environment") { // additional env vars
	    QCStringList env = environment();
	    for( QCStringList::ConstIterator it = env.begin();
		 it != env.end();
		 ++it )
		writeLine( *it );
	    writeLine( "" );
	} else if (line == "end") {
	    return 0;
	} else 
	{
	    kdWarning(900) << k_lineinfo << "Unknown request: -->" << line 
		           << "<--\n";
	    return 1;
	}
    }

    return 0;
}


void StubProcess::notifyTaskbar(const TQString &)
{
    kdWarning(900) << "Obsolete StubProcess::notifyTaskbar() called!" << endl;
}

void StubProcess::virtual_hook( int id, void* data )
{ PtyProcess::virtual_hook( id, data ); }

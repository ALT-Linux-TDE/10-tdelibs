/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
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
 **/

/*
 * Implementation of simple checking mechanism. Rules are defined in
 * the form of an URI. Available syntax is:
 *	- exec:/<execname>	->	check for an executable in
 *					$PATH variable.
 *	- config:/path/to/file	->      check for the existence of a file
 *					or directory in KDE or standard
 *					UNIX config locations
 *	- file:/path/to/file
 *	- dir:/path/to/dir	->	simply check the existence of the
 *					a file or directory
 *	- service:/serv 	->	try to connect to a port on the
 *					specified host (usually localhost)
 *					"serv" can be a port value or service name
 *
 * TO BE IMPLEMENTED:
 *	- run:/<execname>	->	check for a running executable
 */

#include "tdeprintcheck.h"

#include <kstandarddirs.h>
#include <kdebug.h>
#include <kextsock.h>
#include <tqfile.h>
#include <unistd.h>

static const char* const config_stddirs[] = {
	"/etc/",
	"/usr/etc/",
	"/usr/local/etc/",
	"/opt/etc/",
	"/opt/local/etc/",
	0
};

bool KdeprintChecker::check(TDEConfig *conf, const TQString& group)
{
	if (!group.isEmpty())
		conf->setGroup(group);
	TQStringList	uris = conf->readListEntry("Require");
	return check(uris);
}

bool KdeprintChecker::check(const TQStringList& uris)
{
	bool	state(true);
	for (TQStringList::ConstIterator it=uris.begin(); it!=uris.end() && state; ++it)
	{
		state = (state && checkURL(KURL(*it)));
		// kdDebug( 500 ) << "auto-detection uri=" << *it << ", state=" << state << endl;
	}
	return state;
}

bool KdeprintChecker::checkURL(const KURL& url)
{
	TQString	prot(url.protocol());
	if (prot == "config")
		return checkConfig(url);
	else if (prot == "exec")
		return checkExec(url);
	else if (prot == "file" || prot == "dir")
		return TDEStandardDirs::exists(url.url());
	else if (prot == "service")
		return checkService(url);
	return false;
}

bool KdeprintChecker::checkConfig(const KURL& url)
{
	// get the config filename (may contain a path)
	TQString	f(url.path().mid(1));
	bool	state(false);

	// first check for standard KDE config file
	if (!locate("config",f).isEmpty())
		state = true;
	else
	// otherwise check in standard UNIX config directories
	{
		const char* const *p = config_stddirs;
		while (*p)
		{
			// kdDebug( 500 ) << "checkConfig() with " << TQString::fromLatin1( *p ) + f << endl;
			if ( TQFile::exists( TQString::fromLatin1( *p ) + f ) )
			{
				state = true;
				break;
			}
			else
				p++;
		}
	}
	return state;
}

bool KdeprintChecker::checkExec(const KURL& url)
{
	TQString	execname(url.path().mid(1));
	return !(TDEStandardDirs::findExe(execname).isEmpty());
}

bool KdeprintChecker::checkService(const KURL& url)
{
	TQString	serv(url.path().mid(1));
	KExtendedSocket	sock;

	bool	ok;
	int	port = serv.toInt(&ok);

	if (ok) sock.setAddress("localhost", port);
	else sock.setAddress("localhost", serv);
	return (sock.connect() == 0);
}

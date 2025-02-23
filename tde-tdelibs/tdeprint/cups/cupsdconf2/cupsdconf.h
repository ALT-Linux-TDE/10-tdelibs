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

#ifndef	CUPSDCONF_H
#define	CUPSDCONF_H

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqptrlist.h>
#include <tqtextstream.h>
#include <tqpair.h>

#include "cupsdcomment.h"

enum LogLevelType { LOGLEVEL_DEBUG2 = 0, LOGLEVEL_DEBUG, LOGLEVEL_INFO, LOGLEVEL_WARN, LOGLEVEL_ERROR, LOGLEVEL_NONE };
enum OrderType { ORDER_ALLOW_DENY = 0, ORDER_DENY_ALLOW };
enum AuthTypeType { AUTHTYPE_NONE = 0, AUTHTYPE_BASIC, AUTHTYPE_DIGEST };
enum AuthClassType { AUTHCLASS_ANONYMOUS = 0, AUTHCLASS_USER, AUTHCLASS_SYSTEM, AUTHCLASS_GROUP };
enum EncryptionType { ENCRYPT_ALWAYS = 0, ENCRYPT_NEVER, ENCRYPT_REQUIRED, ENCRYPT_IFREQUESTED };
enum BrowseProtocolType { BROWSE_ALL = 0, BROWSE_CUPS, BROWSE_SLP };
enum PrintcapFormatType { PRINTCAP_BSD = 0, PRINTCAP_SOLARIS };
enum HostnameLookupType { HOSTNAME_OFF = 0, HOSTNAME_ON, HOSTNAME_DOUBLE };
enum ClassificationType { CLASS_NONE = 0, CLASS_CLASSIFIED, CLASS_CONFIDENTIAL, CLASS_SECRET, CLASS_TOPSECRET, CLASS_UNCLASSIFIED, CLASS_OTHER };
enum SatisfyType { SATISFY_ALL = 0, SATISFY_ANY };
enum UnitType { UNIT_KB = 0, UNIT_MB, UNIT_GB, UNIT_TILE };

struct CupsLocation;
struct CupsResource;
enum ResourceType { RESOURCE_GLOBAL, RESOURCE_PRINTER, RESOURCE_CLASS, RESOURCE_ADMIN };

struct CupsdConf
{
// functions member
	CupsdConf();
	~CupsdConf();

	bool loadFromFile(const TQString& filename);
	bool saveToFile(const TQString& filename);
	bool parseOption(const TQString& line);
	bool parseLocation(CupsLocation *location, TQTextStream& file);

	bool loadAvailableResources();

	static CupsdConf* get();
	static void release();

// data members
	static CupsdConf	*unique_;

	// Server
	TQString	servername_;
	TQString	serveradmin_;
	int classification_;
	TQString otherclassname_;
	bool classoverride_;
	TQString charset_;
	TQString language_;
	TQString printcap_;
	int printcapformat_;

	// Security
	TQString remoteroot_;
	TQString systemgroup_;
	TQString encryptcert_;
	TQString encryptkey_;
	TQPtrList<CupsLocation> locations_;
	TQPtrList<CupsResource> resources_;

	// Network
	int hostnamelookup_;
	bool keepalive_;
	int keepalivetimeout_;
	int maxclients_;
	TQString maxrequestsize_;
	int clienttimeout_;
	TQStringList listenaddresses_;

	// Log
	TQString accesslog_;
	TQString errorlog_;
	TQString pagelog_;
	TQString maxlogsize_;
	int loglevel_;

	// Jobs
	bool keepjobhistory_;
	bool keepjobfiles_;
	bool autopurgejobs_;
	int maxjobs_;
	int maxjobsperprinter_;
	int maxjobsperuser_;

	// Filter
	TQString user_;
	TQString group_;
	TQString ripcache_;
	int filterlimit_;

	// Directories
	TQString datadir_;
	TQString documentdir_;
	TQStringList fontpath_;
	TQString requestdir_;
	TQString serverbin_;
	TQString serverfiles_;
	TQString tmpfiles_;

	// Browsing
	bool browsing_;
	TQStringList browseprotocols_;
	int browseport_;
	int browseinterval_;
	int browsetimeout_;
	TQStringList browseaddresses_;
	int browseorder_;
	bool useimplicitclasses_;
	bool hideimplicitmembers_;
	bool useshortnames_;
	bool useanyclasses_;
	
	// cupsd.conf file comments
	CupsdComment	comments_;
	
	// unrecognized options
	TQValueList< TQPair<TQString,TQString> >	unknown_;
};

struct CupsLocation
{
	CupsLocation();
	CupsLocation(const CupsLocation& loc);

	bool parseOption(const TQString& line);
	bool parseResource(const TQString& line);

	CupsResource	*resource_;
	TQString	resourcename_;
	int	authtype_;
	int	authclass_;
	TQString	authname_;
	int	encryption_;
	int	satisfy_;
	int	order_;
	TQStringList	addresses_;
};

struct CupsResource
{
	CupsResource();
	CupsResource(const TQString& path);

	void setPath(const TQString& path);

	int	type_;
	TQString	path_;
	TQString	text_;

	static TQString textToPath(const TQString& text);
	static TQString pathToText(const TQString& path);
	static int typeFromPath(const TQString& path);
	static int typeFromText(const TQString& text);
	static TQString typeToIconName(int type);
};

#endif

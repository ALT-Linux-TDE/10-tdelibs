/* This file is part of the KDE libraries
    Copyright (C) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include <kurl.h>

#include <tqobject.h>
#include <tqptrlist.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqguardedptr.h>
#include <tqdom.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kdebug.h>
#include <tdeio/jobclasses.h>
#include <tdeio/global.h>
#include <tdeio/http.h>
#include <tdeio/davjob.h>
#include <tdeio/job.h>
#include <tdeio/slaveinterface.h>

#define TDEIO_ARGS TQByteArray packedArgs; TQDataStream stream( packedArgs, IO_WriteOnly ); stream

using namespace TDEIO;

class DavJob::DavJobPrivate
{
public:
  TQByteArray savedStaticData;
	TQByteArray str_response; // replaces the TQString previously used in DavJob itself
};

DavJob::DavJob( const KURL& url, int method, const TQString& request, bool showProgressInfo )
  : TransferJob( url, TDEIO::CMD_SPECIAL, TQByteArray(), TQByteArray(), showProgressInfo )
{
  d = new DavJobPrivate;
  // We couldn't set the args when calling the parent constructor,
  // so do it now.
  TQDataStream stream( m_packedArgs, IO_WriteOnly );
  stream << (int) 7 << url << method;
  // Same for static data
  if ( ! request.isEmpty() && ! request.isNull() ) {
    staticData = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" + request.utf8();
    staticData.truncate( staticData.size() - 1 );
    d->savedStaticData = staticData.copy();
  }
}

void DavJob::slotData( const TQByteArray& data )
{
  if(m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || m_error) {
    unsigned int oldSize = d->str_response.size();
    d->str_response.resize( oldSize + data.size() );
    memcpy( d->str_response.data() + oldSize, data.data(), data.size() );
  }
}

void DavJob::slotFinished()
{
  // kdDebug(7113) << "DavJob::slotFinished()" << endl;
  // kdDebug(7113) << d->str_response << endl;
	if (!m_redirectionURL.isEmpty() && m_redirectionURL.isValid() && (m_command == CMD_SPECIAL)) {
		TQDataStream istream( m_packedArgs, IO_ReadOnly );
		int s_cmd, s_method;
		KURL s_url;
		istream >> s_cmd;
		istream >> s_url;
		istream >> s_method;
		// PROPFIND
		if ( (s_cmd == 7) && (s_method == (int)TDEIO::DAV_PROPFIND) ) {
			m_packedArgs.truncate(0);
			TQDataStream stream( m_packedArgs, IO_WriteOnly );
			stream << (int)7 << m_redirectionURL << (int)TDEIO::DAV_PROPFIND;
		}
  } else if ( ! m_response.setContent( d->str_response, true ) ) {
		// An error occurred parsing the XML response
		TQDomElement root = m_response.createElementNS( "DAV:", "error-report" );
		m_response.appendChild( root );

		TQDomElement el = m_response.createElementNS( "DAV:", "offending-response" );
    TQDomText textnode = m_response.createTextNode( d->str_response );
		el.appendChild( textnode );
		root.appendChild( el );
		delete d; // Should be in virtual destructor
		d = 0;
	} else {
		delete d; // Should be in virtual destructor
		d = 0;
	}
  // kdDebug(7113) << m_response.toString() << endl;
	TransferJob::slotFinished();
	if( d ) staticData = d->savedStaticData.copy(); // Need to send DAV request to this host too
}

/* Convenience methods */

// KDE 4: Make it const TQString &
DavJob* TDEIO::davPropFind( const KURL& url, const TQDomDocument& properties, TQString depth, bool showProgressInfo )
{
  DavJob *job = new DavJob( url, (int) TDEIO::DAV_PROPFIND, properties.toString(), showProgressInfo );
  job->addMetaData( "davDepth", depth );
  return job;
}


DavJob* TDEIO::davPropPatch( const KURL& url, const TQDomDocument& properties, bool showProgressInfo )
{
  return new DavJob( url, (int) TDEIO::DAV_PROPPATCH, properties.toString(), showProgressInfo );
}

DavJob* TDEIO::davSearch( const KURL& url, const TQString& nsURI, const TQString& qName, const TQString& query, bool showProgressInfo )
{
  TQDomDocument doc;
  TQDomElement searchrequest = doc.createElementNS( "DAV:", "searchrequest" );
  TQDomElement searchelement = doc.createElementNS( nsURI, qName );
  TQDomText text = doc.createTextNode( query );
  searchelement.appendChild( text );
  searchrequest.appendChild( searchelement );
  doc.appendChild( searchrequest );
  return new DavJob( url, TDEIO::DAV_SEARCH, doc.toString(), showProgressInfo );
}

#include "davjob.moc"

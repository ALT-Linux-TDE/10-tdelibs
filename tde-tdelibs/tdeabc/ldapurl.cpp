/*
    This file is part of libtdeabc.
    Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General  Public
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

#include <kdebug.h>
#include <tqstringlist.h>
#include <tqdir.h>

#include "ldapurl.h"

using namespace TDEABC;

LDAPUrl::LDAPUrl()
{
  m_scope = Base;
}

LDAPUrl::LDAPUrl(const KURL &_url)
  : KURL(_url), m_extensions()
{
  m_dn = path();
  if ( !TQDir::isRelativePath(m_dn) )
#ifdef TQ_WS_WIN
    m_dn.remove(0,3); // e.g. "c:/"
#else
    m_dn.remove(0,1);
#endif
  parseQuery();
}

void LDAPUrl::setDn( const TQString &dn)
{
  m_dn = dn;
  if ( !TQDir::isRelativePath(m_dn) )
#ifdef TQ_WS_WIN
    m_dn.remove(0,3); // e.g. "c:/"
#else
    m_dn.remove(0,1);
#endif
  setPath(m_dn);
}

bool LDAPUrl::hasExtension( const TQString &key ) const
{
  return m_extensions.contains( key );
}

LDAPUrl::Extension LDAPUrl::extension( const TQString &key ) const
{
  TQMap<TQString, Extension>::const_iterator it;

  it = m_extensions.find( key );
  if ( it != m_extensions.constEnd() )
    return (*it);
  else {
    Extension ext;
    ext.value = "";
    ext.critical = false;
    return ext;
  }
}

TQString LDAPUrl::extension( const TQString &key, bool &critical ) const
{
  Extension ext;

  ext = extension( key );
  critical = ext.critical;
  return ext.value;
}

void LDAPUrl::setExtension( const TQString &key, const LDAPUrl::Extension &ext )
{
  m_extensions[ key ] = ext;
  updateQuery();
}

void LDAPUrl::setExtension( const TQString &key, const TQString &value, bool critical )
{
  Extension ext;
  ext.value = value;
  ext.critical = critical;
  setExtension( key, ext );
}

void LDAPUrl::removeExtension( const TQString &key )
{
  m_extensions.remove( key );
  updateQuery();
}

void LDAPUrl::updateQuery()
{
  Extension ext;
  TQMap<TQString, Extension>::iterator it;
  TQString q = "?";

  // set the attributes to query
  if ( m_attributes.count() > 0 ) q += m_attributes.join(",");

  // set the scope
  q += "?";
  switch( m_scope ) {
    case Sub:
      q += "sub";
      break;
    case One:
      q += "one";
      break;
    case Base:
      q += "base";
      break;
  }

  // set the filter
  q += "?";
  if ( m_filter != "(objectClass=*)" && !m_filter.isEmpty() )
    q += m_filter;

  // set the extensions
  q += "?";
  for ( it = m_extensions.begin(); it != m_extensions.end(); ++it ) {
    if ( it.data().critical ) q += "!";
    q += it.key();
    if ( !it.data().value.isEmpty() ) 
      q += "=" + it.data().value;
    q += ",";
  }
  while  ( q.endsWith("?") || q.endsWith(",") )
    q.remove( q.length() - 1, 1 );

  setQuery(q);
  kdDebug(5700) << "LDAP URL updateQuery(): " << prettyURL() << endl;
}

void LDAPUrl::parseQuery()
{
  Extension ext;
  TQStringList extensions;
  TQString q = query();
  // remove first ?
  if (q.startsWith("?"))
    q.remove(0,1);

  // split into a list
  TQStringList url_items = TQStringList::split("?", q, true);

  m_attributes.clear();
  m_scope = Base;
  m_filter = "(objectClass=*)";
  m_extensions.clear();

  int i = 0;
  for ( TQStringList::Iterator it = url_items.begin(); it != url_items.end(); ++it, i++ ) {
    switch (i) {
      case 0:
        m_attributes = TQStringList::split(",", (*it), false);
        break;
      case 1:
        if ( (*it) == "sub" ) m_scope = Sub; else
        if ( (*it) == "one") m_scope = One;
        break;
      case 2:
        m_filter = decode_string( *it );
        break;
      case 3:
        extensions = TQStringList::split(",", (*it), false);
        break;
    }
  }

  TQString name,value;
  for ( TQStringList::Iterator it = extensions.begin(); it != extensions.end(); ++it ) {
    ext.critical = false;
    name = decode_string( (*it).section('=',0,0) ).lower();
    value = decode_string( (*it).section('=',1) );
    if ( name.startsWith("!") ) {
      ext.critical = true;
      name.remove(0, 1);
    }
    kdDebug(5700) << "LDAPUrl extensions name= " << name << " value: " << value << endl;
    ext.value = value.replace( "%2", "," );
    setExtension( name, ext );
  }
}

/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

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

#include "testldapclient.h"

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdecmdlineargs.h>

#include <tqeventloop.h>

#include <assert.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  TDEApplication::disableAutoDcopRegistration();
  TDECmdLineArgs::init(argc,argv,"testldapclient", 0, 0, 0, 0);
  TDEApplication app (/*stylesEnabled=*/ false, /*GUIEnabled=*/ false);

  TestLDAPClient test;
  test.setup();
  test.runAll();
  test.cleanup();
  kdDebug() << "All tests OK." << endl;
  return 0;
}

void TestLDAPClient::setup()
{
}

void TestLDAPClient::runAll()
{
  testIntevation();
}

bool TestLDAPClient::check(const TQString& txt, TQString a, TQString b)
{
  if (a.isEmpty())
    a = TQString::null;
  if (b.isEmpty())
    b = TQString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    cleanup();
    exit(1);
  }
  return true;
}

void TestLDAPClient::cleanup()
{
  mClient = 0;
}

void TestLDAPClient::testIntevation()
{
  kdDebug() << k_funcinfo << endl;
  mClient = new LdapClient( this );

  mClient->setHost( "ca.intevation.de" );
  mClient->setPort( "389" );
  mClient->setBase( "o=Intevation GmbH,c=de" );

  // Same list as in kaddressbook's ldapsearchdialog
  TQStringList attrs;
  attrs << "l" << "Company" << "co" << "department" << "description" << "mail" << "facsimileTelephoneNumber" << "cn" << "homePhone" << "mobile" << "o" << "pager" << "postalAddress" << "st" << "street" << "title" << "uid" << "telephoneNumber" << "postalCode" << "objectClass";
  // the list from ldapclient.cpp
  //attrs << "cn" << "mail" << "givenname" << "sn" << "objectClass";
  mClient->setAttrs( attrs );

  // Taken from LdapSearch
  //TQString mSearchText = TQString::fromUtf8( "Till" );
  //TQString filter = TQString( "&(|(objectclass=person)(objectclass=groupOfNames)(mail=*))(|(cn=%1*)(mail=%2*)(givenName=%3*)(sn=%4*))" )
  //                 .arg( mSearchText ).arg( mSearchText ).arg( mSearchText ).arg( mSearchText );

  // For some reason a fromUtf8 broke the search for me (no results).
  // But this certainly looks fishy, it might break on non-utf8 systems.
  TQString filter = "&(|(objectclass=person)(objectclass=groupofnames)(mail=*))(|(cn=*Ägypten MDK*)(sn=*Ägypten MDK*))";

  connect( mClient, TQ_SIGNAL( result( const TDEABC::LdapObject& ) ),
           this, TQ_SLOT( slotLDAPResult( const TDEABC::LdapObject& ) ) );
  connect( mClient, TQ_SIGNAL( done() ),
           this, TQ_SLOT( slotLDAPDone() ) );
  connect( mClient, TQ_SIGNAL( error( const TQString& ) ),
           this, TQ_SLOT( slotLDAPError( const TQString& ) ) );
  mClient->startQuery( filter );
  kapp->eventLoop()->enterLoop();
  delete mClient; mClient = 0;
}

// from kaddressbook... ugly though...
static TQString asUtf8( const TQByteArray &val )
{
  if ( val.isEmpty() )
    return TQString::null;

  const char *data = val.data();

  //TQString::fromUtf8() bug workaround
  if ( data[ val.size() - 1 ] == '\0' )
    return TQString::fromUtf8( data, val.size() - 1 );
  else
    return TQString::fromUtf8( data, val.size() );
}

static TQString join( const TDEABC::LdapAttrValue& lst, const TQString& sep )
{
  TQString res;
  bool already = false;
  for ( TDEABC::LdapAttrValue::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( already )
      res += sep;
    already = TRUE;
    res += asUtf8( *it );
  }
  return res;
}

void TestLDAPClient::slotLDAPResult( const TDEABC::LdapObject& obj )
{
  TQString cn = join( obj.attrs[ "cn" ], ", " );
  kdDebug() << " cn:" << cn << endl;
  assert( !obj.attrs[ "mail" ].isEmpty() );
  TQString mail = join( obj.attrs[ "mail" ], ", " );
  kdDebug() << " mail:" << mail << endl;
  assert( mail.contains( '@' ) );
}

void TestLDAPClient::slotLDAPError( const TQString& err )
{
  kdDebug() << k_funcinfo << err << endl;
  ::exit( 1 );
}

void TestLDAPClient::slotLDAPDone()
{
  kdDebug() << k_funcinfo << endl;
  kapp->eventLoop()->exitLoop();
}

#include "testldapclient.moc"

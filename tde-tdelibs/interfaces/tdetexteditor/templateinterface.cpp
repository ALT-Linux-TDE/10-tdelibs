/* This file is part of the KDE libraries
  Copyright (C) 2004 Joseph Wenninger <jowenn@kde.org>

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

#include "templateinterface.h"
#include "document.h"
#include <stdaddressbook.h>
#include <addressee.h>
#include <addresseedialog.h>
#include <tqstring.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <tqdatetime.h>
#include <tqregexp.h>
#include <tdemessagebox.h>
#include <kcalendarsystem.h>
#include <unistd.h>

#include <kdebug.h>

using namespace KTextEditor;

unsigned int TemplateInterface::globalTemplateInterfaceNumber = 0;

TemplateInterface::TemplateInterface()
{
  myTemplateInterfaceNumber = globalTemplateInterfaceNumber++;
}

TemplateInterface::~TemplateInterface()
{}

uint TemplateInterface::templateInterfaceNumber () const
{
  return myTemplateInterfaceNumber;
}

void TemplateInterface::setTemplateInterfaceDCOPSuffix ( const TQCString &suffix )
{}

#define INITKABC do { \
  if (addrBook==0) { \
    addrBook=TDEABC::StdAddressBook::self(); \
    userAddress=addrBook->whoAmI(); \
    if (userAddress.isEmpty()) { \
      if ( KMessageBox::questionYesNo(parentWindow, \
           i18n( "This template uses personal data that is stored in the TDE addressbook, but you have not selected a personal entry. You can still use the template without one, but you will have to type personal data. Would you like to select one now?" ), \
           "Personal data requested", \
           KStdGuiItem::yes(), KStdGuiItem::no(), "select personal data entry") == KMessageBox::Yes ) { \
        userAddress = TDEABC::AddresseeDialog::getAddressee(parentWindow); \
        if ( ! userAddress.isEmpty() ) \
          TDEABC::StdAddressBook::self()->setWhoAmI( userAddress ); \
      }\
      /*return false;//no, why??*/ \
    } \
  } \
} while(false)

bool TemplateInterface::expandMacros( TQMap<TQString, TQString> &map, TQWidget *parentWindow )
{
  TDEABC::StdAddressBook *addrBook = 0;
  TDEABC::Addressee userAddress;
  TQDateTime datetime = TQDateTime::currentDateTime();
  TQDate date = datetime.date();
  TQTime time = datetime.time();

  TQMap<TQString,TQString>::Iterator it;
  for ( it = map.begin(); it != map.end(); ++it )
  {
    TQString placeholder = it.key();
    if ( map[ placeholder ].isEmpty() )
    {
      if ( placeholder == "index" ) map[ placeholder ] = "i";
      else if ( placeholder == "loginname" )
      {}
      else if ( placeholder == "firstname" )
      {
        INITKABC;
        if ( !userAddress.isEmpty() )
          map[ placeholder ] = userAddress.givenName();
      }
      else if ( placeholder == "lastname" )
      {
        INITKABC;
        if ( !userAddress.isEmpty() )
          map[ placeholder ] = userAddress.familyName();
      }
      else if ( placeholder == "fullname" )
      {
        INITKABC;
        if ( !userAddress.isEmpty() )
          map[ placeholder ] = userAddress.assembledName();
      }
      else if ( placeholder == "email" )
      {
        INITKABC;
        if ( !userAddress.isEmpty() )
          map[ placeholder ] = userAddress.preferredEmail();
      }
      else if ( placeholder == "date" )
      {
        map[ placeholder ] = TDEGlobal::locale() ->formatDate( date, true );
      }
      else if ( placeholder == "time" )
      {
        map[ placeholder ] = TDEGlobal::locale() ->formatTime( time, true, false );
      }
      else if ( placeholder == "year" )
      {
        map[ placeholder ] = TDEGlobal::locale() ->calendar() ->yearString( date, false );
      }
      else if ( placeholder == "month" )
      {
        map[ placeholder ] = TQString::number( TDEGlobal::locale() ->calendar() ->month( date ) );
      }
      else if ( placeholder == "day" )
      {
        map[ placeholder ] = TQString::number( TDEGlobal::locale() ->calendar() ->day( date ) );
      }
      else if ( placeholder == "hostname" )
      {
        char hostname[ 256 ];
        hostname[ 0 ] = 0;
        gethostname( hostname, 255 );
        hostname[ 255 ] = 0;
        map[ placeholder ] = TQString::fromLocal8Bit( hostname );
      }
      else if ( placeholder == "cursor" )
      {
        map[ placeholder ] = "|";
      }
      else map[ placeholder ] = placeholder;
    }
  }
  return true;
}

bool TemplateInterface::insertTemplateText ( uint line, uint column, const TQString &templateString, const TQMap<TQString, TQString> &initialValues, TQWidget *parentWindow )
{
  TQMap<TQString, TQString> enhancedInitValues( initialValues );

  TQRegExp rx( "[$%]\\{([^}\\s]+)\\}" );
  rx.setMinimal( true );
  int pos = 0;
  int opos = 0;

  while ( pos >= 0 )
  {
    pos = rx.search( templateString, pos );

    if ( pos > -1 )
    {
      if ( ( pos - opos ) > 0 )
      {
        if ( templateString[ pos - 1 ] == '\\' )
        {
          pos = opos = pos + 1;
          continue;
        }
      }
      TQString placeholder = rx.cap( 1 );
      if ( ! enhancedInitValues.contains( placeholder ) )
        enhancedInitValues[ placeholder ] = "";

      pos += rx.matchedLength();
      opos = pos;
    }
  }

  return expandMacros( enhancedInitValues, parentWindow )
         && insertTemplateTextImplementation( line, column, templateString, enhancedInitValues, parentWindow );
}



TemplateInterface *KTextEditor::templateInterface ( KTextEditor::Document *doc )
{
  if ( !doc )
    return 0;

  return dynamic_cast<KTextEditor::TemplateInterface*>( doc );
}


/*
 *  Copyright (C) 2002, 2003 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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

#include <tdecmdlineargs.h>
#include <ktrader.h>
#include <kmimetype.h>
#include <tdeapplication.h>
#include <stdio.h>

static TDECmdLineOptions options[] =
{
  { "+query", "the query", 0 },
  { "+[genericServiceType]", "Application (default), or KParts/ReadOnlyPart", 0 },
  { "+[constraint]", "constraint", 0 },
  { "+[preference]", "preference", 0 },
  TDECmdLineLastOption
};

int main( int argc, char **argv )
{
  TDECmdLineArgs::init( argc, argv, "tdetradertest", "TDETradertest", "A TDETrader testing tool", "0.0" );

  TDECmdLineArgs::addCmdLineOptions( options );

  TDEApplication app( false, false ); // no GUI

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

  if ( args->count() < 1 )
      TDECmdLineArgs::usage();

  TQString query = TQString::fromLocal8Bit( args->arg( 0 ) );

  TQString genericServiceType, constraint, preference;

  if ( args->count() >= 2 )
    genericServiceType = TQString::fromLocal8Bit( args->arg( 1 ) );

  if ( args->count() >= 3 )
    constraint = TQString::fromLocal8Bit( args->arg( 2 ) );

  if ( args->count() == 4 )
    preference = TQString::fromLocal8Bit( args->arg( 3 ) );

  printf( "query is : %s\n", query.local8Bit().data() );
  printf( "genericServiceType is : %s\n", genericServiceType.local8Bit().data() );
  printf( "constraint is : %s\n", constraint.local8Bit().data() );
  printf( "preference is : %s\n", preference.local8Bit().data() );

  TDETrader::OfferList offers = TDETrader::self()->query( query, genericServiceType, constraint, preference );

  printf("got %d offers.\n", offers.count());

  int i = 0;
  TDETrader::OfferList::ConstIterator it = offers.begin();
  TDETrader::OfferList::ConstIterator end = offers.end();
  for (; it != end; ++it, ++i )
  {
    printf("---- Offer %d ----\n", i);
    TQStringList props = (*it)->propertyNames();
    TQStringList::ConstIterator propIt = props.begin();
    TQStringList::ConstIterator propEnd = props.end();
    for (; propIt != propEnd; ++propIt )
    {
      TQVariant prop = (*it)->property( *propIt );

      if ( !prop.isValid() )
      {
        printf("Invalid property %s\n", (*propIt).local8Bit().data());
	continue;
      }

      TQString outp = *propIt;
      outp += " : '";

      switch ( prop.type() )
      {
        case TQVariant::StringList:
          outp += prop.toStringList().join(" - ");
        break;
        case TQVariant::Bool:
          outp += prop.toBool() ? "TRUE" : "FALSE";
          break;
        default:
          outp += prop.toString();
        break;
      }

      if ( !outp.isEmpty() )
        printf("%s'\n", outp.local8Bit().data());
    }
  }
}

/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2003 Michael Goffioul <tdeprint@swing.be>
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

#include "foomatic2loader.h"
#include "driver.h"

#include <tqfile.h>
#include <tqregexp.h>
#include <tqbuffer.h>
#include <kdebug.h>
#include <tdelocale.h>

void tdeprint_foomatic2scanner_init( TQIODevice* );
void tdeprint_foomatic2scanner_terminate();

Foomatic2Loader::Foomatic2Loader()
{
}

Foomatic2Loader::~Foomatic2Loader()
{
}

bool Foomatic2Loader::read( TQIODevice *d )
{
	bool result = true;
	m_foodata.clear();
	tdeprint_foomatic2scanner_init( d );
	if ( tdeprint_foomatic2parse( this ) != 0 )
		result = false;
	tdeprint_foomatic2scanner_terminate();
	return result;
}

bool Foomatic2Loader::readFromFile( const TQString& filename )
{
	TQFile f( filename );
	m_foodata.clear();
	if ( f.open( IO_ReadOnly ) )
		return read( &f );
	return false;
}

bool Foomatic2Loader::readFromBuffer( const TQString& buffer )
{
	TQCString buf = buffer.utf8();
	TQBuffer d( buf );
	m_foodata.clear();
	if ( d.open( IO_ReadOnly ) )
		return read( &d );
	return false;
}

DrBase* Foomatic2Loader::createValue( const TQString& name, const TQStringVariantMap& m ) const
{
	DrBase *choice = new DrBase;
	choice->setName( name );
	choice->set( "text", m.operator[]( "comment" ).toString() );
	return choice;
}

DrBase* Foomatic2Loader::createOption( const TQStringVariantMap& m ) const
{
	TQString type = m.operator[]( "type" ).toString();
	DrBase *opt = NULL;
	if ( type == "enum" )
	{
		DrListOption *lopt = new DrListOption;
		TQVariant a = m.operator[]( "vals_byname" );
		TQStringVariantMap::ConstIterator it = a.mapBegin();
		for ( ; it!=a.mapEnd(); ++it )
		{
			if ( it.data().type() != TQVariant::Map )
				continue;
			DrBase *ch = createValue( it.key(), it.data().toMap() );
			if ( ch )
				lopt->addChoice( ch );
		}
		opt = lopt;
	}
	else if ( type == "int" || type == "float" )
	{
		if ( type == "int" )
			opt = new DrIntegerOption;
		else
			opt = new DrFloatOption;
		opt->set( "minval", m.operator[]( "min" ).toString() );
		opt->set( "maxval", m.operator[]( "max" ).toString() );
	}
	else if ( type == "bool" )
	{
		DrBooleanOption *bopt = new DrBooleanOption;
		DrBase *choice;
		// choice 1
		choice = new DrBase;
		choice->setName( "0" );
		choice->set( "text", m.operator[]( "name_false" ).toString() );
		bopt->addChoice( choice );
		choice = new DrBase;
		choice->setName( "1" );
		choice->set( "text", m.operator[]( "name_true" ).toString() );
		bopt->addChoice( choice );
		opt = bopt;
	}
	else if ( type == "string" )
	{
		opt = new DrStringOption;
	}
	if ( opt )
	{
		opt->setName( m.operator[]( "name" ).toString() );
		opt->set( "text", m.operator[]( "comment" ).toString() );
		TQString defval = m.operator[]( "default" ).toString();
		if ( !defval.isEmpty() )
		{
			opt->setValueText( defval );
			opt->set( "default", defval );
		}
	}
	return opt;
}

DrMain* Foomatic2Loader::buildDriver() const
{
	if ( m_foodata.isEmpty() )
		return NULL;

	TQVariant v = m_foodata.find( "VAR" ).data();
	if ( !v.isNull() && v.type() == TQVariant::Map )
	{
		DrMain *driver = new DrMain;
		TQMap<TQString,DrGroup*> groups;
		driver->set( "manufacturer", v.mapFind( "make" ).data().toString() );
		driver->set( "model", v.mapFind( "model" ).data().toString() );
		driver->set( "matic_printer", v.mapFind( "id" ).data().toString() );
		driver->set( "matic_driver", v.mapFind( "driver" ).data().toString() );
		driver->set( "text", TQString( "%1 %2 (%3)" ).arg( driver->get( "manufacturer" ) ).arg( driver->get( "model" ) ).arg( driver->get( "matic_driver" ) ) );
		if ( m_foodata.contains( "POSTPIPE" ) )
			driver->set( "postpipe", m_foodata.find( "POSTPIPE" ).data().toString() );
		v = v.mapFind( "args" ).data();
		if ( !v.isNull() && v.type() == TQVariant::List )
		{
			TQValueList<TQVariant>::ConstIterator it = v.listBegin();
			for ( ; it!=v.listEnd(); ++it )
			{
				if ( ( *it ).type() != TQVariant::Map )
					continue;
				DrBase *opt = createOption( ( *it ).toMap() );
				if ( opt )
				{
					TQString group = DrGroup::groupForOption( opt->name() );
					DrGroup *grp = NULL;
					if ( !groups.contains( group ) )
					{
						grp = new DrGroup;
						grp->set( "text", group );
						driver->addGroup( grp );
						groups.insert( group, grp );
					}
					else
						grp = groups[ group ];
					grp->addOption( opt );
					if ( opt->name() == "PageSize" )
					{
						// try to add the corresponding page sizes
						TQVariant choices = ( *it ).mapFind( "vals_byname" ).data();
						TQRegExp re( "(\\d+) +(\\d+)" );
						if ( choices.type() == TQVariant::Map )
						{
							TQStringVariantMap::ConstIterator it = choices.mapBegin();
							for ( ; it!=choices.mapEnd(); ++it )
							{
								TQString driverval = ( *it ).mapFind( "driverval" ).data().toString();
								if ( re.exactMatch( driverval ) )
								{
									driver->addPageSize( new DrPageSize( it.key(), re.cap( 1 ).toInt(), re.cap( 2 ).toInt(), 36, 24, 36, 24 ) );
								}
							}
						}
					}
				}
				else
					kdWarning( 500 ) << "Failed to create option: " << ( *it ).toMap()[ "name" ].toString() << endl;
			}
		}
		return driver;
	}
	return NULL;
}

DrMain* Foomatic2Loader::modifyDriver( DrMain *driver ) const
{
	if ( !m_foodata.isEmpty() )
	{
		TQValueList<DrBase*> optList;
		DrGroup *grp = NULL;

		TQVariant V = m_foodata.find( "VAR" ).data();
		if ( !V.isNull() && V.type() == TQVariant::Map )
		{
			TQVariant v = V.mapFind( "args" ).data();
			if ( !v.isNull() && v.type() == TQVariant::List )
			{
				TQValueList<TQVariant>::ConstIterator it = v.listBegin();
				for ( ; it!=v.listEnd(); ++it )
				{
					if ( ( *it ).type() != TQVariant::Map )
						continue;
					DrBase *opt = createOption( ( *it ).toMap() );
					if ( opt )
						optList.append( opt );
					else
						kdWarning( 500 ) << "Failed to create option: " << ( *it ).toMap()[ "name" ].toString() << endl;
				}
			}
			else
			{
				v = V.mapFind( "args_byname" ).data();
				if ( !v.isNull() && v.type() == TQVariant::Map )
				{
					TQStringVariantMap::ConstIterator it = v.mapBegin();
					for ( ; it!=v.mapEnd(); ++it )
					{
						if ( ( *it ).type() != TQVariant::Map )
							continue;
						DrBase *opt = createOption( ( *it ).toMap() );
						if ( opt )
							optList.append( opt );
						else
							kdWarning( 500 ) << "Failed to create option: " << ( *it ).toMap()[ "name" ].toString() << endl;
					}
				}
			}
		}

		for ( TQValueList<DrBase*>::ConstIterator it=optList.begin(); it!=optList.end(); ++it )
		{
			DrBase *opt = ( *it );
			if ( opt )
			{
				switch ( opt->type() )
				{
					case DrBase::List:
					case DrBase::Boolean:
						delete opt;
						break;
					default:
						{
							if ( !grp )
							{
								grp = new DrGroup;
								grp->set( "text", i18n( "Adjustments" ) );
								driver->addGroup( grp );
							}
							DrBase *oldOpt = driver->findOption( opt->name() );
							if ( oldOpt && oldOpt->type() == DrBase::List )
							{
								TQPtrListIterator<DrBase> it( *( static_cast<DrListOption*>( oldOpt )->choices() ) );
								TQString fixedvals;
								for ( ; it.current(); ++it )
								{
									fixedvals.append( it.current()->name() );
									if ( !it.atLast() )
										fixedvals.append( "|" );
								}
								opt->set( "fixedvals", fixedvals );
							}
							driver->removeOptionGlobally( opt->name() );
							grp->addOption( opt );
							break;
						}
				}
			}
		}
	}
	return driver;
}

DrMain* Foomatic2Loader::loadDriver( const TQString& filename )
{
	Foomatic2Loader loader;
	if ( loader.readFromFile( filename ) )
		return loader.buildDriver();
	else
		return NULL;
}

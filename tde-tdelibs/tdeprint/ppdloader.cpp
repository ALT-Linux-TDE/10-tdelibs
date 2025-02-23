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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE   /* Needed for getline */
#endif

#include "ppdloader.h"
#include "foomatic2loader.h"
#include "driver.h"

#include <kfilterdev.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdetempfile.h>
#include <tqfile.h>
#include <math.h>
#include <stdlib.h>

void tdeprint_ppdscanner_init( TQIODevice* );
void tdeprint_ppdscanner_terminate( bool deleteIt = true );
int tdeprint_ppdscanner_numberoflines();

static TQString processLocaleString( const TQString& s )
{
	TQString res;
	uint pos = 0;
	while ( pos < s.length() )
	{
		TQChar c = s[ pos++ ];
		if ( c == '<' )
		{
			bool flag = false;
			uint hc = 0;
			while ( pos < s.length() )
			{
				TQChar cc = s[ pos++ ];
				uint _hc = 0;
				if ( cc == '>' )
					break;
				else if ( cc.isDigit() )
					_hc = cc.digitValue();
				else
					_hc = cc.lower().latin1() - 'a' + 10;
				if ( flag )
				{
					hc |= _hc;
					res.append( TQChar( hc ) );
					hc = 0;
				}
				else
					hc = ( _hc << 4 );
				flag = !flag;
			}
		}
		else
		{
			res.append( c );
		}
	}
	return res;
}

static TQValueList<float> splitNumberString( const TQString& _s )
{
        TQString s = _s.simplifyWhiteSpace();
	TQValueList<float> l;
	int p1 = 1, p2 = 0;
	while ( true )
	{
		p2 = s.find( ' ', p1 );
		if ( p2 != -1 )
		{
			l.append( s.mid( p1, p2-p1 ).toFloat() );
			p1 = p2+1;
		}
		else
		{
			// ignore the final quote
			l.append( s.mid( p1, s.length() - p1 - 1 ).toFloat() );
			break;
		}
	}
	return l;
}

struct PS_private
{
	TQString name;
	struct
	{
		float width, height;
	} size;
	struct
	{
		float left, bottom, right, top;
	} area;
};

PPDLoader::PPDLoader()
{
	m_option = 0;
	m_ps.setAutoDelete( true );
}

PPDLoader::~PPDLoader()
{
}

DrMain* PPDLoader::readFromFile( const TQString& filename )
{
	bool ppdFilenameIsTempFile = false;
	TQString ppdFilename = filename;

	if (filename.startsWith("compressed-ppd:")) {
		KTempFile tempFile(TQString::null, "ppd", 0600);
		tempFile.setAutoDelete(false);
		ppdFilename = tempFile.name();

		TQStringList filenameParts = TQStringList::split(":", filename);
		TQString databaseFilename = TQString::null;
		TQString compressedFilename = TQString::null;
		int i = 0;
		for (TQStringList::Iterator it = filenameParts.begin(); it != filenameParts.end(); ++it) {
			if (i == 1) {
				databaseFilename = *it;
			}
			else if (i > 1) {
				compressedFilename += *it;
			}
			i++;
		}

		TQString command = databaseFilename + " cat " + compressedFilename;

		FILE* file = popen(command.ascii(), "r");
		if (file) {
			char * line = NULL;
			size_t len = 0;
			ssize_t read;

			FILE* tmpFileStream = tempFile.fstream();

			while ((read = getline(&line, &len, file)) != -1) {
				fputs(line, tmpFileStream);
			}
			if (line) {
				free(line);
			}

			tempFile.close();
			pclose(file);
		}
		else {
			fprintf(stderr, "Can't open driver file : %s\n", compressedFilename.ascii());
			return 0;
		}

		ppdFilenameIsTempFile = true;
	}

	// Initialization
	m_groups.clear();
	m_option = NULL;
	m_fonts.clear();
	// Open driver file
	TQIODevice *d = KFilterDev::deviceForFile( ppdFilename );
	if ( d && d->open( IO_ReadOnly ) )
	{
		DrMain *driver = new DrMain;
		bool result = true;

		m_groups.push( driver );
		tdeprint_ppdscanner_init( d );
		if ( tdeprint_ppdparse( this ) != 0 )
			result = false;
		tdeprint_ppdscanner_terminate( true );

		if ( result )
		{
			if ( m_groups.size() > 1 )
				kdWarning( 500 ) << "PPD syntax error, GROUP specification not correctly closed" << endl;
			if ( driver->has( "foodata" ) )
			{
				Foomatic2Loader loader;
				if ( loader.readFromBuffer( driver->get( "foodata" ) ) )
				{
					driver = loader.modifyDriver( driver );
				}
				else
					kdWarning( 500 ) << "PPD syntax error, Foomatic data read failed" << endl;
			}
			processPageSizes( driver );
			if ( !m_fonts.isEmpty() )
				driver->set( "fonts", m_fonts.join( "," ) );
			if (ppdFilenameIsTempFile) {
				driver->set("temporary-cppd", ppdFilename);
			}
			return driver;
		}
		else
			kdWarning( 500 ) << "PPD syntax error, PPD parse failed" << endl;
		delete driver;
		m_ps.clear();
	}
	else
		kdWarning( 500 ) << "PPD read error, unable to open device for file " << ppdFilename << endl;
	return 0;
}

DrMain* PPDLoader::loadDriver( const TQString& filename, TQString* msg )
{
	PPDLoader loader;
	DrMain *driver = loader.readFromFile( filename );
	if ( !driver && msg )
		*msg = filename + i18n( "(line %1): " ).arg( tdeprint_ppdscanner_numberoflines() ) + loader.errorMsg();
	return driver;
}

bool PPDLoader::openUi( const TQString& name, const TQString& desc, const TQString& type )
{
	if ( m_option )
	{
		tqWarning( "PPD syntax error, UI specification not correctly closed" );
		endUi( m_option->name() );
	}

	if ( type == "PickOne" || type == "PickMany" )
		m_option = new DrListOption;
	else if ( type == "Boolean" )
		m_option = new DrBooleanOption;
	else
		return false;
	if ( name[ 0 ] == '*' )
		m_option->setName( name.mid( 1 ) );
	else
		m_option->setName( name );
	if ( desc.isEmpty() )
		m_option->set( "text", m_option->name() );
	else
		m_option->set( "text", processLocaleString( desc ) );
	return true;
}

bool PPDLoader::endUi( const TQString& name )
{
	if ( m_option && ( m_option->name() == name || m_option->name() == name.mid( 1 ) ) )
	{
		if ( m_option->name() == "PageRegion" )
			delete m_option;
		else
		{
			TQString defval = m_option->get( "default" );
			DrGroup *grp = 0;
			if ( !defval.isEmpty() )
				m_option->setValueText( defval );
			if ( m_groups.size() == 1 )
			{
				// we don't have any group defined, create the
				// most adapted one.
				grp = findOrCreateGroupForOption( m_option->name() );
			}
			else
				grp = m_groups.top();
			grp->addOption( m_option );
			if ( grp->get( "text" ).contains( "install", false ) )
				m_option->set( "fixed", "1" );
		}
		m_option = 0;
		return true;
	}
	return false;
}

bool PPDLoader::openGroup( const TQString& name, const TQString& desc )
{
	DrGroup *grp = new DrGroup;
	grp->setName( name );
	if ( desc.isEmpty() )
		grp->set( "text", name );
	else
		grp->set( "text", processLocaleString( desc ) );
	m_groups.top()->addGroup( grp );
	m_groups.push( grp );
	return true;
}

bool PPDLoader::endGroup( const TQString& name )
{
	if ( m_groups.size() > 1 && m_groups.top()->name() == name )
	{
		m_groups.pop();
		return true;
	}
	return false;
}

bool PPDLoader::putStatement( const TQString& keyword, const TQString& name, const TQString& desc, const TQStringList& values )
{
	if ( m_option )
	{
		if ( !name.isEmpty() && m_option->name() == keyword )
		{
			if ( m_option->type() >= DrBase::List )
			{
				DrBase *ch = new DrBase;
				ch->setName( name );
				if ( desc.isEmpty() )
					ch->set( "text", name );
				else
					ch->set( "text", processLocaleString( desc ) );
				static_cast<DrListOption*>( m_option )->addChoice( ch );
			}
			else
			{
				TQString fv = m_option->get( "fixedvals" );
				if ( fv.isEmpty() )
					fv = name;
				else
					fv.append( "|" + name );
				m_option->set( "fixedvals", fv );
			}
		}
		else if ( keyword == "FoomaticRIPOption" && name == m_option->name()
				&& values.size() > 1 )
		{
			TQString type = values[ 0 ];
			if ( type == "float" || type == "int" )
			{
				DrBase *opt = 0;
				if ( type == "float" )
					opt = new DrFloatOption;
				else
					opt = new DrIntegerOption;
				opt->setName( m_option->name() );
				opt->set( "text", m_option->get( "text" ) );
				opt->set( "default", m_option->get( "default" ) );
				if ( m_option->type() == DrBase::List )
				{
					TQStringList vals;
					TQPtrListIterator<DrBase> it( *( static_cast<DrListOption*>( m_option )->choices() ) );
					for ( ; it.current(); ++it )
						vals.append( it.current()->name() );
					opt->set( "fixedvals", vals.join( "|" ) );
				}
				delete m_option;
				m_option = opt;
			}
			// FIXME: support other option types
		}
		else if ( keyword == "FoomaticRIPOptionRange" && name == m_option->name()
				&& values.size() >= 2 && ( m_option->type() == DrBase::Float || m_option->type() == DrBase::Integer ) )
		{
			m_option->set( "minval", values[ 0 ] );
			m_option->set( "maxval", values[ 1 ] );
		}
	}
	else if ( keyword == "Font" && m_groups.size() > 0 )
	{
		m_fonts << name;
	}
	return true;
}

bool PPDLoader::putStatement2( const TQString& keyword, const TQString& value )
{
	if ( !m_option && m_groups.size() == 1 )
	{
		DrGroup *driver = m_groups.top();
		if ( keyword == "NickName" )
		{
			driver->set( "text", value );
			driver->set( "description", value );
		}
		else if ( keyword == "Manufacturer" )
			driver->set( "manufacturer", value );
		else if ( keyword == "ShortNickName" )
			driver->set( "model", value );
		else if ( keyword == "ColorDevice" )
			driver->set( "colordevice", value == "True" ? "1" : "0" );
	}
	return true;
}

bool PPDLoader::putDefault( const TQString& keyword, const TQString& value )
{
	if ( keyword == "Resolution" && m_groups.size() > 0 )
	{
		// Store default resolution as it could be fed back
		// to the application. And default resolution can
		// occur outside a OpenUI/CloseUI pair.
		m_groups[ 0 ]->set( "resolution", value );
	}

	if ( m_option && m_option->name() == keyword )
	{
		m_option->set( "default", value );
		return true;
	}
	else
		return false;
}

bool PPDLoader::putConstraint( const TQString& opt1, const TQString& opt2, const TQString& ch1, const TQString& ch2 )
{
	if ( !m_option && m_groups.size() == 1 )
	{
		DrMain *driver = static_cast<DrMain*>( m_groups.top() );
		driver->addConstraint( new DrConstraint( opt1, opt2, ch1, ch2 ) );
	}
	return true;
}

bool PPDLoader::putFooData( const TQString& data )
{
	if ( !m_option && m_groups.size() == 1 )
	{
		m_groups.top()->set( "foodata", m_groups.top()->get( "foodata" ) + data + "\n" );
	}
	return true;
}

bool PPDLoader::putFooProcessedData( const TQVariant& var )
{
	TQStringVariantMap::ConstIterator it = var.mapFind( "args_byname" );
	if ( it != var.mapEnd() )
	{
		TQVariant opts = it.data();
		for ( it = opts.mapBegin(); it != opts.mapEnd(); ++it )
		{
			TQStringVariantMap opt = it.data().toMap();
			TQString type = opt[ "type" ].toString();
			if ( type == "float" || type == "int" )
			{
				DrBase *o;
				if ( type == "float" )
					o = new DrFloatOption;
				else
					o = new DrIntegerOption;
				o->setName( opt[ "name" ].toString() );
				o->set( "text", opt[ "comment" ].toString() );
				o->set( "minval", opt[ "min" ].toString() );
				o->set( "maxval", opt[ "max" ].toString() );
				o->set( "default", opt[ "default" ].toString() );
				o->setValueText( o->get( "default" ) );

				DrGroup *grp = 0;
				DrBase *old = m_groups.top()->findOption( o->name(), &grp );
				if ( old )
				{
					if ( old->type() == DrBase::List )
					{
						TQStringList vals;
						TQPtrListIterator<DrBase> it( *( static_cast<DrListOption*>( old )->choices() ) );
						for ( ; it.current(); ++it )
							vals.append( it.current()->name() );
						o->set( "fixedvals", vals.join( "|" ) );
					}
					grp->removeOption( o->name() );
					grp->addOption( o );
				}
				else
				{
					tqWarning( "Option %s not found in original PPD file", o->name().latin1() );
					delete o;
				}
			}
		}
	}
	return true;
}

bool PPDLoader::putPaperDimension( const TQString& name, const TQString& s )
{
	TQValueList<float> l = splitNumberString( s );

	PS_private *ps = m_ps.find( name );
	if ( !ps )
	{
		ps = new PS_private;
		ps->name = name;
		m_ps.insert( name, ps );
	}
	ps->size.width = l[ 0 ];
	ps->size.height = l[ 1 ];

	return true;
}

bool PPDLoader::putImageableArea( const TQString& name, const TQString& s )
{
	TQValueList<float> l = splitNumberString( s );

	PS_private *ps = m_ps.find( name );
	if ( !ps )
	{
		ps = new PS_private;
		ps->name = name;
		m_ps.insert( name, ps );
	}
	ps->area.left = l[ 0 ];
	ps->area.bottom = l[ 1 ];
	ps->area.right = l[ 2 ];
	ps->area.top = l[ 3 ];

	return true;
}

DrGroup* PPDLoader::findOrCreateGroupForOption( const TQString& optname )
{
	TQString grpname;
	if ( optname == "PageSize" ||
			optname == "InputSlot" ||
			optname == "ManualFeed" ||
			optname == "MediaType" ||
			optname == "MediaColor" ||
			optname == "MediaWeight" )
		grpname = "General";
	else if ( optname.startsWith( "stp" ) ||
			optname == "Cyan" ||
			optname == "Yellow" ||
			optname == "Magenta" ||
			optname == "Density" ||
			optname == "Contrast" )
		grpname = "Adjustments";
	else if ( optname.startsWith( "JCL" ) )
		grpname = "JCL";
	else
		grpname = "Others";

	DrGroup *grp = 0;
	for ( TQPtrListIterator<DrGroup> it( m_groups[ 0 ]->groups() ); it.current(); ++it )
		if ( it.current()->name() == grpname )
		{
			grp = it.current();
			break;
		}
	if ( !grp )
	{
		grp = new DrGroup;
		grp->setName( grpname );
		grp->set( "text", grpname );
		m_groups[ 0 ]->addGroup( grp );
	}
	return grp;
}

void PPDLoader::processPageSizes( DrMain *driver )
{
	TQDictIterator<PS_private> it( m_ps );
	for ( ; it.current(); ++it )
	{
		//tqDebug( "ADDING PAGESIZE: %16s, Size = ( %.2f, %.2f ),  Area = ( %.2f, %.2f, %.2f, %.2f )", it.current()->name.latin1(),
		//		it.current()->size.width, it.current()->size.height,
		//		it.current()->area.left, it.current()->area.bottom,
		//		it.current()->area.right, it.current()->area.top );
		driver->addPageSize( new DrPageSize( it.current()->name,
					( int )it.current()->size.width, ( int )it.current()->size.height,
					( int )it.current()->area.left, ( int )it.current()->area.bottom,
					( int )ceil( it.current()->size.width - it.current()->area.right ),
					( int )ceil( it.current()->size.height - it.current()->area.top ) ) );
	}
	m_ps.clear();
}

void PPDLoader::setErrorMsg( const TQString& msg )
{
	m_errormsg = msg;
}

TQString PPDLoader::errorMsg() const
{
	return m_errormsg;
}

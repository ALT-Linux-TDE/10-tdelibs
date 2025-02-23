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

#ifndef PPDLOADER_H
#define PPDLOADER_H

#include <tqvaluestack.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqvariant.h>
#include <tqdict.h>

#include <tdelibs_export.h>

class DrGroup;
class DrBase;
class DrMain;
struct PS_private;

class TDEPRINT_EXPORT PPDLoader
{
public:
	PPDLoader();
	~PPDLoader();

	DrMain* readFromFile( const TQString& filename );

	bool openUi( const TQString& name, const TQString& desc, const TQString& type );
	bool endUi( const TQString& name );
	bool openGroup( const TQString& name, const TQString& desc );
	bool endGroup( const TQString& name );
	bool putStatement( const TQString& keyword, const TQString& name, const TQString& desc, const TQStringList& values );
	bool putStatement2( const TQString& keyword, const TQString& value );
	bool putDefault( const TQString& keyword, const TQString& value );
	bool putConstraint( const TQString& opt1, const TQString& opt2, const TQString& ch1, const TQString& ch2 );
	bool putFooData( const TQString& data );
	bool putFooProcessedData( const TQVariant& var );
	bool putPaperDimension( const TQString& name, const TQString& s );
	bool putImageableArea( const TQString& name, const TQString& s );

	void setErrorMsg( const TQString& msg );
	TQString errorMsg() const;

	static DrMain* loadDriver( const TQString& filename, TQString* msg = NULL );

private:
	TQValueStack<DrGroup*> m_groups;
	DrBase*               m_option;
	TQDict<PS_private>     m_ps;
	TQStringList           m_fonts;
	TQString               m_errormsg;

	friend int tdeprint_ppdparse(void*);
	DrGroup* findOrCreateGroupForOption( const TQString& );
	void processPageSizes( DrMain* );
};

#endif /* PPDLOADER_H */

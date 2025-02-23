/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2002 Michael Goffioul <tdeprint@swing.be>
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

#include "sizewidget.h"

#include <tqcombobox.h>
#include <tqspinbox.h>
#include <tqlayout.h>
#include <tqregexp.h>
#include <tdelocale.h>

SizeWidget::SizeWidget( TQWidget *parent, const char *name )
	: TQWidget( parent, name )
{
	m_size = new TQSpinBox( 0, 9999, 1, this );
	m_unit = new TQComboBox( this );

	m_unit->insertItem( i18n( "KB" ) );
	m_unit->insertItem( i18n( "MB" ) );
	m_unit->insertItem( i18n( "GB" ) );
	m_unit->insertItem( i18n( "Tiles" ) );
	m_unit->setCurrentItem( 1 );
	m_size->setSpecialValueText( i18n( "Unlimited" ) );

	TQHBoxLayout *l0 = new TQHBoxLayout( this, 0, 5 );
	l0->addWidget( m_size, 1 );
	l0->addWidget( m_unit, 0 );
}

void SizeWidget::setSizeString( const TQString& sz )
{
	int p = sz.find( TQRegExp( "\\D" ) );
	m_size->setValue( sz.left( p ).toInt() );
	switch( sz[ p ].latin1() )
	{
		case 'k': p = 0; break;
		default:
		case 'm': p = 1; break;
		case 'g': p = 2; break;
		case 't': p = 3; break;
	}
	m_unit->setCurrentItem( p );
}

TQString SizeWidget::sizeString() const
{
	TQString result = TQString::number( m_size->value() );
	switch ( m_unit->currentItem() )
	{
		case 0: result.append( "k" ); break;
		case 1: result.append( "m" ); break;
		case 2: result.append( "g" ); break;
		case 3: result.append( "t" ); break;
	}
	return result;
}

void SizeWidget::setValue( int value )
{
	m_size->setValue( value );
	m_unit->setCurrentItem( 1 );
}

int SizeWidget::value() const
{
	return m_size->value();
}

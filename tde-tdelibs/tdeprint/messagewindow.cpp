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

#include "messagewindow.h"

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqpixmap.h>
#include <tqhbox.h>
#include <kiconloader.h>
#include <tdeapplication.h>
#include <kdebug.h>

TQPtrDict<MessageWindow> MessageWindow::m_windows;

MessageWindow::MessageWindow( const TQString& txt, int delay, TQWidget *parent, const char *name )
	: TQWidget( parent, name, (WFlags)(WStyle_Customize|WStyle_NoBorder|WShowModal|WType_Dialog|WDestructiveClose) )
{
	TQHBox *box = new TQHBox( this );
	box->setFrameStyle( TQFrame::Panel|TQFrame::Raised );
	box->setLineWidth( 1 );
	box->setSpacing( 10 );
	box->setMargin( 5 );
	TQLabel *pix = new TQLabel( box );
	pix->setPixmap( DesktopIcon( "tdeprint_printer" ) );
	m_text = new TQLabel( txt, box );

	TQHBoxLayout *l0 = new TQHBoxLayout( this, 0, 0 );
	l0->addWidget( box );

	m_windows.insert( parent, this );

	if ( delay == 0 )
		slotTimer();
	else
		TQTimer::singleShot( delay, this, TQ_SLOT( slotTimer() ) );
}

MessageWindow::~MessageWindow()
{
	m_windows.remove( parentWidget() );
}

void MessageWindow::slotTimer()
{
	TQSize psz = parentWidget()->size(), sz = sizeHint();
	move( parentWidget()->mapToGlobal( TQPoint( (psz.width()-sz.width())/2, (psz.height()-sz.height())/2 ) ) );
	if ( !isVisible() )
	{
		show();
		kapp->processEvents();
	}
}

TQString MessageWindow::text() const
{
	return m_text->text();
}

void MessageWindow::setText( const TQString& txt )
{
	m_text->setText( txt );
}

void MessageWindow::add( TQWidget *parent, const TQString& txt, int delay )
{
	if ( !parent )
		kdWarning( 500 ) << "Cannot add a message window to a null parent" << endl;
	else
	{
		MessageWindow *w = m_windows.find( parent );
		if ( w )
			w->setText( txt );
		else
			new MessageWindow( txt, delay, parent, "MessageWindow" );
	}
}

void MessageWindow::remove( TQWidget *parent )
{
	if ( parent )
		delete m_windows.find( parent );
}

void MessageWindow::change( TQWidget *parent, const TQString& txt )
{
	if ( parent )
	{
		MessageWindow *w = m_windows.find( parent );
		if ( w )
			w->setText( txt );
		else
			kdWarning( 500 ) << "MessageWindow::change, no message window found" << endl;
	}
}

void MessageWindow::removeAll()
{
	TQPtrDictIterator<MessageWindow> it( m_windows );
	while ( it.current() )
		delete it.current();
}

#include "messagewindow.moc"

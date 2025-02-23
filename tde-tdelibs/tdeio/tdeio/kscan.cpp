/* This file is part of the KDE libraries
    Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <tqfile.h>

#include <tdelocale.h>
#include <ktrader.h>

#include "kscan.h"

// static factory method
KScanDialog * KScanDialog::getScanDialog( TQWidget *parent, const char *name,
					  bool modal )
{
    TDETrader::OfferList offers = TDETrader::self()->query("KScan/KScanDialog");
    if ( offers.isEmpty() )
	return 0L;
	
    KService::Ptr ptr = *(offers.begin());
    KLibFactory *factory = KLibLoader::self()->factory( TQFile::encodeName(ptr->library()) );

    if ( !factory )
        return 0;

    TQStringList args;
    args << TQString::number( (int)modal );

    TQObject *res = factory->create( parent, name, "KScanDialog", args );

    return dynamic_cast<KScanDialog *>( res );
}


KScanDialog::KScanDialog( int dialogFace, int buttonMask,
			  TQWidget *parent, const char *name, bool modal )
    : KDialogBase( dialogFace, i18n("Acquire Image"), buttonMask, Close,
		   parent, name, modal, true ),
      m_currentId( 1 )
{
}

KScanDialog::~KScanDialog()
{
}

bool KScanDialog::setup()
{
    return true;
}

///////////////////////////////////////////////////////////////////


// static factory method
KOCRDialog * KOCRDialog::getOCRDialog( TQWidget *parent, const char *name,
					  bool modal )
{
    TDETrader::OfferList offers = TDETrader::self()->query("KScan/KOCRDialog");
    if ( offers.isEmpty() )
	return 0L;
	
    KService::Ptr ptr = *(offers.begin());
    KLibFactory *factory = KLibLoader::self()->factory( TQFile::encodeName(ptr->library()) );

    if ( !factory )
        return 0;

    TQStringList args;
    args << TQString::number( (int)modal );

    TQObject *res = factory->create( parent, name, "KOCRDialog", args );

    return dynamic_cast<KOCRDialog *>( res );
}


KOCRDialog::KOCRDialog( int dialogFace, int buttonMask,
			  TQWidget *parent, const char *name, bool modal )
    : KDialogBase( dialogFace, i18n("OCR Image"), buttonMask, Close,
		   parent, name, modal, true ),
      m_currentId( 1 )
{

}

KOCRDialog::~KOCRDialog()
{
}


///////////////////////////////////////////////////////////////////


KScanDialogFactory::KScanDialogFactory( TQObject *parent, const char *name )
    : KLibFactory( parent, name ),
      m_instance( 0L )
{
}

KScanDialogFactory::~KScanDialogFactory()
{
    delete m_instance;
}

TQObject *KScanDialogFactory::createObject( TQObject *parent, const char *name,
                                           const char *classname,
                                           const TQStringList &args )
{
    if ( strcmp( classname, "KScanDialog" ) != 0 )
        return 0;

    if ( parent && !parent->isWidgetType() )
       return 0;

    bool modal = false;

    if ( args.count() == 1 )
        modal = (bool)args[ 0 ].toInt();

    return createDialog( static_cast<TQWidget*>( parent ), name, modal );
}


///////////////////////////////////////////////////////////////////


KOCRDialogFactory::KOCRDialogFactory( TQObject *parent, const char *name )
    : KLibFactory( parent, name ),
      m_instance( 0L )
{
}

KOCRDialogFactory::~KOCRDialogFactory()
{
    delete m_instance;
}

TQObject *KOCRDialogFactory::createObject( TQObject *parent, const char *name,
                                           const char *classname,
                                           const TQStringList &args )
{
    if ( strcmp( classname, "KOCRDialog" ) != 0 )
        return 0;

    if ( parent && !parent->isWidgetType() )
       return 0;

    bool modal = false;

    if ( args.count() == 1 )
        modal = (bool)args[ 0 ].toInt();

    return createDialog( static_cast<TQWidget*>( parent ), name, modal );
}

void KScanDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KScanDialogFactory::virtual_hook( int id, void* data )
{ KLibFactory::virtual_hook( id, data ); }

void KOCRDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KOCRDialogFactory::virtual_hook( int id, void* data )
{ KLibFactory::virtual_hook( id, data ); }


#include "kscan.moc"

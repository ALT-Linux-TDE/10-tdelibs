/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include "kdatatool.h"

#include <kstandarddirs.h>
#include <klibloader.h>
#include <kdebug.h>
#include <kinstance.h>

#include <ktrader.h>
#include <tdeparts/componentfactory.h>

#include <tqpixmap.h>
#include <tqfile.h>

/*************************************************
 *
 * KDataToolInfo
 *
 *************************************************/

KDataToolInfo::KDataToolInfo()
{
    m_service = 0;
}

KDataToolInfo::KDataToolInfo( const KService::Ptr& service, TDEInstance* instance )
{
    m_service = service;
    m_instance = instance;

    if ( !!m_service && !m_service->serviceTypes().contains( "KDataTool" ) )
    {
        kdDebug(30003) << "The service " << m_service->name().latin1()
                       << " does not feature the service type KDataTool" << endl;
        m_service = 0;
    }
}

KDataToolInfo::KDataToolInfo( const KDataToolInfo& info )
{
    m_service = info.service();
    m_instance = info.instance();
}

KDataToolInfo& KDataToolInfo::operator= ( const KDataToolInfo& info )
{
    m_service = info.service();
    m_instance = info.instance();
    return *this;
}

TQString KDataToolInfo::dataType() const
{
    if ( !m_service )
        return TQString::null;

    return m_service->property( "DataType" ).toString();
}

TQStringList KDataToolInfo::mimeTypes() const
{
    if ( !m_service )
        return TQStringList();

    return m_service->property( "DataMimeTypes" ).toStringList();
}

bool KDataToolInfo::isReadOnly() const
{
    if ( !m_service )
        return true;

    return m_service->property( "ReadOnly" ).toBool();
}

TQPixmap KDataToolInfo::icon() const
{
    if ( !m_service )
        return TQPixmap();

    TQPixmap pix;
    TQStringList lst = TDEGlobal::dirs()->resourceDirs("icon");
    TQStringList::ConstIterator it = lst.begin();
    while (!pix.load( *it + "/" + m_service->icon() ) && it != lst.end() )
        it++;

    return pix;
}

TQPixmap KDataToolInfo::miniIcon() const
{
    if ( !m_service )
        return TQPixmap();

    TQPixmap pix;
    TQStringList lst = TDEGlobal::dirs()->resourceDirs("mini");
    TQStringList::ConstIterator it = lst.begin();
    while (!pix.load( *it + "/" + m_service->icon() ) && it != lst.end() )
        it++;

    return pix;
}

TQString KDataToolInfo::iconName() const
{
    if ( !m_service )
        return TQString::null;
    return m_service->icon();
}

TQStringList KDataToolInfo::commands() const
{
    if ( !m_service )
        return TQString();

    return m_service->property( "Commands" ).toStringList();
}

TQStringList KDataToolInfo::userCommands() const
{
    if ( !m_service )
        return TQString();

    return TQStringList::split( ',', m_service->comment() );
}

KDataTool* KDataToolInfo::createTool( TQObject* parent, const char* name ) const
{
    if ( !m_service )
        return 0;

    KDataTool* tool = KParts::ComponentFactory::createInstanceFromService<KDataTool>( m_service, parent, name );
    if ( tool )
        tool->setInstance( m_instance );
    return tool;
}

KService::Ptr KDataToolInfo::service() const
{
    return m_service;
}

TQValueList<KDataToolInfo> KDataToolInfo::query( const TQString& datatype, const TQString& mimetype, TDEInstance* instance )
{
    TQValueList<KDataToolInfo> lst;

    TQString constr;

    if ( !datatype.isEmpty() )
    {
        constr = TQString::fromLatin1( "DataType == '%1'" ).arg( datatype );
    }
    if ( !mimetype.isEmpty() )
    {
        TQString tmp = TQString::fromLatin1( "'%1' in DataMimeTypes" ).arg( mimetype );
        if ( constr.isEmpty() )
            constr = tmp;
        else
            constr = constr + " and " + tmp;
    }
/* Bug in TDETrader ? Test with HEAD-tdelibs!
    if ( instance )
    {
        TQString tmp = TQString::fromLatin1( "not ('%1' in ExcludeFrom)" ).arg( instance->instanceName() );
        if ( constr.isEmpty() )
            constr = tmp;
        else
            constr = constr + " and " + tmp;
    } */

    // Query the trader
    //kdDebug() << "KDataToolInfo::query " << constr << endl;
    TDETrader::OfferList offers = TDETrader::self()->query( "KDataTool", constr );

    TDETrader::OfferList::ConstIterator it = offers.begin();
    for( ; it != offers.end(); ++it )
    {
        // Temporary replacement for the non-working trader query above
        if ( !instance || !(*it)->property("ExcludeFrom").toStringList()
             .contains( instance->instanceName() ) )
            lst.append( KDataToolInfo( *it, instance ) );
        else
            kdDebug() << (*it)->entryPath() << " excluded." << endl;
    }

    return lst;
}

bool KDataToolInfo::isValid() const
{
    return( m_service );
}

/*************************************************
 *
 * KDataToolAction
 *
 *************************************************/
KDataToolAction::KDataToolAction( const TQString & text, const KDataToolInfo & info, const TQString & command,
                                    TQObject * parent, const char * name )
    : TDEAction( text, info.iconName(), 0, parent, name ),
      m_command( command ),
      m_info( info )
{
}

void KDataToolAction::slotActivated()
{
    emit toolActivated( m_info, m_command );
}

TQPtrList<TDEAction> KDataToolAction::dataToolActionList( const TQValueList<KDataToolInfo> & tools, const TQObject *receiver, const char* slot )
{
    TQPtrList<TDEAction> actionList;
    if ( tools.isEmpty() )
        return actionList;

    actionList.append( new TDEActionSeparator() );
    TQValueList<KDataToolInfo>::ConstIterator entry = tools.begin();
    for( ; entry != tools.end(); ++entry )
    {
        TQStringList userCommands = (*entry).userCommands();
        TQStringList commands = (*entry).commands();
        Q_ASSERT(!commands.isEmpty());
        if ( commands.count() != userCommands.count() )
            kdWarning() << "KDataTool desktop file error (" << (*entry).service()->entryPath()
                        << "). " << commands.count() << " commands and "
                        << userCommands.count() << " descriptions." << endl;
        TQStringList::ConstIterator uit = userCommands.begin();
        TQStringList::ConstIterator cit = commands.begin();
        for (; uit != userCommands.end() && cit != commands.end(); ++uit, ++cit )
        {
            //kdDebug() << "creating action " << *uit << " " << *cit << endl;
            KDataToolAction * action = new KDataToolAction( *uit, *entry, *cit );
            connect( action, TQ_SIGNAL( toolActivated( const KDataToolInfo &, const TQString & ) ),
                     receiver, slot );
            actionList.append( action );
        }
    }

    return actionList;
}

/*************************************************
 *
 * KDataTool
 *
 *************************************************/

KDataTool::KDataTool( TQObject* parent, const char* name )
    : TQObject( parent, name ), m_instance( 0L )
{
}

TDEInstance* KDataTool::instance() const
{
   return m_instance;
}

void KDataToolAction::virtual_hook( int id, void* data )
{ TDEAction::virtual_hook( id, data ); }

void KDataTool::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdatatool.moc"

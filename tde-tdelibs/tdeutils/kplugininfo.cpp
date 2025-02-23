/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#include "kplugininfo.h"
#include <ksimpleconfig.h>
#include <ktrader.h>
#include <kdebug.h>
#include <tdeconfigbase.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kservice.h>

class KPluginInfo::KPluginInfoPrivate
{
    public:
        KPluginInfoPrivate()
            : hidden( false )
            , enabledbydefault( false )
            , pluginenabled( false )
            , config( 0 )
            , kcmservicesCached( false )
            {}

        ~KPluginInfoPrivate()
        {
            delete config;
        }

        TQString specfile; // the filename of the file containing all the info
        TQString name;
        TQString comment;
        TQString icon;
        TQString author;
        TQString email;
        TQString pluginName; // the name attribute in the .rc file
        TQString version;
        TQString website; // URL to the website of the plugin/author
        TQString category;
        TQString license;
        TQStringList dependencies;

        bool hidden;
        bool enabledbydefault;
        bool pluginenabled;

        TDEConfig * config;
        TQString configgroup;
        KService::Ptr service;
        TQValueList<KService::Ptr> kcmservices;
        bool kcmservicesCached;
};

KPluginInfo::KPluginInfo( const TQString & filename, const char* resource )
: d( new KPluginInfoPrivate )
{
    KDesktopFile file( filename, true, resource );

    d->specfile = filename;

    if( filename.endsWith( TQString::fromAscii( ".desktop" ) ) )
    {
        file.setDesktopGroup();
        d->hidden = file.readBoolEntry( "Hidden", false );
        if( d->hidden )
            return;

        d->name = file.readName();
        d->comment = file.readComment();
        d->icon = file.readEntry( "Icon" );
        d->author = file.readEntry( "X-TDE-PluginInfo-Author" );
        d->email = file.readEntry( "X-TDE-PluginInfo-Email" );
        d->pluginName = file.readEntry( "X-TDE-PluginInfo-Name" );
        d->version = file.readEntry( "X-TDE-PluginInfo-Version" );
        d->website = file.readEntry( "X-TDE-PluginInfo-Website" );
        d->category = file.readEntry( "X-TDE-PluginInfo-Category" );
        d->license = file.readEntry( "X-TDE-PluginInfo-License" );
        d->dependencies = file.readListEntry( "X-TDE-PluginInfo-Depends" );
        d->enabledbydefault = file.readBoolEntry(
                "X-TDE-PluginInfo-EnabledByDefault", false );
    }
    else if( filename.endsWith( TQString::fromAscii( ".plugin" ) ) )
    { // provided for noatun style .plugin files compatibility

        d->name = file.readName();
        d->comment = file.readComment();
        d->icon = file.readEntry( "Icon" );
        d->author = file.readEntry( "Author" );
        d->email = file.readEntry( "Email" );
        d->pluginName = file.readPathEntry( "Filename" );
        // no version
        d->website = file.readEntry( "Site" );
        d->category = file.readEntry( "Type" );
        d->license = file.readEntry( "License" );
        d->dependencies = file.readListEntry( "Require" );
    }
}

KPluginInfo::KPluginInfo( const KService::Ptr service )
: d( new KPluginInfoPrivate )
{
    d->service = service;
    d->specfile = service->desktopEntryPath();

    if ( service->isDeleted() )
    {
        d->hidden = true;
        return;
    }

    d->name = service->name();
    d->comment = service->comment();
    d->icon = service->icon();
    d->author = service->property( "X-TDE-PluginInfo-Author" ).toString();
    d->email = service->property( "X-TDE-PluginInfo-Email" ).toString();
    d->pluginName = service->property( "X-TDE-PluginInfo-Name" ).toString();
    d->version = service->property( "X-TDE-PluginInfo-Version" ).toString();
    d->website = service->property( "X-TDE-PluginInfo-Website" ).toString();
    d->category = service->property( "X-TDE-PluginInfo-Category" ).toString();
    d->license = service->property( "X-TDE-PluginInfo-License" ).toString();
    d->dependencies =
        service->property( "X-TDE-PluginInfo-Depends" ).toStringList();
    TQVariant tmp = service->property( "X-TDE-PluginInfo-EnabledByDefault" );
    d->enabledbydefault = tmp.isValid() ? tmp.toBool() : false;
}

//X KPluginInfo::KPluginInfo()
//X : d( new KPluginInfoPrivate )
//X {
//X     d->hidden = true;
//X }

KPluginInfo::~KPluginInfo()
{
    delete d;
}

TQValueList<KPluginInfo*> KPluginInfo::fromServices( const KService::List & services, TDEConfig * config, const TQString & group )
{
    TQValueList<KPluginInfo*> infolist;
    KPluginInfo * info;
    for( KService::List::ConstIterator it = services.begin();
            it != services.end(); ++it )
    {
        info = new KPluginInfo( *it );
        info->setConfig( config, group );
        infolist += info;
    }
    return infolist;
}

TQValueList<KPluginInfo*> KPluginInfo::fromFiles( const TQStringList & files, TDEConfig * config, const TQString & group )
{
    TQValueList<KPluginInfo*> infolist;
    for( TQStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
    {
        KPluginInfo * info = new KPluginInfo( *it );
        info->setConfig( config, group );
        infolist += info;
    }
    return infolist;
}

TQValueList<KPluginInfo*> KPluginInfo::fromKPartsInstanceName( const TQString & name, TDEConfig * config, const TQString & group )
{
    TQStringList files = TDEGlobal::dirs()->findAllResources( "data", name +
            "/kpartplugins/*.desktop", true, false );
    return fromFiles( files, config, group );
}

bool KPluginInfo::isHidden() const
{
    return d->hidden;
}

void KPluginInfo::setPluginEnabled( bool enabled )
{
    kdDebug( 703 ) << k_funcinfo << endl;
    d->pluginenabled = enabled;
}

bool KPluginInfo::isPluginEnabled() const
{
    kdDebug( 703 ) << k_funcinfo << endl;
    return d->pluginenabled;
}

bool KPluginInfo::isPluginEnabledByDefault() const
{
    kdDebug( 703 ) << k_funcinfo << endl;
    return d->enabledbydefault;
}

const TQString & KPluginInfo::name() const
{
    return d->name;
}

const TQString & KPluginInfo::comment() const
{
    return d->comment;
}

const TQString & KPluginInfo::icon() const
{
    return d->icon;
}

const TQString & KPluginInfo::specfile() const
{
    return d->specfile;
}

const TQString & KPluginInfo::author() const
{
    return d->author;
}

const TQString & KPluginInfo::email() const
{
    return d->email;
}

const TQString & KPluginInfo::category() const
{
    return d->category;
}

const TQString & KPluginInfo::pluginName() const
{
    return d->pluginName;
}

const TQString & KPluginInfo::version() const
{
    return d->version;
}

const TQString & KPluginInfo::website() const
{
    return d->website;
}

const TQString & KPluginInfo::license() const
{
    return d->license;
}

const TQStringList & KPluginInfo::dependencies() const
{
    return d->dependencies;
}

KService::Ptr KPluginInfo::service() const
{
    return d->service;
}

const TQValueList<KService::Ptr> & KPluginInfo::kcmServices() const
{
    if ( !d->kcmservicesCached )
    {
        d->kcmservices = TDETrader::self()->query( "TDECModule", "'" + d->pluginName +
            "' in [X-TDE-ParentComponents]" );
        kdDebug( 703 ) << "found " << d->kcmservices.count() << " offers for " <<
            d->pluginName << endl;

        d->kcmservicesCached = true;
    }

    return d->kcmservices;
}

void KPluginInfo::setConfig( TDEConfig * config, const TQString & group )
{
    d->config = config;
    d->configgroup = group;
}

TDEConfig * KPluginInfo::config() const
{
    return d->config;
}

const TQString & KPluginInfo::configgroup() const
{
    return d->configgroup;
}

TQVariant KPluginInfo::property( const TQString & key ) const
{
    if( d->service )
        return d->service->property( key );
    else
        return TQVariant();
}

TQVariant KPluginInfo::operator[]( const TQString & key ) const
{
    return property( key );
}

void KPluginInfo::save( TDEConfigGroup * config )
{
    kdDebug( 703 ) << k_funcinfo << endl;
    if( 0 == config )
    {
        if( 0 == d->config )
        {
            kdWarning( 703 ) << "no TDEConfigGroup, cannot save" << endl;
            return;
        }
        d->config->setGroup( d->configgroup );
        d->config->writeEntry( d->pluginName + "Enabled", isPluginEnabled() );
    }
    else
        config->writeEntry( d->pluginName + "Enabled", isPluginEnabled() );
}

void KPluginInfo::load( TDEConfigGroup * config )
{
    kdDebug( 703 ) << k_funcinfo << endl;
    if( 0 == config )
    {
        if( 0 == d->config )
        {
            kdWarning( 703 ) << "no TDEConfigGroup, cannot load" << endl;
            return;
        }
        d->config->setGroup( d->configgroup );
        setPluginEnabled( d->config->readBoolEntry( d->pluginName + "Enabled", isPluginEnabledByDefault() ) );
    }
    else
        setPluginEnabled( config->readBoolEntry( d->pluginName + "Enabled", isPluginEnabledByDefault() ) );
}

void KPluginInfo::defaults()
{
    kdDebug( 703 ) << k_funcinfo << endl;
    setPluginEnabled( isPluginEnabledByDefault() );
}

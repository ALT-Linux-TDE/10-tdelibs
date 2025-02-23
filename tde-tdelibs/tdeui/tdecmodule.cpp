/*
   This file is part of the KDE libraries

<<<Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
   Copyright (C) 2004 Frans Englich <frans.englich@telia.com>

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

#include <tqlayout.h>

#include <tdeaboutdata.h>
#include <tdeconfigskeleton.h>
#include <tdeconfigdialogmanager.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <kinstance.h>
#include <tdelocale.h>

#include "tdecmodule.h"
#include "tdecmodule.moc"

class TDECModulePrivate
{
public:
    TDECModulePrivate():
        _about( 0 ),
        _useRootOnlyMsg( false ),
        _hasOwnInstance( true ),
        _unmanagedWidgetChangeState( false )
        { }

    TDEInstance *_instance;
    TDEAboutData *_about;
    TQString _rootOnlyMsg;
    bool _useRootOnlyMsg;
    bool _hasOwnInstance;
    TQPtrList<TDEConfigDialogManager> managers;
    TQString _quickHelp;

    // this member is used to record the state on non-automatically
    // managed widgets, allowing for mixed TDEConfigXT-drive and manual
    // widgets to coexist peacefully and do the correct thing with
    // the changed(bool) signal
    bool _unmanagedWidgetChangeState;
};

TDECModule::TDECModule(TQWidget *parent, const char *name, const TQStringList &)
    : TQWidget(parent, name)
{
    init();
    if (name && strlen(name)) {
        d->_instance = new TDEInstance(name);
        TDEGlobal::locale()->insertCatalogue(name);
    } else
        d->_instance = new TDEInstance("kcmunnamed");
    TDEGlobal::setActiveInstance(this->instance());

    d->managers.setAutoDelete( true );

}

TDECModule::TDECModule(TDEInstance *instance, TQWidget *parent, const TQStringList & )
    : TQWidget(parent, instance ? instance->instanceName().data() : 0)
{
    init();
    d->_instance = instance;

    if (instance)
    {
        TDEGlobal::locale()->insertCatalogue(instance->instanceName());
    }

    d->_hasOwnInstance = false;
    TDEGlobal::setActiveInstance(this->instance());
}

void TDECModule::init()
{
    d = new TDECModulePrivate;
   _btn = Help|Default|Apply;
}

TDEConfigDialogManager* TDECModule::addConfig( TDEConfigSkeleton *config, TQWidget* widget )
{
    TDEConfigDialogManager* manager = new TDEConfigDialogManager( widget, config, name() );
    connect( manager, TQ_SIGNAL( widgetModified() ), TQ_SLOT( widgetChanged() ));
    d->managers.append( manager );
    return manager;
}

TDECModule::~TDECModule()
{
    if (d->_hasOwnInstance)
       delete d->_instance;
    delete d->_about;
    delete d;
}

void TDECModule::load()
{
    TDEConfigDialogManager* manager;
    for( manager = d->managers.first(); manager; manager = d->managers.next() )
        manager->updateWidgets();
}

void TDECModule::save()
{
    TDEConfigDialogManager* manager;
    for( manager = d->managers.first(); manager; manager = d->managers.next() )
        manager->updateSettings();
    emit( changed( false ));
}

void TDECModule::defaults()
{
    TDEConfigDialogManager* manager;
    for( manager = d->managers.first(); manager; manager = d->managers.next() )
        manager->updateWidgetsDefault();
}

void TDECModule::widgetChanged()
{
    emit changed(d->_unmanagedWidgetChangeState || managedWidgetChangeState());
}

bool TDECModule::managedWidgetChangeState() const
{
    TDEConfigDialogManager* manager;
    for( manager = d->managers.first(); manager; manager = d->managers.next() )
    {
        if ( manager->hasChanged() )
            return true;
    }

    return false;
}

void TDECModule::unmanagedWidgetChangeState(bool changed)
{
    d->_unmanagedWidgetChangeState = changed;
    widgetChanged();
}

const TDEAboutData *TDECModule::aboutData() const
{
    return d->_about;
}

void TDECModule::setAboutData( TDEAboutData* about )
{
    delete d->_about;
    d->_about = about;
}

void TDECModule::setRootOnlyMsg(const TQString& msg)
{
    d->_rootOnlyMsg = msg;
}

TQString TDECModule::rootOnlyMsg() const
{
    return d->_rootOnlyMsg;
}

void TDECModule::setUseRootOnlyMsg(bool on)
{
    d->_useRootOnlyMsg = on;
}

bool TDECModule::useRootOnlyMsg() const
{
    return d->_useRootOnlyMsg;
}

void TDECModule::changed()
{
    emit changed(true);
}

TDEInstance *TDECModule::instance() const
{
    return d->_instance;
}

void TDECModule::setQuickHelp( const TQString& help )
{
    d->_quickHelp = help;
    emit( quickHelpChanged() );
}

TQString TDECModule::quickHelp() const
{
    return d->_quickHelp;
}

TQString TDECModule::handbookDocPath() const
{
    return TQString::null;
}

TQString TDECModule::handbookSection() const
{
    return TQString::null;
}

const TQPtrList<TDEConfigDialogManager>& TDECModule::configs() const
{
    return d->managers;
}

void TDECModule::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

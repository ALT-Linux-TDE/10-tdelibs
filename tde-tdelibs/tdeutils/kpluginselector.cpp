/*  This file is part of the KDE project
    Copyright (C) 2002-2003 Matthias Kretz <kretz@kde.org>

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

#include "kpluginselector.h"
#include "kpluginselector_p.h"

#include <tqtooltip.h>
#include <tqvbox.h>
#include <tqlabel.h>
#include <tqstrlist.h>
#include <tqfile.h>
#include <tqstring.h>
#include <tqlayout.h>
#include <tqptrlist.h>
#include <tqwidgetstack.h>
#include <tqcursor.h>
#include <tqapplication.h>
#include <tqobjectlist.h>
#include <tqcstring.h>

#include <kdebug.h>
#include <tdelocale.h>
#include <tdelistview.h>
#include <ksimpleconfig.h>
#include <kdialog.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kstandarddirs.h>
#include <ktabctl.h>
#include <tdecmoduleinfo.h>
#include <tqvaluelist.h>
#include <kservice.h>
#include <ktrader.h>
#include <ktabwidget.h>
#include <kiconloader.h>
#include <tdecmodule.h>
#include "tdecmoduleinfo.h"
#include "tdecmoduleloader.h"
#include <tqsplitter.h>
#include <tqframe.h>
#include "kplugininfo.h"
#include <kinstance.h>
#include <tqptrdict.h>
#include <tqstringlist.h>
#include "tdecmoduleproxy.h"

/*
    QCheckListViewItem that holds a pointer to the KPluginInfo object.
    Used in the tooltip code to access additional fields
*/
class KPluginInfoLVI : public TQCheckListItem
{
public:
    KPluginInfoLVI( KPluginInfo *pluginInfo, TDEListView *parent )
    : TQCheckListItem( parent, pluginInfo->name(), TQCheckListItem::CheckBox ), m_pluginInfo( pluginInfo )
    {
    }

    KPluginInfo * pluginInfo() { return m_pluginInfo; }

private:
    KPluginInfo *m_pluginInfo;
};

/*
	Custom TQToolTip for the list view.
	The decision whether or not to show tooltips is taken in
	maybeTip(). See also the TQListView sources from Qt itself.
*/
class KPluginListViewToolTip : public TQToolTip
{
public:
	KPluginListViewToolTip( TQWidget *parent, TDEListView *lv );

	void maybeTip( const TQPoint &pos );

private:
	TDEListView *m_listView;
};

KPluginListViewToolTip::KPluginListViewToolTip( TQWidget *parent, TDEListView *lv )
: TQToolTip( parent ), m_listView( lv )
{
}

void KPluginListViewToolTip::maybeTip( const TQPoint &pos )
{
    if ( !parentWidget() || !m_listView )
        return;

    KPluginInfoLVI *item = dynamic_cast<KPluginInfoLVI *>( m_listView->itemAt( pos ) );
    if ( !item )
        return;

    TQString toolTip = i18n( "<qt><table>"
        "<tr><td><b>Description:</b></td><td>%1</td></tr>"
        "<tr><td><b>Author:</b></td><td>%2</td></tr>"
        "<tr><td><b>Version:</b></td><td>%3</td></tr>"
        "<tr><td><b>License:</b></td><td>%4</td></tr></table></qt>" ).arg( item->pluginInfo()->comment(),
        item->pluginInfo()->author(), item->pluginInfo()->version(), item->pluginInfo()->license() );

    //kdDebug( 702 ) << k_funcinfo << "Adding tooltip: itemRect: " << itemRect << ", tooltip:  " << toolTip << endl;
    tip( m_listView->itemRect( item ), toolTip );
}

struct KPluginSelectionWidget::KPluginSelectionWidgetPrivate
{
    KPluginSelectionWidgetPrivate( KPluginSelector * _kps,
                                   const TQString & _cat,
                                   TDEConfigGroup * _config )
     : widgetstack( 0 )
        , kps( _kps )
        , config( _config )
        , tooltip( 0 )
        , catname( _cat )
        , currentplugininfo( 0 )
        , visible( true )
        , currentchecked( false )
        , changed( 0 )
    {
        moduleParentComponents.setAutoDelete( true );
    }

    ~KPluginSelectionWidgetPrivate()
    {
        delete config;
    }

    TQMap<TQCheckListItem*, KPluginInfo*> pluginInfoMap;

    TQWidgetStack * widgetstack;
    KPluginSelector * kps;
    TDEConfigGroup * config;
    KPluginListViewToolTip *tooltip;

    TQDict<TDECModuleInfo> pluginconfigmodules;
    TQMap<TQString, int> widgetIDs;
    TQMap<KPluginInfo*, bool> plugincheckedchanged;
    TQString catname;
    TQValueList<TDECModuleProxy*> modulelist;
    TQPtrDict<TQStringList> moduleParentComponents;

    KPluginInfo * currentplugininfo;
    bool visible;
    bool currentchecked;
    int changed;
};

KPluginSelectionWidget::KPluginSelectionWidget(
        const TQValueList<KPluginInfo*> & plugininfos, KPluginSelector * kps,
        TQWidget * parent, const TQString & catname, const TQString & category,
        TDEConfigGroup * config, const char * name )
    : TQWidget( parent, name )
    , d( new KPluginSelectionWidgetPrivate( kps, catname, config ) )
{
    init( plugininfos, category );
}

inline TQString KPluginSelectionWidget::catName() const
{
    return d->catname;
}

void KPluginSelectionWidget::init( const TQValueList<KPluginInfo*> & plugininfos,
        const TQString & category )
{
    // setup Widgets
    ( new TQVBoxLayout( this, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    TDEListView * listview = new TDEListView( this );
    d->tooltip = new KPluginListViewToolTip( listview->viewport(), listview );
    connect( listview, TQ_SIGNAL( pressed( TQListViewItem * ) ), this,
            TQ_SLOT( executed( TQListViewItem * ) ) );
    connect( listview, TQ_SIGNAL( spacePressed( TQListViewItem * ) ), this,
            TQ_SLOT( executed( TQListViewItem * ) ) );
    connect( listview, TQ_SIGNAL( returnPressed( TQListViewItem * ) ), this,
            TQ_SLOT( executed( TQListViewItem * ) ) );
    connect( listview, TQ_SIGNAL( selectionChanged( TQListViewItem * ) ), this,
            TQ_SLOT( executed( TQListViewItem * ) ) );
    listview->setSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Preferred );
    listview->setAcceptDrops( false );
    listview->setFullWidth( true );
    listview->setSelectionModeExt( TDEListView::Single );
    listview->setAllColumnsShowFocus( true );
    listview->addColumn( i18n( "Name" ) );
    for( TQValueList<KPluginInfo*>::ConstIterator it = plugininfos.begin();
            it != plugininfos.end(); ++it )
    {
        d->plugincheckedchanged[ *it ] = false;
        if( !( *it )->isHidden() &&
                ( category.isNull() || ( *it )->category() == category ) )
        {
            TQCheckListItem * item = new KPluginInfoLVI( *it, listview );
            if( ! ( *it )->icon().isEmpty() )
                item->setPixmap( 0, SmallIcon( ( *it )->icon(), IconSize( TDEIcon::Small ) ) );
            item->setOn( ( *it )->isPluginEnabled() );
            d->pluginInfoMap.insert( item, *it );
        }
    }

    // widgetstack
    d->widgetstack = d->kps->widgetStack();
    load();
    // select and highlight the first item in the plugin list
    if( listview->firstChild() )
        listview->setSelected( listview->firstChild(), true );
}

KPluginSelectionWidget::~KPluginSelectionWidget()
{
    delete d->tooltip;
    delete d;
}

bool KPluginSelectionWidget::pluginIsLoaded( const TQString & pluginName ) const
{
    for( TQMap<TQCheckListItem*, KPluginInfo*>::ConstIterator it =
            d->pluginInfoMap.begin(); it != d->pluginInfoMap.end(); ++it )
        if( it.data()->pluginName() == pluginName )
            return it.data()->isPluginEnabled();
    return false;
}


TQWidget * KPluginSelectionWidget::insertKCM( TQWidget * parent,
        const TDECModuleInfo & moduleinfo )
{
    TDECModuleProxy * module = new TDECModuleProxy( moduleinfo, false,
            parent );
    if( !module->realModule() )
    {
        //FIXME: not very verbose
        TQLabel * label = new TQLabel( i18n( "Error" ), parent );
        label->setAlignment( TQt::AlignCenter );

        return label;
    }
    // add the KCM to the list so that we can call load/save/defaults on it
    d->modulelist.append( module );
    TQStringList * parentComponents = new TQStringList(
            moduleinfo.service()->property(
                "X-TDE-ParentComponents" ).toStringList() );
    d->moduleParentComponents.insert( module, parentComponents );
    connect( module, TQ_SIGNAL( changed( bool ) ), TQ_SLOT( clientChanged( bool ) ) );
    return module;
}

void KPluginSelectionWidget::embeddPluginKCMs( KPluginInfo * plugininfo, bool checked )
{
    //if we have Services for the plugin we should be able to
    //create KCM(s)
    TQApplication::setOverrideCursor( TQt::WaitCursor );
    if( plugininfo->kcmServices().size() > 1 )
    {
        // we need a tabwidget
        KTabWidget * tabwidget = new KTabWidget( d->widgetstack );
        tabwidget->setEnabled( checked );

        int id = d->widgetstack->addWidget( tabwidget );
        d->kps->configPage( id );
        d->widgetIDs[ plugininfo->pluginName() ] = id;

        for( TQValueList<KService::Ptr>::ConstIterator it =
                plugininfo->kcmServices().begin();
                it != plugininfo->kcmServices().end(); ++it )
        {
            if( !( *it )->noDisplay() )
            {
                TDECModuleInfo moduleinfo( *it );
                TQWidget * module = insertKCM( tabwidget, moduleinfo );
                tabwidget->addTab( module, moduleinfo.moduleName() );
            }
        }
    }
    else
    {
        if( !plugininfo->kcmServices().front()->noDisplay() )
        {
            TDECModuleInfo moduleinfo(
                    plugininfo->kcmServices().front() );
            TQWidget * module = insertKCM( d->widgetstack, moduleinfo );
            module->setEnabled( checked );

            int id = d->widgetstack->addWidget( module );
            d->kps->configPage( id );
            d->widgetIDs[ plugininfo->pluginName() ] = id;
        }
    }
    TQApplication::restoreOverrideCursor();
}

inline void KPluginSelectionWidget::updateConfigPage()
{
    updateConfigPage( d->currentplugininfo, d->currentchecked );
}

void KPluginSelectionWidget::updateConfigPage( KPluginInfo * plugininfo,
        bool checked )
{
    //kdDebug( 702 ) << k_funcinfo << endl;
    d->currentplugininfo = plugininfo;
    d->currentchecked = checked;

    // if this widget is not currently visible (meaning that it's in a tabwidget
    // and another tab is currently opened) it's not allowed to change the
    // widgetstack
    if( ! d->visible )
        return;

    if( 0 == plugininfo )
    {
        d->kps->configPage( 1 );
        return;
    }

    if( plugininfo->kcmServices().empty() )
        d->kps->configPage( 1 );
    else
    {
        if( !d->widgetIDs.contains( plugininfo->pluginName() ) )
            // if no widget exists for the plugin create it
            embeddPluginKCMs( plugininfo, checked );
        else
        {
            // the page already exists
            int id = d->widgetIDs[ plugininfo->pluginName() ];
            d->kps->configPage( id );
            d->widgetstack->widget( id )->setEnabled( checked );
        }
    }
}

void KPluginSelectionWidget::clientChanged( bool didchange )
{
    kdDebug( 702 ) << k_funcinfo << endl;
    d->changed += didchange ? 1 : -1;
    if( d->changed == 1 )
        emit changed( true );
    else if( d->changed == 0 )
        emit changed( false );
    else if( d->changed < 0 )
        kdError( 702 ) << "negative changed value: " << d->changed << endl;
}

void KPluginSelectionWidget::tabWidgetChanged( TQWidget * widget )
{
    if( widget == this )
    {
        d->visible = true;
        updateConfigPage();
    }
    else
        d->visible = false;
}

void KPluginSelectionWidget::executed( TQListViewItem * item )
{
    kdDebug( 702 ) << k_funcinfo << endl;
    if( item == 0 )
        return;

    // Why not a dynamic_cast? - Martijn
    // because this is what the Qt API suggests; and since gcc 3.x I don't
    // trust dynamic_cast anymore - mkretz
    if( item->rtti() != 1 ) //check for a QCheckListItem
        return;

    TQCheckListItem * citem = static_cast<TQCheckListItem *>( item );
    bool checked = citem->isOn();
    //kdDebug( 702 ) << "it's a " << ( checked ? "checked" : "unchecked" )
    //    << " TQCheckListItem" << endl;

    KPluginInfo * info = d->pluginInfoMap[ citem ];
    Q_ASSERT( !info->isHidden() );

    if ( info->isPluginEnabled() != checked )
    {
        kdDebug( 702 ) << "Item changed state, emitting changed()" << endl;

        if( ! d->plugincheckedchanged[ info ] )
        {
            ++d->changed;
            if ( d->changed == 1 )
                emit changed( true );
        }
        d->plugincheckedchanged[ info ] = true;

        checkDependencies( info );
    }
    else
    {
        if( d->plugincheckedchanged[ info ] )
        {
            --d->changed;
            if ( d->changed == 0 )
                emit changed( false );
        }
        d->plugincheckedchanged[ info ] = false;
        // FIXME: plugins that depend on this plugin need to be disabled, too
    }

    updateConfigPage( info, checked );
}

void KPluginSelectionWidget::load()
{
    //kdDebug( 702 ) << k_funcinfo << endl;

    for( TQMap<TQCheckListItem*, KPluginInfo*>::Iterator it =
            d->pluginInfoMap.begin(); it != d->pluginInfoMap.end(); ++it )
    {
        KPluginInfo * info = it.data();
        info->load( d->config );
        it.key()->setOn( info->isPluginEnabled() );
        if( d->visible && info == d->currentplugininfo )
            d->currentchecked = info->isPluginEnabled();
    }

    for( TQValueList<TDECModuleProxy*>::Iterator it = d->modulelist.begin();
            it != d->modulelist.end(); ++it )
        if( ( *it )->changed() )
            ( *it )->load();

    updateConfigPage();
    // TODO: update changed state
}

void KPluginSelectionWidget::save()
{
    kdDebug( 702 ) << k_funcinfo << endl;

    for( TQMap<TQCheckListItem*, KPluginInfo*>::Iterator it =
            d->pluginInfoMap.begin(); it != d->pluginInfoMap.end(); ++it )
    {
        KPluginInfo * info = it.data();
        bool checked = it.key()->isOn();
        info->setPluginEnabled( checked );
        info->save( d->config );
        d->plugincheckedchanged[ info ] = false;
    }
    TQStringList updatedModules;
    for( TQValueList<TDECModuleProxy*>::Iterator it = d->modulelist.begin();
            it != d->modulelist.end(); ++it )
        if( ( *it )->changed() )
        {
            ( *it )->save();
            TQStringList * names = d->moduleParentComponents[ *it ];
            if( names->size() == 0 )
                names->append( TQString::null );
            for( TQStringList::ConstIterator nameit = names->begin();
                    nameit != names->end(); ++nameit )
                if( updatedModules.find( *nameit ) == updatedModules.end() )
                    updatedModules.append( *nameit );
        }
    for( TQStringList::ConstIterator it = updatedModules.begin(); it != updatedModules.end(); ++it )
        emit configCommitted( ( *it ).latin1() );

    updateConfigPage();
    kdDebug( 702 ) << "syncing config file" << endl;
    d->config->sync();
    d->changed = 0;
    emit changed( false );
}

void KPluginSelectionWidget::checkDependencies( const KPluginInfo * info )
{
    if( info->dependencies().isEmpty() )
        return;

    for( TQStringList::ConstIterator it = info->dependencies().begin();
            it != info->dependencies().end(); ++it )
        for( TQMap<TQCheckListItem*,
                KPluginInfo*>::Iterator infoIt = d->pluginInfoMap.begin();
                infoIt != d->pluginInfoMap.end(); ++infoIt )
            if( infoIt.data()->pluginName() == *it )
            {
                if( !infoIt.key()->isOn() )
                {
                    infoIt.key()->setOn( true );
                    checkDependencies( infoIt.data() );
                }
                continue;
            }
}

class KPluginSelector::KPluginSelectorPrivate
{
    public:
        KPluginSelectorPrivate()
            : frame( 0 )
            , tabwidget( 0 )
            , widgetstack( 0 )
            , hideconfigpage( false )
            {
            }

        TQFrame * frame;
        KTabWidget * tabwidget;
        TQWidgetStack * widgetstack;
        TQValueList<KPluginSelectionWidget *> pswidgets;
        bool hideconfigpage;
};

KPluginSelector::KPluginSelector( TQWidget * parent, const char * name )
: TQWidget( parent, name )
, d( new KPluginSelectorPrivate )
{
    TQBoxLayout * hbox = new TQHBoxLayout( this, 0, KDialog::spacingHint() );
    hbox->setAutoAdd( true );

    TQSplitter* splitter = new TQSplitter( TQt::Horizontal, this );
    d->frame = new TQFrame( splitter, "KPluginSelector left frame" );
    d->frame->setFrameStyle( TQFrame::NoFrame );
    ( new TQVBoxLayout( d->frame, 0, KDialog::spacingHint() ) )->setAutoAdd( true );

    // widgetstack
    d->widgetstack = new TQWidgetStack( splitter, "KPluginSelector Config Pages" );
    d->widgetstack->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
    d->widgetstack->setMinimumSize( 200, 200 );

    TQLabel * label = new TQLabel( i18n( "(This plugin is not configurable)" ),
            d->widgetstack );
    ( new TQVBoxLayout( label, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    label->setAlignment( TQt::AlignCenter );
    label->setMinimumSize( 200, 200 );

    d->widgetstack->addWidget( label, 1 );

    configPage( 1 );
}

KPluginSelector::~KPluginSelector()
{
    delete d;
}

void KPluginSelector::checkNeedForTabWidget()
{
    kdDebug( 702 ) << k_funcinfo << endl;
    if( ! d->tabwidget && d->pswidgets.size() == 1 )
    {
        kdDebug( 702 ) << "no TabWidget and one KPluginSelectionWidget" << endl;
        // there's only one KPluginSelectionWidget yet, we need a TabWidget
        KPluginSelectionWidget * w = d->pswidgets.first();
        if( w )
        {
            kdDebug( 702 ) << "create TabWidget" << endl;
            d->tabwidget = new KTabWidget( d->frame,
                    "KPluginSelector TabWidget" );
            w->reparent( d->tabwidget, TQPoint( 0, 0 ) );
            d->tabwidget->addTab( w, w->catName() );
            connect( d->tabwidget, TQ_SIGNAL( currentChanged( TQWidget * ) ), w,
                    TQ_SLOT( tabWidgetChanged( TQWidget * ) ) );
        }
    }
}

static TQValueList<KPluginInfo*> tdepartsPluginInfos( const TQString& instanceName )
{
    if( instanceName.isNull() )
        return TQValueList<KPluginInfo*>(); //nothing

    const TQStringList desktopfilenames = TDEGlobal::dirs()->findAllResources( "data",
            instanceName + "/kpartplugins/*.desktop", true, false );
    return KPluginInfo::fromFiles( desktopfilenames );
}

void KPluginSelector::addPlugins( const TQString & instanceName,
        const TQString & catname, const TQString & category, TDEConfig * config )
{
    const TQValueList<KPluginInfo*> plugininfos = tdepartsPluginInfos( instanceName );
    if ( plugininfos.isEmpty() )
        return;
    checkNeedForTabWidget();
    Q_ASSERT( config ); // please set config, or use addPlugins( instance, ... ) which takes care of it
    if ( !config ) // KDE4: ensure that config is always set; make it second in the arg list?
        config = new KSimpleConfig(  instanceName ); // memleak!
    TDEConfigGroup * cfgGroup = new TDEConfigGroup( config, "KParts Plugins" );
    kdDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup << endl;
    addPluginsInternal( plugininfos, catname, category, cfgGroup );
}

void KPluginSelector::addPluginsInternal( const TQValueList<KPluginInfo*> plugininfos,
                                          const TQString & catname, const TQString & category,
                                          TDEConfigGroup* cfgGroup )
{
    KPluginSelectionWidget * w;
    if( d->tabwidget )
    {
        w = new KPluginSelectionWidget( plugininfos, this,
                d->tabwidget, catname, category, cfgGroup );
        d->tabwidget->addTab( w, catname );
        connect( d->tabwidget, TQ_SIGNAL( currentChanged( TQWidget * ) ), w,
                TQ_SLOT( tabWidgetChanged( TQWidget * ) ) );
    }
    else
        w = new KPluginSelectionWidget( plugininfos, this, d->frame,
                catname, category, cfgGroup );
    w->setMinimumSize( 200, 200 );
    connect( w, TQ_SIGNAL( changed( bool ) ), this, TQ_SIGNAL( changed( bool ) ) );
    connect( w, TQ_SIGNAL( configCommitted( const TQCString & ) ), this,
            TQ_SIGNAL( configCommitted( const TQCString & ) ) );
    d->pswidgets += w;
}

void KPluginSelector::addPlugins( const TDEInstance * instance, const TQString &
        catname, const TQString & category, TDEConfig * config )
{
    if ( !config )
        config = instance->config();
    addPlugins( instance->instanceName(), catname, category, config );
}

void KPluginSelector::addPlugins( const TQValueList<KPluginInfo*> & plugininfos,
        const TQString & catname, const TQString & category, TDEConfig * config )
{
    checkNeedForTabWidget();
    // the TDEConfigGroup becomes owned by KPluginSelectionWidget
    TDEConfigGroup * cfgGroup = new TDEConfigGroup( config ? config : TDEGlobal::config(), "Plugins" );
    kdDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup << endl;
    addPluginsInternal( plugininfos, catname, category, cfgGroup );
}

TQWidgetStack * KPluginSelector::widgetStack()
{
    return d->widgetstack;
}

inline void KPluginSelector::configPage( int id )
{
    if( id == 1 )
    {
        // no config page
        if( d->hideconfigpage )
        {
            d->widgetstack->hide();
            return;
        }
    }
    else
        d->widgetstack->show();

    d->widgetstack->raiseWidget( id );
}

void KPluginSelector::setShowEmptyConfigPage( bool show )
{
    d->hideconfigpage = !show;
    if( d->hideconfigpage )
        if( d->widgetstack->id( d->widgetstack->visibleWidget() ) == 1 )
            d->widgetstack->hide();
}

void KPluginSelector::load()
{
    for( TQValueList<KPluginSelectionWidget *>::Iterator it =
            d->pswidgets.begin(); it != d->pswidgets.end(); ++it )
    {
        ( *it )->load();
    }
}

void KPluginSelector::save()
{
    for( TQValueList<KPluginSelectionWidget *>::Iterator it =
            d->pswidgets.begin(); it != d->pswidgets.end(); ++it )
    {
        ( *it )->save();
    }
}

void KPluginSelector::defaults()
{
    kdDebug( 702 ) << k_funcinfo << endl;

    // what should defaults do? here's what I think:
    // Pressing a button in the dialog should not change any widgets that are
    // not visible for the user. Therefor we may only change the currently
    // visible plugin's KCM. Restoring the default plugin selections is therefor
    // not possible. (if the plugin has multiple KCMs they will be shown in a
    // tabwidget - defaults() will be called for all of them)

    TQWidget * pluginconfig = d->widgetstack->visibleWidget();
    TDECModuleProxy * kcm = ::tqt_cast<TDECModuleProxy*>(pluginconfig);
    if( kcm )
    {
        kdDebug( 702 ) << "call TDECModule::defaults() for the plugins KCM"
            << endl;
        kcm->defaults();
        return;
    }

    // if we get here the visible Widget must be a tabwidget holding more than
    // one KCM
    TQObjectList * kcms = pluginconfig->queryList( "TDECModuleProxy",
            0, false, false );
    TQObjectListIt it( *kcms );
    TQObject * obj;
    while( ( obj = it.current() ) != 0 )
    {
        ++it;
        ( ( TDECModule* )obj )->defaults();
    }
    delete kcms;
    // FIXME: update changed state
}
#include "kpluginselector.moc"
#include "kpluginselector_p.moc"

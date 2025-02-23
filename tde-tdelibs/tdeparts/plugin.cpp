/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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

#include <config.h>
#include <tdeparts/plugin.h>
#include <tdeparts/part.h>
#include <tdeparts/componentfactory.h>

#include <assert.h>

#include <tqfile.h>
#include <tqobjectlist.h>
#include <tqfileinfo.h>

#include <klibloader.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kxmlguifactory.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <ksimpleconfig.h>

using namespace KParts;

class Plugin::PluginPrivate
{
public:
    PluginPrivate() : m_parentInstance( 0 ) {}

    const TDEInstance *m_parentInstance;
    TQString m_library; // filename of the library
};

Plugin::Plugin( TQObject* parent, const char* name )
    : TQObject( parent, name )
{
  //kdDebug() << className() << endl;
  d = new PluginPrivate();
}

Plugin::~Plugin()
{
    delete d;
}

TQString Plugin::xmlFile() const
{
    TQString path = KXMLGUIClient::xmlFile();

    if ( !d->m_parentInstance || ( path.length() > 0 && path[ 0 ] == '/' ) )
        return path;

    TQString absPath = locate( "data", TQString::fromLatin1( d->m_parentInstance->instanceName() ) + '/' + path );
    assert( !absPath.isEmpty() );
    return absPath;
}

TQString Plugin::localXMLFile() const
{
    TQString path = KXMLGUIClient::xmlFile();

    if ( !d->m_parentInstance || ( path.length() > 0 && path[ 0 ] == '/' ) )
        return path;

    TQString absPath = locateLocal( "data", TQString::fromLatin1( d->m_parentInstance->instanceName() ) + '/' + path );
    assert( !absPath.isEmpty() );
    return absPath;
}

//static
TQValueList<Plugin::PluginInfo> Plugin::pluginInfos( const TDEInstance * instance )
{
  if ( !instance )
    kdError(1000) << "No instance ???" << endl;

  TQValueList<PluginInfo> plugins;

  // KDE4: change * into *.rc and remove test for .desktop from the for loop below.
  const TQStringList pluginDocs = instance->dirs()->findAllResources(
    "data", instance->instanceName()+"/kpartplugins/*", true, false );

  TQMap<TQString,TQStringList> sortedPlugins;

  TQStringList::ConstIterator pIt = pluginDocs.begin();
  TQStringList::ConstIterator pEnd = pluginDocs.end();
  for (; pIt != pEnd; ++pIt )
  {
      TQFileInfo fInfo( *pIt );
      if ( fInfo.extension() == TQString::fromLatin1( "desktop" ) )
          continue;

      TQMap<TQString,TQStringList>::Iterator mapIt = sortedPlugins.find( fInfo.fileName() );
      if ( mapIt == sortedPlugins.end() )
          mapIt = sortedPlugins.insert( fInfo.fileName(), TQStringList() );

      mapIt.data().append( *pIt );
  }

  TQMap<TQString,TQStringList>::ConstIterator mapIt = sortedPlugins.begin();
  TQMap<TQString,TQStringList>::ConstIterator mapEnd = sortedPlugins.end();
  for (; mapIt != mapEnd; ++mapIt )
  {
      PluginInfo info;
      TQString doc;
      info.m_absXMLFileName = KXMLGUIClient::findMostRecentXMLFile( mapIt.data(), doc );
      if ( info.m_absXMLFileName.isEmpty() )
          continue;

      kdDebug( 1000 ) << "found KParts Plugin : " << info.m_absXMLFileName << endl;
      info.m_relXMLFileName = "kpartplugins/";
      info.m_relXMLFileName += mapIt.key();

      info.m_document.setContent( doc );
      if ( info.m_document.documentElement().isNull() )
          continue;

      plugins.append( info );
  }

  return plugins;
}

void Plugin::loadPlugins( TQObject *parent, const TDEInstance *instance )
{
  loadPlugins( parent, pluginInfos( instance ), instance );
}

void Plugin::loadPlugins( TQObject *parent, const TQValueList<PluginInfo> &pluginInfos, const TDEInstance *instance )
{
   TQValueList<PluginInfo>::ConstIterator pIt = pluginInfos.begin();
   TQValueList<PluginInfo>::ConstIterator pEnd = pluginInfos.end();
   for (; pIt != pEnd; ++pIt )
   {
     TQString library = (*pIt).m_document.documentElement().attribute( "library" );

     if ( library.isEmpty() || hasPlugin( parent, library ) )
       continue;

     Plugin *plugin = loadPlugin( parent, TQFile::encodeName(library) );

     if ( plugin )
     {
       plugin->d->m_parentInstance = instance;
       plugin->setXMLFile( (*pIt).m_relXMLFileName, false, false );
       plugin->setDOMDocument( (*pIt).m_document );

     }
   }

}

void Plugin::loadPlugins( TQObject *parent, const TQValueList<PluginInfo> &pluginInfos )
{
   loadPlugins(parent, pluginInfos, 0);
}

// static
Plugin* Plugin::loadPlugin( TQObject * parent, const char* libname )
{
    Plugin* plugin = ComponentFactory::createInstanceFromLibrary<Plugin>( libname, parent, libname );
    if ( !plugin )
        return 0L;
    plugin->d->m_library = libname;
    return plugin;
}

TQPtrList<KParts::Plugin> Plugin::pluginObjects( TQObject *parent )
{
  TQPtrList<KParts::Plugin> objects;

  if (!parent )
    return objects;

  TQObjectList *plugins = parent->queryList( "KParts::Plugin", 0, false, false );

  TQObjectListIt it( *plugins );
  for ( ; it.current() ; ++it )
  {
    objects.append( static_cast<Plugin *>( it.current() ) );
  }

  delete plugins;

  return objects;
}

bool Plugin::hasPlugin( TQObject* parent, const TQString& library )
{
  TQObjectList *plugins = parent->queryList( "KParts::Plugin", 0, false, false );
  TQObjectListIt it( *plugins );
  for ( ; it.current() ; ++it )
  {
      if ( static_cast<Plugin *>( it.current() )->d->m_library == library )
      {
          delete plugins;
          return true;
      }
  }
  delete plugins;
  return false;
}

void Plugin::setInstance( TDEInstance *instance )
{
    TDEGlobal::locale()->insertCatalogue( instance->instanceName() );
    KXMLGUIClient::setInstance( instance );
}

void Plugin::loadPlugins( TQObject *parent, KXMLGUIClient* parentGUIClient, TDEInstance* instance, bool enableNewPluginsByDefault )
{
    TDEConfigGroup cfgGroup( instance->config(), "KParts Plugins" );
    TQValueList<PluginInfo> plugins = pluginInfos( instance );
    TQValueList<PluginInfo>::ConstIterator pIt = plugins.begin();
    TQValueList<PluginInfo>::ConstIterator pEnd = plugins.end();
    for (; pIt != pEnd; ++pIt )
    {
        TQDomElement docElem = (*pIt).m_document.documentElement();
        TQString library = docElem.attribute( "library" );

        if ( library.isEmpty() )
            continue;

        // Check configuration
        const TQString name = docElem.attribute( "name" );

        bool pluginEnabled = enableNewPluginsByDefault;
        if ( cfgGroup.hasKey( name + "Enabled" ) )
        {
            pluginEnabled = cfgGroup.readBoolEntry( name + "Enabled" );
        }
        else
        { // no user-setting, load plugin default setting
            TQString relPath = TQString( instance->instanceName() ) + "/" + (*pIt).m_relXMLFileName;
            relPath.truncate( relPath.findRev( '.' ) ); // remove extension
            relPath += ".desktop";
            //kdDebug(1000) << "looking for " << relPath << endl;
            const TQString desktopfile = instance->dirs()->findResource( "data", relPath );
            if( !desktopfile.isEmpty() )
            {
                //kdDebug(1000) << "loadPlugins found desktop file for " << name << ": " << desktopfile << endl;
                KSimpleConfig desktop( desktopfile, true );
                desktop.setDesktopGroup();
                pluginEnabled = desktop.readBoolEntry(
                    "X-TDE-PluginInfo-EnabledByDefault", enableNewPluginsByDefault );
            }
            else
            {
                //kdDebug(1000) << "loadPlugins no desktop file found in " << relPath << endl;
            }
        }

        // search through already present plugins
        TQObjectList *pluginList = parent->queryList( "KParts::Plugin", 0, false, false );
        TQObjectListIt it( *pluginList );
        bool pluginFound = false;
        for ( ; it.current() ; ++it )
        {
            Plugin * plugin = static_cast<Plugin *>( it.current() );
            if( plugin->d->m_library == library )
            {
                // delete and unload disabled plugins
                if( !pluginEnabled )
                {
                    kdDebug( 1000 ) << "remove plugin " << name << endl;
                    KXMLGUIFactory * factory = plugin->factory();
                    if( factory )
                        factory->removeClient( plugin );
                    delete plugin;
                }

                pluginFound = true;
                break;
            }
        }
        delete pluginList;

        // if the plugin is already loaded or if it's disabled in the
        // configuration do nothing
        if( pluginFound || !pluginEnabled )
            continue;

        kdDebug( 1000 ) << "load plugin " << name << endl;
        Plugin *plugin = loadPlugin( parent, TQFile::encodeName(library) );

        if ( plugin )
        {
            plugin->d->m_parentInstance = instance;
            plugin->setXMLFile( (*pIt).m_relXMLFileName, false, false );
            plugin->setDOMDocument( (*pIt).m_document );
            parentGUIClient->insertChildClient( plugin );
        }
    }
}
#include "plugin.moc"

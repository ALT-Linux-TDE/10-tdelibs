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

#ifndef KPLUGINSELECTOR_H
#define KPLUGINSELECTOR_H

#include <tqwidget.h>
#include <tqstring.h>

#include <tdelibs_export.h>

class TDEInstance;
class KPluginInfo;
class TQWidgetStack;
class TDEConfig;
class TDEConfigGroup;

/**
 * @ingroup main
 * @ingroup plugin
 * @short A widget to select what plugins to load and configure the plugins.
 *
 * It shows the list of available plugins on top (if there's more than one
 * category this is a TabWidget) and the configuration of the selected plugin
 * below that.
 *
 * Since the user needs a way to know what a specific plugin does every plugin
 * sould install a desktop file containing a name, comment and category field.
 * The category is usefull for applications that can use different kinds of
 * plugins like a playlist, skin or visualization.
 *
 * The location of these desktop files is the
 * share/apps/&lt;instancename&gt;/&lt;plugindir&gt; directory. But if you need
 * you may use a different directory.
 *
 * Often a program has more than one kind of plugin. In that case you want to
 * make a visible distinction between those plugins. All you have to do is to
 * create a KPluginSelectionWidget for every category and then add them all
 * to the KPluginSelector.
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class TDEUTILS_EXPORT KPluginSelector : public TQWidget
{
    friend class KPluginSelectionWidget;

    TQ_OBJECT
    public:
        /**
         * Create a new KPluginSelector.
         */
        KPluginSelector( TQWidget * parent, const char * name = 0 );
        ~KPluginSelector();

        /**
         * Add a list of KParts plugins
         *
         * If you want to support non-KParts plugins use the following
         * function.
         *
         * The information about the plugins will be loaded from the
         * share/apps/&lt;instancename&gt;/kpartplugins directory.
         *
         * @param instanceName The name of the TDEInstance of the plugin's parent.
         * @param catname      The translated name of the category. This is the
         *                     name that is shown in the TabWidget if there is
         *                     more than one category.
         * @param category     When you have different categories of KParts
         *                     plugins you distinguish between the plugins using
         *                     the Category key in the .desktop file. Use this
         *                     parameter to select only those KParts plugins
         *                     with the Category key == @p category. If @p
         *                     category is not set the Category key is ignored
         *                     and all plugins are shown.
         * @param config       The TDEConfig object that holds the state of the
         *                     plugins being enabled or not. By default it should
         *                     be instance->config(). It is recommended to
         *                     always pass a TDEConfig object if you use
         *                     KSettings::PluginPage since you never know from where the
         *                     page will be called (think global config app).
         *                     For example KViewCanvas passes KSimpleConfig(
         *                     "kviewcanvas" ).
         */
        void addPlugins( const TQString & instanceName,
                const TQString & catname = TQString::null,
                const TQString & category = TQString::null,
                TDEConfig * config = 0 );

        /**
         * Add a list of KParts plugins. Convenience method for the one above.
         * If not set explicitely, @p config is set to instance->config().
         */
        void addPlugins( const TDEInstance * instance,
                const TQString & catname = TQString::null,
                const TQString & category = TQString::null,
                TDEConfig * config = 0 );

        /**
         * Add a list of non-KParts plugins
         *
         * @param plugininfos  A list of KPluginInfo objects containing the
         *                     necessary information for the plugins you want to
         *                     add to the list.
         * @param catname      The translated name of the category. This is the
         *                     name that is shown in the TabWidget if there is
         *                     more than one category.
         * @param category     When you have different categories of KParts
         *                     plugins you distinguish between the plugins using
         *                     the Category key in the .desktop file. Use this
         *                     parameter to select only those KParts plugins
         *                     with the Category key == @p category. If @p
         *                     category is not set the Category key is ignored
         *                     and all plugins are shown.
         * @param config       The TDEConfig object that holds the state of the
         *                     plugins being enabled or not. By default it will
         *                     use TDEGlobal::config(). It is recommended to
         *                     always pass a TDEConfig object if you use
         *                     KSettings::PluginPage since you never know from where the
         *                     page will be called (think global config app).
         *                     For example KViewCanvas passes KSimpleConfig(
         *                     "kviewcanvas" ).
         */
        void addPlugins( const TQValueList<KPluginInfo*> & plugininfos,
                const TQString & catname = TQString::null,
                const TQString & category = TQString::null,
                TDEConfig * config = 0 );

        /**
         * Set whether the area for showing the KCMs of the plugins should be
         * hidden if the plugin doesn't have a KCM or whether the layout should
         * rather stay static and only an message should be shown.
         *
         * By default the config page is not hidden.
         */
        void setShowEmptyConfigPage( bool );

        /**
         * Load the state of the plugins (selected or not) from the KPluginInfo
         * objects. For KParts plugins everything should work automatically. For
         * your own type of plugins you might need to reimplement the
         * KPluginInfo::isPluginEnabled() method. If that doesn't fit your needs
         * you can also reimplement this method.
         */
        void load();

        /**
         * Save the configuration
         */
        void save();

        /**
         * Change to applications defaults
         */
        void defaults();

    signals:
        /**
         * Tells you whether the configuration is changed or not.
         */
        void changed( bool );

        /**
         * Emitted after the config of an embedded KCM has been saved. The
         * argument is the name of the parent component that needs to reload
         * its config
         */
        void configCommitted( const TQCString & instanceName );

    private:
        /**
         * return the KCM widgetstack
         *
         * @internal
         */
        TQWidgetStack * widgetStack();

        /**
         * Show an info page in the widgetstack.
         *
         * @internal
         */
        void configPage( int id );

        /**
         * @internal
         */
        void checkNeedForTabWidget();

        /**
         * @internal
         */
	void addPluginsInternal( const TQValueList<KPluginInfo*> plugininfos,
                                 const TQString & catname, const TQString & category,
                                 TDEConfigGroup* cfgGroup );

	class KPluginSelectorPrivate;
        KPluginSelectorPrivate * d;
};
#endif // KPLUGINSELECTOR_H

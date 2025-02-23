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

#ifndef KPLUGININFO_H
#define KPLUGININFO_H

#include <tqstring.h>
#include <tqmap.h>
#include <tqstringlist.h>
#include <tqvaluelist.h>
#include <kservice.h>

class TDEConfigGroup;

/**
 * @ingroup main
 * @ingroup plugin
 * Information about a plugin.
 *
 * This holds all the information about a plugin there is. It's used for the
 * user to decide whether he wants to use this plugin or not.
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class TDEUTILS_EXPORT KPluginInfo
{
    public:
        typedef TQValueList<KPluginInfo*> List;

        /**
         * Read plugin info from @p filename.
         *
         * The file should be of the following form:
         * \verbatim
           [Desktop Entry]
           Name=User Visible Name
           Comment=Description of what the plugin does

           [X-TDE Plugin Info]
           Author=Author's Name
           Email=author@foo.bar
           PluginName=internalname
           Version=1.1
           Website=http://www.plugin.org/
           Category=playlist
           Depends=plugin1,plugin3
           License=GPL
           EnabledByDefault=true
           \endverbatim
         * The first two entries in the "Desktop Entry" group always need to
         * be present.
         *
         * The "X-TDE-PluginInfo" keys you may add further entries which
         * will be available using property(). The Website,Category,Require
         * keys are optional.
         * For EnabledByDefault look at isPluginEnabledByDefault.
         *
         * @param filename  The filename of the .desktop file.
         * @param resource  If filename is relative, you need to specify a resource type
         * (e.g. "service", "apps"... TDEStandardDirs). Otherwise,
         * resource isn't used.
         */
        KPluginInfo( const TQString & filename, const char* resource = 0 );

        /**
         * Read plugin info from a KService object.
         *
         * The .desktop file should look like this:
         * \verbatim
           [Desktop Entry]
           Encoding=UTF-8
           Icon=mypluginicon
           Type=Service
           ServiceTypes=KPluginInfo

           X-TDE-PluginInfo-Author=Author's Name
           X-TDE-PluginInfo-Email=author@foo.bar
           X-TDE-PluginInfo-Name=internalname
           X-TDE-PluginInfo-Version=1.1
           X-TDE-PluginInfo-Website=http://www.plugin.org/
           X-TDE-PluginInfo-Category=playlist
           X-TDE-PluginInfo-Depends=plugin1,plugin3
           X-TDE-PluginInfo-License=GPL
           X-TDE-PluginInfo-EnabledByDefault=true

           Name=User Visible Name
           Comment=Description of what the plugin does
           \endverbatim
         * In the first three entries the Icon entry is optional.
         */
        KPluginInfo( const KService::Ptr service );

//X         /**
//X          * Create an empty hidden plugin.
//X          * @internal
//X          */
//X         KPluginInfo();

        virtual ~KPluginInfo();

        /**
         * @return A list of KPluginInfo objects constructed from a list of
         * KService objects. If you get a trader offer of the plugins you want
         * to use you can just pass them to this function.
         */
        static KPluginInfo::List fromServices( const KService::List & services, TDEConfig * config = 0, const TQString & group = TQString::null );

        /**
         * @return A list of KPluginInfo objects constructed from a list of
         * filenames. If you make a lookup using, for example,
         * TDEStandardDirs::findAllResources() you pass the list of files to this
         * function.
         */
        static KPluginInfo::List fromFiles( const TQStringList & files, TDEConfig * config = 0, const TQString & group = TQString::null );

        /**
         * @return A list of KPluginInfo objects for the KParts plugins of an
         * instance. You only need the name of the instance not a pointer to the
         * TDEInstance object.
         */
        static KPluginInfo::List fromKPartsInstanceName( const TQString &, TDEConfig * config = 0, const TQString & group = TQString::null );

        /**
         * @return Whether the plugin should be hidden.
         */
        bool isHidden() const;

        /**
         * Set whether the plugin is currently loaded.
         *
         * You might need to reimplement this method for special needs.
         *
         * @see isPluginEnabled()
         * @see save()
         */
        virtual void setPluginEnabled( bool enabled );

        /**
         * @return Whether the plugin is currently loaded.
         *
         * You might need to reimplement this method for special needs.
         *
         * @see setPluginEnabled()
         * @see load()
         */
        virtual bool isPluginEnabled() const;

        /**
         * @return The default value whether the plugin is enabled or not.
         * Defaults to the value set in the desktop file, or if that isn't set
         * to false.
         */
        bool isPluginEnabledByDefault() const;

        /**
         * @return The value associated the the @p key. You can use it if you
         *         want to read custom values. To do this you need to define
         *         your own servicetype and add it to the ServiceTypes keys.
         *
         * @see operator[]
         */
        TQVariant property( const TQString & key ) const;

        /**
         * This is the same as property(). It is provided for convenience.
         *
         * @return The value associated with the @p key.
         *
         * @see property()
         */
        TQVariant operator[]( const TQString & key ) const;

        /**
         * @return The user visible name of the plugin.
         */
        const TQString & name() const;

        /**
         * @return A comment describing the plugin.
         */
        const TQString & comment() const;

        /**
         * @return The iconname for this plugin
         */
        const TQString & icon() const;

        /**
         * @return The file containing the information about the plugin.
         */
        const TQString & specfile() const;

        /**
         * @return The author of this plugin.
         */
        const TQString & author() const;

        /**
         * @return The email address of the author.
         */
        const TQString & email() const;

        /**
         * @return The category of this plugin (e.g. playlist/skin).
         */
        const TQString & category() const;

        /**
         * @return The internal name of the plugin (for KParts Plugins this is
         * the same name as set in the .rc file).
         */
        const TQString & pluginName() const;

        /**
         * @return The version of the plugin.
         */
        const TQString & version() const;

        /**
         * @return The website of the plugin/author.
         */
        const TQString & website() const;


        /**
         * @return The license of this plugin.
         */
        const TQString & license() const;

        /**
         * @return A list of plugins required for this plugin to be enabled. Use
         *         the pluginName in this list.
         */
        const TQStringList & dependencies() const;

        /**
         * @return The KService object for this plugin. You might need it if you
         *         want to read custom values. To do this you need to define
         *         your own servicetype and add it to the ServiceTypes keys.
         *         Then you can use the KService::property() method to read your
         *         keys.
         *
         * @see property()
         */
        KService::Ptr service() const;

        /**
         * @return A list of Service pointers if the plugin installs one or more
         *         TDECModule
         */
        const TQValueList<KService::Ptr> & kcmServices() const;

        /**
         * Set the TDEConfigGroup to use for load()ing and save()ing the
         * configuration. This will be overridden by the TDEConfigGroup passed to
         * save() or load() (if one is passed).
         */
        void setConfig( TDEConfig * config, const TQString & group );

        /**
         * @return If the KPluginInfo object has a TDEConfig object set return
         * it, else return 0.
         */
        TDEConfig * config() const;

        /**
         * @return The groupname used in the TDEConfig object for load()ing and
         * save()ing whether the plugin is enabled.
         */
        const TQString & configgroup() const;

        /**
         * Save state of the plugin - enabled or not. This function is provided
         * for reimplementation if you need to save somewhere else.
         * @param config    The TDEConfigGroup holding the information whether
         *                  plugin is enabled.
         */
        virtual void save( TDEConfigGroup * config = 0 );

        /**
         * Load the state of the plugin - enabled or not. This function is provided
         * for reimplementation if you need to save somewhere else.
         * @param config    The TDEConfigGroup holding the information whether
         *                  plugin is enabled.
         */
        virtual void load( TDEConfigGroup * config = 0 );

        /**
         * Restore defaults (enabled or not).
         */
        virtual void defaults();

    private:
        KPluginInfo( const KPluginInfo & );
        const KPluginInfo & operator=( const KPluginInfo & );

        class KPluginInfoPrivate;
        KPluginInfoPrivate * d;
};
#endif // KPLUGININFO_H

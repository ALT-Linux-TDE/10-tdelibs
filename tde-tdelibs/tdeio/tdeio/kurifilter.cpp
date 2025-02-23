/* This file is part of the KDE libraries
 *  Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
 *  Copyright (C) 2000 Dawit Alemayehu <adawit at kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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

#include <config.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <ktrader.h>
#include <kmimetype.h>
#include <klibloader.h>
#include <kstaticdeleter.h>
#include <tdeparts/componentfactory.h>

#ifdef HAVE_ELFICON
#include <tqimage.h>
#include "tdelficon.h"
#endif // HAVE_ELFICON

#include "kurifilter.h"

template class TQPtrList<KURIFilterPlugin>;

KURIFilterPlugin::KURIFilterPlugin( TQObject *parent, const char *name, double pri )
                 :TQObject( parent, name )
{
    m_strName = TQString::fromLatin1( name );
    m_dblPriority = pri;
}

void KURIFilterPlugin::setFilteredURI( KURIFilterData& data, const KURL& uri ) const
{
    if ( data.uri() != uri )
    {
        data.m_pURI = uri;
        data.m_bChanged = true;
    }
}

class KURIFilterDataPrivate
{
public:
    KURIFilterDataPrivate() {};
    TQString abs_path;
    TQString args;
    TQString typedString;
};

KURIFilterData::KURIFilterData( const KURIFilterData& data )
{
    m_iType = data.m_iType;
    m_pURI = data.m_pURI;
    m_strErrMsg = data.m_strErrMsg;
    m_strIconName = data.m_strIconName;
    m_bChanged = data.m_bChanged;
    m_bCheckForExecutables = data.m_bCheckForExecutables;
    d = new KURIFilterDataPrivate;
    d->abs_path = data.absolutePath();
    d->typedString = data.typedString();
    d->args = data.argsAndOptions();
}

KURIFilterData::~KURIFilterData()
{
    delete d;
    d = 0;
}

void KURIFilterData::init( const KURL& url )
{
    m_iType = KURIFilterData::UNKNOWN;
    m_pURI = url;
    m_strErrMsg = TQString::null;
    m_strIconName = TQString::null;
    m_bCheckForExecutables = true;
    m_bChanged = true;
    d = new KURIFilterDataPrivate;
    d->typedString = url.url();
}

void KURIFilterData::init( const TQString& url )
{
    m_iType = KURIFilterData::UNKNOWN;
    m_pURI = url;
    m_strErrMsg = TQString::null;
    m_strIconName = TQString::null;
    m_bCheckForExecutables = true;
    m_bChanged = true;
    d = new KURIFilterDataPrivate;
    d->typedString = url;
}

void KURIFilterData::reinit(const KURL &url)
{
    delete d;
    init(url);
}

void KURIFilterData::reinit(const TQString &url)
{
    delete d;
    init(url);
}

TQString KURIFilterData::typedString() const
{
    return d->typedString;
}

void KURIFilterData::setCheckForExecutables( bool check )
{
    m_bCheckForExecutables = check;
}

bool KURIFilterData::hasArgsAndOptions() const
{
    return !d->args.isEmpty();
}

bool KURIFilterData::hasAbsolutePath() const
{
    return !d->abs_path.isEmpty();
}

bool KURIFilterData::setAbsolutePath( const TQString& absPath )
{
    // Since a malformed URL could possibly be a relative
    // URL we tag it as a possible local resource...
    if( (!m_pURI.isValid() || m_pURI.isLocalFile()) )
    {
        d->abs_path = absPath;
        return true;
    }
    return false;
}

TQString KURIFilterData::absolutePath() const
{
    return d->abs_path;
}

TQString KURIFilterData::argsAndOptions() const
{
    return d->args;
}

TQString KURIFilterData::iconName()
{
    if( m_bChanged )
    {
        m_customIconPixmap = TQPixmap();
        switch ( m_iType )
        {
            case KURIFilterData::LOCAL_FILE:
            case KURIFilterData::LOCAL_DIR:
            case KURIFilterData::NET_PROTOCOL:
            {
                m_strIconName = KMimeType::iconForURL( m_pURI );
                break;
            }
            case KURIFilterData::EXECUTABLE:
            {
                TQString exeName = m_pURI.url();
                exeName = exeName.mid( exeName.findRev( '/' ) + 1 ); // strip path if given
                KService::Ptr service = KService::serviceByDesktopName( exeName );
#ifndef HAVE_ELFICON
                // Try to find an icon with the same name as the binary (useful for non-tde apps)
                // FIXME: We should only do this if the binary is in the system path somewhere,
                // otherwise TDE could end up showing system icons for user binaries
                if (service && service->icon() != TQString::fromLatin1( "unknown" )) {
                    m_strIconName = service->icon();
                }
                else if ( !TDEGlobal::iconLoader()->loadIcon( exeName, TDEIcon::NoGroup, 16, TDEIcon::DefaultState, 0, true ).isNull() ) {
                    m_strIconName = exeName;
                }
                else {
                    // use default
                    m_strIconName = TQString::fromLatin1("application-x-executable");
                }
#else // HAVE_ELFICON
		// Try to find an icon with the same name as the binary (useful for non-tde apps)
		// FIXME: We should only do this if the binary is in the system path somewhere,
		// otherwise TDE could end up showing system icons for user binaries
		if (service && service->icon() != TQString::fromLatin1( "unknown" )) {
			m_strIconName = service->icon();
		}
		else if ( !TDEGlobal::iconLoader()->loadIcon( exeName, TDEIcon::NoGroup, 16, TDEIcon::DefaultState, 0, true ).isNull() ) {
			m_strIconName = exeName;
		}
		else {
			// use default
			m_strIconName = TQString::fromLatin1("application-x-executable");
		}
		// Try to load from elf file (if supported)
		// Check for an embedded icon
		unsigned int icon_size;
		libr_icon *icon = NULL;
		libr_file *handle = NULL;
		libr_access_t access = LIBR_READ;
		char libr_can_continue = 1;

		if((handle = libr_open(const_cast<char*>(m_pURI.path().ascii()), access)) == NULL)
		{
			kdWarning() << "failed to open file " << m_pURI.path() << endl;
			libr_can_continue = 0;
		}

		if (libr_can_continue == 1) {
			icon_size = 32;	// FIXME: Is this a reasonable size request for all possible usages of kurifilter?
			icon = libr_icon_geticon_bysize(handle, icon_size);

			if (libr_can_continue == 1) {
				// See if the embedded icon name matches any icon file names already on the system
				// If it does, use the system icon instead of the embedded one
				int iconresnamefound = 0;
				iconentry *entry = NULL;
				iconlist icons;
				if(!get_iconlist(handle, &icons))
				{
					// Failed to obtain a list of ELF icons
					kdDebug() << "failed to obtain ELF icon from " << m_pURI.path() << ": " << libr_errmsg() << endl;

					// See if there is a system icon we can use
					TQString sysIconName = elf_get_resource(handle, ".metadata_sysicon");
					if (!sysIconName.isEmpty()) {
						if (TDEGlobal::iconLoader()->iconPath(sysIconName.ascii(), 0, true) != "") {
							m_strIconName = sysIconName;
						}
					}

					libr_close(handle);
					libr_can_continue = 0;
				}
				else {
					while((entry = get_nexticon(&icons, entry)) != NULL)
					{
						if(icon == NULL)
						{
							// Try loading this icon as fallback
							icon = libr_icon_geticon_byname(handle, entry->name);
						}
						if (TDEGlobal::iconLoader()->iconPath(entry->name, 0, true) != "") {
							iconresnamefound = 1;
							m_strIconName = entry->name;
							break;
						}
					}
				}

				if (libr_can_continue == 1) {
					if ((iconresnamefound == 0) && (icon)) {
						// Extract the embedded icon
						size_t icon_data_length;
						char* icondata = libr_icon_malloc(icon, &icon_data_length);
						m_customIconPixmap.loadFromData(static_cast<uchar*>(static_cast<void*>(icondata)), icon_data_length);	// EVIL CAST
						if (icon_size != 0) {
							TQImage ip = m_customIconPixmap.convertToImage();
							ip = ip.smoothScale(icon_size, icon_size);
							m_customIconPixmap.convertFromImage(ip);
						}
						free(icondata);
						libr_icon_close(icon);
					}

					libr_close(handle);
				}
			}
		}
#endif // HAVE_ELFICON
                break;
            }
            case KURIFilterData::HELP:
            {
                m_strIconName = TQString::fromLatin1("khelpcenter");
                break;
            }
            case KURIFilterData::SHELL:
            {
                m_strIconName = TQString::fromLatin1("konsole");
                break;
            }
            case KURIFilterData::ERROR:
            case KURIFilterData::BLOCKED:
            {
                m_strIconName = TQString::fromLatin1("error");
                break;
            }
            default:
                m_strIconName = TQString::null;
                break;
        }
        m_bChanged = false;
    }
    return m_strIconName;
}

TQPixmap KURIFilterData::customIconPixmap()
{
    return m_customIconPixmap;
}

//********************************************  KURIFilterPlugin **********************************************
void KURIFilterPlugin::setArguments( KURIFilterData& data, const TQString& args ) const
{
    data.d->args = args;
}

//********************************************  KURIFilter **********************************************
KURIFilter *KURIFilter::s_self;
static KStaticDeleter<KURIFilter> kurifiltersd;

KURIFilter *KURIFilter::self()
{
    if (!s_self)
        s_self = kurifiltersd.setObject(s_self, new KURIFilter);
    return s_self;
}

KURIFilter::KURIFilter()
{
    m_lstPlugins.setAutoDelete(true);
    loadPlugins();
}

KURIFilter::~KURIFilter()
{
}

bool KURIFilter::filterURI( KURIFilterData& data, const TQStringList& filters )
{
    bool filtered = false;
    KURIFilterPluginList use_plugins;

    // If we have a filter list, only include the once
    // explicitly specified by it. Otherwise, use all available filters...
    if( filters.isEmpty() )
        use_plugins = m_lstPlugins;  // Use everything that is loaded...
    else
    {
        //kdDebug() << "Named plugins requested..."  << endl;
        for( TQStringList::ConstIterator lst = filters.begin(); lst != filters.end(); ++lst )
        {
            TQPtrListIterator<KURIFilterPlugin> it( m_lstPlugins );
            for( ; it.current() ; ++it )
            {
                if( (*lst) == it.current()->name() )
                {
                    //kdDebug() << "Will use filter plugin named: " << it.current()->name() << endl;
                    use_plugins.append( it.current() );
                    break;  // We already found it ; so lets test the next named filter...
                }
            }
        }
    }

    TQPtrListIterator<KURIFilterPlugin> it( use_plugins );
    //kdDebug() << "Using " << use_plugins.count() << " out of the "
    //          << m_lstPlugins.count() << " available plugins" << endl;
    for (; it.current() && !filtered; ++it)
    {
        //kdDebug() << "Using a filter plugin named: " << it.current()->name() << endl;
        filtered |= it.current()->filterURI( data );
    }
    return filtered;
}

bool KURIFilter::filterURI( KURL& uri, const TQStringList& filters )
{
    KURIFilterData data = uri;
    bool filtered = filterURI( data, filters );
    if( filtered ) uri = data.uri();
    return filtered;
}

bool KURIFilter::filterURI( TQString& uri, const TQStringList& filters )
{
    KURIFilterData data = uri;
    bool filtered = filterURI( data, filters );
    if( filtered )  uri = data.uri().url();
    return filtered;

}

KURL KURIFilter::filteredURI( const KURL &uri, const TQStringList& filters )
{
    KURIFilterData data = uri;
    filterURI( data, filters );
    return data.uri();
}

TQString KURIFilter::filteredURI( const TQString &uri, const TQStringList& filters )
{
    KURIFilterData data = uri;
    filterURI( data, filters );
    return data.uri().url();
}

TQPtrListIterator<KURIFilterPlugin> KURIFilter::pluginsIterator() const
{
    return TQPtrListIterator<KURIFilterPlugin>(m_lstPlugins);
}

TQStringList KURIFilter::pluginNames() const
{
    TQStringList list;
    for(TQPtrListIterator<KURIFilterPlugin> i = pluginsIterator(); *i; ++i)
        list.append((*i)->name());
    return list;
}

void KURIFilter::loadPlugins()
{
    TDETrader::OfferList offers = TDETrader::self()->query( "KURIFilter/Plugin" );

    TDETrader::OfferList::ConstIterator it = offers.begin();
    TDETrader::OfferList::ConstIterator end = offers.end();

    for (; it != end; ++it )
    {
      KURIFilterPlugin *plugin = KParts::ComponentFactory::createInstanceFromService<KURIFilterPlugin>( *it, 0, (*it)->desktopEntryName().latin1() );
      if ( plugin )
        m_lstPlugins.append( plugin );
    }

    // NOTE: Plugin priority is now determined by
    // the entry in the .desktop files...
    // TODO: Config dialog to differentiate "system"
    // plugins from "user-defined" ones...
    // m_lstPlugins.sort();
}

void KURIFilterPlugin::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kurifilter.moc"

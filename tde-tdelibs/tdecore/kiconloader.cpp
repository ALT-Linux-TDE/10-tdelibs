/*
 *
 * $Id$
 *
 * This file is part of the KDE project, module tdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * kiconloader.cpp: An icon loader for KDE with theming functionality.
 */

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqptrlist.h>
#include <tqintdict.h>
#include <tqpixmap.h>
#include <tqpixmapcache.h>
#include <tqimage.h>
#include <tqfileinfo.h>
#include <tqdir.h>
#include <tqiconset.h>
#include <tqmovie.h>
#include <tqbitmap.h>

#include <tdeapplication.h>
#include <kipc.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <ksimpleconfig.h>
#include <kinstance.h>

#include <kicontheme.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include <sys/types.h>
#include <stdlib.h>	//for abs
#include <unistd.h>     //for readlink
#include <dirent.h>
#include <config.h>
#include <assert.h>

#ifdef HAVE_LIBART
#include "svgicons/ksvgiconengine.h"
#include "svgicons/ksvgiconpainter.h"
#endif

#include <kimageeffect.h>

#include "kiconloader_p.h"

/*** TDEIconThemeNode: A node in the icon theme dependancy tree. ***/

TDEIconThemeNode::TDEIconThemeNode(TDEIconTheme *_theme)
{
    theme = _theme;
}

TDEIconThemeNode::~TDEIconThemeNode()
{
    delete theme;
}

void TDEIconThemeNode::printTree(TQString& dbgString) const
{
    /* This method doesn't have much sense anymore, so maybe it should
       be removed in the (near?) future */
    dbgString += "(";
    dbgString += theme->name();
    dbgString += ")";
}

void TDEIconThemeNode::queryIcons(TQStringList *result,
				int size, TDEIcon::Context context) const
{
    // add the icons of this theme to it
    *result += theme->queryIcons(size, context);
}

void TDEIconThemeNode::queryIconsByContext(TQStringList *result,
				int size, TDEIcon::Context context) const
{
    // add the icons of this theme to it
    *result += theme->queryIconsByContext(size, context);
}

TDEIcon TDEIconThemeNode::findIcon(const TQString& name, int size,
			       TDEIcon::MatchType match) const
{
    return theme->iconPath(name, size, match);
}


/*** TDEIconGroup: Icon type description. ***/

struct TDEIconGroup
{
    int size;
    bool dblPixels;
    bool alphaBlending;
};

// WARNING
// Enabling this in production will cause a massive slowdown of (and a related memory leak in)
// any application that creates and destroys large numbers of TDEIconLoader instances
//#define TDEICONLOADER_DEBUG

#ifdef TDEICONLOADER_DEBUG
// Keep a list of recently created and destroyed TDEIconLoader instances in order
// to detect bugs like #68528.
struct TDEIconLoaderDebug
    {
    TDEIconLoaderDebug( TDEIconLoader* l, const TQString& a )
        : loader( l ), appname( a ), valid( true )
        {}
    TDEIconLoaderDebug() {}; // this TQValueList feature annoys me
    TDEIconLoader* loader;
    TQString appname;
    bool valid;
    TQString delete_bt;
    };

static TQValueList< TDEIconLoaderDebug > *kiconloaders;
#endif

/*** TDEIconLoader: the icon loader ***/

TDEIconLoader::TDEIconLoader(const TQString& _appname, TDEStandardDirs *_dirs)
{
#ifdef TDEICONLOADER_DEBUG
    if( kiconloaders == NULL )
        kiconloaders = new TQValueList< TDEIconLoaderDebug>();
    // check for the (very unlikely case) that new TDEIconLoader gets allocated
    // at exactly same address like some previous one
    for( TQValueList< TDEIconLoaderDebug >::Iterator it = kiconloaders->begin();
         it != kiconloaders->end();
         )
        {
        if( (*it).loader == this )
            it = kiconloaders->remove( it );
        else
            ++it;
        }
    kiconloaders->append( TDEIconLoaderDebug( this, _appname ));
#endif
    d = new TDEIconLoaderPrivate;
    d->q = this;
    d->mpGroups = 0L;
    d->imgDict.setAutoDelete(true);
    d->links.setAutoDelete(true);

    if (kapp) {
        kapp->addKipcEventMask(KIPC::IconChanged);
        TQObject::connect(kapp, TQ_SIGNAL(updateIconLoaders()), d, TQ_SLOT(reconfigure()));
    }

    init( _appname, _dirs );
}

void TDEIconLoader::reconfigure( const TQString& _appname, TDEStandardDirs *_dirs )
{
    d->links.clear();
    d->imgDict.clear();
    d->mThemesInTree.clear();
    d->lastImage.reset();
    d->lastImageKey = TQString::null;
    delete [] d->mpGroups;

    init( _appname, _dirs );
}

void TDEIconLoader::init( const TQString& _appname, TDEStandardDirs *_dirs )
{
    // If this is unequal to 0, the iconloader is initialized
    // successfully.
    d->mpThemeRoot = 0L;

    d->appname = _appname;
    d->extraDesktopIconsLoaded = false;
    d->delayedLoading = false;

    if (_dirs)
        d->mpDirs = _dirs;
    else
        d->mpDirs = TDEGlobal::dirs();

    TQString appname = _appname;
    if (appname.isEmpty())
        appname = TDEGlobal::instance()->instanceName();

    // Add the default theme and its base themes to the theme tree
    TDEIconTheme *def = new TDEIconTheme(TDEIconTheme::current(), appname);
    if (!def->isValid())
    {
        delete def;
        // warn, as this is actually a small penalty hit
        kdDebug(264) << "Couldn't find current icon theme, falling back to default." << endl;
	def = new TDEIconTheme(TDEIconTheme::defaultThemeName(), appname);
        if (!def->isValid())
        {
            kdError(264) << "Error: standard icon theme"
                         << " \"" << TDEIconTheme::defaultThemeName() << "\" "
                         << " not found!" << endl;
            d->mpGroups=0L;
            return;
        }
    }
    d->mpThemeRoot = new TDEIconThemeNode(def);
    d->links.append(d->mpThemeRoot);
    d->mThemesInTree += TDEIconTheme::current();
    addBaseThemes(d->mpThemeRoot, appname);

    // These have to match the order in kicontheme.h
    static const char * const groups[] = { "Desktop", "Toolbar", "MainToolbar", "Small", "Panel", 0L };
    TDEConfig *config = TDEGlobal::config();
    TDEConfigGroupSaver cs(config, "dummy");

    // loading config and default sizes
    d->mpGroups = new TDEIconGroup[(int) TDEIcon::LastGroup];
    for (TDEIcon::Group i=TDEIcon::FirstGroup; i<TDEIcon::LastGroup; i++)
    {
	if (groups[i] == 0L)
	    break;
	config->setGroup(TQString::fromLatin1(groups[i]) + "Icons");
	d->mpGroups[i].size = config->readNumEntry("Size", 0);
	d->mpGroups[i].dblPixels = config->readBoolEntry("DoublePixels", false);
	if (TQPixmap::defaultDepth()>8)
	    d->mpGroups[i].alphaBlending = config->readBoolEntry("AlphaBlending", true);
	else
	    d->mpGroups[i].alphaBlending = false;

	if (!d->mpGroups[i].size)
	    d->mpGroups[i].size = d->mpThemeRoot->theme->defaultSize(i);
    }

    // Insert application specific themes at the top.
    d->mpDirs->addResourceType("appicon", TDEStandardDirs::kde_default("data") +
		appname + "/pics/");
    // ################## KDE4: consider removing the toolbar directory
    d->mpDirs->addResourceType("appicon", TDEStandardDirs::kde_default("data") +
		appname + "/toolbar/");

    // Add legacy icon dirs.
    TQStringList dirs;
    dirs += d->mpDirs->resourceDirs("icon");
    dirs += d->mpDirs->resourceDirs("pixmap");
    dirs += d->mpDirs->resourceDirs("xdgdata-icon");
    dirs += "/usr/share/pixmaps";
    // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
    dirs += d->mpDirs->resourceDirs("xdgdata-pixmap");
    for (TQStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it)
	d->mpDirs->addResourceDir("appicon", *it);

#ifndef NDEBUG
    TQString dbgString = "Theme tree: ";
    d->mpThemeRoot->printTree(dbgString);
    kdDebug(264) << dbgString << endl;
#endif
}

TDEIconLoader::~TDEIconLoader()
{
#ifdef TDEICONLOADER_DEBUG
    for( TQValueList< TDEIconLoaderDebug >::Iterator it = kiconloaders->begin();
         it != kiconloaders->end();
         ++it )
        {
        if( (*it).loader == this )
            {
            (*it).valid = false;
            (*it).delete_bt = kdBacktrace();
            break;
            }
        }
#endif
    /* antlarr: There's no need to delete d->mpThemeRoot as it's already
       deleted when the elements of d->links are deleted */
    d->mpThemeRoot=0;
    delete[] d->mpGroups;
    delete d;
}

void TDEIconLoader::enableDelayedIconSetLoading( bool enable )
{
    d->delayedLoading = enable;
}

bool TDEIconLoader::isDelayedIconSetLoadingEnabled() const
{
    return d->delayedLoading;
}

void TDEIconLoader::addAppDir(const TQString& appname)
{
    d->mpDirs->addResourceType("appicon", TDEStandardDirs::kde_default("data") +
		appname + "/pics/");
    // ################## KDE4: consider removing the toolbar directory
    d->mpDirs->addResourceType("appicon", TDEStandardDirs::kde_default("data") +
		appname + "/toolbar/");
    addAppThemes(appname);
}

void TDEIconLoader::addAppThemes(const TQString& appname)
{
    if ( TDEIconTheme::current() != TDEIconTheme::defaultThemeName() )
    {
        TDEIconTheme *def = new TDEIconTheme(TDEIconTheme::current(), appname);
        if (def->isValid())
        {
            TDEIconThemeNode* node = new TDEIconThemeNode(def);
            d->links.append(node);
            addBaseThemes(node, appname);
        }
        else
            delete def;
    }

    TDEIconTheme *def = new TDEIconTheme(TDEIconTheme::defaultThemeName(), appname);
    TDEIconThemeNode* node = new TDEIconThemeNode(def);
    d->links.append(node);
    addBaseThemes(node, appname);
}

void TDEIconLoader::addBaseThemes(TDEIconThemeNode *node, const TQString &appname)
{
    TQStringList lst = node->theme->inherits();
    TQStringList::ConstIterator it;

    for (it=lst.begin(); it!=lst.end(); ++it)
    {
	if( d->mThemesInTree.contains(*it) && (*it) != "hicolor")
	    continue;
	TDEIconTheme *theme = new TDEIconTheme(*it,appname);
	if (!theme->isValid()) {
	    delete theme;
	    continue;
	}
        TDEIconThemeNode *n = new TDEIconThemeNode(theme);
	d->mThemesInTree.append(*it);
	d->links.append(n);
	addBaseThemes(n, appname);
    }
}

void TDEIconLoader::addExtraDesktopThemes()
{
    if ( d->extraDesktopIconsLoaded ) return;

    TQStringList list;
    TQStringList icnlibs = TDEGlobal::dirs()->resourceDirs("icon");
    TQStringList::ConstIterator it;
    char buf[1000];
    int r;
    for (it=icnlibs.begin(); it!=icnlibs.end(); ++it)
    {
	TQDir dir(*it);
	if (!dir.exists())
	    continue;
	TQStringList lst = dir.entryList("default.*", TQDir::Dirs);
	TQStringList::ConstIterator it2;
	for (it2=lst.begin(); it2!=lst.end(); ++it2)
	{
	    if (!TDEStandardDirs::exists(*it + *it2 + "/index.desktop")
		&& !TDEStandardDirs::exists(*it + *it2 + "/index.theme"))
		continue;
	    r=readlink( TQFile::encodeName(*it + *it2) , buf, sizeof(buf)-1);
	    if ( r>0 )
	    {
	      buf[r]=0;
	      TQDir dir2( buf );
	      TQString themeName=dir2.dirName();

	      if (!list.contains(themeName))
		list.append(themeName);
	    }
	}
    }

    for (it=list.begin(); it!=list.end(); ++it)
    {
	if ( d->mThemesInTree.contains(*it) )
		continue;
	if ( *it == TQString("default.tde") ) continue;

	TDEIconTheme *def = new TDEIconTheme( *it, "" );
	TDEIconThemeNode* node = new TDEIconThemeNode(def);
	d->mThemesInTree.append(*it);
	d->links.append(node);
	addBaseThemes(node, "" );
    }

    d->extraDesktopIconsLoaded=true;

}

bool TDEIconLoader::extraDesktopThemesAdded() const
{
    return d->extraDesktopIconsLoaded;
}

TQString TDEIconLoader::removeIconExtension(const TQString &name) const
{
    int extensionLength=0;

    TQString ext = name.right(4);

    static const TQString &png_ext = TDEGlobal::staticQString(".png");
    static const TQString &xpm_ext = TDEGlobal::staticQString(".xpm");
    if (ext == png_ext || ext == xpm_ext)
      extensionLength=4;
#ifdef HAVE_LIBART
    else
    {
	static const TQString &svgz_ext = TDEGlobal::staticQString(".svgz");
	static const TQString &svg_ext = TDEGlobal::staticQString(".svg");

	if (name.right(5) == svgz_ext)
	    extensionLength=5;
	else if (ext == svg_ext)
	    extensionLength=4;
    }
#endif

    if ( extensionLength > 0 )
    {
	return name.left(name.length() - extensionLength);
    }
    return name;
}

TQString TDEIconLoader::removeIconExtensionInternal(const TQString &name) const
{
    TQString name_noext = removeIconExtension(name);

#ifndef NDEBUG
    if (name != name_noext)
    {
	kdDebug(264) << "Application " << TDEGlobal::instance()->instanceName()
		     << " loads icon " << name << " with extension." << endl;
    }
#endif

    return name_noext;
}

TDEIcon TDEIconLoader::findMatchingIcon(const TQString& name, int size) const
{
    TDEIcon icon;

    const TQString *ext[4];
    int count=0;
    static const TQString &png_ext = TDEGlobal::staticQString(".png");
    ext[count++]=&png_ext;
#ifdef HAVE_LIBART
    static const TQString &svgz_ext = TDEGlobal::staticQString(".svgz");
    ext[count++]=&svgz_ext;
    static const TQString &svg_ext = TDEGlobal::staticQString(".svg");
    ext[count++]=&svg_ext;
#endif
    static const TQString &xpm_ext = TDEGlobal::staticQString(".xpm");
    ext[count++]=&xpm_ext;

    /* JRT: To follow the XDG spec, the order in which we look for an
       icon 1s:

       png, svgz, svg, xpm exact match
       png, svgz, svg, xpm best match
       next theme in inheritance tree : png, svgz, svg, xpm exact match
                                        png, svgz, svg, xpm best match
       next theme in inheritance tree : png, svgz, svg, xpm exact match
                                        png, svgz, svg, xpm best match
       and so on

       */
    for ( TDEIconThemeNode *themeNode = d->links.first() ; themeNode ;
	themeNode = d->links.next() )
    {
	for (int i = 0 ; i < count ; i++)
	{
	    icon = themeNode->theme->iconPath(name + *ext[i], size, TDEIcon::MatchExact);
	    if (icon.isValid()) goto icon_found ;
	}

	for (int i = 0 ; i < count ; i++)
	{
	    icon = themeNode->theme->iconPath(name + *ext[i], size, TDEIcon::MatchBest);
	    if (icon.isValid()) goto icon_found;
	}
    }
    icon_found:
    return icon;
}

inline TQString TDEIconLoader::unknownIconPath( int size ) const
{
    static const TQString &str_unknown = TDEGlobal::staticQString("unknown");

    TDEIcon icon = findMatchingIcon(str_unknown, size);
    if (!icon.isValid())
    {
        kdDebug(264) << "Warning: could not find \"Unknown\" icon for size = "
                     << size << endl;
        return TQString::null;
    }
    return icon.path;
}

// Finds the absolute path to an icon.

TQString TDEIconLoader::iconPath(const TQString& _name, int group_or_size,
			      bool canReturnNull) const
{
    if (d->mpThemeRoot == 0L)
	return TQString::null;

    if (!TQDir::isRelativePath(_name))
	return _name;

    TQString name = removeIconExtensionInternal( _name );

    TQString path;
    if (group_or_size == TDEIcon::User)
    {
	static const TQString &png_ext = TDEGlobal::staticQString(".png");
	static const TQString &xpm_ext = TDEGlobal::staticQString(".xpm");
	path = d->mpDirs->findResource("appicon", name + png_ext);

#ifdef HAVE_LIBART
	static const TQString &svgz_ext = TDEGlobal::staticQString(".svgz");
	static const TQString &svg_ext = TDEGlobal::staticQString(".svg");
	if (path.isEmpty())
	    path = d->mpDirs->findResource("appicon", name + svgz_ext);
	if (path.isEmpty())
	   path = d->mpDirs->findResource("appicon", name + svg_ext);
#endif
	if (path.isEmpty())
	     path = d->mpDirs->findResource("appicon", name + xpm_ext);
	return path;
    }

    if (group_or_size >= TDEIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << endl;
	return path;
    }

    int size;
    if (group_or_size >= 0)
	size = d->mpGroups[group_or_size].size;
    else
	size = -group_or_size;

    if (_name.isEmpty()) {
        if (canReturnNull)
            return TQString::null;
        else
            return unknownIconPath(size);
    }

    TDEIcon icon = findMatchingIcon(name, size);

    if (!icon.isValid())
    {
	// Try "User" group too.
	path = iconPath(name, TDEIcon::User, true);
	if (!path.isEmpty() || canReturnNull)
	    return path;

	if (canReturnNull)
	    return TQString::null;
        else
            return unknownIconPath(size);
    }
    return icon.path;
}

TQPixmap TDEIconLoader::loadIcon(const TQString& _name, TDEIcon::Group group, int size,
                              int state, TQString *path_store, bool canReturnNull) const
{
    TQString name = _name;
    TQPixmap pix;
    TQString key;
    bool absolutePath=false, favIconOverlay=false;

    if (d->mpThemeRoot == 0L)
	return pix;

    // Special case for absolute path icons.
    if (name.startsWith("favicons/"))
    {
       favIconOverlay = true;
       name = locateLocal("cache", name+".png");
    }
    if (!TQDir::isRelativePath(name)) absolutePath=true;

    static const TQString &str_unknown = TDEGlobal::staticQString("unknown");

    // Special case for "User" icons.
    if (group == TDEIcon::User)
    {
	key = "$kicou_";
        key += TQString::number(size); key += '_';
	key += name;
	bool inCache = TQPixmapCache::find(key, pix);
	if (inCache && (path_store == 0L))
	    return pix;

	TQString path = (absolutePath) ? name :
			iconPath(name, TDEIcon::User, canReturnNull);
	if (path.isEmpty())
	{
	    if (canReturnNull)
		return pix;
	    // We don't know the desired size: use small
	    path = iconPath(str_unknown, TDEIcon::Small, true);
	    if (path.isEmpty())
	    {
		kdDebug(264) << "Warning: Cannot find \"unknown\" icon." << endl;
		return pix;
	    }
	}

	if (path_store != 0L)
	    *path_store = path;
	if (inCache)
	    return pix;
	TQImage img(path);
	if (size != 0)
	    img=img.smoothScale(size,size);

	pix.convertFromImage(img);
	TQPixmapCache::insert(key, pix);
	return pix;
    }

    // Regular case: Check parameters

    if ((group < -1) || (group >= TDEIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << endl;
	group = TDEIcon::Desktop;
    }

    int overlay = (state & TDEIcon::OverlayMask);
    state &= ~TDEIcon::OverlayMask;
    if ((state < 0) || (state >= TDEIcon::LastState))
    {
	kdDebug(264) << "Illegal icon state: " << state << endl;
	state = TDEIcon::DefaultState;
    }

    if (size == 0 && group < 0)
    {
	kdDebug(264) << "Neither size nor group specified!" << endl;
	group = TDEIcon::Desktop;
    }

    if (!absolutePath)
    {
        if (!canReturnNull && name.isEmpty())
            name = str_unknown;
        else
	    name = removeIconExtensionInternal(name);
    }

    // If size == 0, use default size for the specified group.
    if (size == 0)
    {
	size = d->mpGroups[group].size;
    }
    favIconOverlay = favIconOverlay && size > 22;

    // Generate a unique cache key for the icon.

    key = "$kico_";
    key += name; key += '_';
    key += TQString::number(size); key += '_';

    TQString overlayStr = TQString::number( overlay );

    TQString noEffectKey = key + '_' + overlayStr;

    if (group >= 0)
    {
	key += d->mpEffect.fingerprint(group, state);
	if (d->mpGroups[group].dblPixels)
	    key += TQString::fromLatin1(":dblsize");
    } else
	key += TQString::fromLatin1("noeffect");
    key += '_';
    key += overlayStr;

    // Is the icon in the cache?
    bool inCache = TQPixmapCache::find(key, pix);
    if (inCache && (path_store == 0L))
	return pix;

    TQImage *img = 0;
    int iconType;
    int iconThreshold;

    if ( ( path_store != 0L ) ||
         noEffectKey != d->lastImageKey )
    {
        // No? load it.
        TDEIcon icon;
        if (absolutePath && !favIconOverlay)
        {
            icon.context=TDEIcon::Any;
            icon.type=TDEIcon::Scalable;
            icon.path=name;
        }
        else
        {
            if (!name.isEmpty())
                icon = findMatchingIcon(favIconOverlay ? TQString("www") : name, size);

            if (!icon.isValid())
            {
                // Try "User" icon too. Some apps expect this.
                if (!name.isEmpty())
                    pix = loadIcon(name, TDEIcon::User, size, state, path_store, true);
                if (!pix.isNull() || canReturnNull) {
                    TQPixmapCache::insert(key, pix);
                    return pix;
                }

                icon = findMatchingIcon(str_unknown, size);
                if (!icon.isValid())
                {
                    kdDebug(264)
                        << "Warning: could not find \"Unknown\" icon for size = "
                        << size << endl;
                    return pix;
                }
            }
        }

        if (path_store != 0L)
            *path_store = icon.path;
        if (inCache)
            return pix;

	// Use the extension as the format. Works for XPM and PNG, but not for SVG
	TQString ext = icon.path.right(3).upper();
	if(ext != "SVG" && ext != "VGZ")
	{
	    img = new TQImage(icon.path, ext.latin1());
	    if (img->isNull()) {
                delete img;
		return pix;
            }
	}
	else
	{
#ifdef HAVE_LIBART
	    // Special stuff for SVG icons
	    KSVGIconEngine *svgEngine = new KSVGIconEngine();

	    if(svgEngine->load(size, size, icon.path))
		img = svgEngine->painter()->image();
	    else
		img = new TQImage();

	    delete svgEngine;
#else
	    img = new TQImage();
#endif
	}

        iconType = icon.type;
        iconThreshold = icon.threshold;

        d->lastImage = img->copy();
        d->lastImageKey = noEffectKey;
        d->lastIconType = iconType;
        d->lastIconThreshold = iconThreshold;
    }
    else
    {
        img = new TQImage( d->lastImage.copy() );
        iconType = d->lastIconType;
        iconThreshold = d->lastIconThreshold;
    }

    // Blend in all overlays
    if (overlay)
    {
	TQImage *ovl;
	TDEIconTheme *theme = d->mpThemeRoot->theme;
	if ((overlay & TDEIcon::LockOverlay) &&
		((ovl = loadOverlay(theme->lockOverlay(), size)) != 0L))
	    TDEIconEffect::overlay(*img, *ovl);
	if ((overlay & TDEIcon::LinkOverlay) &&
		((ovl = loadOverlay(theme->linkOverlay(), size)) != 0L))
	    TDEIconEffect::overlay(*img, *ovl);
	if ((overlay & TDEIcon::ZipOverlay) &&
		((ovl = loadOverlay(theme->zipOverlay(), size)) != 0L))
	    TDEIconEffect::overlay(*img, *ovl);
	if ((overlay & TDEIcon::ShareOverlay) &&
	    ((ovl = loadOverlay(theme->shareOverlay(), size)) != 0L))
	  TDEIconEffect::overlay(*img, *ovl);
        if (overlay & TDEIcon::HiddenOverlay)
        {
	    if (img->depth() != 32)
	        *img = img->convertDepth(32);
            for (int y = 0; y < img->height(); y++)
            {
		TQRgb *line = reinterpret_cast<TQRgb *>(img->scanLine(y));
                for (int x = 0; x < img->width();  x++)
                    line[x] = (line[x] & 0x00ffffff) | (TQMIN(0x80, tqAlpha(line[x])) << 24);
	    }
	}
    }

    // Scale the icon and apply effects if necessary
    if (iconType == TDEIcon::Scalable && size != img->width())
    {
        *img = img->smoothScale(size, size);
    }
    if (iconType == TDEIcon::Threshold && size != img->width())
    {
	if ( abs(size-img->width())>iconThreshold )
	    *img = img->smoothScale(size, size);
    }
    if (group >= 0 && d->mpGroups[group].dblPixels)
    {
	*img = d->mpEffect.doublePixels(*img);
    }
    if (group >= 0)
    {
	*img = d->mpEffect.apply(*img, group, state);
    }

    if (favIconOverlay)
    {
        TQImage favIcon(name, "PNG");
        int x = img->width() - favIcon.width() - 1,
            y = img->height() - favIcon.height() - 1;
        if( favIcon.depth() != 32 )
            favIcon = favIcon.convertDepth( 32 );
        if( img->depth() != 32 )
            *img = img->convertDepth( 32 );
        for( int line = 0;
             line < favIcon.height();
             ++line )
        {
            TQRgb* fpos = reinterpret_cast< TQRgb* >( favIcon.scanLine( line ));
            TQRgb* ipos = reinterpret_cast< TQRgb* >( img->scanLine( line + y )) + x;
            for( int i = 0;
                 i < favIcon.width();
                 ++i, ++fpos, ++ipos )
                *ipos = tqRgba( ( tqRed( *ipos ) * ( 255 - tqAlpha( *fpos )) + tqRed( *fpos ) * tqAlpha( *fpos )) / 255,
                               ( tqGreen( *ipos ) * ( 255 - tqAlpha( *fpos )) + tqGreen( *fpos ) * tqAlpha( *fpos )) / 255,
                               ( tqBlue( *ipos ) * ( 255 - tqAlpha( *fpos )) + tqBlue( *fpos ) * tqAlpha( *fpos )) / 255,
                               ( tqAlpha( *ipos ) * ( 255 - tqAlpha( *fpos )) + tqAlpha( *fpos ) * tqAlpha( *fpos )) / 255 );
        }
    }

    if (TQPaintDevice::x11AppDepth() == 32) pix.convertFromImage(KImageEffect::convertToPremultipliedAlpha( *img ));
    else pix.convertFromImage(*img);

    delete img;

    TQPixmapCache::insert(key, pix);
    return pix;
}

TQImage *TDEIconLoader::loadOverlay(const TQString &name, int size) const
{
    TQString key = name + '_' + TQString::number(size);
    TQImage *image = d->imgDict.find(key);
    if (image != 0L)
	return image;

    TDEIcon icon = findMatchingIcon(name, size);
    if (!icon.isValid())
    {
	kdDebug(264) << "Overlay " << name << "not found." << endl;
	return 0L;
    }
    image = new TQImage(icon.path);
    // In some cases (since size in findMatchingIcon() is more a hint than a
    // constraint) image->size can be != size. If so perform rescaling.
    if ( size != image->width() )
        *image = image->smoothScale( size, size );
    d->imgDict.insert(key, image);
    return image;
}



TQMovie TDEIconLoader::loadMovie(const TQString& name, TDEIcon::Group group, int size) const
{
    TQString file = moviePath( name, group, size );
    if (file.isEmpty())
	return TQMovie();
    int dirLen = file.findRev('/');
    TQString icon = iconPath(name, size ? -size : group, true);
    if (!icon.isEmpty() && file.left(dirLen) != icon.left(dirLen))
	return TQMovie();
    return TQMovie(file);
}

TQString TDEIconLoader::moviePath(const TQString& name, TDEIcon::Group group, int size) const
{
    if (!d->mpGroups) return TQString::null;

    if ( (group < -1 || group >= TDEIcon::LastGroup) && group != TDEIcon::User )
    {
	kdDebug(264) << "Illegal icon group: " << group << endl;
	group = TDEIcon::Desktop;
    }
    if (size == 0 && group < 0)
    {
	kdDebug(264) << "Neither size nor group specified!" << endl;
	group = TDEIcon::Desktop;
    }

    TQString file = name + ".mng";
    if (group == TDEIcon::User)
    {
	file = d->mpDirs->findResource("appicon", file);
    }
    else
    {
	if (size == 0)
	    size = d->mpGroups[group].size;

        TDEIcon icon;

	for ( TDEIconThemeNode *themeNode = d->links.first() ; themeNode ;
		themeNode = d->links.next() )
	{
	    icon = themeNode->theme->iconPath(file, size, TDEIcon::MatchExact);
	    if (icon.isValid()) goto icon_found ;

		icon = themeNode->theme->iconPath(file, size, TDEIcon::MatchBest);
	    if (icon.isValid()) goto icon_found ;
	}

	icon_found:
	file = icon.isValid() ? icon.path : TQString::null;
    }
    return file;
}


TQStringList TDEIconLoader::loadAnimated(const TQString& name, TDEIcon::Group group, int size) const
{
    TQStringList lst;

    if (!d->mpGroups) return lst;

    if ((group < -1) || (group >= TDEIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << endl;
	group = TDEIcon::Desktop;
    }
    if ((size == 0) && (group < 0))
    {
	kdDebug(264) << "Neither size nor group specified!" << endl;
	group = TDEIcon::Desktop;
    }

    TQString file = name + "/0001";
    if (group == TDEIcon::User)
    {
	file = d->mpDirs->findResource("appicon", file + ".png");
    } else
    {
	if (size == 0)
	    size = d->mpGroups[group].size;
	TDEIcon icon = findMatchingIcon(file, size);
	file = icon.isValid() ? icon.path : TQString::null;

    }
    if (file.isEmpty())
	return lst;

    TQString path = file.left(file.length()-8);
    DIR* dp = opendir( TQFile::encodeName(path) );
    if(!dp)
        return lst;

    struct dirent* ep;
    while( ( ep = readdir( dp ) ) != 0L )
    {
        TQString fn(TQFile::decodeName(ep->d_name));
        if(!(fn.left(4)).toUInt())
            continue;

        lst += path + fn;
    }
    closedir ( dp );
    lst.sort();
    return lst;
}

TDEIconTheme *TDEIconLoader::theme() const
{
    if (d->mpThemeRoot) return d->mpThemeRoot->theme;
    return 0L;
}

int TDEIconLoader::currentSize(TDEIcon::Group group) const
{
    if (!d->mpGroups) return -1;

    if (group < 0 || group >= TDEIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group << endl;
	return -1;
    }
    return d->mpGroups[group].size;
}

TQStringList TDEIconLoader::queryIconsByDir( const TQString& iconsDir ) const
{
  TQDir dir(iconsDir);
  TQStringList lst = dir.entryList("*.png;*.xpm", TQDir::Files);
  TQStringList result;
  TQStringList::ConstIterator it;
  for (it=lst.begin(); it!=lst.end(); ++it)
    result += iconsDir + "/" + *it;
  return result;
}

TQStringList TDEIconLoader::queryIconsByContext(int group_or_size,
					    TDEIcon::Context context) const
{
    TQStringList result;
    if (group_or_size >= TDEIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << endl;
	return result;
    }
    int size;
    if (group_or_size >= 0)
	size = d->mpGroups[group_or_size].size;
    else
	size = -group_or_size;

    for ( TDEIconThemeNode *themeNode = d->links.first() ; themeNode ;
            themeNode = d->links.next() )
       themeNode->queryIconsByContext(&result, size, context);

    // Eliminate duplicate entries (same icon in different directories)
    TQString name;
    TQStringList res2, entries;
    TQStringList::ConstIterator it;
    for (it=result.begin(); it!=result.end(); ++it)
    {
	int n = (*it).findRev('/');
	if (n == -1)
	    name = *it;
	else
	    name = (*it).mid(n+1);
	name = removeIconExtension(name);
	if (!entries.contains(name))
	{
	    entries += name;
	    res2 += *it;
	}
    }
    return res2;

}

TQStringList TDEIconLoader::queryIcons(int group_or_size, TDEIcon::Context context) const
{
    TQStringList result;
    if (group_or_size >= TDEIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << endl;
	return result;
    }
    int size;
    if (group_or_size >= 0)
	size = d->mpGroups[group_or_size].size;
    else
	size = -group_or_size;

    for ( TDEIconThemeNode *themeNode = d->links.first() ; themeNode ;
            themeNode = d->links.next() )
       themeNode->queryIcons(&result, size, context);

    // Eliminate duplicate entries (same icon in different directories)
    TQString name;
    TQStringList res2, entries;
    TQStringList::ConstIterator it;
    for (it=result.begin(); it!=result.end(); ++it)
    {
	int n = (*it).findRev('/');
	if (n == -1)
	    name = *it;
	else
	    name = (*it).mid(n+1);
	name = removeIconExtension(name);
	if (!entries.contains(name))
	{
	    entries += name;
	    res2 += *it;
	}
    }
    return res2;
}

// used by TDEIconDialog to find out which contexts to offer in a combobox
bool TDEIconLoader::hasContext(TDEIcon::Context context) const
{
    for ( TDEIconThemeNode *themeNode = d->links.first() ; themeNode ;
            themeNode = d->links.next() )
       if( themeNode->theme->hasContext( context ))
           return true;
    return false;
}

TDEIconEffect * TDEIconLoader::iconEffect() const
{
    return &d->mpEffect;
}

bool TDEIconLoader::alphaBlending(TDEIcon::Group group) const
{
    if (!d->mpGroups) return false;

    if (group < 0 || group >= TDEIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group << endl;
	return false;
    }
    return d->mpGroups[group].alphaBlending;
}

TQIconSet TDEIconLoader::loadIconSet(const TQString& name, TDEIcon::Group group, int size, bool canReturnNull)
{
    return loadIconSet( name, group, size, canReturnNull, true );
}

TQIconSet TDEIconLoader::loadIconSet(const TQString& name, TDEIcon::Group group, int size)
{
    return loadIconSet( name, group, size, false );
}

/*** class for delayed icon loading for TQIconSet ***/

class TDEIconFactory
    : public TQIconFactory
    {
    public:
        TDEIconFactory( const TQString& iconName_P, TDEIcon::Group group_P,
            int size_P, TDEIconLoader* loader_P );
        TDEIconFactory( const TQString& iconName_P, TDEIcon::Group group_P,
            int size_P, TDEIconLoader* loader_P, bool canReturnNull );
        virtual TQPixmap* createPixmap( const TQIconSet&, TQIconSet::Size, TQIconSet::Mode, TQIconSet::State );
    private:
        TQString iconName;
        TDEIcon::Group group;
        int size;
        TDEIconLoader* loader;
        bool canReturnNull;
    };


TQIconSet TDEIconLoader::loadIconSet( const TQString& name, TDEIcon::Group g, int s,
    bool canReturnNull, bool immediateExistenceCheck)
{
    if ( !d->delayedLoading )
        return loadIconSetNonDelayed( name, g, s, canReturnNull );

    if (g < -1 || g > 6) {
        kdDebug() << "TDEIconLoader::loadIconSet " << name << " " << (int)g << " " << s << endl;
        tqDebug("%s", kdBacktrace().latin1());
        abort();
    }

    if(canReturnNull && immediateExistenceCheck)
    { // we need to find out if the icon actually exists
        TQPixmap pm = loadIcon( name, g, s, TDEIcon::DefaultState, NULL, true );
        if( pm.isNull())
            return TQIconSet();

        TQIconSet ret( pm );
        ret.installIconFactory( new TDEIconFactory( name, g, s, this ));
        return ret;
    }

    TQIconSet ret;
    ret.installIconFactory( new TDEIconFactory( name, g, s, this, canReturnNull ));
    return ret;
}

TQIconSet TDEIconLoader::loadIconSetNonDelayed( const TQString& name,
                                             TDEIcon::Group g,
                                             int s, bool canReturnNull )
{
    TQIconSet iconset;
    TQPixmap tmp = loadIcon(name, g, s, TDEIcon::ActiveState, NULL, canReturnNull);
    iconset.setPixmap( tmp, TQIconSet::Small, TQIconSet::Active );
    // we don't use QIconSet's resizing anyway
    iconset.setPixmap( tmp, TQIconSet::Large, TQIconSet::Active );
    tmp = loadIcon(name, g, s, TDEIcon::DisabledState, NULL, canReturnNull);
    iconset.setPixmap( tmp, TQIconSet::Small, TQIconSet::Disabled );
    iconset.setPixmap( tmp, TQIconSet::Large, TQIconSet::Disabled );
    tmp = loadIcon(name, g, s, TDEIcon::DefaultState, NULL, canReturnNull);
    iconset.setPixmap( tmp, TQIconSet::Small, TQIconSet::Normal );
    iconset.setPixmap( tmp, TQIconSet::Large, TQIconSet::Normal );
    return iconset;
}

TDEIconFactory::TDEIconFactory( const TQString& iconName_P, TDEIcon::Group group_P,
    int size_P, TDEIconLoader* loader_P )
    : iconName( iconName_P ), group( group_P ), size( size_P ), loader( loader_P )
{
    canReturnNull = false;
    setAutoDelete( true );
}

TDEIconFactory::TDEIconFactory( const TQString& iconName_P, TDEIcon::Group group_P,
    int size_P, TDEIconLoader* loader_P, bool canReturnNull_P )
    : iconName( iconName_P ), group( group_P ), size( size_P ),
      loader( loader_P ), canReturnNull( canReturnNull_P)
{
    setAutoDelete( true );
}

TQPixmap* TDEIconFactory::createPixmap( const TQIconSet&, TQIconSet::Size, TQIconSet::Mode mode_P, TQIconSet::State )
    {
#ifdef TDEICONLOADER_DEBUG
    bool found = false;
    for( TQValueList< TDEIconLoaderDebug >::Iterator it = kiconloaders->begin();
         it != kiconloaders->end();
         ++it )
        {
        if( (*it).loader == loader )
            {
            found = true;
            if( !(*it).valid )
                {
#ifdef NDEBUG
                loader = TDEGlobal::iconLoader();
                iconName = "no_way_man_you_will_get_broken_icon";
#else
                kdWarning() << "Using already destroyed TDEIconLoader for loading an icon!" << endl;
                kdWarning() << "Appname:" << (*it).appname << ", icon:" << iconName << endl;
                kdWarning() << "Deleted at:" << endl;
                kdWarning() << (*it).delete_bt << endl;
                kdWarning() << "Current:" << endl;
                kdWarning() << kdBacktrace() << endl;
                abort();
                return NULL;
#endif
                }
            break;
            }
        }
    if( !found )
        {
#ifdef NDEBUG
        loader = TDEGlobal::iconLoader();
        iconName = "no_way_man_you_will_get_broken_icon";
#else
        kdWarning() << "Using unknown TDEIconLoader for loading an icon!" << endl;
        kdWarning() << "Icon:" << iconName << endl;
        kdWarning() << kdBacktrace() << endl;
        abort();
        return NULL;
#endif
        }
#endif
    // TQIconSet::Mode to TDEIcon::State conversion
    static const TDEIcon::States tbl[] = { TDEIcon::DefaultState, TDEIcon::DisabledState, TDEIcon::ActiveState };
    int state = TDEIcon::DefaultState;
    if( mode_P <= TQIconSet::Active )
        state = tbl[ mode_P ];
    if( group >= 0 && state == TDEIcon::ActiveState )
    { // active and normal icon are usually the same
	if( loader->iconEffect()->fingerprint(group, TDEIcon::ActiveState )
            == loader->iconEffect()->fingerprint(group, TDEIcon::DefaultState ))
            return 0; // so let TQIconSet simply duplicate it
    }
    // ignore passed size
    // ignore passed state (i.e. on/off)
    TQPixmap pm = loader->loadIcon( iconName, group, size, state, 0, canReturnNull );
    return new TQPixmap( pm );
    }

// Easy access functions

TQPixmap DesktopIcon(const TQString& name, int force_size, int state,
	TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, TDEIcon::Desktop, force_size, state);
}

TQPixmap DesktopIcon(const TQString& name, TDEInstance *instance)
{
    return DesktopIcon(name, 0, TDEIcon::DefaultState, instance);
}

TQIconSet DesktopIconSet(const TQString& name, int force_size, TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, TDEIcon::Desktop, force_size );
}

TQPixmap BarIcon(const TQString& name, int force_size, int state,
	TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, TDEIcon::Toolbar, force_size, state);
}

TQPixmap BarIcon(const TQString& name, TDEInstance *instance)
{
    return BarIcon(name, 0, TDEIcon::DefaultState, instance);
}

TQIconSet BarIconSet(const TQString& name, int force_size, TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, TDEIcon::Toolbar, force_size );
}

TQPixmap SmallIcon(const TQString& name, int force_size, int state,
	TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, TDEIcon::Small, force_size, state);
}

TQPixmap SmallIcon(const TQString& name, TDEInstance *instance)
{
    return SmallIcon(name, 0, TDEIcon::DefaultState, instance);
}

TQIconSet SmallIconSet(const TQString& name, int force_size, TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, TDEIcon::Small, force_size );
}

TQPixmap MainBarIcon(const TQString& name, int force_size, int state,
	TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, TDEIcon::MainToolbar, force_size, state);
}

TQPixmap MainBarIcon(const TQString& name, TDEInstance *instance)
{
    return MainBarIcon(name, 0, TDEIcon::DefaultState, instance);
}

TQIconSet MainBarIconSet(const TQString& name, int force_size, TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, TDEIcon::MainToolbar, force_size );
}

TQPixmap UserIcon(const TQString& name, int state, TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, TDEIcon::User, 0, state);
}

TQPixmap UserIcon(const TQString& name, TDEInstance *instance)
{
    return UserIcon(name, TDEIcon::DefaultState, instance);
}

TQIconSet UserIconSet(const TQString& name, TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, TDEIcon::User );
}

int IconSize(TDEIcon::Group group, TDEInstance *instance)
{
    TDEIconLoader *loader = instance->iconLoader();
    return loader->currentSize(group);
}

TQPixmap TDEIconLoader::unknown()
{
    TQPixmap pix;
    if ( TQPixmapCache::find("unknown", pix) )
            return pix;

    TQString path = TDEGlobal::iconLoader()->iconPath("unknown", TDEIcon::Small, true);
    if (path.isEmpty())
    {
	kdDebug(264) << "Warning: Cannot find \"unknown\" icon." << endl;
	pix.resize(32,32);
    } else
    {
        pix.load(path);
        TQPixmapCache::insert("unknown", pix);
    }

    return pix;
}

void TDEIconLoaderPrivate::reconfigure()
{
  q->reconfigure(appname, mpDirs);
}

#include "kiconloader_p.moc"

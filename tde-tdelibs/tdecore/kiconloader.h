/*
 *
 * This file is part of the KDE project, module tdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef __TDEIconLoader_h_Included__
#define __TDEIconLoader_h_Included__

#include <tqstring.h>
#include <tqpixmap.h>
#include <tqiconset.h>

// Grmbl, X headers.....
#ifdef Status
#define TDEIconLoaderXStatus Status
#undef Status
#endif
#include <tqmovie.h>
#ifdef TDEIconLoaderXStatus
#define Status int
#undef TDEIconLoaderXStatus
#endif

#include <tdeglobal.h>
#include <kinstance.h>
#include <kicontheme.h>

struct TDEIconGroup;
class TDEIconThemeNode;
class TDEConfig;
struct TDEIconLoaderPrivate;
class TDEStandardDirs;
class TDEIconEffect;


/**
 * Iconloader for KDE.
 *
 * TDEIconLoader will load the current icon theme and all its base themes.
 * Icons will be searched in any of these themes. Additionally, it caches
 * icons and applies effects according the the user's preferences.
 *
 * In KDE, it is encouraged to load icons by "Group". An icon group is a
 * location on the screen where icons are being used. Standard groups are:
 * Desktop, Toolbar, MainToolbar, Small and Panel. Each group has some
 * centrally configured properties bound to it, including the icon size
 * and effects. This makes it possible to offer a consistent icon look in
 * all KDE applications.
 *
 * The standard groups are defined below.
 *
 * @li TDEIcon::Desktop: Icons in the iconview of konqueror, kdesktop and similar apps.
 * @li TDEIcon::Toolbar: Icons in toolbars.
 * @li TDEIcon::MainToolbar: Icons in the main toolbars.
 * @li TDEIcon::Small: Various small (typical 16x16) places: titlebars, listviews
 * and menu entries.
 * @li TDEIcon::Panel: Icons in kicker's panel
 *
 * The icons are stored on disk in an icon theme or in a standalone
 * directory. The icon theme directories contain multiple sizes and/or
 * depths for the same icon. The iconloader will load the correct one based
 * on the icon group and the current theme. Icon themes are stored globally
 * in share/icons, or, application specific in share/apps/$appdir/icons.
 *
 * The standalone directories contain just one version of an icon. The
 * directories that are searched are: $appdir/pics and $appdir/toolbar.
 * Icons in these directories can be loaded by using the special group
 * "User".
 *
 */
class TDECORE_EXPORT TDEIconLoader
{
public:

    /**
     * Constructs an iconloader.
     * @param appname Add the data directories of this application to the
     * icon search path for the "User" group. The default argument adds the
     * directories of the current application.
     * @param dirs the TDEStandardDirs object to use. If null the global one is used
     *
     * Usually, you use the default iconloader, which can be accessed via
     * TDEGlobal::iconLoader(), so you hardly ever have to create an
     * iconloader object yourself. That one is the current TDEInstance's
     * (typically TDEApplication's) iconloader.
     * @see TDEGlobal::iconLoader()
     * @see TDEInstance::iconLoader()
     */
    TDEIconLoader(const TQString& appname=TQString::null, TDEStandardDirs *dirs = 0);

    /**
     * Cleanup
     */
    ~TDEIconLoader();

    /**
     * Adds @p appname to the list of application specific directories.
     * @param appname The application name.
     */
    void addAppDir(const TQString& appname);

    /**
     * Loads an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or
     * the "unknown" pixmap is returned, depending on the value of the
     * @p canReturnNull parameter.
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See TDEIcon::StdSizes.
     * @param state The icon state: @p DefaultState, @p ActiveState or
     * @p DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     * @param path_store If not null, the path of the icon is stored here.
     * @param canReturnNull Can return a null pixmap? If false, the
     * "unknown" pixmap is returned when no appropriate icon has been found.
     * @return the TQPixmap. Can be null when not found, depending on
     *         @p canReturnNull.
     */
    TQPixmap loadIcon(const TQString& name, TDEIcon::Group group, int size=0,
		     int state=TDEIcon::DefaultState, TQString *path_store=0L,
		     bool canReturnNull=false) const;

    /**
     * Creates an icon set, that will do on-demand loading of the icon.
     * Loading itself is done by calling loadIcon .
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See TDEIcon::StdSizes.
     * @param canReturnNull Can return a null iconset? If false, iconset
     * containing the "unknown" pixmap is returned when no appropriate icon has
     * been found.
     * @param immediateExistenceCheck If true on-demand icon loading will be
     * disabled for canReturnNull and a null iconset may be returned immediately
     * @return the icon set. Can be null when not found, depending on
     *          @p canReturnNull.
     * @since 3.5
     */
    TQIconSet loadIconSet(const TQString& name, TDEIcon::Group group, int size,
                         bool canReturnNull, bool immediateExistenceCheck);

    // KDE4 merge as (const TQString&,TDEIcon::Group,int=0,bool=false,bool=true);
    /**
     * Creates an icon set, that will do on-demand loading of the icon.
     * Loading itself is done by calling loadIcon .
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See TDEIcon::StdSizes.
     * @param canReturnNull Can return a null iconset? If false, iconset
     * containing the "unknown" pixmap is returned when no appropriate icon has
     * been found.
     * @return the icon set. Can be null when not found, depending on
     *          @p canReturnNull.
     * @since 3.1
     */
    TQIconSet loadIconSet(const TQString& name, TDEIcon::Group group, int size,
                         bool canReturnNull);

    // KDE4 merge as (const TQString&,TDEIcon::Group,int=0,bool=false,bool=true);
    /**
     * Creates an icon set, that will do on-demand loading of the icon.
     * Loading itself is done by calling loadIcon .
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See TDEIcon::StdSizes.
     * @return the icon set. Can be null when not found
     */
    TQIconSet loadIconSet(const TQString& name, TDEIcon::Group group, int size=0);

    /**
     * Returns the path of an icon.
     * @param name The name of the icon, without extension. If an absolute
     * path is supplied for this parameter, iconPath will return it
     * directly.
     * @param group_or_size If positive, search icons whose size is
     * specified by the icon group @p group_or_size. If negative, search
     * icons whose size is - @p group_or_size.
     *             See TDEIcon::Group and TDEIcon::StdSizes
     * @param canReturnNull Can return a null string? If not, a path to the
     *                      "unknown" icon will be returned.
     * @return the path of an icon, can be null or the "unknown" icon when
     *         not found, depending on @p canReturnNull.
     */
    TQString iconPath(const TQString& name, int group_or_size,
		     bool canReturnNull=false) const;

    /**
     * Loads an animated icon.
     * @param name The name of the icon.
     * @param group The icon group. See loadIcon().
     * @param size Override the default size for @p group.
     *             See TDEIcon::StdSizes.
     * @return A TQMovie object. Can be null if not found.
     */
    TQMovie loadMovie(const TQString& name, TDEIcon::Group group, int size=0) const;

    /**
     * Returns the path to an animated icon.
     * @param name The name of the icon.
     * @param group The icon group. See loadIcon().
     * @param size Override the default size for @p group.
     *             See TDEIcon::StdSizes.
     * @return the full path to the movie, ready to be passed to QMovie's constructor.
     * Empty string if not found.
     */
    TQString moviePath(const TQString& name, TDEIcon::Group group, int size=0) const;

    /**
     * Loads an animated icon as a series of still frames. If you want to load
     * a .mng animation as TQMovie instead, please use loadMovie() instead.
     * @param name The name of the icon.
     * @param group The icon group. See loadIcon().
     * @param size Override the default size for @p group.
     *             See TDEIcon::StdSizes.
     * @return A TQStringList containing the absolute path of all the frames
     * making up the animation.
     */
    TQStringList loadAnimated(const TQString& name, TDEIcon::Group group, int size=0) const;

    /**
     * Queries all available icons for a specific group, having a specific
     * context.
     * @param group_or_size If positive, search icons whose size is
     * specified by the icon group @p group_or_size. If negative, search
     * icons whose size is - @p group_or_size.
     *             See TDEIcon::Group and TDEIcon::StdSizes
     * @param context The icon context.
     * @return a list of all icons
     */
    TQStringList queryIcons(int group_or_size, TDEIcon::Context context=TDEIcon::Any) const;

    /**
     * Queries all available icons for a specific context.
     * @param group_or_size The icon preferred group or size. If available
     * at this group or size, those icons will be returned, in other case,
     * icons of undefined size will be returned. Positive numbers are groups,
     * negative numbers are negated sizes. See TDEIcon::Group and
     * TDEIcon::StdSizes
     * @param context The icon context.
     * @return A TQStringList containing the icon names
     * available for that context
     */
    TQStringList queryIconsByContext(int group_or_size,
				    TDEIcon::Context context=TDEIcon::Any) const;

    /**
     * @internal
     */
    bool hasContext( TDEIcon::Context context ) const;

    /**
     * Returns a list of all icons (*.png or *.xpm extension) in the
     * given directory.
     * @param iconsDir the directory to search in
     * @return A TQStringList containing the icon paths
     * @since 3.1
     */
    TQStringList queryIconsByDir( const TQString& iconsDir ) const;

    /**
     * Returns the current size of the group.
     * @param group the group to check.
     * @return the current size for an icon group.
     */
    int currentSize(TDEIcon::Group group) const;

    /**
     * Returns a pointer to the current theme. Can be used to query
     * available and default sizes for groups.
     * @return a pointer to the current theme. 0 if no theme set.
     */
    TDEIconTheme *theme() const;

    /**
     * Returns a pointer to the TDEIconEffect object used by the icon loader.
     * @return the TDEIconEffect.
     */
    TDEIconEffect *iconEffect() const;

    /**
     * Called by TDEInstance::newIconLoader to reconfigure the icon loader.
     * @param _appname the new application name
     * @param _dirs the new standard directories. If 0, the directories
     *              from TDEGlobal will be taken.
     */
    void reconfigure( const TQString& _appname, TDEStandardDirs *_dirs );

    /**
     * Returns the unknown icon. An icon that is used when no other icon
     * can be found.
     * @return the unknown pixmap
     */
    static TQPixmap unknown();

    /**
     * Checks whether the user wants to blend the icons with the background
     *  using the alpha channel information for a given group.
     * @param group the group to check
     * @return true if alpha blending is desired
     * @obsolete
     */
    bool alphaBlending( TDEIcon::Group group ) const;

    /**
     * Adds all the default themes from other desktops at the end of
     * the list of icon themes.
     * @since 3.1
     */
    void addExtraDesktopThemes();

    /**
     * Returns if the default icon themes of other desktops have been added
     * to the list of icon themes where icons are searched.
     * @since 3.1
     */
    bool extraDesktopThemesAdded() const;

    /**
     * Enables on-demand icon loading for QIconSets using TQIconFactory.
     * Icons loaded via loadIconSet() will be loaded as soon as they
     * need to be displayed, not earlier.
     *
     * Note that enabling or disabling this only affects loadIconSet()
     * calls after this setting is changed.
     *
     * The default is disabled, as the iconloader object must not be
     * destroyed before all those iconsets are destroyed.
     *
     * (Some broken applications use temporary TDEIconLoader objects).
     * Every TDEInstance 's iconloader has this feature enabled.
     *
     * @param enable true to enable delayed icon loading, false to disable
     * @see isDelayedIconSetLoadingEnabled()
     * @see QIconFactory
     * @since 3.1
     */
    void enableDelayedIconSetLoading( bool enable );

    /**
     * Checks whether delayed loading for TQIconSet is enabled.
     * @return whether icons for QIconSets will be loaded on demand.
     * @see enableDelayedIconSetLoading()
     * @see QIconFactory
     * @since 3.1
     */
    bool isDelayedIconSetLoadingEnabled() const;


 private:
    /**
     * @internal
     */
    void init( const TQString& _appname, TDEStandardDirs *_dirs );

    /**
     * @internal
     * tries to find an icon with the name. It tries some extension and
     * match strategies
     */
    TDEIcon findMatchingIcon(const TQString& name, int size) const;

    /**
     * @internal
     * Loads and caches an overlay.
     */
     TQImage *loadOverlay(const TQString& name, int size) const;

    /**
     * @internal
     * Adds themes installed in the application's directory.
     **/
    void addAppThemes(const TQString& appname);

    /**
     * Adds all themes that are part of this node and the themes
     * below (the fallbacks of the theme) in the tree.
     * @internal
     */
    void addBaseThemes(TDEIconThemeNode *node, const TQString &appname);

    /**
     * @internal
     * return the path for the unknown icon in that size
     * @since 3.1
     */
    TQString unknownIconPath( int size ) const;

    /**
     * Checks if name ends in one of the supported icon formats (i.e. .png)
     * and returns the name without the extension if it does.
     *
     * Otherwise name is returned unchanged.
     *
     * Currently supported:
     *   - png
     *   - xpm
     *   - svg  (if libart is being used)
     *   - svgz (if libart is being used)
     *
     * TODO: KDE 4 make public & static
     * @since 3.1
     */
    TQString removeIconExtension(const TQString &name) const;

    /**
     * Same as removeIconExtension except it prints a debug message
     * if an extension is removed to help catch programming errors.
     *
     * @see findMatchingIcon()
     * @see iconPath()
     *
     * TODO: KDE 4 make static
     */
    TQString removeIconExtensionInternal(const TQString &name) const;

    /**
     * Loads all the different sizes for an iconset.
     */
    TQIconSet loadIconSetNonDelayed( const TQString& name, TDEIcon::Group group,
                                    int size, bool canReturnNull );

    // @internal the data object
    TDEIconLoaderPrivate *d;
};

/**
 * \relates TDEIconLoader
 * Load a desktop icon.
 */
TDECORE_EXPORT TQPixmap DesktopIcon(const TQString& name, int size=0,
		    int state=TDEIcon::DefaultState,
		    TDEInstance *instance=TDEGlobal::instance());

/**
 * \relates TDEIconLoader
 * Load a desktop icon.
 */
TDECORE_EXPORT TQPixmap DesktopIcon(const TQString& name, TDEInstance *instance);

/**
 * \relates TDEIconLoader
 * Load a desktop icon, and apply the necessary effects to get an IconSet.
 */
TDECORE_EXPORT TQIconSet DesktopIconSet(const TQString& name, int size=0,
		    TDEInstance *instance=TDEGlobal::instance());

/**
 * \relates TDEIconLoader
 * Load a toolbar icon.
 */
TDECORE_EXPORT TQPixmap BarIcon(const TQString& name, int size=0, int state=TDEIcon::DefaultState,
	TDEInstance *instance=TDEGlobal::instance());

/**
 * \relates TDEIconLoader
 * Load a toolbar icon.
 */
TDECORE_EXPORT TQPixmap BarIcon(const TQString& name, TDEInstance *instance);

/**
 * \relates TDEIconLoader
 * Load a toolbar icon, and apply the necessary effects to get an IconSet.
 */
TDECORE_EXPORT TQIconSet BarIconSet(const TQString& name, int size=0,
		    TDEInstance *instance=TDEGlobal::instance());

/**
 * \relates TDEIconLoader
 * Load a small icon.
 */
TDECORE_EXPORT TQPixmap SmallIcon(const TQString& name, int size=0,
		  int state=TDEIcon::DefaultState,
		  TDEInstance *instance=TDEGlobal::instance());

/**
 * \relates TDEIconLoader
 * Load a small icon.
 */
TDECORE_EXPORT TQPixmap SmallIcon(const TQString& name, TDEInstance *instance);

/**
 * \relates TDEIconLoader
 * Load a small icon, and apply the necessary effects to get an IconSet.
 */
TDECORE_EXPORT TQIconSet SmallIconSet(const TQString& name, int size=0,
		    TDEInstance *instance=TDEGlobal::instance());

/**
 * \relates TDEIconLoader
 * Load a main toolbar icon.
 */
TDECORE_EXPORT TQPixmap MainBarIcon(const TQString& name, int size=0,
		    int state=TDEIcon::DefaultState,
		    TDEInstance *instance=TDEGlobal::instance());

/**
 * \relates TDEIconLoader
 * Load a main toolbar icon.
 */
TDECORE_EXPORT TQPixmap MainBarIcon(const TQString& name, TDEInstance *instance);

/**
 * \relates TDEIconLoader
 * Load a main toolbar icon, and apply the effects to get an IconSet.
 */
TDECORE_EXPORT TQIconSet MainBarIconSet(const TQString& name, int size=0,
		    TDEInstance *instance=TDEGlobal::instance());

/**
 * \relates TDEIconLoader
 * Load a user icon. User icons are searched in $appdir/pics.
 */
TDECORE_EXPORT TQPixmap UserIcon(const TQString& name, int state=TDEIcon::DefaultState,
	TDEInstance *instance=TDEGlobal::instance());

/**
 * \relates TDEIconLoader
 * Load a user icon. User icons are searched in $appdir/pics.
 */
TDECORE_EXPORT TQPixmap UserIcon(const TQString& name, TDEInstance *instance);

/**
 * \relates TDEIconLoader
 * Load a user icon, and apply the effects to get an IconSet.
 */
TDECORE_EXPORT TQIconSet UserIconSet(const TQString& name,
	TDEInstance *instance=TDEGlobal::instance());

/**
 * \relates TDEIconLoader
 * Returns the current icon size for a specific group.
 */
TDECORE_EXPORT int IconSize(TDEIcon::Group group, TDEInstance *instance=TDEGlobal::instance());

#endif // __TDEIconLoader_h_Included__

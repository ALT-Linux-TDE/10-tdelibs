/*
 *
 * This file is part of the KDE project, module tdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 */

#ifndef __TDEIconTheme_h_Included__
#define __TDEIconTheme_h_Included__

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqptrlist.h>
#include <tqvaluelist.h>
#include "tdelibs_export.h"

class TDEConfig;
class TDEIconThemeDir;

class TDEIconThemePrivate;

class TDEIconPrivate;

/**
 * One icon as found by TDEIconTheme. Also serves as a namespace containing
 * icon related constants.
 * @see TDEIconEffect
 * @see TDEIconTheme
 * @see TDEIconLoader
 */
class TDECORE_EXPORT TDEIcon
{
public:
    TDEIcon() { size = 0; }

    /**
     * Return true if this icon is valid, false otherwise.
     */
    bool isValid() const { return size != 0; }

    /**
     * Defines the context of the icon.
     */
    enum Context {
      Any, ///< Some icon with unknown purpose.
      Action, ///< An action icon (e.g. 'save', 'print').
      Application, ///< An icon that represents an application.
      Device, ///< An icon that represents a device.
      FileSystem, ///< An icon that represents a file system.
      MimeType, ///< An icon that represents a mime type (or file type).
      Animation, ///< An icon that is animated.
      Category, ///< An icon that represents a category.
      Emblem, ///< An icon that adds information to an existing icon.
      Emote, ///< An icon that expresses an emotion.
      International, ///< An icon that represents a country's flag.
      Place, ///< An icon that represents a location (e.g. 'home', 'trash').
      StatusIcon ///< An icon that represents an event.
    };

    /**
     * The type of the icon.
     */
    enum Type {
      Fixed, ///< Fixed-size icon.
      Scalable, ///< Scalable-size icon.
      Threshold ///< A threshold icon.
    };

    /**
     * The type of a match.
     */
    enum MatchType {
      MatchExact, ///< Only try to find an exact match.
      MatchBest   ///< Take the best match if there is no exact match.

    };

    // if you add a group here, make sure to change the config reading in
    // TDEIconLoader too
    /**
     * The group of the icon.
     */
    enum Group {
	/// No group
	NoGroup=-1,
	/// Desktop icons
	Desktop=0,
	/// First group
	FirstGroup=0,
	/// Toolbar icons
	Toolbar,
	/// Main toolbar icons
        MainToolbar,
	/// Small icons
	Small,
	/// Panel (Kicker) icons
	Panel,
	/// Last group
	LastGroup,
	/// User icons
	User
         };

    /**
     * These are the standard sizes for icons.
     */
    enum StdSizes {
        /// small icons for menu entries
        SizeSmall=16,
        /// slightly larger small icons for toolbars, panels, etc
        SizeSmallMedium=22,
        /// medium sized icons for the desktop
        SizeMedium=32,
        /// large sized icons for the panel
        SizeLarge=48,
        /// huge sized icons for iconviews
        SizeHuge=64,
        /// enormous sized icons for iconviews
        SizeEnormous=128
         };

    /**
     * Defines the possible states of an icon.
     */
    enum States { DefaultState, ///< The default state.
		  ActiveState,  ///< Icon is active.
		  DisabledState, ///< Icon is disabled.
		  LastState      ///< Last state (last constant)
    };

    /**
     * This defines an overlay, a semi-transparent image that is
     * projected onto the icon. They are used to show that the file
     * represented by the icon is, for example, locked, zipped or hidden.
     */
    enum Overlays {
      LockOverlay=0x100, ///< a file is locked
      ZipOverlay=0x200,  ///< a file is zipped
      LinkOverlay=0x400, ///< a file is a link
      HiddenOverlay=0x800, ///< a file is hidden
      ShareOverlay=0x1000, ///< a file is shared
      OverlayMask = ~0xff
    };

    /**
     * The size in pixels of the icon.
     */
    int size;

    /**
     * The context of the icon.
     */
    Context context;

    /**
     * The type of the icon: Fixed, Scalable or Threshold.
     **/
    Type type;

    /**
     * The threshold in case type == Threshold
     */
    int threshold;

    /**
     * The full path of the icon.
     */
    TQString path;

private:
    TDEIconPrivate *d;
};

inline TDEIcon::Group& operator++(TDEIcon::Group& group) { group = static_cast<TDEIcon::Group>(group+1); return group; }
inline TDEIcon::Group operator++(TDEIcon::Group& group,int) { TDEIcon::Group ret = group; ++group; return ret; }

/**
 * Class to use/access icon themes in KDE. This class is used by the
 * iconloader but can be used by others too.
 * @see TDEIconLoader
 */
class TDECORE_EXPORT TDEIconTheme
{
public:
    /**
     * Load an icon theme by name.
     * @param name the name of the theme (e.g. "hicolor" or "keramik")
     * @param appName the name of the application. Can be null. This argument
     *        allows applications to have themed application icons.
     */
    TDEIconTheme(const TQString& name, const TQString& appName=TQString::null);
    ~TDEIconTheme();

    /**
     * The stylized name of the icon theme.
     * @return the (human-readable) name of the theme
     */
    TQString name() const { return mName; }

    /**
     * A description for the icon theme.
     * @return a human-readable description of the theme, TQString::null
     *         if there is none
     */
    TQString description() const { return mDesc; }

    /**
     * Return the name of the "example" icon. This can be used to
     * present the theme to the user.
     * @return the name of the example icon, TQString::null if there is none
     */
    TQString example() const;

    /**
     * Return the name of the screenshot.
     * @return the name of the screenshot, TQString::null if there is none
     */
    TQString screenshot() const;

    /**
     * Returns the name of this theme's link overlay.
     * @return the name of the link overlay
     */
    TQString linkOverlay() const;

    /**
     * Returns the name of this theme's zip overlay.
     * @return the name of the zip overlay
     */
    TQString zipOverlay() const;

    /**
     * Returns the name of this theme's lock overlay.
     * @return the name of the lock overlay
     */
    TQString lockOverlay() const;

    /**
     * Returns the name of this theme's share overlay.
     * @return the name of the share overlay
     * @since 3.1
     */
    TQString shareOverlay () const;

    /**
     * Returns the toplevel theme directory.
     * @return the directory of the theme
     */
    TQString dir() const { return mDir; }

    /**
     * The themes this icon theme falls back on.
     * @return a list of icon themes that are used as fall-backs
     */
    TQStringList inherits() const { return mInherits; }

    /**
     * The icon theme exists?
     * @return true if the icon theme is valid
     */
    bool isValid() const;

    /**
     * The icon theme should be hidden to the user?
     * @return true if the icon theme is hidden
     * @since 3.1
     */
    bool isHidden() const;

    /**
     * The minimum display depth required for this theme. This can either
     * be 8 or 32.
     * @return the minimum bpp (8 or 32)
     */
    int depth() const { return mDepth; }

    /**
     * The default size of this theme for a certain icon group.
     * @param group The icon group. See TDEIcon::Group.
     * @return The default size in pixels for the given icon group.
     */
    int defaultSize(TDEIcon::Group group) const;

    /**
     * Query available sizes for a group.
     * @param group The icon group. See TDEIcon::Group.
     * @return a list of available sized for the given group
     */
    TQValueList<int> querySizes(TDEIcon::Group group) const;

    /**
     * Query available icons for a size and context.
     * @param size the size of the icons
     * @param context the context of the icons
     * @return the list of icon names
     */
    TQStringList queryIcons(int size, TDEIcon::Context context = TDEIcon::Any) const;

    /**
     * Query available icons for a context and preferred size.
     * @param size the size of the icons
     * @param context the context of the icons
     * @return the list of icon names
     */
    TQStringList queryIconsByContext(int size, TDEIcon::Context context = TDEIcon::Any) const;


    /**
     * Lookup an icon in the theme.
     * @param name The name of the icon, without extension.
     * @param size The desired size of the icon.
     * @param match The matching mode. TDEIcon::MatchExact returns an icon
     * only if matches exactly. TDEIcon::MatchBest returns the best matching
     * icon.
     * @return A TDEIcon class that describes the icon. If an icon is found,
     * @see TDEIcon::isValid will return true, and false otherwise.
     */
    TDEIcon iconPath(const TQString& name, int size, TDEIcon::MatchType match) const;
    
    /**
     * Returns true if the theme has any icons for the given context.
     * @since 3.5.5
     */
    bool hasContext( TDEIcon::Context context ) const;

    /**
     * List all icon themes installed on the system, global and local.
     * @return the list of all icon themes
     */
    static TQStringList list();

    /**
     * Returns the current icon theme.
     * @return the name of the current theme
     */
    static TQString current();

    /**
     * Reconfigure the theme.
     */
    static void reconfigure();

    /**
     * Returns the default icon theme.
     * @return the name of the default theme name
     * @since 3.1
     */
    static TQString defaultThemeName();

private:
    int mDefSize[8];
    TQValueList<int> mSizes[8];

    int mDepth;
    TQString mDir, mName, mDesc;
    TQStringList mInherits;
    TQPtrList<TDEIconThemeDir> mDirs;
    TDEIconThemePrivate *d;

    static TQString *_theme;
    static TQStringList *_theme_list;
};

#endif

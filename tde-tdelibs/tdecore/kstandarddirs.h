/*
  This file is part of the KDE libraries
  Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
  Copyright (C) 1999 Stephan Kulow <coolo@kde.org>
  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>

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

#ifndef SSK_KSTDDIRS_H
#define SSK_KSTDDIRS_H

#include <tqstring.h>
#include <tqdict.h>
#include <tqstringlist.h>
#include <tdeglobal.h>

class TDEConfig;
class TDEStandardDirsPrivate;

/**
 * @short Site-independent access to standard KDE directories.
 * @author Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
 *
 * This is one of the most central classes in tdelibs as
 * it provides a basic service: It knows where the files
 * reside on the user's hard disk. And it's meant to be the
 * only one that knows -- to make the real location as
 * transparent as possible to both the user and the applications.
 *
 * To this end it insulates the application from all information
 * and applications always refer to a file with a resource type
 * (e.g. icon) and a filename (e.g. khexdit.xpm). In an ideal world
 * the application would make no assumption where this file is and
 * leave it up to TDEStandardDirs::findResource("apps", "Home.desktop")
 * to apply this knowledge to return /opt/kde/share/applnk/Home.desktop
 * or ::locate("data", "kgame/background.jpg") to return
 * /opt/kde/share/apps/kgame/background.jpg
 *
 * The main idea behind TDEStandardDirs is that there are several
 * toplevel prefixes below which the files lie. One of these prefixes is
 * the one where the user installed tdelibs, one is where the
 * application was installed, and one is $HOME/.trinity, but there
 * may be even more. Under these prefixes there are several well
 * defined suffixes where specific resource types are to be found.
 * For example, for the resource type "html" the suffixes could be
 * share/doc/HTML and share/doc/tde/HTML.
 * So the search algorithm basically appends to each prefix each registered
 * suffix and tries to locate the file there.
 * To make the thing even more complex, it's also possible to register
 * absolute paths that TDEStandardDirs looks up after not finding anything
 * in the former steps. They can be useful if the user wants to provide
 * specific directories that aren't in his $HOME/.trinity directory for,
 * for example, icons.
 *
 * <b>Standard resources that tdelibs allocates are:</b>\n
 *
 * @li apps - Applications menu (.desktop files).
 * @li cache - Cached information (e.g. favicons, web-pages)
 * @li cgi - CGIs to run from kdehelp.
 * @li config - Configuration files.
 * @li data - Where applications store data.
 * @li exe - Executables in $prefix/bin. findExe() for a function that takes $PATH into account.
 * @li html - HTML documentation.
 * @li icon - Icons, see TDEIconLoader.
 * @li lib - Libraries.
 * @li locale - Translation files for TDELocale.
 * @li mime - Mime types.
 * @li module - Module (dynamically loaded library).
 * @li tqtplugins - TQt plugins (dynamically loaded objects for TQt)
 * @li services - Services.
 * @li servicetypes - Service types.
 * @li scripts - Application scripting additions.
 * @li sound - Application sounds.
 * @li templates - Templates
 * @li wallpaper - Wallpapers.
 * @li tmp - Temporary files (specific for both current host and current user)
 * @li socket - UNIX Sockets (specific for both current host and current user)
 * @li emoticons - Emoticons themes  (Since KDE 3.4)
 *
 * A type that is added by the class TDEApplication if you use it, is
 * appdata. This one makes the use of the type data a bit easier as it
 * appends the name of the application.
 * So while you had to ::locate("data", "appname/filename") so you can
 * also write ::locate("appdata", "filename") if your TDEApplication instance
 * is called "appname" (as set via TDEApplication's constructor or TDEAboutData, if
 * you use the global TDEStandardDirs object TDEGlobal::dirs()).
 * Please note though that you cannot use the "appdata"
 * type if you intend to use it in an applet for Kicker because 'appname' would
 * be "Kicker" instead of the applet's name. Therefore, for applets, you've got
 * to work around this by using ::locate("data", "appletname/filename").
 *
 * <b>TDEStandardDirs supports the following environment variables:</b>
 *
 * @li TDEDIRS: This may set an additional number of directory prefixes to
 *          search for resources. The directories should be separated
 *          by ':'. The directories are searched in the order they are
 *          specified.
 * @li TDEDIR:  Used for backwards compatibility. As TDEDIRS but only a single
 *          directory may be specified. If TDEDIRS is set TDEDIR is
 *          ignored.
 * @li TDEHOME: The directory where changes are saved to. This directory is
 *          used to search for resources first. If TDEHOME is not
 *          specified it defaults to "$HOME/.trinity"
 * @li TDEROOTHOME: Like TDEHOME, but used for the root user.
 *          If TDEROOTHOME is not set it defaults to the .kde directory in the
 *          home directory of root, usually "/root/.trinity".
 *          Note that the setting of $HOME is ignored in this case.
 *
 * @see TDEGlobalSettings
 */
class TDECORE_EXPORT TDEStandardDirs
{
public:
        /**
	 * TDEStandardDirs' constructor. It just initializes the caches.
	 **/
	TDEStandardDirs( );

	/**
	 * TDEStandardDirs' destructor.
	 */
	virtual ~TDEStandardDirs();

	/**
	 * Adds another search dir to front of the @p fsstnd list.
	 *
	 * @li When compiling tdelibs, the prefix is added to this.
	 * @li TDEDIRS or TDEDIR is taking into account
	 * @li Additional dirs may be loaded from kdeglobals.
	 *
	 * @param dir The directory to append relative paths to.
	 */
	void addPrefix( const TQString& dir );

	/**
	 * Adds another search dir to front of the XDG_CONFIG_XXX list
	 * of prefixes.
	 * This prefix is only used for resources that start with "xdgconf-"
	 *
	 * @param dir The directory to append relative paths to.
	 */
	void addXdgConfigPrefix( const TQString& dir );

	/**
	 * Adds another search dir to front of the XDG_DATA_XXX list
	 * of prefixes.
	 * This prefix is only used for resources that start with "xdgdata-"
	 *
	 * @param dir The directory to append relative paths to.
	 */
	void addXdgDataPrefix( const TQString& dir );

	/**
	 * Adds suffixes for types.
	 *
	 * You may add as many as you need, but it is advised that there
	 * is exactly one to make writing definite.
	 * All basic types ( kde_default) are added by addKDEDefaults(),
	 * but for those you can add more relative paths as well.
	 *
	 * The later a suffix is added, the higher its priority. Note, that the
	 * suffix should end with / but doesn't have to start with one (as prefixes
	 * should end with one). So adding a suffix for app_pics would look
	 * like TDEGlobal::dirs()->addResourceType("app_pics", "share/app/pics");
	 *
	 * @param type Specifies a short descriptive string to access
	 * files of this type.
	 * @param relativename Specifies a directory relative to the root
	 * of the KFSSTND.
	 * @return true if successful, false otherwise.
	 */
	bool addResourceType( const char *type,
			      const TQString& relativename );

	/**
	 * Adds absolute path at the end of the search path for
	 * particular types (for example in case of icons where
	 * the user specifies extra paths).
	 *
	 * You shouldn't need this
	 * function in 99% of all cases besides adding user-given
	 * paths.
	 *
	 * @param type Specifies a short descriptive string to access files
	 * of this type.
	 * @param absdir Points to directory where to look for this specific
	 * type. Non-existant directories may be saved but pruned.
	 * @return true if successful, false otherwise.
	 */
	bool addResourceDir( const char *type,
			     const TQString& absdir);

	/**
	 * Tries to find a resource in the following order:
	 * @li All PREFIX/\<relativename> paths (most recent first).
	 * @li All absolute paths (most recent first).
	 *
	 * The filename should be a filename relative to the base dir
	 * for resources. So is a way to get the path to libtdecore.la
	 * to findResource("lib", "libtdecore.la"). TDEStandardDirs will
	 * then look into the subdir lib of all elements of all prefixes
	 * ($TDEDIRS) for a file libtdecore.la and return the path to
	 * the first one it finds (e.g. /opt/kde/lib/libtdecore.la)
	 *
	 * @param type The type of the wanted resource
	 * @param filename A relative filename of the resource.
	 *
	 * @return A full path to the filename specified in the second
	 *         argument, or TQString::null if not found.
	 */
	TQString findResource( const char *type,
			      const TQString& filename ) const;

	/**
	 * Checks whether a resource is restricted as part of the KIOSK
	 * framework. When a resource is restricted it means that user-
	 * specific files in the resource are ignored.
	 *
	 * E.g. by restricting the "wallpaper" resource, only system-wide
	 * installed wallpapers will be found by this class. Wallpapers
	 * installed under the $TDEHOME directory will be ignored.
	 *
	 * @param type The type of the resource to check
	 * @param relPath A relative path in the resource.
	 *
	 * @return True if the resource is restricted.
	 * @since 3.1
	 */
	bool isRestrictedResource( const char *type,
			      const TQString& relPath=TQString::null ) const;

        /**
         * Returns a number that identifies this version of the resource.
         * When a change is made to the resource this number will change.
         *
	 * @param type The type of the wanted resource
	 * @param filename A relative filename of the resource.
	 * @param deep If true, all resources are taken into account
	 *        otherwise only the one returned by findResource().
	 *
	 * @return A number identifying the current version of the
	 *          resource.
	 */
	TQ_UINT32 calcResourceHash( const char *type,
			      const TQString& filename, bool deep) const;

	/**
	 * Tries to find all directories whose names consist of the
	 * specified type and a relative path. So would
	 * findDirs("apps", "Settings") return
	 * @li /opt/kde/share/applnk/Settings/
	 * @li /home/joe/.trinity/share/applnk/Settings/
	 *
	 * Note that it appends / to the end of the directories,
	 * so you can use this right away as directory names.
	 *
	 * @param type The type of the base directory.
	 * @param reldir Relative directory.
	 *
	 * @return A list of matching directories, or an empty
	 *         list if the resource specified is not found.
	 */
	TQStringList findDirs( const char *type,
                              const TQString& reldir ) const;

	/**
	 * Tries to find the directory the file is in.
	 * It works the same as findResource(), but it doesn't
	 * return the filename but the name of the directory.
	 *
	 * This way the application can access a couple of files
	 * that have been installed into the same directory without
	 * having to look for each file.
	 *
	 * findResourceDir("lib", "libtdecore.la") would return the
	 * path of the subdir libtdecore.la is found first in
	 * (e.g. /opt/kde/lib/)
	 *
	 * @param type The type of the wanted resource
	 * @param filename A relative filename of the resource.
	 * @return The directory where the file specified in the second
	 *         argument is located, or TQString::null if the type
	 *         of resource specified is unknown or the resource
	 *         cannot be found.
	 */
	TQString findResourceDir( const char *type,
				 const TQString& filename) const;


	/**
	 * Tries to find all resources with the specified type.
	 *
	 * The function will look into all specified directories
	 * and return all filenames in these directories.
	 *
	 * @param type The type of resource to locate directories for.
	 * @param filter Only accept filenames that fit to filter. The filter
	 *        may consist of an optional directory and a QRegExp
	 *        wildcard expression. E.g. "images\*.jpg". Use TQString::null
	 *        if you do not want a filter.
	 * @param recursive Specifies if the function should decend
	 *        into subdirectories.
	 * @param unique If specified,  only return items which have
	 *        unique suffixes - suppressing duplicated filenames.
	 *
	 * @return List of all the files whose filename matches the
	 *         specified filter.
	 */
	TQStringList findAllResources( const char *type,
				       const TQString& filter = TQString::null,
				       bool recursive = false,
				       bool unique = false) const;

	/**
	 * Tries to find all resources with the specified type.
	 *
	 * The function will look into all specified directories
	 * and return all filenames (full and relative paths) in
	 * these directories.
	 *
	 * @param type The type of resource to locate directories for.
	 * @param filter Only accept filenames that fit to filter. The filter
	 *        may consist of an optional directory and a QRegExp
	 *        wildcard expression. E.g. "images\*.jpg". Use TQString::null
	 *        if you do not want a filter.
	 * @param recursive Specifies if the function should decend
	 *        into subdirectories.
	 * @param unique If specified,  only return items which have
	 *        unique suffixes.
	 * @param relPaths The list to store the relative paths into
	 *        These can be used later to ::locate() the file
	 *
	 * @return List of all the files whose filename matches the
	 *         specified filter.
	 */
	TQStringList findAllResources( const char *type,
				       const TQString& filter,
				       bool recursive,
				       bool unique,
				       TQStringList &relPaths) const;

	/**
	 * Returns a TQStringList list of pathnames in the system path.
	 *
	 * @param pstr  The path which will be searched. If this is
	 * 		null (default), the $PATH environment variable will
	 *		be searched.
	 *
	 * @return a TQStringList list of pathnames in the system path.
	 */
	static TQStringList systemPaths( const TQString& pstr=TQString::null );

	/**
	 * Finds the executable in the system path.
	 *
	 * A valid executable must
	 * be a file and have its executable bit set.
	 *
	 * @param appname The name of the executable file for which to search.
	 * @param pathstr The path which will be searched. If this is
	 * 		null (default), the $PATH environment variable will
	 *		be searched.
	 * @param ignoreExecBit	If true, an existing file will be returned
	 *			even if its executable bit is not set.
	 *
	 * @return The path of the executable. If it was not found,
	 *         it will return TQString::null.
	 * @see findAllExe()
	 */
	static TQString findExe( const TQString& appname,
				const TQString& pathstr=TQString::null,
				bool ignoreExecBit=false );

	/**
	 * Finds all occurrences of an executable in the system path.
	 *
	 * @param list	Will be filled with the pathnames of all the
	 *		executables found. Will be empty if the executable
	 *		was not found.
	 * @param appname	The name of the executable for which to
	 *	 		search.
	 * @param pathstr	The path list which will be searched. If this
	 *		is 0 (default), the $PATH environment variable will
	 *		be searched.
	 * @param ignoreExecBit If true, an existing file will be returned
	 *			even if its executable bit is not set.
	 *
	 * @return The number of executables found, 0 if none were found.
	 *
	 * @see	findExe()
	 */
	static int findAllExe( TQStringList& list, const TQString& appname,
			       const TQString& pathstr=TQString::null,
			       bool ignoreExecBit=false );

	/**
	 * This function adds the defaults that are used by the current
	 * KDE version.
	 *
	 * It's a series of addResourceTypes()
	 * and addPrefix() calls.
	 * You normally wouldn't call this function because it's called
	 * for you from TDEGlobal.
	 */
	void addKDEDefaults();

	/**
	 * Reads customized entries out of the given config object and add
	 * them via addResourceDirs().
	 *
	 * @param config The object the entries are read from. This should
	 *        contain global config files
	 * @return true if new config paths have been added
	 * from @p config.
	 **/
	bool addCustomized(TDEConfig *config);

	/**
	 * This function is used internally by almost all other function as
	 * it serves and fills the directories cache.
         *
	 * @param type The type of resource
	 * @return The list of possible directories for the specified @p type.
	 * The function updates the cache if possible.  If the resource
	 * type specified is unknown, it will return an empty list.
         * Note, that the directories are assured to exist beside the save
         * location, which may not exist, but is returned anyway.
	 */
	TQStringList resourceDirs(const char *type) const;

	/**
	 * This function will return a list of all the types that TDEStandardDirs
	 * supports.
	 *
	 * @return All types that KDE supports
	 */
	TQStringList allTypes() const;

	/**
	 * Finds a location to save files into for the given type
	 * in the user's home directory.
	 *
	 * @param type The type of location to return.
	 * @param suffix A subdirectory name.
	 *             Makes it easier for you to create subdirectories.
	 *   You can't pass filenames here, you _have_ to pass
	 *       directory names only and add possible filename in
	 *       that directory yourself. A directory name always has a
 	 *       trailing slash ('/').
	 * @param create If set, saveLocation() will create the directories
	 *        needed (including those given by @p suffix).
	 *
	 * @return A path where resources of the specified type should be
	 *         saved, or TQString::null if the resource type is unknown.
	 */
	 TQString saveLocation(const char *type,
			      const TQString& suffix = TQString::null,
			      bool create = true) const;

        /**
         * Converts an absolute path to a path relative to a certain
         * resource.
         *
         * If "abs = ::locate(resource, rel)"
         * then "rel = relativeLocation(resource, abs)" and vice versa.
         *
         * @param type The type of resource.
         *
         * @param absPath An absolute path to make relative.
         *
         * @return A relative path relative to resource @p type that
         * will find @p absPath. If no such relative path exists, absPath
         * will be returned unchanged.
         */
         TQString relativeLocation(const char *type, const TQString &absPath);

	/**
	 * Recursively creates still-missing directories in the given path.
	 *
	 * The resulting permissions will depend on the current umask setting.
	 * permission = mode & ~umask.
	 *
	 * @param dir Absolute path of the directory to be made.
	 * @param mode Directory permissions.
	 * @return true if successful, false otherwise
	 */
	static bool makeDir(const TQString& dir, int mode = 0755);

	/**
	 * This returns a default relative path for the standard KDE
	 * resource types. Below is a list of them so you get an idea
	 * of what this is all about.
	 *
	 * @li data - share/apps
	 * @li html - share/doc/tde/HTML
	 * @li icon - share/icon
	 * @li config - share/config
	 * @li pixmap - share/pixmaps
	 * @li apps - share/applnk
	 * @li sound - share/sounds
	 * @li locale - share/locale
	 * @li services - share/services
	 * @li servicetypes - share/servicetypes
	 * @li mime - share/mimelnk
	 * @li wallpaper - share/wallpapers
	 * @li templates - share/templates
	 * @li exe - bin
	 * @li lib - lib
	 *
	 * @returns Static default for the specified resource.  You
	 *          should probably be using locate() or locateLocal()
	 *          instead.
	 * @see locate()
	 * @see locateLocal()
	 */
	static TQString kde_default(const char *type);

	/**
	 * @internal (for use by sycoca only)
	 */
	TQString kfsstnd_prefixes();

	/**
	 * @internal (for use by sycoca only)
	 */
	TQString kfsstnd_xdg_conf_prefixes();

	/**
	 * @internal (for use by sycoca only)
	 */
	TQString kfsstnd_xdg_data_prefixes();

	/**
	 * Returns the toplevel directory in which TDEStandardDirs
	 * will store things. Most likely $HOME/.trinity
	 * Don't use this function if you can use locateLocal
	 * @return the toplevel directory
	 */
	TQString localtdedir() const;

	/**
	 * @internal
	 * Returns the default toplevel directory where KDE is installed.
	 */
	static TQString kfsstnd_defaultprefix();

	/**
	 * @internal
	 * Returns the default bin directory in which KDE executables are stored.
	 */
	static TQString kfsstnd_defaultbindir();

	/**
	 * @return $XDG_DATA_HOME
	 * See also http://www.freedesktop.org/standards/basedir/draft/basedir-spec/basedir-spec.html
	 */
	TQString localxdgdatadir() const;

	/**
	 * @return $XDG_CONFIG_HOME
	 * See also http://www.freedesktop.org/standards/basedir/draft/basedir-spec/basedir-spec.html
	 */
	TQString localxdgconfdir() const;

	/**
	 * Checks for existence and accessability of a file or directory.
	 * Faster than creating a TQFileInfo first.
	 * @param fullPath the path to check. IMPORTANT: must end with a slash if expected to be a directory
	 *                 (and no slash for a file, obviously).
	 * @return true if the directory exists
	 */
	static bool exists(const TQString &fullPath);

	/**
	 * Expands all symbolic links and resolves references to
	 * '/./', '/../' and extra  '/' characters in @p dirname
	 * and returns the canonicalized absolute pathname.
	 * The resulting path will have no symbolic link, '/./'
	 * or '/../' components.
	 * @since 3.1
	 */
	static TQString realPath(const TQString &dirname);

	/**
	 * Expands all symbolic links and resolves references to
	 * '/./', '/../' and extra  '/' characters in @p filename
	 * and returns the canonicalized absolute pathname.
	 * The resulting path will have no symbolic link, '/./'
	 * or '/../' components.
	 * @since 3.4
	 */
	static TQString realFilePath(const TQString &filename);

 private:

	TQStringList prefixes;

	// Directory dictionaries
	TQDict<TQStringList> absolutes;
	TQDict<TQStringList> relatives;

	mutable TQDict<TQStringList> dircache;
	mutable TQDict<TQString> savelocations;

	// Disallow assignment and copy-construction
	TDEStandardDirs( const TDEStandardDirs& );
	TDEStandardDirs& operator= ( const TDEStandardDirs& );

	bool addedCustoms;

	class TDEStandardDirsPrivate;
	TDEStandardDirsPrivate *d;

	void checkConfig() const;
	void applyDataRestrictions(const TQString &) const;
	void createSpecialResource(const char*);

        // Like their public counter parts but with an extra priority argument
        // If priority is true, the directory is added directly after
        // $TDEHOME/$XDG_DATA_HOME/$XDG_CONFIG_HOME
	void addPrefix( const TQString& dir, bool priority );
	void addXdgConfigPrefix( const TQString& dir, bool priority );
	void addXdgDataPrefix( const TQString& dir, bool priority );

	// If priority is true, the directory is added before any other,
	// otherwise after
	bool addResourceType( const char *type,
			      const TQString& relativename, bool priority );
	bool addResourceDir( const char *type,
			     const TQString& absdir, bool priority);
};

/**
 * \addtogroup locates Locate Functions
 *  @{
 * On The Usage Of 'locate' and 'locateLocal'
 *
 * Typical KDE applications use resource files in one out of
 * three ways:
 *
 * 1) A resource file is read but is never written. A system
 *    default is supplied but the user can override this
 *    default in his local .kde directory:
 *
 *    \code
 *    // Code example
 *    myFile = locate("appdata", "groups.lst");
 *    myData =  myReadGroups(myFile); // myFile may be null
 *    \endcode
 *
 * 2) A resource file is read and written. If the user has no
 *    local version of the file the system default is used.
 *    The resource file is always written to the users local
 *    .kde directory.
 *
 *    \code
 *    // Code example
 *    myFile = locate("appdata", "groups.lst")
 *    myData =  myReadGroups(myFile);
 *    ...
 *    doSomething(myData);
 *    ...
 *    myFile = locateLocal("appdata", "groups.lst");
 *    myWriteGroups(myFile, myData);
 *    \endcode
 *
 * 3) A resource file is read and written. No system default
 *    is used if the user has no local version of the file.
 *    The resource file is always written to the users local
 *    .kde directory.
 *
 *    \code
 *    // Code example
 *    myFile = locateLocal("appdata", "groups.lst");
 *    myData =  myReadGroups(myFile);
 *    ...
 *    doSomething(myData);
 *    ...
 *    myFile = locateLocal("appdata", "groups.lst");
 *    myWriteGroups(myFile, myData);
 *    \endcode
 **/

/*!
 * \relates TDEStandardDirs
 * This function is just for convenience. It simply calls
 *instance->dirs()->\link TDEStandardDirs::findResource() findResource\endlink(type, filename).
 **/
TDECORE_EXPORT TQString locate( const char *type, const TQString& filename, const TDEInstance* instance = TDEGlobal::instance() );

/*!
 * \relates TDEStandardDirs
 * This function is much like locate. However it returns a
 * filename suitable for writing to. No check is made if the
 * specified filename actually exists. Missing directories
 * are created. If filename is only a directory, without a
 * specific file, filename must have a trailing slash.
 *
 **/
TDECORE_EXPORT TQString locateLocal( const char *type, const TQString& filename, const TDEInstance* instance = TDEGlobal::instance() );

/*!
 * \relates TDEStandardDirs
 * This function is much like locate. No check is made if the
 * specified filename actually exists. Missing directories
 * are created if @p createDir is true. If filename is only
 * a directory, without a specific file,
 * filename must have a trailing slash.
 *
 **/
TDECORE_EXPORT TQString locateLocal( const char *type, const TQString& filename, bool createDir, const TDEInstance* instance = TDEGlobal::instance() );

/*! @} */

#endif // SSK_KSTDDIRS_H

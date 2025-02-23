/* This file is part of the KDE libraries
   Copyright (c) 1999 Pietro Iglio <iglio@kde.org>

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
#ifndef _KDESKTOPFILE_H
#define _KDESKTOPFILE_H

#include "tdeconfig.h"
#include "tdelibs_export.h"

class KDesktopFilePrivate;

/** 
 * KDE Desktop File Management.
 *
 * @author Pietro Iglio <iglio@kde.org>
 * @see  TDEConfigBase  TDEConfig
 * @short KDE Desktop File Management class
 */
class TDECORE_EXPORT KDesktopFile : public TDEConfig
{
  TQ_OBJECT

public:
  /**
   * Constructs a KDesktopFile object and make it either read-write
   * or read-only.
   *
   * @param fileName  The name or path of the desktop file. If it
   *                  is not absolute, it will be located
   *                  using the resource type @p resType.
   * @param readOnly  Whether the object should be read-only.
   * @param resType   Allows you to change what sort of resource
   *                  to search for if @p fileName is not absolute.  For
   *                  instance, you might want to specify "config".
   */
  KDesktopFile( const TQString &fileName, bool readOnly = false,
		const char * resType = "apps");

  /**
   * Destructs the KDesktopFile object.
   *
   * Writes back any dirty configuration entries.
   */
  virtual ~KDesktopFile();

  /**
   * Checks whether this is really a desktop file.
   *
   * The check is performed looking at the file extension (the file is not
   * opened).
   * Currently, valid extensions are ".kdelnk" and ".desktop".
   * @param path the path of the file to check
   * @return true if the file appears to be a desktop file.
   */
  static bool isDesktopFile(const TQString& path);

  /**
   * Checks whether the user is authorized to run this desktop file.
   * By default users are authorized to run all desktop files but
   * the KIOSK framework can be used to activate certain restrictions.
   * See README.kiosk for more information.
   * @param path the file to check
   * @return true if the user is authorized to run the file
   * @since 3.1
   */
  static bool isAuthorizedDesktopFile(const TQString& path);

  /**
   * Returns the location where changes for the .desktop file @p path
   * should be written to.
   * @since 3.2
   */
  static TQString locateLocal(const TQString &path);

  /**
   * Returns the value of the "Type=" entry.
   * @return the type or TQString::null if not specified
   */
  TQString readType() const;

  /**
   * Returns the value of the "Icon=" entry.
   * @return the icon or TQString::null if not specified 
   */
  TQString readIcon() const;

  /**
   * Returns the value of the "Name=" entry.
   * @return the name or TQString::null if not specified
   */
  TQString readName() const;

  /**
   * Returns the value of the "Comment=" entry.
   * @return the comment or TQString::null if not specified
   */
  TQString readComment() const;

  /**
   * Returns the value of the "GenericName=" entry.
   * @return the generic name or TQString::null if not specified
   */
  TQString readGenericName() const;

  /**
   * Returns the value of the "Path=" entry.
   * @return the path or TQString::null if not specified
   */
  TQString readPath() const;

  /**
   * Returns the value of the "Dev=" entry.
   * @return the device or TQString::null if not specified
   */
  TQString readDevice() const;

  /**
   * Returns the value of the "URL=" entry.
   * @return the URL or TQString::null if not specified
   */
  TQString readURL() const;

  /**
   * Returns a list of the "Actions=" entries.
   * @return the list of actions
   */
  TQStringList readActions() const;

  /**
   * Sets the desktop action group.
   * @param group the new action group
   */
  void setActionGroup(const TQString &group);

  /**
   * Returns true if the action group exists, false otherwise
   * @param group the action group to test
   * @return true if the action group exists
   */
  bool hasActionGroup(const TQString &group) const;

  /**
   * Checks whether there is a "Type=Link" entry.
   *
   * The link points to the "URL=" entry.
   * @return true if there is a "Type=Link" entry
   */
  bool hasLinkType() const;

  /**
   * Checks whether there is an entry "Type=Application".
   * @return true if there is a "Type=Application" entry
   */
  bool hasApplicationType() const;

  /**
   * Checks whether there is an entry "Type=MimeType".
   * @return true if there is a "Type=MimeType" entry
   */
  bool hasMimeTypeType() const; // funny name :)

  /**
   * Checks whether there is an entry "Type=FSDev".
   * @return true if there is a "Type=FSDev" entry
   */
  bool hasDeviceType() const;

  /**
   * Checks whether the TryExec field contains a binary
   * which is found on the local system.
   * @return true if TryExec contains an existing binary
   */
  bool tryExec() const;

  /**
   * Returns the file name.
   * @return The filename as passed to the constructor.
   */
  TQString fileName() const;

  /**
   * Returns the resource.
   * @return The resource type as passed to the constructor.
   */
  TQString resource() const;

  /**
   * Returns the value of the "X-DocPath=" entry.
   * @return The value of the "X-DocPath=" entry.
   * @since 3.1
   */
  TQString readDocPath() const;

  /**
   * Returns the entry of the "SortOrder=" entry.
   * @return the value of the "SortOrder=" entry.
   */
  TQStringList sortOrder() const;

  /**
   * Copies all entries from this config object to a new 
   * KDesktopFile object that will save itself to @p file.
   *
   * Actual saving to @p file happens when the returned object is
   * destructed or when sync() is called upon it.
   *
   * @param file the new KDesktopFile object it will save itself to.
   * @since 3.2
   */
  KDesktopFile* copyTo(const TQString &file) const;

#ifdef KDE_NO_COMPAT
private:
#endif
  /**
   * @deprecated Use fileName() instead.
   */
    TDE_DEPRECATED TQString filename() const { return fileName(); };

private:

  TQString translatedEntry(const char*) const;

  // copy-construction and assignment are not allowed
  KDesktopFile( const KDesktopFile& );
  KDesktopFile& operator= ( const KDesktopFile& );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KDesktopFilePrivate *d;
};


#endif


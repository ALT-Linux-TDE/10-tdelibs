/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                       1999 Waldo Bastian <bastian@kde.org>

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

#ifndef __kservicetype_h__
#define __kservicetype_h__

#include "tdesycocaentry.h"
#include "kservice.h"

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqptrlist.h>
#include <tqmap.h>
#include <tqshared.h>
#include <tqdatastream.h>
#include <tqvariant.h>

#include <ksimpleconfig.h>

/**
 * A service type is the generic notion for a mimetype, a type of service
 * instead of a type of file.
 * For instance, KOfficeFilter is a service type.
 * It is associated to services according to the user profile (kuserprofile.h).
 * Service types are stored as desktop files in $TDEHOME/share/servicetypes.
 * @see KService
 */
class TDEIO_EXPORT KServiceType : public KSycocaEntry
{
  K_SYCOCATYPE( KST_KServiceType, KSycocaEntry )

public:
  typedef TDESharedPtr<KServiceType> Ptr;
  typedef TQValueList<Ptr> List;
public:

  /**
   * Constructor.  You may pass in arguments to create a servicetype with
   * specific properties.
   * @param _fullpath the path of the service type's desktop file
   * @param _name the name of the service type
   * @param _icon the icon name of the service type (can be null)
   * @param _comment a comment (can be null)
   */
  KServiceType( const TQString & _fullpath, const TQString& _name,
                const TQString& _icon, const TQString& _comment);

  /**
   * Construct a service type and take all informations from a config file.
   * @param _fullpath path of the desktop file, set to "" if calling from
   *                  a inherited constructor.
   */
  KServiceType( const TQString & _fullpath );

  /**
   * Construct a service type and take all informations from a deskop file.
   * @param config the configuration file
   */
  KServiceType( KDesktopFile *config);

  /**
   * @internal construct a service from a stream.
   * The stream must already be positionned at the correct offset
   */
  KServiceType( TQDataStream& _str, int offset );

  virtual ~KServiceType();

  /**
   * Returns the icon associated with this service type. Some
   *         derived classes offer special functions which take for
   *         example an URL and returns a special icon for this
   *         URL. An example is KMimeType, KFolderType and
   *         others.
   * @return the name of the icon, can be TQString::null.
   */
  TQString icon() const { return m_strIcon; }

  /**
   * Returns the descriptive comment associated, if any.
   * @return the comment, or TQString::null
   */
  TQString comment() const { return m_strComment; }

  /**
   * Returns the name of this service type.
   * @return the name of the service type
   */
  TQString name() const { return m_strName; }

  /**
   * Returns the relative path to the desktop entry file responsible for
   *         this servicetype.
   * For instance inode/directory.desktop, or kpart.desktop
   * @return the path of the desktop file
   */
  TQString desktopEntryPath() const { return entryPath(); }

  /**
   * Checks whether this service type inherits another one.
   * @return true if this service type inherits another one
   * @see parentServiceType()
   */
  bool isDerived() const { return m_bDerived; }

  /**
   * If this service type inherits from another service type,
   * return the name of the parent.
   * @return the parent service type, or TQString:: null if not set
   * @see isDerived()
   */
  TQString parentServiceType() const;

  /**
   * Checks whether this service type is or inherits from @p servTypeName.
   * @return true if this servicetype is or inherits from @p servTypeName
   * @since 3.1
   */
  bool inherits( const TQString& servTypeName ) const;

  /**
   * Returns the requested property. Some often used properties
   * have convenience access functions like name(),
   * comment() etc.
   *
   * @param _name the name of the property
   * @return the property, or invalid if not found
   */
  virtual TQVariant property( const TQString& _name ) const;

  /**
   * Returns the list of all properties of this service type.
   * @return the list of properties
   */
  virtual TQStringList propertyNames() const;

  /**
   * Checks whether the service type is valid.
   * @return true if the service is valid (e.g. name is not empty)
   */
  bool isValid() const { return m_bValid; }

  /**
   * Returns the type of the property with the given @p _name.
   *
   * @param _name the name of the property
   * @return the property type, or null if not found
   */
  virtual TQVariant::Type propertyDef( const TQString& _name ) const;

  virtual TQStringList propertyDefNames() const;
  virtual const TQMap<TQString,TQVariant::Type>& propertyDefs() const { return m_mapPropDefs; }

  /**
   * @internal
   * Save ourselves to the data stream.
   */
  virtual void save( TQDataStream& );

  /**
   * @internal
   * Load ourselves from the data stream.
   */
  virtual void load( TQDataStream& );

  /**
   * @internal
   * Pointer to parent serice type
   */
  // gcc 2.95.x doesn't understand KServiceType::Ptr here
  /* KServiceType:: */ Ptr parentType(); 
  /**
   * @internal
   * Register service that provides this service type
   */
  void addService(KService::Ptr service);
  /**
   * @internal
   * List serices that provide this service type
   */
  KService::List services();

  /**
   * Returns a pointer to the servicetype '_name' or 0L if the
   *         service type is unknown.
   * VERY IMPORTANT : don't store the result in a KServiceType * !
   * @param _name the name of the service type to search
   * @return the pointer to the service type, or 0
   */
  static Ptr serviceType( const TQString& _name );

  /**
   * Returns all services supporting the given servicetype name.
   * This doesn't take care of the user profile.
   * In fact it is used by KServiceTypeProfile,
   * which is used by TDETrader, and that's the one you should use.
   * @param _servicetype the name of the service type to search
   * @return the list of all services of the given type
   */
  static KService::List offers( const TQString& _servicetype );

  /**
   * Returns a list of all the supported servicetypes. Useful for
   *         showing the list of available servicetypes in a listbox,
   *         for example.
   * More memory consuming than the ones above, don't use unless
   * really necessary.
   * @return the list of all services
   */
  static List allServiceTypes();

protected:
  void init( KDesktopFile *config );

protected:
  TQString m_strName;
  TQString m_strIcon;
  TQString m_strComment;
  TQStringVariantMap m_mapProps;
  TQMap<TQString,TQVariant::Type> m_mapPropDefs;

  bool m_bValid:1;
  bool m_bDerived:1;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KServiceTypePrivate;
  KServiceTypePrivate* d;
};

//TQDataStream& operator>>( TQDataStream& _str, KServiceType& s );
//TQDataStream& operator<<( TQDataStream& _str, KServiceType& s );

#endif

/* This file is part of the KDE project
   Copyright (C) 2005 Till Adam <adam@kde.org>

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

#ifndef __kacl_h__
#define __kacl_h__

#include <sys/types.h>
#include <tdeio/global.h>

typedef TQPair<TQString, unsigned short> ACLUserPermissions;
typedef TQValueList<ACLUserPermissions> ACLUserPermissionsList;
typedef TQValueListIterator<ACLUserPermissions> ACLUserPermissionsIterator;
typedef TQValueListConstIterator<ACLUserPermissions> ACLUserPermissionsConstIterator;

typedef TQPair<TQString, unsigned short> ACLGroupPermissions;
typedef TQValueList<ACLGroupPermissions> ACLGroupPermissionsList;
typedef TQValueListIterator<ACLGroupPermissions> ACLGroupPermissionsIterator;
typedef TQValueListConstIterator<ACLGroupPermissions> ACLGroupPermissionsConstIterator;

/**
 * The KCAL class encapsulates a POSIX Access Control List. It follows the 
 * little standard that couldn't, 1003.1e/1003.2c, which died in draft status.
 * @short a POSIX ACL encapsulation
 * @author Till Adam <adam@kde.org>
 */
class TDEIO_EXPORT KACL
{
public:
  /**
   * Creates a new KACL from @p aclString. If the string is a valid acl
   * string, isValid() will afterwards return true.
   */
  KACL( const TQString & aclString );

  /** Copy ctor */
  KACL( const KACL& rhs );

  /** 
   * Creates a new KACL from the basic permissions passed in @p basicPermissions.
   * isValid() will return true, afterwards.
   */
  KACL( mode_t basicPermissions );

  /**
   * Creates an empty KACL. Until a valid acl string is set via setACL,
   * isValid() will return false.
   */
  KACL();

  virtual ~KACL();

  KACL& operator=( const KACL& rhs ) { 
    if ( this != &rhs )
      setACL( rhs.asString() );
    return *this;
  }

  bool operator==( const KACL& rhs ) const;

  bool operator!=( const KACL& rhs ) const {
    return !operator==( rhs );
  }

  /**
   * Returns whether the KACL object represents a valid acl.
   * @return whether the KACL object represents a valid acl.
   */
  bool isValid() const;

  /** The standard (non-extended) part of an ACL. These map directly to 
   * standard unix file permissions. Setting them will never make a valid
   * ACL invalid. */

  /** @return the owner's premissions entry */
  unsigned short ownerPermissions() const;

  /** Set the owner's permissions entry.
   * @return success or failure */
  bool setOwnerPermissions( unsigned short );

  /** @return the owning group's premissions entry */
  unsigned short owningGroupPermissions() const;

  /** Set the owning group's permissions entry.
   * @return success or failure */
  bool setOwningGroupPermissions( unsigned short );

  /** @return the premissions entry for others */
  unsigned short othersPermissions() const;

  /** Set the permissions entry for others.
   * @return success or failure */
  bool setOthersPermissions( unsigned short );

  /** @return the basic (owner/group/others) part of the ACL as a mode_t */
  mode_t basePermissions() const;

  /** The interface to the extended ACL. This is a mask, permissions for 
   * n named users and permissions for m named groups. */

  /**
   * Return whether the ACL contains extended entries or can be expressed
   * using only basic file permissions.
   * @return whether the ACL contains extended entries */
  bool isExtended() const;

  /**
   * Return the entry for the permissions mask if there is one and sets
   * @p exists to true. If there is no such entry, @p exists is set to false.
   * @return the permissions mask entry */
  unsigned short maskPermissions( bool &exists ) const;

  /** Set the permissions mask for the ACL. Permissions set for individual 
   * entries will be masked with this, such that their effective permissions
   * are the result of the logical and of their entry and the mask. 
   * @return success or failure */
  bool setMaskPermissions( unsigned short );

  /** 
   * Access to the permissions entry for a named user, if such an entry 
   * exists. @p exists is set to true if a matching entry exists and
   * to false otherwise.
   * @return the permissions for a user entry with the name in @p name */
  unsigned short namedUserPermissions( const TQString& name, bool *exists ) const;


  /** Set the permissions for a user with the name @p name. Will fail
   * if the user doesn't exist, in which case the ACL will be unchanged.
   * @return success or failure. */
  bool setNamedUserPermissions( const TQString& name, unsigned short );

  /** Returns the list of all group permission entries. Each entry consists
   * of a name/permissions pair. This is a TQPair, therefore access is provided 
   * via the .first and .next members.
   * @return the list of all group permission entries. */
  ACLUserPermissionsList allUserPermissions() const;

  /** Replace the list of all user permissions with @p list. If one
   * of the entries in the list does not exists, or setting of the ACL
   * entry fails for any reason, the ACL will be left unchanged.
   * @return success or failure */
  bool setAllUserPermissions( const ACLUserPermissionsList &list );

  /**
   * Access to the permissions entry for a named group, if such an entry 
   * exists. @p exists is set to true if a matching entry exists and
   * to false otherwise.
   * @return the permissions for a group with the name in @p name */
  unsigned short namedGroupPermissions( const TQString& name, bool *exists ) const;

  /** Set the permissions for a group with the name @p name. Will fail
   * if the group doesn't exist, in which case the ACL be unchanged.
   * @return success or failure. */
  bool setNamedGroupPermissions( const TQString& name, unsigned short );

  /** Returns the list of all group permission entries. Each entry consists
   * of a name/permissions pair. This is a TQPair, therefor access is provided 
   * via the .first and .next members.
   * @return the list of all group permission entries. */

  ACLGroupPermissionsList allGroupPermissions() const;
  /** Replace the list of all user permissions with @p list. If one
   * of the entries in the list does not exists, or setting of the ACL
   * entry fails for any reason, the ACL will be left unchanged.
   * @return success or failure */
  bool setAllGroupPermissions( const ACLGroupPermissionsList & );

  /** Sets the whole list from a string. If the string in @p aclStr represents 
   * a valid ACL, it will be set, otherwise the ACL remains unchanged.
   * @return whether setting the ACL was successful. */
  bool setACL( const TQString &aclStr );

  /** Return a string representation of the ACL.
   * @return a string version of the ACL in the format compatible with libacl and
   * POSIX 1003.1e. Implementations conforming to that standard should be able
   * to take such strings as input. */
  TQString asString() const;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KACLPrivate;
  KACLPrivate * d;
  TDEIO_EXPORT friend TQDataStream & operator<< ( TQDataStream & s, const KACL & a );
  TDEIO_EXPORT friend TQDataStream & operator>> ( TQDataStream & s, KACL & a );
};

TDEIO_EXPORT TQDataStream & operator<< ( TQDataStream & s, const KACL & a );
TDEIO_EXPORT TQDataStream & operator>> ( TQDataStream & s, KACL & a );

#endif

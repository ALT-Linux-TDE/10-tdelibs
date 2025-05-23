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
// $Id: kacl.cpp 424977 2005-06-13 15:13:22Z tilladam $

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#ifdef USE_POSIX_ACL
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
#include <acl/libacl.h>
#else
#include <posixacladdons.h>
#endif
#endif
#include <tqintdict.h>

#include <kdebug.h>

#include "kacl.h"


#ifdef USE_POSIX_ACL
static void printACL( acl_t acl, const TQString &comment );
static TQString aclAsString(const acl_t acl);
#endif

class KACL::KACLPrivate {
public:
    KACLPrivate() : m_acl( 0 ) { init(); }
#ifdef USE_POSIX_ACL
    KACLPrivate( acl_t acl )
        : m_acl( acl ) { init(); }
    ~KACLPrivate() { if ( m_acl ) acl_free( m_acl ); }
#endif
    void init() {
        m_usercache.setAutoDelete( true );
        m_groupcache.setAutoDelete( true );
    }
    // helpers
#ifdef USE_POSIX_ACL
    bool setMaskPermissions( unsigned short v );
    TQString getUserName( uid_t uid ) const;
    TQString getGroupName( gid_t gid ) const;
    bool setAllUsersOrGroups( const TQValueList< TQPair<TQString, unsigned short> > &list, acl_tag_t type );
    bool setNamedUserOrGroupPermissions( const TQString& name, unsigned short permissions, acl_tag_t type );

    acl_t m_acl;
#else
    int m_acl;
#endif
    mutable TQIntDict<TQString> m_usercache;
    mutable TQIntDict<TQString> m_groupcache;
};

KACL::KACL( const TQString &aclString )
    : d( new KACLPrivate )
{
    setACL( aclString );
}

KACL::KACL( mode_t basePermissions )
#ifdef USE_POSIX_ACL
    : d( new KACLPrivate( acl_from_mode( basePermissions ) ) )
#else
    : d( new KACLPrivate )
#endif
{
#ifndef USE_POSIX_ACL
    Q_UNUSED( basePermissions );
#endif
}

KACL::KACL()
    : d( new KACLPrivate )
{
}

KACL::KACL( const KACL& rhs )
    : d( new KACLPrivate )
{
    setACL( rhs.asString() );
}

KACL::~KACL()
{
    delete d;
}

bool KACL::operator==( const KACL& rhs ) const {
#ifdef USE_POSIX_ACL
    return ( acl_cmp( d->m_acl, rhs.d->m_acl ) == 0 );
#else
    Q_UNUSED( rhs );
    return true;
#endif
}

bool KACL::isValid() const
{
    bool valid = false;
#ifdef USE_POSIX_ACL
    if ( d->m_acl ) {
        valid = ( acl_valid( d->m_acl ) == 0 );
    }
#endif
    return valid;
}

bool KACL::isExtended() const
{
#ifdef USE_POSIX_ACL
    return ( acl_equiv_mode( d->m_acl, NULL ) != 0 );
#else
    return false;
#endif
}

#ifdef USE_POSIX_ACL
static acl_entry_t entryForTag( acl_t acl, acl_tag_t tag )
{
    acl_entry_t entry;
    int ret = acl_get_entry( acl, ACL_FIRST_ENTRY, &entry );
    while ( ret == 1 ) {
        acl_tag_t currentTag;
        acl_get_tag_type( entry, &currentTag );
        if ( currentTag == tag )
            return entry;
        ret = acl_get_entry( acl, ACL_NEXT_ENTRY, &entry );
    }
    return 0;
}

static unsigned short entryToPermissions( acl_entry_t entry )
{
    if ( entry == 0 ) return 0;
    acl_permset_t permset;
    if ( acl_get_permset( entry, &permset ) != 0 ) return 0;
    return( acl_get_perm( permset, ACL_READ ) << 2 |
            acl_get_perm( permset, ACL_WRITE ) << 1 |
            acl_get_perm( permset, ACL_EXECUTE ) );
}

static void permissionsToEntry( acl_entry_t entry, unsigned short v )
{
    if ( entry == 0 ) return;
    acl_permset_t permset;
    if ( acl_get_permset( entry, &permset ) != 0 ) return;
    acl_clear_perms( permset );
    if ( v & 4 ) acl_add_perm( permset, ACL_READ );
    if ( v & 2 ) acl_add_perm( permset, ACL_WRITE );
    if ( v & 1 ) acl_add_perm( permset, ACL_EXECUTE );
}

static void printACL( acl_t acl, const TQString &comment )
{
    kdDebug() << comment << aclAsString( acl ) << endl;
}

static int getUidForName( const TQString& name )
{
    struct passwd *user = getpwnam( name.latin1() );
    if ( user )
        return user->pw_uid;
    else
        return -1;
}

static int getGidForName( const TQString& name )
{
    struct group *group = getgrnam( name.latin1() );
    if ( group )
        return group->gr_gid;
    else
        return -1;
}
#endif
// ------------------ begin API implementation ------------

unsigned short KACL::ownerPermissions() const
{
#ifdef USE_POSIX_ACL
    return entryToPermissions( entryForTag( d->m_acl, ACL_USER_OBJ ) );
#else
    return 0;
#endif
}

bool KACL::setOwnerPermissions( unsigned short v )
{
#ifdef USE_POSIX_ACL
    permissionsToEntry( entryForTag( d->m_acl, ACL_USER_OBJ ), v );
#else
    Q_UNUSED( v );
#endif
    return true;
}

unsigned short KACL::owningGroupPermissions() const
{
#ifdef USE_POSIX_ACL
    return entryToPermissions( entryForTag( d->m_acl, ACL_GROUP_OBJ ) );
#else
    return 0;
#endif
}

bool KACL::setOwningGroupPermissions( unsigned short v )
{
#ifdef USE_POSIX_ACL
    permissionsToEntry( entryForTag( d->m_acl, ACL_GROUP_OBJ ), v );
#else
    Q_UNUSED( v );
#endif
    return true;
}

unsigned short KACL::othersPermissions() const
{
#ifdef USE_POSIX_ACL
    return entryToPermissions( entryForTag( d->m_acl, ACL_OTHER ) );
#else
    return 0;
#endif
}

bool KACL::setOthersPermissions( unsigned short v )
{
#ifdef USE_POSIX_ACL
    permissionsToEntry( entryForTag( d->m_acl, ACL_OTHER ), v );
#else
    Q_UNUSED( v );
#endif
    return true;
}

mode_t KACL::basePermissions() const
{
    mode_t perms( 0 );
#ifdef USE_POSIX_ACL
    if ( ownerPermissions() & ACL_READ ) perms |= S_IRUSR;
    if ( ownerPermissions() & ACL_WRITE ) perms |= S_IWUSR;
    if ( ownerPermissions() & ACL_EXECUTE ) perms |= S_IXUSR;
    if ( owningGroupPermissions() & ACL_READ ) perms |= S_IRGRP;
    if ( owningGroupPermissions() & ACL_WRITE ) perms |= S_IWGRP;
    if ( owningGroupPermissions() & ACL_EXECUTE ) perms |= S_IXGRP;
    if ( othersPermissions() & ACL_READ ) perms |= S_IROTH;
    if ( othersPermissions() & ACL_WRITE ) perms |= S_IWOTH;
    if ( othersPermissions() & ACL_EXECUTE ) perms |= S_IXOTH;
#endif
   return perms;
}

unsigned short KACL::maskPermissions( bool &exists ) const
{
    exists = true;
#ifdef USE_POSIX_ACL
    acl_entry_t entry = entryForTag( d->m_acl, ACL_MASK );
    if ( entry == 0 ) {
        exists = false;
        return 0;
    }
    return entryToPermissions( entry );
#else
    return 0;
#endif
}

#ifdef USE_POSIX_ACL
bool KACL::KACLPrivate::setMaskPermissions( unsigned short v )
{
    acl_entry_t entry = entryForTag( m_acl, ACL_MASK );
    if ( entry == 0 ) {
        acl_create_entry( &m_acl, &entry );
        acl_set_tag_type( entry, ACL_MASK );
    }
    permissionsToEntry( entry, v );
    return true;
}
#endif

bool KACL::setMaskPermissions( unsigned short v )
{
#ifdef USE_POSIX_ACL
    return d->setMaskPermissions( v );
#else
    Q_UNUSED( v );
    return true;
#endif
}

/**************************
 * Deal with named users  *
 **************************/
unsigned short KACL::namedUserPermissions( const TQString& name, bool *exists ) const
{
#ifdef USE_POSIX_ACL
    acl_entry_t entry;
    uid_t id;
    *exists = false;
    int ret = acl_get_entry( d->m_acl, ACL_FIRST_ENTRY, &entry );
    while ( ret == 1 ) {
        acl_tag_t currentTag;
        acl_get_tag_type( entry, &currentTag );
        if ( currentTag ==  ACL_USER ) {
            id = *( (uid_t*) acl_get_qualifier( entry ) );
            if ( d->getUserName( id ) == name ) {
                *exists = true;
                return entryToPermissions( entry );
            }
        }
        ret = acl_get_entry( d->m_acl, ACL_NEXT_ENTRY, &entry );
    }
#else
    Q_UNUSED( name );
    Q_UNUSED( exists );
#endif
    return 0;
}

#ifdef USE_POSIX_ACL
bool KACL::KACLPrivate::setNamedUserOrGroupPermissions( const TQString& name, unsigned short permissions, acl_tag_t type )
{
    bool allIsWell = true;
    acl_t newACL = acl_dup( m_acl );
    acl_entry_t entry;
    bool createdNewEntry = false;
    bool found = false;
    int ret = acl_get_entry( newACL, ACL_FIRST_ENTRY, &entry );
    while ( ret == 1 ) {
        acl_tag_t currentTag;
        acl_get_tag_type( entry, &currentTag );
        if ( currentTag == type ) {
            int id = * (int*)acl_get_qualifier( entry );
            const TQString entryName = type == ACL_USER? getUserName( id ): getGroupName( id );
            if ( entryName == name ) {
              // found him, update
              permissionsToEntry( entry, permissions );
              found = true;
              break;
            }
        }
        ret = acl_get_entry( newACL, ACL_NEXT_ENTRY, &entry );
    }
    if ( !found ) {
        acl_create_entry( &newACL, &entry );
        acl_set_tag_type(  entry, type );
        int id = type == ACL_USER? getUidForName( name ): getGidForName( name );
        if ( id == -1 ||  acl_set_qualifier( entry, &id ) != 0 ) {
            acl_delete_entry( newACL, entry );
            allIsWell = false;
        } else {
            permissionsToEntry( entry, permissions );
            createdNewEntry = true;
        }
    }
    if ( allIsWell && createdNewEntry ) {
        // 23.1.1 of 1003.1e states that as soon as there is a named user or
        // named group entry, there needs to be a mask entry as well, so add
        // one, if the user hasn't explicitely set one.
        if ( entryForTag( newACL, ACL_MASK ) == 0 ) {
            acl_calc_mask( &newACL );
        }
    }

    if ( !allIsWell || acl_valid( newACL ) != 0 ) {
        acl_free( newACL );
        allIsWell = false;
    } else {
        acl_free( m_acl );
        m_acl = newACL;
    }
    return allIsWell;
}
#endif

bool KACL::setNamedUserPermissions( const TQString& name, unsigned short permissions )
{
#ifdef USE_POSIX_ACL
    return d->setNamedUserOrGroupPermissions( name, permissions, ACL_USER );
#else
    Q_UNUSED( name );
    Q_UNUSED( permissions );
    return true;
#endif
}

ACLUserPermissionsList KACL::allUserPermissions() const
{
    ACLUserPermissionsList list;
#ifdef USE_POSIX_ACL
    acl_entry_t entry;
    uid_t id;
    int ret = acl_get_entry( d->m_acl, ACL_FIRST_ENTRY, &entry );
    while ( ret == 1 ) {
        acl_tag_t currentTag;
        acl_get_tag_type( entry, &currentTag );
        if ( currentTag ==  ACL_USER ) {
            id = *( (uid_t*) acl_get_qualifier( entry ) );
            TQString name = d->getUserName( id );
            unsigned short permissions = entryToPermissions( entry );
            ACLUserPermissions pair = qMakePair( name, permissions );
            list.append( pair );
        }
        ret = acl_get_entry( d->m_acl, ACL_NEXT_ENTRY, &entry );
    }
#endif
    return list;
}

#ifdef USE_POSIX_ACL
bool KACL::KACLPrivate::setAllUsersOrGroups( const TQValueList< TQPair<TQString, unsigned short> > &list, acl_tag_t type )
{
    bool allIsWell = true;
    bool atLeastOneUserOrGroup = false;

    // make working copy, in case something goes wrong
    acl_t newACL = acl_dup( m_acl );
    acl_entry_t entry;

//printACL( newACL, "Before cleaning: " );
    // clear user entries
    int ret = acl_get_entry( newACL, ACL_FIRST_ENTRY, &entry );
    while ( ret == 1 ) {
        acl_tag_t currentTag;
        acl_get_tag_type( entry, &currentTag );
        if ( currentTag ==  type ) {
            acl_delete_entry( newACL, entry );
            // we have to start from the beginning, the iterator is
            // invalidated, on deletion
            ret = acl_get_entry( newACL, ACL_FIRST_ENTRY, &entry );
        } else {
            ret = acl_get_entry( newACL, ACL_NEXT_ENTRY, &entry );
        }
    }
//printACL( newACL, "After cleaning out entries: " );

    // now add the entries from the list
    TQValueList< TQPair<TQString, unsigned short> >::const_iterator it = list.constBegin();
    while ( it != list.constEnd() ) {
        acl_create_entry( &newACL, &entry );
        acl_set_tag_type( entry, type );
        int id = type == ACL_USER? getUidForName( (*it).first):getGidForName( (*it).first );
        if ( id == -1 || acl_set_qualifier( entry, &id ) != 0 ) {
            // user or group doesn't exist => error
            acl_delete_entry( newACL, entry );
            allIsWell = false;
            break;
        } else {
            permissionsToEntry( entry, (*it).second );
            atLeastOneUserOrGroup = true;
        }
        ++it;
    }
//printACL( newACL, "After adding entries: " );
    if ( allIsWell && atLeastOneUserOrGroup ) {
        // 23.1.1 of 1003.1e states that as soon as there is a named user or
        // named group entry, there needs to be a mask entry as well, so add
        // one, if the user hasn't explicitely set one.
        if ( entryForTag( newACL, ACL_MASK ) == 0 ) {
            acl_calc_mask( &newACL );
        }
    }
    if ( allIsWell && ( acl_valid( newACL ) == 0 ) ) {
        acl_free( m_acl );
        m_acl = newACL;
    } else {
        acl_free( newACL );
    }
    return allIsWell;
}
#endif

bool KACL::setAllUserPermissions( const ACLUserPermissionsList &users )
{
#ifdef USE_POSIX_ACL
    return d->setAllUsersOrGroups( users, ACL_USER );
#else
    Q_UNUSED( users );
    return true;
#endif
}


/**************************
 * Deal with named groups  *
 **************************/

unsigned short KACL::namedGroupPermissions( const TQString& name, bool *exists ) const
{
    *exists = false;
#ifdef USE_POSIX_ACL
    acl_entry_t entry;
    gid_t id;
    int ret = acl_get_entry( d->m_acl, ACL_FIRST_ENTRY, &entry );
    while ( ret == 1 ) {
        acl_tag_t currentTag;
        acl_get_tag_type( entry, &currentTag );
        if ( currentTag ==  ACL_GROUP ) {
            id = *( (gid_t*) acl_get_qualifier( entry ) );
            if ( d->getGroupName( id ) == name ) {
                *exists = true;
                return entryToPermissions( entry );
            }
        }
        ret = acl_get_entry( d->m_acl, ACL_NEXT_ENTRY, &entry );
    }
#else
    Q_UNUSED( name );
#endif
    return 0;
}

bool KACL::setNamedGroupPermissions( const TQString& name, unsigned short permissions )
{
#ifdef USE_POSIX_ACL
    return d->setNamedUserOrGroupPermissions( name, permissions, ACL_GROUP );
#else
    Q_UNUSED( name );
    Q_UNUSED( permissions );
    return true;
#endif
}


ACLGroupPermissionsList KACL::allGroupPermissions() const
{
    ACLGroupPermissionsList list;
#ifdef USE_POSIX_ACL
    acl_entry_t entry;
    gid_t id;
    int ret = acl_get_entry( d->m_acl, ACL_FIRST_ENTRY, &entry );
    while ( ret == 1 ) {
        acl_tag_t currentTag;
        acl_get_tag_type( entry, &currentTag );
        if ( currentTag ==  ACL_GROUP ) {
            id = *( (gid_t*) acl_get_qualifier( entry ) );
            TQString name = d->getGroupName( id );
            unsigned short permissions = entryToPermissions( entry );
            ACLGroupPermissions pair = qMakePair( name, permissions );
            list.append( pair );
        }
        ret = acl_get_entry( d->m_acl, ACL_NEXT_ENTRY, &entry );
    }
#endif
    return list;
}

bool KACL::setAllGroupPermissions( const ACLGroupPermissionsList &groups )
{
#ifdef USE_POSIX_ACL
    return d->setAllUsersOrGroups( groups, ACL_GROUP );
#else
    Q_UNUSED( groups );
    return true;
#endif
}

/**************************
 * from and to string     *
 **************************/

bool KACL::setACL( const TQString &aclStr )
{
    bool ret = false;
#ifdef USE_POSIX_ACL
    if ( aclStr.isEmpty() )
        return false;

    acl_t temp = acl_from_text( aclStr.latin1() );
    if ( acl_valid( temp ) != 0 ) {
        // TODO errno is set, what to do with it here?
        acl_free( temp );
    } else {
        if ( d->m_acl )
            acl_free( d->m_acl );
        d->m_acl = temp;
        ret = true;
    }
#else
    Q_UNUSED( aclStr );
#endif
    return ret;
}

TQString KACL::asString() const
{
#ifdef USE_POSIX_ACL
    return aclAsString( d->m_acl );
#else
    return TQString::null;
#endif
}


// helpers

#ifdef USE_POSIX_ACL
TQString KACL::KACLPrivate::getUserName( uid_t uid ) const
{
    TQString *temp;
    temp = m_usercache.find( uid );
    if ( !temp ) {
        struct passwd *user = getpwuid( uid );
        if ( user ) {
            m_usercache.insert( uid, new TQString(TQString::fromLatin1(user->pw_name)) );
            return TQString::fromLatin1( user->pw_name );
        }
        else
            return TQString::number( uid );
    }
    else
        return *temp;
}


TQString KACL::KACLPrivate::getGroupName( gid_t gid ) const
{
    TQString *temp;
    temp = m_groupcache.find( gid );
    if ( !temp ) {
        struct group *grp = getgrgid( gid );
        if ( grp ) {
            m_groupcache.insert( gid, new TQString(TQString::fromLatin1(grp->gr_name)) );
            return TQString::fromLatin1( grp->gr_name );
        }
        else
            return TQString::number( gid );
    }
    else
        return *temp;
}

static TQString aclAsString(const acl_t acl)
{
    char *aclString = acl_to_text( acl, 0 );
    TQString ret = TQString::fromLatin1( aclString );
    acl_free( (void*)aclString );
    return ret;
}


#endif

void KACL::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

TQDataStream & operator<< ( TQDataStream & s, const KACL & a )
{
    s << a.asString();
    return s;
}

TQDataStream & operator>> ( TQDataStream & s, KACL & a )
{
    TQString str;
    s >> str;
    a.setACL( str );
    return s;
}

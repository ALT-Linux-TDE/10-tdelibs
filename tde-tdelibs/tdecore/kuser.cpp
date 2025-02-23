/*
 *  KUser - represent a user/account
 *  Copyright (C) 2002 Tim Jansen <tim@tjansen.de>
 *
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
 */

#include <kuser.h>

#include "kstringhandler.h"
#include <tqvaluelist.h>
#include <tqstringlist.h>

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <grp.h>


class KUserPrivate : public TDEShared
{
public:
	bool valid;
	long uid, gid;
	TQString loginName, fullName;
	TQString roomNumber, workPhone, homePhone;
	TQString homeDir, shell;

	KUserPrivate() : valid(false) {}

	KUserPrivate(long _uid,
		     long _gid,
		     const TQString &_loginname,
		     const TQString &_fullname,
		     const TQString &_room,
		     const TQString &_workPhone,
		     const TQString &_homePhone,
		     const TQString &_homedir,
		     const TQString &_shell) :
		valid(true),
		uid(_uid),
		gid(_gid),
		loginName(_loginname),
		fullName(_fullname),
		roomNumber(_room),
		workPhone(_workPhone),
		homePhone(_homePhone),
		homeDir(_homedir),
		shell(_shell) {}
};


KUser::KUser(UIDMode mode) {
	long _uid = ::getuid(), _euid;
	if (mode == UseEffectiveUID && (_euid = ::geteuid()) != _uid )
		fillPasswd( ::getpwuid( _euid ) );
	else {
		fillName( ::getenv( "LOGNAME" ) );
		if (uid() != _uid) {
			fillName( ::getenv( "USER" ) );
			if (uid() != _uid)
				fillPasswd( ::getpwuid( _uid ) );
		}
	}
}

KUser::KUser(long uid) {
	fillPasswd( ::getpwuid( uid ) );
}

KUser::KUser(const TQString& name) {
	fillName( name.local8Bit().data() );
}

KUser::KUser(const char *name) {
	fillName( name );
}

KUser::KUser(struct passwd *p) {
    fillPasswd(p);
}

KUser::KUser(const KUser & user) 
  : d(user.d) 
{
}

KUser& KUser::operator =(const KUser& user) 
{
  d = user.d;
  return *this;
}

bool KUser::operator ==(const KUser& user) const {
    if (isValid() != user.isValid())
	return false;
    if (isValid())
	return uid() == user.uid();
    else
	return true;
}

bool KUser::operator !=(const KUser& user) const {
	return !operator ==(user);
}

void KUser::fillName(const char *name) {
	fillPasswd(name ? ::getpwnam( name ) : 0);
}

void KUser::fillPasswd(struct passwd *p) {
	if (p) {
		TQString gecos = KStringHandler::from8Bit(p->pw_gecos); 
		TQStringList gecosList = TQStringList::split(',', gecos, true);

		d = new KUserPrivate(p->pw_uid,
				     p->pw_gid,
				     TQString::fromLocal8Bit(p->pw_name),
				     (gecosList.size() > 0) ? gecosList[0] : TQString::null,
				     (gecosList.size() > 1) ? gecosList[1] : TQString::null,
				     (gecosList.size() > 2) ? gecosList[2] : TQString::null,
				     (gecosList.size() > 3) ? gecosList[3] : TQString::null,
				     TQString::fromLocal8Bit(p->pw_dir),
				     TQString::fromLocal8Bit(p->pw_shell));
	}
	else
		d = new KUserPrivate();
}

bool KUser::isValid() const {
	return d->valid;
}

long KUser::uid() const {
	if (d->valid)
		return d->uid;
	else
		return -1;
}

long KUser::gid() const {
	if (d->valid)
		return d->gid;
	else
		return -1;
}

bool KUser::isSuperUser() const {
	return uid() == 0;
}

TQString KUser::loginName() const {
	if (d->valid)
		return d->loginName;
	else
		return TQString::null;
}

TQString KUser::fullName() const {
	if (d->valid)
		return d->fullName;
	else
		return TQString::null;
}

TQString KUser::roomNumber() const {
	if (d->valid)
		return d->roomNumber;
	else
		return TQString::null;
}

TQString KUser::workPhone() const {
	if (d->valid)
		return d->workPhone;
	else
		return TQString::null;
}

TQString KUser::homePhone() const {
	if (d->valid)
		return d->homePhone;
	else
		return TQString::null;
}

TQString KUser::homeDir() const {
	if (d->valid)
		return d->homeDir;
	else
		return TQString::null;
}

TQString KUser::shell() const {
	if (d->valid)
		return d->shell;
	else
		return TQString::null;
}

TQValueList<KUserGroup> KUser::groups() const {
  TQValueList<KUserGroup> result;
  TQValueList<KUserGroup> allGroups = KUserGroup::allGroups();
  TQValueList<KUserGroup>::const_iterator it;
  for ( it = allGroups.begin(); it != allGroups.end(); ++it ) {
    TQValueList<KUser> users = (*it).users();
    if ( users.find( *this ) != users.end()) {
       result.append(*it);
    }
  }
  return result;
}

TQStringList KUser::groupNames() const {
  TQStringList result;
  TQValueList<KUserGroup> allGroups = KUserGroup::allGroups();
  TQValueList<KUserGroup>::const_iterator it;
  for ( it = allGroups.begin(); it != allGroups.end(); ++it ) {
    TQValueList<KUser> users = (*it).users();
    if ( users.find( *this ) != users.end()) {
       result.append((*it).name());
    }
  }
  return result;
}


TQValueList<KUser> KUser::allUsers() {
  TQValueList<KUser> result;

  struct passwd* p;

  while ((p = getpwent()))  {
    result.append(KUser(p));
  }

  endpwent();

  return result;
}

TQStringList KUser::allUserNames() {
  TQStringList result;

  struct passwd* p;

  while ((p = getpwent()))  {
    result.append(TQString::fromLocal8Bit(p->pw_name));
  }

  endpwent();
  return result;
}


KUser::~KUser() {
}

class KUserGroupPrivate : public TDEShared
{
public:
  bool valid;
  long gid;
  TQString name;
  TQValueList<KUser> users;
  
  KUserGroupPrivate() : valid(false) {}
  
  KUserGroupPrivate(long _gid, 
                const TQString & _name, 
                const TQValueList<KUser> & _users):
    valid(true), 
    gid(_gid), 
    name(_name),
    users(_users) {}
};

KUserGroup::KUserGroup(KUser::UIDMode mode) {
  KUser user(mode);
  fillGroup(getgrgid(user.gid()));
}

KUserGroup::KUserGroup(long gid) {
  fillGroup(getgrgid(gid));
}

KUserGroup::KUserGroup(const TQString& name) {
  fillName(name.local8Bit().data());
}

KUserGroup::KUserGroup(const char *name) {
  fillName(name);
}

KUserGroup::KUserGroup(struct group *g) {
  fillGroup(g);
}


KUserGroup::KUserGroup(const KUserGroup & group) 
  : d(group.d)
{
}

KUserGroup& KUserGroup::operator =(const KUserGroup& group) {
  d = group.d;
  return *this;
}

bool KUserGroup::operator ==(const KUserGroup& group) const {
  if (isValid() != group.isValid())
    return false;
  if (isValid())
    return gid() == group.gid();
  else
    return true;
}

bool KUserGroup::operator !=(const KUserGroup& user) const {
  return !operator ==(user);
}

void KUserGroup::fillName(const char *name) {
  fillGroup(name ? ::getgrnam( name ) : 0);
}

void KUserGroup::fillGroup(struct group *p) {
  if (!p) {
    d = new KUserGroupPrivate();
    return;
  }
  
  TQString name = KStringHandler::from8Bit(p->gr_name); 
  TQValueList<KUser> users;
  
  char **user = p->gr_mem;  
  for ( ; *user; user++) {
    KUser kUser(TQString::fromLocal8Bit(*user));
    users.append(kUser);
  }
  
  d = new KUserGroupPrivate(p->gr_gid,
            TQString::fromLocal8Bit(p->gr_name),
            users);  

}

bool KUserGroup::isValid() const {
  return d->valid;
}

long KUserGroup::gid() const {
  if (d->valid)
    return d->gid;
  else
    return -1;
}

TQString KUserGroup::name() const {
  if (d->valid)
    return d->name;
  else
    return TQString::null;
}

const TQValueList<KUser>& KUserGroup::users() const {
  return d->users;
}

TQStringList KUserGroup::userNames() const {
  TQStringList result;
  TQValueList<KUser>::const_iterator it;
  for ( it = d->users.begin(); it != d->users.end(); ++it ) {
    result.append((*it).loginName());
  }
  return result;
}



TQValueList<KUserGroup> KUserGroup::allGroups() {
  TQValueList<KUserGroup> result;
  
  struct group* g;
  while ((g = getgrent()))  {
     result.append(KUserGroup(g));
  }

  endgrent();

  return result;
}

TQStringList KUserGroup::allGroupNames() {
  TQStringList result;
  
  struct group* g;
  while ((g = getgrent()))  {
     result.append(TQString::fromLocal8Bit(g->gr_name));
  }

  endgrent();

  return result;
}


KUserGroup::~KUserGroup() {
}


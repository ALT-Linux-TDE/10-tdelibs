/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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

#ifndef CUPSINFOS_H
#define CUPSINFOS_H

#include <tqstring.h>
#include "kpreloadobject.h"

class CupsInfos : public KPReloadObject
{
public:
	static CupsInfos* self();

	CupsInfos();
	~CupsInfos();

	const TQString& host() const;
        TQString  hostaddr() const;
	int port() const;
	const TQString& login() const;
	const TQString& password() const;
	const TQString& realLogin() const;
	bool savePassword() const;

	void setHost(const TQString& s);
	void setPort(int p);
	void setLogin(const TQString& s);
	void setPassword(const TQString& s);
	void setSavePassword( bool on );

	const char* getPasswordCB();

	void load();
	void save();

protected:
	void reload();
	void configChanged();

private:
	static CupsInfos	*unique_;

	TQString	host_;
	int	port_;
	TQString	login_;
	TQString	password_;
	TQString	reallogin_;
	bool savepwd_;

	int	count_;
};

inline const TQString& CupsInfos::host() const
{ return host_; }

inline int CupsInfos::port() const
{ return port_; }

inline const TQString& CupsInfos::login() const
{ return login_; }

inline const TQString& CupsInfos::password() const
{ return password_; }

inline const TQString& CupsInfos::realLogin() const
{ return reallogin_; }

inline bool CupsInfos::savePassword() const
{ return savepwd_; }

#endif

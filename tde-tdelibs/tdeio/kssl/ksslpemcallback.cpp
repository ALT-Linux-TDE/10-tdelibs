/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kpassdlg.h>
#include <tdelocale.h>
#include "ksslpemcallback.h"

int KSSLPemCallback(char *buf, int size, int rwflag, void *userdata) {
#ifdef KSSL_HAVE_SSL
	TQString pass2;
	Q_UNUSED(userdata);
	Q_UNUSED(rwflag);

	if (!buf) return -1;
	int rc = KPasswordDialog::getPassword(pass2, i18n("Certificate password"));
	if (rc != KPasswordDialog::Accepted) return -1;

	TQCString pass = pass2.utf8();  // utf8 length may differ from TQString length
	const uint passlen = pass.length();
	if (passlen > (unsigned int)size-1)
		pass.truncate((unsigned int)size-1);

	tqstrncpy(buf, pass, size-1);
  buf[size-1] = 0;
  pass.fill(' ');
  pass2.fill(' ');
	return (int)passlen;
#else
	Q_UNUSED(buf);
	Q_UNUSED(size);
	Q_UNUSED(rwflag);
	Q_UNUSED(userdata);
	return -1;
#endif
}



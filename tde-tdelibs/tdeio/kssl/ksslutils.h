/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
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
#ifndef _INCLUDE_KSSLUTILS_H
#define _INCLUDE_KSSLUTILS_H

#include <tdemacros.h>

#ifdef TQ_WS_WIN
#include "ksslconfig_win.h"
#else
#include "ksslconfig.h"
#endif

#include <unistd.h>
#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/x509.h>
#undef crypt
#endif
class TQString;
class TQDateTime;


#ifdef KSSL_HAVE_SSL
// This functionality is missing in OpenSSL
/**
 *  Convert an ASN1 UTCTIME value to a string.  Uses TDELocale settings.
 *
 *  @param tm the OpenSSL ASN1_UTCTIME pointer
 *
 *  @return the date formatted in a TQString
 *  @see ASN1_UTCTIME_QDateTime
 */
TDE_EXPORT TQString ASN1_UTCTIME_QString(ASN1_UTCTIME *tm);

/**
 *  Convert an ASN1 UTCTIME value to a TQDateTime.  Uses TDELocale settings.
 *
 *  @param tm the OpenSSL ASN1_UTCTIME pointer
 *  @param isGmt set to 1 if the date is set to GMT
 *
 *  @return the date formatted in a QDateTime
 */
TDE_EXPORT TQDateTime ASN1_UTCTIME_QDateTime(ASN1_UTCTIME *tm, int *isGmt);


/**
 *  Convert an ASN1 INTEGER value to a string.
 *
 *  @param aint the OpenSSL ASN1_INTEGER pointer
 *
 *  @return the number formatted in a TQString
 */
TDE_EXPORT TQString ASN1_INTEGER_QString(ASN1_INTEGER *aint);
#endif




#endif

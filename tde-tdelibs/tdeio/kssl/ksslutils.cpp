/* This file is part of the KDE project
 *
 * Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
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


#include "ksslutils.h"

#include <tqstring.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tqdatetime.h>

#include "kopenssl.h"

#ifdef KSSL_HAVE_SSL
// This code is mostly taken from OpenSSL v0.9.5a
// by Eric Young
TQDateTime ASN1_UTCTIME_QDateTime(ASN1_UTCTIME *tm, int *isGmt) {
TQDateTime qdt;
char *v;
int gmt=0;
int gentime=0;
int yoffset=0;
int yearbase=1900;
int i;
int y=0,M=0,d=0,h=0,m=0,s=0;
TQDate qdate;
TQTime qtime;
 
  i = tm->length;
  v = (char *)tm->data;
  if ((i == 15) || (i == 21)) {
          gentime=1;
          yoffset=2;
          yearbase=0;
  }
  if (i < 10) goto auq_err;
  if (v[i-1] == 'Z') gmt=1;
  for (i=0; i<10+yoffset; i++)
          if ((v[i] > '9') || (v[i] < '0')) goto auq_err;
  y = (v[0+yoffset]-'0')*10+(v[1+yoffset]-'0');
  if (gentime)
          y += (v[0]-'0')*1000+(v[1]-'0')*100;
  if (y < 50) y+=100;
  M = (v[2+yoffset]-'0')*10+(v[3+yoffset]-'0');
  if ((M > 12) || (M < 1)) goto auq_err;
  d = (v[4+yoffset]-'0')*10+(v[5+yoffset]-'0');
  h = (v[6+yoffset]-'0')*10+(v[7+yoffset]-'0');
  m =  (v[8+yoffset]-'0')*10+(v[9+yoffset]-'0');
  if (    (v[10+yoffset] >= '0') && (v[10+yoffset] <= '9') &&
          (v[11+yoffset] >= '0') && (v[11+yoffset] <= '9'))
          s = (v[10+yoffset]-'0')*10+(v[11+yoffset]-'0');

  // localize the date and display it.
  qdate.setYMD(y+yearbase, M, d);
  qtime.setHMS(h,m,s);
  qdt.setDate(qdate); qdt.setTime(qtime);
  auq_err:
  if (isGmt) *isGmt = gmt;
return qdt;
}


TQString ASN1_UTCTIME_QString(ASN1_UTCTIME *tm) {
  TQString qstr;
  int gmt;
  TQDateTime qdt = ASN1_UTCTIME_QDateTime(tm, &gmt);

  qstr = TDEGlobal::locale()->formatDateTime(qdt, false, true);
  if (gmt) { 
    qstr += " ";
    qstr += i18n("GMT");
  }
  return qstr;
}


TQString ASN1_INTEGER_QString(ASN1_INTEGER *aint) {
	char *rep = KOSSL::self()->i2s_ASN1_INTEGER(NULL, aint);
	TQString yy = rep;
	KOSSL::self()->CRYPTO_free(rep);
	return yy;
}


#endif


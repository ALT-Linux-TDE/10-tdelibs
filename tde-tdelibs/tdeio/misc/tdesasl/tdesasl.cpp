/* This file is part of the KDE libraries
   Copyright (C) 2001-2002 Michael H�ckel <haeckel@kde.org>
   $Id$

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "tdesasl.h"

#include <kmdcodec.h>
#include <kurl.h>

#include <tqstrlist.h>

#include <stdlib.h>
#include <string.h>

KDESasl::KDESasl(const KURL &aUrl)
{
  mProtocol = aUrl.protocol();
  mUser = aUrl.user();
  mPass = aUrl.pass();
  mFirst = true;
}

KDESasl::KDESasl(const TQString &aUser, const TQString &aPass,
  const TQString &aProtocol)
{
  mProtocol = aProtocol;
  mUser = aUser;
  mPass = aPass;
  mFirst = true;
}

KDESasl::~KDESasl() {
}

TQCString KDESasl::chooseMethod(const TQStrIList aMethods)
{
  if (aMethods.contains("DIGEST-MD5")) mMethod = "DIGEST-MD5";
  else if (aMethods.contains("CRAM-MD5")) mMethod = "CRAM-MD5";
  else if (aMethods.contains("PLAIN")) mMethod = "PLAIN";
  else if (aMethods.contains("LOGIN")) mMethod = "LOGIN";
  else mMethod = TQCString();
  return mMethod;
}

void KDESasl::setMethod(const TQCString &aMethod)
{
  mMethod = aMethod.upper();
}

TQByteArray KDESasl::getPlainResponse()
{
  TQCString user = mUser.utf8();
  TQCString pass = mPass.utf8();
  int userlen = user.length();
  int passlen = pass.length();
  // result = $user\0$user\0$pass (no trailing \0)
  TQByteArray result(2 * userlen + passlen + 2);
  if ( userlen ) {
    memcpy( result.data(), user.data(), userlen );
    memcpy( result.data() + userlen + 1, user.data(), userlen );
  }
  if ( passlen )
    memcpy( result.data() + 2 * userlen + 2, pass.data(), passlen );
  result[userlen] = result[2*userlen+1] = '\0';
  return result;
}

TQByteArray KDESasl::getLoginResponse()
{
  TQByteArray result = (mFirst) ? mUser.utf8() : mPass.utf8();
  mFirst = !mFirst;
  if (result.size()) result.resize(result.size() - 1);
  return result;
}

TQByteArray KDESasl::getCramMd5Response(const TQByteArray &aChallenge)
{
  uint i;
  TQByteArray secret = mPass.utf8();
  int len = mPass.utf8().length();
  secret.resize(len);
  if (secret.size() > 64)
  {
    KMD5 md5(secret);
    secret.duplicate((const char*)(&(md5.rawDigest()[0])), 16);
    len = 16;
  }
  secret.resize(64);
  for (i = len; i < 64; i++) secret[i] = 0;
  TQByteArray XorOpad(64);
  for (i = 0; i < 64; i++) XorOpad[i] = secret[i] ^ 0x5C;
  TQByteArray XorIpad(64);
  for (i = 0; i < 64; i++) XorIpad[i] = secret[i] ^ 0x36;
  KMD5 md5;
  md5.update(XorIpad);
  md5.update(aChallenge);
  KMD5 md5a;
  md5a.update(XorOpad);
  md5a.update(md5.rawDigest(), 16);
  TQByteArray result = mUser.utf8();
  len = mUser.utf8().length();
  result.resize(len + 33);
  result[len] = ' ';
  TQCString ch = md5a.hexDigest();
  for (i = 0; i < 32; i++) result[i+len+1] = *(ch.data() + i);
  return result;
}

TQByteArray KDESasl::getDigestMd5Response(const TQByteArray &aChallenge)
{
  mFirst = !mFirst;
  if (mFirst) return TQByteArray();
  TQCString str, realm, nonce, qop, algorithm, charset;
  TQCString nc = "00000001";
  unsigned int a, b, c, d;
  a = 0;
  while (a < aChallenge.size())
  {
    b = a;
    while (b < aChallenge.size() && aChallenge[b] != '=') b++;
    c = b + 1;
    if (aChallenge[c] == '"')
    {
      d = c + 1;
      while (d < aChallenge.size() && aChallenge[d] != '"') d++;
      c++;
    } else {
      d = c;
      while (d < aChallenge.size() && aChallenge[d] != ',') d++;
    }
    str = TQCString(aChallenge.data() + c, d - c + 1);
    if (tqstrnicmp(aChallenge.data() + a, "realm=", 6) == 0) realm = str;
    else if (tqstrnicmp(aChallenge.data() + a, "nonce=", 6) == 0) nonce = str;
    else if (tqstrnicmp(aChallenge.data() + a, "qop=", 4) == 0) qop = str;
    else if (tqstrnicmp(aChallenge.data() + a, "algorithm=", 10) == 0)
      algorithm = str;
    else if (tqstrnicmp(aChallenge.data() + a, "charset=", 8) == 0)
      charset = str;
    a = (d < aChallenge.size() && aChallenge[d] == '"') ? d + 2 : d + 1;
  }
  if (qop.isEmpty()) qop = "auth";
  qop = "auth";
  bool utf8 = tqstricmp(charset, "utf-8") == 0;
  TQCString digestUri = TQCString(mProtocol.latin1()) + "/" + realm;

  /* Calculate the response */
  /* Code based on code from the http io-slave
     Copyright (C) 2000,2001 Dawit Alemayehu <adawit@kde.org>
     Copyright (C) 2000,2001 Waldo Bastian <bastian@kde.org>
     Copyright (C) 2000,2001 George Staikos <staikos@kde.org> */
  KMD5 md, md2;
  TQCString HA1, HA2;
  TQCString cnonce;
  cnonce.setNum((1 + static_cast<int>(100000.0*rand()/(RAND_MAX+1.0))));
  cnonce = KCodecs::base64Encode( cnonce );

  // Calculate H(A1)
  TQCString authStr = (utf8) ? mUser.utf8() : TQCString(mUser.latin1());
  authStr += ':';
  authStr += realm;
  authStr += ':';
  authStr += (utf8) ? mPass.utf8() : TQCString(mPass.latin1());

  md.update( authStr );
  authStr = "";
  if ( algorithm == "md5-sess" )
  {
    authStr += ':';
    authStr += nonce;
    authStr += ':';
    authStr += cnonce;
  }
  md2.reset();
  /* SASL authentication uses rawDigest here, whereas HTTP authentication uses
     hexDigest() */
  md2.update(md.rawDigest(), 16);
  md2.update( authStr );
  md2.hexDigest( HA1 );

  // Calcualte H(A2)
  authStr = "AUTHENTICATE:";
  authStr += digestUri;
  if ( qop == "auth-int" || qop == "auth-conf" )
  {
    authStr += ":00000000000000000000000000000000";
  }
  md.reset();
  md.update( authStr );
  md.hexDigest( HA2 );

  // Calcualte the response.
  authStr = HA1;
  authStr += ':';
  authStr += nonce;
  authStr += ':';
  if ( !qop.isEmpty() )
  {
    authStr += nc;
    authStr += ':';
    authStr += cnonce;
    authStr += ':';
    authStr += qop;
    authStr += ':';
  }
  authStr += HA2;
  md.reset();
  md.update( authStr );
  TQCString response = md.hexDigest();
  /* End of response calculation */

  TQCString result;
  if (utf8)
  {
    result = "charset=utf-8,username=\"" + mUser.utf8();
  } else {
    result = "charset=iso-8859-1,username=\"" + TQCString(mUser.latin1());
  }
  result += "\",realm=\"" + realm + "\",nonce=\"" + nonce;
  result += "\",nc=" + nc + ",cnonce=\"" + cnonce;
  result += "\",digest-uri=\"" + digestUri;
  result += "\",response=" + response + ",qop=" + qop;
  TQByteArray ba;
  ba.duplicate(result.data(), result.length());
  return ba;
}

TQByteArray KDESasl::getBinaryResponse(const TQByteArray &aChallenge, bool aBase64)
{
  if (aBase64)
  {
    TQByteArray ba;
    KCodecs::base64Decode(aChallenge, ba);
    KCodecs::base64Encode(getBinaryResponse(ba, false), ba);
    return ba;
  }
  if (tqstricmp(mMethod, "PLAIN") == 0) return getPlainResponse();
  if (tqstricmp(mMethod, "LOGIN") == 0) return getLoginResponse();
  if (tqstricmp(mMethod, "CRAM-MD5") == 0)
    return getCramMd5Response(aChallenge);
  if (tqstricmp(mMethod, "DIGEST-MD5") == 0)
    return getDigestMd5Response(aChallenge);
//    return getDigestMd5Response(TQCString("realm=\"elwood.innosoft.com\",nonce=\"OA6MG9tEQGm2hh\",qop=\"auth\",algorithm=md5-sess,charset=utf-8"));
  return TQByteArray();
}

TQCString KDESasl::getResponse(const TQByteArray &aChallenge, bool aBase64)
{
  TQByteArray ba = getBinaryResponse(aChallenge, aBase64);
  return TQCString(ba.data(), ba.size() + 1);
}

TQCString KDESasl::method() const {
  return mMethod;
}

bool KDESasl::clientStarts() const {
  return method() == "PLAIN";
}

bool KDESasl::dialogComplete( int n ) const {
  if ( method() == "PLAIN" || method() == "CRAM-MD5" )
    return n >= 1;
  if ( method() == "LOGIN" || method() == "DIGEST-MD5" )
    return n >= 2;
  return true;
}

bool KDESasl::isClearTextMethod() const {
  return method() == "PLAIN" || method() == "LOGIN" ;
}

//  dataprotocol.cpp
// ==================
//
// Implementation of the data protocol (rfc 2397)
//
// Author: Leo Savernik
// Email: l.savernik@aon.at
// (C) 2002, 2003 by Leo Savernik
// Created: Sam Dez 28 14:11:18 CET 2002

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; version 2.                 *
 *                                                                         *
 ***************************************************************************/

#include "dataprotocol.h"

#include <kdebug.h>
#include <kmdcodec.h>
#include <kurl.h>
#include <tdeio/global.h>

#include <tqcstring.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqtextcodec.h>

#ifdef DATAKIOSLAVE
#  include <kinstance.h>
#  include <stdlib.h>
#endif
#ifdef TESTKIO
#  include <iostream>
#endif

#if !defined(DATAKIOSLAVE) && !defined(TESTKIO)
#  define DISPATCH(f) dispatch_##f
#else
#  define DISPATCH(f) f
#endif

using namespace TDEIO;
#ifdef DATAKIOSLAVE
extern "C" {

  int kdemain( int argc, char **argv ) {
    TDEInstance instance( "tdeio_data" );

    kdDebug(7101) << "*** Starting tdeio_data " << endl;

    if (argc != 4) {
      kdDebug(7101) << "Usage: tdeio_data  protocol domain-socket1 domain-socket2" << endl;
      exit(-1);
    }

    DataProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kdDebug(7101) << "*** tdeio_data Done" << endl;
    return 0;
  }
}
#endif

/** structure containing header information */
struct DataHeader {
  TQString mime_type;		// mime type of content (lowercase)
  MetaData attributes;		// attribute/value pairs (attribute lowercase,
  				// 	value unchanged)
  bool is_base64;		// true if data is base64 encoded
  TQString url;			// reference to decoded url
  int data_offset;		// zero-indexed position within url
  				// where the real data begins. May point beyond
      				// the end to indicate that there is no data
  TQString *charset;		// shortcut to charset (it always exists)
};

// constant string data
const TQChar text_plain_str[] = { 't','e','x','t','/','p','l','a','i','n' };
const TQChar charset_str[] = { 'c','h','a','r','s','e','t' };
const TQChar us_ascii_str[] = { 'u','s','-','a','s','c','i','i' };
const TQChar base64_str[] = { 'b','a','s','e','6','4' };

/** returns the position of the first occurrence of any of the given characters
  * @p c1 to @p c3 or buf.length() if none is contained.
  * @param buf buffer where to look for c
  * @param begin zero-indexed starting position
  * @param c1 character to find
  * @param c2 alternative character to find or '\0' to ignore
  * @param c3 alternative character to find or '\0' to ignore
  */
static int find(const TQString &buf, int begin, TQChar c1, TQChar c2 = '\0',
		TQChar c3 = '\0') {
  int pos = begin;
  int size = (int)buf.length();
  while (pos < size) {
    TQChar ch = buf[pos];
    if (ch == c1
    	|| (c2 != '\0' && ch == c2)
	|| (c3 != '\0' && ch == c3))
      break;
    pos++;
  }/*wend*/
  return pos;
}

/** extracts the string between the current position @p pos and the first
 * occurrence of either @p c1 to @p c3 exclusively and updates @p pos
 * to point at the found delimiter or at the end of the buffer if
 * neither character occurred.
 * @param buf buffer where to look for
 * @param pos zero-indexed position within buffer
 * @param c1 character to find
 * @param c2 alternative character to find or 0 to ignore
 * @param c3 alternative character to find or 0 to ignore
 */
inline TQString extract(const TQString &buf, int &pos, TQChar c1,
		TQChar c2 = '\0', TQChar c3 = '\0') {
  int oldpos = pos;
  pos = find(buf,oldpos,c1,c2,c3);
  return TQString(buf.unicode() + oldpos, pos - oldpos);
}

/** ignores all whitespaces
 * @param buf buffer to operate on
 * @param pos position to shift to first non-whitespace character
 *	Upon return @p pos will either point to the first non-whitespace
 *	character or to the end of the buffer.
 */
inline void ignoreWS(const TQString &buf, int &pos) {
  int size = (int)buf.length();
  TQChar ch = buf[pos];
  while (pos < size && (ch == ' ' || ch == '\t' || ch == '\n'
  	|| ch == '\r'))
    ch = buf[++pos];
}

/** parses a quoted string as per rfc 822.
 *
 * If trailing quote is missing, the whole rest of the buffer is returned.
 * @param buf buffer to operate on
 * @param pos position pointing to the leading quote
 * @return the extracted string. @p pos will be updated to point to the
 * 	character following the trailing quote.
 */
static TQString parseQuotedString(const TQString &buf, int &pos) {
  int size = (int)buf.length();
  TQString res;
  pos++;		// jump over leading quote
  bool escaped = false;	// if true means next character is literal
  bool parsing = true;	// true as long as end quote not found
  while (parsing && pos < size) {
    TQChar ch = buf[pos++];
    if (escaped) {
      res += ch;
      escaped = false;
    } else {
      switch (ch) {
        case '"': parsing = false; break;
        case '\\': escaped = true; break;
        default: res += ch; break;
      }/*end switch*/
    }/*end if*/
  }/*wend*/
  return res;
}

/** parses the header of a data url
 * @param url the data url
 * @param header_info fills the given DataHeader structure with the header
 *		information
 */
static void parseDataHeader(const KURL &url, DataHeader &header_info) {
  TQConstString text_plain(text_plain_str,sizeof text_plain_str/sizeof text_plain_str[0]);
  TQConstString charset(charset_str,sizeof charset_str/sizeof charset_str[0]);
  TQConstString us_ascii(us_ascii_str,sizeof us_ascii_str/sizeof us_ascii_str[0]);
  TQConstString base64(base64_str,sizeof base64_str/sizeof base64_str[0]);
  // initialize header info members
  header_info.mime_type = text_plain.string();
  header_info.charset = &header_info.attributes.insert(
  			charset.string(),us_ascii.string())
		.data();
  header_info.is_base64 = false;

  // decode url and save it
  TQString &raw_url = header_info.url = TQString::fromLatin1("data:") + url.path();
  int raw_url_len = (int)raw_url.length();

  // jump over scheme part (must be "data:", we don't even check that)
  header_info.data_offset = raw_url.find(':');
  header_info.data_offset++;	// jump over colon or to begin if scheme was missing

  // read mime type
  if (header_info.data_offset >= raw_url_len) return;
  TQString mime_type = extract(raw_url,header_info.data_offset,';',',')
  			.stripWhiteSpace();
  if (!mime_type.isEmpty()) header_info.mime_type = mime_type;

  if (header_info.data_offset >= raw_url_len) return;
  // jump over delimiter token and return if data reached
  if (raw_url[header_info.data_offset++] == ',') return;

  // read all attributes and store them
  bool data_begin_reached = false;
  while (!data_begin_reached && header_info.data_offset < raw_url_len) {
    // read attribute
    TQString attribute = extract(raw_url,header_info.data_offset,'=',';',',')
    			.stripWhiteSpace();
    if (header_info.data_offset >= raw_url_len
    	|| raw_url[header_info.data_offset] != '=') {
      // no assigment, must be base64 option
      if (attribute == base64.string())
        header_info.is_base64 = true;
    } else {
      header_info.data_offset++; // jump over '=' token

      // read value
      ignoreWS(raw_url,header_info.data_offset);
      if (header_info.data_offset >= raw_url_len) return;

      TQString value;
      if (raw_url[header_info.data_offset] == '"') {
        value = parseQuotedString(raw_url,header_info.data_offset);
        ignoreWS(raw_url,header_info.data_offset);
      } else
        value = extract(raw_url,header_info.data_offset,';',',')
        		.stripWhiteSpace();

      // add attribute to map
      header_info.attributes[attribute.lower()] = value;

    }/*end if*/
    if (header_info.data_offset < raw_url_len
	&& raw_url[header_info.data_offset] == ',')
      data_begin_reached = true;
    header_info.data_offset++; // jump over separator token
  }/*wend*/
}

#ifdef DATAKIOSLAVE
DataProtocol::DataProtocol(const TQCString &pool_socket, const TQCString &app_socket)
	: SlaveBase("tdeio_data", pool_socket, app_socket) {
#else
DataProtocol::DataProtocol() {
#endif
  kdDebug() << "DataProtocol::DataProtocol()" << endl;
}

/* --------------------------------------------------------------------- */

DataProtocol::~DataProtocol() {
  kdDebug() << "DataProtocol::~DataProtocol()" << endl;
}

/* --------------------------------------------------------------------- */

void DataProtocol::get(const KURL& url) {
  ref();
  //kdDebug() << "===============================================================================================================================================================================" << endl;
  kdDebug() << "tdeio_data@"<<this<<"::get(const KURL& url)" << endl ;

  DataHeader hdr;
  parseDataHeader(url,hdr);

  int size = (int)hdr.url.length();
  int data_ofs = TQMIN(hdr.data_offset,size);
  // FIXME: string is copied, would be nice if we could have a reference only
  TQString url_data = hdr.url.mid(data_ofs);
  TQCString outData;

#ifdef TESTKIO
//  cout << "current charset: \"" << *hdr.charset << "\"" << endl;
#endif
  if (hdr.is_base64) {
    // base64 stuff is expected to contain the correct charset, so we just
    // decode it and pass it to the receiver
    KCodecs::base64Decode(url_data.local8Bit(),outData);
  } else {
    // FIXME: This is all flawed, must be reworked thoroughly
    // non encoded data must be converted to the given charset
    TQTextCodec *codec = TQTextCodec::codecForName(hdr.charset->latin1());
    if (codec != 0) {
      outData = codec->fromUnicode(url_data);
    } else {
      // if there is no approprate codec, just use local encoding. This
      // should work for >90% of all cases.
      outData = url_data.local8Bit();
    }/*end if*/
  }/*end if*/

  //kdDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
  //kdDebug() << "emit mimeType@"<<this << endl ;
  mimeType(hdr.mime_type);
  //kdDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
  //kdDebug() << "emit totalSize@"<<this << endl ;
  totalSize(outData.size());

  //kdDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
  //kdDebug() << "emit setMetaData@"<<this << endl ;
#if defined(TESTKIO) || defined(DATAKIOSLAVE)
  MetaData::ConstIterator it;
  for (it = hdr.attributes.begin(); it != hdr.attributes.end(); ++it) {
    setMetaData(it.key(),it.data());
  }/*next it*/
#else
  setAllMetaData(hdr.attributes);
#endif

  //kdDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
  //kdDebug() << "emit sendMetaData@"<<this << endl ;
  sendMetaData();
  //kdDebug() << "^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C" << endl;
//   kdDebug() << "(1) queue size " << dispatchQueue.size() << endl;
  // empiric studies have shown that this shouldn't be queued & dispatched
  /*DISPATCH*/(data(outData));
//   kdDebug() << "(2) queue size " << dispatchQueue.size() << endl;
  DISPATCH(data(TQByteArray()));
//   kdDebug() << "(3) queue size " << dispatchQueue.size() << endl;
  DISPATCH(finished());
//   kdDebug() << "(4) queue size " << dispatchQueue.size() << endl;
  deref();
}

/* --------------------------------------------------------------------- */

void DataProtocol::mimetype(const KURL &url) {
  ref();
  DataHeader hdr;
  parseDataHeader(url,hdr);
  mimeType(hdr.mime_type);
  finished();
  deref();
}

/* --------------------------------------------------------------------- */


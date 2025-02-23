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

#ifndef IPPREQUEST_H
#define IPPREQUEST_H

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqtextstream.h>
#include <tqmap.h>

#include <cups/ipp.h>

#include "config.h"

class IppRequest
{
public:
	IppRequest();
	~IppRequest();

	void init();	// re-initializes the request

	// request building functions
	void addMime(int group, const TQString& name, const TQString& mime);
	void addKeyword(int group, const TQString& name, const TQString& key);
	void addKeyword(int group, const TQString& name, const TQStringList& keys);
	void addURI(int group, const TQString& name, const TQString& uri);
	void addURI(int group, const TQString& name, const TQStringList& uris);
	void addText(int group, const TQString& name, const TQString& txt);
	void addText(int group, const TQString& name, const TQStringList& txts);
	void addName(int group, const TQString& name, const TQString& nm);
	void addName(int group, const TQString& name, const TQStringList& nms);
	void addInteger(int group, const TQString& name, int value);
	void addInteger(int group, const TQString& name, const TQValueList<int>& values);
	void addEnum(int group, const TQString& name, int value);
	void addEnum(int group, const TQString& name, const TQValueList<int>& values);
	void addBoolean(int group, const TQString& name, bool value);
	void addBoolean(int group, const TQString& name, const TQValueList<bool>& values);

	void setOperation(int op);
	void setHost(const TQString& host);
	void setPort(int p);

	// request answer functions
	int status();
	TQString statusMessage();
	bool integer(const TQString& name, int& value);
	bool boolean(const TQString& name, bool& value);
	bool enumvalue(const TQString& name, int& value);
	bool name(const TQString& name, TQString& value);
	bool name(const TQString& name, TQStringList& value);
	bool text(const TQString& name, TQString& value);
	bool text(const TQString& name, TQStringList& value);
	bool uri(const TQString& name, TQString& value);
	bool uri(const TQString& name, TQStringList& value);
	bool keyword(const TQString& name, TQString& value);
	bool keyword(const TQString& name, TQStringList& value);
	bool mime(const TQString& name, TQString& value);
	ipp_attribute_t* first();
#ifndef HAVE_CUPS_1_6
	ipp_attribute_t* last();
#endif // HAVE_CUPS_1_6
	ipp_t* request();
	TQMap<TQString,TQString> toMap(int group = -1);
	void setMap(const TQMap<TQString,TQString>& opts);

	// processing functions
	bool doRequest(const TQString& res);
	bool doFileRequest(const TQString& res, const TQString& filename = TQString::null);

	// report functions
	bool htmlReport(int group, TQTextStream& output);

	// debug function
	void dump(int state);

protected:
	void addString_p(int group, int type, const TQString& name, const TQString& value);
	void addStringList_p(int group, int type, const TQString& name, const TQStringList& values);
	void addInteger_p(int group, int type, const TQString& name, int value);
	void addIntegerList_p(int group, int type, const TQString& name, const TQValueList<int>& values);
	bool stringValue_p(const TQString& name, TQString& value, int type);
	bool stringListValue_p(const TQString& name, TQStringList& values, int type);
	bool integerValue_p(const TQString& name, int& value, int type);

private:
	ipp_t	*request_;
	TQString	host_;
	int 	port_;
	bool	connect_;
	int	dump_;
};

inline void IppRequest::addMime(int group, const TQString& name, const TQString& mime)
{ addString_p(group, IPP_TAG_MIMETYPE, name, mime); }

inline void IppRequest::addKeyword(int group, const TQString& name, const TQString& key)
{ addString_p(group, IPP_TAG_KEYWORD, name, key); }

inline void IppRequest::addKeyword(int group, const TQString& name, const TQStringList& keys)
{ addStringList_p(group, IPP_TAG_KEYWORD, name, keys); }

inline void IppRequest::addURI(int group, const TQString& name, const TQString& uri)
{ addString_p(group, IPP_TAG_URI, name, uri); }

inline void IppRequest::addURI(int group, const TQString& name, const TQStringList& uris)
{ addStringList_p(group, IPP_TAG_URI, name, uris); }

inline void IppRequest::addText(int group, const TQString& name, const TQString& txt)
{ addString_p(group, IPP_TAG_TEXT, name, txt); }

inline void IppRequest::addText(int group, const TQString& name, const TQStringList& txts)
{ addStringList_p(group, IPP_TAG_TEXT, name, txts); }

inline void IppRequest::addName(int group, const TQString& name, const TQString& nm)
{ addString_p(group, IPP_TAG_NAME, name, nm); }

inline void IppRequest::addName(int group, const TQString& name, const TQStringList& nms)
{ addStringList_p(group, IPP_TAG_NAME, name, nms); }

inline void IppRequest::addInteger(int group, const TQString& name, int value)
{ addInteger_p(group, IPP_TAG_INTEGER, name, value); }

inline void IppRequest::addInteger(int group, const TQString& name, const TQValueList<int>& values)
{ addIntegerList_p(group, IPP_TAG_INTEGER, name, values); }

inline void IppRequest::addEnum(int group, const TQString& name, int value)
{ addInteger_p(group, IPP_TAG_ENUM, name, value); }

inline void IppRequest::addEnum(int group, const TQString& name, const TQValueList<int>& values)
{ addIntegerList_p(group, IPP_TAG_ENUM, name, values); }

inline bool IppRequest::integer(const TQString& name, int& value)
{ return integerValue_p(name, value, IPP_TAG_INTEGER); }

inline bool IppRequest::enumvalue(const TQString& name, int& value)
{ return integerValue_p(name, value, IPP_TAG_ENUM); }

inline bool IppRequest::name(const TQString& name, TQString& value)
{ return stringValue_p(name, value, IPP_TAG_NAME); }

inline bool IppRequest::name(const TQString& name, TQStringList& values)
{ return stringListValue_p(name, values, IPP_TAG_NAME); }

inline bool IppRequest::text(const TQString& name, TQString& value)
{ return stringValue_p(name, value, IPP_TAG_TEXT); }

inline bool IppRequest::text(const TQString& name, TQStringList& values)
{ return stringListValue_p(name, values, IPP_TAG_TEXT); }

inline bool IppRequest::uri(const TQString& name, TQString& value)
{ return stringValue_p(name, value, IPP_TAG_URI); }

inline bool IppRequest::uri(const TQString& name, TQStringList& values)
{ return stringListValue_p(name, values, IPP_TAG_URI); }

inline bool IppRequest::keyword(const TQString& name, TQString& value)
{ return stringValue_p(name, value, IPP_TAG_KEYWORD); }

inline bool IppRequest::keyword(const TQString& name, TQStringList& values)
{ return stringListValue_p(name, values, IPP_TAG_KEYWORD); }

inline bool IppRequest::mime(const TQString& name, TQString& value)
{ return stringValue_p(name, value, IPP_TAG_MIMETYPE); }

inline bool IppRequest::doRequest(const TQString& res)
{ return doFileRequest(res); }

#ifndef HAVE_CUPS_1_6
inline ipp_attribute_t* IppRequest::last()
{ return (request_ ? request_->last : NULL); }
#endif // HAVE_CUPS_1_6

inline void IppRequest::setHost(const TQString& host)
{ host_ = host; }

inline void IppRequest::setPort(int p)
{ port_ = p; }

inline void IppRequest::dump(int state)
{ dump_ = state; }

inline ipp_t* IppRequest::request()
{ return request_; }

#endif

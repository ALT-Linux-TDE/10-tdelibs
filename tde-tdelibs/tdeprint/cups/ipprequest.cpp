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

#include "ipprequest.h"
#include "cupsinfos.h"

#include <stdlib.h>
#include <string>
#include <cups/language.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tqdatetime.h>
#include <tqregexp.h>
#include <cups/cups.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_CUPS_NO_PWD_CACHE
#include <tqcstring.h>
static TQCString cups_authstring = "";
#endif

void dumpRequest(ipp_t *req, bool answer = false, const TQString& s = TQString::null)
{
	kdDebug(500) << "==========" << endl;
	if (s.isEmpty())
		kdDebug(500) << (answer ? "Answer" : "Request") << endl;
	else
		kdDebug(500) << s << endl;
	kdDebug(500) << "==========" << endl;
	if (!req)
	{
		kdDebug(500) << "Null request" << endl;
		return;
	}
#ifdef HAVE_CUPS_1_6
	kdDebug(500) << "State = 0x" << TQString::number(ippGetState(req), 16) << endl;
	kdDebug(500) << "ID = 0x" << TQString::number(ippGetRequestId(req), 16) << endl;
	if (answer)
	{
		kdDebug(500) << "Status = 0x" << TQString::number(ippGetStatusCode(req), 16) << endl;
		kdDebug(500) << "Status message = " << ippErrorString(ippGetStatusCode(req)) << endl;
	}
	else
		kdDebug(500) << "Operation = 0x" << TQString::number(ippGetOperation(req), 16) << endl;
	int minorVersion;
	int majorVersion = ippGetVersion(req, &minorVersion);
	kdDebug(500) << "Version = " << (int)(majorVersion) << "." << (int)(minorVersion) << endl;
	kdDebug(500) << endl;

	ipp_attribute_t *attr = ippFirstAttribute(req);
	while (attr)
	{
		TQString s = TQString::fromLatin1("%1 (0x%2) = ").arg(ippGetName(attr)).arg(ippGetValueTag(attr), 0, 16);
		for (int i=0;i<ippGetCount(attr);i++)
		{
			switch (ippGetValueTag(attr))
			{
				case IPP_TAG_INTEGER:
				case IPP_TAG_ENUM:
					s += ("0x"+TQString::number(ippGetInteger(attr, i), 16));
					break;
				case IPP_TAG_BOOLEAN:
					s += (ippGetBoolean(attr, i) ? "true" : "false");
					break;
				case IPP_TAG_STRING:
				case IPP_TAG_TEXT:
				case IPP_TAG_NAME:
				case IPP_TAG_KEYWORD:
				case IPP_TAG_URI:
				case IPP_TAG_MIMETYPE:
				case IPP_TAG_NAMELANG:
				case IPP_TAG_TEXTLANG:
				case IPP_TAG_CHARSET:
				case IPP_TAG_LANGUAGE:
					s += ippGetString(attr, i, NULL);
					break;
				default:
					break;
			}
			if (i != (ippGetCount(attr)-1))
				s += ", ";
		}
		kdDebug(500) << s << endl;
		attr = ippNextAttribute(req);
	}
#else
	kdDebug(500) << "State = 0x" << TQString::number(req->state, 16) << endl;
	kdDebug(500) << "ID = 0x" << TQString::number(req->request.status.request_id, 16) << endl;
	if (answer)
	{
		kdDebug(500) << "Status = 0x" << TQString::number(req->request.status.status_code, 16) << endl;
		kdDebug(500) << "Status message = " << ippErrorString(req->request.status.status_code) << endl;
	}
	else
		kdDebug(500) << "Operation = 0x" << TQString::number(req->request.op.operation_id, 16) << endl;
	kdDebug(500) << "Version = " << (int)(req->request.status.version[0]) << "." << (int)(req->request.status.version[1]) << endl;
	kdDebug(500) << endl;

	ipp_attribute_t *attr = req->attrs;
	while (attr)
	{
		TQString s = TQString::fromLatin1("%1 (0x%2) = ").arg(attr->name).arg(attr->value_tag, 0, 16);
		for (int i=0;i<attr->num_values;i++)
		{
			switch (attr->value_tag)
			{
				case IPP_TAG_INTEGER:
				case IPP_TAG_ENUM:
					s += ("0x"+TQString::number(attr->values[i].integer, 16));
					break;
				case IPP_TAG_BOOLEAN:
					s += (attr->values[i].boolean ? "true" : "false");
					break;
				case IPP_TAG_STRING:
				case IPP_TAG_TEXT:
				case IPP_TAG_NAME:
				case IPP_TAG_KEYWORD:
				case IPP_TAG_URI:
				case IPP_TAG_MIMETYPE:
				case IPP_TAG_NAMELANG:
				case IPP_TAG_TEXTLANG:
				case IPP_TAG_CHARSET:
				case IPP_TAG_LANGUAGE:
					s += attr->values[i].string.text;
					break;
				default:
					break;
			}
			if (i != (attr->num_values-1))
				s += ", ";
		}
		kdDebug(500) << s << endl;
		attr = attr->next;
	}
#endif
}

TQString errorString(int status)
{
	TQString	str;
	switch (status)
	{
		case IPP_FORBIDDEN:
			str = i18n("You don't have access to the requested resource.");
			break;
		case IPP_NOT_AUTHORIZED:
			str = i18n("You are not authorized to access the requested resource.");
			break;
		case IPP_NOT_POSSIBLE:
			str = i18n("The requested operation cannot be completed.");
			break;
		case IPP_SERVICE_UNAVAILABLE:
			str = i18n("The requested service is currently unavailable.");
			break;
		case IPP_NOT_ACCEPTING:
			str = i18n("The target printer is not accepting print jobs.");
			break;
		default:
			str = TQString::fromLocal8Bit(ippErrorString((ipp_status_t)status));
			break;
	}
	return str;
}

//*************************************************************************************

IppRequest::IppRequest()
{
	request_ = 0;
	port_ = -1;
	host_ = TQString();
	dump_ = 0;
	init();
}

IppRequest::~IppRequest()
{
	ippDelete(request_);
}

void IppRequest::init()
{
	connect_ = true;

	if (request_)
	{
		ippDelete(request_);
		request_ = 0;
	}
	request_ = ippNew();
	//kdDebug(500) << "tdeprint: IPP request, lang=" << TDEGlobal::locale()->language() << endl;
        TQCString langstr = TDEGlobal::locale()->language().latin1();
	cups_lang_t*	lang = cupsLangGet(langstr.data());
	// default charset to UTF-8 (ugly hack)
	lang->encoding = CUPS_UTF8;
	ippAddString(request_, IPP_TAG_OPERATION, IPP_TAG_CHARSET, "attributes-charset", NULL, cupsLangEncoding(lang));
	ippAddString(request_, IPP_TAG_OPERATION, IPP_TAG_LANGUAGE, "attributes-natural-language", NULL, lang->language);
	cupsLangFree(lang);
}

void IppRequest::addString_p(int group, int type, const TQString& name, const TQString& value)
{
	if (!name.isEmpty())
		ippAddString(request_,(ipp_tag_t)group,(ipp_tag_t)type,name.latin1(),NULL,(value.isEmpty() ? "" : value.local8Bit().data()));
}

void IppRequest::addStringList_p(int group, int type, const TQString& name, const TQStringList& values)
{
	if (!name.isEmpty())
	{
		//> Values buffer and references offset prepare
		const char *vlsRefs[values.count()];
		std::string vlsBuf;
		for(unsigned i_vl = 0; i_vl < values.count(); i_vl++)
		{
		    vlsRefs[i_vl] = (const char*)vlsBuf.size();
		    vlsBuf += values[i_vl].local8Bit();
		    vlsBuf += (char)0;
		}
		//> References update to pointers
		for(unsigned i_vl = 0; i_vl < values.count(); i_vl++)
		    vlsRefs[i_vl] = vlsBuf.data()+(intptr_t)vlsRefs[i_vl];
		ippAddStrings(request_,(ipp_tag_t)group,(ipp_tag_t)type,name.latin1(),(int)(values.count()),NULL,(const char**)&vlsRefs);
	}
}

void IppRequest::addInteger_p(int group, int type, const TQString& name, int value)
{
	if (!name.isEmpty()) ippAddInteger(request_,(ipp_tag_t)group,(ipp_tag_t)type,name.latin1(),value);
}

void IppRequest::addIntegerList_p(int group, int type, const TQString& name, const TQValueList<int>& values)
{
	if (!name.isEmpty())
	{
		ipp_attribute_t	*attr = ippAddIntegers(request_,(ipp_tag_t)group,(ipp_tag_t)type,name.latin1(),(int)(values.count()),NULL);
		int	i(0);
		for (TQValueList<int>::ConstIterator it=values.begin(); it != values.end(); ++it, i++)
#ifdef HAVE_CUPS_1_6
			ippSetInteger(request_, &attr, i, *it);
#else
			attr->values[i].integer = *it;
#endif
	}
}

void IppRequest::addBoolean(int group, const TQString& name, bool value)
{
	if (!name.isEmpty()) ippAddBoolean(request_,(ipp_tag_t)group,name.latin1(),(char)value);
}

void IppRequest::addBoolean(int group, const TQString& name, const TQValueList<bool>& values)
{
	if (!name.isEmpty())
	{
		ipp_attribute_t	*attr = ippAddBooleans(request_,(ipp_tag_t)group,name.latin1(),(int)(values.count()),NULL);
		int	i(0);
		for (TQValueList<bool>::ConstIterator it=values.begin(); it != values.end(); ++it, i++)
#ifdef HAVE_CUPS_1_6
			ippSetBoolean(request_, &attr, i, (char)(*it));
#else
			attr->values[i].boolean = (char)(*it);
#endif
	}
}

void IppRequest::setOperation(int op)
{
#ifdef HAVE_CUPS_1_6
	ippSetOperation(request_, (ipp_op_t)op);
	ippSetRequestId(request_, 1);		// 0 is not RFC-compliant, should be at least 1
#else
	request_->request.op.operation_id = (ipp_op_t)op;
	request_->request.op.request_id = 1;	// 0 is not RFC-compliant, should be at least 1
#endif
}

int IppRequest::status()
{
#ifdef HAVE_CUPS_1_6
	return (request_ ? ippGetStatusCode(request_) : (connect_ ? cupsLastError() : -2));
#else
	return (request_ ? request_->request.status.status_code : (connect_ ? cupsLastError() : -2));
#endif
}

TQString IppRequest::statusMessage()
{
	TQString	msg;
	switch (status())
	{
		case -2:
			msg = i18n("Connection to CUPS server failed. Check that the CUPS server is correctly installed and running.");
			break;
		case -1:
			msg = i18n("The IPP request failed for an unknown reason.");
			break;
		default:
			msg = errorString(status());
			break;
	}
	return msg;
}

bool IppRequest::integerValue_p(const TQString& name, int& value, int type)
{
	if (!request_ || name.isEmpty()) return false;
	ipp_attribute_t	*attr = ippFindAttribute(request_, name.latin1(), (ipp_tag_t)type);
	if (attr)
	{
#ifdef HAVE_CUPS_1_6
		value = ippGetInteger(attr, 0);
#else
		value = attr->values[0].integer;
#endif
		return true;
	}
	else return false;
}

bool IppRequest::stringValue_p(const TQString& name, TQString& value, int type)
{
	if (!request_ || name.isEmpty()) return false;
	ipp_attribute_t	*attr = ippFindAttribute(request_, name.latin1(), (ipp_tag_t)type);
	if (attr)
	{
#ifdef HAVE_CUPS_1_6
		value = TQString::fromLocal8Bit(ippGetString(attr, 0, NULL));
#else
		value = TQString::fromLocal8Bit(attr->values[0].string.text);
#endif
		return true;
	}
	else return false;
}

bool IppRequest::stringListValue_p(const TQString& name, TQStringList& values, int type)
{
	if (!request_ || name.isEmpty()) return false;
	ipp_attribute_t	*attr = ippFindAttribute(request_, name.latin1(), (ipp_tag_t)type);
	values.clear();
	if (attr)
	{
#ifdef HAVE_CUPS_1_6
		for (int i=0;i<ippGetCount(attr);i++)
			values.append(TQString::fromLocal8Bit(ippGetString(attr, i, NULL)));
#else
		for (int i=0;i<attr->num_values;i++)
			values.append(TQString::fromLocal8Bit(attr->values[i].string.text));
#endif
		return true;
	}
	else return false;
}

bool IppRequest::boolean(const TQString& name, bool& value)
{
	if (!request_ || name.isEmpty()) return false;
	ipp_attribute_t	*attr = ippFindAttribute(request_, name.latin1(), IPP_TAG_BOOLEAN);
	if (attr)
	{
#ifdef HAVE_CUPS_1_6
		value = (bool)ippGetBoolean(attr, 0);
#else
		value = (bool)attr->values[0].boolean;
#endif
		return true;
	}
	else return false;
}

bool IppRequest::doFileRequest(const TQString& res, const TQString& filename)
{
	TQString	myHost = host_;
	int 	myPort = port_;
	if (myHost.isEmpty()) myHost = CupsInfos::self()->host();
	if (myPort <= 0) myPort = CupsInfos::self()->port();
	http_t	*HTTP = httpConnect(myHost.latin1(),myPort);

	connect_ = (HTTP != NULL);

	if (HTTP == NULL)
	{
		ippDelete(request_);
		request_ = 0;
		return false;
	}

#ifdef HAVE_CUPS_NO_PWD_CACHE
#if CUPS_VERSION_MAJOR < 1 || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 2)
   strncpy(  HTTP->authstring, cups_authstring.data(), HTTP_MAX_VALUE );
#else
   httpSetAuthString( HTTP, NULL, cups_authstring.data() );
#endif
#endif

	if (dump_ > 0)
	{
		dumpRequest(request_, false, "Request to "+myHost+":"+TQString::number(myPort));
	}

	request_ = cupsDoFileRequest(HTTP, request_, (res.isEmpty() ? "/" : res.latin1()), (filename.isEmpty() ? NULL : filename.latin1()));
#ifdef HAVE_CUPS_NO_PWD_CACHE
#if CUPS_VERSION_MAJOR < 1 || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 2)
   cups_authstring = HTTP->authstring;
#else
	cups_authstring = httpGetAuthString( HTTP );
#endif
#endif
	httpClose(HTTP);

	if (dump_ > 1)
	{
		dumpRequest(request_, true);
	}

	/* No printers found */
#ifdef HAVE_CUPS_1_6
	if ( request_ && ippGetStatusCode(request_) == 0x406 )
#else
	if ( request_ && request_->request.status.status_code == 0x406 )
#endif
		return true;

#ifdef HAVE_CUPS_1_6
	if (!request_ || ippGetState(request_) == IPP_ERROR || (ippGetStatusCode(request_) & 0x0F00))
#else
	if (!request_ || request_->state == IPP_ERROR || (request_->request.status.status_code & 0x0F00))
#endif
		return false;


	return true;
}

bool IppRequest::htmlReport(int group, TQTextStream& output)
{
	if (!request_) return false;
	// start table
	output << "<table border=\"1\" cellspacing=\"0\" cellpadding=\"0\">" << endl;
	output << "<tr><th bgcolor=\"dark blue\"><font color=\"white\">" << i18n("Attribute") << "</font></th>" << endl;
	output << "<th bgcolor=\"dark blue\"><font color=\"white\">" << i18n("Values") << "</font></th></tr>" << endl;
	// go to the first attribute of the specified group
#ifdef HAVE_CUPS_1_6
	ipp_attribute_t	*attr = ippFirstAttribute(request_);
	while (attr && ippGetGroupTag(attr) != group)
		attr = ippNextAttribute(request_);
#else
	ipp_attribute_t	*attr = request_->attrs;
	while (attr && attr->group_tag != group)
		attr = attr->next;
#endif
	// print each attribute
	const ipp_uchar_t	*d;
	TQCString		dateStr;
	TQDateTime		dt;
	bool			bg(false);
#ifdef HAVE_CUPS_1_6
	while (attr && ippGetGroupTag(attr) == group)
	{
		output << "  <tr bgcolor=\"" << (bg ? "#ffffd9" : "#ffffff") << "\">\n    <td><b>" << ippGetName(attr) << "</b></td>\n    <td>" << endl;
		bg = !bg;
		for (int i=0; i<ippGetCount(attr); i++)
		{
			switch (ippGetValueTag(attr))
			{
				case IPP_TAG_INTEGER:
					if (ippGetName(attr) && strstr(ippGetName(attr), "time"))
					{
						dt.setTime_t((unsigned int)(ippGetInteger(attr, i)));
						output << dt.toString();
					}
					else
						output << ippGetInteger(attr, i);
					break;
				case IPP_TAG_ENUM:
					output << "0x" << hex << ippGetInteger(attr, i) << dec;
					break;
				case IPP_TAG_BOOLEAN:
					output << (ippGetBoolean(attr, i) ? i18n("True") : i18n("False"));
					break;
				case IPP_TAG_STRING:
				case IPP_TAG_TEXTLANG:
				case IPP_TAG_NAMELANG:
				case IPP_TAG_TEXT:
				case IPP_TAG_NAME:
				case IPP_TAG_KEYWORD:
				case IPP_TAG_URI:
				case IPP_TAG_CHARSET:
				case IPP_TAG_LANGUAGE:
				case IPP_TAG_MIMETYPE:
					output << ippGetString(attr, i, NULL);
					break;
				case IPP_TAG_RESOLUTION:
					int xres;
					int yres;
					ipp_res_t units;
					xres = ippGetResolution(attr, i, &yres, &units);
					output << "( " << xres
					       << ", " << yres << " )";
					break;
				case IPP_TAG_RANGE:
					int lowervalue;
					int uppervalue;
					lowervalue = ippGetRange(attr, i, &uppervalue);
					output << "[ " << (lowervalue > 0 ? lowervalue : 1)
					       << ", " << (uppervalue > 0 ? uppervalue : 65535) << " ]";
					break;
				case IPP_TAG_DATE:
					d = ippGetDate(attr, i);
					dateStr.sprintf("%.4d-%.2d-%.2d, %.2d:%.2d:%.2d %c%.2d%.2d",
							d[0]*256+d[1], d[2], d[3],
							d[4], d[5], d[6],
							d[8], d[9], d[10]);
					output << dateStr;
					break;
				default:
					continue;
			}
			if (i < ippGetCount(attr)-1)
				output << "<br>";
		}
		output << "</td>\n  </tr>" << endl;
		attr = ippNextAttribute(request_);
#else
	while (attr && attr->group_tag == group)
	{
		output << "  <tr bgcolor=\"" << (bg ? "#ffffd9" : "#ffffff") << "\">\n    <td><b>" << attr->name << "</b></td>\n    <td>" << endl;
		bg = !bg;
		for (int i=0; i<attr->num_values; i++)
		{
			switch (attr->value_tag)
			{
				case IPP_TAG_INTEGER:
					if (attr->name && strstr(attr->name, "time"))
					{
						dt.setTime_t((unsigned int)(attr->values[i].integer));
						output << dt.toString();
					}
					else
						output << attr->values[i].integer;
					break;
				case IPP_TAG_ENUM:
					output << "0x" << hex << attr->values[i].integer << dec;
					break;
				case IPP_TAG_BOOLEAN:
					output << (attr->values[i].boolean ? i18n("True") : i18n("False"));
					break;
				case IPP_TAG_STRING:
				case IPP_TAG_TEXTLANG:
				case IPP_TAG_NAMELANG:
				case IPP_TAG_TEXT:
				case IPP_TAG_NAME:
				case IPP_TAG_KEYWORD:
				case IPP_TAG_URI:
				case IPP_TAG_CHARSET:
				case IPP_TAG_LANGUAGE:
				case IPP_TAG_MIMETYPE:
					output << attr->values[i].string.text;
					break;
				case IPP_TAG_RESOLUTION:
					output << "( " << attr->values[i].resolution.xres
					       << ", " << attr->values[i].resolution.yres << " )";
					break;
				case IPP_TAG_RANGE:
					output << "[ " << (attr->values[i].range.lower > 0 ? attr->values[i].range.lower : 1)
					       << ", " << (attr->values[i].range.upper > 0 ? attr->values[i].range.upper : 65535) << " ]";
					break;
				case IPP_TAG_DATE:
					d = attr->values[i].date;
					dateStr.sprintf("%.4d-%.2d-%.2d, %.2d:%.2d:%.2d %c%.2d%.2d",
							d[0]*256+d[1], d[2], d[3],
							d[4], d[5], d[6],
							d[8], d[9], d[10]);
					output << dateStr;
					break;
				default:
					continue;
			}
			if (i < attr->num_values-1)
				output << "<br>";
		}
		output << "</td>\n  </tr>" << endl;
		attr = attr->next;
#endif
	}
	// end table
	output << "</table>" << endl;

	return true;
}

TQMap<TQString,TQString> IppRequest::toMap(int group)
{
	TQMap<TQString,TQString>	opts;
	if (request_)
	{
		ipp_attribute_t	*attr = first();
		while (attr)
		{
#ifdef HAVE_CUPS_1_6
			if (group != -1 && ippGetGroupTag(attr) != group)
			{
				attr = ippNextAttribute(request_);
				continue;
			}
			TQString	value;
			for (int i=0; i<ippGetCount(attr); i++)
			{
				switch (ippGetValueTag(attr))
				{
					case IPP_TAG_INTEGER:
					case IPP_TAG_ENUM:
						value.append(TQString::number(ippGetInteger(attr, i))).append(",");
						break;
					case IPP_TAG_BOOLEAN:
						value.append((ippGetBoolean(attr, i) ? "true" : "false")).append(",");
						break;
					case IPP_TAG_RANGE:
						int lowervalue;
						int uppervalue;
						lowervalue = ippGetRange(attr, i, &uppervalue);
						if (lowervalue > 0)
							value.append(TQString::number(lowervalue));
						if (lowervalue != uppervalue)
						{
							value.append("-");
							if (uppervalue > 0)
								value.append(TQString::number(uppervalue));
						}
						value.append(",");
						break;
					case IPP_TAG_STRING:
					case IPP_TAG_TEXT:
					case IPP_TAG_NAME:
					case IPP_TAG_KEYWORD:
					case IPP_TAG_URI:
					case IPP_TAG_MIMETYPE:
					case IPP_TAG_NAMELANG:
					case IPP_TAG_TEXTLANG:
					case IPP_TAG_CHARSET:
					case IPP_TAG_LANGUAGE:
						value.append(TQString::fromLocal8Bit(ippGetString(attr, i, NULL))).append(",");
						break;
					default:
						break;
				}
			}
			if (!value.isEmpty())
				value.truncate(value.length()-1);
			opts[TQString::fromLocal8Bit(ippGetName(attr))] = value;
			attr = ippNextAttribute(request_);
#else
			if (group != -1 && attr->group_tag != group)
			{
				attr = attr->next;
				continue;
			}
			TQString	value;
			for (int i=0; i<attr->num_values; i++)
			{
				switch (attr->value_tag)
				{
					case IPP_TAG_INTEGER:
					case IPP_TAG_ENUM:
						value.append(TQString::number(attr->values[i].integer)).append(",");
						break;
					case IPP_TAG_BOOLEAN:
						value.append((attr->values[i].boolean ? "true" : "false")).append(",");
						break;
					case IPP_TAG_RANGE:
						if (attr->values[i].range.lower > 0)
							value.append(TQString::number(attr->values[i].range.lower));
						if (attr->values[i].range.lower != attr->values[i].range.upper)
						{
							value.append("-");
							if (attr->values[i].range.upper > 0)
								value.append(TQString::number(attr->values[i].range.upper));
						}
						value.append(",");
						break;
					case IPP_TAG_STRING:
					case IPP_TAG_TEXT:
					case IPP_TAG_NAME:
					case IPP_TAG_KEYWORD:
					case IPP_TAG_URI:
					case IPP_TAG_MIMETYPE:
					case IPP_TAG_NAMELANG:
					case IPP_TAG_TEXTLANG:
					case IPP_TAG_CHARSET:
					case IPP_TAG_LANGUAGE:
						value.append(TQString::fromLocal8Bit(attr->values[i].string.text)).append(",");
						break;
					default:
						break;
				}
			}
			if (!value.isEmpty())
				value.truncate(value.length()-1);
			opts[TQString::fromLocal8Bit(attr->name)] = value;
			attr = attr->next;
#endif
		}
	}
	return opts;
}

void IppRequest::setMap(const TQMap<TQString,TQString>& opts)
{
	if (!request_)
		return;

	TQRegExp	re("^\"|\"$");
	cups_option_t	*options = NULL;
	int	n = 0;
	for (TQMap<TQString,TQString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
	{
		if (it.key().startsWith("kde-") || it.key().startsWith("app-"))
			continue;
		TQString	value = it.data().stripWhiteSpace(), lovalue;
		value.replace(re, "");
		lovalue = value.lower();

		// handles specific cases: boolean, empty strings, or option that has that boolean
		// keyword as value (to prevent them from conversion to real boolean)
		if (value == "true" || value == "false")
			addBoolean(IPP_TAG_JOB, it.key(), (value == "true"));
		else if (value.isEmpty() || lovalue == "off" || lovalue == "on"
		         || lovalue == "yes" || lovalue == "no"
			 || lovalue == "true" || lovalue == "false")
			addName(IPP_TAG_JOB, it.key(), value);
		else
			n = cupsAddOption(it.key().local8Bit(), value.local8Bit(), n, &options);
	}
	if (n > 0)
		cupsEncodeOptions(request_, n, options);
	cupsFreeOptions(n, options);

	// find an remove that annoying "document-format" attribute
#if CUPS_VERSION_MAJOR > 1 || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 2)
    ipp_attribute_t *attr = ippFindAttribute(request_, "document-format", IPP_TAG_NAME);
    ippDeleteAttribute(request_, attr);
#else
	// (can't use IppDeleteAttribute as older cups doesn't have that)
	ipp_attribute_t	*attr = request_->attrs;
	while (attr)
	{
		if (attr->next && strcmp(attr->next->name, "document-format") == 0)
		{
			ipp_attribute_t	*attr2 = attr->next;
			attr->next = attr2->next;
			_ipp_free_attr(attr2);
			break;
		}
		attr = attr->next;
	}
#endif
}

#ifdef HAVE_CUPS_1_6
ipp_attribute_t* IppRequest::first()
{ return (request_ ? ippFirstAttribute(request_) : NULL); }
#else
ipp_attribute_t* IppRequest::first()
{ return (request_ ? request_->attrs : NULL); }
#endif

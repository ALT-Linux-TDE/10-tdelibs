/*
   Copyright (c) 2001 Nikolas Zimmermann <wildfox@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef MCOPDCOPOBJECT_H
#define MCOPDCOPOBJECT_H

#include <dcopobject.h>

#include <tqvaluelist.h>
#include <tqcstring.h>

typedef TQValueList<TQCString> QCStringList;

class MCOPDCOPObjectPrivate;

class MCOPEntryInfo
{
public:
	MCOPEntryInfo()
	{ }
	
	~MCOPEntryInfo()
	{ }

	TQCString functionType()
	{ return m_functionType; }
	
	TQCString functionName()
	{ return m_functionName; }

	TQCString signature()
	{ return m_signature; }
	
	QCStringList signatureList()
	{ return m_signatureList; }
	
	void setFunctionType(TQCString s)
	{ m_functionType = s; }

	void setFunctionName(TQCString s)
	{ m_functionName = s; }

	void setSignature(TQCString s)
	{ m_signature = s; }

	void setSignatureList(QCStringList s)
	{ m_signatureList = s; }

private:
	TQCString m_functionType, m_functionName, m_signature;
	QCStringList m_signatureList;
};

class MCOPDCOPObject : public DCOPObject
{
public:
	MCOPDCOPObject(TQCString name);
	~MCOPDCOPObject();

	virtual bool processDynamic(const TQCString &fun, const TQByteArray &data, TQCString &replyType, TQByteArray &replyData);
	virtual QCStringList functionsDynamic();

	void addDynamicFunction(TQCString value, MCOPEntryInfo *entry);

	Arts::Buffer *callFunction(MCOPEntryInfo *entry, TQCString ifaceName, const TQByteArray &data);
	
private:
    MCOPDCOPObjectPrivate *d;
};


#endif


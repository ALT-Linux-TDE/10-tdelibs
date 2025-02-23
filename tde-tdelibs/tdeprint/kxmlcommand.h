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

#ifndef KXMLCOMMAND_H
#define KXMLCOMMAND_H

#include <tqdom.h>
#include <tqmap.h>
#include <tqobject.h>

#include <tdelibs_export.h>

class DrMain;
class DrGroup;
class DrBase;

class TDEPRINT_EXPORT KXmlCommand : public TQObject
{
	friend class KXmlCommandManager;

public:
	~KXmlCommand();

	TQString name() const;
	void setName(const TQString&);
	TQString command();
	void setCommand(const TQString&);
	DrMain* driver();
	DrMain* takeDriver();
	void setDriver(DrMain*);
	TQString io(bool io_input = true, bool io_pipe = false);
	void setIo(const TQString&, bool io_input = true, bool io_pipe = false);
	TQString description();
	void setDescription(const TQString&);
	TQString mimeType();
	void setMimeType(const TQString&);
	bool acceptMimeType(const TQString&);
	TQStringList inputMimeTypes();
	void setInputMimeTypes(const TQStringList&);
	TQStringList requirements();
	void setRequirements(const TQStringList&);
	TQString comment();
	void setComment( const TQString& );
	bool isValid();

	TQString buildCommand(const TQMap<TQString,TQString>& opts, bool pipein = true, bool pipeout = true);
	void setOptions(const TQMap<TQString,TQString>& opts);
	void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);

protected:
	void init();
	void loadXml();
	void saveXml();
	void loadDesktop();
	void saveDesktop();
	void check(bool use_xml = false);
	DrGroup* parseGroup(const TQDomElement& e, DrGroup *grp = 0);
	DrBase* parseArgument(const TQDomElement& e);
	void parseIO(const TQDomElement& e, int n);
	TQDomElement createIO(TQDomDocument&, int, const TQString&);
	TQDomElement createGroup(TQDomDocument&, DrGroup*);
	TQDomElement createElement(TQDomDocument&, DrBase*);

	// use protected constructor to only allow the manager to
	// create KXmlCommand object.
	KXmlCommand(const TQString& xmlId = TQString::null);

private:
	class KXmlCommandPrivate;
	KXmlCommandPrivate	*d;
};

class TDEPRINT_EXPORT KXmlCommandManager : public TQObject
{
public:
	enum IO_CheckType	{ None = 0, Basic, Advanced };

	KXmlCommandManager();
	~KXmlCommandManager();

	KXmlCommand* loadCommand(const TQString& xmlId, bool check = false);
	void saveCommand(KXmlCommand *xmlCmd);
	TQStringList commandList();
	TQStringList commandListWithDescription();
	TQString selectCommand(TQWidget *parent = 0);

	TQStringList autoConvert(const TQString& mimesrc, const TQString& mimedest);
	int insertCommand(TQStringList& list, const TQString& filtername, bool defaultToStart = true);
	bool checkCommand(const TQString&, int inputCheck = Advanced, int outputCheck = Advanced, TQString *msg = 0);
	bool configure(KXmlCommand*, TQWidget *parent = 0);
	void cleanUp();

	static KXmlCommandManager* self();

protected:
	void preload();
	KXmlCommand* command(const TQString&) const;

private:
	class KXmlCommandManagerPrivate;
	KXmlCommandManagerPrivate	*d;
	static KXmlCommandManager	*m_self;
};

#endif

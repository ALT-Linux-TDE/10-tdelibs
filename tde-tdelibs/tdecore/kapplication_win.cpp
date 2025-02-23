/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

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

#include <tdeapplication.h>
#include <kstandarddirs.h>
#include <tdelocale.h>
#include <kurl.h>

#include "kcheckaccelerators.h"
#include "kappdcopiface.h"

#include <qassistantclient.h>
#include <tqdir.h>

#include "windows.h"
#include "shellapi.h"

/**
 * MS Windows-related actions for TDEApplication startup.
 * 
 * - Use Qt translation which will be usable for TQFileDialog 
 *    and other Qt-only GUIs. The "qt_<language>.qm" file should be stored
 *    in the same place as .po files for a given language.
 *
 * @internal
*/
void TDEApplication_init_windows(bool /*GUIenabled*/)
{
	TQString qt_transl_file = ::locate( "locale", TDEGlobal::locale()->language() 
		+ "/LC_MESSAGES/qt_" + TDEGlobal::locale()->language() + ".qm" );
	QTranslator *qt_transl = new QTranslator();
	if (qt_transl->load( qt_transl_file, ""))
		kapp->installTranslator( qt_transl );
	else
		delete qt_transl;
}

//unsafe; create kapplication_p.h instead!
typedef void* IceIOErrorHandler;

class TDEApplicationPrivate
{
public:
  TDEApplicationPrivate();
  ~TDEApplicationPrivate();

  bool actionRestrictions : 1;
  bool guiEnabled : 1;
  int refCount;
  IceIOErrorHandler oldIceIOErrorHandler;
  KCheckAccelerators* checkAccelerators;
  TQString overrideStyle;
  TQString geometry_arg;
  TQCString startup_id;
  TQTimer* app_started_timer;
  KAppDCOPInterface *m_KAppDCOPInterface;
  bool session_save;
  QAssistantClient* qassistantclient;
};

void TDEApplication::invokeHelp( const TQString& anchor,
	const TQString& _appname, const TQCString& startup_id ) const
{
	if (!d->qassistantclient) {
		d->qassistantclient = new QAssistantClient(
			TDEStandardDirs::findExe( "assistant" ), 0);
		TQStringList args;
		args << "-profile";
		args << TQDir::convertSeparators( locate("html", TQString(name())+"/"+TQString(name())+".adp") );
		d->qassistantclient->setArguments(args);
	}
	d->qassistantclient->openAssistant();
}

// on win32, for invoking browser we're using win32 API
// see kapplication_win.cpp
void TDEApplication::invokeBrowser( const TQString &url, const TQCString& startup_id )
{
	TQCString s = url.latin1();
	const unsigned short *l = (const unsigned short *)s.data();
	ShellExecuteA(0, "open", s.data(), 0, 0, SW_NORMAL);
}

void TDEApplication::invokeMailer(const TQString &to, const TQString &cc, const TQString &bcc,
                                const TQString &subject, const TQString &body,
                                const TQString & /*messageFile TODO*/, const TQStringList &attachURLs,
                                const TQCString& startup_id )
{
	KURL url("mailto:"+to);
	url.setQuery("?subject="+subject);
	url.addQueryItem("cc", cc);
	url.addQueryItem("bcc", bcc);
	url.addQueryItem("body", body);
	for (TQStringList::ConstIterator it = attachURLs.constBegin(); it != attachURLs.constEnd(); ++it)
		url.addQueryItem("attach", KURL::encode_string(*it));

	TQCString s = url.url().latin1();
	const unsigned short *l = (const unsigned short *)s.data();
	ShellExecuteA(0, "open", s.data(), 0, 0, SW_NORMAL);
}


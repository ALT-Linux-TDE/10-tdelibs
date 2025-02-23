/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>
                 2000 Malte Starostik <starosti@zedat.fu-berlin.de>
		 2000,2003 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "knotifyclient.h"

#include <tqdatastream.h>
#include <tqptrstack.h>

#include <tdeapplication.h>
#include <kstandarddirs.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <dcopclient.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#ifdef TQ_WS_X11
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <fixx11h.h>
#endif

static const char daemonName[] = "knotify";

static bool canAvoidStartupEvent( const TQString& event, const TQString& appname, int present )
{
    static bool checkAvoid = true;
    if( !checkAvoid )
        return false;
    if(( appname != "twin" && appname != "ksmserver" ) || present > 0 ) {
        checkAvoid = false;
        return false;
    }
    // starttde event is in global events file
    static TDEConfig* configfile = appname != "ksmserver"
        ? new TDEConfig( appname + ".eventsrc", true, false )
        : new TDEConfig( "knotify.eventsrc", true, false );
    static TDEConfig* eventsfile = appname != "ksmserver"
        ? new TDEConfig( appname + "/eventsrc", true, false, "data" )
        : new TDEConfig( "knotify/eventsrc", true, false, "data" );
    configfile->setGroup( event );
    eventsfile->setGroup( event );
    int ev1 = configfile->readNumEntry( "presentation", -2 );
    int ev2 = eventsfile->readNumEntry( "default_presentation", -2 );
    if(( ev1 == -2 && ev2 == -2 ) // unknown
        || ev1 > 0 // configured to have presentation
        || ( ev1 == -2 && ev2 > 0 )) { // not configured, has default presentation
        checkAvoid = false;
        return false;
    }
    return true;
}

static int sendNotifyEvent(const TQString &message, const TQString &text,
                            int present, int level, const TQString &sound,
                            const TQString &file, int winId )
{
  if (!kapp) return 0;

  // ensure tray icon is shown and positioned before sending event to notification daemon
#ifdef TQ_WS_X11
  XFlush(tqt_xdisplay());
#endif

  DCOPClient *client=kapp->dcopClient();
  if (!client->isAttached())
  {
    client->attach();
    if (!client->isAttached())
      return 0;
  }

  TQString appname = KNotifyClient::instance()->instanceName();

  if( canAvoidStartupEvent( message, appname, present ))
      return -1; // done "successfully" - there will be no event presentation

  int uniqueId = kMax( 1, kapp->random() ); // must not be 0 -- means failure!

  // knotify daemon needs toplevel window
  TQWidget* widget = TQWidget::find( (WId)winId );
  if( widget )
    winId = (int)widget->topLevelWidget()->winId();

  TQByteArray data;
  TQDataStream ds(data, IO_WriteOnly);
  ds << message << appname << text << sound << file << present << level
     << winId << uniqueId;

  if ( !KNotifyClient::startDaemon() )
      return 0;

  if ( client->send(daemonName, "Notify", "notify(TQString,TQString,TQString,TQString,TQString,int,int,int,int)", data) )
  {
      return uniqueId;
  }

  return 0;
}

int KNotifyClient::event( StandardEvent type, const TQString& text )
{
    return event( 0, type, text );
}

int KNotifyClient::event(const TQString &message, const TQString &text)
{
    return event(0, message, text);
}

int KNotifyClient::userEvent(const TQString &text, int present, int level,
                              const TQString &sound, const TQString &file)
{
    return userEvent( 0, text, present, level, sound, file );
}


int KNotifyClient::event( int winId, StandardEvent type, const TQString& text )
{
    TQString message;
    switch ( type ) {
    case cannotOpenFile:
	message = TQString::fromLatin1("cannotopenfile");
	break;
    case warning:
	message = TQString::fromLatin1("warning");
	break;
    case fatalError:
	message = TQString::fromLatin1("fatalerror");
	break;
    case catastrophe:
	message = TQString::fromLatin1("catastrophe");
	break;
    case notification: // fall through
    default:
	message = TQString::fromLatin1("notification");
	break;
    }

    return sendNotifyEvent( message, text, Default, Default,
			    TQString::null, TQString::null, winId );
}

int KNotifyClient::event(int winId, const TQString &message,
                          const TQString &text)
{
  return sendNotifyEvent(message, text, Default, Default, TQString::null, TQString::null, winId);
}

int KNotifyClient::userEvent(int winId, const TQString &text, int present,
                              int level,
                              const TQString &sound, const TQString &file)
{
  return sendNotifyEvent(TQString::null, text, present, level, sound, file, winId);
}

int KNotifyClient::getPresentation(const TQString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;

	TDEConfig eventsfile( KNotifyClient::instance()->instanceName()+".eventsrc", true, false);
	eventsfile.setGroup(eventname);

	present=eventsfile.readNumEntry("presentation", -1);

	return present;
}

TQString KNotifyClient::getFile(const TQString &eventname, int present)
{
	if (eventname.isEmpty()) return TQString::null;

	TDEConfig eventsfile( KNotifyClient::instance()->instanceName()+".eventsrc", true, false);
	eventsfile.setGroup(eventname);

	switch (present)
	{
	case (Sound):
		return eventsfile.readPathEntry("soundfile");
	case (Logfile):
		return eventsfile.readPathEntry("logfile");
	}

	return TQString::null;
}

int KNotifyClient::getDefaultPresentation(const TQString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;

	TDEConfig eventsfile( KNotifyClient::instance()->instanceName()+"/eventsrc", true, false, "data");
	eventsfile.setGroup(eventname);

	present=eventsfile.readNumEntry("default_presentation", -1);

	return present;
}

TQString KNotifyClient::getDefaultFile(const TQString &eventname, int present)
{
	if (eventname.isEmpty()) return TQString::null;

	TDEConfig eventsfile( KNotifyClient::instance()->instanceName()+"/eventsrc", true, false, "data");
	eventsfile.setGroup(eventname);

	switch (present)
	{
	case (Sound):
		return eventsfile.readPathEntry("default_sound");
	case (Logfile):
		return eventsfile.readPathEntry("default_logfile");
	}

	return TQString::null;
}

bool KNotifyClient::startDaemon()
{
  static bool firstTry = true;
  if (!kapp->dcopClient()->isApplicationRegistered(daemonName)) {
    if( firstTry ) {
      firstTry = false;
      return TDEApplication::startServiceByDesktopName(daemonName) == 0;
    }
    return false;
  }
  return true;
}


void KNotifyClient::beep(const TQString& reason)
{
  if ( !kapp || KNotifyClient::Instance::currentInstance()->useSystemBell() ) {
    TQApplication::beep();
    return;
  }

  DCOPClient *client=kapp->dcopClient();
  if (!client->isAttached())
  {
    client->attach();
    if (!client->isAttached() || !client->isApplicationRegistered(daemonName))
    {
      TQApplication::beep();
      return;
    }
  }
  // The kaccess daemon handles visual and other audible beeps
  if ( client->isApplicationRegistered( "kaccess" ) )
  {
      TQApplication::beep();
      return;
  }

  KNotifyClient::event(KNotifyClient::notification, reason);
}


TDEInstance * KNotifyClient::instance() {
    return KNotifyClient::Instance::current();
}


class KNotifyClient::InstanceStack
{
public:
	InstanceStack() { m_defaultInstance = 0; }
	virtual ~InstanceStack() { delete m_defaultInstance; }
	void push(Instance *instance) { m_instances.push(instance); }

	void pop(Instance *instance)
	{
		if (m_instances.top() == instance)
			m_instances.pop();
		else if (!m_instances.isEmpty())
		{
			kdWarning(160) << "Tried to remove an Instance that is not the current," << endl;
			kdWarning(160) << "Resetting to the main TDEApplication." << endl;
			m_instances.clear();
		}
		else
			kdWarning(160) << "Tried to remove an Instance, but the stack was empty." << endl;
    }

	Instance *currentInstance()
	{
		if (m_instances.isEmpty())
		{
			m_defaultInstance = new Instance(kapp);
		}
		return m_instances.top();
	}

private:
	TQPtrStack<Instance> m_instances;
	Instance *m_defaultInstance;
};

KNotifyClient::InstanceStack * KNotifyClient::Instance::s_instances = 0L;
static KStaticDeleter<KNotifyClient::InstanceStack > instancesDeleter;

struct KNotifyClient::InstancePrivate
{
    TDEInstance *instance;
    bool useSystemBell;
};

KNotifyClient::Instance::Instance(TDEInstance *instance)
{
    d = new InstancePrivate;
    d->instance = instance;
    instances()->push(this);

    TDEConfig *config = instance->config();
    TDEConfigGroupSaver cs( config, "General" );
    d->useSystemBell = config->readBoolEntry( "UseSystemBell", false );
}

KNotifyClient::Instance::~Instance()
{
	if (s_instances)
		s_instances->pop(this);
	delete d;
}

KNotifyClient::InstanceStack *KNotifyClient::Instance::instances()
{
	if (!s_instances)
		instancesDeleter.setObject(s_instances, new InstanceStack);
	return s_instances;
}

bool KNotifyClient::Instance::useSystemBell() const
{
    return d->useSystemBell;
}


// static methods

// We always return a valid KNotifyClient::Instance here. If no special one
// is available, we have a default-instance with kapp as TDEInstance.
// We make sure to always have that default-instance in the stack, because
// the stack might have gotten cleared in the destructor.
// We can't use QPtrStack::setAutoDelete( true ), because no instance besides
// our default instance is owned by us.
KNotifyClient::Instance * KNotifyClient::Instance::currentInstance()
{
	return instances()->currentInstance();
}

TDEInstance *KNotifyClient::Instance::current()
{
    return currentInstance()->d->instance;
}

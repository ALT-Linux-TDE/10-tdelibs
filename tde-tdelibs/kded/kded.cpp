/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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

#include <tqdir.h>

#include "kded.h"
#include "kdedmodule.h"

#include <kresourcelist.h>
#include <kcrash.h>

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <tqfile.h>
#include <tqtimer.h>

#include <dcopclient.h>

#include <kuniqueapplication.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>
#include <kdatastream.h>
#include <tdeio/global.h>
#include <kservicetype.h>

#ifdef TQ_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

Kded *Kded::_self = 0;

static bool checkStamps = true;
static bool delayedCheck = false;

static void runBuildSycoca(TQObject *callBackObj=0, const char *callBackSlot=0)
{
   TQStringList args;
   args.append("--incremental");
   if(checkStamps)
      args.append("--checkstamps");
   if(delayedCheck)
      args.append("--nocheckfiles");
   else
      checkStamps = false; // useful only during kded startup
   if (callBackObj)
   {
      TQByteArray data;
      TQDataStream dataStream( data, IO_WriteOnly );
      dataStream << TQString("tdebuildsycoca") << args;
      TQCString _launcher = TDEApplication::launcher();

      kapp->dcopClient()->callAsync(_launcher, _launcher, "tdeinit_exec_wait(TQString,TQStringList)", data, callBackObj, callBackSlot);
   }
   else
   {
      TDEApplication::tdeinitExecWait( "tdebuildsycoca", args );
   }
}

static void runKonfUpdate()
{
   TDEApplication::tdeinitExecWait( "tdeconf_update", TQStringList(), 0, 0, "0" /*no startup notification*/ );
}

static void runDontChangeHostname(const TQCString &oldName, const TQCString &newName)
{
   TQStringList args;
   args.append(TQFile::decodeName(oldName));
   args.append(TQFile::decodeName(newName));
   TDEApplication::tdeinitExecWait( "kdontchangethehostname", args );
}

Kded::Kded(bool checkUpdates, bool new_startup)
  : DCOPObject("tdebuildsycoca"), DCOPObjectProxy(),
    b_checkUpdates(checkUpdates),
    m_needDelayedCheck(false),
    m_newStartup( new_startup )
{
  _self = this;
  TQCString cPath;
  TQCString tdesycoca_env = getenv("TDESYCOCA");
  if (tdesycoca_env.isEmpty())
     cPath = TQFile::encodeName(TDEGlobal::dirs()->saveLocation("tmp")+"tdesycoca");
  else
     cPath = tdesycoca_env;
  m_pTimer = new TQTimer(this);
  connect(m_pTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(recreate()));

  TQTimer::singleShot(100, this, TQ_SLOT(installCrashHandler()));

  m_pDirWatch = 0;

  m_windowIdList.setAutoDelete(true);

  m_recreateCount = 0;
  m_recreateBusy = false;
}

Kded::~Kded()
{
  _self = 0;
  m_pTimer->stop();
  delete m_pTimer;
  delete m_pDirWatch;
  // We have to delete the modules while we're still able to process incoming
  // DCOP messages, since modules might make DCOP calls in their destructors.
  TQAsciiDictIterator<KDEDModule> it(m_modules);
  while (!it.isEmpty()) 
	delete it.toFirst();
}

bool Kded::process(const TQCString &obj, const TQCString &fun,
                   const TQByteArray &data,
                   TQCString &replyType, TQByteArray &replyData)
{
  if (obj == "tdesycoca") return false; // Ignore this one.

  if (m_dontLoad[obj])
     return false;

  KDEDModule *module = loadModule(obj, true);
  if (!module)
     return false;

  module->setCallingDcopClient(kapp->dcopClient());
  return module->process(fun, data, replyType, replyData);
}

void Kded::initModules()
{
     m_dontLoad.clear();
     TDEConfig *config = kapp->config();
     bool tde_running = !( getenv( "TDE_FULL_SESSION" ) == NULL || getenv( "TDE_FULL_SESSION" )[ 0 ] == '\0' );
    // not the same user like the one running the session (most likely we're run via sudo or something)
    if( getenv( "TDE_SESSION_UID" ) != NULL && uid_t( atoi( getenv( "TDE_SESSION_UID" ))) != getuid())
        tde_running = false;
     // Preload kded modules.
     KService::List kdedModules = KServiceType::offers("KDEDModule");
     TQString version = getenv( "KDE_SESSION_VERSION" );
     TQStringList blacklist;
     if ( !(version == NULL) && version >= "4" )
     {
         kdDebug(7020) << "KDE4 is running:" << endl;
         kdDebug(7020) << "  KDE_SESSION_VERSION: " << version << endl;
         kdDebug(7020) << "  Blacklisting mediamanager, medianotifier, kmilod, kwrited." << endl;
         blacklist << "mediamanager" << "medianotifier" << "kmilod" << "kwrited";
     }
     for(KService::List::ConstIterator it = kdedModules.begin(); it != kdedModules.end(); ++it)
     {
         KService::Ptr service = *it;
         bool autoload = service->property("X-TDE-Kded-autoload", TQVariant::Bool).toBool();
         config->setGroup(TQString("Module-%1").arg(service->desktopEntryName()));
         autoload = config->readBoolEntry("autoload", autoload);
         for (TQStringList::Iterator module = blacklist.begin(); module != blacklist.end(); ++module)
         {
            if (service->desktopEntryName() == *module)
            {
               autoload = false;
               break;
            }
         }
         if( m_newStartup )
         {
            // see ksmserver's README for description of the phases
            TQVariant phasev = service->property("X-TDE-Kded-phase", TQVariant::Int );
            int phase = phasev.isValid() ? phasev.toInt() : 2;
            bool prevent_autoload = false;
            switch( phase )
            {
                case 0: // always autoload
                    break;
                case 1: // autoload only in TDE
                    if( !tde_running )
                        prevent_autoload = true;
                    break;
                case 2: // autoload delayed, only in TDE
                default:
                    prevent_autoload = true;
                    break;   
            }
            if (autoload && !prevent_autoload)
               loadModule(service, false);
         }
         else
         {
            if (autoload && tde_running)
               loadModule(service, false);
         }
         bool dontLoad = false;
         TQVariant p = service->property("X-TDE-Kded-load-on-demand", TQVariant::Bool);
         if (p.isValid() && (p.toBool() == false))
            dontLoad = true;
         if (dontLoad)
            noDemandLoad(service->desktopEntryName());

         if (dontLoad && !autoload)
            unloadModule(service->desktopEntryName().latin1());
     }
}

void Kded::loadSecondPhase()
{
     kdDebug(7020) << "Loading second phase autoload" << endl;
     TDEConfig *config = kapp->config();
     KService::List kdedModules = KServiceType::offers("KDEDModule");
     for(KService::List::ConstIterator it = kdedModules.begin(); it != kdedModules.end(); ++it)
     {
         KService::Ptr service = *it;
         bool autoload = service->property("X-TDE-Kded-autoload", TQVariant::Bool).toBool();
         config->setGroup(TQString("Module-%1").arg(service->desktopEntryName()));
         autoload = config->readBoolEntry("autoload", autoload);
         TQVariant phasev = service->property("X-TDE-Kded-phase", TQVariant::Int );
         int phase = phasev.isValid() ? phasev.toInt() : 2;
         if( phase == 2 && autoload )
            loadModule(service, false);
     }
}

void Kded::noDemandLoad(const TQString &obj)
{
  m_dontLoad.insert(obj.latin1(), this);
}

KDEDModule *Kded::loadModule(const TQCString &obj, bool onDemand)
{
  KDEDModule *module = m_modules.find(obj);
  if (module)
     return module;
  KService::Ptr s = KService::serviceByDesktopPath("kded/"+obj+".desktop");
  return loadModule(s, onDemand);
}

KDEDModule *Kded::loadModule(const KService *s, bool onDemand)
{
  KDEDModule *module = 0;
  if (s && !s->library().isEmpty())
  {
    TQCString obj = s->desktopEntryName().latin1();
    KDEDModule *oldModule = m_modules.find(obj);
    if (oldModule)
       return oldModule;

    if (onDemand)
    {
      TQVariant p = s->property("X-TDE-Kded-load-on-demand", TQVariant::Bool);
      if (p.isValid() && (p.toBool() == false))
      {
         noDemandLoad(s->desktopEntryName());
         return 0;
      }
    }
    // get the library loader instance

    KLibLoader *loader = KLibLoader::self();

    TQVariant v = s->property("X-TDE-FactoryName", TQVariant::String);
    TQString factory = v.isValid() ? v.toString() : TQString::null;
    if (factory.isEmpty())
    {
       // Stay bugward compatible
       v = s->property("X-TDE-Factory", TQVariant::String);
       factory = v.isValid() ? v.toString() : TQString::null;
    }
    if (factory.isEmpty())
      factory = s->library();

    factory = "create_" + factory;
    TQString libname = "kded_"+s->library();

    KLibrary *lib = loader->library(TQFile::encodeName(libname));
    if (!lib)
    {
      kdWarning() << k_funcinfo << "Could not load library. [ "
		  << loader->lastErrorMessage() << " ]" << endl;
      libname.prepend("lib");
      lib = loader->library(TQFile::encodeName(libname));
    }
    if (lib)
    {
      // get the create_ function
      void *create = lib->symbol(TQFile::encodeName(factory));

      if (create)
      {
        // create the module
        KDEDModule* (*func)(const TQCString &);
        func = (KDEDModule* (*)(const TQCString &)) create;
        module = func(obj);
        if (module)
        {
          m_modules.insert(obj, module);
          m_libs.insert(obj, lib);
          connect(module, TQ_SIGNAL(moduleDeleted(KDEDModule *)), TQ_SLOT(slotKDEDModuleRemoved(KDEDModule *)));
          kdDebug(7020) << "Successfully loaded module '" << obj << "'\n";
          return module;
        }
      }
      loader->unloadLibrary(TQFile::encodeName(libname));
    }
    else
    {
	kdWarning() << k_funcinfo << "Could not load library. [ "
		    << loader->lastErrorMessage() << " ]" << endl;
    }
    kdDebug(7020) << "Could not load module '" << obj << "'\n";
  }
  return 0;
}

bool Kded::unloadModule(const TQCString &obj)
{
  KDEDModule *module = m_modules.take(obj);
  if (!module)
     return false;
  kdDebug(7020) << "Unloading module '" << obj << "'\n";
  delete module;
  return true;
}

// DCOP
QCStringList Kded::loadedModules()
{
	QCStringList modules;
	TQAsciiDictIterator<KDEDModule> it( m_modules );
	for ( ; it.current(); ++it)
		modules.append( it.currentKey() );

	return modules;
}

QCStringList Kded::functions()
{
    QCStringList res = DCOPObject::functions();
    res += "ASYNC recreate()";
    return res;
}

void Kded::slotKDEDModuleRemoved(KDEDModule *module)
{
  m_modules.remove(module->objId());
  KLibrary *lib = m_libs.take(module->objId());
  if (lib)
     lib->unload();
}

void Kded::slotApplicationRemoved(const TQCString &appId)
{
  for(TQAsciiDictIterator<KDEDModule> it(m_modules); it.current(); ++it)
  {
     it.current()->removeAll(appId);
  }

  TQValueList<long> *windowIds = m_windowIdList.find(appId);
  if (windowIds)
  {
     for( TQValueList<long>::ConstIterator it = windowIds->begin();
          it != windowIds->end(); ++it)
     {
        long windowId = *it;
        m_globalWindowIdList.remove(windowId);
        for(TQAsciiDictIterator<KDEDModule> it(m_modules); it.current(); ++it)
        {
            emit it.current()->windowUnregistered(windowId);
        }
     }
     m_windowIdList.remove(appId);
  }
}

void Kded::updateDirWatch()
{
  if (!b_checkUpdates) return;

  delete m_pDirWatch;
  m_pDirWatch = new KDirWatch;

  TQObject::connect( m_pDirWatch, TQ_SIGNAL(dirty(const TQString&)),
           this, TQ_SLOT(update(const TQString&)));
  TQObject::connect( m_pDirWatch, TQ_SIGNAL(created(const TQString&)),
           this, TQ_SLOT(update(const TQString&)));
  TQObject::connect( m_pDirWatch, TQ_SIGNAL(deleted(const TQString&)),
           this, TQ_SLOT(dirDeleted(const TQString&)));

  // For each resource
  for( TQStringList::ConstIterator it = m_allResourceDirs.begin();
       it != m_allResourceDirs.end();
       ++it )
  {
     readDirectory( *it );
  }
}

void Kded::updateResourceList()
{
  delete KSycoca::self();

  if (!b_checkUpdates) return;

  if (delayedCheck) return;

  TQStringList dirs = KSycoca::self()->allResourceDirs();
  // For each resource
  for( TQStringList::ConstIterator it = dirs.begin();
       it != dirs.end();
       ++it )
  {
     if (m_allResourceDirs.find(*it) == m_allResourceDirs.end())
     {
        m_allResourceDirs.append(*it);
        readDirectory(*it);
     }
  }
}

void Kded::crashHandler(int)
{
   DCOPClient::emergencyClose();
   if (_self) { // Don't restart if we were closing down
      tqWarning("Last DCOP call before KDED crash was from application '%s'\n"
         "to object '%s', function '%s'.",
         DCOPClient::postMortemSender(),
         DCOPClient::postMortemObject(),
         DCOPClient::postMortemFunction());
      tqWarning("Restarting KDED...\n");
      if (system("kded") < 0) {
         tqWarning("Unable to restart KDED!\n");
      }
   }
}

void Kded::installCrashHandler()
{
   TDECrash::setEmergencySaveFunction(crashHandler);
}

void Kded::recreate()
{
   recreate(false);
}

void Kded::runDelayedCheck()
{
   if( m_needDelayedCheck )
      recreate(false);
   m_needDelayedCheck = false;
}

void Kded::recreate(bool initial)
{
   m_recreateBusy = true;
   // Using TDELauncher here is difficult since we might not have a
   // database

   if (!initial)
   {
      updateDirWatch(); // Update tree first, to be sure to miss nothing.
      runBuildSycoca(this, TQ_SLOT(recreateDone()));
   }
   else
   {
      if(!delayedCheck)
         updateDirWatch(); // this would search all the directories
      runBuildSycoca();
      recreateDone();
      if(delayedCheck)
      {
         // do a proper tdesycoca check after a delay
         TQTimer::singleShot( 60000, this, TQ_SLOT( runDelayedCheck()));
         m_needDelayedCheck = true;
         delayedCheck = false;
      }
      else
         m_needDelayedCheck = false;
   }
}

void Kded::recreateDone()
{
   updateResourceList();

   for(; m_recreateCount; m_recreateCount--)
   {
      TQCString replyType = "void";
      TQByteArray replyData;
      DCOPClientTransaction *transaction = m_recreateRequests.first();
      if (transaction)
         kapp->dcopClient()->endTransaction(transaction, replyType, replyData);
      m_recreateRequests.remove(m_recreateRequests.begin());
   }
   m_recreateBusy = false;

   // Did a new request come in while building?
   if (!m_recreateRequests.isEmpty())
   {
      m_pTimer->start(2000, true /* single shot */ );
      m_recreateCount = m_recreateRequests.count();
   }
}

void Kded::dirDeleted(const TQString& path)
{
  update(path);
}

void Kded::update(const TQString& )
{
  if (!m_recreateBusy)
  {
    m_pTimer->start( 2000, true /* single shot */ );
  }
  else
  {
    m_recreateRequests.append(0);
  }
}

bool Kded::process(const TQCString &fun, const TQByteArray &data,
                           TQCString &replyType, TQByteArray &replyData)
{
  if (fun == "recreate()") {
    if (!m_recreateBusy)
    {
       if (m_recreateRequests.isEmpty())
       {
          m_pTimer->start(0, true /* single shot */ );
          m_recreateCount = 0;
       }
       m_recreateCount++;
    }
    m_recreateRequests.append(kapp->dcopClient()->beginTransaction());
    replyType = "void";
    return true;
  } else {
    return DCOPObject::process(fun, data, replyType, replyData);
  }
}


void Kded::readDirectory( const TQString& _path )
{
  TQString path( _path );
  if ( path.right(1) != "/" )
    path += "/";

  if ( m_pDirWatch->contains( path ) ) // Already seen this one?
     return;

  TQDir d( _path, TQString::null, TQDir::Unsorted, TQDir::Readable | TQDir::Executable | TQDir::Dirs | TQDir::Hidden );
  // set TQDir ...


  //************************************************************************
  //                           Setting dirs
  //************************************************************************

  m_pDirWatch->addDir(path);          // add watch on this dir

  if ( !d.exists() )                            // exists&isdir?
  {
    kdDebug(7020) << TQString(TQString("Does not exist! (%1)").arg(_path)) << endl;
    return;                             // return false
  }

  // Note: If some directory is gone, dirwatch will delete it from the list.

  //************************************************************************
  //                               Reading
  //************************************************************************
  TQString file;
  unsigned int i;                           // counter and string length.
  unsigned int count = d.count();
  for( i = 0; i < count; i++ )                        // check all entries
  {
     if (d[i] == "." || d[i] == ".." || d[i] == "magic")
       continue;                          // discard those ".", "..", "magic"...

     file = path;                           // set full path
     file += d[i];                          // and add the file name.

     readDirectory( file );      // yes, dive into it.
  }
}

bool Kded::isWindowRegistered(long windowId)
{
  return m_globalWindowIdList.find(windowId) != 0;

}

// DCOP
void Kded::registerWindowId(long windowId)
{
  m_globalWindowIdList.replace(windowId, &windowId);
  TQCString sender = callingDcopClient()->senderId();
  if( sender.isEmpty()) // local call
      sender = callingDcopClient()->appId();
  TQValueList<long> *windowIds = m_windowIdList.find(sender);
  if (!windowIds)
  {
    windowIds = new TQValueList<long>;
    m_windowIdList.insert(sender, windowIds);
  }
  windowIds->append(windowId);


  for(TQAsciiDictIterator<KDEDModule> it(m_modules); it.current(); ++it)
  {
     emit it.current()->windowRegistered(windowId);
  }
}

// DCOP
void Kded::unregisterWindowId(long windowId)
{
  m_globalWindowIdList.remove(windowId);
  TQCString sender = callingDcopClient()->senderId();
  if( sender.isEmpty()) // local call
      sender = callingDcopClient()->appId();
  TQValueList<long> *windowIds = m_windowIdList.find(sender);
  if (windowIds)
  {
     windowIds->remove(windowId);
     if (windowIds->isEmpty())
        m_windowIdList.remove(sender);
  }

  for(TQAsciiDictIterator<KDEDModule> it(m_modules); it.current(); ++it)
  {
    emit it.current()->windowUnregistered(windowId);
  }
}


static void sighandler(int /*sig*/)
{
    if (kapp)
       kapp->quit();
}

KUpdateD::KUpdateD()
{
    m_pDirWatch = new KDirWatch;
    m_pTimer = new TQTimer;
    connect(m_pTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(runKonfUpdate()));
    TQObject::connect( m_pDirWatch, TQ_SIGNAL(dirty(const TQString&)),
           this, TQ_SLOT(slotNewUpdateFile()));

    TQStringList dirs = TDEGlobal::dirs()->findDirs("data", "tdeconf_update");
    for( TQStringList::ConstIterator it = dirs.begin();
         it != dirs.end();
         ++it )
    {
       TQString path = *it;
       if (path[path.length()-1] != '/')
          path += "/";

       if (!m_pDirWatch->contains(path))
          m_pDirWatch->addDir(path);
    }
}

KUpdateD::~KUpdateD()
{
    delete m_pDirWatch;
    delete m_pTimer;
}

void KUpdateD::runKonfUpdate()
{
    ::runKonfUpdate();
}

void KUpdateD::slotNewUpdateFile()
{
    m_pTimer->start( 500, true /* single shot */ );
}

KHostnameD::KHostnameD(int pollInterval)
{
    m_Timer.start(pollInterval, false /* repetitive */ );
    connect(&m_Timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(checkHostname()));
    checkHostname();
}

KHostnameD::~KHostnameD()
{
    // Empty
}

void KHostnameD::checkHostname()
{
    char buf[1024+1];
    if (gethostname(buf, 1024) != 0)
       return;
    buf[sizeof(buf)-1] = '\0';

    if (m_hostname.isEmpty())
    {
       m_hostname = buf;
       return;
    }

    if (m_hostname == buf)
       return;

    TQCString newHostname = buf;

    runDontChangeHostname(m_hostname, newHostname);
    m_hostname = newHostname;
}


static TDECmdLineOptions options[] =
{
  { "check", I18N_NOOP("Check Sycoca database only once"), 0 },
  { "new-startup", "Internal", 0 },
  TDECmdLineLastOption
};

class KDEDQtDCOPObject : public DCOPObject
{
public:
  KDEDQtDCOPObject() : DCOPObject("qt/kded") { }

  virtual bool process(const TQCString &fun, const TQByteArray &data,
                       TQCString& replyType, TQByteArray &replyData)
    {
      if ( kapp && (fun == "quit()") )
      {
        kapp->quit();
        replyType = "void";
        return true;
      }
      return DCOPObject::process(fun, data, replyType, replyData);
    }

  QCStringList functions()
    {
       QCStringList res = DCOPObject::functions();
       res += "void quit()";
       return res;
    }
};

class KDEDApplication : public KUniqueApplication
{
public:
  KDEDApplication() : KUniqueApplication( )
    {
       startup = true;
       dcopClient()->connectDCOPSignal( "DCOPServer", "", "terminateTDE()",
                                        objId(), "quit()", false );
    }

  int newInstance()
    {
       if (startup) {
          startup = false;
          if( Kded::self()->newStartup())
             Kded::self()->initModules();
          else
	     TQTimer::singleShot(500, Kded::self(), TQ_SLOT(initModules()));
       } else 
          runBuildSycoca();

       return 0;
    }

  QCStringList functions()
    {
       QCStringList res = KUniqueApplication::functions();
       res += "bool loadModule(TQCString)";
       res += "bool unloadModule(TQCString)";
       res += "void registerWindowId(long int)";
       res += "void unregisterWindowId(long int)";
       res += "QCStringList loadedModules()";
       res += "void reconfigure()";
       res += "void loadSecondPhase()";
       res += "void quit()";
       return res;
    }

  bool process(const TQCString &fun, const TQByteArray &data,
               TQCString &replyType, TQByteArray &replyData)
  {
    if (fun == "loadModule(TQCString)") {
      TQCString module;
      TQDataStream arg( data, IO_ReadOnly );
      arg >> module;
      bool result = (Kded::self()->loadModule(module, false) != 0);
      replyType = "bool";
      TQDataStream _replyStream( replyData, IO_WriteOnly );
      _replyStream << result;
      return true;
    }
    else if (fun == "unloadModule(TQCString)") {
      TQCString module;
      TQDataStream arg( data, IO_ReadOnly );
      arg >> module;
      bool result = Kded::self()->unloadModule(module);
      replyType = "bool";
      TQDataStream _replyStream( replyData, IO_WriteOnly );
      _replyStream << result;
      return true;
    }
    else if (fun == "registerWindowId(long int)") {
      long windowId;
      TQDataStream arg( data, IO_ReadOnly );
      arg >> windowId;
      Kded::self()->setCallingDcopClient(callingDcopClient());
      Kded::self()->registerWindowId(windowId);
      replyType = "void";
      return true;
    }
     else if (fun == "unregisterWindowId(long int)") {
      long windowId;
      TQDataStream arg( data, IO_ReadOnly );
      arg >> windowId;
      Kded::self()->setCallingDcopClient(callingDcopClient());
      Kded::self()->unregisterWindowId(windowId);
      replyType = "void";
      return true;
    }
    else if (fun == "loadedModules()") {
      replyType = "QCStringList";
      TQDataStream _replyStream(replyData, IO_WriteOnly);
      _replyStream << Kded::self()->loadedModules();
      return true;
    }
    else if (fun == "reconfigure()") {
      config()->reparseConfiguration();
      Kded::self()->initModules();
      replyType = "void";
      return true;
    }
    else if (fun == "loadSecondPhase()") {
      Kded::self()->loadSecondPhase();
      replyType = "void";
      return true;
    }
    else if (fun == "quit()") {
      quit();
      replyType = "void";
      return true;
    }
    return KUniqueApplication::process(fun, data, replyType, replyData);
  }

  bool startup;
  KDEDQtDCOPObject kdedQtDcopObject;
};

extern "C" TDE_EXPORT int kdemain(int argc, char *argv[])
{
     TDEAboutData aboutData( "kded", I18N_NOOP("TDE Daemon"),
        "$Id$",
        I18N_NOOP("TDE Daemon - triggers Sycoca database updates when needed"));

     TDEApplication::installSigpipeHandler();

     TDECmdLineArgs::init(argc, argv, &aboutData);

     KUniqueApplication::addCmdLineOptions();

     TDECmdLineArgs::addCmdLineOptions( options );

     // this program is in tdelibs so it uses tdelibs as catalog
     TDELocale::setMainCatalogue("tdelibs");

     // WABA: Make sure not to enable session management.
     putenv(strdup("SESSION_MANAGER="));

     // Parse command line before checking DCOP
     TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

     // Check DCOP communication.
     {
        DCOPClient testDCOP;
        TQCString dcopName = testDCOP.registerAs("kded", false);
        if (dcopName.isEmpty())
        {
           kdFatal() << "DCOP communication problem!" << endl;
           return 1;
        }
     }

     TDEInstance *instance = new TDEInstance(&aboutData);
     TDEConfig *config = instance->config(); // Enable translations.

     if (args->isSet("check"))
     {
        config->setGroup("General");
        checkStamps = config->readBoolEntry("CheckFileStamps", true);
        runBuildSycoca();
        runKonfUpdate();
        exit(0);
     }

     if (!KUniqueApplication::start())
     {
        fprintf(stderr, "[kded] Daemon (kded) is already running.\n");
        exit(0);
     }

     KUniqueApplication::dcopClient()->setQtBridgeEnabled(false);

     config->setGroup("General");
     int HostnamePollInterval = config->readNumEntry("HostnamePollInterval", 5000);
     bool bCheckSycoca = config->readBoolEntry("CheckSycoca", true);
     bool bCheckUpdates = config->readBoolEntry("CheckUpdates", true);
     bool bCheckHostname = config->readBoolEntry("CheckHostname", true);
     checkStamps = config->readBoolEntry("CheckFileStamps", true);
     delayedCheck = config->readBoolEntry("DelayedCheck", false);

     Kded *kded = new Kded(bCheckSycoca, args->isSet("new-startup")); // Build data base

     signal(SIGTERM, sighandler);
     signal(SIGHUP, sighandler);
     KDEDApplication k;

     kded->recreate(true); // initial

     if (bCheckUpdates)
        (void) new KUpdateD; // Watch for updates

     runKonfUpdate(); // Run it once.

     if (bCheckHostname)
        (void) new KHostnameD(HostnamePollInterval); // Watch for hostname changes

     DCOPClient *client = kapp->dcopClient();
     TQObject::connect(client, TQ_SIGNAL(applicationRemoved(const TQCString&)),
             kded, TQ_SLOT(slotApplicationRemoved(const TQCString&)));
     client->setNotifications(true);
     client->setDaemonMode( true );

     // During startup kdesktop waits for KDED to finish.
     // Send a notifyDatabaseChanged signal even if the database hasn't
     // changed.
     // If the database changed, tdebuildsycoca's signal didn't go anywhere
     // anyway, because it was too early, so let's send this signal
     // unconditionnally (David)
     TQByteArray data;
     client->send( "*", "tdesycoca", "notifyDatabaseChanged()", data );
     client->send( "ksplash", "", "upAndRunning(TQString)",  TQString("kded"));
#ifdef TQ_WS_X11
     XEvent e;
     e.xclient.type = ClientMessage;
     e.xclient.message_type = XInternAtom( tqt_xdisplay(), "_KDE_SPLASH_PROGRESS", False );
     e.xclient.display = tqt_xdisplay();
     e.xclient.window = tqt_xrootwin();
     e.xclient.format = 8;
     strcpy( e.xclient.data.b, "kded" );
     XSendEvent( tqt_xdisplay(), tqt_xrootwin(), False, SubstructureNotifyMask, &e );
#endif
     int result = k.exec(); // keep running

     delete kded;
     delete instance; // Deletes config as well

     return result;
}

#include "kded.moc"

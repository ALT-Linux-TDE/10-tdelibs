/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *            (C) 1999 Waldo Bastian <bastian@kde.org>
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

#ifndef __kded_h__
#define __kded_h__ 

#include <tqobject.h>
#include <tqstring.h>
#include <tqtimer.h>
#include <tqasciidict.h>
#include <tqintdict.h>

#include <dcopclient.h>
#include <dcopobject.h>

#include <tdesycoca.h>
#include <tdesycocatype.h>
#include <kdedmodule.h>
#include <klibloader.h>

class KDirWatch;
class KService;

// No need for this in libtdeio - apps only get readonly access
class Kded : public TQObject, public DCOPObject, public DCOPObjectProxy
{
  TQ_OBJECT
public:
   Kded(bool checkUpdates, bool new_startup);
   virtual ~Kded();

   static Kded *self() { return _self;}
   /**
    * Catch calls to unknown objects.
    */
   bool process(const TQCString &obj, const TQCString &fun, 
                const TQByteArray &data, 
		TQCString &replyType, TQByteArray &replyData);

   /**
    * process DCOP message.  Only calls to "recreate" are supported at
    * this time.
    */
   bool process(const TQCString &fun, const TQByteArray &data, 
		TQCString &replyType, TQByteArray &replyData);

   virtual QCStringList functions();

   void noDemandLoad(const TQString &obj); // Don't load obj on demand

   KDEDModule *loadModule(const TQCString &obj, bool onDemand);
   KDEDModule *loadModule(const KService *service, bool onDemand);
   QCStringList loadedModules();
   bool unloadModule(const TQCString &obj);
   bool isWindowRegistered(long windowId);
   void registerWindowId(long windowId);
   void unregisterWindowId(long windowId);
   void recreate(bool initial);
   void loadSecondPhase();

public slots:
   /**
    * Loads / unloads modules according to config.
    */
   void initModules();

   /**
    * Recreate the database file
    */
   void recreate();

   /**
    * Recreating finished
    */
   void recreateDone();

   /**
    * Collect all directories to watch
    */
   void updateDirWatch();

   /**
    * Update directories to watch
    */
   void updateResourceList();

   /**
    * An application unregistered itself with DCOP
    */
   void slotApplicationRemoved(const TQCString &appId);

   /**
    * A KDEDModule is about to get destroyed.
    */
   void slotKDEDModuleRemoved(KDEDModule *);

protected slots:

   /**
    * @internal Triggers rebuilding
    */
   void dirDeleted(const TQString& path);
 
   /**
    * @internal Triggers rebuilding
    */
   void update (const TQString& dir );

   /**
    * @internal Installs crash handler
    */
   void installCrashHandler();
   
   void runDelayedCheck();

protected:
   /**
    * Scans dir for new files and new subdirectories.
    */
   void readDirectory(const TQString& dir );
   

   static void crashHandler(int);

   /**
    * Pointer to the dirwatch class which tells us, when some directories
    * changed.
    * Slower polling for remote file systems is now done in KDirWatch (JW).
    */
   KDirWatch* m_pDirWatch;

   bool b_checkUpdates;

   /**
    * When a desktop file is updated, a timer is started (5 sec)
    * before rebuilding the binary - so that multiple updates result
    * in only one rebuilding.
    */
   TQTimer* m_pTimer;
   
   TQValueList<DCOPClientTransaction *> m_recreateRequests;
   int m_recreateCount;
   bool m_recreateBusy;
   
   TQAsciiDict<KDEDModule> m_modules;
   TQAsciiDict<KLibrary> m_libs;
   TQAsciiDict<TQObject> m_dontLoad;
   TQAsciiDict<TQValueList<long> > m_windowIdList;
   TQIntDict<long> m_globalWindowIdList;
   TQStringList m_allResourceDirs;
   bool m_needDelayedCheck;
   bool m_newStartup;
public:
   bool newStartup() const { return m_newStartup; }
private:
     
   static Kded *_self;
};

class KUpdateD : public TQObject
{
   TQ_OBJECT
public:
   KUpdateD();
   ~KUpdateD();
   
public slots:
   void runKonfUpdate();
   void slotNewUpdateFile();

private:
   /**
    * Pointer to the dirwatch class which tells us, when some directories
    * changed.
    * Slower polling for remote file systems is now done in KDirWatch (JW).
    */
   KDirWatch* m_pDirWatch;

   /**
    * When a desktop file is updated, a timer is started (5 sec)
    * before rebuilding the binary - so that multiple updates result
    * in only one rebuilding.
    */
   TQTimer* m_pTimer;
};

class KHostnameD : public TQObject
{
   TQ_OBJECT
public:
   KHostnameD(int pollInterval);
   ~KHostnameD();
   
public slots:
   void checkHostname();

private:
   /**
    * Timer for interval hostname checking.
    */
   TQTimer m_Timer;
   TQCString m_hostname;
};

#endif

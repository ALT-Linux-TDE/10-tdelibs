/* This file is part of the KDE libraries
   Copyright (C) 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

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


// KSimpleDirWatch is a basic copy of KDirWatch
// but with the TDEIO linking requirement removed

#include <config.h>
#include <errno.h>

#ifdef HAVE_DNOTIFY
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#endif


#include <sys/stat.h>
#include <assert.h>
#include <tqdir.h>
#include <tqfile.h>
#include <tqintdict.h>
#include <tqptrlist.h>
#include <tqsocketnotifier.h>
#include <tqstringlist.h>
#include <tqtimer.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeconfig.h>
#include <tdeglobal.h>
#include <kstaticdeleter.h>
#include <kde_file.h>

// debug
#include <sys/ioctl.h>

#ifdef Q_OS_SOLARIS
#include <sys/filio.h> /* FIONREAD is defined here */
#endif /* solaris */

#ifdef HAVE_INOTIFY
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#ifdef Q_OS_LINUX
#include <linux/types.h>
#endif /* Linux */
// Linux kernel headers are documented to not compile
#define _S390_BITOPS_H
#include <sys/inotify.h>

#ifndef __NR_inotify_init
#if defined(__i386__)
#define __NR_inotify_init       291
#define __NR_inotify_add_watch  292
#define __NR_inotify_rm_watch   293
#endif
#if defined(__PPC__)
#define __NR_inotify_init       275
#define __NR_inotify_add_watch  276
#define __NR_inotify_rm_watch   277
#endif
#if defined(__x86_64__)
#define __NR_inotify_init       253
#define __NR_inotify_add_watch  254
#define __NR_inotify_rm_watch   255
#endif
#endif

#ifndef  IN_ONLYDIR
#define  IN_ONLYDIR 0x01000000
#endif

#ifndef IN_DONT_FOLLOW
#define IN_DONT_FOLLOW 0x02000000
#endif

#ifndef IN_MOVE_SELF
#define IN_MOVE_SELF 0x00000800
#endif

#endif

#include <sys/utsname.h>

#include "ksimpledirwatch.h"
#include "ksimpledirwatch_p.h"

#define NO_NOTIFY (time_t) 0

static KSimpleDirWatchPrivate* dwp_self = 0;

#ifdef HAVE_DNOTIFY

static int dnotify_signal = 0;

/* DNOTIFY signal handler
 *
 * As this is called asynchronously, only a flag is set and
 * a rescan is requested.
 * This is done by writing into a pipe to trigger a TQSocketNotifier
 * watching on this pipe: a timer is started and after a timeout,
 * the rescan is done.
 */
void KSimpleDirWatchPrivate::dnotify_handler(int, siginfo_t *si, void *)
{
  if (!dwp_self) return;

  // write might change errno, we have to save it and restore it
  // (Richard Stevens, Advanced programming in the Unix Environment)
  int saved_errno = errno;

  Entry* e = dwp_self->fd_Entry.find(si->si_fd);

//  kdDebug(7001) << "DNOTIFY Handler: fd " << si->si_fd << " path "
//		<< TQString(e ? e->path:"unknown") << endl;

  if(e && e->dn_fd == si->si_fd)
    e->dirty = true;

  char c = 0;
  write(dwp_self->mPipe[1], &c, 1);
  errno = saved_errno;
}

static struct sigaction old_sigio_act;
/* DNOTIFY SIGIO signal handler
 *
 * When the kernel queue for the dnotify_signal overflows, a SIGIO is send.
 */
void KSimpleDirWatchPrivate::dnotify_sigio_handler(int sig, siginfo_t *si, void *p)
{
  if (dwp_self)
  {
    // write might change errno, we have to save it and restore it
    // (Richard Stevens, Advanced programming in the Unix Environment)
    int saved_errno = errno;

    dwp_self->rescan_all = true;
    char c = 0;
    write(dwp_self->mPipe[1], &c, 1);

    errno = saved_errno;
  }

  // Call previous signal handler
  if (old_sigio_act.sa_flags & SA_SIGINFO)
  {
    if (old_sigio_act.sa_sigaction)
      (*old_sigio_act.sa_sigaction)(sig, si, p);
  }
  else
  {
    if ((old_sigio_act.sa_handler != SIG_DFL) &&
        (old_sigio_act.sa_handler != SIG_IGN))
      (*old_sigio_act.sa_handler)(sig);
  }
}
#endif


//
// Class KSimpleDirWatchPrivate (singleton)
//

/* All entries (files/directories) to be watched in the
 * application (coming from multiple KSimpleDirWatch instances)
 * are registered in a single KSimpleDirWatchPrivate instance.
 *
 * At the moment, the following methods for file watching
 * are supported:
 * - Polling: All files to be watched are polled regularly
 *   using stat (more precise: TQFileInfo.lastModified()).
 *   The polling frequency is determined from global tdeconfig
 *   settings, defaulting to 500 ms for local directories
 *   and 5000 ms for remote mounts
 * - FAM (File Alternation Monitor): first used on IRIX, SGI
 *   has ported this method to LINUX. It uses a kernel part
 *   (IMON, sending change events to /dev/imon) and a user
 *   level damon (fam), to which applications connect for
 *   notification of file changes. For NFS, the fam damon
 *   on the NFS server machine is used; if IMON is not built
 *   into the kernel, fam uses polling for local files.
 * - DNOTIFY: In late LINUX 2.3.x, directory notification was
 *   introduced. By opening a directory, you can request for
 *   UNIX signals to be sent to the process when a directory
 *   is changed.
 * - INOTIFY: In LINUX 2.6.13, inode change notification was
 *   introduced. You're now able to watch arbitrary inode's
 *   for changes, and even get notification when they're
 *   unmounted.
 */

KSimpleDirWatchPrivate::KSimpleDirWatchPrivate()
  : rescan_timer(0, "KSimpleDirWatchPrivate::rescan_timer")
{
  timer = new TQTimer(this, "KSimpleDirWatchPrivate::timer");
  connect (timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(slotRescan()));
  freq = 3600000; // 1 hour as upper bound
  statEntries = 0;
  delayRemove = false;
  m_ref = 0;

  TDEConfigGroup config(TDEGlobal::config(), TQCString("DirWatch"));
  m_nfsPollInterval = config.readNumEntry("NFSPollInterval", 5000);
  m_PollInterval = config.readNumEntry("PollInterval", 500);

  TQString available("Stat");

  // used for FAM and DNOTIFY
  rescan_all = false;
  connect(&rescan_timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(slotRescan()));

#ifdef HAVE_FAM
  // It's possible that FAM server can't be started
  if (FAMOpen(&fc) ==0) {
    available += ", FAM";
    use_fam=true;
    sn = new TQSocketNotifier( FAMCONNECTION_GETFD(&fc),
			      TQSocketNotifier::Read, this);
    connect( sn, TQ_SIGNAL(activated(int)),
 	     this, TQ_SLOT(famEventReceived()) );
  }
  else {
    kdDebug(7001) << "Can't use FAM (fam daemon not running?)" << endl;
    use_fam=false;
  }
#endif

#ifdef HAVE_INOTIFY
  supports_inotify = true;

  m_inotify_fd = inotify_init();

  if ( m_inotify_fd <= 0 ) {
    kdDebug(7001) << "Can't use Inotify, kernel doesn't support it" << endl;
    supports_inotify = false;
  }

  {
    struct utsname uts;
    int major, minor, patch;
    if (uname(&uts) < 0)
      supports_inotify = false; // *shrug*
    else if (sscanf(uts.release, "%d.%d.%d", &major, &minor, &patch) != 3)
      supports_inotify = false; // *shrug*
    else if( major * 1000000 + minor * 1000 + patch < 2006014 ) { // <2.6.14
      kdDebug(7001) << "Can't use INotify, Linux kernel too old" << endl;
      supports_inotify = false;
    }
  }

  if ( supports_inotify ) {
    available += ", Inotify";
    fcntl(m_inotify_fd, F_SETFD, FD_CLOEXEC);

    mSn = new TQSocketNotifier( m_inotify_fd, TQSocketNotifier::Read, this );
    connect( mSn, TQ_SIGNAL(activated( int )), this, TQ_SLOT( slotActivated() ) );
  }
#endif

#ifdef HAVE_DNOTIFY

  // if we have inotify, disable dnotify.
#ifdef HAVE_INOTIFY
  supports_dnotify = !supports_inotify;
#else
  // otherwise, not guilty until proven guilty.
  supports_dnotify = true;
#endif

  struct utsname uts;
  int major, minor, patch;
  if (uname(&uts) < 0)
    supports_dnotify = false; // *shrug*
  else if (sscanf(uts.release, "%d.%d.%d", &major, &minor, &patch) != 3)
    supports_dnotify = false; // *shrug*
  else if( major * 1000000 + minor * 1000 + patch < 2004019 ) { // <2.4.19
    kdDebug(7001) << "Can't use DNotify, Linux kernel too old" << endl;
    supports_dnotify = false;
  }

  if( supports_dnotify ) {
    available += ", DNotify";

    pipe(mPipe);
    fcntl(mPipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(mPipe[1], F_SETFD, FD_CLOEXEC);
    fcntl(mPipe[0], F_SETFL, O_NONBLOCK | fcntl(mPipe[0], F_GETFL));
    fcntl(mPipe[1], F_SETFL, O_NONBLOCK | fcntl(mPipe[1], F_GETFL));
    mSn = new TQSocketNotifier( mPipe[0], TQSocketNotifier::Read, this);
    connect(mSn, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotActivated()));
    // Install the signal handler only once
    if ( dnotify_signal == 0 )
    {
       dnotify_signal = SIGRTMIN + 8;

       struct sigaction act;
       act.sa_sigaction = KSimpleDirWatchPrivate::dnotify_handler;
       sigemptyset(&act.sa_mask);
       act.sa_flags = SA_SIGINFO;
#ifdef SA_RESTART
       act.sa_flags |= SA_RESTART;
#endif
       sigaction(dnotify_signal, &act, NULL);

       act.sa_sigaction = KSimpleDirWatchPrivate::dnotify_sigio_handler;
       sigaction(SIGIO, &act, &old_sigio_act);
    }
  }
  else
  {
    mPipe[0] = -1;
    mPipe[1] = -1;
  }
#endif

  kdDebug(7001) << "Available methods: " << available << endl;
}

/* This is called on app exit (KStaticDeleter) */
KSimpleDirWatchPrivate::~KSimpleDirWatchPrivate()
{
  timer->stop();

  /* remove all entries being watched */
  removeEntries(0);

#ifdef HAVE_FAM
  if (use_fam) {
    FAMClose(&fc);
    kdDebug(7001) << "KSimpleDirWatch deleted (FAM closed)" << endl;
  }
#endif
#ifdef HAVE_INOTIFY
  if ( supports_inotify )
    ::close( m_inotify_fd );
#endif
#ifdef HAVE_DNOTIFY
  close(mPipe[0]);
  close(mPipe[1]);
#endif
}

#include <stdlib.h>

void KSimpleDirWatchPrivate::slotActivated()
{
#ifdef HAVE_DNOTIFY
  if ( supports_dnotify )
  {
    char dummy_buf[4096];
    read(mPipe[0], &dummy_buf, 4096);

    if (!rescan_timer.isActive())
      rescan_timer.start(m_PollInterval, true /* singleshot */);

    return;
  }
#endif

#ifdef HAVE_INOTIFY
  if ( !supports_inotify )
    return;

  int pending = -1;
  int offset = 0;
  char buf[4096];
  assert( m_inotify_fd > -1 );
  ioctl( m_inotify_fd, FIONREAD, &pending );

  while ( pending > 0 ) {

    if ( pending > (int)sizeof( buf ) )
      pending = sizeof( buf );

    pending = read( m_inotify_fd, buf, pending);

    while ( pending > 0 ) {
      struct inotify_event *event = (struct inotify_event *) &buf[offset];
      pending -= sizeof( struct inotify_event ) + event->len;
      offset += sizeof( struct inotify_event ) + event->len;

      TQString path;
      if ( event->len )
        path = TQFile::decodeName( TQCString( event->name, event->len ) );

      if ( path.length() && isNoisyFile( path.latin1() ) )
        continue;

      kdDebug(7001) << "ev wd: " << event->wd << " mask " << event->mask << " path: " << path << endl;

      // now we're in deep trouble of finding the
      // associated entries
      // for now, we suck and iterate
      for ( EntryMap::Iterator it = m_mapEntries.begin();
            it != m_mapEntries.end(); ++it ) {
        Entry* e = &( *it );
        if ( e->wd == event->wd ) {
          e->dirty = true;

          if ( 1 || e->isDir) {
            if( event->mask & IN_DELETE_SELF) {
              kdDebug(7001) << "-->got deleteself signal for " << e->path << endl;
              e->m_status = NonExistent;
              if (e->isDir)
                addEntry(0, TQDir::cleanDirPath(e->path+"/.."), e, true);
              else
                addEntry(0, TQFileInfo(e->path).dirPath(true), e, true);
            }
            if ( event->mask & IN_IGNORED ) {
              e->wd = 0;
            }
            if ( event->mask & (IN_CREATE|IN_MOVED_TO) ) {
              Entry *sub_entry = e->m_entries.first();
              for(;sub_entry; sub_entry = e->m_entries.next())
                if (sub_entry->path == e->path + "/" + path) break;

              if (sub_entry /*&& sub_entry->isDir*/) {
                removeEntry(0,e->path, sub_entry);
                KDE_struct_stat stat_buf;
                TQCString tpath = TQFile::encodeName(path);
                KDE_stat(tpath, &stat_buf);

                //sub_entry->isDir = S_ISDIR(stat_buf.st_mode);
                //sub_entry->m_ctime = stat_buf.st_ctime;
                //sub_entry->m_status = Normal;
                //sub_entry->m_nlink = stat_buf.st_nlink;

                if(!useINotify(sub_entry))
                  useStat(sub_entry);
                sub_entry->dirty = true;
              }
            }
          }

          if (!rescan_timer.isActive())
            rescan_timer.start(m_PollInterval, true /* singleshot */);

          break; // there really should be only one matching wd
        }
      }

    }
  }
#endif
}

/* In DNOTIFY/FAM mode, only entries which are marked dirty are scanned.
 * We first need to mark all yet nonexistent, but possible created
 * entries as dirty...
 */
void KSimpleDirWatchPrivate::Entry::propagate_dirty()
{
  for (TQPtrListIterator<Entry> sub_entry (m_entries); 
       sub_entry.current(); ++sub_entry)
  {
     if (!sub_entry.current()->dirty)
     {
        sub_entry.current()->dirty = true;
        sub_entry.current()->propagate_dirty();
     }
  }
}


/* A KSimpleDirWatch instance is interested in getting events for
 * this file/Dir entry.
 */
void KSimpleDirWatchPrivate::Entry::addClient(KSimpleDirWatch* instance)
{
  Client* client = m_clients.first();
  for(;client; client = m_clients.next())
    if (client->instance == instance) break;

  if (client) {
    client->count++;
    return;
  }

  client = new Client;
  client->instance = instance;
  client->count = 1;
  client->watchingStopped = instance->isStopped();
  client->pending = NoChange;

  m_clients.append(client);
}

void KSimpleDirWatchPrivate::Entry::removeClient(KSimpleDirWatch* instance)
{
  Client* client = m_clients.first();
  for(;client; client = m_clients.next())
    if (client->instance == instance) break;

  if (client) {
    client->count--;
    if (client->count == 0) {
      m_clients.removeRef(client);
      delete client;
    }
  }
}

/* get number of clients */
int KSimpleDirWatchPrivate::Entry::clients()
{
  int clients = 0;
  Client* client = m_clients.first();
  for(;client; client = m_clients.next())
    clients += client->count;

  return clients;
}


KSimpleDirWatchPrivate::Entry* KSimpleDirWatchPrivate::entry(const TQString& _path)
{
// we only support absolute paths
  if (TQDir::isRelativePath(_path)) {
    return 0;
  }

  TQString path = _path;

  if ( path.length() > 1 && path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  EntryMap::Iterator it = m_mapEntries.find( path );
  if ( it == m_mapEntries.end() )
    return 0;
  else
    return &(*it);
}

// set polling frequency for a entry and adjust global freq if needed
void KSimpleDirWatchPrivate::useFreq(Entry* e, int newFreq)
{
  e->freq = newFreq;

  // a reasonable frequency for the global polling timer
  if (e->freq < freq) {
    freq = e->freq;
    if (timer->isActive()) timer->changeInterval(freq);
    kdDebug(7001) << "Global Poll Freq is now " << freq << " msec" << endl;
  }
}


#ifdef HAVE_FAM
// setup FAM notification, returns false if not possible
bool KSimpleDirWatchPrivate::useFAM(Entry* e)
{
  if (!use_fam) return false;

  // handle FAM events to avoid deadlock
  // (FAM sends back all files in a directory when monitoring)
  famEventReceived();

  e->m_mode = FAMMode;
  e->dirty = false;

  if (e->isDir) {
    if (e->m_status == NonExistent) {
      // If the directory does not exist we watch the parent directory
      addEntry(0, TQDir::cleanDirPath(e->path+"/.."), e, true);
    }
    else {
      int res =FAMMonitorDirectory(&fc, TQFile::encodeName(e->path),
				   &(e->fr), e);
      if (res<0) {
	e->m_mode = UnknownMode;
	use_fam=false;
	return false;
      }
      kdDebug(7001) << " Setup FAM (Req "
		    << FAMREQUEST_GETREQNUM(&(e->fr))
		    << ") for " << e->path << endl;
    }
  }
  else {
    if (e->m_status == NonExistent) {
      // If the file does not exist we watch the directory
      addEntry(0, TQFileInfo(e->path).dirPath(true), e, true);
    }
    else {
      int res = FAMMonitorFile(&fc, TQFile::encodeName(e->path),
			       &(e->fr), e);
      if (res<0) {
	e->m_mode = UnknownMode;
	use_fam=false;
	return false;
      }

      kdDebug(7001) << " Setup FAM (Req "
		    << FAMREQUEST_GETREQNUM(&(e->fr))
		    << ") for " << e->path << endl;
    }
  }

  // handle FAM events to avoid deadlock
  // (FAM sends back all files in a directory when monitoring)
  famEventReceived();

  return true;
}
#endif


#ifdef HAVE_DNOTIFY
// setup DNotify notification, returns false if not possible
bool KSimpleDirWatchPrivate::useDNotify(Entry* e)
{
  e->dn_fd = 0;
  e->dirty = false;
  if (!supports_dnotify) return false;

  e->m_mode = DNotifyMode;

  if (e->isDir) {
    if (e->m_status == Normal) {
      int fd = KDE_open(TQFile::encodeName(e->path).data(), O_RDONLY);
      // Migrate fd to somewhere above 128. Some libraries have
      // constructs like:
      //    fd = socket(...)
      //    if (fd > ARBITRARY_LIMIT)
      //       return error;
      //
      // Since programs might end up using a lot of KSimpleDirWatch objects
      // for a rather long time the above braindamage could get
      // triggered.
      //
      // By moving the ksimpledirwatch fd's to > 128, calls like socket() will keep
      // returning fd's < ARBITRARY_LIMIT for a bit longer.
      int fd2 = fcntl(fd, F_DUPFD, 128);
      if (fd2 >= 0)
      {
        close(fd);
        fd = fd2;
      }
      if (fd<0) {
	e->m_mode = UnknownMode;
	return false;
      }

      int mask = DN_DELETE|DN_CREATE|DN_RENAME|DN_MULTISHOT;
      // if dependant is a file watch, we check for MODIFY & ATTRIB too
      for(Entry* dep=e->m_entries.first();dep;dep=e->m_entries.next())
	if (!dep->isDir) { mask |= DN_MODIFY|DN_ATTRIB; break; }

      if(fcntl(fd, F_SETSIG, dnotify_signal) < 0 ||
	 fcntl(fd, F_NOTIFY, mask) < 0) {

	kdDebug(7001) << "Not using Linux Directory Notifications."
		      << endl;
	supports_dnotify = false;
	::close(fd);
	e->m_mode = UnknownMode;
	return false;
      }

      fd_Entry.replace(fd, e);
      e->dn_fd = fd;

      kdDebug(7001) << " Setup DNotify (fd " << fd
		    << ") for " << e->path << endl;
    }
    else { // NotExisting
      addEntry(0, TQDir::cleanDirPath(e->path+"/.."), e, true);
    }
  }
  else { // File
    // we always watch the directory (DNOTIFY can't watch files alone)
    // this notifies us about changes of files therein
    addEntry(0, TQFileInfo(e->path).dirPath(true), e, true);
  }

  return true;
}
#endif

#ifdef HAVE_INOTIFY
// setup INotify notification, returns false if not possible
bool KSimpleDirWatchPrivate::useINotify( Entry* e )
{
  e->wd = 0;
  e->dirty = false;
  if (!supports_inotify) return false;

  e->m_mode = INotifyMode;

  int mask = IN_DELETE|IN_DELETE_SELF|IN_CREATE|IN_MOVE|IN_MOVE_SELF|IN_DONT_FOLLOW;
  if(!e->isDir)
    mask |= IN_MODIFY|IN_ATTRIB;
  else
    mask |= IN_ONLYDIR;

  // if dependant is a file watch, we check for MODIFY & ATTRIB too
  for(Entry* dep=e->m_entries.first();dep;dep=e->m_entries.next()) {
    if (!dep->isDir) { mask |= IN_MODIFY|IN_ATTRIB; break; }
  }

  if ( ( e->wd = inotify_add_watch( m_inotify_fd,
        TQFile::encodeName( e->path ), mask) ) > 0 )
    return true;

  if ( e->m_status == NonExistent ) {
    if (e->isDir) 
      addEntry(0, TQDir::cleanDirPath(e->path+"/.."), e, true);
    else
      addEntry(0, TQFileInfo(e->path).dirPath(true), e, true);
    return true;
  }

  return false;
}
#endif

bool KSimpleDirWatchPrivate::useStat(Entry* e)
{
  useFreq(e, m_PollInterval);

  if (e->m_mode != StatMode) {
    e->m_mode = StatMode;
    statEntries++;

    if ( statEntries == 1 ) {
      // if this was first STAT entry (=timer was stopped)
      timer->start(freq);      // then start the timer
      kdDebug(7001) << " Started Polling Timer, freq " << freq << endl;
    }
  }

  kdDebug(7001) << " Setup Stat (freq " << e->freq
		<< ") for " << e->path << endl;

  return true;
}


/* If <instance> !=0, this KSimpleDirWatch instance wants to watch at <_path>,
 * providing in <isDir> the type of the entry to be watched.
 * Sometimes, entries are dependant on each other: if <sub_entry> !=0,
 * this entry needs another entry to watch himself (when notExistent).
 */
void KSimpleDirWatchPrivate::addEntry(KSimpleDirWatch* instance, const TQString& _path,
				Entry* sub_entry, bool isDir)
{
  TQString path = _path;
  if (path.startsWith("/dev/") || (path == "/dev"))
    return; // Don't even go there.

  if ( path.length() > 1 && path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  EntryMap::Iterator it = m_mapEntries.find( path );
  if ( it != m_mapEntries.end() )
  {
    if (sub_entry) {
       (*it).m_entries.append(sub_entry);
       kdDebug(7001) << "Added already watched Entry " << path
		     << " (for " << sub_entry->path << ")" << endl;

#ifdef HAVE_DNOTIFY
     {
       Entry* e = &(*it);
       if( (e->m_mode == DNotifyMode) && (e->dn_fd > 0) ) {
         int mask = DN_DELETE|DN_CREATE|DN_RENAME|DN_MULTISHOT;
         // if dependant is a file watch, we check for MODIFY & ATTRIB too
         for(Entry* dep=e->m_entries.first();dep;dep=e->m_entries.next())
     	   if (!dep->isDir) { mask |= DN_MODIFY|DN_ATTRIB; break; }
	 if( fcntl(e->dn_fd, F_NOTIFY, mask) < 0) { // shouldn't happen
	   ::close(e->dn_fd);
	   e->m_mode = UnknownMode;
  	   fd_Entry.remove(e->dn_fd);
           e->dn_fd = 0;
           useStat( e );
         }
       }
     }
#endif

#ifdef HAVE_INOTIFY
     {
       Entry* e = &(*it);
       if( (e->m_mode == INotifyMode) && (e->wd > 0) ) {
         int mask = IN_DELETE|IN_DELETE_SELF|IN_CREATE|IN_MOVE|IN_MOVE_SELF|IN_DONT_FOLLOW;
         if(!e->isDir)
           mask |= IN_MODIFY|IN_ATTRIB;
         else
           mask |= IN_ONLYDIR;

         inotify_rm_watch (m_inotify_fd, e->wd);
         e->wd = inotify_add_watch( m_inotify_fd, TQFile::encodeName( e->path ), mask);
       }
    }
#endif
 
    }
    else {
       (*it).addClient(instance);
       kdDebug(7001) << "Added already watched Entry " << path
		     << " (now " <<  (*it).clients() << " clients)"
		     << TQString(TQString(" [%1]").arg(instance->name())) << endl;
    }
    return;
  }

  // we have a new path to watch

  KDE_struct_stat stat_buf;
  TQCString tpath = TQFile::encodeName(path);
  bool exists = (KDE_stat(tpath, &stat_buf) == 0);

  Entry newEntry;
  m_mapEntries.insert( path, newEntry );
  // the insert does a copy, so we have to use <e> now
  Entry* e = &(m_mapEntries[path]);

  if (exists) {
    e->isDir = S_ISDIR(stat_buf.st_mode);

    if (e->isDir && !isDir)
      kdWarning() << "KSimpleDirWatch: " << path << " is a directory. Use addDir!" << endl;
    else if (!e->isDir && isDir)
      kdWarning() << "KSimpleDirWatch: " << path << " is a file. Use addFile!" << endl;

    e->m_ctime = stat_buf.st_ctime;
    e->m_status = Normal;
    e->m_nlink = stat_buf.st_nlink;
  }
  else {
    e->isDir = isDir;
    e->m_ctime = invalid_ctime;
    e->m_status = NonExistent;
    e->m_nlink = 0;
  }

  e->path = path;
  if (sub_entry)
     e->m_entries.append(sub_entry);
  else
    e->addClient(instance);

  kdDebug(7001) << "Added " << (e->isDir ? "Dir ":"File ") << path
		<< (e->m_status == NonExistent ? " NotExisting" : "")
		<< (sub_entry ? TQString(TQString(" for %1").arg(sub_entry->path)) : TQString(""))
		<< (instance ? TQString(TQString(" [%1]").arg(instance->name())) : TQString(""))
		<< endl;


  // now setup the notification method
  e->m_mode = UnknownMode;
  e->msecLeft = 0;

  if ( isNoisyFile( tpath ) )
    return;

#ifdef HAVE_FAM
  if (useFAM(e)) return;
#endif

#ifdef HAVE_INOTIFY
  if (useINotify(e)) return;
#endif

#ifdef HAVE_DNOTIFY
  if (useDNotify(e)) return;
#endif

  useStat(e);
}


void KSimpleDirWatchPrivate::removeEntry( KSimpleDirWatch* instance,
				    const TQString& _path, Entry* sub_entry )
{
  kdDebug(7001) << "KSimpleDirWatchPrivate::removeEntry for '" << _path << "' sub_entry: " << sub_entry << endl;
  Entry* e = entry(_path);
  if (!e) {
    kdDebug(7001) << "KSimpleDirWatchPrivate::removeEntry can't handle '" << _path << "'" << endl;
    return;
  }

  if (sub_entry)
    e->m_entries.removeRef(sub_entry);
  else
    e->removeClient(instance);

  if (e->m_clients.count() || e->m_entries.count()) {
    kdDebug(7001) << "removeEntry: unwatched " << e->path << " " << _path << endl;
    return;
  }

  if (delayRemove) {
    // removeList is allowed to contain any entry at most once
    if (removeList.findRef(e)==-1)
      removeList.append(e);
    // now e->isValid() is false
    return;
  }

#ifdef HAVE_FAM
  if (e->m_mode == FAMMode) {
    if ( e->m_status == Normal) {
      FAMCancelMonitor(&fc, &(e->fr) );
      kdDebug(7001) << "Cancelled FAM (Req "
		    << FAMREQUEST_GETREQNUM(&(e->fr))
		    << ") for " << e->path << endl;
    }
    else {
      if (e->isDir)
	removeEntry(0, TQDir::cleanDirPath(e->path+"/.."), e);
      else
	removeEntry(0, TQFileInfo(e->path).dirPath(true), e);
    }
  }
#endif

#ifdef HAVE_INOTIFY
  kdDebug(7001) << "inotify remove " << ( e->m_mode == INotifyMode ) << " " << ( e->m_status == Normal )  << endl;
  if (e->m_mode == INotifyMode) {
    if ( e->m_status == Normal ) {
      (void) inotify_rm_watch( m_inotify_fd, e->wd );
      kdDebug(7001) << "Cancelled INotify (fd " <<
        m_inotify_fd << ", "  << e->wd <<
        ") for " << e->path << endl;
    }
    else {
      if (e->isDir)
	removeEntry(0, TQDir::cleanDirPath(e->path+"/.."), e);
      else
	removeEntry(0, TQFileInfo(e->path).dirPath(true), e);
    }
  }
#endif

#ifdef HAVE_DNOTIFY
  if (e->m_mode == DNotifyMode) {
    if (!e->isDir) {
      removeEntry(0, TQFileInfo(e->path).dirPath(true), e);
    }
    else { // isDir
      // must close the FD.
      if ( e->m_status == Normal) {
	if (e->dn_fd) {
	  ::close(e->dn_fd);
	  fd_Entry.remove(e->dn_fd);

	  kdDebug(7001) << "Cancelled DNotify (fd " << e->dn_fd
			<< ") for " << e->path << endl;
	  e->dn_fd = 0;

	}
      }
      else {
	removeEntry(0, TQDir::cleanDirPath(e->path+"/.."), e);
      }
    }
  }
#endif

  if (e->m_mode == StatMode) {
    statEntries--;
    if ( statEntries == 0 ) {
      timer->stop(); // stop timer if lists are empty
      kdDebug(7001) << " Stopped Polling Timer" << endl;
    }
  }

  kdDebug(7001) << "Removed " << (e->isDir ? "Dir ":"File ") << e->path
		<< (sub_entry ? TQString(TQString(" for %1").arg(sub_entry->path)) : TQString(""))
		<< (instance ? TQString(TQString(" [%1]").arg(instance->name())) : TQString(""))
		<< endl;
  m_mapEntries.remove( e->path ); // <e> not valid any more
}


/* Called from KSimpleDirWatch destructor:
 * remove <instance> as client from all entries
 */
void KSimpleDirWatchPrivate::removeEntries( KSimpleDirWatch* instance )
{
  TQPtrList<Entry> list;
  int minfreq = 3600000;

  // put all entries where instance is a client in list
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it ) {
    Client* c = (*it).m_clients.first();
    for(;c;c=(*it).m_clients.next())
      if (c->instance == instance) break;
    if (c) {
      c->count = 1; // forces deletion of instance as client
      list.append(&(*it));
    }
    else if ( (*it).m_mode == StatMode && (*it).freq < minfreq )
      minfreq = (*it).freq;
  }

  for(Entry* e=list.first();e;e=list.next())
    removeEntry(instance, e->path, 0);

  if (minfreq > freq) {
    // we can decrease the global polling frequency
    freq = minfreq;
    if (timer->isActive()) timer->changeInterval(freq);
    kdDebug(7001) << "Poll Freq now " << freq << " msec" << endl;
  }
}

// instance ==0: stop scanning for all instances
bool KSimpleDirWatchPrivate::stopEntryScan( KSimpleDirWatch* instance, Entry* e)
{
  int stillWatching = 0;
  Client* c = e->m_clients.first();
  for(;c;c=e->m_clients.next()) {
    if (!instance || instance == c->instance)
      c->watchingStopped = true;
    else if (!c->watchingStopped)
      stillWatching += c->count;
  }

  kdDebug(7001) << instance->name() << " stopped scanning " << e->path
		<< " (now " << stillWatching << " watchers)" << endl;

  if (stillWatching == 0) {
    // if nobody is interested, we don't watch
    e->m_ctime = invalid_ctime; // invalid
    e->m_status = NonExistent;
    //    e->m_status = Normal;
  }
  return true;
}

// instance ==0: start scanning for all instances
bool KSimpleDirWatchPrivate::restartEntryScan( KSimpleDirWatch* instance, Entry* e,
					 bool notify)
{
  int wasWatching = 0, newWatching = 0;
  Client* c = e->m_clients.first();
  for(;c;c=e->m_clients.next()) {
    if (!c->watchingStopped)
      wasWatching += c->count;
    else if (!instance || instance == c->instance) {
      c->watchingStopped = false;
      newWatching += c->count;
    }
  }
  if (newWatching == 0)
    return false;

  kdDebug(7001) << (instance ? instance->name() : "all") << " restarted scanning " << e->path
		<< " (now " << wasWatching+newWatching << " watchers)" << endl;

  // restart watching and emit pending events

  int ev = NoChange;
  if (wasWatching == 0) {
    if (!notify) {
      KDE_struct_stat stat_buf;
      bool exists = (KDE_stat(TQFile::encodeName(e->path), &stat_buf) == 0);
      if (exists) {
	e->m_ctime = stat_buf.st_ctime;
	e->m_status = Normal;
        e->m_nlink = stat_buf.st_nlink;
      }
      else {
	e->m_ctime = invalid_ctime;
	e->m_status = NonExistent;
        e->m_nlink = 0;
      }
    }
    e->msecLeft = 0;
    ev = scanEntry(e);
  }
  emitEvent(e,ev);

  return true;
}

// instance ==0: stop scanning for all instances
void KSimpleDirWatchPrivate::stopScan(KSimpleDirWatch* instance)
{
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it )
    stopEntryScan(instance, &(*it));
}


void KSimpleDirWatchPrivate::startScan(KSimpleDirWatch* instance,
				 bool notify, bool skippedToo )
{
  if (!notify)
    resetList(instance,skippedToo);

  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it )
    restartEntryScan(instance, &(*it), notify);

  // timer should still be running when in polling mode
}


// clear all pending events, also from stopped
void KSimpleDirWatchPrivate::resetList( KSimpleDirWatch* /*instance*/,
				  bool skippedToo )
{
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it ) {

    Client* c = (*it).m_clients.first();
    for(;c;c=(*it).m_clients.next())
      if (!c->watchingStopped || skippedToo)
	c->pending = NoChange;
  }
}

// Return event happened on <e>
//
int KSimpleDirWatchPrivate::scanEntry(Entry* e)
{
#ifdef HAVE_FAM
  if (e->m_mode == FAMMode) {
    // we know nothing has changed, no need to stat
    if(!e->dirty) return NoChange;
    e->dirty = false;
  }
#endif

  // Shouldn't happen: Ignore "unknown" notification method
  if (e->m_mode == UnknownMode) return NoChange;

#if defined ( HAVE_DNOTIFY ) || defined( HAVE_INOTIFY )
  if (e->m_mode == DNotifyMode || e->m_mode == INotifyMode ) {
    // we know nothing has changed, no need to stat
    if(!e->dirty) return NoChange;
    kdDebug(7001) << "scanning " << e->path << " " << e->m_status << " " << e->m_ctime << endl;
    e->dirty = false;
  }
#endif

  if (e->m_mode == StatMode) {
    // only scan if timeout on entry timer happens;
    // e.g. when using 500msec global timer, a entry
    // with freq=5000 is only watched every 10th time

    e->msecLeft -= freq;
    if (e->msecLeft>0) return NoChange;
    e->msecLeft += e->freq;
  }

  KDE_struct_stat stat_buf;
  bool exists = (KDE_stat(TQFile::encodeName(e->path), &stat_buf) == 0);
  if (exists) {

    if (e->m_status == NonExistent) {
      e->m_ctime = stat_buf.st_ctime;
      e->m_status = Normal;
      e->m_nlink = stat_buf.st_nlink;
      return Created;
    }

    if ( (e->m_ctime != invalid_ctime) &&
	 ((stat_buf.st_ctime != e->m_ctime) ||
	  (stat_buf.st_nlink != (nlink_t) e->m_nlink)) ) {
      e->m_ctime = stat_buf.st_ctime;
      e->m_nlink = stat_buf.st_nlink;
      return Changed;
    }

    return NoChange;
  }

  // dir/file doesn't exist

  if (e->m_ctime == invalid_ctime && e->m_status == NonExistent) {
    e->m_nlink = 0;
    e->m_status = NonExistent;
    return NoChange;
  }

  e->m_ctime = invalid_ctime;
  e->m_nlink = 0;
  e->m_status = NonExistent;

  return Deleted;
}

/* Notify all interested KSimpleDirWatch instances about a given event on an entry
 * and stored pending events. When watching is stopped, the event is
 * added to the pending events.
 */
void KSimpleDirWatchPrivate::emitEvent(Entry* e, int event, const TQString &fileName)
{
  TQString path = e->path;
  if (!fileName.isEmpty()) {
    if (!TQDir::isRelativePath(fileName))
      path = fileName;
    else
#ifdef Q_OS_UNIX
      path += "/" + fileName;
#elif defined(TQ_WS_WIN)
      //current drive is passed instead of /
      path += TQDir::currentDirPath().left(2) + "/" + fileName;
#endif
  }

  TQPtrListIterator<Client> cit( e->m_clients );
  for ( ; cit.current(); ++cit )
  {
    Client* c = cit.current();

    if (c->instance==0 || c->count==0) continue;

    if (c->watchingStopped) {
      // add event to pending...
      if (event == Changed)
	c->pending |= event;
      else if (event == Created || event == Deleted)
	c->pending = event;
      continue;
    }
    // not stopped
    if (event == NoChange || event == Changed)
      event |= c->pending;
    c->pending = NoChange;
    if (event == NoChange) continue;

    if (event & Deleted) {
      c->instance->setDeleted(path);
      // emit only Deleted event...
      continue;
    }

    if (event & Created) {
      c->instance->setCreated(path);
      // possible emit Change event after creation
    }

    if (event & Changed)
      c->instance->setDirty(path);
  }
}

// Remove entries which were marked to be removed
void KSimpleDirWatchPrivate::slotRemoveDelayed()
{
  Entry* e;
  delayRemove = false;
  for(e=removeList.first();e;e=removeList.next())
    removeEntry(0, e->path, 0);
  removeList.clear();
}

/* Scan all entries to be watched for changes. This is done regularly
 * when polling and once after a DNOTIFY signal. This is NOT used by FAM.
 */
void KSimpleDirWatchPrivate::slotRescan()
{
  EntryMap::Iterator it;

  // People can do very long things in the slot connected to dirty(),
  // like showing a message box. We don't want to keep polling during
  // that time, otherwise the value of 'delayRemove' will be reset.
  bool timerRunning = timer->isActive();
  if ( timerRunning )
    timer->stop();

  // We delay deletions of entries this way.
  // removeDir(), when called in slotDirty(), can cause a crash otherwise
  delayRemove = true;

#if defined(HAVE_DNOTIFY) || defined(HAVE_INOTIFY)
  TQPtrList<Entry> dList, cList;
#endif

  if (rescan_all)
  {
    // mark all as dirty
    it = m_mapEntries.begin();
    for( ; it != m_mapEntries.end(); ++it )
      (*it).dirty = true;
    rescan_all = false;
  }
  else
  {
    // progate dirty flag to dependant entries (e.g. file watches)
    it = m_mapEntries.begin();
    for( ; it != m_mapEntries.end(); ++it )
      if (((*it).m_mode == INotifyMode || (*it).m_mode == DNotifyMode) && (*it).dirty )
        (*it).propagate_dirty();
  }

  it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it ) {
    // we don't check invalid entries (i.e. remove delayed)
    if (!(*it).isValid()) continue;

    int ev = scanEntry( &(*it) );


#ifdef HAVE_INOTIFY
    if ((*it).m_mode == INotifyMode && ev == Created && (*it).wd == 0) {
      cList.append( &(*it) );
      if (! useINotify( &(*it) )) {
        useStat( &(*it) );
      }
    }
#endif

#ifdef HAVE_DNOTIFY
    if ((*it).m_mode == DNotifyMode) {
      if ((*it).isDir && (ev == Deleted)) {
	dList.append( &(*it) );

	// must close the FD.
	if ((*it).dn_fd) {
	  ::close((*it).dn_fd);
	  fd_Entry.remove((*it).dn_fd);
	  (*it).dn_fd = 0;
	}
      }

      else if ((*it).isDir && (ev == Created)) {
	// For created, but yet without DNOTIFYing ...
	if ( (*it).dn_fd == 0) {
	  cList.append( &(*it) );
	  if (! useDNotify( &(*it) )) {
	    // if DNotify setup fails...
	    useStat( &(*it) );
	  }
	}
      }
    }
#endif

    if ( ev != NoChange )
      emitEvent( &(*it), ev);
  }


#if defined(HAVE_DNOTIFY) || defined(HAVE_INOTIFY)
  // Scan parent of deleted directories for new creation
  Entry* e;
  for(e=dList.first();e;e=dList.next())
    addEntry(0, TQDir::cleanDirPath( e->path+"/.."), e, true);

  // Remove watch of parent of new created directories
  for(e=cList.first();e;e=cList.next())
    removeEntry(0, TQDir::cleanDirPath( e->path+"/.."), e);
#endif

  if ( timerRunning )
    timer->start(freq);

  TQTimer::singleShot(0, this, TQ_SLOT(slotRemoveDelayed()));
}

bool KSimpleDirWatchPrivate::isNoisyFile( const char * filename )
{
  // $HOME/.X.err grows with debug output, so don't notify change
  if ( *filename == '.') {
    if (strncmp(filename, ".X.err", 6) == 0) return true;
    if (strncmp(filename, ".xsession-errors", 16) == 0) return true;
    // fontconfig updates the cache on every KDE app start
    // (inclusive tdeio_thumbnail slaves)
    if (strncmp(filename, ".fonts.cache", 12) == 0) return true;
  }

  return false;
}

#ifdef HAVE_FAM
void KSimpleDirWatchPrivate::famEventReceived()
{
  static FAMEvent fe;

  delayRemove = true;

  while(use_fam && FAMPending(&fc)) {
    if (FAMNextEvent(&fc, &fe) == -1) {
      kdWarning(7001) << "FAM connection problem, switching to polling."
		      << endl;
      use_fam = false;
      delete sn; sn = 0;

      // Replace all FAMMode entries with DNotify/Stat
      EntryMap::Iterator it;
      it = m_mapEntries.begin();
      for( ; it != m_mapEntries.end(); ++it )
	if ((*it).m_mode == FAMMode && (*it).m_clients.count()>0) {
#ifdef HAVE_INOTIFY
	  if (useINotify( &(*it) )) continue;
#endif
#ifdef HAVE_DNOTIFY
	  if (useDNotify( &(*it) )) continue;
#endif
	  useStat( &(*it) );
	}
    }
    else
      checkFAMEvent(&fe);
  }

  TQTimer::singleShot(0, this, TQ_SLOT(slotRemoveDelayed()));
}

void KSimpleDirWatchPrivate::checkFAMEvent(FAMEvent* fe)
{
  // Don't be too verbose ;-)
  if ((fe->code == FAMExists) ||
      (fe->code == FAMEndExist) ||
      (fe->code == FAMAcknowledge)) return;

  if ( isNoisyFile( fe->filename ) )
    return;

  Entry* e = 0;
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it )
    if (FAMREQUEST_GETREQNUM(&( (*it).fr )) ==
       FAMREQUEST_GETREQNUM(&(fe->fr)) ) {
      e = &(*it);
      break;
    }

  // Entry* e = static_cast<Entry*>(fe->userdata);

#if 0 // #88538
  kdDebug(7001) << "Processing FAM event ("
		<< ((fe->code == FAMChanged) ? "FAMChanged" :
		    (fe->code == FAMDeleted) ? "FAMDeleted" :
		    (fe->code == FAMStartExecuting) ? "FAMStartExecuting" :
		    (fe->code == FAMStopExecuting) ? "FAMStopExecuting" :
		    (fe->code == FAMCreated) ? "FAMCreated" :
		    (fe->code == FAMMoved) ? "FAMMoved" :
		    (fe->code == FAMAcknowledge) ? "FAMAcknowledge" :
		    (fe->code == FAMExists) ? "FAMExists" :
		    (fe->code == FAMEndExist) ? "FAMEndExist" : "Unknown Code")
		<< ", " << fe->filename
		<< ", Req " << FAMREQUEST_GETREQNUM(&(fe->fr))
		<< ")" << endl;
#endif

  if (!e) {
    // this happens e.g. for FAMAcknowledge after deleting a dir...
    //    kdDebug(7001) << "No entry for FAM event ?!" << endl;
    return;
  }

  if (e->m_status == NonExistent) {
    kdDebug(7001) << "FAM event for nonExistent entry " << e->path << endl;
    return;
  }

  // Delayed handling. This rechecks changes with own stat calls.
  e->dirty = true;
  if (!rescan_timer.isActive())
    rescan_timer.start(m_PollInterval, true);

  // needed FAM control actions on FAM events
  if (e->isDir)
    switch (fe->code)
    {
      case FAMDeleted:
       // file absolute: watched dir
        if (!TQDir::isRelativePath(fe->filename))
        {
          // a watched directory was deleted

          e->m_status = NonExistent;
          FAMCancelMonitor(&fc, &(e->fr) ); // needed ?
          kdDebug(7001) << "Cancelled FAMReq "
                        << FAMREQUEST_GETREQNUM(&(e->fr))
                        << " for " << e->path << endl;
          // Scan parent for a new creation
          addEntry(0, TQDir::cleanDirPath( e->path+"/.."), e, true);
        }
        break;

      case FAMCreated: {
          // check for creation of a directory we have to watch
          Entry *sub_entry = e->m_entries.first();
          for(;sub_entry; sub_entry = e->m_entries.next())
            if (sub_entry->path == e->path + "/" + fe->filename) break;
          if (sub_entry && sub_entry->isDir) {
            TQString path = e->path;
            removeEntry(0,e->path,sub_entry); // <e> can be invalid here!!
            sub_entry->m_status = Normal;
            if (!useFAM(sub_entry))
            {
#ifdef HAVE_INOTIFY
              if (!useINotify(sub_entry ))
#endif
              {
                useStat(sub_entry);
              }
            }
          }
          break;
        }

      default:
        break;
    }
}
#else
void KSimpleDirWatchPrivate::famEventReceived() {}
#endif


void KSimpleDirWatchPrivate::statistics()
{
  EntryMap::Iterator it;

  kdDebug(7001) << "Entries watched:" << endl;
  if (m_mapEntries.count()==0) {
    kdDebug(7001) << "  None." << endl;
  }
  else {
    it = m_mapEntries.begin();
    for( ; it != m_mapEntries.end(); ++it ) {
      Entry* e = &(*it);
      kdDebug(7001) << "  " << e->path << " ("
		    << ((e->m_status==Normal)?"":"Nonexistent ")
		    << (e->isDir ? "Dir":"File") << ", using "
		    << ((e->m_mode == FAMMode) ? "FAM" :
                        (e->m_mode == INotifyMode) ? "INotify" :
			(e->m_mode == DNotifyMode) ? "DNotify" :
			(e->m_mode == StatMode) ? "Stat" : "Unknown Method")
		    << ")" << endl;

      Client* c = e->m_clients.first();
      for(;c; c = e->m_clients.next()) {
	TQString pending;
	if (c->watchingStopped) {
	  if (c->pending & Deleted) pending += "deleted ";
	  if (c->pending & Created) pending += "created ";
	  if (c->pending & Changed) pending += "changed ";
	  if (!pending.isEmpty()) pending = " (pending: " + pending + ")";
	  pending = ", stopped" + pending;
	}
	kdDebug(7001) << "    by " << c->instance->name()
		      << " (" << c->count << " times)"
		      << pending << endl;
      }
      if (e->m_entries.count()>0) {
	kdDebug(7001) << "    dependent entries:" << endl;
	Entry* d = e->m_entries.first();
	for(;d; d = e->m_entries.next()) {
          kdDebug(7001) << "      " << d << endl;
	  kdDebug(7001) << "      " << d->path << " (" << d << ") " << endl;
	}
      }
    }
  }
}


//
// Class KSimpleDirWatch
//

static KStaticDeleter<KSimpleDirWatch> sd_dw;
KSimpleDirWatch* KSimpleDirWatch::s_pSelf = 0L;

KSimpleDirWatch* KSimpleDirWatch::self()
{
  if ( !s_pSelf ) {
    sd_dw.setObject( s_pSelf, new KSimpleDirWatch );
  }

  return s_pSelf;
}

bool KSimpleDirWatch::exists()
{
  return s_pSelf != 0;
}

KSimpleDirWatch::KSimpleDirWatch (TQObject* parent, const char* name)
  : TQObject(parent,name)
{
  if (!name) {
    static int nameCounter = 0;

    nameCounter++;
    setName(TQString(TQString("KSimpleDirWatch-%1").arg(nameCounter)).ascii());
  }

  if (!dwp_self)
    dwp_self = new KSimpleDirWatchPrivate;
  d = dwp_self;
  d->ref();

  _isStopped = false;
}

KSimpleDirWatch::~KSimpleDirWatch()
{
  d->removeEntries(this);
  if ( d->deref() )
  {
    // delete it if it's the last one
    delete d;
    dwp_self = 0L;
  }
}


// TODO: add watchFiles/recursive support
void KSimpleDirWatch::addDir( const TQString& _path,
			bool watchFiles, bool recursive)
{
  if (watchFiles || recursive) {
    kdDebug(7001) << "addDir - recursive/watchFiles not supported yet in KDE 3.x" << endl;
  }
  if (d) d->addEntry(this, _path, 0, true);
}

void KSimpleDirWatch::addFile( const TQString& _path )
{
  if (d) d->addEntry(this, _path, 0, false);
}

TQDateTime KSimpleDirWatch::ctime( const TQString &_path )
{
  KSimpleDirWatchPrivate::Entry* e = d->entry(_path);

  if (!e)
    return TQDateTime();

  TQDateTime result;
  result.setTime_t(e->m_ctime);
  return result;
}

void KSimpleDirWatch::removeDir( const TQString& _path )
{
  if (d) d->removeEntry(this, _path, 0);
}

void KSimpleDirWatch::removeFile( const TQString& _path )
{
  if (d) d->removeEntry(this, _path, 0);
}

bool KSimpleDirWatch::stopDirScan( const TQString& _path )
{
  if (d) {
    KSimpleDirWatchPrivate::Entry *e = d->entry(_path);
    if (e && e->isDir) return d->stopEntryScan(this, e);
  }
  return false;
}

bool KSimpleDirWatch::restartDirScan( const TQString& _path )
{
  if (d) {
    KSimpleDirWatchPrivate::Entry *e = d->entry(_path);
    if (e && e->isDir)
      // restart without notifying pending events
      return d->restartEntryScan(this, e, false);
  }
  return false;
}

void KSimpleDirWatch::stopScan()
{
  if (d) d->stopScan(this);
  _isStopped = true;
}

void KSimpleDirWatch::startScan( bool notify, bool skippedToo )
{
  _isStopped = false;
  if (d) d->startScan(this, notify, skippedToo);
}


bool KSimpleDirWatch::contains( const TQString& _path ) const
{
  KSimpleDirWatchPrivate::Entry* e = d->entry(_path);
  if (!e)
     return false;

  KSimpleDirWatchPrivate::Client* c = e->m_clients.first();
  for(;c;c=e->m_clients.next())
    if (c->instance == this) return true;

  return false;
}

void KSimpleDirWatch::statistics()
{
  if (!dwp_self) {
    kdDebug(7001) << "KSimpleDirWatch not used" << endl;
    return;
  }
  dwp_self->statistics();
}


void KSimpleDirWatch::setCreated( const TQString & _file )
{
  kdDebug(7001) << name() << " emitting created " << _file << endl;
  emit created( _file );
}

void KSimpleDirWatch::setDirty( const TQString & _file )
{
  kdDebug(7001) << name() << " emitting dirty " << _file << endl;
  emit dirty( _file );
}

void KSimpleDirWatch::setDeleted( const TQString & _file )
{
  kdDebug(7001) << name() << " emitting deleted " << _file << endl;
  emit deleted( _file );
}

KSimpleDirWatch::Method KSimpleDirWatch::internalMethod()
{
#ifdef HAVE_FAM
  if (d->use_fam)
    return KSimpleDirWatch::FAM;
#endif
#ifdef HAVE_INOTIFY
  if (d->supports_inotify)
    return KSimpleDirWatch::INotify;
#endif
#ifdef HAVE_DNOTIFY
  if (d->supports_dnotify)
    return KSimpleDirWatch::DNotify;
#endif
  return KSimpleDirWatch::Stat;
}


#include "ksimpledirwatch.moc"
#include "ksimpledirwatch_p.moc"

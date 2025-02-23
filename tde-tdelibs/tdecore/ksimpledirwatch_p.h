/* Private Header for class of KSimpleDirWatchPrivate
 *
 * this separate header file is needed for MOC processing
 * because KSimpleDirWatchPrivate has signals and slots
 *
 * KSimpleDirWatch is a basic copy of KDirWatch
 * but with the TDEIO linking requirement removed
 */

#ifndef _KSIMPLEDIRWATCH_P_H
#define _KSIMPLEDIRWATCH_P_H

#ifdef HAVE_FAM
#include <fam.h>
#endif

#include <ctime>

#define invalid_ctime ((time_t)-1)

/* KSimpleDirWatchPrivate is a singleton and does the watching
 * for every KSimpleDirWatch instance in the application.
 */
class KSimpleDirWatchPrivate : public TQObject
{
  TQ_OBJECT
public:

  enum entryStatus { Normal = 0, NonExistent };
  enum entryMode { UnknownMode = 0, StatMode, DNotifyMode, INotifyMode, FAMMode };
  enum { NoChange=0, Changed=1, Created=2, Deleted=4 };

  struct Client {
    KSimpleDirWatch* instance;
    int count;
    // did the instance stop watching
    bool watchingStopped;
    // events blocked when stopped
    int pending;
  };

  class Entry
  {
  public:
    // the last observed modification time
    time_t m_ctime;
    // the last observed link count
    int m_nlink;
    entryStatus m_status;
    entryMode m_mode;
    bool isDir;
    // instances interested in events
    TQPtrList<Client> m_clients;
    // nonexistent entries of this directory
    TQPtrList<Entry> m_entries;
    TQString path;

    int msecLeft, freq;

    void addClient(KSimpleDirWatch*);
    void removeClient(KSimpleDirWatch*);
    int clients();
    bool isValid() { return m_clients.count() || m_entries.count(); }

    bool dirty;
    void propagate_dirty();

#ifdef HAVE_FAM
    FAMRequest fr;
#endif

#ifdef HAVE_DNOTIFY
    int dn_fd;
#endif
#ifdef HAVE_INOTIFY
    int wd;
#endif
  };

  typedef TQMap<TQString,Entry> EntryMap;

  KSimpleDirWatchPrivate();
  ~KSimpleDirWatchPrivate();

  void resetList (KSimpleDirWatch*,bool);
  void useFreq(Entry* e, int newFreq);
  void addEntry(KSimpleDirWatch*,const TQString&, Entry*, bool);
  void removeEntry(KSimpleDirWatch*,const TQString&, Entry*);
  bool stopEntryScan(KSimpleDirWatch*, Entry*);
  bool restartEntryScan(KSimpleDirWatch*, Entry*, bool );
  void stopScan(KSimpleDirWatch*);
  void startScan(KSimpleDirWatch*, bool, bool);

  void removeEntries(KSimpleDirWatch*);
  void statistics();

  Entry* entry(const TQString&);
  int scanEntry(Entry* e);
  void emitEvent(Entry* e, int event, const TQString &fileName = TQString::null);

  // Memory management - delete when last KSimpleDirWatch gets deleted
  void ref() { m_ref++; }
  bool deref() { return ( --m_ref == 0 ); }

 static bool isNoisyFile( const char *filename );

public slots:
  void slotRescan();
  void famEventReceived(); // for FAM
  void slotActivated(); // for DNOTIFY
  void slotRemoveDelayed();

public:
  TQTimer *timer;
  EntryMap m_mapEntries;

  int freq;
  int statEntries;
  int m_nfsPollInterval, m_PollInterval;
  int m_ref;
  bool useStat(Entry*);

  bool delayRemove;
  TQPtrList<Entry> removeList;

  bool rescan_all;
  TQTimer rescan_timer;

#ifdef HAVE_FAM
  TQSocketNotifier *sn;
  FAMConnection fc;
  bool use_fam;

  void checkFAMEvent(FAMEvent*);
  bool useFAM(Entry*);
#endif

#if defined(HAVE_DNOTIFY) || defined(HAVE_INOTIFY)
   TQSocketNotifier *mSn;
#endif

#ifdef HAVE_DNOTIFY
  bool supports_dnotify;
  int mPipe[2];
  TQIntDict<Entry> fd_Entry;

  static void dnotify_handler(int, siginfo_t *si, void *);
  static void dnotify_sigio_handler(int, siginfo_t *si, void *);
  bool useDNotify(Entry*);
#endif

#ifdef HAVE_INOTIFY
  bool supports_inotify;
  int m_inotify_fd;

  bool useINotify(Entry*);
#endif
};

#endif // KSIMPLEDIRWATCH_P_H


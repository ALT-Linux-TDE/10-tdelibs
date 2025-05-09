/*
 *  Copyright (C) 2003-2005 Thiago Macieira <thiago.macieira@kdemail.net>
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included 
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef KRESOLVER_P_H
#define KRESOLVER_P_H

#include <config.h>
#include <sys/types.h>

#include <tqstring.h>
#include <tqcstring.h>
#include <tqvaluelist.h>
#include <tqptrlist.h>
#include <tqptrqueue.h>
#include <tqthread.h>
#include <tqmutex.h>
#include <tqwaitcondition.h>
#include <tqsemaphore.h>
#include <tqevent.h>

#include "kresolver.h"

/* decide whether we need a mutex */
#if !defined(HAVE_GETPROTOBYNAME_R) || !defined(HAVE_GETSERVBYNAME_R) || !defined(HAVE_GETHOSTBYNAME_R) || !defined(HAVE_GETSERVBYPORT_R)
# define NEED_MUTEX
extern TQMutex getXXbyYYmutex;
#endif

/* some systems have the functions, but don't declare them */
#if defined(HAVE_GETSERVBYNAME_R) && !HAVE_DECL_GETSERVBYNAME_R
extern "C" {
  struct servent;
  extern int getservbyname_r(const char* serv, const char* proto,
			     struct servent* servbuf, 
			     char* buf, size_t buflen,
			     struct servent** result);
  extern int getservbyport_r(int port, const char* proto,
			     struct servent* servbuf,
			     char* buf, size_t buflen,
			     struct servent** result);

  struct protoent;
  extern int getprotobyname_r(const char* proto, struct protoent* protobuf,
			      char *buf, size_t buflen, 
			      struct protoent** result);
  extern int getprotobynumber_r(int proto, struct protoent* protobuf,
				char *buf, size_t buflen,
				struct protoent** result);
}
#endif

/* decide whether res_init is thread-safe or not */
#if defined(__GLIBC__)
# undef RES_INIT_THREADSAFE
#endif

namespace KNetwork
{
  // defined in network/qresolverworkerbase.h
  class KResolverWorkerBase;
  class KResolverWorkerFactoryBase;
  class KResolverPrivate;

  namespace Internal
  {
    class KResolverManager;
    class KResolverThread;
    struct RequestData;

    struct InputData
    {
      TQString node, service;
      TQCString protocolName;
      int flags;
      int familyMask;
      int socktype;
      int protocol;
    };
  }
    
  class KResolverPrivate
  {
  public:
    // parent class. Should never be changed!
    KResolver* parent;
    bool deleteWhenDone : 1;
    bool waiting : 1;

    // class status. Should not be changed by worker threads!
    volatile int status;
    volatile int errorcode, syserror;

    // input data. Should not be changed by worker threads!
    Internal::InputData input;

    // mutex
    TQMutex mutex;

    // output data
    KResolverResults results;

    KResolverPrivate(KResolver* _parent,
		     const TQString& _node = TQString::null, 
		     const TQString& _service = TQString::null)
      : parent(_parent), deleteWhenDone(false), waiting(false),
	status(0), errorcode(0), syserror(0)
    {
      input.node = _node;
      input.service = _service;
      input.flags = 0;
      input.familyMask = KResolver::AnyFamily;
      input.socktype = 0;
      input.protocol = 0;

      results.setAddress(_node, _service);
    }
  };

  namespace Internal
  {
    struct RequestData
    {
      // worker threads should not change values in the input data
      KNetwork::KResolverPrivate *obj;
      const KNetwork::Internal::InputData *input;
      KNetwork::KResolverWorkerBase *worker; // worker class
      RequestData *requestor; // class that requested us

      volatile int nRequests; // how many requests that we made we still have left
    };

    /*
     * @internal
     * This class is the resolver manager
     */
    class KResolverManager
    {
    public:
      enum EventTypes
	{ ResolutionCompleted = 1576 }; // arbitrary value;

      /*
       * This wait condition is used to notify wait states (KResolver::wait) that
       * the resolver manager has finished processing one or more objects. All
       * objects in wait state will be woken up and will check if they are done.
       * If they aren't, they will go back to sleeping.
       */
      TQWaitCondition notifyWaiters;

    private:
      /*
       * This variable is used to count the number of threads that are running
       */
      volatile unsigned short runningThreads;

      /*
       * This variable is used to count the number of threads that are currently
       * waiting for data.
       */
      unsigned short availableThreads;

      /*
       * This wait condition is used to notify worker threads that there is new
       * data available that has to be processed. All worker threads wait on this
       * waitcond for a limited amount of time.
       */
      TQWaitCondition feedWorkers;

      // this mutex protects the data in this object
      TQMutex mutex;

      // hold a list of all the current threads we have
      TQPtrList<KResolverThread> workers;

      // hold a list of all the new requests we have
      TQPtrList<RequestData> newRequests;

      // hold a list of all the requests in progress we have
      TQPtrList<RequestData> currentRequests;

      // hold a list of all the workers we have
      TQPtrList<KNetwork::KResolverWorkerFactoryBase> workerFactories;

      // private constructor
      KResolverManager();

    public:
      static KResolverManager* manager() TDE_NO_EXPORT;	// creates and returns the global manager

      // destructor
      ~KResolverManager();

      /*
       * Register this thread in the pool
       */
      void registerThread(KResolverThread* id);

      /*
       * Unregister this thread from the pool
       */
      void unregisterThread(KResolverThread* id);

      /*
       * Requests new data to work on.
       *
       * This function should only be called from a worker thread. This function
       * is thread-safe.
       *
       * If there is data to be worked on, this function will return it. If there is
       * none, this function will return a null pointer.
       */
      RequestData* requestData(KResolverThread* id, int maxWaitTime);

      /*
       * Releases the resources and returns the resolved data.
       *
       * This function should only be called from a worker thread. It is
       * thread-safe. It does not post the event to the manager.
       */
      void releaseData(KResolverThread *id, RequestData* data);

      /*
       * Registers a new worker class by way of its factory.
       *
       * This function is NOT thread-safe.
       */
      void registerNewWorker(KNetwork::KResolverWorkerFactoryBase *factory);

      /*
       * Enqueues new resolutions.
       */
      void enqueue(KNetwork::KResolver *obj, RequestData* requestor);

      /*
       * Dispatch a new request
       */
      void dispatch(RequestData* data);

      /*
       * Dequeues a resolution.
       */
      void dequeue(KNetwork::KResolver *obj);

      /*
       * Notifies the manager that the given resolution is about to
       * be deleted. This function should only be called by the
       * KResolver destructor.
       */
      void aboutToBeDeleted(KNetwork::KResolver *obj);

      /*
       * Notifies the manager that new events are ready.
       */
      void newEvent();

      /*
       * This function is called by the manager to receive a new event. It operates
       * on the @ref eventSemaphore semaphore, which means it will block till there
       * is at least one event to go.
       */
      void receiveEvent();

    private:
      /*
       * finds a suitable worker for this request
       */
      KNetwork::KResolverWorkerBase *findWorker(KNetwork::KResolverPrivate *p);

      /*
       * finds data for this request
       */
      RequestData* findData(KResolverThread*);

      /*
       * Handle completed requests.
       *
       * This function is called by releaseData above
       */
      void handleFinished();

      /*
       * Handle one completed request.
       *
       * This function is called by handleFinished above.
       */
      bool handleFinishedItem(RequestData* item);

      /*
       * Notifies the parent class that this request is done.
       *
       * This function deletes the request
       */
      void doNotifying(RequestData *p);

      /*
       * Dequeues and notifies an object that is in Queued state
       * Returns true if the object is no longer queued; false if it could not 
       * be dequeued (i.e., it's running)
       */
      bool dequeueNew(KNetwork::KResolver* obj);
    };

    /*
     * @internal
     * This class is a worker thread in the resolver system.
     * This class must be thread-safe.
     */
    class KResolverThread: public TQThread
    {
    private:
      // private constructor. Only the manager can create worker threads
      KResolverThread();
      RequestData* data;
  
    protected:
      virtual void run();		// here the thread starts

      friend class KNetwork::Internal::KResolverManager;
      friend class KNetwork::KResolverWorkerBase;

    public:
      bool checkResolver();	// see KResolverWorkerBase::checkResolver
      void acquireResolver();	// see KResolverWorkerBase::acquireResolver
      void releaseResolver();	// see KResolverWorkerBase::releaseResolver
    };

  } // namespace Internal

} // namespace KNetwork


#endif

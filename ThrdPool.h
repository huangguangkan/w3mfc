/*
Module : ThrdPool.h
Purpose: Interface for an C++ wrapper class for thread pools
Created: PJN / 15-04-2001
History: PJN / 21-07-2001 1. Made destructors of the two classes virtual as both can be used as base classes
         PJN / 25-07-2001 1. Code now uses a Win9x compatible IO completion port if we fail to use the build 
                          in OS one. This IOCP is implemented in the class "CIOCP9x" in IOCP9x.h/cpp.
         PJN / 07-08-2001 1. Added a WaitForThreadsInitInstance. This allows code in the main thread to synchronise
                          with the all the InitInstance calls in the thread pool
         PJN / 23-08-2001 1. Removed unnecessary "#pragma message"
         PJN / 15-04-2002 1. Class now uses new "CDirectedIOCPRequest" class instead of an SDK IOCP or dummy one
                          for Win9x. This allows the thread pool to now support recycling of threads after a 
                          specified interval in the thread pool.
                          2. Tidied up the exposed API to not reflect the exact attributes of IO completion
                          ports and instead be more object oriented.
                          3. Change some of the class API parameters to provide for more flexibility
         PJN / 29-04-2002 1. Fixed a bug in the CDirectedIOCP class which was causing multiple threads in the thread 
                          pool to be released into the depths of the CDirectedIOCP::GetRequest code.
                          2. Fixed a bug which was causing m_Threads array to be accessed from multiple threads
                          when thread recycling was enabled.
         PJN / 16-05-2002 1. Now provides an option to have a Q size different than the thread pool size.
                          2. Also provides a method to post to the IOCP without first checking the limit
         PJN / 18-08-2002 1. Renamed CDirectedIOCP to CDirectedThreadPoolQueue
                          2. Renamed CDirectedIOCPRequest to CThreadPoolRequest
                          3. Now user can decide which queing mechanism to use thro the Start method. 2 pre built
                          classes are provided, namely CDirectedThreadPoolQueue and CIOCPThreadPoolQueue
                          4. Provision of virtual GetNonDirectedRequestIndexToRemove and GetDirectedRequestIndexToRemove
                          methods in the CDirectedThreadPoolQueue class. This allows derived classes to implement 
                          their own schemes as to which requests to prioritize on the thread pool queue
         PJN / 20-08-2002 1. Provided virtual destructors for all the classes which constitute the Thread pool framework
                          2. Removed forward reference of the now defunct class CDirectedIOCP
                          3. Removed unreferenced parameters level 4 warnings in the CThreadPool class declaration
                          4. Fixed usage of 2 int variables in CDirectedThreadPoolQueue::GetNonDirectedRequestIndexToRemove
                          and GetDirectedRequestIndexToRemove which were incorrectly declared as BOOL's. Thanks to 
                          Serhiy Pavlov for these updates.
         PJN / 04-10-2002 1. CThreadPoolClient::Run now has a return value to decide whether or not the worker thread
                          should continue to service requests upon return from handling the current request
         PJN / 08-10-2002 1. Shutting down of the thread pool now uses directed requests instead of undirected requests. 
                          This should improve the speed of shutdown of the thread pool when it contains a lot of requests
                          on the queue.
                          2. Defined enums for m_dwID member of the CThreadPoolRequest class
         PJN / 12-10-2002 1. Removed and replaced the PostRequestWithoutLimitCheck method with the standard PostRequest
                          method. This avoids the problem with TRACE messages appearing along the lines
                          "CDirectedThreadPoolQueue::GetRequest, Failed to release a semaphore". Thanks to Frank Schmidt
                          for reporting this problem.
                          2. Fixed a minor flow in "CDirectedThreadPoolQueue::Create()" where I forgot to call Close() 
                          when the creation of "m_hGetRequestSemaphore" fails. Again thanks to Frank Schmidt for spotting 
                          this.
         PJN / 14-10-2002 1. Reintroduced the function CThreadPoolQueue::PostRequestWithoutLimitCheck as some users of
                          the thread pool class had legit reasons to use this function.
                          2. Changed a VERIFY call into an ASSERT in CThreadPoolServer::RecycleThread
         PJN / 17-10-2002 1. Fixed a problem where CThreadPoolServer::Stop() would hang if an I/O completion port based
                          thread pool is being used. Thanks to Frank Schmidt for spotting this problem.
                          2. Made the thread pool class Win95 compliant by dynamically linking to the waitable timer 
                          API's. Even though the code did gracefully degrade if the waitable timer functions failed, the
                          fact that they did not use GetProcAddress to link to the functions meant that any app / dll
                          which included the thread pool class would fail to load on Win95. Thanks to Frank Schmidt 
                          for this update.
         PJN / 07-11-2002 1. Minor update to the thread pool class to provide a virtual function which gets call when 
                          the m_bRequestToStop is being set. 
         PJN / 13-01-2004 1. Made the m_bRequestToStop member variable "volatile" as it can be modified from
                          multiple threads while possible been read in a loop in another thread. Thanks to Dan Baker
                          for reporting this issue.
         PJN / 25-10-2004 1. Updated to compile cleanly when Detect 64 bit issues and Force conformance in for loop
                          options are enabled in Visual Studio .Net
         PJN / 06-09-2005 1. Reworked the CThreadPoolClient::_Run method so that it can now be customized at runtime. 
                          There is now a non static version which is also called _Run and there is also a virtual Main 
                          function which implements the main loop of the thread pool.
                          2. A CEvent namely "m_RequestToStop" is now also set when the thread pool is being stopped.
         PJN / 13-04-2006 1. Updated copyright details.
                          2. Renamed CThreadPoolClient::m_RequestToStop to m_evtRequestToStop.
         PJN / 31-05-2006 1. Removed the virtual destructor for CThreadPoolRequest. Please note that this class is not
                          meant to be derived from. For further information on its usage please read the comments in 
                          Thrdpool.h
                          2. Reworked thread recycling logic in CThreadPoolServer::Monitor to use a much simpler
                          WaitForSingleObject call.
                          3. Optimized CThreadPoolServer constructor code
                          4. Optimized CThreadPoolRequest constructor code
         PJN / 08-08-2006 1. Provision of a CThreadPoolClient::m_bPumpMessageQueue boolean. This allows you to specify 
                          that internally the thread pool client will use MsgWaitForMultipleObjects to wait for a request
                          to appear on the thread pool queue or a windows message to appear on the thread's system queue. If
                          a windows message is detected, then the thread pool will internally pump the message queue until 
                          no more messages are present. If CThreadPoolClient::m_bPumpMessageQueue is left at the default of 
                          FALSE, then you will get the old thread pool behaviour which is to directly call into 
                          CThreadPoolQueue::GetRequest which will block until a request is available. This new addition prevents 
                          problems if your thread pool initializes and uses COM as an STA. In this case the thread is required 
                          to pump messages since COM uses the message queue for synchronisation when it is initialized as an STA.
                          If you do not pump the message queue when you are using COM as an STA, you will notice unusual behaviour 
                          such as the Windows shell being unresponsive when ShellExecute calls are running. This is because 
                          internally ShellExecute uses DDE which itself broadcasts window messages to all top level windows. When 
                          you call CoInitialize to use an STA, internally a top level window is created, but because there is no 
                          code servicing this windows message queue, the shell's ShellExecute calls will seem to hang for 30 
                          seconds. Boy did it take me some work to find the root of this problem. You have been warned!!!
         PJN / 19-11-2007 1. THREADPOOL_SHUTDOWN_REQUEST and THREADPOOL_USER_DEFINED_REQUEST constants have now been made enums of
                          the CThreadPoolRequest class
         PJN / 31-05-2008 1. Updated copyright details
                          2. Code now compiles cleanly using Code Analysis (/analyze)
         PJN / 16-03-2014 1. Updated copyright details
         PJN / 29-03-2014 1. Reworked the CThreadPoolRequest, CThreadPoolQueue, CThreadPool, CIOCPThreadPoolQueue and CDirectedThreadPoolQueue 
                          classes to compile without MFC. To achieve this the internal dependencies on MFC have been removed resulting in some 
                          breaking changes. For example the CThreadPool class is now templatised on a TCLIENT and TQUEUE class instead of via
                          CRuntimeClass values being passed as parameters for these to the CThreadPool::Start method. In addition the threads 
                          in the thread pool are creating using the CRT function _beginthreadex instead of via AfxBeginThread. Also all the 
                          dependencies on the MFC multithreading classes such as CCriticalSection and CEvent have been replaced by ATL or raw 
                          Win32 functionality. You can use the W3MFC sample web server source code to see the required client side changes
                          to use the updated thread pool classes.

                          Comments on correct use of CThreadPoolRequest:

                          Please note that some values sent down in CThreadPoolRequest::m_dwID, specifically THREADPOOL_SHUTDOWN_REQUEST are 
                          reserved for use by the thread pool framework itself. If you want to place you own value / enum here, then offset it 
                          from THREADPOOL_USER_DEFINED_REQUEST. i.e. use code like the following:
                          
                          CThreadPoolRequest request;
                          request.m_dwID = THREADPOOL_USER_DEFINED_REQUEST + dwMyValue;
                          if (!ThreadPool.GetQueue()->PostRequest(request))
                          ...
                          
                          Then in your thread pool client class you would use something like the following code:
                          
                          CMyThreadPoolClient::Run(const CThreadPoolRequest& request)
                          {
                            DWORD dwMyValue = request.m_dwID - THREADPOOL_USER_DEFINES_REQUEST;
                            ...
                           
                          If I update the REQUEST enum values in the future, I will ensure that any built-in values are less than 
                          THREADPOOL_USER_DEFINED_REQUEST. This will ensure that client code will continue to operate as expected. Also note 
                          that if you want to send anything more than a simple integer to the thread pool for each request, then you can use 
                          the m_pData  member variable. The canonical use of this would be to store the address of a heap allocated struct in 
                          it, which you can then easily dot into in your thread pools Run method, but don't forget to delete it before you 
                          return from Run. Here's an example:
                          
                           struct CMyStruct
                           {
                             int nSomeValue;
                             DWORD nSomeOtherValue;
                             CString sStringValue;
                             ...
                           }; 
                          
                           CMyStruct* pStruct = new CMyStruct;
                           pStruct->nSomeValue = something;
                           pStruct->nSomeOtherValue = somethingElse;
                           pStruct->sStringValue = someString;
                           CThreadPoolRequest request;
                           request.m_pData = pStruct;
                           request.m_dwID = THREADPOOL_USER_DEFINED_REQUEST;
                           if (!ThreadPool.GetQueue()->PostRequest(request))
                             delete pStruct;
                           ...

                          Note the code above which deletes the struct if it is not delivered to the thread pool. Then in our thread pool 
                          client code we have the following:
                          
                          CMyThreadPoolClient::Run(const CThreadPoolRequest& request)
                          {
                            CMyStruct* pStruct = (CMyStruct*) request->m_pData;
                            ...
                            use pStruct->nSomeValue, pStruct->nSomeOtherValue or pStruct->sStringValue
                            ...
                            //Finally now that we are finished with it, tidy up the heap memory in the request
                            delete pStruct;
                            ...
                            return ...
                          }

                          Also note that CThreadPoolRequest is not meant to be derived from.
         PJN / 17-07-2016 1. Added SAL annotations to the CThreadPoolRequest, CThreadPoolQueue, CThreadPoolClient & CThreadPoolServer 
                          classes.
         PJN / 11-08-2016 1. Fixed two handle leaks in the CThreadPoolClient destructor. Thanks to Robin Hilliard for reporting this 
                          bug.

Copyright (c) 2002 - 2017 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////////// Macros / Defines //////////////////////////////

#pragma once

#ifndef __THRDPOOL_H__
#define __THRDPOOL_H__

#ifndef _In_opt_
#define _In_opt_
#endif //#ifndef _In_opt_


/////////////////////////////// Includes //////////////////////////////////////

#ifndef _VECTOR_
#pragma message("To avoid this message, please put vector in your pre compiled header (normally stdafx.h)")
#include <vector>
#endif //#ifndef _VECTOR_

#ifndef __ATLBASE_H__
#pragma message("To avoid this message, please put atlbase.h in your pre compiled header (normally stdafx.h)")
#include <atlbase.h>
#endif //#ifndef __ATLBASE_H__

#ifndef __ATLCORE_H__
#pragma message("To avoid this message, please put atlcore.h in your pre compiled header (normally stdafx.h)")
#include <atlcore.h>
#endif //#ifndef __ATLCORE_H__


/////////////////////////////// Classes ///////////////////////////////////////

//Represents an instance of a request as stored on the thread pool queue
class CThreadPoolRequest
{
public:
//Enums
  enum ThreadPoolRequestType
  {
    THREADPOOL_SHUTDOWN_REQUEST = 0,
    THREADPOOL_USER_DEFINED_REQUEST = 1
  };

//Constructors / Destructors
  CThreadPoolRequest() : m_bDirectedRequest(FALSE),
                         m_nDirectedRequestIndex(-1),
                         m_dwID(THREADPOOL_USER_DEFINED_REQUEST),
                         m_pData(nullptr)
  {
  }

  CThreadPoolRequest(_In_ const CThreadPoolRequest& request)
  {
    *this = request;
  }

//methods
  CThreadPoolRequest& operator=(_In_ const CThreadPoolRequest& request)
  {
    m_bDirectedRequest = request.m_bDirectedRequest;
    m_nDirectedRequestIndex = request.m_nDirectedRequestIndex;
    m_dwID = request.m_dwID;
    m_pData = request.m_pData;
  
    return *this;
  }

//Member variables
  BOOL    m_bDirectedRequest;      //If this to be a directed request i.e. targetted at a specific thread
  INT_PTR m_nDirectedRequestIndex; //The index of the associated thread if this is a directed request
  void*   m_pData;                 //Any opaque data you want to associate with the requested
  DWORD   m_dwID;                  //Any DWORD item data you want to associate with the request
};

//Base class which implements a queue for the thread pool
class CThreadPoolQueue
{
public:
//Constructors / Destructors
  CThreadPoolQueue()
  {
  }

  virtual ~CThreadPoolQueue() 
  {
  }  

//Methods
  virtual BOOL Create(DWORD /*dwMaxQSize*/) 
  { 
    return FALSE; 
  }
  virtual BOOL PostRequest(_In_ const CThreadPoolRequest& /*request*/, _In_ DWORD /*dwMilliseconds*/ = INFINITE, _In_ BOOL /*bLock*/ = TRUE)
  { 
    return FALSE; 
  }
  virtual BOOL PostRequestWithoutLimitCheck(_In_ const CThreadPoolRequest& /*request*/, _In_ BOOL /*bLock*/ = TRUE)
  { 
    return FALSE; 
  }
  virtual BOOL GetRequest(_In_ CThreadPoolRequest& /*request*/, int /*nThreadIndexForDirectedRequest*/, _In_ DWORD /*dwMilliseconds*/, _In_ BOOL /*bLock*/, _In_ BOOL /*bPumpMessage*/)
  { 
    return FALSE; 
  }
  virtual BOOL IsCreated() const 
  { 
    return FALSE; 
  }
  virtual BOOL SupportsDirectedRequests() const 
  { 
    return FALSE; 
  }
};

//Class which you derive from to implement your own thread pool behaviour
class CThreadPoolClient
{
public:
//Constructors / Destructors
  CThreadPoolClient() : m_hRequestToStop(nullptr),
                        m_hInitCompleted(nullptr),
                        m_hWorkerThread(nullptr),
                        m_pQueue(nullptr),
                        m_lInitOK(FALSE),
                        m_lRequestToStop(FALSE),
                        m_nThreadPriority(THREAD_PRIORITY_NORMAL),
                        m_nStackSize(0),
                        m_nThreadIndex(-1),
                        m_bPumpMessageQueue(FALSE)
  {
  }

  virtual ~CThreadPoolClient()
  {
    ATLASSERT(m_hWorkerThread == nullptr); //Thread should be destroyed by now

    //Free up the two event handles we have been using
    if (m_hRequestToStop != nullptr)
    {
      CloseHandle(m_hRequestToStop);
      m_hRequestToStop = nullptr;
    }
    if (m_hInitCompleted != nullptr)
    {
      CloseHandle(m_hInitCompleted);
      m_hInitCompleted = nullptr;
    }
  }

//Virtual methods
  virtual void SetRequestToStop() //Called by the manager when CThreadPoolServer::Stop is called. You can use this or 
                                  //the member variables m_lRequestToStop or m_hRequestToStop to effect a premature exit 
                                  //from your Run method, rather than waiting for a stop request to arrive via the 
                                  //thread pool queue
  {
    //Signal the boolean and the event
    InterlockedExchange(&m_lRequestToStop, TRUE);

    ATLASSUME(m_hRequestToStop != nullptr);
    SetEvent(m_hRequestToStop);
  }

  virtual void ResetRequestToStop()
  {
    //Signal the boolean and the event
    InterlockedExchange(&m_lRequestToStop, FALSE);

    ATLASSUME(m_hRequestToStop != nullptr);
    ResetEvent(m_hRequestToStop);
  }

  virtual BOOL Create() //Called in the context of the main thread to allow error handling of construction events
  {
    //Create the two event objects we will need

    //Create the request to stop event (of the manual-reset variety)
    ATLASSERT(m_hRequestToStop == nullptr);
    m_hRequestToStop = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (m_hRequestToStop == nullptr)
    {
      ATLTRACE(_T("CThreadPoolClient::Create, Failed to create the request to stop event, Error:%u\n"), GetLastError());
      return FALSE;
    }

    //Create the init completed event (of the auto-reset variety)
    ATLASSERT(m_hInitCompleted == nullptr);
    m_hInitCompleted = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_hInitCompleted == nullptr)
    {
      ATLTRACE(_T("CThreadPoolClient::Create, Failed to create the init completed event, Error:%u\n"), GetLastError());
      return FALSE;
    }

    return TRUE;
  }

//Standard methods
  HANDLE GetThreadHandle()
  {
    return m_hWorkerThread;
  }

  void SetThreadHandle(_In_opt_ HANDLE hWorkerThread)
  {
    m_hWorkerThread = hWorkerThread;
  }

  CThreadPoolQueue* GetThreadPoolQueue() const
  {
    return m_pQueue;
  }

  void SetQueue(_In_opt_ CThreadPoolQueue* pQueue)
  {
    m_pQueue = pQueue;
  }

  int GetThreadPriority() const
  {
    return m_nThreadPriority;
  }

  void SetThreadPriority(_In_ int nPriority)
  {
    m_nThreadPriority = nPriority;
  }

  int GetStackSize() const
  {
    return m_nStackSize;
  }

  void SetStackSize(_In_ int nStackSize)
  {
    m_nStackSize = nStackSize;
  }

  int GetThreadIndex() const
  {
    return m_nThreadIndex;
  }

  void SetThreadIndex(_In_ int nIndex)
  {
    m_nThreadIndex = nIndex;
  }

  LONG GetRequestToStop() const
  {
    return m_lRequestToStop;
  }

  HANDLE GetInitCompletedHandle() const
  {
    return m_hInitCompleted;
  }

  LONG GetInitOK() const
  {
    return m_lInitOK;
  }

//The worker thread
  static unsigned __stdcall _Run(_In_ void* pParam) //Standard SDK style thread function
  {
    //Validate our parameters
    ATLASSUME(pParam != nullptr);

    //Get back the "this" pointer
    CThreadPoolClient* pClient = static_cast<CThreadPoolClient*>(pParam);
    ATLASSUME(pClient != nullptr);

    //Delegate the work to the non static version of _Run
    return pClient->_Run();
  }

  virtual unsigned _Run() //Non static thread function called by the static version
  {
    //Call InitInstance to allow thread pool initialization customisation,
    //also store its return value in m_bInitOK
    InterlockedExchange(&m_lInitOK, InitInstance());

    //Signal the event to let the server know that we have completed InitInstance
    ATLASSERT(m_hInitCompleted != nullptr);
    SetEvent(m_hInitCompleted);

    //Should we run the "main" loop
    if (m_lInitOK)
      Main(); //Call the Main function which implements the "main" loop

    //Let the ExitInstance method decide the return code from the thread
    return ExitInstance();
  }

protected:
//Virtual methods
  virtual BOOL InitInstance() //Similiar to the MFC function of the same name 
  {
    return TRUE; //Default behaviour is to allow "Run" to be called
  }

  virtual int ExitInstance() //Similiar to the MFC function of the same name
  {
    return 1; //By default, return 1 as the thread exit code
  }

  virtual BOOL Run(_In_ const CThreadPoolRequest& /*request*/) //The function which is called to handle client requests
  {
    ATLASSERT(FALSE); //You need to override CThreadPoolClient::Run in your derived class
    return FALSE;
  }

  virtual void Main() //Implements the "main" loop
  {
    //Validate our parameters
    ATLASSUME(m_pQueue != nullptr);

    //Get the queued request posted to us from the queue class
    CThreadPoolRequest request;
    BOOL bContinue = TRUE;
    while (bContinue && !m_lRequestToStop)
    {
      if (m_pQueue->GetRequest(request, m_nThreadIndex, INFINITE, TRUE, m_bPumpMessageQueue))
      {
        if (request.m_dwID == CThreadPoolRequest::THREADPOOL_SHUTDOWN_REQUEST)
          bContinue = FALSE; //break out of the loop, since a shutdown request was sent to us
        else
          bContinue = Run(request); //Allow the virtual function to handle the client request
      }
    }
  }

//Member variables
  HANDLE             m_hWorkerThread;     //The actual worker thread handle
  CThreadPoolQueue*  m_pQueue;            //Reference to this objects manager 
  volatile LONG      m_lRequestToStop;    //Set to TRUE by the manager when CThreadPoolServer::Stop is called. You can use this inside of
                                          //your derived Run method to effect a premature exit from it, rather than waiting for a Stop
                                          //request to arrive via the thread pool queue
  HANDLE             m_hRequestToStop;    //synchronizable version of "m_bRequestToStop"
  HANDLE             m_hInitCompleted;    //Signalled when InitInstance has completed, Use by CThreadPoolServer::WaitForThreadsInitInstance 
  LONG               m_lInitOK;           //Has InitInstance completed successfully
  int                m_nThreadPriority;   //The thread priority which this thread started with
  UINT               m_nStackSize;        //The size of the stack this thread started with
  int                m_nThreadIndex;      //The thread index of this thread in the thread pool
  BOOL               m_bPumpMessageQueue; //Should we pump the message queue
};

//The class which manages the thread pool
template <class TCLIENT, class TQUEUE>
class CThreadPoolServer
{
public:
//Constructors / Destructors
  CThreadPoolServer() : m_bMaxLifetime(FALSE),
                        m_dwMaxLifetime(0),
                        m_hLifetimeMonitorThread(nullptr),
                        m_nLifetimeThreadIndex(0),
                        m_hLifeTimeThreadRequestToStop(nullptr),
                        m_pQueue(nullptr)
  {
  }

  virtual ~CThreadPoolServer()
  {
    Stop();
  }

//Methods  
  virtual BOOL Start(_In_ int nPoolSize, _In_ int nQueueSize, _In_ BOOL bSuspended = FALSE, _In_ int nPriority = THREAD_PRIORITY_NORMAL, _In_ UINT nStackSize = 0) //Starts up the thread pool
  {
    //Validate our parameters
    ATLASSERT(nPoolSize != 0); //You must have at least 1 thread in the pool

    //Stop if currently started
    Stop();

    //Create the queue
    ATLASSERT(m_pQueue == nullptr);
    m_pQueue = new TQUEUE;

    //Serialize access to the threads array
    ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csThreads, TRUE);

    ATLASSERT(m_Threads.size() == 0); //Should be empty by now

    //Create the thread pool queue
    if (!m_pQueue->Create(nQueueSize))
    {
      ATLTRACE(_T("CThreadPoolServer::Start, Failed to create the queue, Error:%u\n"), GetLastError());
      return FALSE;
    }

    //What will be the return value from this method (assume the best)
    BOOL bSuccess = TRUE;

    //Setup the worker threads in the thread pool
    m_Threads.reserve(nPoolSize);
    for (int i=0; i<nPoolSize && bSuccess; i++)
    {
      //Create the thread pool client object
      CThreadPoolClient* pClient = new TCLIENT;

      //Setup its member variables
      pClient->SetQueue(m_pQueue);
      pClient->SetThreadPriority(nPriority);
      pClient->SetStackSize(nStackSize);
      pClient->SetThreadIndex(i);

      //Give the client object a chance to initialise in this thread
      if (!pClient->Create())
      {
        ATLTRACE(_T("CThreadPoolServer::Start, Failed in call to CThreadPoolClient::Create for thread at index %d\n"), i);
        delete pClient;
        bSuccess = FALSE;
      }
      else
      {
        //Delegate the creation of the thread to the helper method
        bSuccess = CreateWorkerThread(pClient, i, nStackSize, nPriority);
        if (!bSuccess)
        {
          delete pClient;
        }
        else
        {
          //Add the client to the collection
          m_Threads.push_back(pClient);
        }
      }
    }

    if (bSuccess)
    {
      //Now that everything is setup we can resume the threads in the thread pool (if required)
      if (!bSuspended)
      {
        for (std::vector<CThreadPoolClient*>::size_type i=0; i<m_Threads.size(); i++)
        {
          CThreadPoolClient* pClient = m_Threads[i];
          ATLASSUME(pClient != nullptr);
          HANDLE hWorkerThread = pClient->GetThreadHandle();
          ATLASSUME(hWorkerThread != nullptr);
          if (ResumeThread(hWorkerThread) == static_cast<DWORD>(-1))
          {
            ATLTRACE(_T("CThreadPoolServer::Start, Failed to resume thread at index %d, Error:%u\n"), static_cast<int>(i), GetLastError());
            bSuccess = FALSE;
          }
        }
      }
    }

    //Tidy up if anything failed  
    if (!bSuccess)
      Stop();

    return bSuccess;
  }

  virtual void Stop() //Closes down the thread pool
  {
    //Serialize access to the threads array
    ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csThreads, TRUE);

    std::vector<CThreadPoolClient*>::size_type nThreads = m_Threads.size();
    if (nThreads)
    {
      ATLASSUME(m_pQueue != nullptr);

      //Set the m_bRequestToStop in each thread to ask them to exit if they are
      //currently processing a request in CThreadPoolClient::Run
      for (std::vector<CThreadPoolClient*>::size_type i=0; i<nThreads; i++)
      {
        CThreadPoolClient* pClient = m_Threads[i];
        pClient->SetRequestToStop();
      }

      //Now post enough requests to get each thread in the thread pool to stop via
      //a "special" request with an ID of THREADPOOL_SHUTDOWN_REQUEST
      for (std::vector<CThreadPoolClient*>::size_type i=0; i<nThreads; i++)
      {
        CThreadPoolRequest killRequest;
        killRequest.m_dwID = CThreadPoolRequest::THREADPOOL_SHUTDOWN_REQUEST;

        //Only use directed requests to shut down the thread pool if the pool supports
        //directed requests
        if (m_pQueue->SupportsDirectedRequests())
        {
          killRequest.m_bDirectedRequest = TRUE;
          killRequest.m_nDirectedRequestIndex = i;
        }
        ATLASSERT(m_pQueue != nullptr);
        m_pQueue->PostRequest(killRequest);
      }

      //Wait for all the threads to exit in the thread pool
      BOOL bMoreThreads = TRUE;
      std::vector<CThreadPoolClient*>::size_type nCurrentThreadIndex = 0;
      while (bMoreThreads)
      {
        //Wait for as many threads at once as possible
        std::vector<CThreadPoolClient*>::size_type nCurrentThreadsToStop = min(MAXIMUM_WAIT_OBJECTS, nThreads - nCurrentThreadIndex);

        //Setup the array of threads to wait on to exit
        HANDLE hThreads[MAXIMUM_WAIT_OBJECTS];
        memset(hThreads, 0, sizeof(hThreads));
        for (std::vector<CThreadPoolClient*>::size_type j=0; j<nCurrentThreadsToStop; j++)
        {
          CThreadPoolClient* pClient = m_Threads[j + nCurrentThreadIndex];
          ATLASSUME(pClient != nullptr);
          HANDLE hWorkerThread = pClient->GetThreadHandle();
          ATLASSUME(hWorkerThread != nullptr);
          hThreads[j] = hWorkerThread;
        }

        //Wait for the threads to exit
        WaitForMultipleObjects(static_cast<DWORD>(nCurrentThreadsToStop), hThreads, TRUE, INFINITE);

        //Get ready for the next time around
        nCurrentThreadIndex += nCurrentThreadsToStop;
        bMoreThreads = (nCurrentThreadIndex < nThreads);
      }

      //Now free up all the memory associated with each thread
      for (std::vector<CThreadPoolClient*>::size_type i=0; i<nThreads; i++)
      {
        CThreadPoolClient* pClient = m_Threads[i];
        ATLASSUME(pClient != nullptr);
        HANDLE hWorkerThread = pClient->GetThreadHandle();
        CloseHandle(hWorkerThread);
        pClient->SetThreadHandle(nullptr);
        delete pClient;
      }
      m_Threads.clear();
    }

    //Close our queue object
    if (m_pQueue != nullptr)
    {
        delete m_pQueue;
        m_pQueue = nullptr;
    }

    //Bring down the monitoring thread if required
    SetMaxThreadClientLifetime(FALSE, 0);  
  }

  virtual CThreadPoolClient* GetAtClient(_In_ std::vector<CThreadPoolClient*>::size_type nIndex)
  {
    //It is assumed client code which calls this method "GetAtClient" has done its own serialization of "m_Threads"
    return m_Threads[nIndex];
  }

  virtual BOOL WaitForThreadsInitInstance()
  {
    //What will be the return value from this method (assume the best)
    BOOL bSuccess = TRUE;

    //Serialize access to the threads array
    ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csThreads, TRUE);

    std::vector<CThreadPoolClient*>::size_type nThreads = m_Threads.size();
    if (nThreads)
    {
      BOOL bMoreThreads = TRUE;
      std::vector<CThreadPoolClient*>::size_type nCurrentThreadIndex = 0;
      while (bMoreThreads)
      {
        //Wait for as many threads at once as possible
        std::vector<CThreadPoolClient*>::size_type nEventsToWaitOn = min(MAXIMUM_WAIT_OBJECTS, nThreads - nCurrentThreadIndex);

        //Setup the array of threads to wait on to exit
        HANDLE hEvents[MAXIMUM_WAIT_OBJECTS];
        memset(hEvents, 0, sizeof(hEvents));
        for (std::vector<CThreadPoolClient*>::size_type i=0; i<nEventsToWaitOn; i++)
        {
          CThreadPoolClient* pClient = m_Threads[i + nCurrentThreadIndex];
          ATLASSUME(pClient != nullptr);
          hEvents[i] = pClient->GetInitCompletedHandle();
        }

        //Wait for the threads to to complete their InitInstance code
        WaitForMultipleObjects(static_cast<DWORD>(nEventsToWaitOn), hEvents, TRUE, INFINITE);

        //Update the "bSuccess" variable which is the logical "And" of all the InitInstances
        for (std::vector<CThreadPoolClient*>::size_type i=0; i<nEventsToWaitOn && bSuccess; i++)
        {
          CThreadPoolClient* pClient = m_Threads[i + nCurrentThreadIndex];
          ATLASSUME(pClient != nullptr);
          bSuccess &= pClient->GetInitOK();
        }

        //Get ready for the next time around
        nCurrentThreadIndex += nEventsToWaitOn;
        bMoreThreads = (nCurrentThreadIndex < nThreads);
      }
    }
    else
      bSuccess = FALSE;

    return bSuccess;
  }

  BOOL SetMaxThreadClientLifetime(_In_ BOOL bEnableThreadLifetime, _In_ DWORD dwMinutes)
  {
    //Kill the monitoring thread if currently active
    if (m_hLifetimeMonitorThread != nullptr)
    {
      ATLASSUME(m_hLifeTimeThreadRequestToStop != nullptr);
      if (!SetEvent(m_hLifeTimeThreadRequestToStop))
      {
        ATLTRACE(_T("CThreadPoolServer::SetMaxThreadClientLifetime, Failed to set request to stop event, Error:%u\n"), GetLastError());
      }
      WaitForSingleObject(m_hLifetimeMonitorThread, INFINITE);
      CloseHandle(m_hLifetimeMonitorThread);
      m_hLifetimeMonitorThread = nullptr;
    }

    //Destroy the request to stop event if valid
    if (m_hLifeTimeThreadRequestToStop != nullptr)
    {
      CloseHandle(m_hLifeTimeThreadRequestToStop);
      m_hLifeTimeThreadRequestToStop = nullptr;
    }

    //Hive away the member variables
    m_bMaxLifetime = bEnableThreadLifetime;
    m_dwMaxLifetime = dwMinutes;

    //Recreate the monitoring thread if required
    if (m_bMaxLifetime)
    {
      ATLASSUME(m_pQueue != nullptr);

      if (!m_pQueue->SupportsDirectedRequests())
      {
        ATLTRACE(_T("CThreadPoolServer::SetMaxThreadClientLifetime, Recyclying of threads in the thread pool is not supported because the queue does not support directed requests\n"));
        SetLastError(ERROR_NOT_SUPPORTED);
        m_bMaxLifetime = FALSE;
        m_dwMaxLifetime = 0;
        return FALSE;
      }

      //Create the data available event (of the auto-reset variety)
      ATLASSERT(m_hLifeTimeThreadRequestToStop == nullptr);
      m_hLifeTimeThreadRequestToStop = CreateEvent(nullptr, FALSE, FALSE, nullptr);
      if (m_hLifeTimeThreadRequestToStop == nullptr)
      {
        ATLTRACE(_T("CThreadPoolServer::SetMaxThreadClientLifetime, Failed to create request to stop event, Error:%u\n"), GetLastError());
        m_bMaxLifetime = FALSE;
        m_dwMaxLifetime = 0;
        return FALSE;
      }

      //Create the worker thread
      ATLASSERT(m_hLifetimeMonitorThread == nullptr);
      m_hLifetimeMonitorThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, CThreadPoolServer::_Monitor, this, CREATE_SUSPENDED, nullptr));
      if (m_hLifetimeMonitorThread == nullptr)
      {
        ATLTRACE(_T("CThreadPoolServer::SetMaxThreadClientLifetime, Failed to create worker thread for monitoring thread in thread pool\n"));
        SetLastError(ERROR_NOT_SUPPORTED);
        m_bMaxLifetime = FALSE;
        m_dwMaxLifetime = 0;
        return FALSE;
      }
      else
      {
        if (!SetThreadPriority(m_hLifetimeMonitorThread, THREAD_PRIORITY_IDLE))
        {
          ATLTRACE(_T("CThreadPoolServer::SetMaxThreadClientLifetime, Failed to set thread priority, Error:%u\n"), GetLastError());
        }
        if (ResumeThread(m_hLifetimeMonitorThread) == static_cast<DWORD>(-1))
        {
          ATLTRACE(_T("CThreadPoolServer::SetMaxThreadClientLifetime, Failed to resume thread, Error:%u\n"), GetLastError());
          return FALSE;
        }
      }
    }

    return TRUE;
  }

  BOOL GetEnabledThreadClientLifetime() const 
  { 
    return m_bMaxLifetime; 
  }

  DWORD GetMaxThreadClientLifetime() const 
  { 
    return m_dwMaxLifetime; 
  }

  std::vector<CThreadPoolClient*>::size_type GetPoolSize() const 
  { 
    //Serialize access to the threads array
    ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csThreads, TRUE);

    return m_Threads.size(); 
  }

  CThreadPoolQueue* GetQueue() const 
  { 
    return m_pQueue; 
  }

protected:
//Methods
  static unsigned __stdcall _Monitor(_In_ void* pParam)
  {
    //Validate our parameters
    ATLASSUME(pParam != nullptr);

    //Convert from the SDK world to the C++ world
    CThreadPoolServer* pServer = static_cast<CThreadPoolServer*>(pParam);
    ATLASSUME(pServer != nullptr);
    return pServer->Monitor();
  }

  virtual unsigned Monitor()
  {
    //Serialize access to the threads array
    ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csThreads, TRUE);

    //Work out the time interval (in ms) at which threads in the thread pool need to be recycled
    DWORD dwWaitInterval = m_dwMaxLifetime * 60000 / static_cast<DWORD>(m_Threads.size()); 
    sl.Unlock();

    BOOL bWantStop = FALSE;
    while (!bWantStop)
    {
      //Check to see if the m_evtRequestLifetimeThread event is signaled
      ATLASSUME(m_hLifeTimeThreadRequestToStop != nullptr);
      DWORD dwWait = WaitForSingleObject(m_hLifeTimeThreadRequestToStop, dwWaitInterval);
      if (dwWait == WAIT_OBJECT_0)
        bWantStop = TRUE;
      else
        RecycleThread();
    }

    return 0L;
  }

  virtual BOOL RecycleThread()
  {
    //Serialize access to the threads array
    ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csThreads, TRUE);

    //What will be the retuen value from this method (assume the best)
    BOOL bSuccess = TRUE;

    //Pull out the thread we will be operating on  
    CThreadPoolClient* pClient = GetAtClient(m_nLifetimeThreadIndex);
    ATLASSUME(pClient != nullptr);


    //Get the specified thread to kill

    //Set the m_bRequestToStop to ask it to exit if they are
    //currently processing a request in CThreadPoolClient::Run
    pClient->SetRequestToStop();

    //Check to see if we need to post a directed request to the thread pool to get it to exit
    HANDLE hWorkerThread = pClient->GetThreadHandle();
    BOOL bAlreadyExited = (WaitForSingleObject(hWorkerThread, 0) == WAIT_OBJECT_0);
    if (!bAlreadyExited)
    {
      //Also post a directed request to the thread pool directly
      CThreadPoolRequest killRequest;
      killRequest.m_dwID = CThreadPoolRequest::THREADPOOL_SHUTDOWN_REQUEST;
      killRequest.m_bDirectedRequest = TRUE;
      killRequest.m_nDirectedRequestIndex = m_nLifetimeThreadIndex;
      ATLASSERT(m_pQueue != nullptr);
      BOOL bPostedOK = m_pQueue->PostRequest(killRequest); 
      UNREFERENCED_PARAMETER(bPostedOK);
      ATLASSERT(bPostedOK); //If this call fails then you may be using a CThreadPoolQueue derived 
                            //class which does not support directed requests, e.g. CIOCPThreadPoolQueue
    }

    //Wait for the thread to exit
    ATLASSUME(hWorkerThread != nullptr);
    WaitForSingleObject(hWorkerThread, INFINITE);
    CloseHandle(hWorkerThread);
    pClient->SetThreadHandle(nullptr);

    //Now recreate the thread
    if (CreateWorkerThread(pClient, m_nLifetimeThreadIndex, pClient->GetStackSize(), pClient->GetThreadPriority()))
    {
      //Resume the thread
      #pragma warning(suppress: 6001)
      if (ResumeThread(hWorkerThread) == static_cast<DWORD>(-1))
      {
        ATLTRACE(_T("CThreadPoolServer::RecycleThread, Failed to resume thread at index %d, Error:%u\n"), m_nLifetimeThreadIndex, GetLastError());
        bSuccess = FALSE;
      }
    }
    else
      bSuccess = FALSE;

    //increment the thread index, ready for the next call into RecyleThread at a later date
    m_nLifetimeThreadIndex = (m_nLifetimeThreadIndex + 1) % (static_cast<int>(m_Threads.size()));

    return bSuccess;
  }

  virtual BOOL CreateWorkerThread(_In_ CThreadPoolClient* pClient, _In_ int nIndex, _In_ UINT nStackSize, _In_ int nPriority)
  {
    //What will be the return value from this method (assume the best)
    BOOL bSuccess = TRUE;

    UNREFERENCED_PARAMETER(nIndex);

    //Reset the request to stop for this thread
    pClient->ResetRequestToStop();

    //Spin of a worker thread for it (initially suspened so that we can setup it correctly!)
    ATLASSERT(pClient->GetThreadHandle() == nullptr);
    HANDLE hWorkerThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, nStackSize, CThreadPoolClient::_Run, pClient, CREATE_SUSPENDED, nullptr));
    if (hWorkerThread == 0)
    {
      ATLTRACE(_T("CThreadPoolServer::CreateWorkerThread, Failed to create worker thread for thread pool at index %d, Error:%u\n"), nIndex, GetLastError());
      bSuccess = FALSE;
    }
    else
    {
      //Update the thread handle in the client object and add it to the thread pool
      pClient->SetThreadHandle(hWorkerThread);

      //Set the thread priority
      if (!SetThreadPriority(hWorkerThread, nPriority))
      {
        ATLTRACE(_T("CThreadPoolServer::CreateWorkerThread, Failed to set thread priority at index %d, Error:%u\n"), nIndex, GetLastError());
      }
    }

    return bSuccess;
  }

//Member variables
  std::vector<CThreadPoolClient*> m_Threads;                      //The actual thread pool
  CThreadPoolQueue*               m_pQueue;                       //The queue class instance
  ATL::CComAutoCriticalSection    m_csThreads;                    //Serializes access to "m_Threads"
  BOOL                            m_bMaxLifetime;                 //Should threads be limited to a certain lifetime
  DWORD                           m_dwMaxLifetime;                //Lifetime of threads if m_bMaxLifetime is TRUE
  HANDLE                          m_hLifetimeMonitorThread;       //The thread which recycles the client threads
  HANDLE                          m_hLifeTimeThreadRequestToStop; //Event which gets set to request the lifetime monitoring thread to exit
  int                             m_nLifetimeThreadIndex;         //The index of the next thread to be recycled
};

#endif //#ifndef __THRDPOOL_H__

/*
Module : DirectedThreadPoolQueue.h
Purpose: Interface for an C++ class which implements a Pseudo IOCP like class which also 
         supports a "Directed" request to the IOCP. i.e. the request is for a specific thread 
         in the thread pool (which is the normal use of a Directed IOCP) instead of a 
         "Non-Directed" request which can be picked up by any thread in the thread pool.
Created: PJN / 16-04-2002
History: PJN / 11-11-2004 1. Provided a GetRequestArray() method which allows access to the internal
                          array used to hold the thread pool requests. Can prove handy to have access
                          to this in certain circumstances.
                          2. Updated to compile cleanly when Detect 64 bit issues and Force conformance in for loop
                          options are enabled in Visual Studio .Net
         PJN / 30-11-2004 1. Updated the macro which detects if arrays use INT_PTR for index rather than int.
                          2. Also removed some ASSERTS which were overly restrictive in light of the queue now 
                          being externally modifiable via CThreadPoolServer::GetQueue
         PJN / 13-04-2006 1. Updated copyright details.
                          2. Provision of Lock and Unlock methods along the lines of the author's CSyncCollection class
                          3. CDirectedThreadPoolQueue::PostRequest, PostRequestWithoutLimitCheck, Close and GetRequest now 
                          includes a parameter which decides if internal locking should be done. Again this allows 
                          more fine tuned external synchronisation of the class
                          4. Made timeout value in PostRequest default to "INFINITE"
                          5. Made GetDirectedRequestIndexToRemove method virtual
                          6. Optimized CDirectedThreadPoolQueue constructor code
         PJN / 08-08-2006 1. Addition of a m_evtDataAvailable member variable which is signalled when an item is available
                          on the queue
         PJN / 19-11-2007 1. Updated copyright details
         PJN / 01-05-2008 1. Addition of a GetCurrentQueueSize method.
         PJN / 09-01-2010 1. Reworked the low level internals of the thread pool to implement message pumping in 
                          CDirectedThreadPoolQueue::GetRequest instead of CThreadPoolClient::Main. This avoids a thread 
                          deadlock problem in CThreadPoolClient::Main if you want to pump messages.
                          2. Fixed a runtime ASSERT issue when removing directed requests in 
                          CDirectedThreadPoolQueue::GetRequest.
         PJN / 16-03-2014 1. Updated copyright details
         PJN / 29-03-2014 1. Reworked the CThreadPoolRequest, CThreadPoolQueue, CThreadPool, CIOCPThreadPoolQueue and 
                          CDirectedThreadPoolQueue classes to compile without MFC.
         PJN / 04-01-2015 1. Addition of CDirectedThreadPoolQueue::GetCriticalSection method.
                          2. Updated copyright details.
         PJN / 17-07-2016 1. Added SAL annotations to the CDirectedThreadPoolQueue class.
         PJN / 22-11-2016 1. The last error value is now set correctly upon return in CDirectedThreadPoolQueue::Create

Copyright (c) 2002 - 2017 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////// Defines /////////////////////////////////////////////////

#pragma once

#ifndef __DIRECTEDTHREADPOOLQUEUE_H__
#define __DIRECTEDTHREADPOOLQUEUE_H__

#ifndef THRDPOOL_EXT_CLASS
#define THRDPOOL_EXT_CLASS
#endif


///////////////////// Includes ////////////////////////////////////////////////

#include "ThrdPool.h"


///////////////////// Classes /////////////////////////////////////////////////

//Class which implements a "Directed" IO Completion Port like queue
class THRDPOOL_EXT_CLASS CDirectedThreadPoolQueue : public CThreadPoolQueue
{
public:
//Constructors / Destructors
  CDirectedThreadPoolQueue() : m_hPostRequestSemaphore(nullptr),
                               m_hGetRequestSemaphore(nullptr),
                               m_hDataAvailable(nullptr)
  {
  }

  virtual ~CDirectedThreadPoolQueue()
  {
    Close();
  }

//Methods
  virtual BOOL Create(_In_ DWORD dwMaxQSize)
  {
    //Close if already created (this will empty out the request Q for us)
    Close(TRUE);

    //Create the data available event (of the auto-reset variety)
    ATLASSERT(m_hDataAvailable == nullptr);
    m_hDataAvailable = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_hDataAvailable == nullptr)
    {
      DWORD dwError = GetLastError();
      ATLTRACE(_T("CDirectedThreadPoolQueue::Create, Failed to create the data available event, Error:%u\n"), dwError);
      Close(TRUE);
      SetLastError(dwError);
      return FALSE;
    }

    //Create the PostRequest semaphores
    ATLASSERT(m_hPostRequestSemaphore == nullptr);
    m_hPostRequestSemaphore = CreateSemaphore(nullptr, dwMaxQSize, dwMaxQSize, nullptr);
    if (m_hPostRequestSemaphore == nullptr)
    {
      DWORD dwError = GetLastError();
      ATLTRACE(_T("CDirectedThreadPoolQueue::Create, Failed to create the PostRequest semaphore, Error:%u\n"), dwError);
      Close(TRUE);
      SetLastError(dwError);
      return FALSE;
    }
    ATLASSERT(m_PostRequestSemaphores.size() == 0);
    m_PostRequestSemaphores.reserve(dwMaxQSize);
    for (DWORD i=0; i<dwMaxQSize; i++)
    {
      HANDLE hSemaphore = CreateSemaphore(nullptr, dwMaxQSize, dwMaxQSize, nullptr);
      if (hSemaphore == nullptr)
      {
        DWORD dwError = GetLastError();
        ATLTRACE(_T("CDirectedThreadPoolQueue::Create, Failed to create a PostRequest semaphore at index %u, Error:%u\n"), i, dwError);
        Close(TRUE);
        SetLastError(dwError);
        return FALSE;
      }
      m_PostRequestSemaphores.push_back(hSemaphore);
    }

    //Create the GetRequest semaphores
    m_hGetRequestSemaphore = CreateSemaphore(nullptr, 0, dwMaxQSize, nullptr);
    if (m_hGetRequestSemaphore == nullptr)
    {
      DWORD dwError = GetLastError();
      ATLTRACE(_T("CDirectedThreadPoolQueue::Create, Failed to create the GetRequest semaphore, Error:%u\n"), dwError);
      Close(TRUE);
      SetLastError(dwError);
      return FALSE;
    }
    ATLASSERT(m_GetRequestSemaphores.size() == 0);
    m_GetRequestSemaphores.reserve(dwMaxQSize);
    for (DWORD i=0; i<dwMaxQSize; i++)
    {
      HANDLE hSemaphore = CreateSemaphore(nullptr, 0, dwMaxQSize, nullptr);
      if (hSemaphore == nullptr)
      {
        DWORD dwError = GetLastError();
        ATLTRACE(_T("CDirectedThreadPoolQueue::Create, Failed to create a GetRequest semaphore at index %u, Error:%u\n"), i, dwError);
        Close(TRUE);
        SetLastError(dwError);
        return FALSE;
      }
      m_GetRequestSemaphores.push_back(hSemaphore);
    }

    return TRUE;
  }

  virtual BOOL Close(_In_ BOOL bLock = TRUE)
  {
    //Empty out the request queue
    ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csRequests, FALSE);
    if (bLock)
      sl.Lock();
    m_Requests.clear();
  
    //Free up the PostRequest semaphores
    if (m_hPostRequestSemaphore)
    {
      CloseHandle(m_hPostRequestSemaphore);
      m_hPostRequestSemaphore = nullptr;
    }
    for (std::vector<HANDLE>::size_type i=0; i<m_PostRequestSemaphores.size(); i++)
    {
      HANDLE hSemaphore = m_PostRequestSemaphores[i];
      CloseHandle(hSemaphore);
    }
    m_PostRequestSemaphores.clear();

    //Free up the GetRequest semaphores
    if (m_hGetRequestSemaphore)
    {
      CloseHandle(m_hGetRequestSemaphore);
      m_hGetRequestSemaphore = nullptr;
    }
    for (std::vector<HANDLE>::size_type i=0; i<m_GetRequestSemaphores.size(); i++)
    {
      HANDLE hSemaphore = m_GetRequestSemaphores[i];
      CloseHandle(hSemaphore);
    }
    m_GetRequestSemaphores.clear();

    //Free up the data available event
    if (m_hDataAvailable != nullptr)
    {
      CloseHandle(m_hDataAvailable);
      m_hDataAvailable = nullptr;
    }

    return TRUE;
  }

  virtual BOOL PostRequest(_In_ const CThreadPoolRequest& request, _In_ DWORD dwMilliseconds = INFINITE, _In_ BOOL bLock = TRUE)
  {
    ATLASSERT(IsCreated()); //Must have been created

    //Wait for the post request semaphore
    DWORD dwWait = 0;
    if (request.m_bDirectedRequest)
      dwWait = WaitForSingleObject(m_PostRequestSemaphores[request.m_nDirectedRequestIndex], dwMilliseconds);
    else
      dwWait = WaitForSingleObject(m_hPostRequestSemaphore, dwMilliseconds);
    if (dwWait != WAIT_OBJECT_0)
    {
      ATLTRACE(_T("CDirectedThreadPoolQueue::PostRequest, Failed while waiting for the queue to free up, Error:%u\n"), GetLastError());
      return FALSE;
    }

    //Pass the buck to the other PostRequest method
    return PostRequestWithoutLimitCheck(request, bLock);
  }

  #pragma warning(suppress: 26165)
  virtual BOOL PostRequestWithoutLimitCheck(_In_ const CThreadPoolRequest& request, _In_ BOOL bLock = TRUE)
  {
    ATLASSERT(IsCreated()); //Must have been created

    //What will be the return value from this method (assume the best)
    BOOL bSuccess = TRUE;

    //Add the request to the request queue
    ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csRequests, FALSE);
    if (bLock)
      sl.Lock();
    CThreadPoolRequest CopyOfRequest(request);
    m_Requests.push_back(CopyOfRequest);
    INT_PTR nRequestIndex = static_cast<INT_PTR>(m_Requests.size() - 1);
    ATLASSERT(nRequestIndex != -1);

    //Release the semaphore
    if (request.m_bDirectedRequest)
    {
      if (!ReleaseSemaphore(m_GetRequestSemaphores[request.m_nDirectedRequestIndex], 1, nullptr))
      {
        bSuccess = FALSE;

        //Remove the item from the Q since we could not update the "Get" semaphores
        m_Requests.erase(m_Requests.begin() + nRequestIndex);

        ATLTRACE(_T("CDirectedThreadPoolQueue::PostRequestWithoutLimitCheck, Failed to release a semaphore, Error:%u\n"), GetLastError());
      }
    }
    else
    {
      if (!ReleaseSemaphore(m_hGetRequestSemaphore, 1, nullptr))
      {
        bSuccess = FALSE;

        //Remove the item from the Q since we could not update the "Get" semaphores
        m_Requests.erase(m_Requests.begin() + nRequestIndex);

        ATLTRACE(_T("CDirectedThreadPoolQueue::PostRequestWithoutLimitCheck, Failed to release a semaphore, Error:%u\n"), GetLastError());
      }
    }

    //set the event to signify that data is now available on the collection
    ATLASSUME(m_hDataAvailable != nullptr);
    if (!SetEvent(m_hDataAvailable))
      ATLTRACE(_T("CDirectedThreadPoolQueue::PostRequestWithoutLimitCheck, Failed to set data available event, Error:%u\n"), GetLastError());

    return bSuccess;
  }

  virtual BOOL PumpMessage()
  {
  #ifdef _AFX 
    return AfxGetApp()->PumpMessage(); 
  #else
    MSG msg;
    if (!GetMessage(&msg, nullptr, 0, 0))
      return FALSE;

    //process this message
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    return TRUE;
  #endif
  }

  virtual BOOL PumpMessages()
  {
    //Pump the message queue if necessary
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE))
    {
      if (!PumpMessage())
        return FALSE;
    }

    return TRUE;
  }

  #pragma warning(suppress: 26165)
  virtual BOOL GetRequest(_Inout_ CThreadPoolRequest& request, _In_ int nThreadIndexForDirectedRequest, _In_ DWORD dwMilliseconds, _In_ BOOL bLock, _In_ BOOL bPumpMessage)
  {
    ATLASSERT(IsCreated()); //Must have been created

    //Wait for either a non directed request or a directed request for this thread to become available on the Q
    HANDLE hWaitHandles[2];
    hWaitHandles[0] = m_GetRequestSemaphores[nThreadIndexForDirectedRequest];
    hWaitHandles[1] = m_hGetRequestSemaphore;
    DWORD dwWait = 0;
    if (bPumpMessage)
    {
      dwWait = MsgWaitForMultipleObjects(2, hWaitHandles, FALSE, dwMilliseconds, QS_ALLINPUT);
      if (dwWait == (WAIT_OBJECT_0 + 2)) //Is a message waiting?
      {
        if (!PumpMessages())
          return FALSE;
      }
    }
    else
      dwWait = WaitForMultipleObjects(2, hWaitHandles, FALSE, dwMilliseconds);
    int nSignaledHandle = dwWait - WAIT_OBJECT_0;

    //Work out what the return value from WFMO means!
    BOOL bRemoveDirected = FALSE;
    if (nSignaledHandle == 0)
      bRemoveDirected = TRUE;
    else if (nSignaledHandle != 1)
    {
      ATLTRACE(_T("CDirectedThreadPoolQueue::GetRequest, Failed while waiting for the item on the queue, Error:%u\n"), GetLastError());
      return FALSE;
    }

    //What will be the return value from the function (assume the best)
    BOOL bSuccess = TRUE;

    //Remove some item from the request Q  
    ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csRequests, FALSE);
    if (bLock)
      sl.Lock(); //Lock down access to the Q

    if (bRemoveDirected)
    {
      //Work out the item to remove from the Q
      INT_PTR nIndexToRemoveAt = GetDirectedRequestIndexToRemove(nThreadIndexForDirectedRequest);
      if (nIndexToRemoveAt != -1)
      {
        request = m_Requests[nIndexToRemoveAt];
        ATLASSERT(request.m_bDirectedRequest); //the GetDirectedRequestIndexToRemove call above has returned an incorrect index
        m_Requests.erase(m_Requests.begin() + nIndexToRemoveAt);

        //Release the PostRequest semaphore
        ReleaseSemaphore(m_PostRequestSemaphores[nThreadIndexForDirectedRequest], 1, nullptr);
      }
      else
        bSuccess = FALSE;
    }
    else
    {
      //Work out the item to remove from the Q
      INT_PTR nIndexToRemoveAt = GetNonDirectedRequestIndexToRemove();
      if (nIndexToRemoveAt != -1)
      {
        request = m_Requests[nIndexToRemoveAt];
        ATLASSERT(!request.m_bDirectedRequest); //the GetNonDirectedRequestIndexToRemove call above has returned an incorrect index
        m_Requests.erase(m_Requests.begin() + nIndexToRemoveAt);

        //Release the PostRequest semaphore
        ReleaseSemaphore(m_hPostRequestSemaphore, 1, nullptr);
      }
      else
        bSuccess = FALSE;
    }
  
    //Now check to see if there is an outstanding item on the queue after we have removed the current item
    //and if so signal the data available event
    std::vector<CThreadPoolRequest>::size_type nItems = m_Requests.size();
    if (nItems)
    {
      ATLASSERT(m_hDataAvailable != nullptr);
      if (!SetEvent(m_hDataAvailable))
        ATLTRACE(_T("CDirectedThreadPoolQueue::GetRequest, Failed to set data available event, Error:%u\n"), GetLastError());
    }

    return bSuccess;
  }

  virtual BOOL IsCreated() const
  {
    return (m_hPostRequestSemaphore != nullptr);
  }

  virtual INT_PTR GetDirectedRequestIndexToRemove(_In_ int nThreadIndexForDirectedRequest)
  {
    //Work out the item to remove from the Q, by default we pick the first 
    //directed request for this thread starting from the tail of the queue
    INT_PTR nIndexToRemoveAt = -1;
    INT_PTR nRequestSize = static_cast<INT_PTR>(m_Requests.size());
    for (INT_PTR i=0; (i<nRequestSize) && (nIndexToRemoveAt == -1); i++)
    {
      CThreadPoolRequest& tempRequest = m_Requests[i];
      if (tempRequest.m_bDirectedRequest && nThreadIndexForDirectedRequest == tempRequest.m_nDirectedRequestIndex)
        nIndexToRemoveAt = i;
    }

    return nIndexToRemoveAt;
  }

  virtual INT_PTR GetNonDirectedRequestIndexToRemove()
  {
    //Work out the item to remove from the Q, by default we pick
    //the first non directed request starting from the tail of the queue
    INT_PTR nIndexToRemoveAt = -1;
    INT_PTR nRequestSize = static_cast<INT_PTR>(m_Requests.size());
    for (INT_PTR i=0; (i<nRequestSize) && (nIndexToRemoveAt == -1); i++)
    {
      CThreadPoolRequest& tempRequest = m_Requests[i];
      if (!tempRequest.m_bDirectedRequest)
        nIndexToRemoveAt = i;
    }

    return nIndexToRemoveAt;
  }

  virtual BOOL SupportsDirectedRequests() const 
  { 
    return TRUE; 
  }

  std::vector<CThreadPoolRequest>& GetRequestArray() 
  { 
    return m_Requests; 
  }

  virtual INT_PTR GetCurrentQueueSize()
  {
    //Serialize access to the requests array
    ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_csRequests, TRUE);

    return static_cast<INT_PTR>(m_Requests.size());
  }

  virtual ATL::CComAutoCriticalSection& GetCriticalSection()
  {
    return m_csRequests;
  }

//Member variables
  HANDLE m_hDataAvailable; //Signalled when a item is available on the queue

protected:
//Member variables
  HANDLE                          m_hPostRequestSemaphore; //Semaphore which is used to implement synchronisation in "PostRequest"
  HANDLE                          m_hGetRequestSemaphore;  //Semaphore which is used to implement synchronisation in "GetRequest"
  ATL::CComAutoCriticalSection    m_csRequests;            //Serializes access to the Q
  std::vector<CThreadPoolRequest> m_Requests;              //The request list
  std::vector<HANDLE>             m_PostRequestSemaphores; //Array of semaphores used for signalling when a directed request is available
  std::vector<HANDLE>             m_GetRequestSemaphores;  //Array of semaphores used for signalling when a directed request is available
};

#endif //#ifndef __DIRECTEDTHREADPOOLQUEUE_H__

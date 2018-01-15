/*
Module : IOCPThreadPoolQueue.h
Purpose: Interface for an C++ class which implements a queue class for CThreadPoolServer using an SDK IO Completion port
Created: PJN / 18-08-2002
History: PJN / 10-11-2002 1. Fixed an unreferrenced variable in the function CIOCPThreadPoolQueue::GetRequest, Thanks to
                          Michael K. O'Neill for reporting this issue.
         PJN / 27-06-2006 1. Optimized CIOCPThreadPoolQueue constructor code
         PJN / 16-10-2007 1. Updated code to compile cleanly for x64
         PJN / 16-03-2014 1. Updated copyright details
         PJN / 29-03-2014 1. Reworked the CThreadPoolRequest, CThreadPoolQueue, CThreadPool, CIOCPThreadPoolQueue and 
                          CDirectedThreadPoolQueue classes to compile without MFC.
         PJN / 17-07-2016 1. Added SAL annotations to the CIOCPThreadPoolQueue class.
         PJN / 22-11-2016 1. The last error value is now set correctly upon return in CIOCPThreadPoolQueue::Create

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

#ifndef __IOCPTHREADPOOLQUEUE_H__
#define __IOCPTHREADPOOLQUEUE_H__


///////////////////// Includes ////////////////////////////////////////////////

#include "ThrdPool.h"


///////////////////// Classes /////////////////////////////////////////////////

//Class which implements a queue for CThreadPoolServer using an IO Completion Port
class CIOCPThreadPoolQueue : public CThreadPoolQueue
{
public:
//Constructors / Destructors
  CIOCPThreadPoolQueue() : m_hIOCP(nullptr)
  {
  }

  virtual ~CIOCPThreadPoolQueue()
  {
    Close();
  }

//Methods
  virtual BOOL Create(_In_ DWORD dwMaxQSize)
  {
    //Close if already created
    Close();

    //Create the IOCP
    m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, dwMaxQSize);
    if (m_hIOCP == nullptr)
    {
      DWORD dwError = GetLastError();
      ATLTRACE(_T("CIOCPThreadPoolQueue::Create, Failed to create a IO Completion Port, Error:%u\n"), dwError);
      Close();
      SetLastError(dwError);
      return FALSE;
    }

    return TRUE;
  }

  virtual BOOL Close()
  {
    //Free up the IOCP
    if (m_hIOCP)
    {
      CloseHandle(m_hIOCP);
      m_hIOCP = nullptr;
    }

    return TRUE;
  }

  virtual BOOL PostRequest(_In_ const CThreadPoolRequest& request, _In_ DWORD /*dwMilliseconds*/ = INFINITE, _In_ BOOL /*bLock*/ = TRUE)
  {
    ATLASSERT(IsCreated()); //Must have been created

    //An IOCP does not support directed requests
    if (request.m_bDirectedRequest)
    {
      ATLTRACE(_T("CIOCPThreadPoolQueue::PostRequest, An IOCP based thread pool queue does not support directed requests\n"));
      return FALSE;
    }
    else
    {
      if (!PostQueuedCompletionStatus(m_hIOCP, request.m_dwID, NULL, static_cast<LPOVERLAPPED>(request.m_pData)))
      {
        ATLTRACE(_T("CIOCPThreadPoolQueue::PostRequest, Failed in call to PostQueuedCompletionStatus, Error:%u\n"), GetLastError());
        return FALSE;
      }
    }

    return TRUE;
  }

  virtual BOOL PostRequestWithoutLimitCheck(_In_ const CThreadPoolRequest& request, _In_ BOOL bLock = TRUE)
  { 
    return PostRequest(request, 0, bLock); 
  }

  virtual BOOL GetRequest(_In_ CThreadPoolRequest& request, int /*nThreadIndexForDirectedRequest*/, _In_ DWORD dwMilliseconds, _In_ BOOL /*bLock*/, _In_ BOOL /*bPumpMessage*/)
  {
    ATLASSERT(IsCreated()); //Must have been created

    //Pull of a request from the IOCP
    DWORD dwBytesTransferred = 0;
    ULONG_PTR dwCompletionKey = 0;
    LPOVERLAPPED lpOverlapped = nullptr;
    if (!GetQueuedCompletionStatus(m_hIOCP, &dwBytesTransferred, &dwCompletionKey, &lpOverlapped, dwMilliseconds))
    {
      ATLTRACE(_T("CIOCPThreadPoolQueue::GetRequest, Failed while waiting for the item on the Q, Error:%u\n"), GetLastError());
      return FALSE;
    }

    request.m_bDirectedRequest = FALSE; //Always will be a non-directed request from an IOCP queue
    request.m_nDirectedRequestIndex = -1; 
    request.m_dwID = dwBytesTransferred;
    request.m_pData = lpOverlapped;

    return TRUE;
  }

  virtual BOOL IsCreated() const
  {
    return (m_hIOCP != nullptr);
  }

  virtual BOOL SupportsDirectedRequests() const 
  { 
    return FALSE; 
  }

protected:
//Member variables
  HANDLE m_hIOCP; //Handle to the SDK IOCP
};

#endif //#ifndef __IOCPTHREADPOOLQUEUE_H__

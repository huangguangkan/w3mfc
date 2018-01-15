/*
Module : W3MFC.cpp
Purpose: Implementation for a simple MFC class encapsulation of a HTTP server
Created: PJN / 22-04-1999

Copyright (c) 1999 - 2017 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


//////////////// Includes /////////////////////////////////////////////////////

#include "stdafx.h"
#include "SocMFC.h"
#include "W3MFC.h"


//////////////// Macros / Defines /////////////////////////////////////////////

//automatically pull in Winsock 2
#pragma comment(lib, "Ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG


//////////////// Implementation ///////////////////////////////////////////////

CW3MFCDirectedThreadPoolQueue::~CW3MFCDirectedThreadPoolQueue()
{
  //Before we are destroyed, deallocate any user requests
  //which may still be lying around in the requests array
  for (std::vector<CThreadPoolRequest>::size_type i=0; i<m_Requests.size(); i++)
  {
    CThreadPoolRequest& request = m_Requests[i];
    if (request.m_dwID == CThreadPoolRequest::THREADPOOL_USER_DEFINED_REQUEST)
    {
      CW3MFCThreadPoolRequest* pW3MFCRequest = static_cast<CW3MFCThreadPoolRequest*>(request.m_pData);
      delete pW3MFCRequest;
      pW3MFCRequest = nullptr;
    }
  }
}

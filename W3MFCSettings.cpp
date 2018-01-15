/*
Module : W3MFCSettings.cpp
Purpose: Implementation for the CW3MFCServerSettings class
Created: PJN / 23-03-2014
History: PJN / 28-04-2017 1. Updated the code to compile cleanly using /permissive-.

Copyright (c) 2015 - 2017 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

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
#include "W3MFCSettings.h"


//////////////// Macros / Defines /////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG


//////////////// Implementation ///////////////////////////////////////////////

CW3MFCSettings::CW3MFCSettings() : m_nPort(80),                    //Default to the standard HTTP port
                                   m_bBind(FALSE),                 //Default to not binding to a specific IP address
                                   m_bIPv6(FALSE),
                                   m_dwIdleClientTimeout(30000),   //Default to client idle timeout of 30 seconds
                                   m_sServerName(_T("W3MFC/2.08")), //Default server name will be the name of the MFC classes i.e "W3MFC" plus the current version number 
                                   m_nThreadPoolSize(10),
                                   m_nThreadPoolQueueSize(100),
                                   m_nListenThreadStackSize(0),
                                   m_bDNSLookup(FALSE),
                                   m_dwWritableTimeout(10000),
                                   m_bEnableThreadLifetime(FALSE),
                                   m_dwThreadLifetime(120),
                                   m_pMimeManager(nullptr),
                                   m_bUseIOCPQueue(FALSE),
                                   m_bAutoExpire(FALSE),
                                 #ifndef W3MFC_NO_CGI_SUPPORT
                                   m_dwCGIResponseBufferSize(4096),
                                 #endif //#ifndef W3MFC_NO_CGI_SUPPORT
                                   m_bAllowAnonymous(TRUE),
                                   m_bAllowBasicAuthentication(TRUE),
                                   m_bPerformPassthroughAuthentication(TRUE),
                                 #ifndef W3MFC_NO_ISAPI_SUPPORT
                                   m_bCacheISAPI(TRUE),
                                   m_pISAPIManager(nullptr),
                                   m_pISAPI(nullptr),
                                 #endif //#ifndef W3MFC_NO_ISAPI_SUPPORT
                                 #ifndef W3MFC_NO_CGI_SUPPORT
                                   m_pCGI(nullptr),
                                 #endif //#ifndef W3MFC_NO_CGI_SUPPORT
                                 #ifdef W3MFC_SSL_SUPPORT
                                   m_bSSL(FALSE),
                                   m_dwSSLEnabledProtocols(0),
                                   m_sServerCertificateName(_T("localhost")),
                                   m_dwSSLReadTimeout(5000),
                                   m_dwSSLWriteTimeout(5000),
                                 #endif //#ifdef W3MFC_SSL_SUPPORT
                                   m_bAllowDeleteRequest(FALSE)
{
  //Default root directory will be where the exe is running from
  TCHAR szPath[_MAX_PATH];
  szPath[0] = _T('\0');
  if (GetModuleFileName(nullptr, szPath, _MAX_PATH))
  {
    TCHAR szDrive[_MAX_DRIVE];   
    szDrive[0] = _T('\0');
    TCHAR szDir[_MAX_DIR];
    szDir[0] = _T('\0');
    _tsplitpath_s(szPath, szDrive, sizeof(szDrive)/sizeof(TCHAR), szDir, sizeof(szDir)/sizeof(TCHAR), nullptr, 0, nullptr, 0);
    _tmakepath_s(szPath, sizeof(szPath)/sizeof(TCHAR), szDrive, szDir, nullptr, nullptr);

    CW3MFCDirectory* pDir = new CW3MFCDirectory;
    pDir->SetDirectory(szPath);    
    pDir->SetAlias(_T("/"));
    pDir->SetDefaultFile(_T("default.htm")); //Default filename returned for root requests will be "default.htm"
    m_Directories.Add(pDir);
  }
}

CW3MFCSettings::~CW3MFCSettings()
{
  FreeDirectoryArray();
}

void CW3MFCSettings::FreeDirectoryArray()
{
  for (INT_PTR i=0; i<m_Directories.GetSize(); i++)
  {
    CW3MFCDirectory* pDir = m_Directories[i];
    delete pDir; 
    pDir = nullptr;
  }
  m_Directories.RemoveAll();
}

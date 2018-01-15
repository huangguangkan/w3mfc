/*
Module : W3MFCISAPI.cpp
Purpose: Implementation for the CW3MFCISAPI class
Created: PJN / 21-02-2003
History: PJN / 22-02-2004 1. Fixed a memory leak in CW3MFCISAPI::CachedLoad. Thanks to "mori" for
                          reporting and suggesting the fix for this bug.
         PJN / 19-02-2006 1. Replaced all calls to ZeroMemory and CopyMemory with memset and memcpy
         PJN / 27-06-2006 1. Optimized CW3MFCISAPIExtension constructor code
                          2. Optimized CW3MFCISAPI constructor code
         PJN / 19-11-2007 1. Updated copyright details.
                          2. CHttpISAPI class has been renamed to CW3MFCISAPI
                          3. CW3MFCISAPIExtension class has been renamed to CW3MFCISAPIExtension
         PJN / 31-05-2008 1. Code now compiles cleanly using Code Analysis (/analyze)
         PJN / 23-05-2009 1. Removed use of CT2A throughout the code.
         PJN / 16-03-2014 1. Updated copyright details
         PJN / 17-07-2016 1. Updated copyright details
         PJN / 28-04-2017 1. Updated the code to compile cleanly using /permissive-.
         PJN / 13-08-2017 1. Replaced CString::operator LPC*STR() calls throughout the codebase with
                          CString::GetString calls

Copyright (c) 2003 - 2017 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

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
#include "W3MFCISAPI.h"
#include "W3MFCClient.h"


//////////////// Macros / Defines /////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG


//////////////// Implementation ///////////////////////////////////////////////

CW3MFCISAPIExtension::CW3MFCISAPIExtension() : m_hDLL(nullptr),
                                               m_lpGetExtensionVersion(nullptr),
                                               m_lpHttpExtensionProc(nullptr),
                                               m_lpTerminateExtension(nullptr)
{
}

CW3MFCISAPIExtension::~CW3MFCISAPIExtension()
{
  if (m_hDLL != nullptr)
  {
    //Call the terminate function (if it exists)
    if (m_lpTerminateExtension != nullptr)
      CallTerminateExtension();
      
    //Finally free up the library
    FreeLibrary(m_hDLL);
  }
}

BOOL CW3MFCISAPIExtension::CallGetExtensionVersion(HSE_VERSION_INFO* phseVerInfo)
{
  //Validate our parameters
  ASSERT(m_lpGetExtensionVersion != nullptr);

  //Assume the worst
  BOOL bSuccess = FALSE;

  __try
  {
    bSuccess = m_lpGetExtensionVersion(phseVerInfo);
  }
  __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) //Note W3MFC only handles access violations from an ISAPI dll, everything else is considered critical
  {
    bSuccess = FALSE;
  }

  return bSuccess;
}

BOOL CW3MFCISAPIExtension::CallTerminateExtension()
{
  //Validate our parameters
  ASSERT(m_lpTerminateExtension != nullptr);

  //Assume the worst
  BOOL bSuccess = FALSE;

  __try
  {
    bSuccess = m_lpTerminateExtension(HSE_TERM_MUST_UNLOAD);
  }
  __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) //Note W3MFC only handles access violations from an ISAPI dll, everything else is considered critical
  {
    bSuccess = FALSE;
  }

  return bSuccess;
}



CW3MFCISAPI::CW3MFCISAPI(int nISAPIHashTableSize) : m_mapExtensions(nISAPIHashTableSize),
                                                    m_pError(nullptr),
                                                    m_pSettings(nullptr)
{
}

CW3MFCISAPI::~CW3MFCISAPI()
{
  FreeMapEntries();
}

void CW3MFCISAPI::SetError(IW3MFCError* pError)
{
  ASSERT(pError != nullptr);
  m_pError = pError;
}

void CW3MFCISAPI::FreeMapEntries()
{
  //Prevent the hash table from being manipulated
  //by multiple threads at the one time
  CSingleLock sl(&m_CS, TRUE);

  //Remove all the entries in the hash table
  for (POSITION pos = m_mapExtensions.GetStartPosition(); pos != nullptr; )
  {
    CW3MFCISAPIExtension* pEntry = nullptr;
    CString sKey;
    m_mapExtensions.GetNextAssoc(pos, sKey, reinterpret_cast<void*&>(pEntry));
    if (pEntry != nullptr)
      delete pEntry;
  }
  m_mapExtensions.RemoveAll();
}

BOOL CW3MFCISAPI::UncachedLoad(const CString& sPath, CW3MFCISAPIExtension& extension)
{
  //Validate our parameters
  AFXASSUME(m_pError != nullptr);

  //What will be the return value
  BOOL bSuccess = FALSE;
 
  //Load up the DLL
  ASSERT(extension.m_hDLL == nullptr);
  extension.m_sPath = sPath;
  extension.m_hDLL = LoadLibraryEx(sPath, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
  if (extension.m_hDLL != nullptr)
  {
    //Get the "GetExtensionVersion" function
    extension.m_lpGetExtensionVersion = reinterpret_cast<CW3MFCISAPIExtension::LPGETEXTENSIONVERSION>(GetProcAddress(extension.m_hDLL, "GetExtensionVersion"));
    if (extension.m_lpGetExtensionVersion == nullptr)
    {
      //Report the error that we could not find the function
      CString sError;
      sError.Format(_T("CW3MFCISAPI::UncachedLoad, Could not find entry point GetExtensionVersion in dll %s"), sPath.GetString());
      m_pError->OnError(sError);
    }

    //Get the HttpExtensionProc function
    extension.m_lpHttpExtensionProc = reinterpret_cast<CW3MFCISAPIExtension::LPHTTPEXTENSIONPROC>(GetProcAddress(extension.m_hDLL, "HttpExtensionProc"));
    if (extension.m_lpHttpExtensionProc == nullptr)
    {
      //Report the error that we could not find the function
      CString sError;
      sError.Format(_T("CW3MFCISAPI::UncachedLoad, Could not find entry point HttpExtensionProc in dll %s"), sPath.GetString());
      m_pError->OnError(sError);
    }

    if ((extension.m_lpGetExtensionVersion != nullptr) && (extension.m_lpHttpExtensionProc != nullptr))
    {
      //Now find the optional "TerminateExtension" function (if initialisation succeeded)
      extension.m_lpTerminateExtension = reinterpret_cast<CW3MFCISAPIExtension::LPTERMINATEEXTENSION>(GetProcAddress(extension.m_hDLL, "TerminateExtension"));

      //Also call the GetExtensionVersion function
      HSE_VERSION_INFO hseVerInfo;
      CString sError;
      bSuccess = extension.CallGetExtensionVersion(&hseVerInfo);
      if (!bSuccess)
      {
        //Report the error that we could not call the GetExtensionVersion function
        sError.Format(_T("CW3MFCISAPI::UncachedLoad, Failed in call to GetExtensionVersion in dll %s, error:%u"), sPath.GetString(), GetLastError());
        m_pError->OnError(sError);
      }
    }
  }
  else
  {
    //Report the error that we could not find the function
    DWORD dwError = GetLastError();
    CString sError;
    sError.Format(_T("CW3MFCISAPI::UncachedLoad, Could not load the dll %s, error:%u"), sPath.GetString(), dwError);
    m_pError->OnError(sError);
  }

  return bSuccess;
}

CW3MFCISAPIExtension* CW3MFCISAPI::CachedLoad(const CString& sPath)
{
  //Look up in the hash table case insensitively
  CString sKey(sPath);
  sKey.MakeUpper(); 

  //What will be the return value
  CW3MFCISAPIExtension* pExtension = nullptr;

  //Prevent the hash table from being manipulated by multiple threads at the one time
  CSingleLock sl(&m_CS, TRUE);

  //try looking it up in the cache first
  if (!m_mapExtensions.Lookup(sKey, reinterpret_cast<void*&>(pExtension)))
  {
    //Only created if not found in hash
    pExtension = new CW3MFCISAPIExtension;
 
    //Call the other Load method to do most of the work
    if (!UncachedLoad(sPath, *pExtension))
    {
      delete pExtension;
      pExtension = nullptr;
    }
    else
    {
      //Add it to the hash if caching is enabled
      m_mapExtensions.SetAt(sKey, pExtension);
    }
  }

  return pExtension;
}

BOOL CW3MFCISAPI::GetServerVariable(HCONN hConn, LPSTR lpszVariableName, void* lpBuffer, DWORD* lpdwBufferSize)
{
  //Validate our parameters
  CW3MFCClient* pClient = static_cast<CW3MFCClient*>(hConn);
  if (pClient == nullptr)
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }  
  AFXASSUME(pClient->m_pSettings != nullptr);

  //Assume the best
  BOOL bSuccess = TRUE;

  if (_strcmpi(lpszVariableName, "SERVER_PORT") == 0)
  {
    CStringA sPort;
    sPort.Format("%d", static_cast<int>(pClient->m_pSettings->m_nPort));
    DWORD dwSize = static_cast<DWORD>(sPort.GetLength() + 1);
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(reinterpret_cast<char*>(lpBuffer), dwSize, sPort);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "SERVER_PORT_SECURE") == 0)
  {
  #ifdef W3MFC_SSL_SUPPORT
    if (pClient->m_pSettings->m_bSSL)
    {
      CStringA sSecure;
      sSecure.Format("%d", static_cast<int>(pClient->m_pSettings->m_nPort));
      DWORD dwSize = static_cast<DWORD>(sSecure.GetLength() + 1);
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sSecure);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  #else
    SetLastError(ERROR_NO_DATA);
    bSuccess = FALSE;
  #endif //#ifdef W3MFC_SSL_SUPPORT
  }
  else if (_strcmpi(lpszVariableName, "SERVER_PROTOCOL") == 0)
  {
    char szProtocol[10];
  #ifdef W3MFC_SSL_SUPPORT
    if (pClient->m_pSettings->m_bSSL)
      strcpy_s(szProtocol, sizeof(szProtocol), "HTTPS/1.0");
    else
      strcpy_s(szProtocol, sizeof(szProtocol), "HTTP/1.0");
  #else
    strcpy_s(szProtocol, sizeof(szProtocol), "HTTP/1.0");
  #endif //#ifdef W3MFC_SSL_SUPPORT
    DWORD dwSize = static_cast<DWORD>(strlen(szProtocol) + 1);
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, szProtocol);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "SERVER_SOFTWARE") == 0)
  {
    CStringA sSoftware(pClient->m_pSettings->m_sServerName);
    DWORD dwSize = sSoftware.GetLength() + 1;
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sSoftware);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "URL") == 0)
  {
    CStringA sURL(pClient->m_Request.m_sURL);
    DWORD dwSize = sURL.GetLength() + 1;
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sURL);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "QUERY_STRING") == 0)
  {
    CStringA sQueryString(pClient->m_Request.m_sRawExtra);
    DWORD dwSize = sQueryString.GetLength() + 1;
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sQueryString);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "PATH_INFO") == 0)
  {
    CStringA sPathInfo(pClient->m_Request.m_sPathInfo);
    DWORD dwSize = sPathInfo.GetLength() + 1;
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sPathInfo);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "REMOTE_ADDR") == 0)
  {
  #ifdef CWSOCKET_MFC_EXTENSIONS
    CStringA sRemoteAddress(CWSocket::AddressToString(pClient->m_Request.m_ClientAddress));
  #else
    CStringA sRemoteAddress(CWSocket::AddressToString(pClient->m_Request.m_ClientAddress).c_str());
  #endif //#ifdef CWSOCKET_MFC_EXTENSIONS
    DWORD dwSize = static_cast<DWORD>(sRemoteAddress.GetLength() + 1);
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sRemoteAddress);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "REMOTE_HOST") == 0)
  {
    if (pClient->m_Request.m_sRemoteHost.GetLength())
    {
      CStringA sRemoteHost(pClient->m_Request.m_sRemoteHost);
      DWORD dwSize = sRemoteHost.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sRemoteHost);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      //just return the IP address if the remote host string is empty
      return GetServerVariable(hConn, const_cast<LPSTR>("REMOTE_ADDR"), lpBuffer, lpdwBufferSize);
    }
  }
  else if ((_strcmpi(lpszVariableName, "REMOTE_USER") == 0) ||
           (_strcmpi(lpszVariableName, "AUTH_USER") == 0))
  {
    CStringA sUser(pClient->m_Request.m_sUserName);
    DWORD dwSize = sUser.GetLength() + 1;
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sUser);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
    CW3MFCRequest::SecureEmptyString(sUser);
  }
  else if (_strcmpi(lpszVariableName, "REQUEST_METHOD") == 0)
  {
    CStringA sVerb(pClient->m_Request.m_sVerb);
    DWORD dwSize = sVerb.GetLength() + 1;
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sVerb);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "SERVER_NAME") == 0)
  {
    CStringA sServerName;
    char szHostName[256];
    szHostName[0] = '\0';
  #ifdef CWSOCKET_MFC_EXTENSIONS
    if (pClient->m_pSettings->m_sBindAddress.GetLength())
      sServerName = pClient->m_pSettings->m_sBindAddress;
  #else
    if (pClient->m_pSettings->m_sBindAddress.length())
      sServerName = pClient->m_pSettings->m_sBindAddress.c_str();
  #endif //#ifdef CWSOCKET_MFC_EXTENSIONS
    else
    {
      if (gethostname(szHostName, sizeof(szHostName)) == 0)
        sServerName = szHostName; 
    }

    DWORD dwSize = sServerName.GetLength() + 1;
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sServerName);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "HTTPS") == 0)
  {
    char szHTTPS[10];
    szHTTPS[0] = '\0';
  #ifdef W3MFC_SSL_SUPPORT
    if (pClient->m_pSettings->m_bSSL)
      strcpy_s(szHTTPS, sizeof(szHTTPS), "on");
    else
      strcpy_s(szHTTPS, sizeof(szHTTPS), "off");
  #else
    strcpy_s(szHTTPS, sizeof(szHTTPS), "off");
  #endif //#ifdef W3MFC_SSL_SUPPORT

    DWORD dwSize = static_cast<DWORD>(strlen(szHTTPS) + 1);
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, szHTTPS);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "AUTH_TYPE") == 0)
  {
    char szType[10];
    if (pClient->m_Request.m_AuthorizationType == CW3MFCRequest::HTTP_AUTHORIZATION_PLAINTEXT)
      strcpy_s(szType, sizeof(szType), "Basic");
    else if (pClient->m_Request.m_AuthorizationType == CW3MFCRequest::HTTP_AUTHORIZATION_NTLM)
      strcpy_s(szType, sizeof(szType), "NTLM");
    else
      strcpy_s(szType, sizeof(szType), "");

    DWORD dwSize = static_cast<DWORD>(strlen(szType) + 1);
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, szType);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if ((_strcmpi(lpszVariableName, "AUTH_PASSWORD") == 0) ||
           (_strcmpi(lpszVariableName, "REMOTE_PASSWORD") == 0))
  {
    if (pClient->m_Request.m_sPassword.GetLength())
    {
      CStringA sPassword(pClient->m_Request.m_sPassword);
      DWORD dwSize = sPassword.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sPassword);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
      CW3MFCRequest::SecureEmptyString(sPassword);
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "AUTH_REALM") == 0)
  {
    if (pClient->m_Request.m_AuthorizationType == CW3MFCRequest::HTTP_AUTHORIZATION_PLAINTEXT)
    {
      CStringA sRealm(pClient->m_Request.m_sURL);
      DWORD dwSize = sRealm.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sRealm);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "CONTENT_LENGTH") == 0)
  {
    if (pClient->m_Request.m_nContentLength)
    {
      char szContentLength[10];
      sprintf_s(szContentLength, sizeof(szContentLength), "%d", pClient->m_Request.m_nContentLength);
      DWORD dwSize = static_cast<DWORD>(strlen(szContentLength) + 1);
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, szContentLength);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "CONTENT_TYPE") == 0)
  {
    if (pClient->m_Request.m_sContentType.GetLength())
    {
      CStringA sContentType(pClient->m_Request.m_sContentType);
      DWORD dwSize = sContentType.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sContentType);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "LOGON_USER") == 0)
  {
    if (pClient->m_Request.m_bLoggedOn)
    {
      CStringA sUser(pClient->m_Request.m_sUserName);
      DWORD dwSize = sUser.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sUser);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
      CW3MFCRequest::SecureEmptyString(sUser);
    }
    else if (pClient->m_pSettings->m_sUserName.GetLength())
    {
      CStringA sUser(pClient->m_pSettings->m_sUserName);
      DWORD dwSize = sUser.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sUser);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
      CW3MFCRequest::SecureEmptyString(sUser);
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else if ((_strcmpi(lpszVariableName, "HTTPS_SERVER_SUBJECT") == 0) ||
           (_strcmpi(lpszVariableName, "CERT_SERVER_SUBJECT") == 0))
  {
    CString sSubject;
    if (pClient->GetCertIssuerServerVariable(sSubject))
    {
      CStringA sAsciiSubject(sSubject);
      DWORD dwSize = sAsciiSubject.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sAsciiSubject);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else if ((_strcmpi(lpszVariableName, "HTTPS_SERVER_ISSUER") == 0) ||
           (_strcmpi(lpszVariableName, "CERT_SERVER_ISSUER") == 0))
  { 
    CString sIssuer;
    if (pClient->GetCertIssuerServerVariable(sIssuer))
    {
      CStringA sAsciiIssuer(sIssuer);
      DWORD dwSize = sAsciiIssuer.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sAsciiIssuer);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "CERT_SERIALNUMBER") == 0)
  {
    CString sSerialNumber;
    if (pClient->GetCertSerialNumberServerVariable(sSerialNumber))
    {
      CStringA sAsciiSerialNumber(sSerialNumber);
      DWORD dwSize = sAsciiSerialNumber.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sAsciiSerialNumber);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else if ((_strcmpi(lpszVariableName, "HTTPS_SECRETKEYSIZE") == 0) ||
           (_strcmpi(lpszVariableName, "CERT_SECRETKEYSIZE") == 0))
  {
    DWORD dwBits = 0;
    if (pClient->GetServerKeySizeServerVariable(dwBits))
    {
      char szBits[10];
      szBits[0] = '\0';
      sprintf_s(szBits, sizeof(szBits), "%u", dwBits);
      DWORD dwSize = static_cast<DWORD>(strlen(szBits) + 1);
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, szBits);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else if ((_strcmpi(lpszVariableName, "HTTPS_KEYSIZE") == 0) ||
           (_strcmpi(lpszVariableName, "CERT_KEYSIZE") == 0)) 
  {
    DWORD dwBits = 0;
    if (pClient->GetKeySizeServerVariable(dwBits))
    {
      char szBits[10];
      szBits[0] = '\0';
      sprintf_s(szBits, sizeof(szBits), "%u", dwBits);
      DWORD dwSize = static_cast<DWORD>(strlen(szBits) + 1);
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, szBits);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "PATH_TRANSLATED") == 0)
  {
    LPTSTR pszFilePart;
    TCHAR szTempAbsolutePath[_MAX_PATH];
    szTempAbsolutePath[0] = _T('\0');
    if (GetFullPathName(pClient->m_Request.m_sLocalFile, _MAX_PATH, szTempAbsolutePath, &pszFilePart))
    {
      #pragma warning(suppress: 6102)
      CStringA sAbsolutePath(szTempAbsolutePath);
      DWORD dwSize = sAbsolutePath.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sAbsolutePath);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
      bSuccess = FALSE;
  }
  else if (_strcmpi(lpszVariableName, "SCRIPT_NAME") == 0)
  {
    CString sScriptName(pClient->m_Request.m_sLocalFile);
    sScriptName.Replace(_T('\\'), _T('/')); //Ensure we use unix directory separators
    if (sScriptName.GetLength() && (sScriptName[0] != _T('/')))
      sScriptName = _T("/") + sScriptName;

    CStringA sAsciiScriptName(sScriptName);
    DWORD dwSize = sAsciiScriptName.GetLength() + 1;
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sAsciiScriptName);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "REQUEST_LINE") == 0)
  {
    CStringA sRequest(pClient->m_Request.m_sRequest);
    DWORD dwSize = sRequest.GetLength() + 1;
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sRequest);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "ALL_HTTP") == 0)
  {
    CString sAllHeaders;
    POSITION posMap = pClient->m_Request.m_HeaderMap.GetStartPosition();
    while (posMap)
    {
      CString sKey;
      CString sValue;
      pClient->m_Request.m_HeaderMap.GetNextAssoc(posMap, sKey, sValue);

      CString sLine;
      if (posMap)  
        sLine.Format(_T("HTTP_%s=%s\n"), sKey.GetString(), sValue.GetString());
      else
        sLine.Format(_T("HTTP_%s=%s"), sKey.GetString(), sValue.GetString());
        
      sAllHeaders += sLine;
    }
    
    CStringA sAsciiAllHeaders(sAllHeaders);
    DWORD dwSize = sAsciiAllHeaders.GetLength() + 1;
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sAsciiAllHeaders);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "ALL_RAW") == 0)
  {
    DWORD dwSize = 0;
    if (pClient->m_Request.m_pRawEntity)
      dwSize = static_cast<DWORD>(pClient->m_Request.m_pRawEntity - pClient->m_Request.m_pRawRequest + 1);
    else
      dwSize = pClient->m_Request.m_dwRawRequestSize;
    if (*lpdwBufferSize >= dwSize)
    {
      memcpy_s(lpBuffer, *lpdwBufferSize, pClient->m_Request.m_pRawRequest, dwSize);
      (static_cast<BYTE*>(lpBuffer))[dwSize] = '\0';
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strcmpi(lpszVariableName, "HTTP_VERSION") == 0)
  {
    CStringA sVersion;
    sVersion.Format("%d.%d", static_cast<int>(HIWORD(pClient->m_Request.m_dwHttpVersion)), static_cast<int>(LOWORD(pClient->m_Request.m_dwHttpVersion)));
    DWORD dwSize = static_cast<DWORD>(sVersion.GetLength() + 1);
    if (*lpdwBufferSize >= dwSize)
    {
      strcpy_s(static_cast<char*>(lpBuffer), dwSize, sVersion);
      *lpdwBufferSize = dwSize;
    }
    else
    { 
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      bSuccess = FALSE;
    }
  }
  else if (_strnicmp(lpszVariableName, "HTTP_", 5) == 0)
  {
    CString sKey(lpszVariableName);
    sKey = sKey.Right(sKey.GetLength() - 5);
    sKey.MakeUpper();
    CString sValue;
    if (pClient->m_Request.m_HeaderMap.Lookup(sKey, sValue))
    {
      CStringA sAsciiValue(sValue);
      DWORD dwSize = sAsciiValue.GetLength() + 1;
      if (*lpdwBufferSize >= dwSize)
      {
        strcpy_s(static_cast<char*>(lpBuffer), dwSize, sAsciiValue);
        *lpdwBufferSize = dwSize;
      }
      else
      { 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bSuccess = FALSE;
      }
    }
    else
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
    }
  }
  else
  {
    SetLastError(ERROR_INVALID_INDEX);
    bSuccess = FALSE;
  }

  return bSuccess;
}

BOOL CW3MFCISAPI::ReadClient(HCONN hConn, LPVOID /*lpvBuffer*/, LPDWORD lpdwSize)
{
  //nothing to read, so set the value
  *lpdwSize = 0;

  //Validate our parameters
  CW3MFCClient* pClient = static_cast<CW3MFCClient*>(hConn);
  if (pClient == nullptr)
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }  

  //We do not support reading from the client, since we supply all of the data in the EXTENSION_CONTROL_BLOCK
  SetLastError(ERROR_NO_DATA);
  return FALSE;
}

BOOL CW3MFCISAPI::WriteClient(HCONN hConn, LPVOID Buffer, LPDWORD lpdwBytes, DWORD dwSync)
{
  //Validate our parameters
  CW3MFCClient* pClient = static_cast<CW3MFCClient*>(hConn);
  if ((pClient == nullptr) || (dwSync == HSE_IO_ASYNC))
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }  
  AFXASSUME(pClient->m_pSettings != nullptr);
  
  //Assume the worst
  BOOL bSuccess = FALSE;

  try
  {
  #ifdef W3MFC_SSL_SUPPORT
    pClient->m_Socket.SendWithRetry(Buffer, *lpdwBytes, pClient->m_pSettings->m_dwWritableTimeout, *pClient);
  #else    
    pClient->m_Socket.SendWithRetry(Buffer, *lpdwBytes, pClient->m_pSettings->m_dwWritableTimeout);
  #endif //#ifdef W3MFC_SSL_SUPPORT

    pClient->m_dwDataSentViaWriteClient += *lpdwBytes;
    bSuccess = TRUE;
  }
#ifdef CWSOCKET_MFC_EXTENSIONS
  catch(CWSocketException* pEx)
  {
    //Report the error
    CString sError;
    sError.Format(_T("CW3MFCISAPI::WriteClient, Failed to send to socket, error:%d"), pEx->m_nError);
    pClient->m_pError->OnError(sError);

    //Allow callers to see what the error was
    SetLastError(pEx->m_nError);

    pEx->Delete();  
  }
#else
  catch(CWSocketException& e)
  {
    //Report the error
    CString sError;
    sError.Format(_T("CW3MFCISAPI::WriteClient, Failed to send to socket, error:%d"), e.m_nError);
    pClient->m_pError->OnError(sError);

    //Allow callers to see what the error was
    SetLastError(e.m_nError);
  }
#endif //#ifdef CWSOCKET_MFC_EXTENSIONS

  return bSuccess;
}

BOOL CW3MFCISAPI::ServerSupportFunction(HCONN hConn, DWORD dwHSERRequest, LPVOID lpvBuffer, LPDWORD lpdwSize, LPDWORD lpdwDataType)
{
  //Validate our parameters
  CW3MFCClient* pClient = static_cast<CW3MFCClient*>(hConn);
  if (pClient == nullptr)
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }  
  AFXASSUME(pClient->m_pError != nullptr);
  AFXASSUME(pClient->m_pSettings != nullptr);

  //Assume the best
  BOOL bSuccess = TRUE;

  switch (dwHSERRequest)
  {
    case HSE_REQ_CLOSE_CONNECTION:
    {
      //We support this function but do a NOOP since
      //the connection will be closed when HttpExtensionProc
      //returns normally
      break;
    }
    case HSE_REQ_GET_IMPERSONATION_TOKEN:
    {
      if (pClient->m_Request.m_hImpersonation)
        *(static_cast<HANDLE*>(lpvBuffer)) = pClient->m_Request.m_hImpersonation;
      else
      {
        SetLastError(ERROR_NO_DATA);
        bSuccess = FALSE;
      }
      break;
    }
    case HSE_REQ_GET_SSPI_INFO:
    {
      SetLastError(ERROR_NO_DATA);
      bSuccess = FALSE;
      break;
    }
    case HSE_REQ_IS_KEEP_CONN:
    {
      *(static_cast<BOOL*>(lpvBuffer)) = FALSE;
      break;
    }
    case HSE_REQ_MAP_URL_TO_PATH:
    {
      LPSTR pszURL = static_cast<LPSTR>(lpvBuffer);
      HSE_URL_MAPEX_INFO* pUMI = reinterpret_cast<HSE_URL_MAPEX_INFO*>(lpdwDataType);
      pUMI->dwFlags = 0;

      CString sURL(pszURL);
      BOOL bDirectory = FALSE;
      CW3MFCDirectory* pDirectory = nullptr;
      CString sLocalFile;
      CString sPathInfo;
      if (pClient->MapURLToLocalFilename(sURL, sLocalFile, sPathInfo, bDirectory, pDirectory, pUMI->cchMatchingURL, pUMI->cchMatchingPath))
      {
        CStringA sAsciiLocalFile(sLocalFile);
        DWORD dwSize = sAsciiLocalFile.GetLength() + 1;
        if (*lpdwSize >= dwSize)
        {
          strcpy_s(pszURL, dwSize, sAsciiLocalFile);

          if (!pDirectory->GetScript())
            pUMI->dwFlags |= HSE_URL_FLAGS_READ;
          else
          {
            pUMI->dwFlags |= HSE_URL_FLAGS_EXECUTE;
            pUMI->dwFlags |= HSE_URL_FLAGS_SCRIPT;
          }
          if (pDirectory->GetWritable())
            pUMI->dwFlags |= HSE_URL_FLAGS_WRITE;

        #ifdef W3MFC_SSL_SUPPORT
          if (pClient->m_pSettings->m_bSSL)
             pUMI->dwFlags |= HSE_URL_FLAGS_SSL;
        #endif //#ifdef W3MFC_SSL_SUPPORT
        }
        else
        {
          SetLastError(ERROR_INSUFFICIENT_BUFFER);
          bSuccess = FALSE;
        }
      }
      else
      {
        SetLastError(ERROR_NO_DATA);
        bSuccess = FALSE;
      }
      break;
    }
    case HSE_REQ_MAP_URL_TO_PATH_EX:
    {
      LPSTR pszURL = static_cast<LPSTR>(lpvBuffer);
      HSE_URL_MAPEX_INFO* pUMI = reinterpret_cast<HSE_URL_MAPEX_INFO*>(lpdwDataType);
      pUMI->dwFlags = 0;

      CString sURL(pszURL);
      BOOL bDirectory = FALSE;
      CW3MFCDirectory* pDirectory = nullptr;
      CString sLocalFile;
      CString sPathInfo;
      if (pClient->MapURLToLocalFilename(sURL, sLocalFile, sPathInfo, bDirectory, pDirectory, pUMI->cchMatchingURL, pUMI->cchMatchingPath))
      {
        CStringA sAsciiLocalFile(sLocalFile);
        strcpy_s(pUMI->lpszPath, sizeof(pUMI->lpszPath), sAsciiLocalFile);

        if (!pDirectory->GetScript())
          pUMI->dwFlags |= HSE_URL_FLAGS_READ;
        else
        {
          pUMI->dwFlags |= HSE_URL_FLAGS_EXECUTE;
          pUMI->dwFlags |= HSE_URL_FLAGS_SCRIPT;
        }
        if (pDirectory->GetWritable())
          pUMI->dwFlags |= HSE_URL_FLAGS_WRITE;

      #ifdef W3MFC_SSL_SUPPORT
        if (pClient->m_pSettings->m_bSSL)
           pUMI->dwFlags |= HSE_URL_FLAGS_SSL;
      #endif //#ifdef W3MFC_SSL_SUPPORT
      }
      else
      {
        SetLastError(ERROR_NO_DATA);
        bSuccess = FALSE;
      }
      break;
    }
    case HSE_REQ_SEND_URL:
    case HSE_REQ_SEND_URL_REDIRECT_RESP:
    {
      LPSTR pszURL = static_cast<LPSTR>(lpvBuffer);
      pClient->ReturnRedirectMessage(CString(pszURL));
      pClient->m_nHttpStatusCodeSent = 302;
      break;
    }
    case HSE_REQ_SEND_RESPONSE_HEADER:
    {
      LPSTR pszStatusString = reinterpret_cast<LPSTR>(lpvBuffer);
      LPSTR pszExtraHeaders = reinterpret_cast<LPSTR>(lpdwDataType);

      CW3MFCResponseHeader responseHdr;
      if (pszStatusString && strlen(pszStatusString))
      {
        responseHdr.AddStatus(pszStatusString);
        pClient->m_nHttpStatusCodeSent = atoi(pszStatusString);
      }
      else
      {
        responseHdr.AddStatusCode(200);
        pClient->m_nHttpStatusCodeSent = 200;
      }

      SYSTEMTIME stCurTime;
      ::GetSystemTime(&stCurTime);
      responseHdr.AddDate(stCurTime);
      responseHdr.AddServer(pClient->m_pSettings->m_sServerName);
      responseHdr.AddW3MfcAllowFields(pClient->m_pSettings->m_bAllowDeleteRequest);
      if (pszExtraHeaders && strlen(pszExtraHeaders))
        responseHdr.AddExtraHeaders(pszExtraHeaders);
      responseHdr.SetAddEntitySeparator(FALSE);

      //Send the header 
      pClient->TransmitBuffer(pClient->m_Socket, responseHdr, nullptr, 0, pClient->m_pSettings->m_dwWritableTimeout);
      break;
    }
    case HSE_REQ_SEND_RESPONSE_HEADER_EX:
    {
      HSE_SEND_HEADER_EX_INFO* pHeader = static_cast<HSE_SEND_HEADER_EX_INFO*>(lpvBuffer);

      CW3MFCResponseHeader responseHdr;
      if (pHeader->pszStatus && strlen(pHeader->pszStatus))
      {
        responseHdr.AddStatus(pHeader->pszStatus);
        pClient->m_nHttpStatusCodeSent = atoi(pHeader->pszStatus);
      }
      else
      {
        responseHdr.AddStatusCode(200);
        pClient->m_nHttpStatusCodeSent = 200;
      }

      SYSTEMTIME stCurTime;
      ::GetSystemTime(&stCurTime);
      responseHdr.AddDate(stCurTime);
      responseHdr.AddServer(pClient->m_pSettings->m_sServerName);
      responseHdr.AddW3MfcAllowFields(pClient->m_pSettings->m_bAllowDeleteRequest);
      if (pHeader->pszHeader && strlen(pHeader->pszHeader))
        responseHdr.AddExtraHeaders(pHeader->pszHeader);
      responseHdr.SetAddEntitySeparator(FALSE);

      //Send the header 
      pClient->TransmitBuffer(pClient->m_Socket, responseHdr, nullptr, 0, pClient->m_pSettings->m_dwWritableTimeout);
      break;
    }
    case HSE_REQ_TRANSMIT_FILE:
    {
    #ifdef W3MFC_SSL_SUPPORT
      BOOL bAvailable = pClient->m_pSettings->m_bSSL;
    #else
      BOOL bAvailable = TRUE;
    #endif //#ifdef W3MFC_SSL_SUPPORT
      if (bAvailable)
      {
        CW3MFCResponseHeader responseHdr;
        SYSTEMTIME stCurTime;
        ::GetSystemTime(&stCurTime);
        responseHdr.AddDate(stCurTime);
        responseHdr.AddServer(pClient->m_pSettings->m_sServerName);
        responseHdr.AddW3MfcAllowFields(pClient->m_pSettings->m_bAllowDeleteRequest);

        HSE_TF_INFO* pInfo = static_cast<HSE_TF_INFO*>(lpvBuffer);
        bSuccess = pClient->TransmitFile(pClient->m_Socket, responseHdr, pInfo);
      }
      else
      {
        SetLastError(ERROR_INVALID_INDEX);
        bSuccess = FALSE;
      }
      break;
    }
    default:
    {
      //Report the error
      CString sError;
      sError.Format(_T("CW3MFCISAPI::ServerSupportFunction, An invalid HSERequest value was specified %s, HSERequest:%u"), pClient->m_Request.m_sURL.GetString(), dwHSERRequest);
      pClient->m_pError->OnError(sError);

      SetLastError(ERROR_INVALID_INDEX);
      bSuccess = FALSE;
      break;
    }
  }

  return bSuccess;
}

void CW3MFCISAPI::ReportHttpExtensionProcError(CW3MFCClient* pClient, CW3MFCISAPIExtension& extension, DWORD dwError)
{
  //Validate our parameters
  AFXASSUME(pClient != nullptr);
  AFXASSUME(pClient->m_pError != nullptr);

  //Report the error
  CString sError;
  sError.Format(_T("CW3MFCISAPI::ReportHTTPExtensionProcError, Failed calling the function HttpExtensionProc in the ISAPI extension %s, error:%u"), extension.m_sPath.GetString(), dwError);
  pClient->m_pError->OnError(sError);
}

BOOL CW3MFCISAPI::CallHttpExtensionProc(CW3MFCClient* pClient, CW3MFCISAPIExtension& extension)
{
  USES_CONVERSION;

  //Validate our parameters
  AFXASSUME(extension.m_lpHttpExtensionProc);
  AFXASSUME(pClient != nullptr);

  //Assume the worst
  BOOL bSuccess = FALSE;

  __try
  {
    //Setup the structure
    EXTENSION_CONTROL_BLOCK ecb;
    ecb.cbSize = sizeof(EXTENSION_CONTROL_BLOCK);
    ecb.dwVersion = MAKELONG(HSE_VERSION_MINOR, HSE_VERSION_MAJOR);
    ecb.ConnID = static_cast<HCONN>(pClient);
    ecb.dwHttpStatusCode = 200;
    strcpy_s(ecb.lpszLogData, sizeof(ecb.lpszLogData), "");
    //Note we use the old style T2A macros here instead of the CT2A or CStringA classes
    //because we are using SEH in this function and CT2A and CStringA have destructors which
    //causes warning C4509
    ecb.lpszMethod = T2A(const_cast<LPTSTR>(pClient->m_Request.m_sVerb.GetString()));
    ecb.lpszQueryString = T2A(const_cast<LPTSTR>(pClient->m_Request.m_sRawExtra.GetString()));
    ecb.lpszPathInfo = T2A(const_cast<LPTSTR>(pClient->m_Request.m_sPathInfo.GetString()));
    ecb.lpszPathTranslated = T2A(const_cast<LPTSTR>(pClient->m_Request.m_sLocalFile.GetString()));
    ecb.cbTotalBytes = pClient->m_Request.m_dwRawEntitySize;
    ecb.cbAvailable = ecb.cbTotalBytes;
    ecb.lpbData = pClient->m_Request.m_pRawEntity;
    ecb.lpszContentType = T2A(const_cast<LPTSTR>(pClient->m_Request.m_sContentType.GetString()));
    ecb.GetServerVariable = GetServerVariable;
    ecb.WriteClient = WriteClient;
    ecb.ReadClient = ReadClient;
    ecb.ServerSupportFunction = ServerSupportFunction;

    //Finally call the function with the structure
    DWORD dwSuccess = extension.m_lpHttpExtensionProc(&ecb);
    bSuccess = (dwSuccess == HSE_STATUS_SUCCESS);

    if (!bSuccess)
      ReportHttpExtensionProcError(pClient, extension, dwSuccess);
  }
  __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) //Note W3MFC only handles access violations from an ISAPI dll, everything else is considered critical
  {
    bSuccess = FALSE;
  }

  return bSuccess;
}

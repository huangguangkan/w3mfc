/*
Module : W3MFCRequest.cpp
Purpose: Implementation for the CW3MFCRequest class
Created: PJN / 30-09-1999
History: PJN / 19-02-2006 1. Replaced all calls to ZeroMemory and CopyMemory with memset and memcpy
         PJN / 12-01-2007 1. Optimized CW3MFCRequest constructor code
         PJN / 19-11-2007 1. Updated copyright details.
                          2. CHttpRequest class has been renamed CW3MFCRequest
         PJN / 16-03-2014 1. Updated copyright details
         PJN / 17-07-2016 1. Added a new CW3MFCRequestParams class which provides for parsing of
                          m_sRawExtra into a key value collection. This new class is called 
                          CW3MFCRequestParams and is based on the ATL Server class "CHttpRequestParams"
                          class.
         PJN / 24-07-2016 1. Reworked CW3MFCRequestParams class to be derived from CMapStringToString
                          2. Removed call to m_HeaderMap.InitHashTable in CW3MFCRequest constructor.
         PJN / 28-04-2017 1. Updated the code to compile cleanly using /permissive-.

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
#include "W3MFCRequest.h"

#ifndef __ATLUTIL_H__
#pragma message("To avoid this message, please put atlutil.h in your pre compiled header (usually stdafx.h)")
#include <atlutil.h>
#endif //#ifndef __ATLUTIL_H__



//////////////// Macros ///////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG


//////////////// Implementation ///////////////////////////////////////////////

CW3MFCRequest::CW3MFCRequest() : m_Verb(HTTP_VERB_UNKNOWN),
                                 m_dwHttpVersion(0),
                                 m_bIfModifiedSincePresent(FALSE),
                                 m_AuthorizationType(HTTP_AUTHORIZATION_ANONYMOUS),
                                 m_pRawRequest(nullptr),
                                 m_dwRawRequestSize(0),
                                 m_pRawEntity(nullptr),
                                 m_dwRawEntitySize(0),
                                 m_nContentLength(0),
                                 m_bKeepAlive(FALSE),
                                 m_bLoggedOn(FALSE),
                                 m_hImpersonation(nullptr)
{
  m_IfModifiedSince = { 0 };
  m_ClientAddress = { 0 };
}

CW3MFCRequest& CW3MFCRequest::operator=(const CW3MFCRequest& request)
{
  //Free up any existing memory before we copy over
  if (m_pRawRequest)
  {
    delete [] m_pRawRequest;
    m_pRawRequest = nullptr;
  }

  m_nContentLength          = request.m_nContentLength;
  m_sRequest                = request.m_sRequest;
  m_Verb                    = request.m_Verb;
  m_sVerb                   = request.m_sVerb;
  m_ClientAddress           = request.m_ClientAddress;
  m_sURL                    = request.m_sURL;
  m_sRawURL                 = request.m_sRawURL;
  m_sPathInfo               = request.m_sPathInfo;
  m_sExtra                  = request.m_sExtra;
  m_sRawExtra               = request.m_sRawExtra;
  m_dwHttpVersion           = request.m_dwHttpVersion;
  m_bIfModifiedSincePresent = request.m_bIfModifiedSincePresent; 
  memcpy_s(&m_IfModifiedSince, sizeof(m_IfModifiedSince), &request.m_IfModifiedSince, sizeof(request.m_IfModifiedSince));
  m_AuthorizationType       = request.m_AuthorizationType;
  m_sUserName               = request.m_sUserName;
  m_sPassword               = request.m_sPassword;
  m_sRemoteHost             = request.m_sRemoteHost;
  m_sContentType            = request.m_sContentType;
  m_bKeepAlive              = request.m_bKeepAlive;
  m_sLocalFile              = request.m_sLocalFile;

  if (request.m_pRawRequest)
  {
    m_pRawRequest = new BYTE[request.m_dwRawRequestSize];
    m_dwRawRequestSize  = request.m_dwRawRequestSize;
    m_dwRawEntitySize   = request.m_dwRawEntitySize;
    memcpy_s(m_pRawRequest, request.m_dwRawRequestSize, request.m_pRawRequest, m_dwRawRequestSize);
    if (m_dwRawEntitySize)
      m_pRawEntity = m_pRawRequest + (m_dwRawRequestSize - m_dwRawEntitySize);
    else
      m_pRawEntity = nullptr;
  }
  else
  {
    m_pRawRequest = nullptr;
    m_dwRawRequestSize = 0;
    m_pRawEntity = nullptr;
    m_dwRawEntitySize = 0;
  }

  m_HeaderMap.RemoveAll();
  m_HeaderMap.InitHashTable(request.m_HeaderMap.GetHashTableSize());
  POSITION posMap = request.m_HeaderMap.GetStartPosition();
  while (posMap)
  {
    CString sKey;
    CString sValue;
    request.m_HeaderMap.GetNextAssoc(posMap, sKey, sValue);
    m_HeaderMap.SetAt(sKey, sValue);
  }

  m_hImpersonation = request.m_hImpersonation;

  return *this;
}


void CW3MFCRequestParams::Parse(_In_ LPCTSTR pszString)
{
  //Work on a local variabale so that the method parameter can be declared const
  CString sLocalQueryString(pszString);
  LPTSTR pszQueryString = sLocalQueryString.GetBuffer();

  //Reset the map prior to parsing
  RemoveAll();

  while (pszQueryString && *pszQueryString)
  {
    LPTSTR szUrlCurrent = pszQueryString;
    LPTSTR szName = szUrlCurrent;
    LPCTSTR szPropValue = nullptr;

    while (*pszQueryString)
    {
      if (*pszQueryString == _T('='))
      {
        pszQueryString++;
        break;
      }
      if (*pszQueryString == _T('&'))
        break;
      if (*pszQueryString == _T('+'))
        *szUrlCurrent = _T(' ');
      else if (*pszQueryString == _T('%'))
      {
        //if there is a % without two characters at the end of the url we skip it
        if (*(pszQueryString + 1) && *(pszQueryString + 2))
        {
          int nFirstDigit = ATL::AtlHexValue(static_cast<char>(pszQueryString[1]));
          int nSecondDigit = ATL::AtlHexValue(static_cast<char>(pszQueryString[2]));
          if (nFirstDigit < 0 || nSecondDigit < 0)
            break;
          *szUrlCurrent = static_cast<CHAR>(16 * nFirstDigit + nSecondDigit);
          pszQueryString += 2;
        }
        else
          *szUrlCurrent = _T('\0');
      }
      else
        *szUrlCurrent = *pszQueryString;

      pszQueryString++;
      szUrlCurrent++;
    }

    if (*szUrlCurrent == _T('&'))
    {
      *szUrlCurrent++ = _T('\0');
      pszQueryString++;
      szPropValue = _T("");
    }
    else
    {
      if (*szUrlCurrent)
        *szUrlCurrent++ = _T('\0');

      //we have the property name
      szPropValue = szUrlCurrent;
      while (*pszQueryString && *pszQueryString != _T('#'))
      {
        if (*pszQueryString == _T('&'))
        {
          pszQueryString++;
          break;
        }
        if (*pszQueryString == _T('+'))
          *szUrlCurrent = _T(' ');
        else if (*pszQueryString == _T('%'))
        {
          //if there is a % without two characters at the end of the url we skip it
          if (*(pszQueryString + 1) && *(pszQueryString + 2))
          {
            int nFirstDigit = ATL::AtlHexValue(static_cast<char>(pszQueryString[1]));
            int nSecondDigit = ATL::AtlHexValue(static_cast<char>(pszQueryString[2]));
            if ((nFirstDigit < 0) || (nSecondDigit < 0))
              break;
            *szUrlCurrent = static_cast<CHAR>(16 * nFirstDigit + nSecondDigit);
            pszQueryString += 2;
          }
          else
            *szUrlCurrent = _T('\0');
        }
        else
          *szUrlCurrent = *pszQueryString;
        pszQueryString++;
        szUrlCurrent++;
      }
      //we have the value
      *szUrlCurrent = _T('\0');
      szUrlCurrent++;
    }
    sLocalQueryString.ReleaseBuffer();

    SetAt(szName, szPropValue);
  }
}

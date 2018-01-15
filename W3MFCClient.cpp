/*
Module : W3MFCClient.cpp
Purpose: Implementation for the CW3MFCClient class
Created: PJN / 22-04-1999
History  PJN / 19-02-2006 1. Replaced all calls to ZeroMemory and CopyMemory with memset and memcpy
         PJN / 27-06-2006 1. Combined the functionality of the _W3MFC_DATA class into the main CW3MFCClient class.
         PJN / 19-11-2007 1. CHttpClient class has been renamed to CW3MFCClient
         PJN / 18-02-2008 1. Fixed a memory leak in CW3MFCClient::_TransmitFile
         PJN / 31-05-2008 1. Code now compiles cleanly using Code Analysis (/analyze)
         PJN / 23-05-2009 1. Reworked all token parsing code to use CString::Tokenize
         PJN / 07-09-2009 1. Fixed a debug mode ASSERT when calling TRACE in CW3MFCClient::PostLog
         PJN / 16-03-2014 1. Updated copyright details
         PJN / 17-07-2016 1. Reimplemented CW3MFCClient::UTF8ToCString to use the MultiByteToWideChar Win32 API
                          2. Replaced CW3MFCClient::HexToInt implementation with calls to ATL::AtlHexValue
         PJN / 17-08-2016 1. Reworked CW3MFCClient class to be inherited from SSLWrappers::CSocket. This allows 
                          for easier overloading of the various SSLWrappers class framework virtual functions
         PJN / 28-04-2017 1. Updated the code to compile cleanly using /permissive-.
         PJN / 13-08-2017 1. Fixed an incorrect ASSERT in CW3MFCClient::ReturnErrorMessage
                          2. Fixed a compile error in CW3MFCClient::PostLog when CWSOCKET_MFC_EXTENSIONS is 
                          defined.
                          3. Replaced CString::operator LPC*STR() calls throughout the codebase with 
                          CString::GetString calls

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
#include "W3MFC.h"
#include "W3MFCResponseHeader.h"
#include "W3MFCClient.h"

#ifndef __ATLENC_H__
#pragma message("To avoid this message, please put atlenc.h in your pre compiled header (usually stdafx.h)")
#include <atlenc.h>
#endif //#ifndef __ATLENC_H__

#ifndef __ATLUTIL_H__
#pragma message("To avoid this message, please put atlutil.h in your pre compiled header (usually stdafx.h)")
#include <atlutil.h>
#endif //#ifndef __ATLUTIL_H__


//////////////// Macros / Defines /////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG


//////////////// Implementation ///////////////////////////////////////////////

CW3MFCClient::CW3MFCClient() : m_pError(nullptr),
                             #ifndef W3MFC_NO_ISAPI_SUPPORT
                               m_dwDataSentViaWriteClient(0),
                               m_nHttpStatusCodeSent(0),
                             #endif //#ifndef W3MFC_NO_ISAPI_SUPPORT
                               m_pSettings(nullptr)
{
  LoadHTMLResources();
}

CW3MFCClient::~CW3MFCClient()
{
}

BOOL CW3MFCClient::AllowThisConnection()
{
  return TRUE;
}

#ifdef W3MFC_SSL_SUPPORT
BOOL CW3MFCClient::SetupSSL()
{
  //Hook up the socket to the SSL class
  Attach(m_Socket);

  //Do the SSL handshake
  SECURITY_STATUS status = SSLAccept(FALSE);
  if (status != SEC_E_OK)
  {
    //Report the error
    CString sError;
    sError.Format(_T("CW3MFCClient::SetupSSL, Failed to perform SSL handshake, GetLastError:%08X"), status);
    m_pError->OnError(sError);
    return FALSE;
  }

  return TRUE;
}
#endif //#ifdef W3MFC_SSL_SUPPORT

BOOL CW3MFCClient::Run(_In_ const CThreadPoolRequest& request)
{
  //Validate our parameters
  ASSERT(request.m_pData != nullptr);
  CW3MFCThreadPoolRequest* pW3MFCRequest = static_cast<CW3MFCThreadPoolRequest*>(request.m_pData);
  AFXASSUME(pW3MFCRequest != nullptr);

  //Hive away the parameters in member variables
  SOCKET clientSocket = pW3MFCRequest->m_ClientSocket.Detach();
  m_Socket.Attach(clientSocket);
  memcpy_s(&m_Request.m_ClientAddress, sizeof(m_Request.m_ClientAddress), &pW3MFCRequest->m_ClientAddress, sizeof(pW3MFCRequest->m_ClientAddress));

  //Call the helper function which does all of the work
  HandleClient();

  //Close down the connection
  m_Socket.Close();

#ifdef W3MFC_SSL_SUPPORT
  Detach();
#endif //#ifdef W3MFC_SSL_SUPPORT

  //Tidy up our heap memory after ourselves
  delete pW3MFCRequest;

  //Reset the request data
  m_Request = CW3MFCRequest();

  return TRUE;
}

void CW3MFCClient::HandleClient()
{
  //Validate our parameters
  AFXASSUME(m_pSettings != nullptr);

  //Do the reverse DNS lookup if configured to do so
  m_Request.m_sRemoteHost.Empty();
  if (m_pSettings->m_bDNSLookup)
  {
    char szName[NI_MAXHOST];
    szName[0] = '\0';
    if (getnameinfo(reinterpret_cast<const SOCKADDR*>(&m_Request.m_ClientAddress), sizeof(m_Request.m_ClientAddress), szName, NI_MAXHOST, nullptr, 0, 0) == 0)
      m_Request.m_sRemoteHost = szName;
  }

  //Should we allow this client to connect
  if (!AllowThisConnection())
  {
    ReturnErrorMessage(400); //Bad Request
    return;    
  }

  //Create the SSL connection if required
#ifdef W3MFC_SSL_SUPPORT
  if (m_pSettings->m_bSSL)
  {
    if (!SetupSSL())
      return;
  }
#endif //#ifdef W3MFC_SSL_SUPPORT

  //Use a Win32 event notification on the socket
  ::CEvent dataEvent;
  int nError = WSAEventSelect(m_Socket, dataEvent, FD_READ | FD_CLOSE);
  if (nError == SOCKET_ERROR)
  {
    DWORD dwError = ::GetLastError();

    //Report the error
    CString sError;
    sError.Format(_T("CW3MFCClient::HandleClient, Failed in call to WSAEventSelect, GetLastError:%u"), dwError);
    m_pError->OnError(sError);

    return;
  }

  //Also create the waitable timer
  CWaitableTimer dataTimer;
  if (!dataTimer.Create(TRUE))
  {
    DWORD dwError = ::GetLastError();

    //Report the error
    CString sError;
    sError.Format(_T("CW3MFCClient::HandleClient, Failed in call to create waitable timer, GetLastError:%u"), dwError);
    m_pError->OnError(sError);

    return;
  }
  if (!dataTimer.SetOnceOffRelative(m_pSettings->m_dwIdleClientTimeout))
  {
    DWORD dwError = ::GetLastError();

    //Report the error
    CString sError;
    sError.Format(_T("CW3MFCClient::HandleClient, Failed in call to set waitable timer, GetLastError:%u"), dwError);
    m_pError->OnError(sError);

    return;
  }

#ifdef W3MFC_SSL_SUPPORT
  //Read the client request
  BOOL bReadRequest = m_Socket.ReadRequest(m_Request, m_pSettings->m_dwIdleClientTimeout, 4096, *this, *this, dataTimer, m_StopEvent, dataEvent);
#else
  BOOL bReadRequest = m_Socket.ReadRequest(m_Request, m_pSettings->m_dwIdleClientTimeout, 4096, *this, dataTimer, m_StopEvent, dataEvent);
#endif //#ifdef W3MFC_SSL_SUPPORT
  if (bReadRequest)
  {
    //Parse the client request
    if (ParseRequest())
    {
      //Impersonate the client credentials if authorization type is PLAINTEXT
      ATL::CHandle hImpersonation;
      BOOL bLoggedOn = FALSE;
      if (m_Request.m_AuthorizationType == CW3MFCRequest::HTTP_AUTHORIZATION_PLAINTEXT && m_pSettings->m_bPerformPassthroughAuthentication)
      {
        LPTSTR pszUser = m_Request.m_sUserName.GetBuffer();
        LPTSTR pszPassword = m_Request.m_sPassword.GetBuffer();
        bLoggedOn = LogonUser(pszUser, nullptr, pszPassword, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &hImpersonation.m_h);
        if (bLoggedOn)
        {
          m_Request.m_bLoggedOn = ImpersonateLoggedOnUser(hImpersonation);
          if (!m_Request.m_bLoggedOn)
          {
            //Report the error
            CString sError;
            sError.Format(_T("CW3MFCClient::HandleClient, Failed to impersonate using user name: %s, GetLastError:%u"), pszUser, ::GetLastError());
            m_pError->OnError(sError);
          }
        }
        else
        {
          //Report the error
          CString sError;
          sError.Format(_T("CW3MFCClient::HandleClient, Failed to logon using user name: %s, GetLastError:%u"), pszUser, ::GetLastError());
          m_pError->OnError(sError);
        }
        m_Request.m_sUserName.ReleaseBuffer();
        m_Request.m_sPassword.ReleaseBuffer();
      } 

      m_Request.m_hImpersonation = hImpersonation;

      if (m_Request.m_AuthorizationType == CW3MFCRequest::HTTP_AUTHORIZATION_ANONYMOUS && !m_pSettings->m_bAllowAnonymous)
      {
        //Return an unauthorized message if some form of authentication is enabled
        if (m_pSettings->m_bAllowBasicAuthentication)
          ReturnUnauthorizedMessage(m_Request.m_sURL);
        else
        {
          //Report the error
          m_pError->OnError(_T("CW3MFCClient::HandleClient, Anonymous access is disabled in addition to all authentication mechanisms, All requests will fail!!"));

          ReturnErrorMessage(500); //Internal server error
        }
      }
      else
      {
        if (m_Request.m_Verb == CW3MFCRequest::HTTP_VERB_GET || m_Request.m_Verb == CW3MFCRequest::HTTP_VERB_HEAD || m_Request.m_Verb == CW3MFCRequest::HTTP_VERB_POST)
        {
          if (!PreHandleGetPostHead()) //Allow derived classes to handle GET, HEAD or POST
          {
            BOOL bDirectory = FALSE;
            CW3MFCDirectory* pDirectory = nullptr;
            DWORD dwMatchingURL = 0;
            DWORD dwMatchingPath = 0;
            if (MapURLToLocalFilename(m_Request.m_sURL, m_Request.m_sLocalFile, m_Request.m_sPathInfo, bDirectory, pDirectory, dwMatchingURL, dwMatchingPath))
            {
              AFXASSUME(pDirectory != nullptr);
              pDirectory->HandleDirectory(this, bDirectory);
            }
            else
              ReturnErrorMessage(404); //Not Found
          }
        }
        else if (m_pSettings->m_bAllowDeleteRequest && m_Request.m_Verb == CW3MFCRequest::HTTP_VERB_DELETE)
        {
          //By default, only allow deletion of a file if we are using authorization
          if (m_Request.m_AuthorizationType != CW3MFCRequest::HTTP_AUTHORIZATION_ANONYMOUS)
          {
            CString sLocalFile;
            BOOL bDirectory = FALSE;
            CW3MFCDirectory* pDirectory = nullptr;
            DWORD dwMatchingURL = 0;
            DWORD dwMatchingPath = 0;
            if (MapURLToLocalFilename(m_Request.m_sURL, m_Request.m_sLocalFile, m_Request.m_sPathInfo, bDirectory, pDirectory, dwMatchingURL, dwMatchingPath) && !bDirectory && pDirectory->GetWritable())
            {
              if (DeleteFile(m_Request.m_sLocalFile))
                ReturnFileDeletedOkMessage(m_Request.m_sLocalFile);
              else
              {
                if (::GetLastError() == ERROR_ACCESS_DENIED && !bDirectory)
                  ReturnUnauthorizedMessage(m_Request.m_sURL);
                else 
                  ReturnErrorMessage(500); //Internal server error
              }
            }
            else
              ReturnErrorMessage(404); //Not Found
          }
          else if (m_pSettings->m_bAllowBasicAuthentication)
            ReturnUnauthorizedMessage(m_Request.m_sURL); //Not authorized
          else
            ReturnErrorMessage(404); //Not Found
        }
        else
          ReturnErrorMessage(501); //Not implemented
      }

      //Restore our usual security priviledges
      if (m_Request.m_AuthorizationType == CW3MFCRequest::HTTP_AUTHORIZATION_PLAINTEXT)
      {
        //Revert to the usual security settings
        RevertToSelf();
      }
    }
    else
      ReturnErrorMessage(400); //Bad Request
  }

  //Reset the request data before we exit
  m_Request = CW3MFCRequest();
}

BOOL CW3MFCClient::ParseSimpleRequestLine(_In_ const CString& sLine)
{
  //What will be the return value from this function (assume the worst)
  BOOL bSuccess = FALSE;

  //First parse out the VERB
  static const TCHAR* pszTokens = _T(" ");
  int nTokenPosition = 0;
  CString sVerb(sLine.Tokenize(pszTokens, nTokenPosition));
  if (sVerb.GetLength())
  {
    m_Request.m_sVerb = sVerb;
    if (sVerb.CompareNoCase(_T("GET")) == 0)
      m_Request.m_Verb = CW3MFCRequest::HTTP_VERB_GET;
    else if (sVerb.CompareNoCase(_T("POST")) == 0)
      m_Request.m_Verb = CW3MFCRequest::HTTP_VERB_POST;
    else if (sVerb.CompareNoCase(_T("HEAD")) == 0)
      m_Request.m_Verb = CW3MFCRequest::HTTP_VERB_HEAD;
    else if (sVerb.CompareNoCase(_T("PUT")) == 0)
      m_Request.m_Verb = CW3MFCRequest::HTTP_VERB_PUT;
    else if (sVerb.CompareNoCase(_T("LINK")) == 0)
      m_Request.m_Verb = CW3MFCRequest::HTTP_VERB_LINK;
    else if (sVerb.CompareNoCase(_T("DELETE")) == 0)
      m_Request.m_Verb = CW3MFCRequest::HTTP_VERB_DELETE;
    else if (sVerb.CompareNoCase(_T("UNLINK")) == 0)
      m_Request.m_Verb = CW3MFCRequest::HTTP_VERB_UNLINK;
    else
      m_Request.m_Verb = CW3MFCRequest::HTTP_VERB_UNKNOWN;

    //Parse out the URL
    CString sURL(sLine.Tokenize(pszTokens, nTokenPosition));
    if (sURL.GetLength())
    {
      m_Request.m_sRawURL = sURL;
      m_Request.m_sURL = UTF8ToCString(URLDecode(sURL)); //Convert any embedded escape sequences to their unencoded format 
                                                         //as well as handling UTF8 input data

      //Handle the Search path i.e everything after the ?
      int nQuestion = m_Request.m_sURL.Find(_T('?'));
      if (nQuestion != -1)
      {
        m_Request.m_sExtra = m_Request.m_sURL.Right(m_Request.m_sURL.GetLength() - nQuestion - 1);
        m_Request.m_sURL = m_Request.m_sURL.Left(nQuestion);
      }
      nQuestion = m_Request.m_sRawURL.Find(_T('?'));
      if (nQuestion != -1)
      {
        m_Request.m_sRawExtra = m_Request.m_sRawURL.Right(m_Request.m_sRawURL.GetLength() - nQuestion - 1);

      #ifdef _DEBUG
        //Exercise the CW3MFCRequestParams class in debug builds
        CW3MFCRequestParams params;
        params.Parse(m_Request.m_sRawExtra);
        CString sValue1;
        BOOL bFound = params.Lookup(_T("Key1"), sValue1);
        UNREFERENCED_PARAMETER(bFound);
        POSITION pos = params.GetStartPosition();
        while (pos != nullptr)
        {
          CString sKey;
          CString sValue2;
          params.GetNextAssoc(pos, sKey, sValue2);
        }
      #endif //#ifdef _DEBUG
      }
      else
        m_Request.m_sRawExtra.Empty();

      //Parse out the HTTP version
      CString sVersion(sLine.Tokenize(pszTokens, nTokenPosition));
      if (sVersion.GetLength())
      {
        if (sVersion.Find(_T("HTTP/")) == 0)
        {
          static const TCHAR* pszVersionTokens = _T(".");
          CString sVersionToTokenize(sVersion.Right(sVersion.GetLength() - 5));
          int nVersionTokenPosition = 0;
          CString sMajorVersion(sVersionToTokenize.Tokenize(pszVersionTokens, nVersionTokenPosition));
          if (sMajorVersion.GetLength())
          {
            WORD wMajorVersion = static_cast<WORD>(_ttoi(sMajorVersion));
            CString sMinorVersion(sVersionToTokenize.Tokenize(pszVersionTokens, nVersionTokenPosition));
            if (sMinorVersion.GetLength())
            {
              WORD wMinorVersion = static_cast<WORD>(_ttoi(sMinorVersion));
              m_Request.m_dwHttpVersion = MAKELONG(wMinorVersion, wMajorVersion);
              bSuccess = TRUE;
            }
          }
        }
      }
      else
      {
        //No version included in the request, so set it to HTTP v0.9
        m_Request.m_dwHttpVersion = MAKELONG(9, 0);
        bSuccess = m_Request.m_Verb == CW3MFCRequest::HTTP_VERB_GET; //"GET" is only allowed with HTTP v0.9
      }  
    }
  }

  return bSuccess;
}

_Success_(return != FALSE)
BOOL CW3MFCClient::ParseWeekDay(_In_z_ LPCTSTR pszToken, _Out_ WORD& nWeekDay)
{
  BOOL bSuccess = TRUE;
  if (_tcsicmp(pszToken, _T("Sun")) == 0 || _tcsicmp(pszToken, _T("Sunday")) == 0)
    nWeekDay = 0;
  else if (_tcsicmp(pszToken, _T("Mon")) == 0 || _tcsicmp(pszToken, _T("Monday")) == 0)
    nWeekDay = 1;
  else if (_tcsicmp(pszToken, _T("Tue")) == 0 || _tcsicmp(pszToken, _T("Tuesday")) == 0)
    nWeekDay = 2;
  else if (_tcsicmp(pszToken, _T("Wed")) == 0 || _tcsicmp(pszToken, _T("Wednesday")) == 0)
    nWeekDay = 3;
  else if (_tcsicmp(pszToken, _T("Thu")) == 0 || _tcsicmp(pszToken, _T("Thursday")) == 0)
    nWeekDay = 4;
  else if (_tcsicmp(pszToken, _T("Fri")) == 0 || _tcsicmp(pszToken, _T("Friday")) == 0)
    nWeekDay = 5;
  else if (_tcsicmp(pszToken, _T("Sat")) == 0 || _tcsicmp(pszToken, _T("Saturday")) == 0)
    nWeekDay = 6;
  else
    bSuccess = FALSE;
  return bSuccess;
}

_Success_(return != FALSE)
BOOL CW3MFCClient::ParseMonth(_In_z_ LPCTSTR pszToken, _Out_ WORD& nMonth)
{
  BOOL bSuccess = TRUE;
  if (_tcsicmp(pszToken, _T("Jan")) == 0)
    nMonth = 1;
  else if (_tcsicmp(pszToken, _T("Feb")) == 0)
    nMonth = 2;
  else if (_tcsicmp(pszToken, _T("Mar")) == 0)
    nMonth = 3;
  else if (_tcsicmp(pszToken, _T("Apr")) == 0)
    nMonth = 4;
  else if (_tcsicmp(pszToken, _T("May")) == 0)
    nMonth = 5;
  else if (_tcsicmp(pszToken, _T("Jun")) == 0)
    nMonth = 6;
  else if (_tcsicmp(pszToken, _T("Jul")) == 0)
    nMonth = 7;
  else if (_tcsicmp(pszToken, _T("Aug")) == 0)
    nMonth = 8;
  else if (_tcsicmp(pszToken, _T("Sep")) == 0)
    nMonth = 9;
  else if (_tcsicmp(pszToken, _T("Oct")) == 0)
    nMonth = 10;
  else if (_tcsicmp(pszToken, _T("Nov")) == 0)
    nMonth = 11;
  else if (_tcsicmp(pszToken, _T("Dec")) == 0)
    nMonth = 12;
  else
    bSuccess = FALSE;
  return bSuccess;
}

_Success_(return != FALSE)
BOOL CW3MFCClient::ParseDate(_In_ const CString& sField, _Out_ SYSTEMTIME& time)
{
  //This method understands RFC 1123, RFC 850 and asctime formats

  //What will be the return value from this function (assume the worst)
  BOOL bSuccess = FALSE;

  //Http times never include a millisecond field, so just set it to zero
  time.wMilliseconds = 0;

  int nLength = sField.GetLength();
  if (nLength > 5)
  {
    TCHAR cThirdCharacter = sField[3];
    if (cThirdCharacter == _T(',')) //Parsing a RFC 1123 format date
    {
      //First the weekday
      static const TCHAR* pszTokens = _T(", :");
      int nTokenPosition = 0;
      CString sToken(sField.Tokenize(pszTokens, nTokenPosition));
      if (nTokenPosition == -1)
        return FALSE;
      bSuccess = ParseWeekDay(sToken, time.wDayOfWeek);

      //Then the day of the month
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wDay = static_cast<WORD>(_ttoi(sToken));

      //Then the month
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      bSuccess = bSuccess && ParseMonth(sToken, time.wMonth);

      //And the year
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wYear = static_cast<WORD>(_ttoi(sToken));

      //And the hour
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wHour = static_cast<WORD>(_ttoi(sToken));

      //And the minute
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wMinute = static_cast<WORD>(_ttoi(sToken));

      //And the second
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wSecond = static_cast<WORD>(_ttoi(sToken));
    }
    else if (cThirdCharacter == _T(' ')) //Parsing an asctime format date
    {
      //First the weekday
      static const TCHAR* pszTokens = _T(", :");
      int nTokenPosition = 0;
      CString sToken(sField.Tokenize(pszTokens, nTokenPosition));
      if (nTokenPosition == -1)
        return FALSE;
      bSuccess = ParseWeekDay(sToken, time.wDayOfWeek);

      //Then the month
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      bSuccess = bSuccess && ParseMonth(sToken, time.wMonth);

      //Then the day of the month
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wDay = static_cast<WORD>(_ttoi(sToken));

      //And the hour
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wHour = static_cast<WORD>(_ttoi(sToken));

      //And the minute
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wMinute = static_cast<WORD>(_ttoi(sToken));

      //And the second
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wSecond = static_cast<WORD>(_ttoi(sToken));

      //And the year
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wYear = static_cast<WORD>(_ttoi(sToken));
    }
    else //Must be a RFC 850 format date
    {
      //First the weekday
      static const TCHAR* pszTokens = _T(", :-");
      int nTokenPosition = 0;
      CString sToken(sField.Tokenize(pszTokens, nTokenPosition));
      if (nTokenPosition == -1)
        return FALSE;
      bSuccess = ParseWeekDay(sToken, time.wDayOfWeek);

      //Then the day of the month
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wDay = static_cast<WORD>(_ttoi(sToken));

      //Then the month
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      bSuccess = bSuccess && ParseMonth(sToken, time.wMonth);

      //And the year (2 Digits only, so make some intelligent assumptions)
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wYear = static_cast<WORD>(_ttoi(sToken));
      if (time.wYear < 50)
        time.wYear += 2000;
      else if (time.wYear < 100)
        time.wYear += 1900; 

      //And the hour
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wHour = static_cast<WORD>(_ttoi(sToken));

      //And the minute
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wMinute = static_cast<WORD>(_ttoi(sToken));

      //And the second
      sToken = sField.Tokenize(pszTokens, nTokenPosition);
      if (nTokenPosition == -1)
        return FALSE;
      time.wSecond = static_cast<WORD>(_ttoi(sToken));
    }
  }

  return bSuccess;
}

BOOL CW3MFCClient::ParseAuthorizationLine(_In_ const CString& sField)
{
  //Validate our parameters
  AFXASSUME(m_pSettings != nullptr);

  //What will be the return value from this function (assume the best)
  BOOL bSuccess = TRUE;

  static const char* pszTokens = " ";
  int nTokenPosition = 0;
  CStringA sAsciiField(sField);
  CStringA sToken(sAsciiField.Tokenize(pszTokens, nTokenPosition));
  if (sToken.GetLength())
  {
    //Parse out the base64 encoded username and password if we are doing Basic authentication
    if ((sToken.CompareNoCase("basic") == 0) && m_pSettings->m_bAllowBasicAuthentication)
    {
      //Assume the worst
      bSuccess = FALSE;
    
      //Move to the base64 encoded data after the text "Basic"
      sToken = sAsciiField.Tokenize(pszTokens, nTokenPosition);
      if (sToken.GetLength())
      {
        //Decode the base64 string passed to us
        int nEncodedLength = sToken.GetLength();
        int nDecodedLength = ATL::Base64DecodeGetRequiredLength(nEncodedLength);
        ATL::CHeapPtr<BYTE> szOutput;
        if (szOutput.Allocate(nDecodedLength + 1))
        {
          int nOutputLength = nDecodedLength;
          if (ATL::Base64Decode(sToken, nEncodedLength, szOutput.m_pData, &nOutputLength))
          {
            //Null terminate the decoded data
            szOutput.m_pData[nOutputLength] = '\0';

            //Extract the username and password from the decoded data
            CString sOutput(szOutput);
            int nColon = sOutput.Find(_T(":"));
            if (nColon != -1)
            {
              m_Request.m_AuthorizationType = CW3MFCRequest::HTTP_AUTHORIZATION_PLAINTEXT;
              m_Request.m_sUserName = sOutput.Left(nColon);
              m_Request.m_sPassword = sOutput.Right(sOutput.GetLength() - nColon - 1);
              bSuccess = TRUE;
            }
          }
        }
      }
    }
  }

  return bSuccess;
}

BOOL CW3MFCClient::ParseRequestLine(_In_ const CString& sCurrentLine, _In_ BOOL bFirstLine, _In_ const CString& sField, _In_ const CString& sValue)
{
  //Assume the worst
  BOOL bSuccess = FALSE;

  if (bFirstLine)
  {
    //Handle the first line
    m_Request.m_sRequest = sCurrentLine;
    bSuccess = ParseSimpleRequestLine(sCurrentLine);
  }
  else
  {
    bSuccess = TRUE;

    //Also add the header line to the header map
    CString sKey(sField);
    sKey.MakeUpper();
    m_Request.m_HeaderMap.SetAt(sKey, sValue);

    //Handle any other request headers  
    if (sField.CompareNoCase(_T("If-Modified-Since")) == 0)
    {
      //Handle the If-Modified-Since header
      SYSTEMTIME time;
      if (ParseDate(sValue, time))
      {
        m_Request.m_bIfModifiedSincePresent = TRUE; 
        memcpy_s(&m_Request.m_IfModifiedSince, sizeof(m_Request.m_IfModifiedSince), &time, sizeof(time));
      }
    }
    else if (sField.CompareNoCase(_T("Authorization")) == 0)
    {
      //Handle the Authorization header
      bSuccess = ParseAuthorizationLine(sValue);
    }
    else if (sField.CompareNoCase(_T("Content-Type")) == 0)
      m_Request.m_sContentType = sValue;
    else if (sField.CompareNoCase(_T("Content-Length")) == 0)
      m_Request.m_nContentLength = _ttoi(sValue);
    else if ((sField.CompareNoCase(_T("Connection")) == 0) && (sValue.CompareNoCase(_T("Keep-Alive")) == 0))
      m_Request.m_bKeepAlive = TRUE;
  }

  return bSuccess;
}

LPSTR CW3MFCClient::FindNextTerminatorFromRequest(_In_ LPSTR pszLine)
{
  LPSTR pszCurrentLine = pszLine;
  while (TRUE)
  {
    ++pszCurrentLine;
    if (pszCurrentLine >= (reinterpret_cast<LPSTR>(m_Request.m_pRawEntity)))
      return pszCurrentLine;
    else if (*pszCurrentLine == '\n')
      return pszCurrentLine;
  }
  
  return nullptr;
}

BOOL CW3MFCClient::ParseRequest()
{
  //By default assume the worst 
  BOOL bSuccess = FALSE;

  //Also store a pointer to entity body.
  LPSTR pszEntityTerminator = strstr((LPSTR) m_Request.m_pRawRequest, "\r\n\r\n");
  if (pszEntityTerminator != nullptr)
  {
    m_Request.m_pRawEntity = reinterpret_cast<BYTE*>(pszEntityTerminator + 4);
    m_Request.m_dwRawEntitySize = static_cast<DWORD>(m_Request.m_dwRawRequestSize - (m_Request.m_pRawEntity - m_Request.m_pRawRequest));
  }
  else
  {
    m_Request.m_pRawEntity = nullptr;
    m_Request.m_dwRawEntitySize = 0;
  }

  //Process each line
  BOOL bFirstLine = TRUE;
  LPSTR pszLine = reinterpret_cast<LPSTR>(m_Request.m_pRawRequest);
  LPSTR pszTerminator = strstr(pszLine, "\n");
  BOOL bMoreLines = TRUE;
  do 
  {
    CString sLine;
    if (pszTerminator != nullptr)
    {
      //Form the current line
      size_t nCurSize = pszTerminator - pszLine;
      char* pszCurrentLine = new char[nCurSize];
      strncpy_s(pszCurrentLine, nCurSize, pszLine, nCurSize-1);
      pszCurrentLine[nCurSize-1] = '\0'; 

      //Transfer to the CString variable
      sLine = pszCurrentLine;

      //Tidy up the heap memory now that we are finished with it
      delete [] pszCurrentLine; 

      //Parse each line using the virtual ParseRequestLine method
      if (sLine.GetLength())
      {
        CString sField;
        CString sValue;
        if (!bFirstLine)
        {
          m_Socket.SplitRequestLine(sLine, sField, sValue);
          bSuccess = ParseRequestLine(sLine, FALSE, sField, sValue);
        }
        else
        {
          bSuccess = ParseRequestLine(sLine, TRUE, sField, sValue);
          bFirstLine = FALSE;
        }
      }

      //Move onto the next line
      if (pszTerminator != nullptr)
      {
        pszLine = pszTerminator + 1;
        if (pszLine >= reinterpret_cast<LPSTR>(m_Request.m_pRawEntity))
          bMoreLines = FALSE;
        else 
          pszTerminator = FindNextTerminatorFromRequest(pszLine);
      }
    }  
    else
      bMoreLines = FALSE;
  }
  while (bMoreLines && bSuccess);

  m_Request.m_hImpersonation = m_pSettings->m_hImpersonation;

  return bSuccess;
}

DWORD CW3MFCClient::ReturnErrorMessage(_In_ int nStatusCode)
{
  //Validate our parameters
  AFXASSUME(m_pSettings != nullptr);

  //Form the body of the response
  CStringA sBody(LoadHTML(nStatusCode));
  int nBodyLength = sBody.GetLength();

  if (m_Request.m_dwHttpVersion > MAKELONG(9, 0))
  {
    //Form the header of the response
    CW3MFCResponseHeader responseHdr;
    responseHdr.AddStatusCode(nStatusCode);
    SYSTEMTIME st;
    GetSystemTime(&st);
    responseHdr.AddDate(st);
    responseHdr.AddServer(m_pSettings->m_sServerName);
    responseHdr.AddW3MfcAllowFields(m_pSettings->m_bAllowDeleteRequest);
    responseHdr.AddContentLength(nBodyLength);
    responseHdr.AddContentType(_T("text/html"));

    //Send the header and body all in one
    TransmitBuffer(m_Socket, responseHdr, sBody, m_pSettings->m_dwWritableTimeout);
  }
  else
  {
    //No header sent for HTTP v0.9
    //so just send the body
    try
    {
    #ifdef W3MFC_SSL_SUPPORT
      m_Socket.SendWithRetry(sBody, nBodyLength, m_pSettings->m_dwWritableTimeout, *this);
    #else    
      m_Socket.SendWithRetry(sBody, nBodyLength, m_pSettings->m_dwWritableTimeout);
    #endif //#ifdef W3MFC_SSL_SUPPORT
    }
  #ifdef CWSOCKET_MFC_EXTENSIONS
    catch(CWSocketException* pEx)
    {
      //Report the error
      CString sError;
      sError.Format(_T("CW3MFCClient::ReturnErrorMessage, Failed to send to socket, Error:%d"), pEx->m_nError);
      ASSERT(m_pError != nullptr);
      m_pError->OnError(sError);

      pEx->Delete();  
    }
  #else
    catch(CWSocketException& e)
    {
      //Report the error
      CString sError;
      sError.Format(_T("CW3MFCClient::ReturnErrorMessage, Failed to send to socket, Error:%d"), e.m_nError);
      ASSERT(m_pError != nullptr);
      m_pError->OnError(sError);
    }
  #endif //#ifdef CWSOCKET_MFC_EXTENSIONS
  }

  //Log the information
  PostLog(nStatusCode, nBodyLength);

  //Return the body length
  return nBodyLength;
}

DWORD CW3MFCClient::ReturnRedirectMessage(_In_ const CString& sURL)
{
  //Validate our parameters
  AFXASSUME(m_pSettings != nullptr);

  //Form the body of the response
  CStringA sBody(LoadHTML(302));
  int nBodyLength = sBody.GetLength();

  //Form the header of the response
  CW3MFCResponseHeader responseHdr;
  responseHdr.AddStatusCode(302);
  SYSTEMTIME st;
  GetSystemTime(&st);
  responseHdr.AddDate(st);
  responseHdr.AddServer(m_pSettings->m_sServerName);
  responseHdr.AddW3MfcAllowFields(m_pSettings->m_bAllowDeleteRequest);
  responseHdr.AddLocation(sURL);
  responseHdr.AddContentLength(nBodyLength);
  responseHdr.AddContentType(_T("text/html"));

  //Send the header and body all in one
  TransmitBuffer(m_Socket, responseHdr, sBody, m_pSettings->m_dwWritableTimeout);

  //Log the information
  PostLog(302, nBodyLength);

  return nBodyLength;
}

DWORD CW3MFCClient::ReturnUnauthorizedMessage(_In_ const CString& sRealm)
{
  //Validate our parameters
  AFXASSUME(m_pSettings != nullptr);
  ASSERT(m_pSettings->m_bAllowBasicAuthentication);

  //Form the body of the response
  CStringA sBody(LoadHTML(401));
  int nBodyLength = sBody.GetLength();

  //Form the header of the response
  CW3MFCResponseHeader responseHdr;
  responseHdr.AddStatusCode(401);
  SYSTEMTIME st;
  GetSystemTime(&st);
  responseHdr.AddDate(st);
  responseHdr.AddServer(m_pSettings->m_sServerName);
  responseHdr.AddW3MfcAllowFields(m_pSettings->m_bAllowDeleteRequest);
  if (m_pSettings->m_bAllowBasicAuthentication)
    responseHdr.AddWWWAuthenticateBasic(sRealm);
  responseHdr.AddContentLength(nBodyLength);
  responseHdr.AddContentType(_T("text/html"));

  //Send the header and body all in one
  TransmitBuffer(m_Socket, responseHdr, sBody, m_pSettings->m_dwWritableTimeout);

  //Log the information
  PostLog(401, nBodyLength);

  return nBodyLength;
}

DWORD CW3MFCClient::ReturnFileDeletedOkMessage(_In_ const CString& /*sFile*/)
{
  //Validate our parameters
  AFXASSUME(m_pSettings != nullptr);

  //Form the body of the response
  CStringA sBody(GetFileDeletedHTML());
  int nBodyLength = sBody.GetLength();

  //Form the header of the response
  CW3MFCResponseHeader responseHdr;
  responseHdr.AddStatusCode(200);
  SYSTEMTIME st;
  GetSystemTime(&st);
  responseHdr.AddDate(st);
  responseHdr.AddServer(m_pSettings->m_sServerName);
  responseHdr.AddW3MfcAllowFields(m_pSettings->m_bAllowDeleteRequest);
  responseHdr.AddContentLength(nBodyLength);
  responseHdr.AddContentType(_T("text/html"));

  //Send the header and body all in one
  TransmitBuffer(m_Socket, responseHdr, sBody, m_pSettings->m_dwWritableTimeout);

  //Log the information
  PostLog(200, nBodyLength);

  return nBodyLength;
}

void CW3MFCClient::TransmitBuffer(_In_reads_bytes_(dwSize) BYTE* byData, _In_ DWORD dwSize, _In_ CW3MFCDirectory* /*pDirectory*/, _In_ BOOL bForceExpire)
{
  //validate our settings
  AFXASSUME(m_pSettings != nullptr);
  AFXASSUME(m_pSettings->m_pMimeManager != nullptr);

  CW3MFCResponseHeader responseHdr;

  //Get the current system time in UTC
  SYSTEMTIME stCurTime;
  ::GetSystemTime(&stCurTime);

  if (m_Request.m_dwHttpVersion > MAKELONG(9, 0)) //No header sent for Http 0.9
  {
    //Get the mime type for extension we are about to return
    CString sMime(m_pSettings->m_pMimeManager->GetMimeType(m_Request));

    //Form the header of the response
    responseHdr.AddStatusCode(200);
    responseHdr.AddDate(stCurTime);
    responseHdr.AddServer(m_pSettings->m_sServerName);
    responseHdr.AddW3MfcAllowFields(m_pSettings->m_bAllowDeleteRequest);
    if (bForceExpire)
      responseHdr.AddExpires(stCurTime);
    responseHdr.AddContentLength(dwSize);
    responseHdr.AddContentType(sMime);

    //Send the header and body all in one
    TransmitBuffer(m_Socket, responseHdr, byData, dwSize, m_pSettings->m_dwWritableTimeout);
  }
  else
  {
    //Send back the file contents (if not a HEAD request)
    if (m_Request.m_Verb != CW3MFCRequest::HTTP_VERB_HEAD)
    {
      try
      {
        m_Socket.SendWithRetry(byData, dwSize, m_pSettings->m_dwWritableTimeout);
      }
    #ifdef CWSOCKET_MFC_EXTENSIONS
      catch(CWSocketException* pEx)
      {
        //Report the error
        CString sError;
        sError.Format(_T("CW3MFCClient::TransmitBuffer, Failed to send to socket, Error:%d"), pEx->m_nError);
        m_pError->OnError(sError);

        pEx->Delete();  
      }
    #else
      catch(CWSocketException& e)
      {
        //Report the error
        CString sError;
        sError.Format(_T("CW3MFCClient::TransmitBuffer, Failed to send to socket, Error:%d"), e.m_nError);
        m_pError->OnError(sError);
      }
    #endif //#ifdef CWSOCKET_MFC_EXTENSIONS
    }
  }

  //Log the information
  PostLog(200, dwSize);
}
 
CW3MFCDirectory* CW3MFCClient::GetVirtualDirectory(_In_ const CString& sDirectory)
{
  //Validate our parameters
  AFXASSUME(m_pSettings != nullptr);

  CW3MFCDirectory* pDirectory = nullptr;
  for (INT_PTR i=0; (i<m_pSettings->m_Directories.GetSize()) && (pDirectory == nullptr); i++)
  {
    CW3MFCDirectory* pDir = m_pSettings->m_Directories[i];
    AFXASSUME(pDir != nullptr);
    if (sDirectory.CompareNoCase(pDir->GetAlias()) == 0)
      pDirectory = pDir;
  }

  return pDirectory;
}

TCHAR CW3MFCClient::IntToHex(_In_ int Character)
{
  //This function only expects nibbles
  ASSERT(Character >= 0 && Character <= 15);

  if (Character <= 9)
    return static_cast<TCHAR>(Character + _T('0'));
  else
    return static_cast<TCHAR>(Character - 10 + _T('A'));
}

CString CW3MFCClient::UTF8ToCString(_In_ const CString& sText)
{
  //First call the function to determine how much space we need to allocate
  CStringA sAsciiText(sText);
  int nWideLength = MultiByteToWideChar(CP_UTF8, 0, sAsciiText, -1, nullptr, 0);

  //If the calculated length is zero, then ensure we have at least room for a Null terminator
  if (nWideLength == 0)
    nWideLength = 1;

  //Now recall with the buffer to get the converted text
  CStringW sWideString;
  wchar_t* pszWText = sWideString.GetBuffer(nWideLength);
  MultiByteToWideChar(CP_UTF8, 0, sAsciiText, -1, pszWText, nWideLength);
  sWideString.ReleaseBuffer();

  return CString(sWideString);
}

CString CW3MFCClient::CStringToUTF8(_In_ const CString& sText)
{
  //First call the function to determine how much space we need to allocate
  CStringW sUnicodeText(sText);
  int nAsciiLength = WideCharToMultiByte(CP_UTF8, 0, sUnicodeText, -1, nullptr, 0, nullptr, nullptr);

  //If the calculated length is zero, then ensure we have at least room for a Null terminator
  if (nAsciiLength == 0)
    nAsciiLength = 1;

  //Now recall with the buffer to get the converted text
  CStringA sAsciiString;
  char* pszAsciiText = sAsciiString.GetBuffer(nAsciiLength);
  WideCharToMultiByte(CP_UTF8, 0, sUnicodeText, -1, pszAsciiText, nAsciiLength, nullptr, nullptr);
  sAsciiString.ReleaseBuffer();

  return CString(sAsciiString);
}

CString CW3MFCClient::URLEncode(_In_ const CString& sURL)
{
  CString sEncodedURL;
  int nLength = sURL.GetLength();
  LPTSTR pszEncodedURL = sEncodedURL.GetBufferSetLength((nLength*3) + 1);
  int nOutputIndex = 0;
  for (int i=0; i<nLength; i++)
  {
    //Pull out the current character to evaluate
    BYTE CurrentChar = static_cast<BYTE>(sURL[i]);

    //Should we encode the character or not? See RFC 1738 for the details.
    if ((CurrentChar >= '0' && CurrentChar <= '9') ||
        (CurrentChar >= 'a' && CurrentChar <= 'z') ||
        (CurrentChar >= 'A' && CurrentChar <= 'Z') ||
        (CurrentChar == '$') ||
        (CurrentChar == '-') ||
        (CurrentChar == '_') ||
        (CurrentChar == '.') ||
        (CurrentChar == '+') ||
        (CurrentChar == '!') ||
        (CurrentChar == '*') ||
        (CurrentChar == '\'') ||
        (CurrentChar == '(') ||
        (CurrentChar == ')') ||
        (CurrentChar == ','))
    {
      pszEncodedURL[nOutputIndex] = CurrentChar;
      ++nOutputIndex;
    }
    else 
    {
      pszEncodedURL[nOutputIndex] = _T('%');
      ++nOutputIndex;

      TCHAR nNibble = IntToHex((CurrentChar & 0xF0) >> 4);
      pszEncodedURL[nOutputIndex] = nNibble;
      ++nOutputIndex;

      nNibble = IntToHex(CurrentChar & 0x0F);
      pszEncodedURL[nOutputIndex] = nNibble;
      ++nOutputIndex;
    }
  }
  //Don't forget to Null terminate
  pszEncodedURL[nOutputIndex] = _T('\0');
  sEncodedURL.ReleaseBuffer();

  return sEncodedURL;
}

CString CW3MFCClient::URLDecode(_In_ const CString& sURL)
{
  CString sDecodedURL;
  int nLength = sURL.GetLength();
  LPTSTR pszDecodedURL = sDecodedURL.GetBufferSetLength(nLength + 1);
  int nOutputIndex = 0;
  for (int i=0; i<nLength; i++)
  {
    TCHAR c1 = sURL[i];
    if (c1 != _T('%'))
    {
      if (c1 == '+')
        pszDecodedURL[nOutputIndex] = ' ';  
      else
        pszDecodedURL[nOutputIndex] = c1;
      nOutputIndex++;
    }
    else
    {
      if (i < nLength-2)
      {
        int msb = ATL::AtlHexValue(static_cast<char>(sURL[i+1]));
        int lsb = ATL::AtlHexValue(static_cast<char>(sURL[i+2]));
        if (msb != -1 && lsb != -1)
        {
          int nChar = (msb << 4) + lsb;
          pszDecodedURL[nOutputIndex] = TCHAR(nChar);
          nOutputIndex++;
          i += 2;
        }
        else
        {
          pszDecodedURL[nOutputIndex] = c1;
          nOutputIndex++;
        }
      }
      else
      {
        pszDecodedURL[nOutputIndex] = c1;
        nOutputIndex++;
      }
    }
  }
  //Don't forget to Null terminate
  pszDecodedURL[nOutputIndex] = _T('\0');
  sDecodedURL.ReleaseBuffer();

  return sDecodedURL;
}

_Success_(return != FALSE)
BOOL CW3MFCClient::MapURLToLocalFilename(_Inout_ CString& sURL, _Inout_ CString& sLocalFile, _Inout_ CString& sPathInfo, _Out_ BOOL& bDirectory, _Out_ CW3MFCDirectory*& pDirectory, _Out_ DWORD& dwMatchingURL, _Out_ DWORD& dwMatchingPath)
{
  //Setup the default return value from this function
  BOOL bSuccess = FALSE;
  sLocalFile.Empty();
  sPathInfo.Empty();
  bDirectory = FALSE;
  pDirectory = nullptr;
  dwMatchingURL = 0;
  dwMatchingPath = 0;

  //Convert from Unix to Windows format
  CString sClientURL(sURL);
  sClientURL.Replace(_T("/"), _T("\\"));

  //As a security precaution do not allow any URL's which contains any relative parts in it 
  //(as an example trying to request a file outside of the directories we are serving)
  //We also exclude URL's with a ":" in them as this is how NTFS streams are accessed
  if ((sClientURL.Find(_T("..")) == -1) && (sClientURL.Find(_T(":")) == -1))
  {
    pDirectory = nullptr;
    TCHAR sDir[_MAX_DIR];
    CString sVirtualDir(sClientURL);
    sVirtualDir += _T("\\"); //Initially try a default directory
    do
    {
      sDir[0] = _T('\0');
      _tsplitpath_s(sVirtualDir, nullptr, 0, sDir, sizeof(sDir)/sizeof(TCHAR), nullptr, 0, nullptr, 0);
      if (_tcslen(sDir))
      {
        pDirectory = GetVirtualDirectory(sDir);
        if (pDirectory == nullptr)
        {
          sVirtualDir = sDir;
          sVirtualDir = sVirtualDir.Left(sVirtualDir.GetLength()-1);
        }
      }
    }
    while (pDirectory == nullptr && _tcslen(sDir));

    if (pDirectory != nullptr)
    {
      ASSERT(pDirectory->GetDirectory().GetLength());
      ASSERT(pDirectory->GetAlias().GetLength());

      //Ignore the alias part of the URL now that we have got the virtual directory
      CString sAlias(pDirectory->GetAlias());
      CString sRelativeFile(sClientURL);
      sRelativeFile = sRelativeFile.Right(sRelativeFile.GetLength() - sAlias.GetLength());

      //Form the local filename from the requested URL
      CString sDirectory(pDirectory->GetDirectory());
      int nLength = sDirectory.GetLength();
      if (sDirectory[nLength-1] != _T('\\'))
        sDirectory += _T("\\");
      sLocalFile = sDirectory; 

      //Asking for the default filename
      if (sRelativeFile.IsEmpty())
      {
        bDirectory = pDirectory->GetDirectoryListing();
        if (!bDirectory)
          sLocalFile += pDirectory->GetDefaultFile(); 

        dwMatchingURL = sURL.GetLength();
        dwMatchingPath = sLocalFile.GetLength();
      }
      else
      {
        //Ensure that we don't have two "\" separating the filename from the directory
        if (sClientURL.Find(_T('\\')) == 0)
          sLocalFile += sRelativeFile.Right(sRelativeFile.GetLength());
        else
          sLocalFile += sRelativeFile; 

        dwMatchingURL = sURL.GetLength();
        dwMatchingPath = sLocalFile.GetLength();

        //Keep parsing left to right until we find a piece of the path which is a file.
        //This is used to work out the PathInfo value
        CString sTemp(sRelativeFile);
        int nLocalFileData = 0;
        BOOL bContinueParse = TRUE;
        while (bContinueParse)
        {
          int nSlash = sTemp.Find(_T('\\'));
          if (nSlash != -1)
          {
            nLocalFileData += nSlash;
            CString sFile(sDirectory + sRelativeFile.Left(nLocalFileData));
            sTemp = sTemp.Right(sTemp.GetLength() - nSlash - 1);

            DWORD dwAttributes = GetFileAttributes(sFile);
            if (dwAttributes == INVALID_FILE_ATTRIBUTES)
            {
              bContinueParse = FALSE;
              sPathInfo = sLocalFile.Right(sLocalFile.GetLength() - sFile.GetLength() + nSlash);
              sPathInfo.Replace(_T("\\"), _T("/"));
              sLocalFile = sFile.Left(sFile.GetLength() - nSlash - 1);

              //Remove the PathInfo from the incoming parameter
              sURL = sURL.Left(sURL.GetLength() - sPathInfo.GetLength());

              dwMatchingURL = sClientURL.GetLength() - sPathInfo.GetLength();
              dwMatchingPath = sLocalFile.GetLength();
            }
            else if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
              bContinueParse = FALSE;
              sPathInfo = sLocalFile.Right(sLocalFile.GetLength() - sFile.GetLength() - 1);
              sPathInfo.Replace(_T("\\"), _T("/"));
              sLocalFile = sFile;

              //Remove the PathInfo from the incoming parameter
              sURL = sURL.Left(sURL.GetLength() - sPathInfo.GetLength() - 1);

              dwMatchingURL = sClientURL.GetLength() - sPathInfo.GetLength();
              dwMatchingPath = sLocalFile.GetLength();
            }

            nLocalFileData++; //Move over the directory separator
          }
          else
          {
            bContinueParse = FALSE;

            CString sFile(sDirectory + sRelativeFile.Left(nLocalFileData) + sTemp);

            DWORD dwAttributes = GetFileAttributes(sFile);
            if (dwAttributes == INVALID_FILE_ATTRIBUTES)
            {
              sPathInfo = sTemp;
              sPathInfo.Replace(_T("\\"), _T("/"));
              sLocalFile = sDirectory + sRelativeFile.Left(nLocalFileData);

              //Remove the PathInfo from the incoming parameter
              sURL = sURL.Left(sURL.GetLength() - sPathInfo.GetLength());

              dwMatchingURL = sClientURL.GetLength() - sPathInfo.GetLength();
              dwMatchingPath = sLocalFile.GetLength();
            }
          }
        }

        bDirectory = pDirectory->GetDirectoryListing() && (((GetFileAttributes(sLocalFile) & FILE_ATTRIBUTE_DIRECTORY) != 0));
        if (bDirectory)
        {
          int nURLLength = sURL.GetLength();
          if (nURLLength && sURL[nURLLength-1] != _T('/'))
            sURL += _T("/");
        }
      }

      bSuccess = TRUE;
    }
  }

  return bSuccess;
}

#ifdef _DEBUG
void CW3MFCClient::PostLog(_In_ int nHTTPStatusCode, _In_ DWORD dwBodyLength)
#else
void CW3MFCClient::PostLog(_In_ int /*nHTTPStatusCode*/, _In_ DWORD /*dwBodyLength*/)
#endif //#ifdef _DEBUG
{
#ifdef _DEBUG
  //The default is to just TRACE the details

  //Get the current date and time
  time_t now = time(nullptr);
  tm localtm;
  localtime_s(&localtm, &now);
  tm* pNow = &localtm;  

  //Get the time zone information
  TIME_ZONE_INFORMATION tzi;
  memset(&tzi, 0, sizeof(tzi));
  int nTZBias = 0;
  int nTZI = GetTimeZoneInformation(&tzi);
  if (nTZI == TIME_ZONE_ID_DAYLIGHT)
    nTZBias = tzi.Bias + tzi.DaylightBias;
  else if (nTZI == TIME_ZONE_ID_STANDARD)
    nTZBias = tzi.Bias;

  //Format the date and time appropiately
  TCHAR sDateTime[64];
  sDateTime[0] = _T('\0');
  _tcsftime(sDateTime, 64, _T("[%d/%b/%Y:%H:%M:%S"), pNow);

  //Display the connections to the console window
  CString sUser(m_Request.m_sUserName);
  if (sUser.IsEmpty())
    sUser = _T("-");

#ifdef CWSOCKET_MFC_EXTENSIONS
  TRACE(_T("%s - %s [%s %+.2d%.2d] \"%s\" %d %d\n"), CWSocket::AddressToString(m_Request.m_ClientAddress).GetString(),
        sUser.GetString(), sDateTime, -nTZBias/60, abs(nTZBias) % 60, m_Request.m_sRequest.GetString(), nHTTPStatusCode, dwBodyLength);
#else
  TRACE(_T("%s - %s [%s %+.2d%.2d] \"%s\" %d %d\n"), CWSocket::AddressToString(m_Request.m_ClientAddress).c_str(),
        sUser.GetString(), sDateTime, -nTZBias/60, abs(nTZBias) % 60, m_Request.m_sRequest.GetString(), nHTTPStatusCode, dwBodyLength);
#endif //#ifdef CWSOCKET_MFC_EXTENSIONS

#endif //#ifdef _DEBUG      
}

BOOL CW3MFCClient::TransmitFile(_In_ CW3MFCSocket& socket, _In_ CW3MFCResponseHeader& responseHdr, _In_ HANDLE hFile, _In_ DWORD dwSize, _In_ DWORD dwTimeout)
{
  //Validate our parameters
  AFXASSUME(m_pError != nullptr);

  //What will be the return value from this method (assume the worst)
  BOOL bSuccess = FALSE;

  //Send the header
#ifdef W3MFC_SSL_SUPPORT
  bSuccess = responseHdr.Send(m_Socket, dwTimeout, *this);
#else
  bSuccess = responseHdr.Send(socket, dwTimeout);
#endif //#ifdef W3MFC_SSL_SUPPORT
  if (!bSuccess)
  {
    //Report the error
    CString sError;
    sError.Format(_T("CW3MFCClient::TransmitFile, Failed to send to response header, Error:%u"), GetLastError());
    ASSERT(m_pError != nullptr);
    m_pError->OnError(sError);
  }

  //Send the body
  if (bSuccess)
  {
    try
    {
      BYTE byBuf[4096];
      DWORD dwBytesRead = 0;
      while (bSuccess && (dwBytesRead < dwSize))
      {
        DWORD dwRead = 0;
        bSuccess = ReadFile(hFile, byBuf, 4096, &dwRead, nullptr);
        if (bSuccess)
        {
          dwBytesRead += dwRead;

        #ifdef W3MFC_SSL_SUPPORT
          socket.SendWithRetry(byBuf, dwRead, dwTimeout, *this);
        #else
          socket.SendWithRetry(byBuf, dwRead, dwTimeout);
        #endif //#ifdef W3MFC_SSL_SUPPORT
        }
        else
        {
          //Report the error
          CString sError;
          sError.Format(_T("CW3MFCClient::TransmitFile, Failed to read file, Error:%u"), GetLastError());
          ASSERT(m_pError != nullptr);
          m_pError->OnError(sError);
        }
      } 
    }
  #ifdef CWSOCKET_MFC_EXTENSIONS
    catch(CWSocketException* pEx)
    {
      bSuccess = FALSE;

      //Report the error
      CString sError;
      sError.Format(_T("CW3MFCClient::TransmitFile, Failed to send response file data, Error:%d"), pEx->m_nError);
      ASSERT(m_pError != nullptr);
      m_pError->OnError(sError);

      pEx->Delete();  
    }
  #else
    catch(CWSocketException& e)
    {
      bSuccess = FALSE;

      //Report the error
      CString sError;
      sError.Format(_T("CW3MFCClient::TransmitFile, Failed to send response file data, Error:%d"), e.m_nError);
      ASSERT(m_pError != nullptr);
      m_pError->OnError(sError);
    }
  #endif //#ifdef CWSOCKET_MFC_EXTENSIONS
  }

  return bSuccess;
}

BOOL CW3MFCClient::TransmitBuffer(_In_ CW3MFCSocket& socket, _In_ CW3MFCResponseHeader& responseHdr, _In_ CStringA& sData, _In_ DWORD dwTimeout)
{
  int nDataLength = sData.GetLength();
  LPSTR pszData = sData.GetBuffer();
  BOOL bSuccess = TransmitBuffer(socket, responseHdr, reinterpret_cast<BYTE*>(pszData), nDataLength, dwTimeout);
  sData.ReleaseBuffer();
  return bSuccess;
}

BOOL CW3MFCClient::TransmitBuffer(_In_ CW3MFCSocket& socket, _In_ CW3MFCResponseHeader& responseHdr, _In_reads_bytes_(dwSize) const BYTE* byData, _In_ DWORD dwSize, _In_ DWORD dwTimeout)
{
  //What will be the return value from this method (assume the worst)
  BOOL bSuccess = FALSE;
  
  //Send the header
#ifdef W3MFC_SSL_SUPPORT
  bSuccess = responseHdr.Send(socket, dwTimeout, *this);
#else
  bSuccess = responseHdr.Send(socket, dwTimeout);
#endif //#ifdef W3MFC_SSL_SUPPORT
  if (!bSuccess)
  {
    //Report the error
    CString sError;
    sError.Format(_T("CW3MFCClient::TransmitBuffer, Failed to send response header, Error:%u"), ::GetLastError());
    m_pError->OnError(sError);
  }

  //Send the body if required
  if (bSuccess && dwSize)
  {
    try
    {
    #ifdef W3MFC_SSL_SUPPORT
      socket.SendWithRetry(byData, dwSize, dwTimeout, *this);
    #else
      socket.SendWithRetry(byData, dwSize, dwTimeout);
    #endif //#ifdef W3MFC_SSL_SUPPORT
    }
  #ifdef CWSOCKET_MFC_EXTENSIONS
    catch(CWSocketException* pEx)
    {
      bSuccess = FALSE;

      //Report the error
      CString sError;
      sError.Format(_T("CW3MFCClient::TransmitBuffer, Failed to send response body, Error:%d"), pEx->m_nError);
      m_pError->OnError(sError);

      pEx->Delete();  
    }
  #else
    catch(CWSocketException& e)
    {
      bSuccess = FALSE;

      //Report the error
      CString sError;
      sError.Format(_T("CW3MFCClient::TransmitBuffer, Failed to send response body, Error:%d"), e.m_nError);
      m_pError->OnError(sError);
    }
  #endif //#ifdef CWSOCKET_MFC_EXTENSIONS
  }

  return bSuccess;
}

void CW3MFCClient::SetRequestToStop()
{
  //Let the base class do its thing
  __super::SetRequestToStop();

  //Set the event which signals that we want this worker thread to exit
  m_StopEvent.SetEvent();
}

_Success_(return != FALSE)
BOOL CW3MFCClient::GetKeySizeServerVariable(_Out_ DWORD& dwKeySize)
{
  //What will be the return value
  BOOL bSuccess = FALSE;
  dwKeySize = 0;

#ifdef W3MFC_SSL_SUPPORT
  //Validate our parameters
  AFXASSUME(m_pSettings != nullptr);

  if (m_pSettings->m_bSSL)
  {
    SecPkgContext_ConnectionInfo ConnectionInfo;
    SECURITY_STATUS status = QueryAttribute(SECPKG_ATTR_CONNECTION_INFO, &ConnectionInfo);
    if (status == SEC_E_OK)
    {
      dwKeySize = ConnectionInfo.dwCipherStrength;
      bSuccess = TRUE;
    }
  }
#endif //#ifdef W3MFC_SSL_SUPPORT

  return bSuccess;
}

_Success_(return != FALSE)
BOOL CW3MFCClient::GetServerKeySizeServerVariable(_Out_ DWORD& dwKeySize)
{
  //What will be the return value
  BOOL bSuccess = FALSE;
  dwKeySize = 0;

#ifdef W3MFC_SSL_SUPPORT
  //Validate our parameters
  SSLWrappers::CCachedCredentials* pCredentials = GetCachedCredentials();
  AFXASSUME(pCredentials != nullptr);

  if (m_pSettings->m_bSSL)
  {
    dwKeySize = 0;
    DWORD dwPropertySize = sizeof(dwKeySize);
    bSuccess = pCredentials->m_Certificate.GetProperty(CERT_SUBJECT_PUB_KEY_BIT_LENGTH_PROP_ID, &dwKeySize, &dwPropertySize);
  }
#endif //#ifdef W3MFC_SSL_SUPPORT

  return bSuccess;
}

_Success_(return != FALSE)
BOOL CW3MFCClient::GetCertSerialNumberServerVariable(_Inout_ CString& sSerialNumber)
{
  //What will be the return value (assume the worst)
  BOOL bSuccess = FALSE;

#ifdef W3MFC_SSL_SUPPORT
  //Validate our parameters
  SSLWrappers::CCachedCredentials* pCredentials = GetCachedCredentials();
  AFXASSUME(pCredentials != nullptr);

  if (m_pSettings->m_bSSL)
    bSuccess = CryptoWrappers::BinaryToString(pCredentials->m_Certificate.m_pCert->pCertInfo->SerialNumber.pbData, pCredentials->m_Certificate.m_pCert->pCertInfo->SerialNumber.cbData, CRYPT_STRING_HEX | CRYPT_STRING_NOCRLF, sSerialNumber);
#else
  UNREFERENCED_PARAMETER(sSerialNumber);
#endif //#ifdef W3MFC_SSL_SUPPORT

  return bSuccess;
}

_Success_(return != FALSE)
BOOL CW3MFCClient::GetCertIssuerServerVariable(_Inout_ CString& sIssuer)
{
  //What will be the return value (assume the worst)
  BOOL bSuccess = FALSE;

#ifdef W3MFC_SSL_SUPPORT
  //Validate our parameters
  SSLWrappers::CCachedCredentials* pCredentials = GetCachedCredentials();
  AFXASSUME(pCredentials != nullptr);

  if (m_pSettings->m_bSSL)
  {
    TCHAR szName[1000];
    szName[0] = _T('\0');
    if (CertNameToStr(pCredentials->m_Certificate.m_pCert->dwCertEncodingType, &pCredentials->m_Certificate.m_pCert->pCertInfo->Issuer, CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG, szName, sizeof(szName)/sizeof(TCHAR)))
    {
      sIssuer = szName;
      bSuccess = TRUE;  
    }
  }
#else
  UNREFERENCED_PARAMETER(sIssuer);
#endif //#ifdef W3MFC_SSL_SUPPORT

  return bSuccess;  
}

_Success_(return != FALSE)
BOOL CW3MFCClient::GetCertSubjectServerVariable(_Inout_ CString& sSubject)
{
  //What will be the return value (assume the worst)
  BOOL bSuccess = FALSE;

#ifdef W3MFC_SSL_SUPPORT
  //Validate our parameters
  SSLWrappers::CCachedCredentials* pCredentials = GetCachedCredentials();
  AFXASSUME(pCredentials != nullptr);

  if (m_pSettings->m_bSSL)
  {
    TCHAR szName[1000];
    szName[0] = _T('\0');
    if (CertNameToStr(pCredentials->m_Certificate.m_pCert->dwCertEncodingType, &pCredentials->m_Certificate.m_pCert->pCertInfo->Subject, CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG, szName, sizeof(szName)/sizeof(TCHAR)))
    {
      sSubject = szName;
      bSuccess = TRUE;  
    }
  }
#else
  UNREFERENCED_PARAMETER(sSubject);
#endif //#ifdef W3MFC_SSL_SUPPORT

  return bSuccess;
}

#ifndef W3MFC_NO_ISAPI_SUPPORT
BOOL CW3MFCClient::TransmitFile(_In_ CW3MFCSocket& /*socket*/, _In_ CW3MFCResponseHeader& /*responseHdr*/, _In_ HSE_TF_INFO* /*pInfo*/)
{
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}
#endif //#ifndef W3MFC_NO_ISAPI_SUPPORT

BOOL CW3MFCClient::PreHandleGetPostHead()
{
  //This allows derived classes to internally handle GET, POST or HEAD requests.
  //Return TRUE in your derived classes to tell the framework that the request
  //has been handled and stops the framework from implementing its default processing
  //for a URL

  //In this base class we of course allow default processing to occur
  return FALSE;
}

CStringA CW3MFCClient::GetFileDeletedHTML()
{
  return m_sDeletedHTML;
}
  
void CW3MFCClient::LoadHTMLResources()
{
  LoadHTMLResource(IDH_302, m_s302HTML);
  LoadHTMLResource(IDH_400, m_s400HTML);
  LoadHTMLResource(IDH_401, m_s401HTML);
  LoadHTMLResource(IDH_404, m_s404HTML);
  LoadHTMLResource(IDH_500, m_s500HTML);
  LoadHTMLResource(IDH_FILE_DELETED_OK, m_sDeletedHTML);
}

CStringA CW3MFCClient::LoadHTML(_In_ int nStatusCode)
{
  //What will be the return value
  CStringA sHTML;

  switch (nStatusCode)
  {
    case 302:
    {
      sHTML = m_s302HTML;
      break;
    }
    case 400:
    {
      sHTML = m_s400HTML;
      break;
    }
    case 401:
    {
      sHTML = m_s401HTML;
      break;
    }
    case 404:
    {
      sHTML = m_s404HTML;
      break;
    }
    default:
    {
      sHTML = m_s500HTML;
      break;
    }
  }

  return sHTML;
}

BOOL CW3MFCClient::LoadHTMLResource(_In_ UINT nID, _Inout_ CStringA& sHTML)
{
  //What will be the return value from this function (assume the worst)
  BOOL bSuccess = FALSE;

  HMODULE hModule = AfxFindResourceHandle(MAKEINTRESOURCE(nID), RT_HTML);
  HRSRC hRsrc = FindResource(hModule, MAKEINTRESOURCE(nID), RT_HTML);
  if (hRsrc != nullptr)
  {
    //Load up the resource
    DWORD dwSize = SizeofResource(hModule, hRsrc); 
    HGLOBAL hGlobal = LoadResource(hModule, hRsrc);

    //Allocate a new char array and copy the HTML resource into it 
    if (hGlobal != nullptr)
    {
      LPSTR pszHTML = sHTML.GetBuffer(dwSize + 1);
      char* pszResource = static_cast<char*>(LockResource(hGlobal));
      if (pszResource != nullptr)
      {
        strncpy_s(pszHTML, dwSize + 1, pszResource, dwSize);
        pszHTML[dwSize] = _T('\0');
        bSuccess = TRUE;
      }
      else
      {
        //Report the error
        TRACE(_T("CW3MFCServer::LoadHTMLResource, Failed to load HTML resource, GetLastError:%d\n"), ::GetLastError());
      }
      sHTML.ReleaseBuffer();
    }
  }

  return bSuccess;
}

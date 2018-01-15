/*
Module : W3MFCClient.h
Purpose: Defines the interface for the CW3MFCClient classes
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


/////////////////////////////// Macros / Defines //////////////////////////////

#pragma once

#ifndef __W3MFCCLIENT_H__
#define __W3MFCCLIENT_H__

#ifndef _Success_
#define _Success_(expr)
#endif //#ifndef _Success_


/////////////////////////////// Includes //////////////////////////////////////

#ifndef __AFX_H__
#pragma message("To avoid this message, please put the MFC header files in your pre compiled header (normally stdafx.h)")
#include <afx.h>
#endif //#ifndef __AFX_H__

#include "W3MFCSocket.h"
#include "ThrdPool.h"
#include "W3MFCSettings.h"
#include "W3MFCResponseHeader.h"
#include "W3MFCError.h"


/////////////////////////////// Classes ///////////////////////////////////////

//Class which represents the thread pool request
class CW3MFCThreadPoolRequest
{
public:
//Constructors / Destructors
  CW3MFCThreadPoolRequest() 
  {
    memset(&m_ClientAddress, 0, sizeof(m_ClientAddress));
  }

//Member variables
  SOCKADDR_INET m_ClientAddress;
  CW3MFCSocket  m_ClientSocket;
};

//Class which handles the HTTP client connection
class CW3MFCClient : public CThreadPoolClient
#ifdef W3MFC_SSL_SUPPORT  
                   , public SSLWrappers::CSocket
#endif //#ifdef W3MFC_SSL_SUPPORT  
{
public:
//Constructors / Destructors
  CW3MFCClient();
  virtual ~CW3MFCClient();

//Methods
  virtual BOOL  Run(_In_ const CThreadPoolRequest& request);
  virtual DWORD ReturnErrorMessage(_In_ int nStatusCode);
  virtual DWORD ReturnRedirectMessage(_In_ const CString& sURL);
  virtual DWORD ReturnFileDeletedOkMessage(_In_ const CString& sFile);
  virtual DWORD ReturnUnauthorizedMessage(_In_ const CString& sRealm);
  virtual void  PostLog(_In_ int nHTTPStatusCode, _In_ DWORD dwBodyLength);
  _Success_(return != FALSE) virtual BOOL GetKeySizeServerVariable(_Out_ DWORD& dwKeySize);
  _Success_(return != FALSE) virtual BOOL GetServerKeySizeServerVariable(_Out_ DWORD& dwKeySize);
  _Success_(return != FALSE) virtual BOOL GetCertSerialNumberServerVariable(_Inout_ CString& sSerialNumber);
  _Success_(return != FALSE) virtual BOOL GetCertIssuerServerVariable(_Inout_ CString& sIssuer);
  _Success_(return != FALSE) virtual BOOL GetCertSubjectServerVariable(_Inout_ CString& sSubject);
  virtual BOOL TransmitBuffer(_In_ CW3MFCSocket& socket, _In_ CW3MFCResponseHeader& responseHdr, _In_reads_bytes_(dwSize) const BYTE* byData, _In_ DWORD dwSize, _In_ DWORD dwTimeout);
  virtual BOOL TransmitBuffer(_In_ CW3MFCSocket& socket, _In_ CW3MFCResponseHeader& responseHdr, _In_ CStringA& sData, _In_ DWORD dwTimeout);
  virtual BOOL TransmitFile(_In_ CW3MFCSocket& socket, _In_ CW3MFCResponseHeader& responseHdr, _In_ HANDLE hFile, _In_ DWORD dwSize, _In_ DWORD dwTimeout);
#ifndef W3MFC_NO_ISAPI_SUPPORT
  virtual BOOL  TransmitFile(_In_ CW3MFCSocket& socket, _In_ CW3MFCResponseHeader& responseHdr, _In_ HSE_TF_INFO* pInfo);
#endif //#ifndef W3MFC_NO_ISAPI_SUPPORT
  _Success_(return != FALSE) virtual BOOL MapURLToLocalFilename(_Inout_ CString& sURL, _Inout_ CString& sLocalFile, _Inout_ CString& sPathInfo, _Out_ BOOL& bDirectory, _Out_ CW3MFCDirectory*& pDirectory, _Out_ DWORD& dwMatchingURL, _Out_ DWORD& dwMatchingPath);
  virtual void  SetRequestToStop();
  virtual BOOL  PreHandleGetPostHead();

//Static Methods
  static TCHAR   IntToHex(_In_ int Character);
  static CString URLDecode(_In_ const CString& sURL);
  static CString URLEncode(_In_ const CString& sURL);
  static CString UTF8ToCString(_In_ const CString& sText);
  static CString CStringToUTF8(_In_ const CString& sText);
  _Success_(return != FALSE) static BOOL ParseDate(_In_ const CString& sField, _Out_ SYSTEMTIME& time);
  _Success_(return != FALSE) static BOOL ParseWeekDay(_In_z_ LPCTSTR pszToken, _Out_ WORD& nWeekDay);
  _Success_(return != FALSE) static BOOL ParseMonth(_In_z_ LPCTSTR pszToken, _Out_ WORD& nMonth);

//Member variables
  CW3MFCSettings*       m_pSettings;
  IW3MFCError*          m_pError;
  CW3MFCRequest         m_Request;
  CW3MFCSocket          m_Socket;
#ifndef W3MFC_NO_ISAPI_SUPPORT
  DWORD                 m_dwDataSentViaWriteClient;
  int                   m_nHttpStatusCodeSent;
#endif //#ifndef W3MFC_NO_ISAPI_SUPPORT
protected:
//Methods
  virtual BOOL             ParseRequest();
  virtual BOOL             ParseSimpleRequestLine(_In_ const CString& sLine);
  virtual BOOL             ParseAuthorizationLine(_In_ const CString& sLine);
  virtual BOOL             ParseRequestLine(_In_ const CString& sCurrentLine, _In_ BOOL bFirstLine, _In_ const CString& sField, _In_ const CString& sValue);
  virtual void             TransmitBuffer(_In_reads_bytes_(dwSize) BYTE* byData, _In_ DWORD dwSize, _In_ CW3MFCDirectory* pDirectory, _In_ BOOL bForceExpire);
  virtual BOOL             AllowThisConnection();
  virtual CW3MFCDirectory* GetVirtualDirectory(_In_ const CString& sDirectory);
  virtual void             HandleClient();
  virtual LPSTR            FindNextTerminatorFromRequest(_In_ LPSTR pszLine);
#ifdef W3MFC_SSL_SUPPORT
  virtual BOOL             SetupSSL();
#endif //#ifdef W3MFC_SSL_SUPPORT
  virtual CStringA         GetFileDeletedHTML();
  virtual void             LoadHTMLResources();
  virtual CStringA         LoadHTML(_In_ int nStatusCode);
  virtual BOOL             LoadHTMLResource(_In_ UINT nID, _Inout_ CStringA& sHTML);

//Member variables
  ::CEvent m_StopEvent;
  CStringA m_s302HTML;
  CStringA m_s400HTML;
  CStringA m_s401HTML;
  CStringA m_s404HTML;
  CStringA m_s500HTML;
  CStringA m_sDeletedHTML;
};

#endif //#ifndef __W3MFCCLIENT_H__

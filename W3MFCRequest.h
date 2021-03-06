/*
Module : W3MFCRequest.h
Purpose: Defines the interface for the CW3MFCRequest classes
Created: PJN / 30-09-2001

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

#ifndef __W3MFCREQUEST_H__
#define __W3MFCREQUEST_H__

#ifndef W3MFC_EXT_CLASS
#define W3MFC_EXT_CLASS
#endif //#ifndef W3MFC_EXT_CLASS


/////////////////////////////// Includes //////////////////////////////////////

#ifndef __AFX_H__
#pragma message("To avoid this message, please put the MFC header files in your pre compiled header (normally stdafx.h)")
#include <afx.h>
#endif //#ifndef __AFX_H__


/////////////////////////////// Classes ///////////////////////////////////////

//Class which represents a request from a HTTP client
class W3MFC_EXT_CLASS CW3MFCRequest
{
public:
//Constructors / Destructors
  CW3MFCRequest();
  virtual ~CW3MFCRequest() 
  {
    SecureEmptyString(m_sUserName);
    SecureEmptyString(m_sPassword);
  };

//Enums for m_Verb
  enum HttpVerb
  {
    HTTP_VERB_POST      = 0,
    HTTP_VERB_GET       = 1,
    HTTP_VERB_HEAD      = 2,
    HTTP_VERB_PUT       = 3,
    HTTP_VERB_LINK      = 4,
    HTTP_VERB_DELETE    = 5,
    HTTP_VERB_UNLINK    = 6,
    HTTP_VERB_UNKNOWN   = 7,
  };

//Enums for Authorization type
  enum HttpAuthorization
  {
    HTTP_AUTHORIZATION_ANONYMOUS = 0,
    HTTP_AUTHORIZATION_PLAINTEXT = 1,
    HTTP_AUTHORIZATION_NTLM      = 2,
  };  

//Methods
  CW3MFCRequest& operator=(const CW3MFCRequest& request);
  
  __forceinline static void SecureEmptyString(CStringA& sVal)
  {
    int nLength = sVal.GetLength();
    if (nLength)
    {
      LPSTR pszVal = sVal.GetBuffer(nLength);
      SecureZeroMemory(pszVal, nLength);
      sVal.ReleaseBuffer();
    }
  }

  __forceinline static void SecureEmptyString(CStringW& sVal)
  {
    int nLength = sVal.GetLength();
    if (nLength)
    {
      LPWSTR pszVal = sVal.GetBuffer(nLength);
      SecureZeroMemory(pszVal, nLength*sizeof(wchar_t));
      sVal.ReleaseBuffer();
    }
  }

  __forceinline static void SecureEmptyString(LPSTR sVal)
  {
    size_t nLen = strlen(sVal);
    if (nLen)
      SecureZeroMemory(sVal, nLen);
  }

//Member variables
  BYTE*              m_pRawRequest;                 //The raw request 
  DWORD              m_dwRawRequestSize;            //The raw request size
  BYTE*              m_pRawEntity;                  //The raw entity-body if any
  DWORD              m_dwRawEntitySize;             //The raw entity-body size if any
  int                m_nContentLength;              //The content length of the entity body
  CString            m_sRequest;                    //The Full request line e.g. "GET / HTTP/1.0"
  SOCKADDR_INET      m_ClientAddress;               //The IP address where the request originated from
  HttpVerb           m_Verb;                        //enum of the HTTP verb used e.g. GET, PUT etc
  CString            m_sVerb;                       //String version of the HTTP verb used
  CString            m_sURL;                        //The URL of the request (URL Decoded)
  CString            m_sRawURL;                     //The URL of the request (before it is URL Decoded)
  CString            m_sPathInfo;                   //Any data after the file name but before the first ?
  CString            m_sExtra;                      //Any part of the URL after the "?"
  CString            m_sRawExtra;                   //Any part of the URL after the "?" (before it is URL Decoded)
  DWORD              m_dwHttpVersion;               //The HTTP Version Number of the HTTP client request
                                                    //encoded as MAKELONG(Minor, Major)
  BOOL               m_bIfModifiedSincePresent;     //Is the If-Modified-Since header present
  SYSTEMTIME         m_IfModifiedSince;             //The actual If-Modified-Since header
  HttpAuthorization  m_AuthorizationType;           //What authorization method is being used
  CString            m_sUserName;                   //username if Authentication is being used
  CString            m_sPassword;                   //password if plaintext authorization is being used
  CString            m_sRemoteHost;                 //the domain name of the client (if reverse DNS is enabled otherwise empty)
  CString            m_sContentType;                //the content type of the entity body
  BOOL               m_bKeepAlive;                  //TRUE if the "Connection: Keep-Alive" header has been set 
  BOOL               m_bLoggedOn;                   //Are we impersonating a user via LogonUser
  CString            m_sLocalFile;                  //The local file this request maps to (if any)
  CMapStringToString m_HeaderMap;                   //Hash table of all the headers
  HANDLE             m_hImpersonation;              //The impersonation token the request is using if any
};

//Helper class to allow easy parsing of CW3MFCRequest::m_sRawExtra
class CW3MFCRequestParams : public CMapStringToString
{
public:
//Methods
  void Parse(_In_ LPCTSTR pszString);
};

#endif //#ifndef __W3MFCREQUEST_H__

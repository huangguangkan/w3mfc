/*
Module : W3MFCSettings.h
Purpose: Defines the interface for the 
Created: PJN / 23-03-2014

Copyright (c) 2015 - 2017 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

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

#ifndef __W3MFCSETTINGS_H__
#define __W3MFCSETTINGS_H__


/////////////////////////////// Includes //////////////////////////////////////

#ifndef __AFX_H__
#pragma message("To avoid this message, please put the MFC header files in your pre compiled header (normally stdafx.h)")
#include <afx.h>
#endif //#ifndef __AFX_H__

#include "W3MFCDirectory.h"
#include "W3MFCMimeManager.h"
#include "W3MFCISAPIManager.h"
#ifdef W3MFC_SSL_SUPPORT
#include "SSLWrappers.h" //If you get a compilation error about this missing header file, then you need to download my SSLWrappers code from http://www.naughter.com/sslwrappers.html
#endif //#ifdef W3MFC_SSL_SUPPORT
#include "SocMFC.h"


/////////////////////////////// Classes ///////////////////////////////////////

//forward declarations
class CW3MFCISAPI;
class CW3MFCCGI;

//The settings which the web server uses in the call to CW3MFCServer::Start
class W3MFC_EXT_CLASS CW3MFCSettings
{
public:
//Constructors / Destructors
  CW3MFCSettings();
  virtual ~CW3MFCSettings();

//Methods
  void FreeDirectoryArray();                                                        //Frees up the memory allocated in m_Directories

//Member variables
  unsigned short                              m_nPort;                              //The port on which to run the web server
  CArray<CW3MFCDirectory*, CW3MFCDirectory*&> m_Directories;                        //Directories served up by this server
  BOOL                                        m_bBind;                              //Should the server be bound to an address
  BOOL                                        m_bIPv6;
  CWSocket::String                            m_sBindAddress;                       //The IP address to bind to (if m_bBind is set)
  DWORD                                       m_dwIdleClientTimeout;                //Timeout in ms to wait for client requests
  CString                                     m_sServerName;                        //The Web server name to return in HTTP headers
  CString                                     m_sUserName;                          //The account to run the web server under
  CString                                     m_sPassword;                          //The account's password
  int                                         m_nThreadPoolSize;                    //The size of the thread pool to use
  int                                         m_nThreadPoolQueueSize;               //The size of the thread pool queue to use
  int                                         m_nListenThreadStackSize;             //The stack size to use for the listen thread. 0 implies a default value
  BOOL                                        m_bDNSLookup;                         //Should we do reverse DNS lookups on client addresses when connecting
  DWORD                                       m_dwWritableTimeout;                  //Timeout in ms to wait for client sockets to become writable
  BOOL                                        m_bEnableThreadLifetime;              //TRUE if threads are to be recycled in the thread pool
  DWORD                                       m_dwThreadLifetime;                   //The lifetime for threads (in Minutes) if recycling is enabled
  CW3MFCMimeManager*                          m_pMimeManager;                       //The Mime manager to use
  BOOL                                        m_bUseIOCPQueue;                      //Should CIOCPThreadPoolQueue be used in preference to CDirectedThreadPoolQueue.
                                                                                    //Please note that the IOCP class does not support thread recycling
  BOOL                                        m_bAutoExpire;                        //Should all reponses be marked with an immediate "Expires" header
  BOOL                                        m_bAllowDeleteRequest;                //Should the "DELETE" request be allowed
#ifndef W3MFC_NO_CGI_SUPPORT
  DWORD                                       m_dwCGIResponseBufferSize;            //The size of the buffer to use when reading the data from CGI programs. Note that
                                                                                    //this is important value as the code will only look for Keep Alive headers
                                                                                    //in the first buffer returned. So make sure this value is bigger than any 
                                                                                    //of the headers which any of your CGI programs generate
#endif //#ifndef W3MFC_NO_CGI_SUPPORT                                                                          
  BOOL                                        m_bAllowAnonymous;                    //Should we support anonymous access
  BOOL                                        m_bAllowBasicAuthentication;          //Should we support Basic authentication
  BOOL                                        m_bPerformPassthroughAuthentication;  //Should the credentials sent to us by the client, result in us calling thro to the OS to login that user
#ifndef W3MFC_NO_ISAPI_SUPPORT
  BOOL                                        m_bCacheISAPI;                        //Should we cache ISAPI extensions
  CW3MFCISAPIManager*                         m_pISAPIManager;                      //The ISAPI Extension manager to use
  CW3MFCISAPI*                                m_pISAPI;                             //The actual ISAPI class to use
#endif //#ifndef W3MFC_NO_ISAPI_SUPPORT
#ifndef W3MFC_NO_CGI_SUPPORT
  CW3MFCCGI*                                  m_pCGI;                               //The actual CGI class to use
#endif //#ifndef W3MFC_NO_CGI_SUPPORT
  ATL::CHandle                                m_hImpersonation;
#ifdef W3MFC_SSL_SUPPORT
  BOOL                                        m_bSSL;                               //TRUE to use SSL
  DWORD                                       m_dwSSLEnabledProtocols;              //SCHANNEL_CRED::grbitEnabledProtocols value
  CString                                     m_sServerCertificateName;             //The name of the certificate to use
  DWORD                                       m_dwSSLReadTimeout;                   //The SSL write timeout to use
  DWORD                                       m_dwSSLWriteTimeout;                  //The SSL write timeout to use
#endif //#ifdef W3MFC_SSL_SUPPORT
};

#endif //#ifndef __W3MFCSETTINGS_H__

/*
Module : main.cpp
Purpose: Sample Web server implementation using W3MFC
Created: PJN / 24-05-2002
History: PJN / 31-05-2008 1. Code now compiles cleanly using Code Analysis (/analyze)
         PJN / 12-07-2009 1. Code in CMyHttpClient::PostLog now correctly handles Daylight Savings when logging.
         PJN / 11-08-2012 1. Updated copyright details.
                          2. Updated the code to compile cleanly on VC 2012
         PJN / 16-03-2014 1. Updated copyright details

Copyright (c) 2002 - 2017 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/

///////////////////////// Includes ////////////////////////////////////////////

#include "stdafx.h"
#include "W3MFC.h"
#include "W3MFCClient.h"


///////////////////////// Local Classes ///////////////////////////////////////

class CW3MFCApp : public CWinApp
{
protected:
//virtual method
  virtual BOOL InitInstance();
  virtual int ExitInstance();
};

class CMyHttpClient : public CW3MFCClient
{
public:
  virtual void PostLog(int nHTTPStatusCode, DWORD dwBodyLength);
};

class CMyHttpServerDirected : public CW3MFCServer<CMyHttpClient, CDirectedThreadPoolQueue>
{
public:
  virtual void OnError(const CString& sError);
};

class CMyHttpServerIOCP : public CW3MFCServer<CMyHttpClient, CIOCPThreadPoolQueue>
{
public:
  void OnError(const CString& sError);
};


////////////////// Globals and Statics ////////////////////////////////////////

CMyHttpServerDirected* g_pTheWebServerDirected = nullptr;
CMyHttpServerIOCP* g_pTheWebServerIOCP = nullptr;
CW3MFCApp theApp;


///////////////////////// Implementation //////////////////////////////////////

BOOL WINAPI ConsoleHandlerRoutine(DWORD /*dwCtrlType*/)
{
  _tprintf(_T("Received request to shut down the Web Server\n"));
  if (g_pTheWebServerIOCP != nullptr)
    g_pTheWebServerIOCP->Stop();
  if (g_pTheWebServerDirected != nullptr)
    g_pTheWebServerDirected->Stop();

  return TRUE;
}


void CMyHttpServerDirected::OnError(const CString& sError)
{
  //Just printf out the error to the console window as well as the output window
  TRACE(_T("%s\n"), sError.GetString());
  _tprintf(_T("%s\n"), sError.GetString());
}

void CMyHttpServerIOCP::OnError(const CString& sError)
{
  //Just printf out the error to the console window as well as the output window
  TRACE(_T("%s\n"), sError.GetString());
  _tprintf(_T("%s\n"), sError.GetString());
}


void CMyHttpClient::PostLog(int nHTTPStatusCode, DWORD dwBodyLength)
{
  //Log each request to the console window using the W3C Common
  //log format.

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
  _tcsftime(sDateTime, sizeof(sDateTime)/sizeof(TCHAR), _T("%d/%b/%Y:%H:%M:%S"), pNow);

  //Display the connections to the console window
  CString sUser(m_Request.m_sUserName);
  if (sUser.IsEmpty())
    sUser = _T("-");

#ifdef CWSOCKET_MFC_EXTENSIONS
  _tprintf(_T("%s - %s [%s %+.2d%.2d] \"%s\" %d %u\n"), CWSocket::AddressToString(m_Request.m_ClientAddress, NI_NUMERICHOST).GetString(), 
           sUser.GetString(), sDateTime, -nTZBias/60, abs(nTZBias) % 60, m_Request.m_sRequest.GetString(), nHTTPStatusCode, dwBodyLength);
#else
  _tprintf(_T("%s - %s [%s %+.2d%.2d] \"%s\" %d %u\n"), CWSocket::AddressToString(m_Request.m_ClientAddress, NI_NUMERICHOST).c_str(),
           sUser.GetString(), sDateTime, -nTZBias/60, abs(nTZBias) % 60, m_Request.m_sRequest.GetString(), nHTTPStatusCode, dwBodyLength);
#endif //#ifdef CWSOCKET_MFC_EXTENSIONS
}


BOOL CW3MFCApp::InitInstance()
{
  //Initialise Sockets
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
  {
    _tprintf(_T("Failed to initialise Winsock, Error:%u\n"), ::GetLastError());
    return FALSE;
  }

  //By default, use an ini file in the same location as the exe
  TCHAR szIni[_MAX_PATH];
  szIni[0] = _T('\0');
  if (!GetModuleFileName(nullptr, szIni, _MAX_PATH))
  {
    _tprintf(_T("Failed to get module filename, Error:%u\n"), ::GetLastError());
    return FALSE;
  }
  TCHAR szDrive[_MAX_DRIVE];
  szDrive[0] = _T('\0');
  TCHAR szDir[_MAX_DIR];
  szDir[0] = _T('\0');
  TCHAR szFname[_MAX_FNAME];
  szFname[0] = _T('\0');
  _tsplitpath_s(szIni, szDrive, sizeof(szDrive)/sizeof(TCHAR), szDir, sizeof(szDir)/sizeof(TCHAR), szFname, sizeof(szFname)/sizeof(TCHAR), nullptr, 0);
  _tmakepath_s(szIni, sizeof(szIni)/sizeof(TCHAR), szDrive, szDir, szFname, _T("ini"));
  free(const_cast<void*>(reinterpret_cast<const void*>(m_pszProfileName)));
  m_pszProfileName = _tcsdup(szIni);

  //Parse the command line and change the ini file if a filename is found
  CCommandLineInfo cmdInfo;
  ParseCommandLine(cmdInfo);
  if (!cmdInfo.m_strFileName.IsEmpty())
  {
    free(const_cast<void*>(reinterpret_cast<const void*>(m_pszProfileName)));
    m_pszProfileName = _tcsdup(cmdInfo.m_strFileName);
  }

  //setup all the virtual directories
  CW3MFCSettings settings;
  int nDirectories = GetProfileInt(_T("General"), _T("Directories"), 0);
  if (nDirectories == 0)
  {
    _tprintf(_T("Usage: W3MFC [Configuration Ini File Path]\n"));
    _tprintf(_T("Could not read any directory settings from the ini file\n"));
    return FALSE; 
  }

  settings.FreeDirectoryArray();
  for (int i=0; i<nDirectories; i++)
  {
    //Create each directory. Client samples could instantiate a derived version
    //to customize the functionality
    CW3MFCDirectory* pDirectory = new CW3MFCDirectory;
    
    CString sSection;
    sSection.Format(_T("Dir%d"), i);

    CString sAlias(GetProfileString(sSection, _T("Alias"), _T("")));
    pDirectory->SetAlias(sAlias);

    CString sDirectory(GetProfileString(sSection, _T("Path"), _T("")));
    pDirectory->SetDirectory(sDirectory);

    CString sDefaultFile(GetProfileString(sSection, _T("DefaultFile"), _T("index.html")));
    pDirectory->SetDefaultFile(sDefaultFile);

    BOOL bDirectoryListing = static_cast<BOOL>(GetProfileInt(sSection, _T("DirectoryListing"), FALSE));
    pDirectory->SetDirectoryListing(bDirectoryListing);

    BOOL bWritable = static_cast<BOOL>(GetProfileInt(sSection, _T("Writable"), FALSE));
    pDirectory->SetWritable(bWritable);

    BOOL bScript = static_cast<BOOL>(GetProfileInt(sSection, _T("Script"), FALSE));
    pDirectory->SetScript(bScript);

    CString sUsername(GetProfileString(sSection, _T("Username"), _T("")));
    pDirectory->SetUsername(sUsername);

    CString sPassword(GetProfileString(sSection, _T("Password"), _T("")));
    pDirectory->SetPassword(sPassword);

    CString sRealm(GetProfileString(sSection, _T("Realm"), _T("")));
    pDirectory->SetRealm(sRealm);

    settings.m_Directories.SetAtGrow(i, pDirectory);
  }

  //Do not use port 80 for this sample app. Helps avoid any possible
  //conflict with exisiting web servers which may be running
  settings.m_nPort = static_cast<unsigned short>(GetProfileInt(_T("General"), _T("Port"), 90));

  //Comment out the following line to not do reverse DNS lookups on client connections
  settings.m_bDNSLookup = GetProfileInt(_T("General"), _T("DNSLookup"), FALSE);

  //Should we bind to an IP address
  CString sAddress = GetProfileString(_T("General"), _T("BindAddress"), _T(""));
  if (!sAddress.IsEmpty())
  {
    settings.m_bBind = TRUE;
    settings.m_sBindAddress = sAddress;
  }

  //Read in the IPv6 setting
  settings.m_bIPv6 = GetProfileInt(_T("General"), _T("IPv6"), settings.m_bIPv6);

  //All the other misc settings
  settings.m_dwIdleClientTimeout = GetProfileInt(_T("General"), _T("Timeout"), settings.m_dwIdleClientTimeout);
  settings.m_sServerName = GetProfileString(_T("General"), _T("ServerName"), settings.m_sServerName);
  settings.m_sUserName = GetProfileString(_T("General"), _T("Username"), settings.m_sUserName);
  settings.m_sPassword = GetProfileString(_T("General"), _T("Password"), settings.m_sPassword);
  settings.m_nThreadPoolSize = GetProfileInt(_T("General"), _T("ThreadPoolSize"), settings.m_nThreadPoolSize);
  settings.m_nThreadPoolQueueSize = GetProfileInt(_T("General"), _T("ThreadPoolQueueSize"), settings.m_nThreadPoolQueueSize);
  settings.m_dwWritableTimeout = GetProfileInt(_T("General"), _T("WritableTimeout"), settings.m_dwWritableTimeout);
  settings.m_bEnableThreadLifetime = GetProfileInt(_T("General"), _T("EnableThreadLifetime"), settings.m_bEnableThreadLifetime);
  settings.m_dwThreadLifetime = GetProfileInt(_T("General"), _T("ThreadLifetime"), settings.m_dwThreadLifetime);
  settings.m_bUseIOCPQueue = GetProfileInt(_T("General"), _T("UseIOCPQueue"), settings.m_bUseIOCPQueue);
  settings.m_bAutoExpire = static_cast<BOOL>(GetProfileInt(_T("General"), _T("AutoExpire"), settings.m_bAutoExpire));
  settings.m_bAllowDeleteRequest = static_cast<BOOL>(GetProfileInt(_T("General"), _T("AllowDeletes"), settings.m_bAllowDeleteRequest));
#ifndef W3MFC_NO_CGI_SUPPORT
  settings.m_dwCGIResponseBufferSize = GetProfileInt(_T("General"), _T("CGIResponseBufferSize"), settings.m_dwCGIResponseBufferSize);
#endif //#ifndef W3MFC_NO_CGI_SUPPORT
  settings.m_bAllowAnonymous                   = static_cast<BOOL>(GetProfileInt(_T("General"), _T("AllowAnonymous"), settings.m_bAllowAnonymous));
  settings.m_bAllowBasicAuthentication         = static_cast<BOOL>(GetProfileInt(_T("General"), _T("AllowBasicAuthentication"), settings.m_bAllowBasicAuthentication));
  settings.m_bPerformPassthroughAuthentication = static_cast<BOOL>(GetProfileInt(_T("General"), _T("PerformPassthroughAuthentication"), settings.m_bPerformPassthroughAuthentication));
  settings.m_nListenThreadStackSize            = GetProfileInt(_T("General"), _T("ListenThreadStackSize"), settings.m_nListenThreadStackSize);

#ifdef W3MFC_SSL_SUPPORT
  settings.m_bSSL                    = static_cast<BOOL>(GetProfileInt(_T("SSL"), _T("Enabled"), settings.m_bSSL));
  settings.m_sServerCertificateName  = GetProfileString(_T("SSL"), _T("ServerCertificateName"), settings.m_sServerCertificateName);
  settings.m_dwSSLEnabledProtocols   = GetProfileInt(_T("SSL"), _T("Protocols"), settings.m_dwSSLEnabledProtocols);
  settings.m_dwSSLReadTimeout        = GetProfileInt(_T("SSL"), _T("ReadTimeout"), settings.m_dwSSLReadTimeout);
  settings.m_dwSSLWriteTimeout       = GetProfileInt(_T("SSL"), _T("WriteTimeout"), settings.m_dwSSLWriteTimeout);
#endif //#ifdef W3MFC_SSL_SUPPORT

  //Initialize the Mime manager we are going to use with a mime map as taken from the ini file
  CW3MFCIniMimeManager iniMimeManager;
  int nSizeOfMap = iniMimeManager.Initialize(m_pszProfileName, _T("Mime"));
  if (nSizeOfMap == 0)
  {
    _tprintf(_T("Failed to read any MIME map settings from ini file\n"));
    return FALSE; 
  }
  settings.m_pMimeManager = &iniMimeManager;

#ifndef W3MFC_NO_ISAPI_SUPPORT
  //Initialize the ISAPI manager we are going to use with a extension map as taken from the ini file
  settings.m_bCacheISAPI                = static_cast<BOOL>(GetProfileInt(_T("ISAPI"), _T("Cache"), settings.m_bCacheISAPI));
  CW3MFCIniISAPIManager iniISAPIManager;
  iniISAPIManager.Initialize(m_pszProfileName, _T("ISAPIMap"));
  settings.m_pISAPIManager = &iniISAPIManager;

  //Also initialize the ISAPI implementation
  int nISAPIHashTableSize = GetProfileInt(_T("ISAPI"), _T("HashTableSize"), 7929); 
  CW3MFCISAPI isapi(nISAPIHashTableSize);
  settings.m_pISAPI = &isapi;
#endif //#ifndef W3MFC_NO_ISAPI_SUPPORT

  //And the CGI implementation
#ifndef W3MFC_NO_CGI_SUPPORT
  CW3MFCCGI cgi;
  settings.m_pCGI = &cgi;
#endif //#ifndef W3MFC_NO_CGI_SUPPORT

  //and start it up
  _tprintf(_T("Web server is starting...\n"));
  CMyHttpServerIOCP theWebServerIOCP;
  CMyHttpServerDirected theWebServerDirected;
  g_pTheWebServerIOCP = &theWebServerIOCP;
  g_pTheWebServerDirected = &theWebServerDirected;
  if (settings.m_bUseIOCPQueue ? !theWebServerIOCP.Start(&settings) : !theWebServerDirected.Start(&settings))
  {
    _tprintf(_T("Failed to start up the web server, Error:%u\n"), ::GetLastError());
    return FALSE;
  }

  //Display some additional info when the web servering is starting
  _tprintf(_T("Listening for incoming connections on port %d\n"), static_cast<int>(settings.m_nPort));
  if (settings.m_bDNSLookup)
    _tprintf(_T("Reverse DNS lookups will be performed on client requests\n"));
  if (settings.m_bBind)
  {
  #ifdef CWSOCKET_MFC_EXTENSIONS
    _tprintf(_T("Binding to local address: %s\n"), settings.m_sBindAddress.GetString());
  #else
    _tprintf(_T("Binding to local address: %s\n"), settings.m_sBindAddress.c_str());
  #endif //#ifdef CWSOCKET_MFC_EXTENSIONS
  }
  if (settings.m_bIPv6)
    _tprintf(_T("Will default to IPv6 if explicit local address is not specified\n"));
  else
    _tprintf(_T("Will default to IPv4 if explicit local address is not specified\n"));
  _tprintf(_T("Client idle timeout: %u ms\n"), settings.m_dwIdleClientTimeout);
  _tprintf(_T("Client write timeout: %u ms\n"), settings.m_dwWritableTimeout);
  _tprintf(_T("Configured Server Name: %s\n"), settings.m_sServerName.GetString());
  _tprintf(_T("Thread pool size: %d\n"), settings.m_nThreadPoolSize);
  _tprintf(_T("Thread pool queue size: %d\n"), settings.m_nThreadPoolQueueSize);
  _tprintf(_T("Listen thread stack size: %d\n"), settings.m_nListenThreadStackSize);
  if (settings.m_bEnableThreadLifetime)
    _tprintf(_T("Threads in the thread pool will be recycled every %u minute(s)\n"), settings.m_dwThreadLifetime);
  if (settings.m_bUseIOCPQueue)
    _tprintf(_T("Using IOCP Thread pool queue\n"));
  else
    _tprintf(_T("Using Directed Thread pool queue\n"));
  if (settings.m_bAutoExpire)
    _tprintf(_T("Reponses will be marked with an immediate \"Expires\" header\n"));
  if (settings.m_bAllowDeleteRequest)
    _tprintf(_T("HTTP DELETE verb will be allowed\n"));
  else
    _tprintf(_T("HTTP DELETE verb will not be allowed\n"));
#ifndef W3MFC_NO_CGI_SUPPORT
  _tprintf(_T("CGI Response Buffer size: %u\n"), settings.m_dwCGIResponseBufferSize);
#else
  _tprintf(_T("CGI support is disabled in this configuration\n"));
#endif //#ifndef W3MFC_NO_CGI_SUPPORT

#ifndef W3MFC_NO_ISAPI_SUPPORT
  _tprintf(_T("ISAPI support is provided in this configuration\n"));
  if (settings.m_bCacheISAPI)
    _tprintf(_T("ISAPI dlls will be cached in memory\n"));
  else
    _tprintf(_T("ISAPI dlls will not be cached in memory\n"));
#else
  _tprintf(_T("ISAPI support is disabled in this configuration\n"));
#endif //#ifndef W3MFC_NO_ISAPI_SUPPORT

  if (settings.m_bAllowAnonymous)
    _tprintf(_T("Anonymous Connections allowed\n"));
  else
    _tprintf(_T("Anonymous Connections are not allowed\n"));
  if (settings.m_bAllowBasicAuthentication)
    _tprintf(_T("Basic Authentication allowed\n"));
  if (settings.m_bPerformPassthroughAuthentication)
    _tprintf(_T("Passthrough Authentication will be allowed\n"));
  else
    _tprintf(_T("Passthrough Authentication is not allowed\n"));

#ifdef W3MFC_SSL_SUPPORT
  if (settings.m_bSSL)
    _tprintf(_T("Server certificate name is %s\n"), settings.m_sServerCertificateName.GetString());
#endif //#ifdef W3MFC_SSL_SUPPORT

  //Check that everything in the thread pool also started up ok
  if (settings.m_bUseIOCPQueue ? !theWebServerIOCP.GetThreadPool().WaitForThreadsInitInstance() : !theWebServerDirected.GetThreadPool().WaitForThreadsInitInstance())
    return FALSE;

  //Register the console handler to allow the program to be gracefully terminated
  if (!SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE))
  {
    _tprintf(_T("Failed to set Console Ctrl Handler\n"));
    return FALSE;
  }

  //Wait until the server finishes
  if (settings.m_bUseIOCPQueue)
    theWebServerIOCP.Wait();
  else
    theWebServerDirected.Wait();
  _tprintf(_T("Web server has shut down...\n"));

  return FALSE;
}   

int CW3MFCApp::ExitInstance()
{
  //Close down sockets
  WSACleanup();

  //Let the base class do its thing
  return __super::ExitInstance();
}

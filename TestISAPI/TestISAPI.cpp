#include "stdafx.h"
#include "TestISAPI.h"

CWinApp theApp;

BEGIN_PARSE_MAP(CTestISAPIExtension, CHttpServer)
  // TODO: insert your ON_PARSE_COMMAND() and 
  // ON_PARSE_COMMAND_PARAMS() here to hook up your commands.
  // For example:
  ON_PARSE_COMMAND(Default, CTestISAPIExtension, ITS_EMPTY)
  DEFAULT_PARSE_COMMAND(Default, CTestISAPIExtension)
END_PARSE_MAP(CTestISAPIExtension)

CTestISAPIExtension theExtension;

CTestISAPIExtension::CTestISAPIExtension()
{
}

CTestISAPIExtension::~CTestISAPIExtension()
{
}

BOOL CTestISAPIExtension::GetExtensionVersion(HSE_VERSION_INFO* pVer)
{
  //Call default implementation for initialization
  CHttpServer::GetExtensionVersion(pVer);

  //Load description string
  TCHAR sz[HSE_MAX_EXT_DLL_NAME_LEN+1];
  ISAPIVERIFY(::LoadString(AfxGetResourceHandle(), IDS_SERVER, sz, HSE_MAX_EXT_DLL_NAME_LEN));
  _tcscpy_s(pVer->lpszExtensionDesc, sizeof(pVer->lpszExtensionDesc)/sizeof(TCHAR), sz);
  return TRUE;
}

BOOL CTestISAPIExtension::TerminateExtension(DWORD /*dwFlags*/)
{
  return TRUE;
}

void CTestISAPIExtension::Default(CHttpServerContext* pCtxt)
{
  //Try out all the GetServerVariable calls
  DWORD dwSize = 0;
  pCtxt->GetServerVariable(_T("SERVER_PORT"), NULL, &dwSize);
  char pszVariable[256];
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("SERVER_PORT"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("SERVER_PORT_SECURE"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("SERVER_PROTOCOL"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("SERVER_SOFTWARE"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("PATH_INFO"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("REMOTE_ADDR"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("REMOTE_HOST"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("UNKNOWN"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("REQUEST_METHOD"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("SERVER_NAME"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("HTTPS"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("AUTH_TYPE"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("AUTH_PASSWORD"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("REMOTE_SOFTWARE"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("AUTH_REALM"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("CONTENT_LENGTH"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("LOGON_USER"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("HTTPS_SERVER_SUBJECT"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("CERT_SERVER_SUBJECT"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("HTTPS_SERVER_ISSUER"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("CERT_SERVER_ISSUER"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("CERT_SERIALNUMBER"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("HTTPS_SECRETKEYSIZE"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("CERT_SECRETKEYSIZE"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("HTTPS_KEYSIZE"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("CERT_KEYSIZE"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("PATH_TRANSLATED"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("SCRIPT_NAME"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("REQUEST_LINE"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("ALL_HTTP"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("ALL_RAW"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("HTTP_VERSION"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("HTTP_HOST"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("HTTP_ACCEPT"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("URL"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("QUERY_STRING"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("REMOTE_USER"), pszVariable, &dwSize);
  dwSize = sizeof(pszVariable);
  pCtxt->GetServerVariable(_T("AUTH_USER"), pszVariable, &dwSize);

  dwSize = sizeof(pszVariable);
  pCtxt->ReadClient(pszVariable, &dwSize);

  //pCtxt->ServerSupportFunction(HSE_REQ_CLOSE_CONNECTION, NULL, NULL, NULL);

  HANDLE hToken;
  pCtxt->ServerSupportFunction(HSE_REQ_GET_IMPERSONATION_TOKEN, &hToken, NULL, NULL);

  //pCtxt->ServerSupportFunction(0xffff1234, NULL, NULL, NULL);
  
  CtxtHandle hImpersonation;
  CredHandle credHandle;
  pCtxt->ServerSupportFunction(HSE_REQ_GET_SSPI_INFO, &hImpersonation, NULL, reinterpret_cast<LPDWORD>(&credHandle));

  BOOL bKeepAlive = FALSE;
  pCtxt->ServerSupportFunction(HSE_REQ_IS_KEEP_CONN, &bKeepAlive, NULL, NULL);

  char pszURL[_MAX_PATH];
  strcpy_s(pszURL, sizeof(pszURL), "/w3mfc.gif");
  HSE_URL_MAPEX_INFO umi;
  dwSize = _MAX_PATH; 
  pCtxt->ServerSupportFunction(HSE_REQ_MAP_URL_TO_PATH, pszURL, &dwSize, reinterpret_cast<LPDWORD>(&umi));
  strcpy_s(pszURL, sizeof(pszURL), "/w3mfc.gif");
  dwSize = static_cast<DWORD>(strlen(pszURL));
  pCtxt->ServerSupportFunction(HSE_REQ_MAP_URL_TO_PATH_EX, pszURL, &dwSize, reinterpret_cast<LPDWORD>(&umi));

  //LPSTR pszRedirect = "http://localhost/index.html";
  //pCtxt->ServerSupportFunction(HSE_REQ_SEND_URL, pszRedirect, NULL, NULL);

  //pCtxt->ServerSupportFunction(HSE_REQ_SEND_URL_REDIRECT_RESP, pszRedirect, NULL, NULL);

  //HSE_SEND_HEADER_EX_INFO shei;
  //shei.pszStatus = "200 OK";
  //shei.cchHeader = strlen(shei.pszStatus);
  //shei.fKeepConn = TRUE;
  //shei.pszHeader = _T("x-Header: PJ\r\n");
  //pCtxt->ServerSupportFunction(HSE_REQ_SEND_RESPONSE_HEADER_EX, &shei, NULL, NULL);

  StartContent(pCtxt);
  WriteTitle(pCtxt);

  *pCtxt << _T("This default message was produced by the Internet");
  *pCtxt << _T(" Server DLL Wizard. Edit your CTestISAPIExtension::Default()");
  *pCtxt << _T(" implementation to change it.\r\n");

  EndContent(pCtxt);
}

// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CTestISAPIExtension, CHttpServer)
  //{{AFX_MSG_MAP(CTestISAPIExtension)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0



///////////////////////////////////////////////////////////////////////
// If your extension will not use MFC, you'll need this code to make
// sure the extension objects can find the resource handle for the
// module.  If you convert your extension to not be dependent on MFC,
// remove the comments arounn the following AfxGetResourceHandle()
// and DllMain() functions, as well as the g_hInstance global.

/****

static HINSTANCE g_hInstance;

HINSTANCE AFXISAPI AfxGetResourceHandle()
{
  return g_hInstance;
}

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ulReason,
          LPVOID lpReserved)
{
  if (ulReason == DLL_PROCESS_ATTACH)
  {
    g_hInstance = hInst;
  }

  return TRUE;
}

****/

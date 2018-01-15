#pragma once

#ifndef STRICT
#define STRICT
#endif

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0600
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#define VC_EXTRALEAN //Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS //some CString constructors will be explicit
#define _AFX_ALL_WARNINGS //turns off MFC's hiding of some common and often safely ignored warning messages
#define _ATL_NO_AUTOMATIC_NAMESPACE

#ifndef _SECURE_ATL
#define _SECURE_ATL 1 //Use the Secure C Runtime in ATL
#endif

#include <afxext.h> 
#include <winsock2.h>
#include <afxmt.h>
#include <afxtempl.h>
#include <atlenc.h>
#include <atlsocket.h>
#include <atlutil.h>
#include <string>

#ifndef W3MFC_NO_ISAPI_SUPPORT
  #include <HttpExt.h>
#endif

#ifndef W3MFC_NO_SSL_SUPPORT
  #include <sspi.h>
  #include <schannel.h>
  #include <cryptuiapi.h>
  #include <vector>
#endif

#ifndef CWSOCKET_MFC_EXTENSIONS
#include <exception>
#endif //#ifndef CWSOCKET_MFC_EXTENSIONS

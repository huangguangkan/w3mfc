#pragma once

//We cannot define STRICT here because afxisapi.h defines it without checking to see if it is not already defined!
//#ifndef STRICT
//#define STRICT
//#endif

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

#include <afxwin.h>
#include <afxmt.h>
#include <afxext.h>

#if _MSC_VER > 1400
  #error TestISAPI project is only supported on VC 2005 as MFC support for ISAPI was dropped in VC 2008
#endif
#include <afxisapi.h>
#include <sspi.h>

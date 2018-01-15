/*
Module : W3MFCError.h
Purpose: Defines the interface for the IW3MFCError class
Created: PJN / 22-04-1999

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

#ifndef __W3MFCERROR_H__
#define __W3MFCERROR_H__


/////////////////////////////// Includes //////////////////////////////////////

#ifndef __AFX_H__
#pragma message("To avoid this message, please put the MFC header files in your pre compiled header (normally stdafx.h)")
#include <afx.h>
#endif //#ifndef __AFX_H__


/////////////////////////////// Classes ///////////////////////////////////////

//Simply COM style error interface
class IW3MFCError
{
public:
  virtual void OnError(const CString& sError) = 0;
};

#endif //#ifndef __W3MFCERROR_H__

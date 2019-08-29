#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

// Macro for getting size of an array
#define SIZEOF( a ) sizeof( a ) / sizeof( a[0] )

class CNoFaceLockApp : public CWinApp
{
	public:
		CNoFaceLockApp();

	// Overrides
	public:
		virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CNoFaceLockApp theApp;
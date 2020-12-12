// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <WinSDKVer.h>
#include <Windows.h>
#include <InitGuid.h>
#include <mfapi.h>
#include <Mfidl.h>
#include <wincodec.h>
#include <mfreadwrite.h>
#include <mfobjects.h>
#include <Dbt.h>
#include <ks.h>
#include <ksmedia.h>

#define SafeRelease(x) {\
if(x)\
{\
	x->Release();\
	x = NULL;\
} \
}


// TODO: reference additional headers your program requires here

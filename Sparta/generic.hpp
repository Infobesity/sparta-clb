#pragma once

#pragma comment( linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' \
		version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define PRINT_STATUS 1

#if (PRINT_STATUS == 1)
#define PRINT_DEBUG_INFO
#endif

#ifdef UNICODE
#undef UNICODE
#endif

#ifdef _UNICODE
#undef _UNICODE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _STL_SECURE_NO_WARNINGS
#define _STL_SECURE_NO_WARNINGS
#endif

#ifndef _XDEF_NEXONLOGIN
#define _XDEF_NEXONLOGIN
#endif

/* important includes */
#include <WinSock2.h>
#include <Windows.h>

#include <CommCtrl.h>
#include <WindowsX.h>

#include <tchar.h>
#include <strsafe.h>

#include <algorithm>
#include <functional>
#include <memory>

#include <iostream>
#include <sstream>
#include <string>

#include <thread>
#include <vector>

/* important libraries */
#pragma comment(lib, "ws2_32")

/* preprocessor macros */
#ifndef Padding
#define Padding(x) struct { unsigned char __Padding##x[(x)]; };
#endif

#ifndef WM_SOCKET
#define WM_SOCKET WM_USER + 100
#endif

/* generic functions */
namespace my_time
{
	std::string get_time();
}

namespace utility
{
	std::string upper(std::string const& string);
	std::string lower(std::string const& string);
	
	template<class T>
	std::string delimited_numeric(T value)
	{
		std::stringstream ss;
		ss.imbue(std::locale(""));
		ss << std::fixed << value;
		return ss.str();
	}
}
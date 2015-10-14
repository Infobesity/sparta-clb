#include "generic.hpp"
#include "sparta_form.hpp"

#include "packet_crypto.hpp"

#include "nexon_api\NMCOClientObject.h"
#include "nexon_api\NMManager.h"

namespace nm
{
	bool initialize_authentication()
	{
		CNMManager::GetInstance().Init();
	
		if (!CNMCOClientObject::GetInstance().SetLocale(kLocaleID_EU, kRegionCode_EU3))
		{
			return false;
		}
	
		if (!CNMCOClientObject::GetInstance().Initialize(kGameCode_maplestory_eu))
		{
			return false;
		}

		//HWND hwnd_main_window = NULL;

		//if (!CNMCOClientObject::GetInstance().RegisterCallbackMessage(hwnd_main_window, WM_USER + 0x11))
		//{
		//	return false;
		//}

		return true;
	}
}

namespace wsa
{
	bool initialize()
	{
		WSADATA wsa_data;
		memset(&wsa_data, 0, sizeof(WSADATA));

		return (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0 && wsa_data.wVersion == MAKEWORD(2, 2));
	}

	bool uninitialize()
	{
		return (WSACleanup() == 0);
	}
}

int __stdcall WinMain(HINSTANCE instance, HINSTANCE prev_instance, char* command_line, int command_show)
{
	HANDLE host_mutex = CreateMutex(NULL, FALSE, "sparta_mtx");

	if (!host_mutex || host_mutex == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	else if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}
	
	if (!nm::initialize_authentication())
	{
		ReleaseMutex(host_mutex);
		return 0;
	}
	
	if (!wsa::initialize())	
	{
		ReleaseMutex(host_mutex);
		return 0;
	}

#ifdef PRINT_DEBUG_INFO
	AllocConsole();
	SetConsoleTitle("Terminal");
	AttachConsole(GetCurrentProcessId());
	
	FILE* pFile = nullptr;
	freopen_s(&pFile, "CON", "r", stdin);
	freopen_s(&pFile, "CON", "w", stdout);
	freopen_s(&pFile, "CON", "w", stderr);
#endif

	try
	{
		sparta::gui::sparta_form::get_instance().show(true);
		sparta::execute();
	}
	catch (std::exception& exception)
	{
		MessageBox(0, exception.what(), "An exception occured!", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
	}
	
#ifdef PRINT_DEBUG_INFO
	FreeConsole();
#endif

	wsa::uninitialize();
	ReleaseMutex(host_mutex);
	return 0;
}
#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"NtYieldExecution";
}

__declspec(dllexport) char* __cdecl PluginVersion(void)
{
	return __DATE__;
}

__declspec(dllexport) TCHAR* __cdecl PluginErrorMessage(void)
{
	return sErrorMessage;
}

__declspec(dllexport) DWORD __cdecl PluginDebugCheck(int iWinVer)
{
	typedef NTSTATUS (WINAPI *def_NtYieldExecution)();
	def_NtYieldExecution _NtYieldExecution = 0;

	int iCounter = 0x20,
		iDebugged = 0;

	HMODULE hModuleNtdll = GetModuleHandleW(TEXT("ntdll.dll"));
	if (!hModuleNtdll)
	{
		sErrorMessage = TEXT("ntdll handle not found");
		return -1;
	}

	_NtYieldExecution = (def_NtYieldExecution)GetProcAddress(hModuleNtdll, "NtYieldExecution");
	if (!_NtYieldExecution)
	{
		sErrorMessage = TEXT("NtYieldExecution not found");
		return -1;
	}

	do
	{
		Sleep(0xF);
		if (_NtYieldExecution() != _STATUS_NO_YIELD_PERFORMED)
			iDebugged++;
		iCounter--;
	} while (iCounter > 0);

	if (iDebugged <= 1)
		return 0;
	else
		return 1;
}
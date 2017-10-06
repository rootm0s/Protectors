#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"NtSetDebugFilterState";
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
	typedef NTSTATUS (WINAPI *pNtSetDebugFilterState)(DWORD,DWORD,bool); 

	DWORD NoDebugInherit = 0;
	NTSTATUS Status = 0; 

	HMODULE hNTDLL = GetModuleHandle(L"ntdll.dll");
	if(hNTDLL == INVALID_HANDLE_VALUE)
	{
		sErrorMessage = TEXT("Failed to load ntdll");
		return -1;
	}

	pNtSetDebugFilterState NtSDFS = (pNtSetDebugFilterState)GetProcAddress(hNTDLL,"NtSetDebugFilterState"); 
	if(NtSDFS == NULL)
	{
		sErrorMessage = TEXT("Failed to load NtQueryInformationProcess");
		return -1;
	}

	Status = NtSDFS(0,0,true); 
	if (Status == 0x00000000L)
		return 1;
	else
		return 0;
}
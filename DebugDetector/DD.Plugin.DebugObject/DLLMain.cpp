#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"DebugObject";
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
	typedef NTSTATUS (WINAPI *pNtQueryInformationProcess)(HANDLE,UINT,PVOID,ULONG,PULONG); 

	HANDLE hDebugObject = NULL;
	NTSTATUS Status; 

	HMODULE hNTDLL = GetModuleHandle(L"ntdll.dll");
	if(hNTDLL == INVALID_HANDLE_VALUE)
	{
		sErrorMessage = TEXT("Failed to load ntdll");
		return -1;
	}

	pNtQueryInformationProcess NtQIP = (pNtQueryInformationProcess)GetProcAddress(hNTDLL,"NtQueryInformationProcess"); 
	if(NtQIP == NULL)
	{
		sErrorMessage = TEXT("Failed to load NtQueryInformationProcess");
		return -1;
	}

	Status = NtQIP(GetCurrentProcess(),0x1e,&hDebugObject,4,NULL); 
	if (Status != 0x00000000)
		return 0;

	if(hDebugObject)
		return 1;
	else
		return 0;
}
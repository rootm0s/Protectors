#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"ProcessDebugFlags";
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
	typedef NTSTATUS (WINAPI *pNtQueryInformationProcess)(HANDLE ,UINT ,PVOID ,ULONG , PULONG); 

	DWORD NoDebugInherit = 0;
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

	Status = NtQIP(GetCurrentProcess(),0x1f,&NoDebugInherit,4,NULL); 
	if (Status != 0x00000000)
	{
		sErrorMessage = TEXT("Error in NtQueryInformationProcess");
		return -1; 
	}

	if(NoDebugInherit == FALSE)
		return 1;
	else
		return 0;
}
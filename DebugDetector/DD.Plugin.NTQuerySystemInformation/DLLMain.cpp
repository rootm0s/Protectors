#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"NtQuerySystemInformation";
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
	if(iWinVer >= 0 && iWinVer < 2)
	{
		typedef NTSTATUS (WINAPI *pNtQuerySystemInformation)(ULONG, PVOID, ULONG, PULONG);

		typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION 
		{
			bool DebuggerEnabled;
			bool DebuggerNotPresent;
		} SYSTEM_KERNEL_DEBUGGER_INFORMATION, *PSYSTEM_KERNEL_DEBUGGER_INFORMATION;

		NTSTATUS Status = 0; 
		SYSTEM_KERNEL_DEBUGGER_INFORMATION DebuggerInfo;

		HMODULE hNTDLL = GetModuleHandle(L"ntdll.dll");
		if(hNTDLL == INVALID_HANDLE_VALUE)
		{
			sErrorMessage = TEXT("Failed to load ntdll");
			return -1;
		}

		pNtQuerySystemInformation NtQSI = (pNtQuerySystemInformation)GetProcAddress(hNTDLL,"NtQuerySystemInformation"); 
		if(NtQSI == NULL)
		{
			sErrorMessage = TEXT("Failed to load NtQuerySystemInformation");
			return -1;
		}

		Status = NtQSI(0x23,(PVOID)&DebuggerInfo,sizeof(DebuggerInfo),NULL); 
		if (Status != 0x00000000)
		{
			sErrorMessage = TEXT("Error in NtQuerySystemInformation");
			return -1; 
		}

		if(DebuggerInfo.DebuggerNotPresent == false || DebuggerInfo.DebuggerEnabled == true)
			return 1;
		else
			return 0;
	}
	else
	{
		sErrorMessage = TEXT("Only <= WinXP");
		return -1;
	}
}
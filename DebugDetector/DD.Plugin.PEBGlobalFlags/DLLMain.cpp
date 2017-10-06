#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"PEBGlobalFlag";
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
	__asm
	{
		mov eax, fs:[18h]			;TEB
		mov eax, [eax + 30h]		;PEB
		mov eax, [eax + 68h]		;NtGlobalFlag
		cmp eax,70h
		je DebuggerDetected
	}

	return 0;
	__asm{DebuggerDetected:}
	return 1;

	//PROCESS_BASIC_INFORMATION pPBI;
	//HANDLE hDebugObject = NULL;
	//NTSTATUS Status; 
	//typedef NTSTATUS (WINAPI *pNtQueryInformationProcess)(HANDLE,UINT,PVOID,ULONG,PULONG); 

	//pNtQueryInformationProcess NtQIP = (pNtQueryInformationProcess)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtQueryInformationProcess"); 

	//Status = NtQIP(GetCurrentProcess(),0,&pPBI,sizeof(PROCESS_BASIC_INFORMATION),NULL); 

	//if (Status == 0x00000000)
	//{
	//	char* voFlag = (char*)pPBI.PebBaseAddress + 0x68;

	//	if(*voFlag == 0x70)
	//		return true;
	//	else
	//		return false;
	//}
	//return false;
}
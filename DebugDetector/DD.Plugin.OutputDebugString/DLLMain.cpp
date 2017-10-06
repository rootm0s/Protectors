#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"OutputDebugString";
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
	char szMessage[] = "DebugCheck";

	__asm
	{
		xor eax,eax
		lea ebx, szMessage
		push ebx
		call dword ptr OutputDebugStringA
		cmp eax,1
		ja DebuggerFound
	}
	return 0;
	
	__asm{DebuggerFound:}
	return 1;
}
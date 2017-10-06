#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"PEBProcHeapFlag";
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
	if(iWinVer == 3)
	{
		__asm
		{
			mov eax, fs:[18h]			;TEB
			mov eax, [eax + 30h]		;PEB
			mov eax, [eax + 18h]		;process heap
			cmp [eax + 44h],0			;heap force flags //seems changed on win7  xp = 10h
			jne DebuggerDetected
		}
	}
	else if (iWinVer < 3)
	{
		__asm
		{
			mov eax, fs:[18h]			;TEB
			mov eax, [eax + 30h]	;PEB
			mov eax, [eax + 18h]	;process heap
			cmp [eax + 10h],0		;heap force flags //seems changed on win7  xp = 10h
			jne DebuggerDetected
		}
	}
	return 0;
	__asm{DebuggerDetected:}
	return 1;
}
#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"Hardware Breakpoints";
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
	HANDLE hThread = GetCurrentThread();
	CONTEXT cTT; 
	bool bDebugged = false;

	ZeroMemory(&cTT,sizeof(CONTEXT));
	cTT.ContextFlags = CONTEXT_ALL;

	if(!GetThreadContext(hThread,&cTT))
	{
		sErrorMessage = TEXT("GetThreadContext failed!");
		return -1;
	}

	if(cTT.Dr0 != NULL)
		bDebugged = true;
	if(cTT.Dr1 != NULL)
		bDebugged = true;
	if(cTT.Dr2 != NULL)
		bDebugged = true;
	if(cTT.Dr3 != NULL)
		bDebugged = true;

	if(bDebugged)
		return 1;
	else
		return 0;
}
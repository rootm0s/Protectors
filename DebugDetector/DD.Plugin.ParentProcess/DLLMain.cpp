#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"ParentProcess";
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
	HANDLE hProcessSnap = NULL,hProc = NULL;
	PROCESSENTRY32W pe32w;
	DWORD dwThis = 0,dwExplorer = 0;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(hProcessSnap != INVALID_HANDLE_VALUE)
	{
		pe32w.dwSize = sizeof(PROCESSENTRY32W);
		if(Process32First(hProcessSnap,&pe32w))
		{
			do
			{
				if(wcsstr(pe32w.szExeFile,L"explorer.exe") != NULL)
					dwExplorer = pe32w.th32ProcessID;
				else if(pe32w.th32ProcessID == GetCurrentProcessId())
					dwThis = pe32w.th32ParentProcessID;
			} while(Process32Next(hProcessSnap,&pe32w));
		}
		CloseHandle(hProcessSnap);
	}
	else
	{
		sErrorMessage = TEXT("CreateToolhelp32Snapshot: invalid handle");
		return -1;
	}

	if(dwThis == dwExplorer)
		return 0;
	else
		return 1;
}
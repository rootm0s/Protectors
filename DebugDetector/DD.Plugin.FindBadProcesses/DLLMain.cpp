#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"BadProcessList";
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
	vector<wstring> vProcList;
	
	vProcList.push_back(L"ollydbg.exe");
	vProcList.push_back(L"windbg.exe");
	vProcList.push_back(L"devenv.exe");
	vProcList.push_back(L"ImmunityDebugger.exe");
	vProcList.push_back(L"idaq.exe");

	HANDLE hProcessSnap = NULL,hProc = NULL;
	PROCESSENTRY32W pe32w;
	bool bDebugged = false;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(hProcessSnap != INVALID_HANDLE_VALUE)
	{
		pe32w.dwSize = sizeof(PROCESSENTRY32W);
		if(Process32First(hProcessSnap,&pe32w))
		{
			do
			{
				for(size_t i = 0; i < vProcList.size(); i++)
					if(wcsstr(wcsupr(pe32w.szExeFile),wcsupr((TCHAR*)vProcList[i].c_str())) != NULL)
						bDebugged = true;
			} while(Process32Next(hProcessSnap,&pe32w));
		}
		CloseHandle(hProcessSnap);
	}
	else
	{
		sErrorMessage = TEXT("CreateToolhelp32Snapshot: invalid handle");
		return -1;
	}
	
	if(bDebugged)
		return 1;
	else
		return 0;
}
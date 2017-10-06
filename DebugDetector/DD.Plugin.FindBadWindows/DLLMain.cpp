#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"BadWindowsList";
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
	bool bDebugged = false;
	if(!EnumWindows(EnumWindowsProc,(LPARAM)&bDebugged))
	{
		sErrorMessage = TEXT("EnumWindows failed!");
		return -1;
	}

	if(bDebugged)
		return 1;
	else
		return 0;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
	vector<wstring> vWindowList;
	TCHAR* sTitel = (TCHAR*)malloc(MAX_PATH);
	bool* bDebugged = (bool*)lParam;

	vWindowList.push_back(L"Immunity Debugger");
	vWindowList.push_back(L"IDA Pro");
	vWindowList.push_back(L"Olly");
	vWindowList.push_back(L"- [CPU]");
	vWindowList.push_back(L"PhantOm");
	vWindowList.push_back(L"o_O -");
	vWindowList.push_back(L"Visual Studio");
	vWindowList.push_back(L"WinDbgFrameClass");

	GetWindowText(hwnd,sTitel,MAX_PATH);

	for(size_t i = 0;i < vWindowList.size(); i++)
	{
		if(wcsstr(sTitel,vWindowList[i].c_str()) != NULL)
			*bDebugged = true;
	}
	free(sTitel);
	return true;
}
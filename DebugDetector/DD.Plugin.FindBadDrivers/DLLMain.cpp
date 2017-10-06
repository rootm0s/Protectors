#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"BadDriversList";
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
	LPVOID lpDrivers[1024];
	DWORD cbNeeded = 0;
	int cDrivers = 0;
	vector<wstring> vDriverList;

	vDriverList.push_back(L"olly.sys");

	if(EnumDeviceDrivers(lpDrivers,sizeof(lpDrivers),&cbNeeded) && cbNeeded < sizeof(lpDrivers))
	{ 
		TCHAR szDriver[1024];

		cDrivers = cbNeeded / sizeof(lpDrivers[0]);

		for (int i = 0; i < cDrivers; i++ )
		{
			if(GetDeviceDriverBaseName(lpDrivers[i],szDriver,sizeof(szDriver) / sizeof(szDriver[0])))
			{
				for(size_t a = 0;a < vDriverList.size(); a++)
					if(wcsstr(szDriver,vDriverList[a].c_str()) != NULL)
						return 1;
			}
		}
	}
	else
	{
		sErrorMessage = TEXT("EnumDeviceDrivers failed!");
		return -1;
	}
	return 0;
} 
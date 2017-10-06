#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"CheckHeapMemory";
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
	HANDLE hHeap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE,30,30);
	if(hHeap == INVALID_HANDLE_VALUE)
	{
		sErrorMessage = TEXT("HeapCreate: failed!");
		return -1;
	}

	TCHAR	*pHeapBuffer	= (TCHAR*)HeapAlloc(hHeap,NULL,30),
			*sTemp			= new TCHAR[4	* sizeof(TCHAR)],	//(TCHAR*)malloc(4 * sizeof(TCHAR)),
			*sTempCat		= new TCHAR[20	* sizeof(TCHAR)];	//(TCHAR*)malloc(20 * sizeof(TCHAR));
	if(pHeapBuffer == NULL || sTemp == NULL || sTempCat == NULL)
	{
		sErrorMessage = TEXT("HeapAlloc || malloc: failed to alloc memory");
		return -1;
	}

	bool bDebugged		= false;
	int iHeapCount		= 0,
		iHeapCatCount	= 0;

	memset(sTempCat,0,20 * sizeof(TCHAR));
	while(!bDebugged && iHeapCount <= 30)
	{
		if(iHeapCatCount <= 5)
		{
			swprintf_s(sTemp,4 * sizeof(TCHAR),L"%04X",*(pHeapBuffer + iHeapCount));
			if(wcsstr(sTemp,L"ABAB") != NULL || wcsstr(sTemp,L"FEEE") != NULL || wcsstr(sTemp,L"FDFD") != NULL)
			{
				wcscat_s(sTempCat,20 * sizeof(TCHAR),sTemp);
				iHeapCatCount++;
			}
			iHeapCount++;
		}
		else
		{
			if(wcsstr(sTempCat,L"FDFDFDFDABABABABABABABAB") != NULL)
				bDebugged = true;
			else if(wcsstr(sTempCat,L"FEEEABABABABABABABABFEEE") != NULL)
				bDebugged = true;
		}
	}

	HeapFree(hHeap,NULL,pHeapBuffer);
	delete [] sTemp;
	delete [] sTempCat;
	//free(sTemp);
	//free(sTempCat);
	HeapDestroy(hHeap);

	if(bDebugged)
		return 1;
	else
		return 0;
}
#include "DDMain.h"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX icx;
	icx.dwSize = sizeof(icx);
	icx.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icx);

	DialogBox(hInstance,MAKEINTRESOURCE(IDD_MAINFRAME),hwDlgMainFrame,reinterpret_cast<DLGPROC>(MainDLGProc));

	_CrtDumpMemoryLeaks();
	return false;
}

LRESULT CALLBACK MainDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	hwDlgMainFrame = hWndDlg;
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			LVCOLUMN LvCol;
			HWND hwPluginList = GetDlgItem(hwDlgMainFrame,IDC_PLUGINS);
			SendMessage(hwPluginList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"Name";                         
			LvCol.cx = 0x100;                               
			SendMessage(hwPluginList,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Version";
			LvCol.cx = 0x50;
			SendMessage(hwPluginList,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Debugged"; 
			LvCol.cx = 0x40;
			SendMessage(hwPluginList,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
			LvCol.pszText = L"ErrorMessage"; 
			LvCol.cx = 0x99;
			SendMessage(hwPluginList,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol);

			if(!LoadPlugins())
			{
				MessageBox(hwDlgMainFrame,L"No Plugins found!",L"Debug Detector",MB_OK);
				EndDialog(hwDlgMainFrame,0);
			}
			else
			{
				ExecutePlugins();
				TCHAR* sTemp = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
				swprintf_s(sTemp,MAX_PATH,L"Debug Detector: loaded %d Plugins! - %d of %d detections - ratio: %0.3f %%",
					vPluginList.size(),
					iDetectNum,
					vPluginList.size(),
					((iDetectNum * 1.0 / vPluginList.size() *  1.0)  * 100));

				SetWindowTextW(GetDlgItem(hwDlgMainFrame,IDC_STATE),sTemp);			
				free(sTemp);
			}
			return true;
		}
	case WM_NOTIFY:
		{
			if(((LPNMHDR)lParam)->code == NM_CUSTOMDRAW)
			{
			   SetWindowLong(hwDlgMainFrame,0,(LONG)DrawDetectionColor(lParam));
			}
			return true;
		}
	case WM_CLOSE:
		{
			EndDialog(hwDlgMainFrame,0);
			return true;
		}
	}
	return false;
}

bool LoadPlugins()
{
	WIN32_FIND_DATA FindDataw32;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR* szCurDir = (TCHAR*)malloc(MAX_PATH);

	GetCurrentDirectory(MAX_PATH,szCurDir);
	wcscat_s(szCurDir,MAX_PATH / sizeof(TCHAR),L"\\*");

	hFind = FindFirstFile(szCurDir,&FindDataw32);

	if (INVALID_HANDLE_VALUE == hFind) 
		return false;

	do
	{
		if (!(FindDataw32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if(wcsstr(FindDataw32.cFileName,L".dll") != NULL)
			{
				HMODULE hPlugin = LoadLibrary(FindDataw32.cFileName);

				if(hPlugin != NULL)
				{
					srcPlugin newPlugin;
					newPlugin.dwVersion = (DWORD)GetProcAddress(hPlugin,"PluginVersion");
					newPlugin.dwName = (DWORD)GetProcAddress(hPlugin,"PluginName");
					newPlugin.dwDebugCheck = (DWORD)GetProcAddress(hPlugin,"PluginDebugCheck");
					newPlugin.dwErrorMessage = (DWORD)GetProcAddress(hPlugin,"PluginErrorMessage");
					newPlugin.hPlugin = hPlugin;

					if(newPlugin.dwDebugCheck != NULL && newPlugin.dwName != NULL && newPlugin.dwVersion != NULL && newPlugin.dwErrorMessage != NULL)
						vPluginList.push_back(newPlugin);
					else
						FreeLibrary(hPlugin);
				}
			}
		}
	}
	while (FindNextFile(hFind,&FindDataw32) != 0);

	free(szCurDir);
	if(vPluginList.size() > 0)
		return true;
	else 
		return false;
}

bool ExecutePlugins()
{
	LVITEM LvItem;
	TCHAR* sTemp = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	HWND hwPluginList = GetDlgItem(hwDlgMainFrame,IDC_PLUGINS);
	int iWinVer = GetWinVersion(),
		itemIndex = 0;

	for(size_t i = 0; i < vPluginList.size(); i++)
	{
		PluginName newPluginName = (PluginName)vPluginList[i].dwName;
		PluginVersion newPluginVersion = (PluginVersion)vPluginList[i].dwVersion;
		PluginDebugCheck newPluginDebugCheck = (PluginDebugCheck)vPluginList[i].dwDebugCheck;
		PluginErrorMessage newPluginErrorMessage = (PluginErrorMessage)vPluginList[i].dwErrorMessage;

		itemIndex = SendMessage(hwPluginList,LVM_GETITEMCOUNT,0,0);

		memset(&LvItem,0,sizeof(LvItem));
		swprintf_s(sTemp,MAX_PATH,L"%s",newPluginName());
		LvItem.mask = LVIF_TEXT;
		LvItem.cchTextMax = MAX_PATH * sizeof(TCHAR);
		LvItem.iItem = itemIndex;
		LvItem.iSubItem = 0;
		LvItem.pszText = sTemp;
		SendMessage(hwPluginList,LVM_INSERTITEM,0,(LPARAM)&LvItem);

		swprintf_s(sTemp,MAX_PATH,L"%S",newPluginVersion());
		LvItem.iSubItem = 1;
		SendMessage(hwPluginList,LVM_SETITEM,0,(LPARAM)&LvItem);

		switch(newPluginDebugCheck(iWinVer))
		{
		case 0:
			swprintf_s(sTemp,MAX_PATH,L"%s",L"FALSE");
			break;
		case 1:
			swprintf_s(sTemp,MAX_PATH,L"%s",L"TRUE");	
			iDetectNum++;
			break;
		case -1:
			swprintf_s(sTemp,MAX_PATH,L"%s",newPluginErrorMessage());
			LvItem.iSubItem = 3;
			SendMessage(hwPluginList,LVM_SETITEM,0,(LPARAM)&LvItem);
			memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
			break;
		}
				
		LvItem.iSubItem = 2;
		SendMessage(hwPluginList,LVM_SETITEM,0,(LPARAM)&LvItem);
	}
	free(sTemp);
	return true;
}

int GetWinVersion()
{
	OSVERSIONINFO osVerInfo;
	OSVERSIONINFOEX osVerEx;

	ZeroMemory(&osVerInfo,sizeof(OSVERSIONINFO));
	ZeroMemory(&osVerEx,sizeof(OSVERSIONINFOEX));
	osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	osVerEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	GetVersionEx(&osVerInfo);
	GetVersionEx((OSVERSIONINFO*)&osVerEx);

	if(osVerInfo.dwMajorVersion == 5 && osVerInfo.dwMinorVersion == 0 )
		return 0;//WIN_2000;
	if(osVerInfo.dwMajorVersion == 5 && osVerInfo.dwMinorVersion == 1 )
		return 1;//WIN_XP;
	if(osVerInfo.dwMajorVersion == 6 && osVerInfo.dwMinorVersion == 0 && osVerEx.wProductType == VER_NT_WORKSTATION )
		return 2;//WIN_VISTA;
	if(osVerInfo.dwMajorVersion == 6 && osVerInfo.dwMinorVersion == 1 && osVerEx.wProductType == VER_NT_WORKSTATION )
		return 3;//WIN_7;
	if(osVerInfo.dwMajorVersion == 6 && osVerInfo.dwMinorVersion == 2 && osVerEx.wProductType == VER_NT_WORKSTATION )
		return 4;//WIN_8

	return -1;
}

LRESULT DrawDetectionColor(LPARAM lParam)
{
    LPNMLVCUSTOMDRAW nmlvCustDraw = (LPNMLVCUSTOMDRAW)lParam;
    switch(nmlvCustDraw->nmcd.dwDrawStage) 
    {
        case CDDS_PREPAINT:
            return CDRF_NOTIFYITEMDRAW;
            
        case CDDS_ITEMPREPAINT:
			{
				TCHAR* sTemp = (TCHAR*)malloc(MAX_PATH);
				ListView_GetItemText(GetDlgItem(hwDlgMainFrame,IDC_PLUGINS),(int)nmlvCustDraw->nmcd.dwItemSpec,2,sTemp,MAX_PATH);

				if (wcsstr(sTemp,L"TRUE") != NULL)
				{
					nmlvCustDraw->clrText   = RGB(0,0,0);
					nmlvCustDraw->clrTextBk = RGB(255,0,0);
					free(sTemp);
					return CDRF_NEWFONT;
				}
				if (wcsstr(sTemp,L"FALSE") != NULL)
				{
					nmlvCustDraw->clrText   = RGB(0,0,0);
					nmlvCustDraw->clrTextBk = RGB(0,255,0);
					free(sTemp);
					return CDRF_NEWFONT;
				}
				else{
					nmlvCustDraw->clrText   = RGB(0,0,0);
					nmlvCustDraw->clrTextBk = RGB(0,0,255);
            		free(sTemp);
					return CDRF_NEWFONT;
				}
				free(sTemp);
				break;
			}
    }
    return CDRF_DODEFAULT;
}
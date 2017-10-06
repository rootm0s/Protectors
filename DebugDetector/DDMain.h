#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <vector>
#include "resource.h"

// Leak detection
//#include <stdlib.h>
//#include <crtdbg.h>
//
//#define _CRTDBG_MAP_ALLOC
// Leak detection

using namespace std;

struct srcPlugin 
{
	DWORD dwName;
	DWORD dwVersion;
	DWORD dwDebugCheck;
	DWORD dwErrorMessage;
	HMODULE hPlugin;
};

LRESULT CALLBACK MainDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT DrawDetectionColor(LPARAM lParam);

bool LoadPlugins();
bool ExecutePlugins();

int GetWinVersion();

typedef char*  (*PluginVersion)(void);
typedef TCHAR* (*PluginName)(void);
typedef TCHAR* (*PluginErrorMessage)(void);
typedef DWORD  (*PluginDebugCheck)(int iWinVers);

HWND hwDlgMainFrame = NULL;

int iDetectNum = 0;

vector<srcPlugin> vPluginList;
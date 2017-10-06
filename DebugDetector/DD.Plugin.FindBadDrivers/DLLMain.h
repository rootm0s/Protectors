#include <Windows.h>
#include <vector>
#include <Psapi.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

	__declspec(dllexport) TCHAR* __cdecl PluginName(void);
	__declspec(dllexport) char* __cdecl PluginVersion(void);
	__declspec(dllexport) TCHAR* __cdecl PluginErrorMessage(void);
	__declspec(dllexport) DWORD __cdecl PluginDebugCheck(int iWinVer);

#ifdef __cplusplus
}
#endif

BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);

TCHAR* sErrorMessage;
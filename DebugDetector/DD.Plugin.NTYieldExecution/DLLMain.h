#include <Windows.h>

#define _STATUS_NO_YIELD_PERFORMED                      0x40000024

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

TCHAR* sErrorMessage;
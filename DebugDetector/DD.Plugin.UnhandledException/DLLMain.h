#include <Windows.h>

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

LONG WINAPI UnhandledExcepFilter(PEXCEPTION_POINTERS pExcepPointers);

TCHAR* sErrorMessage;
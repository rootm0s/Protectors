#include "DLLMain.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return true;
}

__declspec(dllexport) TCHAR* __cdecl PluginName(void)
{
	return L"Unhandled Exception";
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
	SetUnhandledExceptionFilter(UnhandledExcepFilter);
	__asm
	{
		xor eax, eax
		div eax
	}

	return 0;
}

LONG WINAPI UnhandledExcepFilter(PEXCEPTION_POINTERS pExcepPointers)
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)pExcepPointers->ContextRecord->Eax);

	pExcepPointers->ContextRecord->Eip += 2;

	return EXCEPTION_CONTINUE_EXECUTION;
}

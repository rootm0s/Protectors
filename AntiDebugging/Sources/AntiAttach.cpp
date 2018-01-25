#include <stdio.h>
#include <Windows.h>

__declspec(naked) void AntiAttach() {
	__asm {
		jmp ExitProcess
	}
}

int main() {
	HANDLE hProcess = GetCurrentProcess();

	HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
	FARPROC func_DbgUiRemoteBreakin = GetProcAddress(hMod, "DbgUiRemoteBreakin");

	WriteProcessMemory(hProcess, func_DbgUiRemoteBreakin, AntiAttach, 6, NULL);

	int a, b;
	scanf("%d %d", &a, &b);

	printf("result : %d\n", a + b);

	system("pause");
	return 0;
}

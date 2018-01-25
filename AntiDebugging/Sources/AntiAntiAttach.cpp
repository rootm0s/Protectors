#include <stdio.h>
#include <Windows.h>
#include <TlHelp32.h>

#define DbgBreakPoint_FUNC_SIZE 0x2
#define DbgUiRemoteBreakin_FUNC_SIZE 0x54
#define NtContinue_FUNC_SIZE 0x18

struct FUNC {
	char *name;
	FARPROC addr;
	SIZE_T size;
};

FUNC funcList[] = {
	{ "DbgBreakPoint", 0, DbgBreakPoint_FUNC_SIZE },
	{ "DbgUiRemoteBreakin", 0, DbgUiRemoteBreakin_FUNC_SIZE },
	{ "NtContinue", 0, NtContinue_FUNC_SIZE }
};

DWORD GetPidByProcessName(WCHAR *name) {
	PROCESSENTRY32W entry;
	memset(&entry, 0, sizeof(PROCESSENTRY32W));
	entry.dwSize = sizeof(PROCESSENTRY32W);

	DWORD pid = -1;
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32FirstW(hSnapShot, &entry)) {
		do {
			if (!wcscmp(name, entry.szExeFile)) {
				pid = entry.th32ProcessID;
				break;
			}
		} while (Process32Next(hSnapShot, &entry));
	}

	CloseHandle(hSnapShot);

	return pid;
}

bool GetModuleNameByPid(DWORD pid, WCHAR *names) {
	MODULEENTRY32W module;
	memset(&module, 0, sizeof(MODULEENTRY32W));
	module.dwSize = sizeof(MODULEENTRY32W);

	bool result = false;

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	if (Module32FirstW(hSnapShot, &module)) {
		do {
			names += wsprintf(names, module.szModule);
		} while (Module32NextW(hSnapShot, &module));

		result = true;
	}

	CloseHandle(hSnapShot);
	return result;
}

WCHAR* charToWChar(const char* text) {
	size_t size = strlen(text) + 1;
	WCHAR* wa = new WCHAR[size];
	mbstowcs(wa, text, size);
	return wa;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("USAGE : AntiAntiAttach.exe PROCESS_NAME\n");
		return 1;
	}

	WCHAR *pname = charToWChar(argv[1]);
	DWORD pid = GetPidByProcessName(pname);

	if (pid == -1) {
		printf("[*] invalid process name\n");
		return 1;
	}

	WCHAR modName[MAX_PATH] = { 0 };
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

	HMODULE hMod = LoadLibraryW(L"ntdll.dll");
	for (int i = 0; i < _countof(funcList); ++i) {
		funcList[i].addr = GetProcAddress(hMod, funcList[i].name);
	}

	bool result = false;
	WCHAR names[2048];

	if (GetModuleNameByPid(pid, names)) {
		if (wcsstr(names, L"ntdll") || wcsstr(names, L"NTDLL")) {
			for (int i = 0; i < _countof(funcList); ++i) {
				DWORD dwOldProtect;

				VirtualProtectEx(hProcess, funcList[i].addr, funcList[i].size, PAGE_EXECUTE_READWRITE, &dwOldProtect);
				result = WriteProcessMemory(hProcess, funcList[i].addr, funcList[i].addr, funcList[i].size, NULL);
				VirtualProtectEx(hProcess, funcList[i].addr, funcList[i].size, dwOldProtect, NULL);

				if (!result) break;
			}
		}
	}

	if (result) {
		printf("[*] patch success\n");
	}
	else {
		printf("[*] fail\n");
	}
	CloseHandle(hProcess);

	return 0;
}

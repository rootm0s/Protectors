#include <stdio.h>
#include <Windows.h>
#include <winternl.h>
#include <TlHelp32.h>

typedef NTSTATUS(WINAPI * fnNtQueryInformationProcess) (
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationCLass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
	PULONG ReturnLength
);

fnNtQueryInformationProcess GetNtQueryInformationProcess() {
	HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
	if (hNtdll == NULL) {
		return NULL;
	}

	FARPROC func = GetProcAddress(hNtdll, "NtQueryInformationProcess");
	fnNtQueryInformationProcess query_func = (fnNtQueryInformationProcess)func;

	return query_func;
}

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

int wmain(int argc, WCHAR *argv[]) {
	if (argc < 2) {
		printf("[*] USAGE : exec PROCESSNAME");
		return 1;
	}

	DWORD pid = GetPidByProcessName(argv[1]);
	fnNtQueryInformationProcess NtQueryInformationProcess = GetNtQueryInformationProcess();
	
	ULONG ReturnLength;
	PROCESS_BASIC_INFORMATION pbi;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &ReturnLength);
	PPEB pPEB = (PPEB)pbi.PebBaseAddress;

	SIZE_T Written;
	DWORD64 CrossProcessFlags = -1;
	ReadProcessMemory(hProcess, (PBYTE)pPEB + 0x50, (LPVOID)&CrossProcessFlags, sizeof(DWORD64), &Written);

	printf("[*] CrossProcessFlags : %p\n", CrossProcessFlags);
	if (CrossProcessFlags & 0x4) {
		printf("[*] veh set\n");
	}
	else {
		printf("[*] veh unset\n");
	}

	return 0;
}

#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>
#include <TlHelp32.h>

#define MAKEULONGLONG(ldw, hdw) ((ULONGLONG(hdw) << 32) | ((ldw) & 0xFFFFFFFF))
DWORD GetMainThreadId(DWORD pid) {
	THREADENTRY32 th32;
	memset(&th32, 0, sizeof(THREADENTRY32));
	th32.dwSize = sizeof(THREADENTRY32);

	DWORD dwMainThreadID = -1;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	if (Thread32First(hSnapshot, &th32)) {
		DWORD64 ullMinCreateTime = 0xFFFFFFFFFFFFFFFF;

		do {
			if (th32.th32OwnerProcessID == pid) {
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, th32.th32ThreadID);

				if (hThread) {
					FILETIME afTimes[4] = { 0 };
					if (GetThreadTimes(hThread, &afTimes[0], &afTimes[1], &afTimes[2], &afTimes[3])) {
						ULONGLONG ullTest = MAKEULONGLONG(afTimes[0].dwLowDateTime, afTimes[0].dwHighDateTime);
						if (ullTest && ullTest < ullMinCreateTime) {
							ullMinCreateTime = ullTest;
							dwMainThreadID = th32.th32ThreadID;
						}
					}
					CloseHandle(hThread);
				}
			}
		} while (Thread32Next(hSnapshot, &th32));
	}

	CloseHandle(hSnapshot);
	return dwMainThreadID;
}


DWORD GetPidByProcessName(WCHAR* name) {
	PROCESSENTRY32W entry;
	memset(&entry, 0, sizeof(entry));
	entry.dwSize = sizeof(PROCESSENTRY32W);

	DWORD pid = -1;
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32FirstW(hSnapShot, &entry)) {
		do {
			if (!wcscmp(name, entry.szExeFile)) {
				pid = entry.th32ProcessID;
				break;
			}
		} while (Process32NextW(hSnapShot, &entry));
	}

	CloseHandle(hSnapShot);
	return pid;
}

int wmain(int argc, WCHAR *argv[]) {
	if (argc < 2) {
		printf("USAGE : exec PROCESSNAME\n");
		return 1;
	}

	DWORD pid = GetPidByProcessName(argv[1]);
	DWORD tid = GetMainThreadId(pid);

	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);

	CONTEXT ctx;
	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	ctx.Dr0 = 0;
	ctx.Dr1 = 0;
	ctx.Dr2 = 0;
	ctx.Dr3 = 0;
	ctx.Dr7 &= (0xffffffffffffffff ^ (0x1 | 0x4 | 0x10 | 0x40));

	SetThreadContext(hThread, &ctx);
	CloseHandle(hThread);
}

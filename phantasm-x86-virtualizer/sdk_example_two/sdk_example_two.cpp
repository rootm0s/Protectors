// sdk_example_two.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "phant.h"

DWORD hashFile(const char *filename) {
	BeginProtect;

	HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	DWORD hash = 0;
	if (file == INVALID_HANDLE_VALUE) {
		printf("could not open file for reading (error: %d)\n", GetLastError());
		return 0;
	}
	else {

		DWORD fileSize = GetFileSize(file, NULL);

		DWORD buf = 0;
		DWORD numRead;

		hash = 0x811c9dc5;
		while (ReadFile(file, &buf, sizeof(buf), &numRead, NULL)) {
			if (!numRead) {
				break;
			}
			hash = (hash ^ buf) * 0x1000193;
			buf = 0;
		}
		CloseHandle(file);

	}

	EndProtect;

	return hash;
}


int main(int argc, const char *argv[])
{
	BeginProtect;

	if (argc != 2) {
		printf("no filename specified\n");
		return 1;
	}

	DWORD hash = hashFile(argv[1]);
	printf("result: 0x%08x\n", hash);

	EndProtect;

    return 0;
}


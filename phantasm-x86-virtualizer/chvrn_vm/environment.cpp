#include "stdafx.h"

static char g_rootDirectory[MAX_PATH];
static char g_logDirectory[MAX_PATH];
static char g_tempDirectory[MAX_PATH];

const char *GetRootDirectory() {
	return g_rootDirectory;
}

const char *GetLogDirectory() {
	return g_logDirectory;
}

const char *GetTempDirectory() {
	return g_tempDirectory;
}

bool CreateDirectoryIfNotExists(const char *path) {
	if (!CreateDirectory(path, NULL)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			return false;
		}
	}
	return true;
}

bool SetupDirectories() {
	DWORD length = GetCurrentDirectory(MAX_PATH, g_rootDirectory);
	if (length == 0) {
		return false;
	}

	sprintf(g_logDirectory, "%s/%s", g_rootDirectory, "logs");
	sprintf(g_tempDirectory, "%s/%s", g_rootDirectory, "temp");

	if (!CreateDirectoryIfNotExists(g_logDirectory)) {
		return false;
	}
	if (!CreateDirectoryIfNotExists(g_tempDirectory)) {
		return false;
	}

	return true;
}
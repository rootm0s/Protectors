#include "stdafx.h"
#include "cli.h"
#include "environment.h"

int _tmain(int argc, _TCHAR* argv[])
{
	if (!SetupDirectories()) {
		logger.write(LOG_ERROR, "Could not create directories");
		return 1;
	}

	SYSTEMTIME lt;
	GetLocalTime(&lt);

	char filename[MAX_PATH];
	sprintf(filename, "%s/%04d-%02d-%02d_%02d%02d.txt", 
		GetLogDirectory(), lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute);
	
	if (!logger.setLogFile(filename)) {
		logger.write(LOG_WARN, "Could not create log file: %s\n", filename);
	}

	logger.setVerbosity(3);
	
	if (!RunCli(argc, argv)) {
		logger.write(LOG_WARN, "There were errors - see above\n");
		return 1;
	}
	
	logger.write(LOG_WARN, "All operations successful\n");
	return 0;
}


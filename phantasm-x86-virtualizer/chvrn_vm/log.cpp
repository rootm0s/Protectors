#include "stdafx.h"

// Global instance
Logger logger;

Logger::Logger()
	: m_verbosityLevel(3), m_outputFile(INVALID_HANDLE_VALUE) {

}

Logger::~Logger() {
	CloseHandle(m_outputFile);
}

bool Logger::setLogFile(const char *filename) {

	if (m_outputFile != INVALID_HANDLE_VALUE) {
		CloseHandle(m_outputFile);
	}

	m_outputFile = INVALID_HANDLE_VALUE;
	if ((m_outputFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr)) == INVALID_HANDLE_VALUE) {
		return false;
	}

	return true;
}

void Logger::setVerbosity(int level) {
	m_verbosityLevel = level;
}

void Logger::write(int level, const char *fmt, ...) {
	char buffer[256];
	va_list args;
	va_start(args, fmt);
	int length = vsprintf(buffer, fmt, args); // TODO: check length
	va_end(args);

	const char *fmts[] = { "Error: %s", "Warning: %s", "%s", "%s" };

	if (level < m_verbosityLevel) {
		if (level < 4) {
			printf(fmts[level], buffer);
		}
		else {
			printf("%s", buffer);
		}
	}

	DWORD written;
	if (m_outputFile != INVALID_HANDLE_VALUE) {
		if (!WriteFile(m_outputFile, buffer, length, &written, NULL)) {
			printf("ERROR: could not write to log file\n");
		}
	}
}
#include "stdafx.h"

class Logger {
public:
	Logger();
	~Logger();
	bool setLogFile(const char *filename);
	void setVerbosity(int level);
	void write(int verbosity, const char *fmt, ...);
private:
	int m_verbosityLevel;
	HANDLE m_outputFile;
};

#define LOG_ERROR		0
#define LOG_WARN		1
#define LOG_MSG			2
#define LOG_VERBOSE		3

extern Logger logger;
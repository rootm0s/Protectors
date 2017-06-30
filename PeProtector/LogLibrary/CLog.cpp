#include <stdarg.h>
#include <windows.h>
#include <time.h>
#include "CLog.h"
#include <string>

using std::string;

namespace
{
   string timeToString(const time_t & time)
   {
      char buffer[80];

      tm now;

      localtime_s(&now, &time);

      strftime(buffer, 80, "%c", &now);

      return string(buffer);
   }
}

CLog & CLog::getInstance()
{
   static CLog log;
   return log;
}

bool CLog::initialize(const string & fileName)
{
   mFileHandle = fopen(fileName.c_str(), "w");
   return true;
}

void CLog::log(const string & type, const char * const format, ...)
{
   va_list args;
   va_start(args, format);
   
   if (mFileHandle != 0)
   {
      fprintf(mFileHandle, "[%s] [%d] [%s] : ", timeToString(time(0)).c_str(), GetCurrentThreadId(), type.c_str());

      vfprintf(mFileHandle, format, args);

      fprintf(mFileHandle, "\n");

      fflush(mFileHandle);
   }

   va_end(args);
}

CLog::CLog()
: mFileHandle()
{
}

CLog::~CLog()
{
   if (mFileHandle != 0)
   {
      fclose(mFileHandle);
   }
}

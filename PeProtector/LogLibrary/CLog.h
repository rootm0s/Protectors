#ifndef CLOG_H
#define CLOG_H

#include <stdio.h>
#include <string>

/**
 * @brief Logger
 */
class CLog
{
public:
   /**
    * @brief Get single instance of logger
    */
   static CLog & getInstance();
   /**
    * @brief Initialize logger
    * @param[in] fileName file where traces will be written
    */
   bool initialize(const std::string & fileName);
   /**
    * @brief Send to log file message
    * @param[in] type type of message. It can be DEBUG, ERROR, WARNING or whatever
    * @param[in] format format string in printf style
    */
   void log(const std::string & type, const char * format, ...);

private:
   /**
    * @brief Constructor
    */
   CLog();
   /**
    * @brief Destructor
    */
   ~CLog();
   /**
    * @brief Copy constructor
    */
   CLog(const CLog &);
   /**
    * @brief Assign operator
    */
   CLog & operator=(const CLog &);
   /**
    * @brief File handle
    */
   FILE * mFileHandle;
};

/**
 * @brief Helper macros
 */
#define LOG_INITIALIZE(fileName) CLog::getInstance().initialize(fileName)
#define LOG_ERROR(format, ...) CLog::getInstance().log("error", format, ##__VA_ARGS__)
#ifdef _DEBUG
#define LOG_DEBUG(format, ...) CLog::getInstance().log("debug", format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif

#endif

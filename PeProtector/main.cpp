#include "../Library/SCommand.h"
#include "ostream"

#include <fstream>
#include "xiosbase"
#include "ProtectPe.h"
#include "../LogLibrary/CLog.h"
#include "ClientFile.h"
#include "PeHeader.h"

using std::ofstream;
using std::ios_base;
using std::exception;
using std::string;

/**
 * @brief PeProtector.exe fileName
 */
int main(int argc, char * argv[], char * env[])
{
   int exitCode = 0;
   if (argc == 2)
   {
      if (CopyFileA(argv[1], (argv[1] + string(".bak")).c_str(), FALSE))
      {
         try
         {
            const NPeProtector::SClientFile clientFile = NPeProtector::getPeFileInfo(argv[1]);
            //test
            NPeProtector::gImageBase = clientFile.mImageBase;

            ofstream fileStream(argv[1], ios_base::binary | ios_base::trunc);

            if (fileStream.is_open())
            {
               LOG_INITIALIZE(string(argv[0]) + ".log");

               NPeProtector::protectPe(fileStream, clientFile);
            }
            else
            {
               printf("Failed to open file %s", argv[1]);
               exitCode = 1;
            }
         }
         catch (const exception & e)
         {
            printf("Failed to protect file, %s", e.what());
            exitCode = 1;
         }
      }
      else
      {
         printf("Failed to copy file, %s", argv[1]);
         exitCode = 1;
      }
   }
   else
   {
      printf("PeProtector.exe fileName");
   }
   return exitCode;
}

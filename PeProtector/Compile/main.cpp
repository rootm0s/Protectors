#include "CLexicalAnalizer.h"
#include "CCompile.h"
#include <string>
#include <fstream>
#include <streambuf>
#include <iosfwd>
#include "..\library\SCommand.h"
#include <iostream>
#include <ostream>
#include <algorithm>
#include <xfunctional>
#include "../LogLibrary/CLog.h"
#include "../Library/Types.h"
#include <sstream> 

using std::exception;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::basic_ostream;
using std::basic_istream;
using std::char_traits;
using std::string;
using std::ios_base;
using std::ostringstream;

/**
 * @brief Compile.exe file.asm file.bin
 */
int main(int argc, char * argv[], char * env[])
{
   int exitCode = 0;
   if (argc == 3)
   {
      LOG_INITIALIZE(string(argv[0]) + ".log");
      try
      {
         ifstream file(argv[1]);
         if (file.is_open())
         {
            // todo подумать об исключение! об ошибке!
            const vector<NPeProtector::SCommand> & commands = NPeProtector::compile(file);

            ofstream binFile(argv[2], ios_base::binary);

            NPeProtector::serialize(binFile, commands);
         }
         else
         {
            exitCode = 1;
            printf("error: can't open file %s", argv[1]);
         }
      }
      catch (const exception & e)
      {
         cout << e.what();
      }
   }
   else
   {
      printf("Compile.exe file.asm file.bin");
   }
   
   return exitCode;
}

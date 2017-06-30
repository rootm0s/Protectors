#ifndef CCOMPILE_H
#define CCOMPILE_H

#include "..\Library\SCommand.h"
#include <istream>
#include <iosfwd>

namespace NPeProtector
{
   /**
    * @brief Compile source code into SCommand array for next processing in Protector
    * @param[in] input character stream of source code
    * @return Array of SCommand
    */
   std::vector<SCommand> compile(std::basic_istream<char, std::char_traits<char> > & input);
}

#endif
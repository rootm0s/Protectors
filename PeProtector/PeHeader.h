#ifndef PE_HEADER_H
#define PE_HEADER_H

#include "iosfwd"
#include "../Library/SCommand.h"
#include "ClientFile.h"

namespace NPeProtector
{
   const int gSectionAlignment = 0x1000;
   const int gFileAlignment = 0x200;
   extern int gImageBase;

   /**
    * @brief Get size of pe header
    */
   int getHeaderSize();

   /**
    * @brief Put header in stream
    * @param[in] output output stream
    * @param[in] commands commands
    * @param[in] clientFile it's used for size of image and image base.
    */
   void putHeader(std::ostream & output, const std::vector<SCommand> & commands, const SClientFile & clientFile);
}

#endif
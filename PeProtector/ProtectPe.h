#ifndef PROTECT_PE_H
#define PROTECT_PE_H

#include "iosfwd"
#include "ClientFile.h"

namespace NPeProtector
{
   /**
    * @brief Put the whole protected file in stream
    * @param[in] output output stream
    * @param[in] clientFile data of source file
    */
   void protectPe(std::ostream & output, const SClientFile & clientFile);
}

#endif
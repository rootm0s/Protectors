#ifndef MUTATION_H
#define MUTATION_H

#include "../Library/SCommand.h"
#include "vector"

namespace NPeProtector
{
   /**
    * @brief Mutate all assembler commands
    */
   void mutateCommands(std::vector<SCommand> & commands);
}

#endif
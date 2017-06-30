#include "../Library/SCommand.h"
#include "iosfwd"

namespace NPeProtector
{
   /**
   * @brief Get size of instruction
   * @param[in] instruction instruction
   * @return Size
   */
   int getInstructionSize(const SInstruction & instruction);
   /**
    * @brief Put instruction in stream
    * @param[in] output output stream
    * @param[in] instruction instruction
    * @param[in] commands commands
    * @param[in] rva offset where instruction wiil be
    */
   void putInstruction(std::ostream & output, const SInstruction & instruction, const std::vector<SCommand> & commands, DWORD rva);
}
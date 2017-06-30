#include "../Library/SCommand.h"
#include "iosfwd"

namespace NPeProtector
{
   /**
    * @brief Setup all import reference in commands
    * @param[in] commands commands
    * @param[in] importRVA offset where import will be
    */
   void resolveImport(std::vector<SCommand> & commands, const DWORD importRVA);
   /**
    * @brief Get size of import directory
    * @param[in] commands commands but processed only import command
    * @return Size
    */
   int getImportSize(const std::vector<SCommand> & commands);
   /**
    * @brief Put import directory in stream
    * @param[in] output output stream
    * @param[in] commands commands
    * @param[in] importRVA offset where import will be
    */
   void putImport(std::ostream & output, const std::vector<SCommand> & commands, const DWORD importRVA);
}

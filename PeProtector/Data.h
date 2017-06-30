#include "../Library/SCommand.h"
#include "iosfwd"

namespace NPeProtector
{
   /**
    * @brief Get size of data
    * @param[in] data data
    * @return Size
    */
   int getDataSize(const SData & data);
   /**
    * @brief Put data in stream
    * @param[in] output output stream
    * @param[in] data data
    * @param[in] commands commands
    */
   void putData(std::ostream & output, const SData & data, const std::vector<SCommand> & commands);
}

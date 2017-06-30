#include "Data.h"
#include <ostream>
#include "assert.h"
#include "PeHeader.h"

using std::ostream;
using std::vector;
using std::exception;

namespace NPeProtector
{
   DWORD getConstantValue(const SConstant & constant, const vector<SCommand> & commands)
   {
      DWORD result = constant.mValue;

      for (unsigned int i = 0; i < constant.mLabels.size(); ++i)
      {
         if (constant.mLabels[i].mSign == NSign::PLUS)
         {
            result += commands[constant.mLabels[i].mIndex].mRVA + gImageBase;
         }
         else
         {
            result -= commands[constant.mLabels[i].mIndex].mRVA + gImageBase;
         }
      }
      return result;
   }

   int getDataSize(const SData & data)
   {
      assert((data.mCount > 1 && data.mConstants.size() == 1) || (data.mCount == 1 && data.mConstants.size() >= 1));

      return data.mCount * data.mSizeData * data.mConstants.size();
   }

   void putData(ostream & output, const SData & data, const vector<SCommand> & commands)
   {
      assert((data.mCount > 1 && data.mConstants.size() == 1) || (data.mCount == 1 && data.mConstants.size() >= 1));

      for (int i = 0; i < data.mCount; ++i)
      {
         for (unsigned int j = 0; j < data.mConstants.size(); ++j)
         {
            const DWORD value = getConstantValue(data.mConstants[j], commands);
            switch (data.mSizeData)
            {
            case 1:
            {
               const char db = static_cast<char>(value);
               output.write((char*)&db, 1);
               break;
            }
            case 2:
            {
               const short dw = static_cast<short>(value);
               output.write((char*)&dw, 2);
               break;
            }
            case 4:
            {
               output.write((char*)&value, 4);
               break;
            }
            default:
               throw exception("Wrong data size");
            }
         }
      }
   }
}

#include "Import.h"
#include <ostream>
#include <map>
#include "assert.h"

using std::ostream;
using std::vector;
using std::string;
using std::pair;
using std::map;

namespace NPeProtector
{
namespace
{
   typedef pair<int/*index in commands*/, string/*function name*/> tIndexToFunction;
   typedef vector<tIndexToFunction> tFunctions;
   typedef map<string, tFunctions> tImport;

   typedef tFunctions::const_iterator tFunctionIterator;
   typedef tImport::const_iterator tImportIterator;

   tImport getImport(const vector<SCommand> & commands)
   {
      tImport result;
      for (unsigned int i = 0; i < commands.size(); ++i)
      {
         if (commands[i].mType == NCommand::IMPORT)
         {
            result[commands[i].mImport.mDllName].push_back(tIndexToFunction(i, commands[i].mImport.mFunctionName));
         }
      }
      return result;
   }

   // total amount of functions!
   int getNumberFunctions(const tImport & import)
   {
      int result = 0;
      for (tImportIterator i = import.begin(); i != import.end(); ++i)
      {
         for (tFunctionIterator j = i->second.begin(); j != i->second.end(); ++j)
         {
            result += 1;
         }
      }
      return result;
   }

   DWORD toRVA(void * pointer, void * base, DWORD importRVA)
   {
      return ((DWORD)pointer - (DWORD)base) + importRVA;
   }
}

   int getImportSize(const vector<SCommand> & commands) 
   {
      const tImport & import = getImport(commands);

      int size = 0;
      for (tImportIterator i = import.begin(); i != import.end(); ++i)
      {
         size += sizeof(IMAGE_IMPORT_DESCRIPTOR);
         size += i->first.length() + 1;
         size += ((i->second.size() + 1/*end*/) * sizeof(DWORD)) * 2 /*OriginalFirstThunk + FirstThunk*/;
         for (tFunctionIterator j = i->second.begin(); j != i->second.end(); ++j)
         {
            size += j->second.length() + 1/*zero byte*/ + sizeof(WORD) /*IMAGE_IMPORT_BY_NAME::Hint*/;
         }
      }
      size += sizeof(IMAGE_IMPORT_DESCRIPTOR) /*end*/;

      return size;
   }

   void resolveImport(vector<SCommand> & commands, const DWORD importRVA)
   {
      const tImport & import = getImport(commands);

      const int dllNumber = import.size();
      const int descriptionSize = (dllNumber + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR);
      const int originalFirstChunkSize = (getNumberFunctions(import) + dllNumber) * sizeof(DWORD);

      DWORD firstChunkRVA = importRVA + descriptionSize + originalFirstChunkSize;

      for (tImportIterator i = import.begin(); i != import.end(); ++i)
      {
         for (tFunctionIterator j = i->second.begin(); j != i->second.end(); ++j)
         {
            assert(commands[j->first].mType == NCommand::IMPORT);
            commands[j->first].mRVA = firstChunkRVA;
            firstChunkRVA += sizeof(DWORD);
         }
         firstChunkRVA += sizeof(DWORD);
      }
   }
   
   void putImport(ostream & output, const vector<SCommand> & commands, const DWORD importRVA)
   {
      const tImport & import = getImport(commands);
      /*
       IMAGE_IMPORT_DESCRIPTOR(1)
       IMAGE_IMPORT_DESCRIPTOR(2)
       IMAGE_IMPORT_DESCRIPTOR(end)
       originalFirstChunk(1)(1)
       originalFirstChunk(1)(2)
       originalFirstChunk(1)(3)
       originalFirstChunk(1)(end)
       originalFirstChunk(2)(1)
       originalFirstChunk(2)(2)
       originalFirstChunk(2)(end)
       firstChunk(1)(1)
       firstChunk(1)(2)
       firstChunk(1)(3)
       firstChunk(1)(end)
       firstChunk(2)(1)
       firstChunk(2)(2)
       firstChunk(2)(end)
       dllName(1)
       functionName(1)(1)
       functionName(1)(2)
       functionName(1)(3)
       dllName(2)
       functionName(2)(1)
       functionName(2)(2)
       **/
      
      const int dllNumber = import.size();
      const int descriptionSize = (dllNumber + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR);
      const int originalFirstChunkSize = (getNumberFunctions(import) + dllNumber) * sizeof(DWORD);
      const int firstChunkSize = originalFirstChunkSize;

      const int importSize = getImportSize(commands);

      char * const buffer = new char[importSize];
      memset(buffer, 0, importSize);

      IMAGE_IMPORT_DESCRIPTOR * descriptionPtr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(buffer);
      DWORD * originalFirstChunkPtr = reinterpret_cast<DWORD *>(buffer + descriptionSize);
      DWORD * firstChunkPtr = reinterpret_cast<DWORD *>(buffer + descriptionSize + originalFirstChunkSize);
      char * namesPtr = reinterpret_cast<char *>(buffer + descriptionSize + originalFirstChunkSize + firstChunkSize);

      for (tImportIterator i = import.begin(); i != import.end(); ++i)
      {
         descriptionPtr->OriginalFirstThunk = toRVA(originalFirstChunkPtr, buffer, importRVA);
         descriptionPtr->FirstThunk = toRVA(firstChunkPtr, buffer, importRVA);
         descriptionPtr->Name = toRVA(namesPtr, buffer, importRVA);

         // copy dll name
#pragma warning(push)
#pragma warning(disable: 4996)
         strcpy(namesPtr, i->first.c_str());
#pragma warning(pop)

         namesPtr += strlen(i->first.c_str()) + 1;

         for (tFunctionIterator j = i->second.begin(); j != i->second.end(); ++j)
         {
            *originalFirstChunkPtr = toRVA(namesPtr, buffer, importRVA);
            *firstChunkPtr = toRVA(namesPtr, buffer, importRVA);
            // copy hint 
            *(namesPtr + 0) = 0;
            *(namesPtr + 1) = 0;

#pragma warning(push)
#pragma warning(disable: 4996)
            strcpy(namesPtr + 2, /*importSize - ((namesPtr + 2)- buffer),*/ j->second.c_str());
#pragma warning(pop)
        
            namesPtr += strlen(j->second.c_str()) + 1 + 2;

            originalFirstChunkPtr++;
            firstChunkPtr++;
         }
         descriptionPtr++;

         // terminated zeros;
         originalFirstChunkPtr++;
         firstChunkPtr++;
      }

      output.write(buffer, importSize);

      delete[] buffer;
   }
}

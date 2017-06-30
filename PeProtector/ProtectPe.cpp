#include "ProtectPe.h"
#include <ostream>
#include <vector>
#include "../Library/SCommand.h"
#include "Instruction.h"
#include "Data.h"
#include "Resources.h"
#include "Import.h"
#include "wtypes.h"
#include "winuser.h"
#include <windows.h>
#include <strsafe.h>
#include "resource.h"
#include <sstream>      // std::stringbuf
#include <string>       // std::string
#include "PeHeader.h"
#include "../LogLibrary/CLog.h"
#include "assert.h"
#include "ClientFile.h"
#include "Mutation.h"

using std::ostream;
using std::vector;
using std::string;
using std::istringstream;
using std::exception;

namespace NPeProtector
{
   //TODO move this
   inline int align(int value, int al)
   {
      if (value % al)
      {
         value += al - (value % al);
      }
      return value;
   }
namespace
{
   int getDirectiveSize(SDirective & directive, const vector<SCommand> & commands, const SClientFile & clientFile)
   {
      if (directive.mName == "IMPORT_DIRECTORY")
      {
         directive.mDirectorySize = getImportSize(commands);
         return directive.mDirectorySize;
      }
      else if (directive.mName == "RECOURCE_DIRECTORY")
      {
         directive.mDirectorySize = getResourcesSize(clientFile);
         return directive.mDirectorySize;
      }
      else if (directive.mName == "COMPRESSED_FILE")
      {
         return clientFile.mCompressed.size();
      }
      else
      {
         throw exception(("Failed to get directive " + directive.mName).c_str());
      }
      return 0;
   }

   void putDirective(ostream & output, const vector<SCommand> & commands, const SClientFile & clientFile, const SDirective & directive, const DWORD baseRVA)
   {
      if (directive.mName == "IMPORT_DIRECTORY")
      {
         putImport(output, commands, baseRVA);
      }
      else if (directive.mName == "RECOURCE_DIRECTORY")
      {
         putResources(output, commands, clientFile, baseRVA);
      }
      else if (directive.mName == "COMPRESSED_FILE")
      {
         if (!clientFile.mCompressed.empty())
         {
            output.write(&clientFile.mCompressed.front(), clientFile.mCompressed.size());
         }
      }
      else
      {
         throw exception(("Failed to get directive " + directive.mName).c_str());
      }
   }

   void resolveLabels(vector<SCommand> & commands, const SClientFile & clientFile)
   {
      int nextRVA = align(getHeaderSize(), gSectionAlignment);
      int nextRAW = align(getHeaderSize(), gFileAlignment);
      int size = 0;
      for (unsigned int i = 0; i < commands.size(); ++i)
      {
         switch (commands[i].mType)
         {
         case NCommand::DIRECTIVE:
         {
            commands[i].mRVA = nextRVA;
            commands[i].mRAW = nextRAW;

            if (!_strcmpi(commands[i].mDirective.mName.c_str(), "IMPORT_DIRECTORY"))
            {
               resolveImport(commands, commands[i].mRVA);
            }

            size = getDirectiveSize(commands[i].mDirective, commands, clientFile);
            nextRVA += size;
            nextRAW += size;
            break;
         }
         case NCommand::INSTRUCTION:
            commands[i].mRVA = nextRVA;
            commands[i].mRAW = nextRAW;
            size = getInstructionSize(commands[i].mInstruction);
            nextRVA += size;
            nextRAW += size;
            break;
         case NCommand::DATA:
            commands[i].mRVA = nextRVA;
            commands[i].mRAW = nextRAW;
            size = getDataSize(commands[i].mData);
            nextRVA += size;
            nextRAW += size;
            break;
         case NCommand::EXTERN:
         case NCommand::IMPORT:
            // skip
            break;
         case NCommand::SECTION:
         case NCommand::END:
            nextRVA = align(nextRVA, gSectionAlignment);
            nextRAW = align(nextRAW, gFileAlignment);
            commands[i].mRVA = nextRVA;
            commands[i].mRAW = nextRAW;
            break;
         }
      }
   }
   void putZeroBytes(ostream & output, const SCommand & command)
   {
      const int currentPosition = static_cast<int>(output.tellp());
      const int size = command.mRAW - currentPosition;
      
      assert(size >= 0);

      for (int i = 0; i < size; ++i)
      {
         char c = 0;
         output.write(&c, 1);
      }
   }

   void putBody(ostream & output, const vector<SCommand> & commands, const SClientFile & clientFile)
   {
      for (unsigned int i = 0; i < commands.size(); ++i)
      {
         switch (commands[i].mType)
         {
         case NCommand::DIRECTIVE:
            putDirective(output, commands, clientFile, commands[i].mDirective, commands[i].mRVA);
            break;
         case NCommand::INSTRUCTION:
            putInstruction(output, commands[i].mInstruction, commands, commands[i].mRVA);
            break;
         case NCommand::DATA:
            putData(output, commands[i].mData, commands);
            break;
         case NCommand::EXTERN:
         case NCommand::IMPORT:
            break;
         case NCommand::SECTION:  // section begin !
         case NCommand::END:
            putZeroBytes(output, commands[i]);
            break;
         }
      }
   }

   void setExterns(vector<SCommand> & commands, const SClientFile & clientFile)
   {
      for (unsigned int i = 0; i < commands.size(); ++i)
      {
         if (commands[i].mType == NCommand::EXTERN)
         {
            if (commands[i].mNameLabel == "externImageBase")
            {
               SConstant constant;
               constant.mValue = clientFile.mImageBase;
               commands[i].mData.mConstants.push_back(constant);
            }
            else if (commands[i].mNameLabel == "externImageSize")
            {
               SConstant constant;
               constant.mValue = clientFile.mImageSize;
               commands[i].mData.mConstants.push_back(constant);
            }
            else if (commands[i].mNameLabel == "externOEP")
            {
               SConstant constant;
               constant.mValue = clientFile.mOEP;
               commands[i].mData.mConstants.push_back(constant);
            }
         }
      }
   }

   vector<SCommand> loadCommands()
   {
      vector<SCommand> commands;

      const HRSRC rsrcHandle = ::FindResource(NULL, MAKEINTRESOURCE(RESOURCE_IDENTIFIER_COMMANDS), RT_RCDATA);
      if (rsrcHandle != 0)
      {
         const DWORD rsrcRawSize = ::SizeofResource(NULL, rsrcHandle);
         if (rsrcRawSize != 0)
         {
            const HGLOBAL rsrcPtr = ::LoadResource(NULL, rsrcHandle);
            if (rsrcPtr != 0)
            {
               istringstream inputRsrc(string((char*)rsrcPtr, rsrcRawSize));

               deserialize(inputRsrc, commands);
            }
         }
      }
      if (commands.empty())
      {
         throw exception("Failed to load resources");
      }
      return commands;
   }
}

   void protectPe(ostream & output, const SClientFile & clientFile)
   {
      // get commands
      vector<SCommand> & commands = loadCommands();

      // set externs
      setExterns(commands, clientFile);
      LOG_DEBUG("before:");
      loggingCommands(commands);
      //mutateCommands(commands);
      //mutateCommands(commands);
      mutateCommands(commands);
      mutateCommands(commands);

      LOG_DEBUG("before:");
      loggingCommands(commands);

      // resolveLabels - done
      resolveLabels(commands, clientFile);
      
      LOG_DEBUG("Resolved:");
      loggingCommands(commands);

      putHeader(output, commands, clientFile);
      putBody(output, commands, clientFile);
   }
}

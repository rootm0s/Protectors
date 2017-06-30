#include "Types.h"

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

namespace NPeProtector
{
   namespace NDataType
   {
      const char * const gStrings[] =
      {
         "DD",
         "DWORD",
         "DW",
         "WORD",
         "DB",
         "BYTE",
      };
      const int gSize = ARRAY_SIZE(NDataType::gStrings);
   }

   namespace NCommand
   {
      const char * const gStrings[] =
      {
         "INSTRUCTION",
         "DATA",
         "DIRECTIVE",
         "IMPORT",
         "EXTERN",
         "SECTION",
         "END" 
      };
      const int gSize = ARRAY_SIZE(NCommand::gStrings);
   }
   namespace NSign
   {
      const char * const gStrings[] =
      {
         "PLUS",
         "MINUS"
      };
      const int gSize = ARRAY_SIZE(NSign::gStrings);
   }

   namespace NOperand
   {
      const char * const gStrings[] =
      {
         "NON",
         "REG8",
         "REG16",
         "REG32",
         "MEM8",
         "MEM16",
         "MEM32",
         "CONSTANT",
      };
      const int gSize = ARRAY_SIZE(NOperand::gStrings);
   }

   namespace NPrefix
   {
      const char * const gStrings[] =
      {
         "REPZ",
         "REPNZ",
         "NON"
      };
      const int gSize = ARRAY_SIZE(NPrefix::gStrings);
   }
   namespace NRegister
   {
      // strigns for type NRegister::EType
      const char * const gStrings[] =
      {
         "EAX",
         "EBX",
         "ECX",
         "EDX",
         "ESP",
         "EBP",
         "ESI",
         "EDI",
         "AX",
         "BX",
         "CX",
         "DX",
         "SP",
         "BP",
         "SI",
         "DI",
         "AL",
         "BL",
         "CL",
         "DL",
         "AH",
         "BH",
         "CH",
         "DH",
      };
      const int gSize = ARRAY_SIZE(NRegister::gStrings);
   }

   namespace NSegment
   {
      const char * const gStrings[] =
      {
         "CS",
         "DS",
         "ES",
         "FS",
         "GS",
         "SS",
         "NON",
      };
      const int gSize = ARRAY_SIZE(NSegment::gStrings);
   }

   namespace NInstruction
   {
      // create instructions string
      const char * const gStrings[] =
      {
#define OPCODE_FIRST(name, opcode1, opcode2, opcode3, opcode4, opcodeSize, operand1, operand2, operand3, flag) #name,
#define OPCODE_NEXT(name, opcode1, opcode2, opcode3, opcode4, opcodeSize, operand1, operand2, operand3, flag)

#include "../Compile/opcodes.def"

#undef OPCODE_NEXT
#undef OPCODE_FIRST   
      };
      const int gSize = ARRAY_SIZE(NInstruction::gStrings);
   }
}
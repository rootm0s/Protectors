#ifndef TYPES_H
#define TYPES_H

namespace NPeProtector
{
   /**
    * @brief Describes type of data in source code, ex: myData DWORD 666
    */
   namespace NDataType
   {
      enum EType
      {
         DD,
         DWORD,
         DW,
         WORD,
         DB,
         BYTE,
      };
      extern const int gSize;
      extern const char * const gStrings[];
   }
  
   /**
    * @brief Describes type of SCommand
    */
   namespace NCommand
   {
      enum EType
      {
         INSTRUCTION,
         DATA,
         DIRECTIVE,
         IMPORT,
         EXTERN,
         SECTION,
         END
      };
      extern const int gSize;
      extern const char * const gStrings[];
   }

   /**
    * @brief Describes section attributes
    */
   namespace NSectionAttributes
   {
      static const int READ = 1;
      static const int WRITE = 2;
      static const int EXECUTE = 4;
      static const int CODE = 8;
      static const int INITIALIZED = 16;
   }

   /**
    * @brief Describes sign of label
    */
   namespace NSign
   {
      enum EType
      {
         PLUS,
         MINUS
      };
      extern const int gSize;
      extern const char * const gStrings[];
   }
  
   /**
    * @brief Describes type of operand in assebmler instruction
    */
   namespace NOperand
   {
      enum EType
      {
         NON,
         REG8,
         REG16,
         REG32,
         MEM8,
         MEM16,
         MEM32,
         CONSTANT,
      };
      extern const int gSize;
      extern const char * const gStrings[];
   }

   /**
    * @brief Describes prefix of assembler instruction
    */
   namespace NPrefix
   {
      enum EType
      {
         REPZ,
         REPNZ,
         NON
      };
      extern const int gSize;
      extern const char * const gStrings[];
   }
  
   /**
    * @brief Describes register
    */
   namespace NRegister
   {
      enum EType
      {
         EAX,
         EBX,
         ECX,
         EDX,
         ESP,
         EBP,
         ESI,
         EDI,
         AX,
         BX,
         CX,
         DX,
         SP,
         BP,
         SI,
         DI,
         AL,
         BL,
         CL,
         DL,
         AH,
         BH,
         CH,
         DH,
      };
      extern const int gSize;
      extern const char * const gStrings[];
   }
  
   /**
    * @brief Describes segment register
    */
   namespace NSegment
   {
      enum EType
      {
         CS,
         DS,
         ES,
         FS,
         GS,
         SS,
         NON,
      };
      extern const int gSize;
      extern const char * const gStrings[];
   }
   
   /**
    * @brief Describes type of instruction
    */
   namespace NInstruction
   {
      enum EType
      {
 #define OPCODE_FIRST(name, opcode1, opcode2, opcode3, opcode4, opcodeSize, operand1, operand2, operand3, flag) name,
 #define OPCODE_NEXT(name, opcode1, opcode2, opcode3, opcode4, opcodeSize, operand1, operand2, operand3, flag)
 
 #include "../Compile/opcodes.def"
 
 #undef OPCODE_NEXT
 #undef OPCODE_FIRST   
      };
      extern const int gSize;
      extern const char * const gStrings[];
   }
}

#endif
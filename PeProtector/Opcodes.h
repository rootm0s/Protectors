#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <windows.h>
#include "../Library/Types.h"

namespace NPeProtector
{
   /**
    * @brief Describe assembler instruction
    */
   struct SOpcode
   {
      enum EOperand
      {
         AL,
         AX,
         EAX,
         CL,
         REG8,
         REG16,
         REG32,
         MEM8,
         MEM16,
         MEM32,
         IMM1,
         IMM8,
         IMM16,
         IMM32,
         REL8,
         REL16,
         REL32,
         NON,
      };

      enum EFlag
      {
         RM_R,
         R_RM,
         R0, // /0
         R1, // /1
         R2, // /2
         R3, // /3
         R4, // /4
         R5, // /5
         R6, // /6
         R7, // /7
         PB, // +rb
         PW, // +rw
         PD, // +rd
         MOFFS8,
         MOFFS16,
         MOFFS32,
         EMPTY,
      };

      NInstruction::EType mType;

      unsigned char mOpcode[4];
      char mOpcodeSize;

      EOperand mOperand1;
      EOperand mOperand2;
      EOperand mOperand3;

      EFlag mFlag;
   };

   extern const SOpcode gOpcodes[];
   extern const int gOpcodeSize;

}

#endif
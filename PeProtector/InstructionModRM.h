#ifndef INSTRUCTION_MOD_RM_H
#define INSTRUCTION_MOD_RM_H

#include "../Library/SCommand.h"
#include "iosfwd"

namespace NPeProtector
{
   int getDisplacementSize(const SConstant & constant, const bool isSigned);
   int getDisplacement(const SConstant & constant, const std::vector<SCommand> & commands);
   
   SMemory normalizeMemory(const SMemory & memory);

   char getModRM(const SOperand & operandRM, const int code);
   char getModRM(const SOperand & operandRM, const NRegister::EType operandReg);
   char getSIB(const SMemory & memory);
   bool isSIB_Required(const SMemory & memory);

   bool isDummyDisplacement8(const SMemory & memory);
   char getRegisterCode(const NRegister::EType reg);
}

#endif

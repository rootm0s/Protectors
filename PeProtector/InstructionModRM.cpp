#include "InstructionModRM.h"
#include "../Library/SCommand.h"
#include "exception"
#include "assert.h"
#include "PeHeader.h"

using std::exception;
using std::basic_ostream;
using std::char_traits;
using std::vector;

namespace NPeProtector
{
   namespace
   {
      int getLabel(const SLabel & label, const vector<SCommand> & commands)
      {
         int va = 0;
         if (commands[label.mIndex].mType == NCommand::EXTERN)
         {
            assert(commands[label.mIndex].mData.mConstants.size() == 1);
            va = commands[label.mIndex].mData.mConstants[0].mValue;
         }
         else
         {
            va = commands[label.mIndex].mRVA + gImageBase;
         }
         return label.mSign == NSign::MINUS ? 0 - va : va;
      }

      int getValueSize(int value, const bool isSigned)
      {
         if (value == 0)
         {
            return 0;
         }
         else if (isSigned && (value >= -128 && value <= 127))
         {
            return 1;
         }
         else if (!isSigned && ((value >= -128 && value <= 127) || (value >= 0 && value <= 0xFF)))
         {
            return 1;
         }
         else
         {
            return 4;
         }
      }
     
      NRegister::EType getRegisterFromCode(const int code)
      {
         switch (code)
         {
         case 0x00:
            return NRegister::EAX;
         case 0x01:
            return NRegister::ECX;
         case 0x02:
            return NRegister::EDX;
         case 0x03:
            return NRegister::EBX;
         case 0x04:
            return NRegister::ESP;
         case 0x05:
            return NRegister::EBP;
         case 0x06:
            return NRegister::ESI;
         case 0x07:
            return NRegister::EDI;
         }
         throw exception("Failed to get register from code field");
      }

      char getModRM_Mod(const SOperand & rm)
      {
         switch (rm.mType)
         {
         case NOperand::REG32:
         case NOperand::REG16:
         case NOperand::REG8:
            return 0x03;
         case NOperand::MEM32:
         case NOperand::MEM16:
         case NOperand::MEM8:
         {
            const int numberRegistersInMemory = rm.mMemory.mRegisters.size();
            const int sizeOfDisplacement = getDisplacementSize(rm.mMemory.mConstant, true);
            if ((numberRegistersInMemory == 1 && rm.mMemory.mRegisters[0] == NRegister::EBP && sizeOfDisplacement == 0) ||
               (numberRegistersInMemory == 2 && rm.mMemory.mRegisters[1] == NRegister::EBP && sizeOfDisplacement == 0))
            {
               // dword ptr [ebp] - There's no such instruction, it needs to use fake disp8
               // dword ptr [eax + ebp] - There's no such instruction, it needs to use fake disp8
               // dword ptr [2 * eax + ebp] - There's no such instruction, it needs to use fake disp8
               return 0x01;
            }
            if (((sizeOfDisplacement == 0) || (sizeOfDisplacement == 4 && numberRegistersInMemory == 0)) ||
                ((sizeOfDisplacement == 0) || (sizeOfDisplacement == 1 && numberRegistersInMemory == 0)))
            {
               return 0x00;
            }
            else if (sizeOfDisplacement == 1)
            {
               return 0x01;
            }
            else if (sizeOfDisplacement == 4 && numberRegistersInMemory > 0)
            {
               return 0x02;
            }
         }
         }
         throw exception("Failed to get modrm mod field");
      }

      char getModRM_RM(const SOperand & rm)
      {
         NRegister::EType reg = NRegister::EType(0);

         switch (rm.mType)
         {
         case NOperand::REG32:
         case NOperand::REG16:
         case NOperand::REG8:
            return getRegisterCode(rm.mRegister);
         case NOperand::MEM32:
         case NOperand::MEM16: 
         case NOperand::MEM8: 
            if (rm.mMemory.mRegisters.size() == 0)
            {
               return 0x05;
            }
            else if (rm.mMemory.mRegisters.size() == 1)
            {
               switch (rm.mMemory.mRegisters[0])
               {
               case NRegister::EAX:
               case NRegister::AX:
               case NRegister::AL:
                  return 0x00;
               case NRegister::ECX:
               case NRegister::CX:
               case NRegister::CL:
                  return 0x01;
               case NRegister::EDX:
               case NRegister::DX:
               case NRegister::DL:
                  return 0x02;
               case NRegister::EBX:
               case NRegister::BX:
               case NRegister::BL:
                  return 0x03;
               case NRegister::ESP:
               case NRegister::SP:
               case NRegister::AH:
                  return 0x04; // point to sib byte
               case NRegister::EBP:
               case NRegister::BP:
               case NRegister::CH:
                  return 0x05;
//                   if (getDisplacementSize(rm.mMemory.mConstant) == 0)
//                   {
//                      return 0x04; //point to sib byte
//                   }
//                   else
//                   {
//                      return 0x05;
//                   }
               case NRegister::ESI:
               case NRegister::SI:
               case NRegister::DH:
                  return 0x06;
               case NRegister::EDI:
               case NRegister::DI:
               case NRegister::BH:
                  return 0x07;
               }
            }
            else if (rm.mMemory.mRegisters.size() == 2)
            {
               return 0x04; // point to sib byte
            }
         }
         throw exception("Failed to get modrm rm field");
      }

      char getModRM_Reg(const NRegister::EType reg)
      {
         return getRegisterCode(reg);
      }

      char getSIB_SS(const int scale)
      {
         switch (scale)
         {
         case 0:
            return 0x00;
         case 2:
            return 0x01;
         case 4:
            return 0x02;
         case 8:
            return 0x03;
         }
         throw exception("Failed to get sib ss field");
      }

      char getSIB_Index(const SMemory & memory)
      {
         if (memory.mRegisters.size() == 1 && memory.mRegisters[0] == NRegister::ESP)
         {
            return 0x04;
         }
         else if (memory.mRegisters.size() == 2)
         {
            return getRegisterCode(memory.mRegisters[0]);
         }
         throw exception("Failed to get SIB index");
      }

      char getSIB_Reg(const SMemory & memory)
      {
         if (memory.mRegisters.size() == 1 && memory.mRegisters[0] == NRegister::ESP)
         {
            return 0x04;
         }
         else if (memory.mRegisters.size() == 2)
         {
            return getRegisterCode(memory.mRegisters[1]);
         }
         throw exception("Failed to get SIB register");
      }

      bool isOperandMemory(const NOperand::EType type)
      {
         return type == NOperand::MEM8 || type == NOperand::MEM16 || type == NOperand::MEM32;
      }
   }
   
   char getModRM(const SOperand & operandRM, const int code)
   {
      return getModRM(operandRM, getRegisterFromCode(code));
   }
   
   SMemory normalizeMemory(const SMemory & memory)
   {
      SMemory result(memory);

      if (memory.mRegisters.size() > 0 && memory.mRegisters[0] == NRegister::ESP && memory.mScale > 0)
      {
         throw exception("Impossible to create [NUMBER * ESP]");
      }
      else if (memory.mRegisters.size() == 2 && memory.mRegisters[0] == NRegister::ESP && memory.mRegisters[1] == NRegister::ESP)
      {
         throw exception("Impossible to create [ESP + ESP]");
      }
      else if (memory.mRegisters.size() == 1 && memory.mScale > 0)
      {
         throw exception("Impossible to create [NUMBER * REG]");
      }
      else if (memory.mRegisters.size() == 2 && memory.mRegisters[0] == NRegister::ESP && memory.mScale == 0)
      {
         // swap registers
         result.mRegisters[1] = memory.mRegisters[0];
         result.mRegisters[0] = memory.mRegisters[1];
      }
      return result;
   }

   char getModRM(const SOperand & operandRM, const NRegister::EType operandReg)
   {
      struct SModRM
      {
         char mRM : 3;
         char mReg : 3;
         char mMod : 2;
      };

      SModRM modrm = { 0 };
      modrm.mRM = getModRM_RM(operandRM);
      modrm.mMod = getModRM_Mod(operandRM);
      modrm.mReg = getModRM_Reg(operandReg);

      assert(sizeof(modrm) == 1);
      return *(reinterpret_cast<char*>(&modrm));
   }

   bool isSIB_Required(const SMemory & memory)
   {
      return memory.mRegisters.size() == 2 ||
         (memory.mRegisters.size() == 1 && memory.mRegisters[0] == NRegister::ESP);
   }

   char getSIB(const SMemory & memory)
   {
      struct SSIB
      {
         char mReg : 3;
         char mIndex : 3;
         char mSS : 2;
      };

      SSIB sib = { 0 };
      sib.mReg = getSIB_Reg(memory);
      sib.mIndex = getSIB_Index(memory);
      sib.mSS = getSIB_SS(memory.mScale);

      assert(sizeof(sib) == 1);

      return *(reinterpret_cast<char*>(&sib));
   }

   int getDisplacementSize(const SConstant & constant, const bool isSigned)
   {
      if (!constant.mLabels.empty())
      {
         return 4;
      }
      else
      {
         return getValueSize(constant.mValue, isSigned);
      }
      return 0;
   }

   int getDisplacement(const SConstant & constant, const vector<SCommand> & commands)
   {
      int result = constant.mValue;
      for (unsigned int i = 0; i < constant.mLabels.size(); ++i)
      {
         result += getLabel(constant.mLabels[i], commands);
      }
      return result;
   }

   bool isDummyDisplacement8(const SMemory & memory)
   {
      bool result = getDisplacementSize(memory.mConstant, true) == 0;
      if (result)
      {
         result = ((memory.mRegisters.size() == 1) && (memory.mRegisters[0] == NRegister::EBP)) ||
            ((memory.mRegisters.size() == 2) && (memory.mRegisters[1] == NRegister::EBP));
      }
      return result;
   }

   char getRegisterCode(const NRegister::EType reg)
   {
      switch (reg)
      {
      case NRegister::EAX:
      case NRegister::AX:
      case NRegister::AL:
         return 0x00;
      case NRegister::ECX:
      case NRegister::CX:
      case NRegister::CL:
         return 0x01;
      case NRegister::EDX:
      case NRegister::DX:
      case NRegister::DL:
         return 0x02;
      case NRegister::EBX:
      case NRegister::BX:
      case NRegister::BL:
         return 0x03;
      case NRegister::ESP:
      case NRegister::SP:
      case NRegister::AH:
         return 0x04;
      case NRegister::EBP:
      case NRegister::BP:
      case NRegister::CH:
         return 0x05;
      case NRegister::ESI:
      case NRegister::SI:
      case NRegister::DH:
         return 0x06;
      case NRegister::EDI:
      case NRegister::DI:
      case NRegister::BH:
         return 0x07;
      }
      throw exception("Failed to get register code field");
   }

}

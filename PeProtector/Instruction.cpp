#include "Instruction.h"
#include <ostream>
#include "Opcodes.h"
#include "InstructionModRM.h"
#include "assert.h"
#include "PeHeader.h"

using std::ostream;
using std::vector;
using std::exception;

namespace NPeProtector
{
namespace
{
   int getSegmentSize(const NSegment::EType segment)
   {
      switch (segment)
      {
      case NSegment::CS:
      case NSegment::DS:
      case NSegment::ES:
      case NSegment::FS:
      case NSegment::GS:
      case NSegment::SS:
         return 1;
      }
      return 0;
   }

   void putSegment(ostream & output, const NSegment::EType segment)
   {
      char value = 0;
      switch (segment)
      {
      case NSegment::CS:
         value = 0x2E;
         break;
      case NSegment::DS:
         value = 0x3e;
         break;
      case NSegment::ES:
         value = 0x26;
         break;
      case NSegment::FS:
         value = 0x64;
         break;
      case NSegment::GS:
         value = 0x65;
         break;
      case NSegment::SS:
         value = 0x36;
         break;
      }
      if (value != 0)
      {
         output.write(&value, 1);
      }
   }

   void putPrefix(ostream & output, const NPrefix::EType prefix)
   {
      unsigned char value = 0;
      switch (prefix)
      {
      case NPrefix::REPZ:
         value = 0xF3;
         break;
      case NPrefix::REPNZ:
         value = 0xF2;
         break;
      }
      if (value != 0)
      {
         output.write((char*)&value, 1);
      }
   }

   int getPrefixSize(const NPrefix::EType prefix)
   {
      switch (prefix)
      {
      case NPrefix::REPZ:
      case NPrefix::REPNZ:
         return 1;
      }
      return 0;
   }

   bool isMatchOperand(const SOpcode::EOperand opcodeOperand, const SOperand & operand)
   {
      switch (opcodeOperand)
      {
      case SOpcode::AL:
         return operand.mType == NOperand::REG8 && operand.mRegister == NRegister::AL;
      case SOpcode::AX:
         return operand.mType == NOperand::REG16 && operand.mRegister == NRegister::AX;
      case SOpcode::EAX:
         return operand.mType == NOperand::REG32 && operand.mRegister == NRegister::EAX;
      case SOpcode::CL:
         return operand.mType == NOperand::REG8 && operand.mRegister == NRegister::CL;
      case SOpcode::REG8:
         return operand.mType == NOperand::REG8;
      case SOpcode::REG16:
         return operand.mType == NOperand::REG16;
      case SOpcode::REG32:
         return operand.mType == NOperand::REG32;
      case SOpcode::MEM8:
         return operand.mType == NOperand::MEM8;
      case SOpcode::MEM16:
         return operand.mType == NOperand::MEM16;
      case SOpcode::MEM32:
         return operand.mType == NOperand::MEM32;
      case SOpcode::IMM1:
         return operand.mType == NOperand::CONSTANT && operand.mConstant.mValue == 1 && operand.mConstant.mLabels.empty();
      case SOpcode::IMM8:
         return operand.mType == NOperand::CONSTANT && getDisplacementSize(operand.mConstant, false) == 1;
      case SOpcode::IMM16:
         return operand.mType == NOperand::CONSTANT && getDisplacementSize(operand.mConstant, false) == 2;
      case SOpcode::IMM32:
         return operand.mType == NOperand::CONSTANT;
      case SOpcode::REL8:
      case SOpcode::REL16:
         return false;
      case SOpcode::REL32:
         return operand.mType == NOperand::CONSTANT;
      case SOpcode::NON:
         throw exception("Failed to get type of operand");
      };
      return false;
   }

   bool isMatch(const SOpcode & opcode, const SInstruction & instruction)
   {
      if (opcode.mType == instruction.mType)
      {
         switch (instruction.mOperands.size())
         {
         case 0:
            return opcode.mOperand1 == SOpcode::NON &&
               opcode.mOperand2 == SOpcode::NON &&
               opcode.mOperand3 == SOpcode::NON;
         case 1:
            return opcode.mOperand1 != SOpcode::NON &&
               opcode.mOperand2 == SOpcode::NON &&
               opcode.mOperand3 == SOpcode::NON &&
               isMatchOperand(opcode.mOperand1, instruction.mOperands[0]);
         case 2:
            return opcode.mOperand1 != SOpcode::NON &&
               opcode.mOperand2 != SOpcode::NON &&
               opcode.mOperand3 == SOpcode::NON &&
               isMatchOperand(opcode.mOperand1, instruction.mOperands[0]) &&
               isMatchOperand(opcode.mOperand2, instruction.mOperands[1]);
         case 3:
            return opcode.mOperand1 != SOpcode::NON &&
               opcode.mOperand2 != SOpcode::NON &&
               opcode.mOperand3 != SOpcode::NON &&
               isMatchOperand(opcode.mOperand1, instruction.mOperands[0]) &&
               isMatchOperand(opcode.mOperand2, instruction.mOperands[1]) &&
               isMatchOperand(opcode.mOperand3, instruction.mOperands[2]);
         default:
            //error
            throw exception("TODO");
         }
      }
      return false;
   }
   bool isOperandMemory(const NOperand::EType type)
   {
      return type == NOperand::MEM8 || type == NOperand::MEM16 || type == NOperand::MEM32;
   }

   const SOpcode & getOpcode(const SInstruction & instruction)
   {
      for (int i = 0; i < gOpcodeSize; ++i)
      {
         if (isMatch(gOpcodes[i], instruction))
         {
            return gOpcodes[i];
         }
      }
      throw exception("Failed to find proper instruction");
   }

   bool isOperandImm(const SOpcode::EOperand opcodeOperand)
   {
      return opcodeOperand == SOpcode::IMM8 ||
         opcodeOperand == SOpcode::IMM16 ||
         opcodeOperand == SOpcode::IMM32;
   }

   bool isOperandRel(const SOpcode::EOperand opcodeOperand)
   {
      return opcodeOperand == SOpcode::REL8 ||
         opcodeOperand == SOpcode::REL16 ||
         opcodeOperand == SOpcode::REL32;
   }

   int getOperandSize(const SOpcode::EOperand opcodeOperand)
   {
      switch (opcodeOperand)
      {
      case SOpcode::IMM8:
      case SOpcode::REL8:
         return 1;
      case SOpcode::IMM16:
      case SOpcode::REL16:
         return 2;
      case SOpcode::IMM32:
      case SOpcode::REL32:
         return 4;
      }
      return 0;
   }

   void putConstant(ostream & output, const int value, const int size)
   {
      switch (size)
      {
      case 0:
      {
         break;
      }
      case 1:
      {
         const char c = static_cast<char>(value);
         output.write(&c, 1);
         break;
      }
      case 2:
      {
         const short s = static_cast<short>(value);
         output.write((char*)&value, 2);
         break;
      }
      case 4:
      {
         output.write((char*)&value, 4);
         break;
      }
      default:
         throw exception("Failed to put constant");
      }
   }

   void putConstant(ostream & output, const SOpcode::EOperand opcodeOperand, const SConstant & constant, const vector<SCommand> & commands)
   {
      // process second operand
      switch (opcodeOperand)
      {
      case SOpcode::IMM8:
      {
         const char value = static_cast<char>(getDisplacement(constant, commands));
         output.write(&value, 1);
         break;
      }
      case SOpcode::IMM16:
      {
         const short value = static_cast<short>(getDisplacement(constant, commands));
         output.write((char*)&value, 2);
         break;
      }
      case SOpcode::IMM32:
      {
         const int value = getDisplacement(constant, commands);
         output.write((char*)&value, 4);
         break;
      }
      default:
         throw exception("Failed to put operand");
      }
   }

   void putRelativeConstant(ostream & output, const SOpcode::EOperand opcodeOperand, const SConstant & constant, const vector<SCommand> & commands, DWORD rva)
   {
      // process second operand
      switch (opcodeOperand)
      {
      case SOpcode::REL8:
      case SOpcode::REL16:
      {
         throw exception("Failed to put operand");
         break;
      }
      case SOpcode::REL32:
      {
         DWORD disp = static_cast<DWORD>(getDisplacement(constant, commands));
         const DWORD value = disp - (rva + 4 + gImageBase);
         output.write((char*)&value, 4);
         break;
      }
      default:
         throw exception("Failed to put operand");
      }
   }

   void putDisplacement(ostream & output, const SMemory & memory, const vector<SCommand> & commands)
   {

      const int displacementSize = memory.mRegisters.size() == 0 ? 4 : getDisplacementSize(memory.mConstant, true);

      if (displacementSize == 0)
      {
         if (isDummyDisplacement8(memory))
         {
            putConstant(output, 0, 1);
         }
      }
      else
      {
         const int displacement = getDisplacement(memory.mConstant, commands);
         // put 1 or 2 or 4 bytes
         putConstant(output, displacement, displacementSize);
      }
   }

}

   int getInstructionSize(const SInstruction & instruction)
   {
      const SOpcode & opcode = getOpcode(instruction);

      //putPrefix(output, instruction.mPrefix);

      int result = opcode.mOpcodeSize;

      result += getPrefixSize(instruction.mPrefix);
      switch (opcode.mFlag)
      {
         case SOpcode::R0: // /0
         case SOpcode::R1: // /1
         case SOpcode::R2: // /2
         case SOpcode::R3: // /3
         case SOpcode::R4: // /4
         case SOpcode::R5: // /5
         case SOpcode::R6: // /6
         case SOpcode::R7: // /7
         case SOpcode::RM_R:
         {
            result += 1; /*ModRM*/
            if (isOperandMemory(instruction.mOperands[0].mType))
            {
               SMemory RM_Memory = normalizeMemory(instruction.mOperands[0].mMemory);
               result += isSIB_Required(RM_Memory);
               result += isDummyDisplacement8(RM_Memory);
               result += RM_Memory.mRegisters.size() == 0 ? 4 : getDisplacementSize(RM_Memory.mConstant, true);
               result += getSegmentSize(RM_Memory.mSegment);
            }

            break;
         }
         case SOpcode::R_RM:
         {
            result += 1; /*ModRM*/
            if (isOperandMemory(instruction.mOperands[1].mType))
            {
               SMemory RM_Memory = normalizeMemory(instruction.mOperands[1].mMemory);
               result += isSIB_Required(RM_Memory);
               result += isDummyDisplacement8(RM_Memory);
               result += RM_Memory.mRegisters.size() == 0 ? 4 : getDisplacementSize(RM_Memory.mConstant, true);
               result += getSegmentSize(RM_Memory.mSegment);
            }
            break;
         }
         case SOpcode::MOFFS8:
         {
            result += 1;
            break;
         }
         case SOpcode::MOFFS16:
         {
            result += 2;
            break;
         }
         case SOpcode::MOFFS32:
         {
            result += 4;
            break;
         }
         case SOpcode::PB: // +rb
         case SOpcode::PW: // +rw
         case SOpcode::PD: // +rd
         case SOpcode::EMPTY:
            break;
      }
      result += getOperandSize(opcode.mOperand1) + getOperandSize(opcode.mOperand2) + getOperandSize(opcode.mOperand3);
      return result;
   }

   void putInstruction(ostream & output, const SInstruction & instruction, const vector<SCommand> & commands, DWORD rva)
   {
      const SOpcode & opcode = getOpcode(instruction);

      putPrefix(output, instruction.mPrefix);

      switch (opcode.mFlag)
      {
      case SOpcode::R0: // /0
      case SOpcode::R1: // /1
      case SOpcode::R2: // /2
      case SOpcode::R3: // /3
      case SOpcode::R4: // /4
      case SOpcode::R5: // /5
      case SOpcode::R6: // /6
      case SOpcode::R7: // /7
      {
         SOperand normalizedRM(instruction.mOperands[0]);
         if (isOperandMemory(normalizedRM.mType))
         {
            normalizedRM.mMemory = normalizeMemory(instruction.mOperands[0].mMemory);

            putSegment(output, normalizedRM.mMemory.mSegment);
         }

         output.write((char*)opcode.mOpcode, opcode.mOpcodeSize);

         // process first operand
         const char modRM = getModRM(normalizedRM, opcode.mFlag - SOpcode::R0);
         output.write(&modRM, 1);

         if (isOperandMemory(normalizedRM.mType))
         {
            if (isSIB_Required(normalizedRM.mMemory))
            {
               const char sib = getSIB(normalizedRM.mMemory);
               output.write(&sib, 1);
            }

            putDisplacement(output, normalizedRM.mMemory, commands);
         }
         break;
      }
      case SOpcode::RM_R:
      {
         // process both operands
         assert(instruction.mOperands.size() > 1 && (
            instruction.mOperands[1].mType == NOperand::REG8 ||
            instruction.mOperands[1].mType == NOperand::REG16 ||
            instruction.mOperands[1].mType == NOperand::REG32));

         SOperand normalizedRM(instruction.mOperands[0]);
         if (isOperandMemory(normalizedRM.mType))
         {
            normalizedRM.mMemory = normalizeMemory(instruction.mOperands[0].mMemory);

            putSegment(output, normalizedRM.mMemory.mSegment);
         }

         output.write((char*)opcode.mOpcode, opcode.mOpcodeSize);

         const char modRM = getModRM(normalizedRM, instruction.mOperands[1].mRegister);
         output.write(&modRM, 1);

         if (isOperandMemory(normalizedRM.mType))
         {
            if (isSIB_Required(normalizedRM.mMemory))
            {
               const char sib = getSIB(normalizedRM.mMemory);
               output.write(&sib, 1);
            }

            putDisplacement(output, normalizedRM.mMemory, commands);
         }
         break;
      }
      case SOpcode::R_RM:
      {
         // process both operands
         assert(instruction.mOperands.size() > 1 && (
            instruction.mOperands[0].mType == NOperand::REG8 ||
            instruction.mOperands[0].mType == NOperand::REG16 ||
            instruction.mOperands[0].mType == NOperand::REG32));

         SOperand normalizedRM(instruction.mOperands[1]);
         if (isOperandMemory(normalizedRM.mType))
         {
            normalizedRM.mMemory = normalizeMemory(instruction.mOperands[1].mMemory);
            putSegment(output, normalizedRM.mMemory.mSegment);
         }

         output.write((char*)opcode.mOpcode, opcode.mOpcodeSize);

         const char modRM = getModRM(normalizedRM, instruction.mOperands[0].mRegister);
         output.write(&modRM, 1);

         if (isOperandMemory(normalizedRM.mType))
         {
            if (isSIB_Required(normalizedRM.mMemory))
            {
               const char sib = getSIB(normalizedRM.mMemory);
               output.write(&sib, 1);
            }

            putDisplacement(output, normalizedRM.mMemory, commands);
         }
         break;
      }
      case SOpcode::MOFFS8:
      {
         break;
      }
      case SOpcode::MOFFS16:
      {
         break;
      }
      case SOpcode::MOFFS32:
      {
         break;
      }
      case SOpcode::PB: // +rb
      case SOpcode::PW: // +rw
      case SOpcode::PD: // +rd
      {
         SOpcode opcodeRegister(opcode);

         if (instruction.mOperands.size() >= 1 && 
           (instruction.mOperands[0].mType == NOperand::REG8 || 
            instruction.mOperands[0].mType == NOperand::REG16 ||
            instruction.mOperands[0].mType == NOperand::REG32))
         {
            opcodeRegister.mOpcode[opcodeRegister.mOpcodeSize - 1] |= getRegisterCode(instruction.mOperands[0].mRegister);
         }
         output.write((char*)opcodeRegister.mOpcode, opcodeRegister.mOpcodeSize);
         break;
      }
      case SOpcode::EMPTY:
         output.write((char*)opcode.mOpcode, opcode.mOpcodeSize);
         rva += opcode.mOpcodeSize; // it needs for relative address !
         break;
      }

      if (isOperandImm(opcode.mOperand1))
      {
         assert(instruction.mOperands.size() > 0 && instruction.mOperands[0].mType == NOperand::CONSTANT);
         putConstant(output, opcode.mOperand1, instruction.mOperands[0].mConstant, commands);
      }

      if (isOperandImm(opcode.mOperand2))
      {
         assert(instruction.mOperands.size() > 1 && instruction.mOperands[1].mType == NOperand::CONSTANT);
         putConstant(output, opcode.mOperand2, instruction.mOperands[1].mConstant, commands);
      }

      if (isOperandImm(opcode.mOperand3))
      {
         assert(instruction.mOperands.size() > 2 && instruction.mOperands[2].mType == NOperand::CONSTANT);
         putConstant(output, opcode.mOperand3, instruction.mOperands[2].mConstant, commands);
      }

      if (isOperandRel(opcode.mOperand1))
      {
         assert(instruction.mOperands.size() > 0 && instruction.mOperands[0].mType == NOperand::CONSTANT);
         putRelativeConstant(output, opcode.mOperand1, instruction.mOperands[0].mConstant, commands, rva);
      }

      if (isOperandRel(opcode.mOperand2))
      {
         assert(instruction.mOperands.size() > 1 && instruction.mOperands[1].mType == NOperand::CONSTANT);
         putRelativeConstant(output, opcode.mOperand2, instruction.mOperands[1].mConstant, commands, rva);
      }

      if (isOperandRel(opcode.mOperand3))
      {
         assert(instruction.mOperands.size() > 2 && instruction.mOperands[2].mType == NOperand::CONSTANT);
         putRelativeConstant(output, opcode.mOperand3, instruction.mOperands[2].mConstant, commands, rva);
      }
   }
}
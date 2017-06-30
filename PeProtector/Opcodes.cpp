#include "Opcodes.h"
#include "winnt.h"

namespace NPeProtector
{
   const SOpcode gOpcodes[] =
   {
#define OPCODE_FIRST(name, opcode1, opcode2, opcode3, opcode4, opcodeSize, operand1, operand2, operand3, flag) {NInstruction::name, opcode1, opcode2, opcode3, opcode4, opcodeSize, SOpcode::operand1, SOpcode::operand2, SOpcode::operand3, SOpcode::flag},
#define OPCODE_NEXT(name, opcode1, opcode2, opcode3, opcode4, opcodeSize, operand1, operand2, operand3, flag)  {NInstruction::name, opcode1, opcode2, opcode3, opcode4, opcodeSize, SOpcode::operand1, SOpcode::operand2, SOpcode::operand3, SOpcode::flag},
#include "../Compile/Opcodes.def"
#undef OPCODE_NEXT
#undef OPCODE_FIRST   
   };
   const int gOpcodeSize = ARRAYSIZE(gOpcodes);
}

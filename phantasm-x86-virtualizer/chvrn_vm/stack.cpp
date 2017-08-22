/*
	Stack operations
	push, pop
*/
#include "stdafx.h"
#include "x86vm.h"
#include "cli.h"


// PUSH and POP register virtualizer
// Supports opcodes: 50-5F, 68
// 50-57 are PUSH 32-bit reg, 58-5F are POP 32-bit reg
bool VirtualizePushPopRegister(CodeChunk *code, DISASM *disasm) {

	// push DWORD or push BYTE extended to DWORD (!= push with prefix BYTE)
	if (disasm->Instruction.Opcode == 0x68 || disasm->Instruction.Opcode == 0x6A) {
		DWORD value = disasm->Instruction.Immediat;
		doVmPush(code, disasm, value);
		doPushDword(code, disasm);
		return true;
	}

	BYTE reg;
	// PUSH reg
	if (0x50 <= disasm->Instruction.Opcode && disasm->Instruction.Opcode <= 0x57) {
		reg = disasm->Instruction.Opcode & 0x07;
		doPushReg(code, disasm, reg);
		return true;
	}

	// POP reg
	if (0x58 <= disasm->Instruction.Opcode && disasm->Instruction.Opcode <= 0x5f) {
		reg = (disasm->Instruction.Opcode - 0x08) & 0x07;
		doPopReg(code, disasm, reg);
		return true;
	}
	
	return false;
}
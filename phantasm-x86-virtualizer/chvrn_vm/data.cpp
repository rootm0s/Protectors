/*
	Data transfer instructions
	mov, lea
*/
#include "stdafx.h"
#include "x86vm.h"
#include "cli.h"

// MOV (+ LEA) virtualizer
bool VirtualizeMov(CodeChunk *code, DISASM *disasm) {
	if ((disasm->Argument1.ArgType & MEMORY_TYPE) != 0) {	// first arg is mem, second is reg or constant
		if ((disasm->Argument2.ArgType & CONSTANT_TYPE) != 0) {	// second arg is constant
			if (g_TargetPe->isRelocatableVa(disasm->Instruction.Immediat)) {
				doVmPush(code, disasm, disasm->Instruction.Immediat-g_TargetPe->getNtHeaders()->OptionalHeader.ImageBase);
				//doVmRebase(code, disasm);
			}
			else {
				doVmPush(code, disasm, disasm->Instruction.Immediat);
			}
		}
		else { // second arg is a reg
			doVmPushReg(code, disasm, getSingleRegister(disasm->Argument2.ArgType));
		}

		pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
		doMove(code, disasm);
	}
	else if ((disasm->Argument2.ArgType & MEMORY_TYPE) != 0) {	// first arg reg, second mem
																// First argument can only be a register
		pushCalculateScaledAddress(code, disasm, &disasm->Argument2);

		if (disasm->Instruction.Opcode != 0x8d) { // no deref for the special snowflake LEA...
			doDeref(code, disasm);
		}

		doVmPopReg(code, disasm, getSingleRegister(disasm->Argument1.ArgType));
	}
	else {	// first arg reg, second constant, or both args reg
		if (disasm->Argument2.ArgType & REGISTER_TYPE) { // second is reg
			doVmPushReg(code, disasm, getSingleRegister(disasm->Argument2.ArgType));
		}
		else { // second is constant
			doVmPush(code, disasm, disasm->Instruction.Immediat);
			if (g_TargetPe->isRelocatableVa(disasm->Instruction.Immediat)) {
				//doVmRebase(code, disasm);
			}
		}

		doVmPopReg(code, disasm, getSingleRegister(disasm->Argument1.ArgType));
	}
	return true;
}
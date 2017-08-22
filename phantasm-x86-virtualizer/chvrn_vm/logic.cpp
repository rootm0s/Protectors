/*
	Logical and bitwise instructions
	not, and, or, xor
*/
#include "stdafx.h"
#include "Vm.h"
#include "VmInstruction.h"
#include "CodeChunk.h"
#include "x86vm.h"
#include "logic.h"

bool Virtualize81(CodeChunk *code, DISASM *disasm) {
	BYTE reg = getReg(disasm);

	switch (reg) {
	case 0:	// ADD
		return VirtualizeAdd(code, disasm);
	case 1:	// OR
		return VirtualizeOr(code, disasm);
	case 2:	// ADC
		break;
	case 3:	// SBB
		break;
	case 4:	// AND
		return VirtualizeAnd(code, disasm);
	case 5:	// SUB
		return VirtualizeSub(code, disasm);
	case 6:	// XOR
		return VirtualizeXor(code, disasm);
	case 7:	// CMP
		return VirtualizeCmp(code, disasm);
	default:
		return false;
	}

	return false;
}

// Opcodes: 83
bool Virtualize83(CodeChunk *code, DISASM *disasm) {

	//DWORD opcode = disasm->Instruction.Opcode;
	BYTE reg = getReg(disasm);

	switch (reg) {
	case 0:	// ADD
		return VirtualizeAdd(code, disasm);
	case 1:	// OR
		return VirtualizeOr(code, disasm);
	case 2:	// ADC
	case 3:	// SBB
		return false;
	case 4:	// AND
		return VirtualizeAnd(code, disasm);
	case 5:	// SUB
		return VirtualizeSub(code, disasm);
	case 6:	// XOR
		return VirtualizeXor(code, disasm);
	case 7:	// CMP
		return VirtualizeCmp(code, disasm);
	default:
		return false;
	}
}

bool VirtualizeAnd(CodeChunk *code, DISASM *disasm) {
	pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
	pushCalculateScaledAddress(code, disasm, &disasm->Argument2);
	doNand(code, disasm);
	doRepush(code, disasm);
	doNand(code, disasm);

	if ((disasm->Argument1.ArgType & MEMORY_TYPE) != 0) {
		doMove(code, disasm);
	}
	else if ((disasm->Argument1.ArgType & REGISTER_TYPE) != 0) {
		doVmPopReg(code, disasm, getSingleRegister(disasm->Argument1.ArgType));
	}
	return true;
}

bool VirtualizeOr(CodeChunk *code, DISASM *disasm) {
	pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
	doRepush(code, disasm);
	doNand(code, disasm);

	pushCalculateScaledAddress(code, disasm, &disasm->Argument2);
	doRepush(code, disasm);
	doNand(code, disasm);

	doNand(code, disasm);

	if ((disasm->Argument1.ArgType & MEMORY_TYPE) != 0) {
		doMove(code, disasm);
	}
	else if ((disasm->Argument1.ArgType & REGISTER_TYPE) != 0) {
		doVmPopReg(code, disasm, getSingleRegister(disasm->Argument1.ArgType));
	}
	return true;
}

bool VirtualizeXor(CodeChunk *code, DISASM *disasm) {
	
	pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
	pushCalculateScaledAddress(code, disasm, &disasm->Argument2);

	doNand(code, disasm);

	pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
	doNand(code, disasm);

	pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
	pushCalculateScaledAddress(code, disasm, &disasm->Argument2);
	doNand(code, disasm);

	pushCalculateScaledAddress(code, disasm, &disasm->Argument2);
	doNand(code, disasm);

	doNand(code, disasm);

	if ((disasm->Argument1.ArgType & MEMORY_TYPE) != 0) {
		doMove(code, disasm);
	}
	else if ((disasm->Argument1.ArgType & REGISTER_TYPE) != 0) {
		doVmPopReg(code, disasm, getSingleRegister(disasm->Argument1.ArgType));
	}

	return true;
}

bool VirtualizeAddSub(CodeChunk *code, DISASM *disasm, bool isAdd) {

	pushCalculateScaledAddress(code, disasm, &disasm->Argument2);
	if ((disasm->Argument2.ArgType & MEMORY_TYPE) != 0) {
		doDeref(code, disasm);
	}

	pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
	if ((disasm->Argument1.ArgType & MEMORY_TYPE) != 0) {
		doDeref(code, disasm);
	}

	if (isAdd) {
		doAdd(code, disasm);
	}
	else {
		doSub(code, disasm);
	}

	if ((disasm->Argument1.ArgType & MEMORY_TYPE) != 0) {
		pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
		doMove(code, disasm);
	}
	else if ((disasm->Argument1.ArgType & REGISTER_TYPE) != 0) {
		doVmPopReg(code, disasm, getSingleRegister(disasm->Argument1.ArgType));
	}

	return true;
}

bool VirtualizeAdd(CodeChunk *code, DISASM *disasm) {
	return VirtualizeAddSub(code, disasm, true);
}

bool VirtualizeSub(CodeChunk *code, DISASM *disasm) {
	return VirtualizeAddSub(code, disasm, false);
}

// 0xf7
bool VirtualizeNeg(CodeChunk *code, DISASM *disasm) {
	pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
	doVmPush(code, disasm, 0);
	doSub(code, disasm);

	if ((disasm->Argument1.ArgType & MEMORY_TYPE) != 0) {
		doMove(code, disasm);
	}
	else if ((disasm->Argument1.ArgType & REGISTER_TYPE) != 0) {
		doVmPopReg(code, disasm, getSingleRegister(disasm->Argument1.ArgType));
	}

	return true;
}

bool VirtualizeCmp(CodeChunk *code, DISASM *disasm) {
	pushCalculateScaledAddress(code, disasm, &disasm->Argument2);
	if ((disasm->Argument2.ArgType & MEMORY_TYPE) != 0) {
		doDeref(code, disasm);
	}

	pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
	if ((disasm->Argument1.ArgType & MEMORY_TYPE) != 0) {
		doDeref(code, disasm);
	}

	doSub(code, disasm);

	doVmPopRemove(code, disasm);

	return true;
}
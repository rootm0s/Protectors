#include "stdafx.h"
#include "settings.h"
#include "CodeChunk.h"
#include "Vm.h"
#include "Utils.h"
#include "registerswap.h"
#include "x86.h"
#include "x86vm.h"


BYTE getMod(BYTE opcode) {
	return (opcode & 0xc0) >> 6;
}

BYTE getReg(BYTE opcode) {
	return (opcode & 0x38) >> 3;
}

BYTE getRm(BYTE opcode) {
	return opcode & 0x07;
}

BYTE getMod(DISASM *disasm) {
	if (disasm->Prefix.Number == 0) {
		return getMod(*(BYTE*)(disasm->EIP + 1));
	}
	else {
		return getMod(*(BYTE*)(disasm->EIP + 2));
	}
}

BYTE getReg(DISASM *disasm) {
	if (disasm->Prefix.Number == 0) {
		return getReg(*(BYTE*)(disasm->EIP + 1));
	}
	else {
		return getReg(*(BYTE*)(disasm->EIP + 2));
	}
}

BYTE getRm(DISASM *disasm) {
	if (disasm->Prefix.Number == 0) {
		return getRm(*(BYTE*)(disasm->EIP + 1));
	}
	else {
		return getRm(*(BYTE*)(disasm->EIP + 2));
	}
}

const char *registerNames[] = { "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI" };
int getSingleRegister(Int32 regs) {
	if (regs & REG0)
		return 0;
	if (regs & REG1)
		return 1;
	if (regs & REG2)
		return 2;
	if (regs & REG3)
		return 3;
	if (regs & REG4)
		return 4;
	if (regs & REG5)
		return 5;
	if (regs & REG6)
		return 6;
	if (regs & REG7)
		return 7;
	return -1;
}

void doVmPopRemove(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::VmPopRemove, g_InstructionNames[VmFile::VmPopRemove]), disasm);
}

void doVmReturn(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::VmReturn, g_InstructionNames[VmFile::VmReturn]), disasm);
}

void doRebase(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::VmRebase, g_InstructionNames[VmFile::VmRebase]), disasm);
}

void doNop(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::Nop, g_InstructionNames[VmFile::Nop]), disasm);
}

// samma som popmem fast adressen finns på stacken och inte kodad i instruktionen
void doMove(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::Move, g_InstructionNames[VmFile::Move]), disasm);
}

void doDeref(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::Deref, g_InstructionNames[VmFile::Deref]), disasm);
}

void doNand(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::Nand, g_InstructionNames[VmFile::Nand]), disasm);
}

// Stack operations
void doRepush(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::VmRepush, g_InstructionNames[VmFile::VmRepush]), disasm);
}

void doNativeDispatch(CodeChunk *code, DISASM *disasm, BYTE *instr, DWORD length) {
	code->addInstruction(VmInstruction(VmFile::VmNativeDispatch, "native", instr, length), disasm);
}

void doNativeHandler(CodeChunk *code, DISASM *disasm, BYTE *instr, DWORD length) {
	BYTE buf[32];
	// pad with NOPs to align to 4 bytes
	DWORD alignedLength = Utils::roundUpMultiple(length, 4);

	buf[0] = (BYTE)alignedLength;	// first encode length
	memcpy(&buf[1], instr, length);	// then the native instruction

									// if length differed, add the alignment NOPs
	if (length < alignedLength) {
		memset(&buf[length + 1], 0x90, alignedLength - length);
	}

	code->addInstruction(VmInstruction(VmFile::VmNativeDispatch, "native", buf, 1 + alignedLength), disasm);
}

void doVmExit(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::VmExit, g_InstructionNames[VmFile::VmExit]), disasm);
}

void _doVmPush(CodeChunk *code, DISASM *disasm, DWORD value) {
	code->addInstruction(VmInstruction(VmFile::VmPush, g_InstructionNames[VmFile::VmPush], value), disasm);
}

void doAdd(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::Add, g_InstructionNames[VmFile::Add]), disasm);
}

void doSub(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::Sub, g_InstructionNames[VmFile::Sub]), disasm);
}

void doVmAdd(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::VmAdd, g_InstructionNames[VmFile::VmAdd]), disasm);
}

void doVmSub(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::VmSub, g_InstructionNames[VmFile::VmSub]), disasm);
}

void doMul(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::Mul, g_InstructionNames[VmFile::Mul]), disasm);
}

void doVmRebase(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::VmRebase, g_InstructionNames[VmFile::VmRebase]), disasm);
}

// Constant obfuscation
void doObfVmPush(CodeChunk *code, DISASM *disasm, DWORD value) {
	
	if (value == 0) { 
		unsigned int val = rand();
		_doVmPush(code, disasm, value);
		return;
	}

	unsigned int d = rand() % value + 1;
	unsigned int q = value / d;
	unsigned int r = value - d*q;

	if (d == 1 || q == 1) {
		if (r == 0) {
			_doVmPush(code, disasm, value);
			return;
		}

		// add(d, r)
		_doVmPush(code, disasm, d);
		_doVmPush(code, disasm, r);
		doVmAdd(code, disasm);
	}
	else if (r == 0) {
		// mul(d, q)
		_doVmPush(code, disasm, d);
		_doVmPush(code, disasm, q);
		doMul(code, disasm);
	}
	else {
		// add(mul(d, q), r)
		_doVmPush(code, disasm, d);
		_doVmPush(code, disasm, q);
		doMul(code, disasm);
		_doVmPush(code, disasm, r);
		doVmAdd(code, disasm);
	}
}

void doVmPush(CodeChunk *code, DISASM *disasm, DWORD value) {
	void (*pfnPush)(CodeChunk *code, DISASM *disasm, DWORD value) = NULL;
	if (g_Settings.unfoldConstants) {
		pfnPush = &doObfVmPush;
	}
	else {
		pfnPush = &_doVmPush;
	}

	pfnPush(code, disasm, value);
}

void doPushDword(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::PushDword, g_InstructionNames[VmFile::PushDword]), disasm);
}

void _doVmPushReg(CodeChunk *code, DISASM *disasm, BYTE reg) {
	code->addInstruction(VmInstruction(VmFile::VmPushReg, g_InstructionNames[VmFile::VmPushReg], reg), disasm);
}

void _doVmPopReg(CodeChunk *code, DISASM *disasm, BYTE reg) {
	code->addInstruction(VmInstruction(VmFile::VmPopReg, g_InstructionNames[VmFile::VmPopReg], reg), disasm);
}

void doVmPushReg(CodeChunk *code, DISASM *disasm, BYTE reg) {
	BYTE newReg = getRegisterCode(reg);
	_doVmPushReg(code, disasm, newReg);
}

void doVmPopReg(CodeChunk *code, DISASM *disasm, BYTE reg) {
	BYTE newReg = getRegisterCode(reg);
	_doVmPopReg(code, disasm, newReg);
}

void doPushReg(CodeChunk *code, DISASM *disasm, BYTE reg) {
	BYTE newReg = getRegisterCode(reg);
	code->addInstruction(VmInstruction(VmFile::PushReg, g_InstructionNames[VmFile::PushReg], reg), disasm);
}

void doPopReg(CodeChunk *code, DISASM *disasm, BYTE reg) {
	BYTE newReg = getRegisterCode(reg);
	code->addInstruction(VmInstruction(VmFile::PopReg, g_InstructionNames[VmFile::PopReg], reg), disasm);
}

void doPopMem(CodeChunk *code, DISASM *disasm, DWORD dest) {
	code->addInstruction(VmInstruction(VmFile::PopMem, g_InstructionNames[VmFile::PopMem], dest), disasm, true);
}

void doPushFlag(CodeChunk *code, DISASM *disasm, DWORD flag) {
	code->addInstruction(VmInstruction(VmFile::PushFlag, g_InstructionNames[VmFile::PushFlag], flag), disasm);
}

void doJumpRelCond(CodeChunk *code, DISASM *disasm, DWORD dest) {
	code->addInstruction(VmInstruction(VmFile::JumpRelCond, g_InstructionNames[VmFile::JumpRelCond], dest), disasm, true);
}

void doJumpAbs(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::JumpAbs, g_InstructionNames[VmFile::JumpAbs]), disasm);
}

// only used as a helper by the VM, not to virtualize XOR
void doXor(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::VmXor, g_InstructionNames[VmFile::VmXor]), disasm);
}

void doCallRel(CodeChunk *code, DISASM *disasm) {
	code->addInstruction(VmInstruction(VmFile::CallRel, g_InstructionNames[VmFile::CallRel]), disasm);
}

// calculate a value in scaled address form ([disp+base+index*scale]) and push onto stack
void pushCalculateScaledAddress(CodeChunk *code, DISASM *disasm, ARGTYPE *arg) {
	if ((arg->ArgType & MEMORY_TYPE) != 0) {
		if (arg->Memory.IndexRegister != 0) {
			doVmPushReg(code, disasm, getSingleRegister(arg->Memory.IndexRegister));
			doVmPush(code, disasm, arg->Memory.Scale);
			doMul(code, disasm);
		}
		if (arg->Memory.BaseRegister != 0) {
			doVmPushReg(code, disasm, getSingleRegister(arg->Memory.BaseRegister));
			if (arg->Memory.IndexRegister != 0) {
				doVmAdd(code, disasm);
			}
		}
		if (arg->Memory.Displacement != 0) {
			doVmPush(code, disasm, arg->Memory.Displacement);
			if (arg->Memory.Scale != 0 || arg->Memory.BaseRegister != 0) {
				doVmAdd(code, disasm);
			}
		}
	}
	else if ((arg->ArgType & CONSTANT_TYPE) != 0) {		
		DWORD imm;
		if (arg->ArgSize == 8) {
			imm = (Int8)disasm->Instruction.Immediat;
		}
		else {
			imm = (DWORD)disasm->Instruction.Immediat;
		}
		doVmPush(code, disasm, imm);
	}
	else if ((arg->ArgType & REGISTER_TYPE) != 0) {
		doVmPushReg(code, disasm, getSingleRegister(arg->ArgType));
	}
	else { // presumed to be NO_ARGUMENT
		logger.write(LOG_ERROR, "invalid argument in expected scaled address\n");
	}
}

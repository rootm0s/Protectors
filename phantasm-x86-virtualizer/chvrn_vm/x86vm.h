#pragma once
#include "stdafx.h"
#include "CodeChunk.h"
#include "x86Instruction.h"

void doVmPopRemove(CodeChunk *code, DISASM *disasm);
void doVmReturn(CodeChunk *code, DISASM *disasm);
void doNop(CodeChunk *code, DISASM *disasm);
void doMove(CodeChunk *code, DISASM *disasm);
void doDeref(CodeChunk *code, DISASM *disasm);
void doNand(CodeChunk *code, DISASM *disasm);
void doRepush(CodeChunk *code, DISASM *disasm);
void doNativeHandler(CodeChunk *code, DISASM *disasm, BYTE *instr, DWORD length);
void doVmExit(CodeChunk *code, DISASM *disasm);
void _doVmPush(CodeChunk *code, DISASM *disasm, DWORD value);
void doAdd(CodeChunk *code, DISASM *disasm);
void doSub(CodeChunk *code, DISASM *disasm);
void doMul(CodeChunk *code, DISASM *disasm);
void doVmRebase(CodeChunk *code, DISASM *disasm);
void doVmPush(CodeChunk *code, DISASM *disasm, DWORD value);
void doPushDword(CodeChunk *code, DISASM *disasm);
void _doVmPushReg(CodeChunk *code, DISASM *disasm, BYTE reg);
void _doVmPopReg(CodeChunk *code, DISASM *disasm, BYTE reg);
void doVmPushReg(CodeChunk *code, DISASM *disasm, BYTE reg);
void doVmPopReg(CodeChunk *code, DISASM *disasm, BYTE reg);
void doPushReg(CodeChunk *code, DISASM *disasm, BYTE reg);
void doPopReg(CodeChunk *code, DISASM *disasm, BYTE reg);
void doPopMem(CodeChunk *code, DISASM *disasm, DWORD dest);
void doPushFlag(CodeChunk *code, DISASM *disasm, DWORD flag);
void doJumpRelCond(CodeChunk *code, DISASM *disasm, DWORD dest);
void doJumpAbs(CodeChunk *code, DISASM *disasm);
void doXor(CodeChunk *code, DISASM *disasm);	
void doCallRel(CodeChunk *code, DISASM *disasm);

// Helper functions
BYTE getMod(BYTE opcode);
BYTE getReg(BYTE opcode);
BYTE getRm(BYTE opcode);
BYTE getMod(DISASM *disasm);
BYTE getReg(DISASM *disasm);
BYTE getRm(DISASM *disasm);

int getSingleRegister(Int32 regs);
void pushCalculateScaledAddress(CodeChunk *code, DISASM *disasm, ARGTYPE *arg);

namespace Flags {
	enum {
		CF = 1 << 0,
		Reserved1 = 1 << 1,
		PF = 1 << 2,
		Reserved2 = 1 << 3,
		AF = 1 << 4,
		Reserved3 = 1 << 5,
		ZF = 1 << 6,
		SF = 1 << 7,
		TF = 1 << 8,
		IF = 1 << 9,
		DF = 1 << 10,
		OF = 1 << 11,
		IOPL = 1 << 12,
		IOPL2 = 1 << 13,
		NT = 1 << 14,
		Reserved4 = 1 << 15,
	};
};

extern const char *registerNames[];
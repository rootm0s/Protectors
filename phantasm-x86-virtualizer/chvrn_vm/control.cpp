/*
	Control flow instructions
	jmp, call, retn
*/
#include "stdafx.h"
#include "x86vm.h"

bool VirtualizeNop(CodeChunk *code, DISASM *disasm) {
	doNop(code, disasm);
	return false;
}

bool VirtualizeRetn(CodeChunk *code, DISASM *disasm) {
	if (disasm->Instruction.Opcode == 0xc3) {	// retn with no size
		doVmPush(code, disasm, 0);
	}
	else if (disasm->Instruction.Opcode == 0xc2) {	// retn with size
		DWORD n = MAKELONG(*(WORD*)(disasm->EIP + 1),0);
		doVmPush(code, disasm, n);
	}
	else {
		return false;
	}

	doVmReturn(code, disasm);
	return true;
}

bool VirtualizeCallRel(CodeChunk *code, DISASM *disasm) {

	BYTE instr[] = 
	{
		0xc7, 0x44, 0x24, 0xfc, 0x00, 0x00, 0x00, 0x00,		// mov dword [esp-4], 0x00000000
		0xff, 0x54, 0x24, 0xfc,								// jmp dword [esp-4]
	};

	*(DWORD*)&instr[4] = disasm->Instruction.AddrValue;

	doNativeHandler(code, disasm, instr, sizeof(instr));

	return true;
}

bool VirtualizeFf(CodeChunk *code, DISASM *disasm) {
	// Reg/opcode byte: 0 = INC, 1 = DEC, 2 = CALL, 3 = CALLF, 4 = JMP, 5 = JMPF, 6 = PUSH
	switch (getReg(disasm)) {
	case 0:
		//logger.write(LOG_MSG, "INC -> native\n");
		return false;
	case 1:
		//logger.write(LOG_MSG, "DEC -> native\n");
		return false;
	case 2:
		// TODO
		// Call into virtualized code vs. call into native
		return false;
	case 3:
		//logger.write(LOG_MSG, "CALLF -> native\n");
		return false;
	case 4: // JMP
		pushCalculateScaledAddress(code, disasm, &disasm->Argument1);
		doDeref(code, disasm);
		doJumpAbs(code, disasm);
		return true;
	case 5: // JMPF
		//logger.write(LOG_MSG, "JMPF -> native\n");
		return false;
	case 6: // PUSH
		pushCalculateScaledAddress(code, disasm, &disasm->Argument2);
		doDeref(code, disasm);
		doPushDword(code, disasm);
		return true;
	default:
		logger.write(LOG_ERROR, "Fix this - unimplemented ff case\n");
		return false;
	}
	return false;
}

bool VirtualizeJmpRel(CodeChunk *code, DISASM *disasm) {
	doVmPush(code, disasm, 1);
	doJumpRelCond(code, disasm, disasm->Instruction.AddrValue);
	return true;
}

bool VirtualizeJcc(CodeChunk *code, DISASM *disasm) {
	// The older beaengine doesn't set the flags correctly for these, so use opcodes instead

	switch (disasm->Instruction.Opcode) {
	case 0x70:	// JO,	OF = 1
	case 0x0f80:
		doPushFlag(code, disasm, Flags::OF);
		break;
	case 0x71:	// JNO,	OF = 0
	case 0x0f81:
		doPushFlag(code, disasm, Flags::OF);
		doVmPush(code, disasm, 1);
		doXor(code, disasm);
		break;
	case 0x72:	// JB/JNAE/JC, CF = 1 
	case 0x0f82:
		doPushFlag(code, disasm, Flags::CF);
		break;
	case 0x73:	// JNB/JNAE/JC, CF = 0
	case 0x0f83:
		doPushFlag(code, disasm, Flags::CF);
		doVmPush(code, disasm, 1);
		doXor(code, disasm);
		break;
	case 0x74:	// JE/JZ, ZF = 1
	case 0x0f84:
		doPushFlag(code, disasm, Flags::ZF);
		break;
	case 0x75:	// JNE/JNZ, ZF = 0
	case 0x0f85:
		doPushFlag(code, disasm, Flags::ZF);
		doVmPush(code, disasm, Flags::ZF);
		doXor(code, disasm);
		break;
	case 0x76:	// JBE/JNA, CF = 1 or ZF = 1
	case 0x0f86:
		doPushFlag(code, disasm, Flags::CF);
		doPushFlag(code, disasm, Flags::ZF);
		//doOr(code, disasm); //TODO
		break;
	case 0x77:	// JA/JNBE, CF = 0 and ZF = 0
	case 0x0f87:
		doPushFlag(code, disasm, Flags::CF);
		doVmPush(code, disasm, 1);
		doXor(code, disasm);
		doPushFlag(code, disasm, Flags::ZF);
		doVmPush(code, disasm, 1);
		doXor(code, disasm);
		//doAnd(code, disasm); //TODO
		break;
	case 0x78:	// JS, SF = 1
	case 0x0f88:
		doPushFlag(code, disasm, Flags::SF);
		break;
	case 0x79:	// JNS, SF = 0
	case 0x0f89:
		doPushFlag(code, disasm, Flags::SF);
		doVmPush(code, disasm, 1);
		doXor(code, disasm);
		break;
	case 0x7A:	// JP/JPE, PF = 1
	case 0x0f8A:
		doPushFlag(code, disasm, Flags::PF);
		break;
	case 0x7B:	// JNP/JPO, PF = 0
	case 0x0f8B:
		doPushFlag(code, disasm, Flags::PF);
		doVmPush(code, disasm, 1);
		doXor(code, disasm);
		break;
	case 0x7C:	// JL/JNGE, SF != OF
	case 0x0f8C:
		doPushFlag(code, disasm, Flags::SF);
		doPushFlag(code, disasm, Flags::OF);
		doXor(code, disasm);
		break;
	case 0x7D:	// JGE/JNL, SF = OF
	case 0x0f8D:
		doPushFlag(code, disasm, Flags::SF);
		doPushFlag(code, disasm, Flags::OF);
		doXor(code, disasm);
		doVmPush(code, disasm, 1);
		doXor(code, disasm);
		break;
	case 0x7E:	// JLE/JNG, ZF = 1 or SF != OF
	case 0x0f8E:
		// eh, later. Debug the others first
		logger.write(LOG_ERROR, "Fix this - unimplemented JCC (JLE)\n");
		return false;
	case 0x7F:	// JG/JNLE, ZF = 0 and SF = OF
	case 0x0f8F:
		logger.write(LOG_ERROR, "Fix this - unimplemented JCC (JG)\n");
		// eh, later. Debug the others first
		return false;
	}

	doJumpRelCond(code, disasm, disasm->Instruction.AddrValue);
	return true;
}
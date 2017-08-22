#include "stdafx.h"
#include "settings.h"
#include "Utils.h"
#include "VirtualizerList.h"
#include "x86vm.h"
#include "x86.h"

#include "control.h"
#include "data.h"
#include "logic.h"
#include "stack.h"
#include "registerswap.h"

VirtualizerList virtualizers;

void RegisterVirtualizers() { 

	DWORD movOpcodes[] = 
	{ 
		0xA1, 0x89, 0x8B, 0x8D, 0xA3, 0xB8, 0xB9, 0xBA, 0xBB, 
		0xBC, 0xBD, 0xBE, 0xBF, 0xC7
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeMov, movOpcodes, sizeof(movOpcodes) / sizeof(DWORD));

	DWORD pushPopOpcodes[] = 
	{ 
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
		0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
		0x68, 0x6A
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizePushPopRegister, pushPopOpcodes, sizeof(pushPopOpcodes) / sizeof(DWORD));

	DWORD jmpRelOpcodes[] = {
		0xeb, 0xe9
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeJmpRel, jmpRelOpcodes, sizeof(jmpRelOpcodes) / sizeof(DWORD));

	DWORD jccOpcodes[] = {
		// one byte for short versions
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
		0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
		// two byte for near versions
		0x0f80, 0x0f81, 0x0f82, 0x0f83, 0x0f84, 0x0f85, 0x0f86, 0x0f87, 
		0x0f88, 0x0f89, 0x0f8A, 0x0f8B, 0x0f8C, 0x0f8D, 0x0f8E, 0x0f8F, 
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeJcc, jccOpcodes, sizeof(jccOpcodes) / sizeof(DWORD));

	DWORD retnOpcodes[] = {
		0xc2, 0xc3,
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeRetn, retnOpcodes, sizeof(retnOpcodes) / sizeof(DWORD));

	DWORD ffOpcodes[] = {
		0xff,
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeFf, ffOpcodes, sizeof(ffOpcodes) / sizeof(DWORD));

	DWORD nopOpcodes[] = {
		0x90,
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeNop, nopOpcodes, sizeof(nopOpcodes) / sizeof(DWORD));

	DWORD andOpcodes[] = {
		0x21, 0x23
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeAnd, andOpcodes, sizeof(andOpcodes) / sizeof(DWORD));

	DWORD orOpcodes[] = {
		0x09,
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeOr, orOpcodes, sizeof(orOpcodes) / sizeof(DWORD));

	DWORD xorOpcodes[] = {
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeXor, xorOpcodes, sizeof(xorOpcodes) / sizeof(DWORD));

	// This needs priority over overlapping ones
	DWORD b81Opcodes[] = {
		0x81,
	};
	virtualizers.addVirtualizer((pfnVirtualize)Virtualize81, b81Opcodes, sizeof(b81Opcodes) / sizeof(DWORD));

	DWORD b83Opcodes[] = {
		0x83,
	};
	virtualizers.addVirtualizer((pfnVirtualize)Virtualize83, b83Opcodes, sizeof(b83Opcodes) / sizeof(DWORD));

	DWORD callRelOpcodes[] = {
		0xe8,
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeCallRel, callRelOpcodes, sizeof(callRelOpcodes) / sizeof(DWORD));

	DWORD addOpcodes[] = {
		0x01, 0x03,
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeAdd, addOpcodes, sizeof(addOpcodes) / sizeof(DWORD));

	DWORD subOpcodes[] = {
		0x29,
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeSub, subOpcodes, sizeof(subOpcodes) / sizeof(DWORD));

	DWORD negOpcodes[] = {
		0xf7,
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeNeg, negOpcodes, sizeof(negOpcodes) / sizeof(DWORD));

	DWORD cmpOpcodes[] = {
		0x39, 0x3b,
	};
	virtualizers.addVirtualizer((pfnVirtualize)VirtualizeCmp, cmpOpcodes, sizeof(cmpOpcodes) / sizeof(DWORD));


	/*
	CMP
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d
	*/

	virtualizers.print();
}


DWORD Virtualize(CodeChunk *code, void *addr, DWORD maxBytes) {
	srand(time(0)); // TODO: flytta in i separat funktion, använd WinAPI istället

	DISASM disasm;
	ZeroMemory(&disasm, sizeof(disasm));

	disasm.EIP = (UIntPtr)addr;

	disasm.VirtualAddr = code->getBaseVa();

	// Stats
	unsigned int virtualizedCount = 0;
	unsigned int nativeCount = 0;
	unsigned int totalInstructions = 0;

	unsigned int totalLen = 0;
	while (1) {
		disasm.SecurityBlock = 0;
		int instrLen = Disasm(&disasm);

		if (instrLen == -1) {
			logger.write(LOG_ERROR, "Disassembling failed\n");
			break;
		}

		// temp, break on NOP
		//if (disasm.Instruction.Opcode == 0x90) {
		//	break;
		//}

		
		if (maxBytes != 0 && (totalLen + instrLen > maxBytes)) {
			break;
		}

		totalInstructions++;
		disasm.SecurityBlock = instrLen; // TEMP

		//
		if (g_Settings.displayDisasm) {
			char hex[32];
			Utils::hexStr((BYTE*)disasm.EIP, instrLen, hex, sizeof(hex), 16, ' ');
			logger.write(LOG_MSG, "%08x %s | %s\n", (DWORD)disasm.VirtualAddr, hex, disasm.CompleteInstr);
		}
		// Attempt to virtualize
		if (virtualizers.virtualize(code, &disasm)) {
			virtualizedCount++;
		} else {
			// Create native handler for it
			//logger.write(LOG_MSG, "TEMP Native: [%s]\n", disasm.CompleteInstr);
			doNativeHandler(code, &disasm, (BYTE*)disasm.EIP, instrLen);
			nativeCount++;
		}
		//

		totalLen += instrLen;
		disasm.VirtualAddr += instrLen;
		disasm.EIP += instrLen;
	}

	logger.write(LOG_MSG,"%d bytes disassembled\n", totalLen);
	logger.write(LOG_MSG, "%d of %d instructions were virtualized (~%d%%)\n", virtualizedCount, totalInstructions, (int)(100 * (virtualizedCount / (float)totalInstructions)));

	return totalLen;
}
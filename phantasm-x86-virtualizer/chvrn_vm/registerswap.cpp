#include "stdafx.h"
#include "CodeChunk.h"
#include "x86vm.h"

int regOriginalCodes[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
int regCodes[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

void obfRandomizeRegisterMapping() {
	srand(time(NULL));

	for (unsigned int i = sizeof(regCodes)/sizeof(regCodes[0]) - 1; i > 0; i--) {
		unsigned int j = rand() % (i+1);
		int temp = regCodes[i];
		regCodes[i] = regCodes[j];
		regCodes[j] = temp;
	}
}

BYTE getRegisterCode(int idx) {
	return regCodes[idx];
}

void doObfSwapRegisters(CodeChunk *code, DISASM *disasm) {
	_doVmPushReg(code, disasm, regCodes[0]);
	_doVmPushReg(code, disasm, regCodes[1]);
	_doVmPushReg(code, disasm, regCodes[2]);
	_doVmPushReg(code, disasm, regCodes[3]);
	_doVmPushReg(code, disasm, regCodes[4]);
	_doVmPushReg(code, disasm, regCodes[5]);
	_doVmPushReg(code, disasm, regCodes[6]);
	_doVmPushReg(code, disasm, regCodes[7]);

	_doVmPopReg(code, disasm, 7);
	_doVmPopReg(code, disasm, 6);
	_doVmPopReg(code, disasm, 5);
	_doVmPopReg(code, disasm, 4);
	_doVmPopReg(code, disasm, 3);
	_doVmPopReg(code, disasm, 2);
	_doVmPopReg(code, disasm, 1);
	_doVmPopReg(code, disasm, 0);
}
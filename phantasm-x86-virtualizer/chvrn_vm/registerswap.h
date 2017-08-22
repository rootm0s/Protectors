#include "stdafx.h"
#include "CodeChunk.h"

BYTE getRegisterCode(int idx);
void obfRandomizeRegisterMapping();
void doObfSwapRegisters(CodeChunk *code, DISASM *disasm);	// inserts VM instructions for randomization/reset
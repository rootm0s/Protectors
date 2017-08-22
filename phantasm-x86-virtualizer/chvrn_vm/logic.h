#pragma once
#include "stdafx.h"

bool Virtualize81(CodeChunk *code, DISASM *disasm);
bool Virtualize83(CodeChunk *code, DISASM *disasm);
bool VirtualizeAnd(CodeChunk *code, DISASM *disasm);
bool VirtualizeOr(CodeChunk *code, DISASM *disasm);
bool VirtualizeXor(CodeChunk *code, DISASM *disasm);

bool VirtualizeAdd(CodeChunk *code, DISASM *disasm);
bool VirtualizeSub(CodeChunk *code, DISASM *disasm);
bool VirtualizeNeg(CodeChunk *code, DISASM *disasm);

bool VirtualizeCmp(CodeChunk *code, DISASM *disasm);
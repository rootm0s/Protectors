#pragma once
#include "stdafx.h"
#include "CodeChunk.h"

bool VirtualizeNop(CodeChunk *code, DISASM *disasm);
bool VirtualizeCallRel(CodeChunk *code, DISASM *disasm);
bool VirtualizeRetn(CodeChunk *code, DISASM *disasm);
bool VirtualizeJcc(CodeChunk *code, DISASM *disasm);
bool VirtualizeJmpRel(CodeChunk *code, DISASM *disasm);
bool VirtualizeFf(CodeChunk *code, DISASM *disasm);
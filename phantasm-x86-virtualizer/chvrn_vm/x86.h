#pragma once
#include "stdafx.h"
#include "CodeChunk.h"

void RegisterVirtualizers();
DWORD Virtualize(CodeChunk *code, void *addr, DWORD maxBytes);

extern int g_regCodes[8];
int getRegCode(int reg);
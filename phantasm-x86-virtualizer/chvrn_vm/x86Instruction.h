#pragma once
#include "stdafx.h"


class x86Instruction {
public:
	x86Instruction(DISASM *disasm);
	x86Instruction(DISASM *disasm, DWORD length, bool vaDependent);

	DWORD getOpcode() const;
	DWORD getVirtualAddress() const;
	DWORD getLength() const;
	const char *getString() const;
	bool isVaDependent() const;

private:
	DWORD m_opcode;
	DWORD m_virtualAddress;
	DWORD m_length;
	char m_string[64];
	bool m_vaDependent;
};
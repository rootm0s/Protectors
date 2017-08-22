#include "stdafx.h"
#include "x86Instruction.h"

x86Instruction::x86Instruction(DISASM *disasm) {
	m_opcode = disasm->Instruction.Opcode;
	m_virtualAddress = (DWORD)disasm->VirtualAddr;
	m_length = disasm->SecurityBlock;
	strncpy(m_string, disasm->CompleteInstr, 64);
	m_vaDependent = false;
}

x86Instruction::x86Instruction(DISASM *disasm, DWORD length, bool vaDependent) {
	m_opcode = disasm->Instruction.Opcode;
	m_virtualAddress = (DWORD)disasm->VirtualAddr;
	m_length = length;
	strncpy(m_string, disasm->CompleteInstr, 64);
	m_vaDependent = vaDependent;
}

DWORD x86Instruction::getOpcode() const {
	return m_opcode;
}

DWORD x86Instruction::getVirtualAddress() const {
	return m_virtualAddress;
}

DWORD x86Instruction::getLength() const {
	return m_length;
}

const char *x86Instruction::getString() const {
	return m_string;
}

bool x86Instruction::isVaDependent() const {
	return m_vaDependent;
}
#pragma once
#include "stdafx.h"
#include "VmInstruction.h"
#include "x86Instruction.h"

class CodeChunk {
public:
	CodeChunk(DWORD base);
	CodeChunk(const CodeChunk& code);
	virtual ~CodeChunk();
	VmInstruction *addInstruction(const VmInstruction &instruction, DISASM *disasm, bool vaDependent=false);
	DWORD getBaseVa() const;
	DWORD getSize() const;
	unsigned int getCount() const;
	DWORD getLastVa() const;

	DWORD getNewVa(DWORD va) const;
	bool recalculateRelativeOperands();

	bool finalize();

	const BYTE *createByteBuffer();
	void print() const;

	VmInstruction*& operator[](const int idx);

	void addAbsoluteMapping(DWORD va);
	BYTE *getAddressLookupTable(DWORD *size);
	BYTE *getAddressValuesTable(DWORD *size);

	DWORD getIp() const;
	void setIp(DWORD ip);
	DWORD getInstructionAddress(DWORD va) const;

protected:
	std::vector<VmInstruction*> m_vmInstructions;
	std::vector<x86Instruction*> m_x86Instructions;
	std::vector<DWORD> m_newVas;
	DWORD m_base;
	DWORD m_size;
	BYTE *m_bytes;
	DWORD m_ip;
};
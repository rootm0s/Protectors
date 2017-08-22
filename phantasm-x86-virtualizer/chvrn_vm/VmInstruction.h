#pragma once
#include "stdafx.h"

class VmInstruction {
public:
	VmInstruction(BYTE opcode, const char *mnemonic);
	VmInstruction(BYTE opcode, const char *mnemonic, BYTE operand);
	VmInstruction(BYTE opcode, const char *mnemonic, DWORD operand);
	VmInstruction(BYTE opcode, const char *mnemonic, BYTE* operand, DWORD length);

	BYTE getOpcode() const;
	void setOpcode(BYTE opcode);

	BYTE getOperand8() const;
	DWORD getOperand32() const;

	void setOperand8(BYTE operand);
	void setOperand32(DWORD operand);

	void setVirtualAddress(DWORD virtualAddress);
	DWORD getVirtualAddress() const;

	const char *toString() const;
	BYTE *getBytes() const;
	DWORD getLength() const;

	bool isRelocatable() const;
	void setRelocationOffset(DWORD offset);
protected:
	char m_mnemonic[16];
	mutable char m_byteStr[32];
	mutable char m_assemblyStr[32];
	mutable char m_fullInstr[64];
	DWORD m_length;
	BYTE m_bytes[16];
	DWORD m_virtualAddress;

	bool m_isRelocatable;
	DWORD m_relocationOffset;
};
#include "stdafx.h"
#include "VmInstruction.h"
#include "Utils.h"

VmInstruction::VmInstruction(BYTE opcode, const char *mnemonic) {
	m_bytes[0] = opcode;
	m_length = 1;
	strcpy(m_mnemonic, mnemonic);
	m_virtualAddress = 0;
}

VmInstruction::VmInstruction(BYTE opcode, const char *mnemonic, BYTE operand) {
	m_bytes[0] = opcode;
	strcpy(m_mnemonic, mnemonic);
	setOperand8(operand);
	m_virtualAddress = 0;
}

VmInstruction::VmInstruction(BYTE opcode, const char *mnemonic, DWORD operand) {
	m_bytes[0] = opcode;
	strcpy(m_mnemonic, mnemonic);
	setOperand32(operand);
	m_virtualAddress = 0;
}

// Variable length, only used for native dispatch
VmInstruction::VmInstruction(BYTE opcode, const char *mnemonic, BYTE *operand, DWORD length) {
	m_bytes[0] = opcode;
	strcpy(m_mnemonic, mnemonic);
	memcpy(&m_bytes[1], operand, length);
	m_length = sizeof(BYTE)+length;
	m_virtualAddress = 0;
}

BYTE VmInstruction::getOpcode() const {
	return m_bytes[0];
}

void VmInstruction::setOpcode(BYTE opcode) {
	m_bytes[0] = opcode;
}

BYTE VmInstruction::getOperand8() const {
	return m_bytes[1];
}

DWORD VmInstruction::getOperand32() const {
	return (m_length < 4) ? (DWORD)getOperand8() : *(DWORD*)&m_bytes[1];
}

const char *VmInstruction::toString() const {
	// byte str
	Utils::padStr(m_byteStr, Utils::hexStr(m_bytes, m_length, m_byteStr, sizeof(m_byteStr)), 32);

	// assembly string
	sprintf(m_assemblyStr, "%s", m_mnemonic);
	if (m_length == 2) {
		sprintf(m_assemblyStr, "%s %02x", m_mnemonic, m_bytes[1]);
	} else if (m_length == 5) {
		sprintf(m_assemblyStr, "%s %08x", m_mnemonic, *(DWORD*)&m_bytes[1]);
	}

	sprintf(m_fullInstr, "%s | %s", m_byteStr, m_assemblyStr);

	return m_fullInstr;
}

BYTE *VmInstruction::getBytes() const {
	return (BYTE*)&m_bytes;
}

DWORD VmInstruction::getLength() const {
	return m_length;
}

void VmInstruction::setOperand8(BYTE operand) {
	m_bytes[1] = operand;
	m_length = sizeof(BYTE) + sizeof(operand);
}

void VmInstruction::setOperand32(DWORD operand) {
	*(DWORD*)&m_bytes[1] = operand;
	m_length = sizeof(BYTE) + sizeof(operand);
}

void VmInstruction::setVirtualAddress(DWORD virtualAddress) {
	m_virtualAddress = virtualAddress;
}

DWORD VmInstruction::getVirtualAddress() const {
	return m_virtualAddress;
}

bool VmInstruction::isRelocatable() const {
	return m_isRelocatable;
}

void VmInstruction::setRelocationOffset(DWORD offset) {
	m_relocationOffset = offset;
	m_isRelocatable = true;
}
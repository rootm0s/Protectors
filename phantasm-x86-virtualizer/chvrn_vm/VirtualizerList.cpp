#include "stdafx.h"
#include "VirtualizerList.h"

VirtualizerList::VirtualizerList() {
	m_virtualizers = new pfnVirtualize[0xff*2];
	ZeroMemory(m_virtualizers, 0xff*2*sizeof(pfnVirtualize));
	m_count = 0;
}

VirtualizerList::~VirtualizerList() {
	delete[] m_virtualizers;
}

bool VirtualizerList::addVirtualizer(pfnVirtualize virtualizer, DWORD opcode) {

	BYTE hiByte = (opcode & 0xff00) >> 8;
	BYTE loByte = opcode & 0xff;

	pfnVirtualize *location;
	if (hiByte == 0x0f) {
		location = &m_virtualizers[0xff+loByte];
	} else {
		location = &m_virtualizers[loByte];
	}
	bool duplicate = false;
	if (*location != 0) {
		duplicate = true;
		logger.write(LOG_WARN, "A virtualizer for opcode %x is already registered\n", opcode);
	}
	*location = virtualizer;
	return duplicate;
}

bool VirtualizerList::addVirtualizer(pfnVirtualize virtualizer, DWORD *opcodes, DWORD count) {
	bool duplicate = false;
	for (unsigned int i = 0; i < count; i++) {
		if (addVirtualizer(virtualizer, opcodes[i])) {
			duplicate = true;
		}
	}
	return duplicate;
}

bool VirtualizerList::virtualize(CodeChunk *code, DISASM *disasm) const {
	DWORD opcode = disasm->Instruction.Opcode;

	BYTE hiByte = (opcode & 0xff00) >> 8;
	BYTE loByte = opcode & 0xff;

	pfnVirtualize virtualizer;
	if (hiByte == 0x0f) {
		virtualizer = m_virtualizers[0xff+loByte];
	} else {
		virtualizer = m_virtualizers[loByte];
	}

	if (!virtualizer) {
		return false;
	} else {
		return virtualizer(code, disasm);
	}
}

void VirtualizerList::print() const {
	logger.write(LOG_MSG, "Supported opcodes:\n");
	for (unsigned int i = 0; i < 0xff*2; i++) {
		if (!m_virtualizers[i]) {
			continue;
		}

		if (i < 0xff) {
			logger.write(LOG_MSG, "%02x ", i);
		} else {
			logger.write(LOG_MSG, "0f%02x ", i);
		}
	}
	logger.write(LOG_MSG, "\n");
}
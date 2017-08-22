#include "stdafx.h"
#include "CodeChunk.h"

CodeChunk::CodeChunk(DWORD base)
	: m_base(base), m_ip(0), m_size(0), m_bytes(nullptr) {
}

CodeChunk::CodeChunk(const CodeChunk& code)
	: m_base(code.m_base), m_ip(code.m_ip), m_size(code.m_size) {
	m_vmInstructions = std::vector<VmInstruction*>(code.m_vmInstructions.size());
	for (std::size_t i = 0; i < code.m_vmInstructions.size(); i++) {
		m_vmInstructions[i] = new VmInstruction(*code.m_vmInstructions[i]);
	}
	m_x86Instructions = std::vector<x86Instruction*>(code.m_x86Instructions.size());
	for (std::size_t i = 0; i < code.m_x86Instructions.size(); i++) {
		m_x86Instructions[i] = code.m_x86Instructions[i] ? new x86Instruction(*code.m_x86Instructions[i]) : nullptr;
	}
	m_bytes = new BYTE[code.m_size];
	memcpy(m_bytes, code.m_bytes, code.m_size);
}

CodeChunk::~CodeChunk() {
	if (m_bytes) {
		delete[] m_bytes;
		m_bytes = nullptr;
	}
	for (auto it : m_vmInstructions) {
		delete it;
	}
	for (auto it : m_x86Instructions) {
		if (it) {
			delete it;
		}
	}
}

VmInstruction *CodeChunk::addInstruction(const VmInstruction& instruction, DISASM *disasm, bool vaDependent) {
	DWORD newVa = getLastVa();
	VmInstruction *newInstr = new VmInstruction(instruction);
	
	newInstr->setVirtualAddress(newVa);
	m_vmInstructions.push_back(newInstr);
	m_size += newInstr->getLength();

	x86Instruction *instr = nullptr;
	if (disasm) {
		instr = new x86Instruction(disasm, disasm->SecurityBlock, vaDependent);
	}
	
	m_x86Instructions.push_back(instr);

	return newInstr;
}

DWORD CodeChunk::getBaseVa() const {
	return m_base;
}

DWORD CodeChunk::getSize() const {
	return m_size;
}

unsigned int CodeChunk::getCount() const {
	return m_vmInstructions.size();
}

DWORD CodeChunk::getLastVa() const {
	return m_base + m_size;
}

DWORD CodeChunk::getNewVa(DWORD va) const {
	for (unsigned int i = 0; i < m_x86Instructions.size(); i++) {
		if (!m_x86Instructions[i]) {
			continue;
		}
		if (m_x86Instructions[i]->getVirtualAddress() == va) {
			return m_vmInstructions[i]->getVirtualAddress();
		}
	}
	return -1;
}
bool CodeChunk::recalculateRelativeOperands() {
	for (unsigned int i = 0; i < m_vmInstructions.size(); i++) {
		// operand needs recalculating
		if (!m_x86Instructions[i]) {	// native instruction has no coupling with a virtual one
			continue;
		}
		if (m_x86Instructions[i]->isVaDependent()) {
			if (m_vmInstructions[i]->getLength() == 5) {
				DWORD oldDest = m_vmInstructions[i]->getOperand32();
				DWORD newDest = getNewVa(oldDest);

				if (newDest == -1) {
					logger.write(LOG_ERROR, "Could not look up VA: %x. Jump outside virtualized chunk?\n", newDest);

					newDest = getBaseVa() + getSize();
					logger.write(LOG_MSG, "-> Setting jmp to end of chunk, %x\n", newDest);
				}

				DWORD newDistance = newDest-(m_vmInstructions[i]->getVirtualAddress() + m_vmInstructions[i]->getLength());
				m_vmInstructions[i]->setOperand32(newDistance);

			} else {
				logger.write(LOG_ERROR, "Non-DWORD lengths not supported yet\n");
				return false;
			}
		}
	}
	return true;
}

bool CodeChunk::finalize() {
	return false;
}

const BYTE *CodeChunk::createByteBuffer() {
	if (m_bytes) {
		delete[] m_bytes;
	}
	m_bytes = new BYTE[m_size];

	DWORD numWritten = 0;
	for (unsigned int i = 0; i < m_vmInstructions.size(); i++) {
		memcpy(m_bytes+numWritten, m_vmInstructions[i]->getBytes(), m_vmInstructions[i]->getLength());
		numWritten += m_vmInstructions[i]->getLength();
	}

	return m_bytes;
}

void CodeChunk::print() const {
	for (unsigned int i = 0; i < m_vmInstructions.size(); i++) {
		logger.write(LOG_MSG, "%08x->%08x %s\n", m_x86Instructions[i] != 0 ? m_x86Instructions[i]->getVirtualAddress() : 0, m_vmInstructions[i]->getVirtualAddress(), m_vmInstructions[i]->toString());
	}
}

VmInstruction*& CodeChunk::operator[](const int idx) {
	return m_vmInstructions[idx];
}

DWORD CodeChunk::getIp() const {
	return m_ip;
}

void CodeChunk::setIp(DWORD ip) {
	m_ip = ip;
}

DWORD CodeChunk::getInstructionAddress(DWORD va) const {
	DWORD newVa = getNewVa(va);
	if (newVa == -1) {
		return (DWORD)-1;
	}

	return newVa+getIp();
}
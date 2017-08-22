#include "stdafx.h"
#include "Vm.h"
#include "patterns.h"

#ifndef STRIPPED_BUILD
const char *g_InstructionNames[] = {
	"VmNativeDispatch",
	"VmExit",
	"VmPush",
	"VmRepush",
	"VmPushReg",
	"PushDword",
	"VmPopReg",
	"PushFlag",
	"Move",
	"Nand",
	"JumpRel",
	"JumpAbs",
	"JumpRelCond",
	"PopMem",
	"Deref",
	"Add",
	"Sub",
	"Mul",
	"Xor",
	"CallRel",
	"Nop",
	"VmRebase",
	"VmReturn",
	"VmPopRemove",
	"VmAdd",
	"VmSub",
	"PushReg",
	"PopReg",
};
#else
const char *g_InstructionNames[] = { // lel
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
};
#endif


VmFile::VmFile(MappedPeFile *container)
	: m_container(container), m_header(nullptr), m_opcodes(nullptr), m_indices(nullptr) {

}

VmFile::~VmFile() {
	if (m_opcodes) {
		delete[] m_opcodes;
	}
	if (m_indices) {
		delete[] m_indices;
	}
}

bool VmFile::findVm(BYTE *data, DWORD size) {
	static DWORD pattern[] = { VM_SIGNATURE_1, VM_SIGNATURE_2, VM_SIGNATURE_3 };
	DWORD vmOffset = findBytePattern(data, size, (BYTE*)pattern, sizeof(pattern));
	if (vmOffset == -1) {
		logger.write(LOG_ERROR, "VM not found inside target\n");
		return false;
	}

	m_header = (VmHeader*)(data + vmOffset);

	return true;
}

bool VmFile::load() {
	PIMAGE_SECTION_HEADER pData = m_container->getSectionFromName(".data");

	BYTE *data = m_container->getFileBuffer() + pData->PointerToRawData;
	DWORD dataSize = pData->SizeOfRawData;
	
	if (!findVm(data, dataSize)) {
		return false;
	}

	// not really necessary unless the implementation of findVm is wrong
	if ((m_header->signature1 != VM_SIGNATURE_1)
		|| (m_header->signature2 != VM_SIGNATURE_2)
		|| (m_header->signature3 != VM_SIGNATURE_3)) {
		logger.write(LOG_ERROR, "VM signature mismatch\n");
		return false;
	}

	m_opcodes = new DWORD[m_header->vmHandlerCount];
	m_indices = new unsigned int[m_header->vmHandlerCount];
	for (unsigned int i = 0; i < m_header->vmHandlerCount; i++) {
		m_opcodes[i] = i;
		m_indices[i] = i;
	}

	return true;
}

VmFile::VmHeader *VmFile::getVmHeader() {
	return m_header;
}

DWORD VmFile::getVirtualAddress() const
{
	return m_container->ptr2va(m_header);
}

BYTE *VmFile::getHandlers() {
	return (BYTE*)m_container->va2ptr(m_header->vmHandlersVa);
}

DWORD VmFile::getHandlerCount() {
	return m_header->vmHandlerCount;
}

BYTE *VmFile::getVmCodeBytes() {
	return  (BYTE*)m_container->va2ptr(m_header->vmBaseVa);
}

DWORD VmFile::getVmSize() const {
	return m_header->vmCodeSize;
}

typedef int (__cdecl *pfnName)(int);

void VmFile::rebase(DWORD newBase) {
	return; // not needed for lib version, for non-lib only relocations are imports
}

void VmFile::randomizeOpcodes() {
	DWORD *handlers = (DWORD*)(m_container->getFileBuffer() + m_container->va2FileOffset(m_header->vmHandlersVa));

	srand(time(NULL));
	for (unsigned int i = m_header->vmHandlerCount - 1; i > 0; i--) {
		unsigned int j = rand() % (i + 1);

		DWORD temp = m_opcodes[i];
		m_opcodes[i] = m_opcodes[j];
		m_opcodes[j] = temp;

		temp = handlers[i];
		handlers[i] = handlers[j];
		handlers[j] = temp;
	}
}

void VmFile::generateOpcodes(CodeChunk *code) {
	DWORD *handlers = (DWORD*)(m_container->getFileBuffer() + m_container->va2FileOffset(m_header->vmHandlersVa));

	logger.write(LOG_MSG, "Generating opcodes and handler table...\n");

	// Update opcodes for the code chunk
	for (unsigned int i = 0; i < code->getCount(); i++) {
		
		BYTE oldOpcode = (*code)[i]->getOpcode();
		BYTE newOpcode;
		for (unsigned int j = 0; j < m_header->vmHandlerCount; j++) {
			if (m_opcodes[j] == oldOpcode) {
				newOpcode = j;
			}
		}
		(*code)[i]->setOpcode(newOpcode);
	}
}

DWORD VmFile::getOpcode(DWORD instructionId) {
	return m_opcodes[instructionId];
}

void VmFile::patchBeginProtect() {
	(getVmCodeBytes() + m_header->vmBeginProtectOffset)[0] = 0x90;
	(getVmCodeBytes() + m_header->vmBeginProtectOffset)[1] = 0x90;
	(getVmCodeBytes() + m_header->vmBeginProtectOffset)[2] = 0x90;
}
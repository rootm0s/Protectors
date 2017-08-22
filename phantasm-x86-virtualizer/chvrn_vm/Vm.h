#pragma once
#include "stdafx.h"
#include "MappedPeFile.h"
#include "CodeChunk.h"

#define VM_SIGNATURE_1 0xb0fa3d74 
#define VM_SIGNATURE_2 0xa53bcf98 
#define VM_SIGNATURE_3 0xee02930e 

extern const char *g_InstructionNames[];

class VmFile {
public:
	// NOTE: must be synchronized with the VM header in the VM file
#pragma pack(1)
	struct VmHeader {
		DWORD signature1;
		DWORD signature2;
		DWORD signature3;
		DWORD majorVersion;
		DWORD minorVersion;
		//
		DWORD moduleBase;
		DWORD addressLookupTable;
		DWORD addressValueTable;
		//
		DWORD vmBaseVa;
		DWORD vmCodeSize;
		DWORD vmInitOffset;
		DWORD vmFetchDecodeOffset;
		DWORD vmReenterOffset;
		DWORD vmExitOffset;
		DWORD vmVirtualAllocPatchOffset1;
		DWORD vmVirtualAllocPatchOffset2;
		DWORD vmVirtualFreePatchOffset1;
		DWORD vmVirtualFreePatchOffset2;
		DWORD vmHandlerPatchOffset;
		DWORD vmReenterPatchOffset;
		DWORD vmExitPatchOffset;
		DWORD vmExitNativeOffset;
		DWORD vmVaTablePatchOffset;
		DWORD vmVaTableValuesPatchOffset;
		DWORD vmHandlersVa;
		DWORD vmHandlerCount;
		DWORD vmBeginProtectOffset;
		DWORD vmEndProtectOffset;
		//
	};
#pragma pack()
	// must be synchronized with the handler table in the VM file
	enum OriginalOpcodes {
		VmNativeDispatch = 0,
		VmExit,
		VmPush,
		VmRepush,
		VmPushReg,
		PushDword,
		VmPopReg,
		PushFlag,
		Move,
		Nand,
		JumpRel,
		JumpAbs,
		JumpRelCond,
		PopMem,
		Deref,
		Add,
		Sub,
		Mul,
		VmXor,
		CallRel,
		Nop,
		VmRebase,
		VmReturn,
		VmPopRemove,
		VmAdd,
		VmSub,
		PushReg,
		PopReg,
	};

	bool load();
	VmFile(MappedPeFile *container);
	~VmFile();

	VmHeader *getVmHeader();

	DWORD getVirtualAddress() const;

	BYTE *getHandlers();
	DWORD getHandlerCount();
	BYTE *getVmCodeBytes();
	DWORD getVmSize() const;

	void rebase(DWORD newBase);
	
	void randomizeOpcodes();
	void generateOpcodes(CodeChunk *code);
	DWORD getOpcode(DWORD instructionId);

	void patchBeginProtect();
private:
	bool findVm(BYTE *data, DWORD size);
	VmHeader *m_header;
	DWORD m_virtualAddress;
	MappedPeFile *m_container;
	std::vector<DWORD> m_nativeHandlerVas;
	DWORD *m_opcodes;
	unsigned int *m_indices;
};
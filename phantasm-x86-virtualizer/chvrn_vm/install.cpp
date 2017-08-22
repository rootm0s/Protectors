#include "stdafx.h"
#include "MappedPeFile.h"
#include "Vm.h"
#include "x86.h"
#include "x86vm.h"
#include "patterns.h"
#include "registerswap.h"
#include "settings.h"
#include "markers.h"

bool createVaLookupTable(MappedPeFile *target, VmFile *vm, std::vector<CodeChunk> *chunks, std::vector<DWORD> vas) {
	logger.write(LOG_MSG, "Creating VA table\n");
	std::vector<std::pair<DWORD, DWORD>> vaTable;
	for (unsigned int j = 0; j < vas.size(); j++) {
		DWORD newVa = -1;
		for (unsigned int k = 0; k < chunks->size(); k++) {
			newVa = (*chunks)[k].getInstructionAddress(vas[j]);
			if (newVa != 1) {
				vaTable.push_back(std::make_pair(vas[j]-target->getNtHeaders()->OptionalHeader.ImageBase, newVa-target->getNtHeaders()->OptionalHeader.ImageBase));
				break;
			}
		}

		if (!newVa) {
			logger.write(LOG_WARN, "VA %08x was not found, skipping\n", vas[j]);
		}
	}

	// Create the table
	logger.write(LOG_MSG, "Address lookup table has %d entries\n", vaTable.size());
	if (vaTable.size() > 0) {
		DWORD vaAddressLookupTable = target->getCurrentVa();
		for (unsigned int i = 0; i < vaTable.size(); i++) {
			target->addToLastSection((BYTE*)&vaTable[i].first, sizeof(DWORD));
		}
		DWORD vaAddressValuesTable = target->getCurrentVa();
		for (unsigned int i = 0; i < vaTable.size(); i++) {
			target->addToLastSection((BYTE*)&vaTable[i].second, sizeof(DWORD));
		}

		for (unsigned int i = 0; i < vaTable.size(); i++) {
			logger.write(LOG_MSG, "%08x | %08x \n", vaTable[i].first, vaTable[i].second);
		}

		vm->getVmHeader()->addressLookupTable = vaAddressLookupTable;
		vm->getVmHeader()->addressValueTable = vaAddressValuesTable;
		logger.write(LOG_MSG, "LookupTable: %08x\nValueTable: %08x\n", vaAddressLookupTable, vaAddressValuesTable);
	}
	else {
		logger.write(LOG_MSG, "Lookup table empty, not adding/patching\n");
	}
	return true;
}

std::vector<CodeChunk> VirtualizeRegions(MappedPeFile *target, VmFile *vm, std::vector<std::pair<DWORD, DWORD>> regions) {
	std::vector<CodeChunk> chunks;

	for (unsigned int i = 0; i < regions.size(); i++) {
		logger.write(LOG_MSG, "Analyzing code at %08x (%d bytes)\n", regions[i].first, regions[i].second);

		BYTE *buf = target->getFileBuffer() + target->va2FileOffset(regions[i].first);

		CodeChunk code(regions[i].first);

		if (g_Settings.randomizeRegisters) {
			obfRandomizeRegisterMapping();
			doObfSwapRegisters(&code, 0);
		}

		DWORD originalLen = Virtualize(&code, buf, regions[i].second);

		logger.write(LOG_MSG, "Recalculating relative operands...\n");
		if (!code.recalculateRelativeOperands()) {
			logger.write(LOG_WARN, "Recalculating relative operands failed\n");
			break;
		}

		// Restore registers
		if (g_Settings.randomizeRegisters) {
			doObfSwapRegisters(&code, 0);
		}

		// Add VmExit to the end of the chunk
		// Must be done AFTER recalculateRelativeOperands
		doVmPush(&code, 0, (DWORD)code.getBaseVa() + 5 + 5); // +5 to skip the push, +5 to skip the jmp
		doVmExit(&code, 0);

		// Apply opcode mapping
		vm->generateOpcodes(&code);

#ifndef STRIPPED_BUILD
		logger.write(LOG_MSG, "Result:\n");
		code.print();
#endif
		logger.write(LOG_MSG,  "-> %d VM instructions (%d bytes)\n", code.getCount(), code.getSize());
		logger.write(LOG_MSG, "Size overhead: %d%%\n", (unsigned int)(100*(originalLen + code.getSize()) / (float)originalLen));
		logger.write(LOG_MSG, "Appending VM bytecode to %08x (offset %d)\n", target->getCurrentVa(), target->va2FileOffset(target->getCurrentVa()));
		
		DWORD vmIp = target->addToLastSection(code.createByteBuffer(), code.getSize());

		// TEMP
		vmIp -= target->getNtHeaders()->OptionalHeader.ImageBase;

		code.setIp(vmIp);
		chunks.push_back(code);

		// NOP out all bytes
		logger.write(LOG_MSG, "Removing original code\n");
		for (unsigned int i = 0; i < originalLen; i++) {
			buf[i] = 0x90;
		}
	}
	return chunks;
}

bool InstallVmLib(MappedPeFile *target, VmFile *vm, std::vector<DWORD> fixups) {
	logger.write(LOG_MSG, "Registering virtualizers\n");
	RegisterVirtualizers();

	BYTE *code = target->getFileBuffer() + target->getSectionFromName(".text")->PointerToRawData;
	DWORD size = target->getSectionFromName(".text")->SizeOfRawData;

	std::vector<std::pair<DWORD, DWORD>> vas = scanForMarkers(code, size);

	if (vas.size() == 0) {
		logger.write(LOG_ERROR, "No markers found. Exiting\n");
		return false;
	}

	logger.write(LOG_MSG, "Detected markers inside target:\n");
	for (unsigned int i = 0; i < vas.size(); i++) {
		vas[i].first += target->getNtHeaders()->OptionalHeader.ImageBase + target->getSectionFromName(".text")->VirtualAddress;
		logger.write(LOG_MSG, "%08x -> %08x (%d bytes)\n", vas[i].first, vas[i].first + vas[i].second, vas[i].second);
	}

	// Patch BeginProtect stub
	logger.write(LOG_MSG, "Patching BeginProtect...\n");
	vm->patchBeginProtect();

	logger.write(LOG_MSG, "Generating opcodes\n");
	if (g_Settings.randomizeOpcodes) {
		vm->randomizeOpcodes();
	}

	std::vector<CodeChunk> chunks;
	chunks = VirtualizeRegions(target, vm, vas);

	logger.write(LOG_MSG, "Patching markers\n");
	for (unsigned int i = 0; i < vas.size(); i++) {
		// Patch in correct VM IP
		target->patchVa(vas[i].first - 10 + 1, chunks[i].getIp());

		// NOP out the end marker 
		for (unsigned int j = 0; j < 10; j++) {
			(target->getFileBuffer() + target->va2FileOffset(vas[i].first + vas[i].second))[j] = 0x90;
		}
	}

	createVaLookupTable(target, vm, &chunks, fixups);

	return true;
}

bool InstallVmStandalone(MappedPeFile *target, VmFile *vm, std::vector<std::pair<DWORD,DWORD>> targets, std::vector<DWORD> vas) {
	//	This function is not up to date with the current VM implementation!
	//	(the patch offsets are fewer and need updating)

	// Must be done first (!)
	logger.write(LOG_MSG, "Registering virtualizers\n");
	RegisterVirtualizers();

	logger.write(LOG_MSG, "Installing VM as standalone\n");

	// Make sure the target file has the needed imports to patch the VM
	DWORD vaVirtualAlloc = target->getIatVa("kernel32.dll", "VirtualAlloc");
	DWORD vaVirtualFree = target->getIatVa("kernel32.dll", "VirtualFree");

	if (!vaVirtualAlloc || !vaVirtualFree) {
		logger.write(LOG_ERROR, "Target has missing imports\n");
		logger.write(LOG_MSG, "VirtualAlloc: %08x, VirtualFree: %08x\n", vaVirtualAlloc, vaVirtualFree);
		return false;
	}
	// VM assembling is done: add it to the file
	//target->createSection(".vm", IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ); // TODO: when no space...
	DWORD vmVa = target->addToLastSection(vm->getVmCodeBytes(), vm->getVmSize());
	logger.write(LOG_MSG, "VM added at %08x (%d bytes)\n", vmVa, vm->getVmSize());
	logger.write(LOG_MSG, "Old VM base was: %08x\n", vm->getVmHeader()->vmBaseVa);

	logger.write(LOG_MSG, "Patching API calls\n"); // TODO
	target->patchVa(vmVa + vm->getVmHeader()->vmVirtualAllocPatchOffset1, vaVirtualAlloc);
	target->patchVa(vmVa + vm->getVmHeader()->vmVirtualAllocPatchOffset2, vaVirtualAlloc);

	target->patchVa(vmVa + vm->getVmHeader()->vmVirtualFreePatchOffset1, vaVirtualFree);
	target->patchVa(vmVa + vm->getVmHeader()->vmVirtualFreePatchOffset2, vaVirtualFree);

	printf("Recalculating jumps\n");
	target->patchVa(vmVa + vm->getVmHeader()->vmReenterPatchOffset, vmVa + vm->getVmHeader()->vmReenterOffset);
	target->patchVa(vmVa + vm->getVmHeader()->vmExitPatchOffset, vmVa + vm->getVmHeader()->vmExitNativeOffset);

	// TODO: scrambling måste ske här, inte i generate opcodes
	// vm->getHandlerTable() efter generateOpcodes() skett
	logger.write(LOG_MSG, "Copying handlers\n");
	logger.write(LOG_MSG, "%d handlers found\n", vm->getVmHeader()->vmHandlerCount);
	DWORD vaHandlerTable = target->addToLastSection(vm->getHandlers(), sizeof(DWORD)*vm->getHandlerCount());
	DWORD *handlerTable = (DWORD*)target->va2ptr(vaHandlerTable);

	logger.write(LOG_MSG, "Patching handler VA to: %08x\n", vaHandlerTable);
	*(DWORD*)target->va2ptr(vmVa + vm->getVmHeader()->vmHandlerPatchOffset) = vaHandlerTable;

	logger.write(LOG_MSG, "Rebasing handler table\n");
	for (unsigned int i = 0; i < vm->getVmHeader()->vmHandlerCount; i++) {
		logger.write(LOG_MSG, "%08x->%08x\n", handlerTable[i], handlerTable[i] - vm->getVmHeader()->vmBaseVa + vmVa);
		handlerTable[i] = handlerTable[i] - vm->getVmHeader()->vmBaseVa + vmVa;
	}

	// 4. 
	DWORD vmInitVa = vmVa + vm->getVmHeader()->vmInitOffset;
	logger.write(LOG_MSG, "New VmInit va: %08x\n", vmInitVa);

	std::vector<CodeChunk> chunks;
	chunks = VirtualizeRegions(target, vm, targets);

	logger.write(LOG_MSG, "Patching call stubs\n");
	for (unsigned int i = 0; i < chunks.size(); i++) {
		BYTE *buf = target->getFileBuffer() + target->va2FileOffset(targets[i].first);
		logger.write(LOG_MSG, "Patching bytes...\n");
		unsigned int idx = 0;
		buf[idx] = 0x68;
		*(DWORD*)&buf[idx + 1] = chunks[i].getIp();	// push IP to VM init
		idx += 5;

		buf[idx] = 0xe9;
		*(DWORD*)&buf[idx + 1] = vmInitVa - (targets[i].first + idx) - 5;	// jmp to VM init
		idx += 5;
	}

	createVaLookupTable(target, vm, &chunks, vas);

	return true;
}
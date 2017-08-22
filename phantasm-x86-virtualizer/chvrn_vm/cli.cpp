#include "stdafx.h"
#include "ArgsParser.h"
#include "install.h"
#include "settings.h"
#include "environment.h"

ArgsParser g_Args;

MappedPeFile *g_TargetPe;

bool RunCli(int argc, char** argv) {
	g_Args.parseMainArgs(argc, argv);

	if (!g_Args.getArgCount("input_file")) {
		logger.write(LOG_ERROR, "Missing required argument input_file\n");
		return false;
	}

	if (!g_Args.getArgCount("output_file")) {
		logger.write(LOG_ERROR, "Missing required argument output_file\n");
		return false;
	}

	std::string inputFilePath = g_Args.getArgString("input_file");
	std::string outputFilePath = g_Args.getArgString("output_file");

	LoadDefaultSettings();
	if (g_Args.getArgCount("randomize_registers")) {
		g_Settings.randomizeRegisters = (bool)g_Args.getArgInt("randomize_registers");
	}
	if (g_Args.getArgCount("unfold_constants")) {
		g_Settings.unfoldConstants = (bool)g_Args.getArgInt("unfold_constants");
	}
	if (g_Args.getArgCount("insert_junk")) {
		g_Settings.insertJunkCode = (bool)g_Args.getArgInt("insert_junk");
	}
	if (g_Args.getArgCount("fixed_base")) {
		g_Settings.fixedBase = (bool)g_Args.getArgInt("fixed_base");
	}
	if (g_Args.getArgCount("display_disasm")) {
		g_Settings.displayDisasm = (bool)g_Args.getArgInt("display_disasm");
	}

	logger.write(LOG_MSG, "Copying %s to %s\n", inputFilePath.c_str(), outputFilePath.c_str());

	if (!CopyFileA(inputFilePath.c_str(), outputFilePath.c_str(), FALSE)) {
		logger.write(LOG_MSG, "Could not create output file: %d\n", GetLastError());
		return false;
	}
	
	// VA targets for absolute jumps, calls
	std::vector<DWORD> vas;
	int vaCount = g_Args.getArgCount("va_target");
	if (vaCount == 0) {
		logger.write(LOG_MSG, "No absolute address fix-ups specified\n");
	}
	else {
		for (int i = 0; i < vaCount; i++) {
			DWORD va = g_Args.getArgIntOccurrence("va_target", i, 16);
			vas.push_back(va);
		}
	}

	MappedPeFile outputFile(outputFilePath.c_str());
	outputFile.map(0x1000); 
	// TODO: rather use 0, then in install.cpp remap with the actual size needed 
	// (which is known after the VirtualizeRegions function is called)

	g_TargetPe = &outputFile;

	// General PE changes
	if (g_Settings.fixedBase) {
		logger.write(LOG_ERROR, "Setting fixed base\n");
		outputFile.getNtHeaders()->FileHeader.Characteristics |= IMAGE_FILE_RELOCS_STRIPPED;
	}

	bool isStandalone = (bool)g_Args.getArgInt("standalone");
	if (!isStandalone) {	
		// Library mode - VM is contained within the target executable
		VmFile vm(&outputFile);
		if (!vm.load()) {
			logger.write(LOG_ERROR, "Vm::Load failed\n");
			return false;
		}

		return InstallVmLib(&outputFile, &vm, vas);
	} else {	
		//	Stand-alone mode, VM needs to be copied into the target,
		//	and regions to protect need to be specified manually

		char originalVmPath[MAX_PATH];
		sprintf(originalVmPath, "%s/%s", GetRootDirectory(), "phant.exe");

		char tempVmPath[MAX_PATH];
		sprintf(tempVmPath, "%s/%s", GetTempDirectory(), "phant_temp.bin");

		// Make a working copy of the VM file
		if (!CopyFileA(originalVmPath, tempVmPath, FALSE)) {
			logger.write(LOG_ERROR, "Could not make a copy of the VM\n");
			logger.write(LOG_MSG, "CopyFile failed: %d\n", GetLastError());
			return false;
		}

		// Map it
		MappedPeFile vmFile(tempVmPath);
		vmFile.map(0x1000);

		VmFile vm(&vmFile);
		if (!vm.load()) {
			logger.write(LOG_ERROR, "Vm::Load failed\n");
			return false;
		}

		// Protected sections
		std::vector<std::pair<DWORD, DWORD>> targets;
		int protectedCount = g_Args.getArgCount("protected_section");
		for (int i = 0; i < protectedCount; i++) {
			DWORD va = g_Args.getArgIntOccurrence("protected_section", i, 16);
			DWORD length = g_Args.getArgIntOccurrence("protected_section_size", i, 10);
			targets.push_back(std::make_pair(va, length));
		}

		if (targets.size() == 0) {
			logger.write(LOG_ERROR, "No code regions to protect\n");
			return false;
		}

		bool success = InstallVmStandalone(&outputFile, &vm, targets, vas);
		logger.write(LOG_MSG, "VM installed, saving and unmapping file\n");
		outputFile.unmap();
		vmFile.unmap();

		DeleteFile(tempVmPath);

		return success;
	}
}


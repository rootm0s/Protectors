#pragma once
#include "stdafx.h"
#include "MappedPeFile.h"
#include "Vm.h"
#include "x86.h"

bool InstallVm();
bool InstallVmLib(MappedPeFile *target, VmFile *vm, std::vector<DWORD> fixups);
bool InstallVmStandalone(MappedPeFile *target, VmFile *vm, std::vector<std::pair<DWORD,DWORD>> targets, std::vector<DWORD> vas);
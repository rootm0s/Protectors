#include "stdafx.h"
#include "ArgsParser.h"
#include "MappedPeFile.h"

extern ArgsParser g_Args;

// Global assigned in RunCli, alive until it returns
extern MappedPeFile *g_TargetPe;

bool RunCli(int argc, char** argv);


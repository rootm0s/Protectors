#pragma once
#include "stdafx.h"

// Settings
struct Settings {
	// Virtualization settings
	bool randomizeOpcodes;
	bool randomizeRegisters;
	bool unfoldConstants;
	bool insertJunkCode;
	// General settings
	bool fixedBase;
	bool displayDisasm;
};

extern Settings g_Settings;
void LoadDefaultSettings();
// /Settings
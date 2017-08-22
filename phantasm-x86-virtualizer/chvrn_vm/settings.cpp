#include "stdafx.h"
#include "settings.h"

Settings g_Settings;

void LoadDefaultSettings() {
	g_Settings.insertJunkCode = true;
	g_Settings.randomizeOpcodes = true;
	g_Settings.randomizeRegisters = false;
	g_Settings.unfoldConstants = true;
	g_Settings.fixedBase = true;
	g_Settings.displayDisasm = true;
}
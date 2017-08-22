#pragma once
#pragma comment(lib, "../Release/libphant.lib")

extern "C" {
	void __stdcall BeginProtect(unsigned int);
	void __stdcall EndProtect(unsigned int);
}

//BeginProtect(0xb1f12057)
//EndProtect(0x5720f1b1);
#define BeginProtect __asm {\
	__asm push 0xb1f12057\
	__asm call BeginProtect\
}
#define EndProtect __asm {\
	__asm push 0x5720f1b1\
	__asm call EndProtect\
}
#pragma once
#include "stdafx.h"

namespace Utils {
	DWORD hexStr(const BYTE *bytes, DWORD len, char *buf, DWORD bufLen, DWORD padLen=0, char padChar=' ') ;
	DWORD padStr(char *buf, DWORD strLen, DWORD padLen, char padChar=' ');
	DWORD roundUpMultiple(DWORD value, DWORD mult);
};
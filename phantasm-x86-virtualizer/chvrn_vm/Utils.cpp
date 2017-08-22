#include "stdafx.h"
#include "Utils.h"

DWORD Utils::hexStr(const BYTE *bytes, DWORD len, char *buf, DWORD bufLen, DWORD padLen, char padChar) {
	static const char *digits = "0123456789abcdef";
	unsigned int i = 0;
	unsigned int j = 0;
	while (i < len && j + 3 < bufLen) {
		BYTE hi = (bytes[i] >> 4) & 0x0f;
		BYTE lo = bytes[i] & 0x0f;
		buf[j] = digits[hi];
		buf[j+1] = digits[lo];
		i++;
		j+=2;
	}
	buf[j] = '\0';

	if (j < padLen) {
		return Utils::padStr(buf, j, padLen, padChar);
	}

	return j;
}

DWORD Utils::padStr(char *str, DWORD strLen, DWORD padLen, char padChar) {
	for (unsigned int i = strLen; i < padLen; i++) {
		if (i == padLen - 1) {
			str[i] = '\0';
		} else {
			str[i] = padChar;
		}
	}

	return padLen;
}

DWORD Utils::roundUpMultiple(DWORD value, DWORD multiple) {
	return ((value + multiple - 1) / multiple) * multiple;
}
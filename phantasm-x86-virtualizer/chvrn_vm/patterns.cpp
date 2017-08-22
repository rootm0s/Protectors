#include "stdafx.h"

// just a "strstr", no wildcards used
DWORD findBytePattern(BYTE *data, DWORD length, BYTE *pattern, DWORD patternLength) {
	DWORD matchCount = 0;
	for (DWORD i = 0; i < length; i++) {
		if (data[i] == pattern[matchCount]) {
			matchCount++;
		}
		else {
			matchCount = 0;
		}
		if (matchCount == patternLength) {
			return i - (patternLength - 1);
		}
	}
	return (DWORD)-1;
}
#include "stdafx.h"
#include "patterns.h"

static DWORD g_markerSize = 5 + 5; // push <dword>, jmp <dword rel offset>

DWORD findBeginProtectMarker(BYTE *data, DWORD length) {
	static BYTE pattern[] = { 0x68, 0x57, 0x20, 0xf1, 0xb1 };
	return findBytePattern(data, length, pattern, sizeof(pattern));
}

DWORD findEndProtectMarker(BYTE *data, DWORD length) {
	static BYTE pattern[] = { 0x68, 0xb1, 0xf1, 0x20, 0x57 };
	return findBytePattern(data, length, pattern, sizeof(pattern));
}

// Scan for pairs of BeginProtect and EndProtect markers
std::vector<std::pair<DWORD, DWORD>> scanForMarkers(BYTE *data, DWORD length) {
	logger.write(LOG_VERBOSE, "Scanning for markers...\n");
	std::vector<std::pair<DWORD, DWORD>> regions;

	DWORD bufferOffset = 0;
	while (true) {
		DWORD beginMarkerOffset = findBeginProtectMarker(data + bufferOffset, length);
		DWORD endMarkerOffset = findEndProtectMarker(data + bufferOffset, length);

		if (beginMarkerOffset == -1) {	// done
			break;
		}

		if (endMarkerOffset == -1) {
			logger.write(LOG_ERROR, "Missing EndProtect marker\n");
			break;
		}
		if (beginMarkerOffset > endMarkerOffset) {
			logger.write(LOG_ERROR, "BeginProtect/EndProtect markers in incorrect order\n");
			break;
		}

		length -= endMarkerOffset + g_markerSize;

		beginMarkerOffset += bufferOffset;
		endMarkerOffset += bufferOffset;

		bufferOffset += endMarkerOffset + g_markerSize;

		logger.write(LOG_VERBOSE, "BeginMarkerOffset: %x\n", beginMarkerOffset);
		logger.write(LOG_VERBOSE, "EndMarkerOffset: %x\n", endMarkerOffset);

		regions.push_back(std::make_pair(beginMarkerOffset + g_markerSize, endMarkerOffset - (beginMarkerOffset + g_markerSize)));
	}

	return regions;
}
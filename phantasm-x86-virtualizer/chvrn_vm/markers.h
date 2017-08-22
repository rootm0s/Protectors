#pragma once
#include "stdafx.h"

DWORD findBeginProtectMarker(BYTE *data, DWORD length);
DWORD findEndProtectMarker(BYTE *data, DWORD length);
std::vector<std::pair<DWORD, DWORD>> scanForMarkers(BYTE *data, DWORD length);
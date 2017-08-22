#pragma once
#include "stdafx.h"
#include "MappedFile.h"

class MappedPeFile : public MappedFile {
public:
	MappedPeFile(const char *filename);
	bool map(DWORD extensionSize=0);

	PIMAGE_DOS_HEADER getDosHeader();
	PIMAGE_NT_HEADERS getNtHeaders();

	// Section handling:
	PIMAGE_SECTION_HEADER getSectionTable();

	DWORD rva2FileOffset(DWORD rva) const;
	DWORD va2FileOffset(DWORD va) const;
	void *va2ptr(DWORD va) const;
	
	DWORD fileOffset2Rva(DWORD offset) const;
	DWORD fileOffset2Va(DWORD offset) const;
	
	DWORD ptr2offset(void *ptr);
	DWORD ptr2va(void *ptr);

	DWORD getOffsetToEof();
	DWORD getNextSectionAlignedVa();
	DWORD createSection(char *szName, DWORD dwCharacteristics);
	DWORD addToLastSection(const BYTE* pbData, DWORD dwSize);

	DWORD getCurrentVa();
	PIMAGE_SECTION_HEADER getSectionFromName(const char *name);
	PIMAGE_SECTION_HEADER getSectionFromVa(DWORD va);

	DWORD getIatVa(const char *module, DWORD ordinal);
	DWORD getIatVa(const char *module, const char *name);

	DWORD patchVa(DWORD va, DWORD value);

	bool hasRelocations();
	bool isRelocatableVa(DWORD va);
protected:
	DWORD roundSectionVa(DWORD rva);
	DWORD roundFileOffset(DWORD offset);

	void parseRelocations();

	PIMAGE_DOS_HEADER m_pDOS;
	PIMAGE_NT_HEADERS m_pNT;
	PIMAGE_SECTION_HEADER m_pSectionTable;
	PIMAGE_DATA_DIRECTORY m_pImportDataDirectory;
	std::vector<DWORD> m_relocations;
};
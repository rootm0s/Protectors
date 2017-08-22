#include "stdafx.h"
#include "MappedPeFile.h"
#include "Utils.h"
#include "cli.h"

MappedPeFile::MappedPeFile(const char *filename) : MappedFile(filename) {

}

bool MappedPeFile::map(DWORD extensionSize)
{
	if (!MappedFile::map(extensionSize)) {
		return false;
	}

	m_pDOS = (PIMAGE_DOS_HEADER)m_fileBuf;
	if (m_pDOS->e_magic != IMAGE_DOS_SIGNATURE) {
		printf("invalid DOS signature\n");
		return false;
	}

	m_pNT = (PIMAGE_NT_HEADERS)(m_fileBuf + m_pDOS->e_lfanew);
	if (m_pNT->Signature != IMAGE_NT_SIGNATURE) {
		printf("invalid PE signature\n");
		return false;
	}
	
	m_pSectionTable = (PIMAGE_SECTION_HEADER)((DWORD)&m_pNT->OptionalHeader + m_pNT->FileHeader.SizeOfOptionalHeader);
	m_pImportDataDirectory = &m_pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	parseRelocations();

	return true;
}

PIMAGE_DOS_HEADER MappedPeFile::getDosHeader() {
	return m_pDOS;
}

PIMAGE_NT_HEADERS MappedPeFile::getNtHeaders() {
	return m_pNT;
}

PIMAGE_SECTION_HEADER MappedPeFile::getSectionTable() {
	return m_pSectionTable;
}

DWORD MappedPeFile::rva2FileOffset(DWORD rva) const
{
	for (int i = 0; i < m_pNT->FileHeader.NumberOfSections; i++)
	{
		if (m_pSectionTable[i].VirtualAddress <= rva && rva < m_pSectionTable[i].VirtualAddress +m_pSectionTable[i].SizeOfRawData) {
			return (rva - m_pSectionTable[i].VirtualAddress) + m_pSectionTable[i].PointerToRawData;
		}
	}
	return 0;
}

DWORD MappedPeFile::va2FileOffset(DWORD va) const
{
	return rva2FileOffset(va-m_pNT->OptionalHeader.ImageBase);
}

void *MappedPeFile::va2ptr(DWORD va) const {
	return (void*)(getFileBuffer() + va2FileOffset(va));
}

DWORD MappedPeFile::fileOffset2Rva(DWORD offset) const
{
	for (int i = 0; i < m_pNT->FileHeader.NumberOfSections; i++)
	{
		if (m_pSectionTable[i].PointerToRawData <= offset && offset < m_pSectionTable[i].PointerToRawData + m_pSectionTable[i].SizeOfRawData) {
			return (offset - m_pSectionTable[i].PointerToRawData) + m_pSectionTable[i].VirtualAddress;
		}
	}
	return 0;
}

DWORD MappedPeFile::fileOffset2Va(DWORD offset) const
{
	return fileOffset2Rva(offset) + m_pNT->OptionalHeader.ImageBase;
}


DWORD MappedPeFile::ptr2offset(void *ptr) {
	return (DWORD)ptr - (DWORD)getFileBuffer();
}

DWORD MappedPeFile::ptr2va(void *ptr) {
	return fileOffset2Va(ptr2offset(ptr));
}

// Returns the offset to the current EOF, which should be a multiple of FileAlignment.
// Unlike using the file size, this will not count any bytes appended to the end of the file that are located past the last section
DWORD MappedPeFile::getOffsetToEof()
{
	return m_pSectionTable[m_pNT->FileHeader.NumberOfSections - 1].PointerToRawData + m_pSectionTable[m_pNT->FileHeader.NumberOfSections - 1].SizeOfRawData;
}

DWORD MappedPeFile::getNextSectionAlignedVa()
{
	PIMAGE_SECTION_HEADER lastSection = &m_pSectionTable[m_pNT->FileHeader.NumberOfSections - 1];
	return roundSectionVa(lastSection->VirtualAddress + lastSection->SizeOfRawData);
}

// Create a new section
// the section won't be valid until data is added to it using AddToSection
DWORD MappedPeFile::createSection(char *szName, DWORD dwCharacteristics)
{
	IMAGE_SECTION_HEADER section;
	ZeroMemory(&section, sizeof(IMAGE_SECTION_HEADER));

	section.Characteristics = dwCharacteristics;
	section.Misc.VirtualSize = 0x0;
	strncpy((char*)section.Name, szName, 8);
	section.PointerToRawData = getOffsetToEof(); 
	section.SizeOfRawData = roundFileOffset(0x0);
	section.VirtualAddress = getNextSectionAlignedVa(); 

	memcpy(&m_pSectionTable[m_pNT->FileHeader.NumberOfSections++], &section, sizeof(IMAGE_SECTION_HEADER));

	return m_pNT->FileHeader.NumberOfSections-1;
}


DWORD MappedPeFile::addToLastSection(const BYTE* pbData, DWORD dwSize)
{
	PIMAGE_SECTION_HEADER pSection = &m_pSectionTable[m_pNT->FileHeader.NumberOfSections - 1];

	memcpy((void*)(m_fileBuf + pSection->PointerToRawData + pSection->Misc.VirtualSize), pbData, dwSize);

	DWORD va = m_pNT->OptionalHeader.ImageBase + pSection->VirtualAddress + pSection->Misc.VirtualSize;

	// Update size
	pSection->Misc.VirtualSize += dwSize;
	pSection->SizeOfRawData = roundFileOffset(pSection->Misc.VirtualSize);
	m_pNT->OptionalHeader.SizeOfImage = pSection->VirtualAddress + pSection->Misc.VirtualSize;

	return va;
}

/*
PIMAGE_SECTION_HEADER MappedPeFile::getLastSection(PE_File *pe_file) {
	return &m_pSectionTable[g_dwLastSectionId];
}
*/

// Includes image base
DWORD MappedPeFile::getCurrentVa()
{
	PIMAGE_SECTION_HEADER pSection = &m_pSectionTable[m_pNT->FileHeader.NumberOfSections-1];
	return m_pNT->OptionalHeader.ImageBase + pSection->VirtualAddress + pSection->Misc.VirtualSize;
}

/*
DWORD MappedPeFile::getCurrentFileOffset(PE_File *pe_file, DWORD dwAlignment = 0)
{
	PIMAGE_SECTION_HEADER pSection = &m_pSectionTable[m_pNT->FileHeader.NumberOfSections-1];

	return RoundUpMultiple(pSection->PointerToRawData + pSection->Misc.VirtualSize, dwAlignment);
}
*/

DWORD MappedPeFile::roundSectionVa(DWORD rva) {
	return Utils::roundUpMultiple(rva, m_pNT->OptionalHeader.SectionAlignment);
}

DWORD MappedPeFile::roundFileOffset(DWORD offset) {
	return Utils::roundUpMultiple(offset, m_pNT->OptionalHeader.FileAlignment);
}

void MappedPeFile::parseRelocations()
{
	if ((getNtHeaders()->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) == 1) {
		return;
	}

	DWORD relocsRva = getNtHeaders()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	DWORD relocsSize = getNtHeaders()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

	PIMAGE_BASE_RELOCATION pBaseReloc = (PIMAGE_BASE_RELOCATION)(va2ptr(getNtHeaders()->OptionalHeader.ImageBase + relocsRva));
	PIMAGE_BASE_RELOCATION reloc = pBaseReloc;

	unsigned int numBytes = 0;
	unsigned int count = 0;

	do {
		if (numBytes >= relocsSize) {
			break;
		}

		WORD *pRelocData = (WORD*)((DWORD)reloc + sizeof(IMAGE_BASE_RELOCATION));
		for (DWORD i = 0; i < (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD); i++)
		{

			int type = pRelocData[i] >> 12; // top 4
			int offset = pRelocData[i] & 0xFFF; // bottom 12

			if (type == IMAGE_REL_BASED_HIGHLOW) {
				DWORD dwRVA = reloc->VirtualAddress + offset;
				DWORD dwVA = (DWORD)getNtHeaders()->OptionalHeader.ImageBase + dwRVA;

				m_relocations.push_back(dwVA);
				count++;
			}
		}
		numBytes += reloc->SizeOfBlock;
		reloc = (PIMAGE_BASE_RELOCATION)((DWORD)reloc + (DWORD)reloc->SizeOfBlock);
	} while (reloc->VirtualAddress != 0);

	return;
}

PIMAGE_SECTION_HEADER MappedPeFile::getSectionFromName(const char *name) {
	for (int i = 0; i < m_pNT->FileHeader.NumberOfSections; i++)
	{
		if (!strcmp((const char*)m_pSectionTable[i].Name, name)) {
			return &m_pSectionTable[i];
		}
	}

	return NULL;
}

// VA as given by the image base, not wherever the file is mapped
PIMAGE_SECTION_HEADER MappedPeFile::getSectionFromVa(DWORD va) {
	for (int i = 0; i < m_pNT->FileHeader.NumberOfSections; i++)
	{
		if (m_pNT->OptionalHeader.ImageBase + m_pSectionTable[i].VirtualAddress <= va && va < m_pNT->OptionalHeader.ImageBase + m_pSectionTable[i].VirtualAddress + m_pSectionTable[i].Misc.VirtualSize)
			return &m_pSectionTable[i];
	}

	return NULL;
}

DWORD MappedPeFile::getIatVa(const char *module, DWORD ordinal)
{
	return getIatVa(module, (const char*)ordinal);
}

DWORD MappedPeFile::getIatVa(const char *module, const char *name)
{
	for (PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)m_pDOS + rva2FileOffset(m_pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)); 
		imp->Characteristics != 0; 
		imp++) {

		const char *szModule = (const char*)((DWORD)m_pDOS + rva2FileOffset(imp->Name));
		if (stricmp(module, szModule) != 0) {
			continue;
		}

		for (PIMAGE_THUNK_DATA originalThunk = (PIMAGE_THUNK_DATA)((DWORD)m_pDOS + rva2FileOffset(imp->OriginalFirstThunk)),
			firstThunk = (PIMAGE_THUNK_DATA)((DWORD)m_pDOS + rva2FileOffset(imp->FirstThunk));
			originalThunk->u1.Function != 0;
			originalThunk++, firstThunk++) {
				if (originalThunk->u1.Function & IMAGE_ORDINAL_FLAG32) {
					if ((DWORD)name == IMAGE_ORDINAL32(originalThunk->u1.Function)) {
						return ptr2va(&firstThunk->u1.Function);
					}
				} else {
					const char *importName = (const char*)((PIMAGE_IMPORT_BY_NAME)((DWORD)m_pDOS + rva2FileOffset(originalThunk->u1.AddressOfData)))->Name;
					if (!stricmp(name, importName)) {
						return ptr2va(&firstThunk->u1.Function);
					}
				}
			}
	}

	return 0;
}

DWORD MappedPeFile::patchVa(DWORD va, DWORD value) {
	DWORD *ptr = (DWORD*)(getFileBuffer()+va2FileOffset(va));
	DWORD oldValue = *ptr;
	*ptr = value;

	logger.write(LOG_MSG, "Patching [%08x] = %08x => %08x (ptr: %x)\n", va, oldValue, value, ptr);

	return oldValue;
}

bool MappedPeFile::hasRelocations()
{
	return m_relocations.size() != 0;
}

bool MappedPeFile::isRelocatableVa(DWORD va)
{
	for (unsigned int i = 0; i < m_relocations.size(); i++) {
		if (m_relocations[i] == va) {
			return true;
		}
	}
	return false;
}

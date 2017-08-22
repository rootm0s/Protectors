#pragma once
#include "stdafx.h"

class MappedFile {
public:
	MappedFile(const char *filename);
	~MappedFile();
	virtual bool map(DWORD extensionSize=0);
	virtual void unmap();
	bool remap(DWORD extensionSize=0);
	BYTE *getFileBuffer() const;
	DWORD getTotalSize() const;
protected:
	bool m_isMapped;
	const char *m_filename;
	DWORD m_extensionSize;
	BYTE *m_fileBuf;
	DWORD m_fileSize;
	HANDLE m_fileHandle;
	HANDLE m_fileMappingObject;
};

MappedFile* File_Map(const char *szFileName, DWORD dwExtensionSize);
void File_Unmap(MappedFile **file);
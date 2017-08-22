#include "stdafx.h"
#include "MappedFile.h"

MappedFile::MappedFile(const char *filename) {
	m_isMapped = false;
	m_filename = filename;
}

MappedFile::~MappedFile() {
	unmap();
}

bool MappedFile::map(DWORD extensionSize) {
	m_extensionSize = extensionSize;

	m_fileHandle = CreateFileA(m_filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_fileHandle == INVALID_HANDLE_VALUE) {
		logger.write(LOG_ERROR, "CreateFile failed: %d\n", GetLastError());
		return false;
	}

	m_fileSize = GetFileSize(m_fileHandle, NULL);
	if (m_fileSize == -1) {
		logger.write(LOG_ERROR, "GetFileSize failed: %d\n", GetLastError());
		CloseHandle(m_fileHandle);
		return false;
	}
	
	m_fileMappingObject = CreateFileMappingA(m_fileHandle, NULL, PAGE_READWRITE, 0, m_fileSize + m_extensionSize, NULL);
	if (m_fileMappingObject == NULL) {
		logger.write(LOG_ERROR, "CreateFileMappingA failed: %d\n", GetLastError());

		CloseHandle(m_fileHandle);
		return false;
	}

	m_fileBuf = (BYTE*)MapViewOfFile(m_fileMappingObject, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (m_fileBuf == NULL) {
		logger.write(LOG_ERROR, "MapViewOfFile failed: %d\n", GetLastError());

		CloseHandle(m_fileHandle);
		return false;
	}

	m_isMapped = true;
	return true;
}

void MappedFile::unmap() {
	if (m_isMapped) {
		FlushViewOfFile(m_fileBuf, 0);
		UnmapViewOfFile(m_fileBuf);
		CloseHandle(m_fileMappingObject);
		CloseHandle(m_fileHandle);
		m_isMapped = false;
	}
}

bool MappedFile::remap(DWORD extensionSize) {
	unmap();
	return map(extensionSize);
}

BYTE *MappedFile::getFileBuffer() const {
	return m_fileBuf;
}

DWORD MappedFile::getTotalSize() const {
	return m_fileSize + m_extensionSize;
};
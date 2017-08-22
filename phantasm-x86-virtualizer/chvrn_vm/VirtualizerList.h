#include "stdafx.h"
#include "CodeChunk.h"

typedef bool (*pfnVirtualize)(CodeChunk *code, DISASM *disasm);

class VirtualizerList {
public:
	VirtualizerList();
	~VirtualizerList();
	bool addVirtualizer(pfnVirtualize virtualizer, DWORD opcode);
	bool addVirtualizer(pfnVirtualize virtualizer, DWORD *opcodes, DWORD count);
	bool virtualize(CodeChunk *code, DISASM *disasm) const;

	void print() const;
private:
	DWORD m_count;
	pfnVirtualize *m_virtualizers;
};
#include "stdafx.h"
#include "MappedPeFile.h"
#include "CodeChunk.h"

/*
	Thought of doing relocations via a handler like so:
	doVmPush
	doVmRebase
	doPushDword
	...
	Pro: possible to add more crap on top of instructions, like XOR:ing before decoding
	Con: the native dispatch handler then needs fixing
*/
unsigned int getRelocationOffset(MappedPeFile *target, DISASM *disasm) {
	for (unsigned int i = disasm->VirtualAddr; i < disasm->VirtualAddr + disasm->SecurityBlock; i++) {
		if (target->isRelocatableVa(i)) {
			return i;
		}
	}
	return (unsigned int)-1;
}

void test_f() {

}

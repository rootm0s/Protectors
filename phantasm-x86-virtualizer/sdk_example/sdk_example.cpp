// sdk_example.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "phant.h"

__forceinline unsigned int fnv32(unsigned char *data, unsigned int size) {
	unsigned int hash = 0x0ce942fa;
	for (unsigned int i = 0; i < size; i++) {
		hash *= 0x01000193;
		hash ^= data[i];
	}
	return hash;
}

/*
	Suggested solution:
	BP the VM IP and observe the control flow for a clean run vs. patched run,
	this lets you find the good conditional jumps. 
	Patch the first DWORD to make the bad jump land at the destination of the last good jump.
*/
int main() 
{
	BeginProtect;
	char buffer[MAX_PATH];
	printf("Enter a string:\n");
	scanf_s("%s", buffer);

	const char *caption = "Evaluation expired";
	const char *text = "Please register the software";

	__asm {
		mov dword ptr[esp - 32], 0x38af1fad	// bad cookie #1
			push esp
			mov dword ptr[esp - 52 + 4], 0x38af1fad	// bad cookie #2
			push MB_OK
			push caption
			push text
			push NULL
			xor dword ptr[esp - 32 + 4 + 16], 0x7f338f5d	// => 479c90f0
			call dword ptr[MessageBoxA]
			pop eax
			xor eax, esp
			jnz do_nothing
			mov eax, dword ptr[esp - 68 - 4]
			mov ebx, caption
			xor eax, ebx
			jnz do_nothing
			mov eax, dword ptr[esp - 32]
			xor eax, 0x479c90f0
			jnz second_check
		access_violation :
			mov dword ptr ds : [0x40100], 0
			second_check :
			mov eax, dword ptr[esp - 52]
			xor eax, 0x38af1fad
			jz do_nothing
	}

	printf("Hash: 0x%x\n", fnv32((unsigned char*)buffer, strlen(buffer)));

do_nothing:
	printf("Done.\n");

	EndProtect;
	
	return 0;
}


#### Example program
```c++
#include "stdafx.h"
#include "phant.h"

DWORD hashFile(const char *filename) {
	BeginProtect;

	HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	DWORD hash = 0;
	if (file == INVALID_HANDLE_VALUE) {
		printf("could not open file for reading (error: %d)\n", GetLastError());
		return 0;
	}
	else {

		DWORD fileSize = GetFileSize(file, NULL);

		DWORD buf = 0;
		DWORD numRead;

		hash = 0x811c9dc5;
		while (ReadFile(file, &buf, sizeof(buf), &numRead, NULL)) {
			if (!numRead) {
				break;
			}
			hash = (hash ^ buf) * 0x1000193;
			buf = 0;
		}
		CloseHandle(file);

	}

	EndProtect;

	return hash;
}

int main(int argc, const char *argv[])
{
	BeginProtect;

	if (argc != 2) {
		printf("no filename specified\n");
		return 1;
	}

	DWORD hash = hashFile(argv[1]);
	printf("result: 0x%08x\n", hash);

	EndProtect;

    return 0;
}
```

The disassembly of the hashFile function looks like so:
```
CPU Disasm
Address   Hex dump          Command                                  Comments
00401B90  /$  55            PUSH EBP                                 ; UINT sdk_example_two.hashFile(void)
00401B91  |.  8BEC          MOV EBP,ESP
00401B93  |.  83EC 0C       SUB ESP,0C
00401B96  |.  A1 F8404000   MOV EAX,DWORD PTR DS:[__security_cookie]
00401B9B  |.  33C5          XOR EAX,EBP
00401B9D  |.  8945 FC       MOV DWORD PTR SS:[LOCAL.1],EAX
00401BA0  |.  53            PUSH EBX
00401BA1  |.  56            PUSH ESI
00401BA2  |.  57            PUSH EDI
00401BA3  |.  8BF1          MOV ESI,ECX
00401BA5  |.  68 5720F1B1   PUSH B1F12057                            ; /Arg1 = B1F12057
00401BAA  |.  E8 59F4FFFF   CALL _BeginProtect@4                     ; \sdk_example_two._BeginProtect@4
00401BAF  |.  6A 00         PUSH 0                                   ; /hTemplate = NULL
00401BB1  |.  68 80000000   PUSH 80                                  ; |Attributes = FILE_ATTRIBUTE_NORMAL
00401BB6  |.  6A 03         PUSH 3                                   ; |CreationDistribution = OPEN_EXISTING
00401BB8  |.  6A 00         PUSH 0                                   ; |pSecurity = NULL
00401BBA  |.  6A 01         PUSH 1                                   ; |ShareMode = FILE_SHARE_READ
00401BBC  |.  68 00000080   PUSH 80000000                            ; |DesiredAccess = GENERIC_READ
00401BC1  |.  56            PUSH ESI                                 ; |FileName => ARG.ECX
00401BC2  |.  FF15 00304000 CALL DWORD PTR DS:[<&KERNEL32.CreateFile ; \KERNEL32.CreateFileA
00401BC8  |.  8BF0          MOV ESI,EAX
00401BCA  |.  83FE FF       CMP ESI,-1
00401BCD  |.  75 27         JNE SHORT 00401BF6
00401BCF  |.  FF15 04304000 CALL DWORD PTR DS:[<&KERNEL32.GetLastErr ; [KERNEL32.GetLastError
00401BD5  |.  50            PUSH EAX
00401BD6  |.  68 28314000   PUSH OFFSET 00403128                     ; /_Format = "could not open file for reading (error: %d)\n"
00401BDB  |.  E8 00010000   CALL printf                              ; \printf
00401BE0  |.  83C4 08       ADD ESP,8
00401BE3  |.  33C0          XOR EAX,EAX
00401BE5  |.  5F            POP EDI
00401BE6  |.  5E            POP ESI
00401BE7  |.  5B            POP EBX
00401BE8  |.  8B4D FC       MOV ECX,DWORD PTR SS:[LOCAL.1]
00401BEB  |.  33CD          XOR ECX,EBP
00401BED  |.  E8 1E010000   CALL __security_check_cookie             ; [__security_check_cookie
00401BF2  |.  8BE5          MOV ESP,EBP
00401BF4  |.  5D            POP EBP
00401BF5  |.  C3            RETN
00401BF6  |>  6A 00         PUSH 0                                   ; /SizeHigh = NULL
00401BF8  |.  56            PUSH ESI                                 ; |hFile
00401BF9  |.  FF15 08304000 CALL DWORD PTR DS:[<&KERNEL32.GetFileSiz ; \KERNEL32.GetFileSize
00401BFF  |.  8B1D 0C304000 MOV EBX,DWORD PTR DS:[<&KERNEL32.ReadFil ; Jump to KERNELBASE.ReadFile
00401C05  |.  8D45 F4       LEA EAX,[LOCAL.3]
00401C08  |.  6A 00         PUSH 0                                   ; /pOverlapped = NULL
00401C0A  |.  50            PUSH EAX                                 ; |pBytesRead => OFFSET LOCAL.3
00401C0B  |.  6A 04         PUSH 4                                   ; |Size = 4
00401C0D  |.  8D45 F8       LEA EAX,[LOCAL.2]                        ; |
00401C10  |.  C745 F8 00000 MOV DWORD PTR SS:[LOCAL.2],0             ; |
00401C17  |.  50            PUSH EAX                                 ; |Buffer => OFFSET LOCAL.2
00401C18  |.  56            PUSH ESI                                 ; |hFile
00401C19  |.  BF C59D1C81   MOV EDI,811C9DC5                         ; |
00401C1E  |.  FFD3          CALL EBX                                 ; \KERNEL32.ReadFile
00401C20  |.  85C0          TEST EAX,EAX
00401C22  |.  74 2B         JZ SHORT 00401C4F
00401C24  |>  837D F4 00    /CMP DWORD PTR SS:[LOCAL.3],0
00401C28  |.  74 25         |JE SHORT 00401C4F
00401C2A  |.  8B45 F8       |MOV EAX,DWORD PTR SS:[LOCAL.2]
00401C2D  |.  33C7          |XOR EAX,EDI
00401C2F  |.  C745 F8 00000 |MOV DWORD PTR SS:[LOCAL.2],0
00401C36  |.  6A 00         |PUSH 0                                  ; /pOverlapped = NULL
00401C38  |.  69F8 93010001 |IMUL EDI,EAX,1000193                    ; |
00401C3E  |.  8D45 F4       |LEA EAX,[LOCAL.3]                       ; |
00401C41  |.  50            |PUSH EAX                                ; |pBytesRead => OFFSET LOCAL.3
00401C42  |.  6A 04         |PUSH 4                                  ; |Size = 4
00401C44  |.  8D45 F8       |LEA EAX,[LOCAL.2]                       ; |
00401C47  |.  50            |PUSH EAX                                ; |Buffer => OFFSET LOCAL.2
00401C48  |.  56            |PUSH ESI                                ; |hFile
00401C49  |.  FFD3          |CALL EBX                                ; \KERNEL32.ReadFile
00401C4B  |.  85C0          |TEST EAX,EAX
00401C4D  |.^ 75 D5         \JNZ SHORT 00401C24
00401C4F  |>  56            PUSH ESI                                 ; /hObject
00401C50  |.  FF15 10304000 CALL DWORD PTR DS:[<&KERNEL32.CloseHandl ; \KERNEL32.CloseHandle
00401C56  |.  68 B1F12057   PUSH 5720F1B1                            ; /Arg1 = 5720F1B1
00401C5B  |.  E8 A5F3FFFF   CALL _EndProtect@4                       ; \sdk_example_two._EndProtect@4
00401C60  |.  8B4D FC       MOV ECX,DWORD PTR SS:[LOCAL.1]
00401C63  |.  8BC7          MOV EAX,EDI
00401C65  |.  33CD          XOR ECX,EBP
00401C67  |.  5F            POP EDI
00401C68  |.  5E            POP ESI
00401C69  |.  5B            POP EBX
00401C6A  |.  E8 A1000000   CALL __security_check_cookie             ; [__security_check_cookie
00401C6F  |.  8BE5          MOV ESP,EBP
00401C71  |.  5D            POP EBP
00401C72  \.  C3            RETN
```

### Output
The generated bytecode for the hashFile function is:
```
VmPush 00000000
PushDword
VmPush 0000003e
VmPush 00000002
Mul
VmPush 00000004
VmAdd
PushDword
VmPush 00000002
VmPush 00000001
VmAdd
PushDword
VmPush 00000000
PushDword
VmPush 00000001
PushDword
VmPush 00002745
VmPush 00034270
Mul
VmPush 000007d0
VmAdd
PushDword
PushReg 06
native
VmPushReg 00
VmPopReg 06
VmPush 00004089
VmPush 0003f782
Mul
VmPush 00000b6d
VmAdd
VmPushReg 06
Sub
VmPopRemove
PushFlag 00000040
VmPush 00000036
VmPush 0000000a
VmAdd
Xor
JumpRelCond 00000093
native
PushReg 00
VmPush 00005d36
VmPush 000000b0
Mul
VmPush 00001c08
VmAdd
PushDword
native
VmPush 00000008
VmPushReg 04
Add
VmPopReg 04
VmPushReg 00
VmPushReg 00
Nand
VmPushReg 00
Nand
VmPushReg 00
VmPushReg 00
Nand
VmPushReg 00
Nand
Nand
VmPopReg 00
PopReg 07
PopReg 06
PopReg 03
VmPushReg 05
VmPush 0000161f
VmPush 000b929a
Mul
VmPush 00000356
VmAdd
VmAdd
Deref
VmPopReg 01
VmPushReg 01
VmPushReg 05
Nand
VmPushReg 01
Nand
VmPushReg 01
VmPushReg 05
Nand
VmPushReg 05
Nand
Nand
VmPopReg 01
native
VmPushReg 05
VmPopReg 04
PopReg 05
VmPush 00000000
VmReturn
VmPush 00000000
PushDword
PushReg 06
native
VmPush 00000a21
VmPush 00000656
Mul
VmPush 000002f6
VmAdd
Deref
VmPopReg 03
VmPushReg 05
VmPush 0000268f
VmPush 0006a3a5
Mul
VmPush 000018c9
VmAdd
VmAdd
VmPopReg 00
VmPush 00000000
PushDword
PushReg 00
VmPush 00000003
VmPush 00000001
VmAdd
PushDword
VmPushReg 05
VmPush 00000f31
VmPush 0010da04
Mul
VmPush 00000934
VmAdd
VmAdd
VmPopReg 00
VmPush 00000000
VmPushReg 05
VmPush 000015f5
VmPush 000ba8bd
Mul
VmPush 00000217
VmAdd
VmAdd
Move
PushReg 00
PushReg 06
VmPush 0000354e
VmPush 00026c11
Mul
VmPush 00002b97
VmAdd
VmPopReg 07
native
native
PushFlag 00000040
JumpRelCond 000000e0
VmPush 00000000
VmPushReg 05
VmPush 00002dd2
VmPush 00059648
Mul
VmPush 000010e4
VmAdd
VmAdd
Deref
Sub
VmPopRemove
PushFlag 00000040
JumpRelCond 000000ba
VmPushReg 05
VmPush 00001108
VmPush 000f07fc
Mul
VmPush 00000418
VmAdd
VmAdd
Deref
VmPopReg 00
VmPushReg 00
VmPushReg 07
Nand
VmPushReg 00
Nand
VmPushReg 00
VmPushReg 07
Nand
VmPushReg 07
Nand
Nand
VmPopReg 00
VmPush 00000000
VmPushReg 05
VmPush 000075a1
VmPush 00022d24
Mul
VmPush 00002854
VmAdd
VmAdd
Move
VmPush 00000000
PushDword
native
VmPushReg 05
VmPush 00000538
VmPush 00310dcb
Mul
VmPush 0000048c
VmAdd
VmAdd
VmPopReg 00
PushReg 00
VmPush 00000002
VmPush 00000002
Mul
PushDword
VmPushReg 05
VmPush 00002fdc
VmPush 00055958
Mul
VmPush 00001058
VmAdd
VmAdd
VmPopReg 00
PushReg 00
PushReg 06
native
native
PushFlag 00000040
VmPush 0000000c
VmPush 00000005
Mul
VmPush 00000004
VmAdd
Xor
JumpRelCond ffffff20
PushReg 06
native
VmPush 00005b14
VmPush 000000b4
Mul
VmPush 000011a9
VmAdd
VmExit
```
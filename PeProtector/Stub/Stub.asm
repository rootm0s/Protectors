; PeProtector

IMPORT KERNEL32.VirtualAlloc
IMPORT KERNEL32.VirtualFree
IMPORT KERNEL32.VirtualProtect
IMPORT KERNEL32.LoadLibraryA
IMPORT KERNEL32.GetProcAddress
IMPORT KERNEL32.IsDebuggerPresent

EXTERN DD externImageBase
EXTERN DD externImageSize
EXTERN DD externOEP

SECTION ".text" crwe
_stubBegin:

 
;   MOV  EAX,DWORD PTR FS:[18H]
;   MOV  EAX,DWORD PTR [EAX + 30H]
;   MOVZX EAX,BYTE PTR [EAX + 2H]
   
   ; test seh begin
   PUSH _sehHandler
   PUSH DWORD PTR FS:[0]	      ; address of next ERR struct
   MOV  DWORD PTR FS:[0], ESP 	  ; push ERR address in FS:[0]
   
   MOV  EAX, 1234H
   MOV  EBX, DWORD PTR [EAX]

   POP DWORD PTR FS:[0]	   ; return previous address from FS:[0]
   ADD ESP, 4H	           ; clear stack
   JMP  _begin
   
_sehHandler: 

; ESP + 4	pointer to EXCEPTION_RECORD
; ESP + 8	pointer to ERR
; ESP + CH	pointer to CONTEXT
; ESP + 10H	Param
   
   ; first debug check
   MOV EAX, 10 ; fake instruction
   CMP BYTE PTR [_sehHandler], 0CCH
   JZ _stubBegin
   
   ; second debug check
   CALL DWORD PTR [KERNEL32.IsDebuggerPresent]
   CMP EAX, 1
	JZ _stubBegin

   MOV  EAX, DWORD PTR [ESP + 0CH]
   ; +B0 eax register
   MOV DWORD PTR [EAX + 0B0H], _begin
   
   MOV  EAX, 0	  ; Returned values:
                  ; eax = 1 execption wasn;t processed, pass to the next exception handler in chain
                  ; eax = 0 reload context and continue execution
   RET

_begin:
   ; 0. save api
   MOV  EAX, DWORD PTR [KERNEL32.LoadLibraryA]
   MOV  DWORD PTR [ddLoadLibraryA], EAX
   
   MOV  EAX, DWORD PTR [KERNEL32.GetProcAddress]
   MOV  DWORD PTR [ddGetProcAddress], EAX

   MOV  EAX, DWORD PTR [KERNEL32.VirtualFree]
   MOV  DWORD PTR [ddVirtualFree], EAX
   
   ; 1. allocate memory for stub code
   PUSH 40H                                 ; PAGE_EXECUTE_READWRITE = 0x40
   PUSH 1000H                               ; MEM_COMMIT             = 0x00001000
   PUSH _stubEnd - _stubBegin               ; size of allocated memory
   PUSH 0                                   ; zero if any address
   CALL DWORD PTR [KERNEL32.VirtualAlloc]   ; LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
   ; save allocated memory
   MOV  DWORD PTR [ddStubMemory], EAX
   
   ; 2. allocate memory for compressed data
   PUSH 40H                                 ; PAGE_EXECUTE_READWRITE = 0x40
   PUSH 1000H                               ; MEM_COMMIT             = 0x00001000
   PUSH _compressedFileEnd - _compressedFileBegin ; size of allocated memory
   PUSH 0                                   ; zero if any address
   CALL DWORD PTR [KERNEL32.VirtualAlloc]   ; LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
   ; save allocated memory
   MOV  DWORD PTR [ddCompressedFileMemory], EAX

   ; 3. set readwrite attributes to image
   PUSH ddOldProtectFlags                    ; lpflOldProtect
   PUSH 40h                                  ; PAGE_EXECUTE_READWRITE - 0x40
   PUSH externImageSize                      ; size
   PUSH externImageBase                      ; lpAddress
   CALL DWORD PTR [KERNEL32.VirtualProtect]  ; BOOL WINAPI VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect);
   
   ; 3. copy stub code to the allocated memory
   MOV  ESI, _stubBegin
   MOV  EDI, DWORD PTR [ddStubMemory]
   MOV  ECX, _stubEnd - _stubBegin
   REPZ  MOVSB
   
   ; 4. copy compressed file to the allocated memory
   MOV  ESI, _compressedFileBegin
   MOV  EDI, DWORD PTR [ddCompressedFileMemory]
   MOV  ECX, _compressedFileEnd - _compressedFileBegin
   REPZ  MOVSB

   ; save relative offset in ebp register
   MOV  EBP, DWORD PTR [ddStubMemory]
   SUB  EBP, _stubBegin
   
   ; 5. pass control
   MOV  EAX, DWORD PTR [ddStubMemory]
   ADD  EAX, _jumpToAllocatedMemory - _stubBegin
   JMP  EAX
_jumpToAllocatedMemory:
   
   ; I am in the allocated memory
   MOV ECX, ECX
   MOV EDX, EDX
   
   ; test memory
   MOV EBX, DWORD PTR [EBP + ddStubMemory]
   MOV EBX, DWORD PTR [EBP + ddCompressedFileMemory]
   
   ; decompress
   PUSH externImageBase                     ; destination TODO provide externMemoryBase
   PUSH  DWORD PTR [EBP + ddCompressedFileMemory] ; source
   CALL _aP_depack_asm                      ; void aP_depack_asm(const void *source, void *destination)
   ADD  ESP, 8                              ; erase stack after cdecl call

   ; Restore import
   
   ; struct IMAGE_DOS_HEADER
   ; 0x3C LONG e_lfanew                     ; File address of new exe header

   ; ASSUME EDI:PTR IMAGE_DOS_HEADER
   MOV  EDI, externImageBase
   MOV  EAX, externImageBase
   ADD  EAX, DWORD PTR [EDI + 3CH]         ; IMAGE_DOS_HEADER.e_lfanew
   ; struct IMAGE_NT_HEADERS32
   ; 0x78 IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES]
   ; 0x80 DWORD DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress

   ; ASSUME EAX:PTR IMAGE_NT_HEADERS32
   MOV  ESI, DWORD PTR [EAX + 80H]          ; 0x80 IMAGE_NT_HEADERS32.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress   
   ADD  ESI, externImageBase

   ; struct IMAGE_IMPORT_DESCRIPTOR
   ; 0x00 DWORD OriginalFirstThunk
   ; 0x04 DWORD TimeDateStamp
   ; 0x08 DWORD ForwarderChain
   ; 0x0C DWORD Name 
   ; 0x10 DWORD FirstThunk
_importDescriptionLoopStart:
   ; ASSUME ESI:PTR IMAGE_IMPORT_DESCRIPTOR
   CMP  DWORD PTR [ESI + 0CH], 0            ; IMAGE_IMPORT_DESCRIPTOR.Name == 0
   JZ   _importDescriptionLoopExit
      ; load library
      MOV  EAX, externImageBase
      ADD  EAX, DWORD PTR [ESI + 0CH]       ; image base + IMAGE_IMPORT_DESCRIPTOR.Name
      PUSH EAX                              ; lpFileName
      CALL DWORD PTR [EBP + ddLoadLibraryA]  ; HMODULE LoadLibrary(LPCTSTR lpFileName);
      MOV  DWORD PTR [EBP + ddModuleHandle], EAX ; save HMODULE in EDI

      ; load thunk array
      MOV  EDI, externImageBase
      ADD  EDI, DWORD PTR [ESI + 10H]       ; IMAGE_IMPORT_DESCRIPTOR.FirstThunk

_importDescriptionThunkLoopStart:
      ; ASSUME EDI:PTR THUNK
      CMP  DWORD PTR [EDI], 0
      JZ   _importDescriptionThunkLoopExit
         TEST  DWORD PTR [EDI], 80000000H    ; if thunk contains ordinal value or name
         JZ   _importDescriptionThunkName
            MOV  EAX, DWORD PTR [EDI]
            AND  EAX, 0FFFFH
            PUSH EAX                                ; lpProcName
            PUSH DWORD PTR [EBP + ddModuleHandle]   ; hLib
            CALL DWORD PTR [EBP + ddGetProcAddress] ; FARPROC GetProcAddress(HMODULE hModule, LPCSTR  lpProcName);
            ; save address
            MOV  DWORD PTR [EDI], EAX
            JMP  _importDescriptionThunkSkipName
_importDescriptionThunkName:
            MOV  EAX, externImageBase
            ADD  EAX, DWORD PTR [EDI]          ; EAX points to NAME

            ; struct _IMAGE_IMPORT_BY_NAME
            ; 0x00 WORD Hint
            ; 0x02 CHAR Name[1]
            
            ; ASSUME EAX:PTR IMAGE_IMPORT_BY_NAME
            ADD  EAX, 2
            
            PUSH EAX                                ; lpProcName
            PUSH DWORD PTR [EBP + ddModuleHandle]   ; hLib
            CALL DWORD PTR [EBP + ddGetProcAddress] ; FARPROC GetProcAddress(HMODULE hModule, LPCSTR  lpProcName);
            ; save address
            MOV  DWORD PTR [EDI], EAX
_importDescriptionThunkSkipName:

      ADD  EDI, 4
      JMP  _importDescriptionThunkLoopStart
_importDescriptionThunkLoopExit:

    ADD  ESI, 14H              ; sizeof IMAGE_IMPORT_DESCRIPTOR
    JMP _importDescriptionLoopStart

_importDescriptionLoopExit:
   
   
; antiDump:
   ;/*<thisrel this+0x30>*/ /*|0x4|*/ struct _PEB* ProcessEnvironmentBlock;
;   MOV   EAX, DWORD PTR FS:[30h]   ; EAX ->> PEB Struct
  
   ;/*<thisrel this+0xc>*/ /*|0x4|*/ struct _PEB_LDR_DATA* Ldr;
;   MOV   EAX, DWORD PTR [EAX + 0CH]  ;EAX->>_PEB_LDR_DATA
   
   ;/*<thisrel this+0xc>*/ /*|0x8|*/ struct _LIST_ENTRY InLoadOrderModuleList.Flink
;   MOV   EAX, DWORD PTR [EAX + 0CH]  ;EAX->>_LDR_DATA_TABLE_ENTRY

   ; loop
;   MOV   EBX, externImageBase
;_antiDump_loopStart: 
;   CMP   EBX, DWORD PTR [EAX + 18H]   ;/*<thisrel this + 0x18>*/ /*|0x4|*/ void* DllBase;
;   JNZ   _antiDump_loopNext
   ; fix DllBase
;   MOV   DWORD PTR [EAX + 18h], 0     ;/*<thisrel this + 0x18>*/ /*|0x4|*/ void* DllBase;
   ; fix SizeOfImage
;   MOV   DWORD PTR [EAX + 20H], 0          ;/*<thisrel this + 0x20>*/ /*|0x4|*/ unsigned long SizeOfImage;
;   JMP  _antiDump_loopExit
;_antiDump_loopNext:
;   PUSH DWORD PTR [EAX]       ;/*<thisrel this + 0x0>*/ /*|0x8|*/ struct _LIST_ENTRY InLoadOrderLinks.Flink
;   POP  EAX
;   JMP  _antiDump_loopStart
;_antiDump_loopExit:
   
   PUSH 8000H                                          ; MEM_RELEASE 0x8000
   PUSH 0;_compressedFileEnd - _compressedFileBegin      ; dwSize
   PUSH DWORD PTR [EBP + ddCompressedFileMemory]       ; lpAddress
   CALL DWORD PTR [EBP + ddVirtualFree] ; BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);

   PUSH 8000H                               ; MEM_RELEASE 0x8000
   PUSH 0;_stubEnd - _stubBegin               ; dwSize
   PUSH DWORD PTR [EBP + ddStubMemory]      ; lpAddress
   ; goto the original entry point
   MOV  EAX, externImageBase
   ADD  EAX, externOEP
   PUSH EAX
   JMP  DWORD PTR [EBP + ddVirtualFree] ; BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
  
   
   ; goto the original entry point
;   MOV  EAX, externImageBase
;   ADD  EAX, externOEP
;   JMP  EAX
   
; aP_depack_asm(const void *source, void *destination)
_aP_depack_asm:
    PUSHAD

    MOV    ESI, DWORD PTR [ESP + 36]    ; C CALLing convention
    MOV    EDI, DWORD PTR [ESP + 40]

    CLD
    MOV    DL, 80H
    XOR    EBX, EBX

literal:
    MOVSB
    MOV    BL, 2
nexttag:
    CALL   getbit
    JNC    literal

    XOR    ECX, ECX
    CALL   getbit
    JNC    codepair
    XOR    EAX, EAX
    CALL   getbit
    JNC    shortmatch
    MOV    BL, 2
    INC    ECX
    MOV    AL, 10h
getmorebits:
    CALL   getbit
    ADC    AL, AL
    JNC    getmorebits
    JNZ    domatch
    STOSB
    JMP    nexttag
codepair:
    CALL   getgamma_no_ECX
    SUB    ECX, EBX
    JNZ    normalcodepair
    CALL   getgamma
    JMP    domatch_lastpos

shortmatch:
    LODSB
    SHR    EAX, 1
    JZ     donedepacking
    ADC    ECX, ECX
    JMP    domatch_with_2inc

normalcodepair:
    XCHG   EAX, ECX
    DEC    EAX
    SHL    EAX, 8
    LODSB
    CALL   getgamma
    CMP    EAX, 32000
    JAE    domatch_with_2inc
    CMP    AH, 5
    JAE    domatch_with_inc
    CMP    EAX, 7fh
    JA     domatch_new_lastpos

domatch_with_2inc:
    INC    ECX

domatch_with_inc:
    INC    ECX

domatch_new_lastpos:
    XCHG   EAX, EBP
domatch_lastpos:
    MOV    EAX, EBP

    MOV    BL, 1

domatch:
    PUSH   ESI
    MOV    ESI, EDI
    SUB    ESI, EAX
    REPZ   MOVSB
    POP    ESI
    JMP    nexttag

getbit:
    ADD     dl, dl
    jnz     stillbitsleft
    MOV     dl,byte PTR [ESI]
    inc     ESI
    adc     dl, dl
stillbitsleft:
    RET

getgamma:
    xor    ECX, ECX
getgamma_no_ECX:
    inc    ECX
getgammaloop:
    CALL   getbit
    adc    ECX, ECX
    CALL   getbit
    jc     getgammaloop
    RET

donedepacking:
    SUB    EDI,DWORD PTR [ESP + 40]
    MOV    DWORD PTR [ESP + 28], EDI    ; RETurn unpacked length in EAX

    POPAD
    RET
    
    
   ddStubMemory DD 0
   ddCompressedFileMemory DD 0
   ddOldProtectFlags  DD 0
   ddLoadLibraryA DD 0
   ddModuleHandle DD 0
   ddGetProcAddress DD 0
   ddVirtualFree DD 0
_stubEnd:
   dummy DB 0
   
SECTION ".rdata"  ir
   DIRECTIVE IMPORT_DIRECTORY
   dummy0 DD 0FF88FF88H                     ; signature
   DIRECTIVE RECOURCE_DIRECTORY
   dummy1 DD 0FF88FF88H                     ; signature

_compressedFileBegin:   
   DIRECTIVE COMPRESSED_FILE
_compressedFileEnd: 
   dummy2 DD 0FF88FF88H                     ; signature

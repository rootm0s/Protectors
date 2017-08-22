format MS COFF

include 'INCLUDE/win32a.inc'

extrn '__imp__VirtualAlloc@16' as VirtualAlloc:dword
extrn '__imp__VirtualFree@12' as VirtualFree:dword
extrn '__imp__GetModuleHandleA@4' as GetModuleHandleA:dword

public GetBuild as "_GetBuild@0"
public BeginProtect as "_BeginProtect@4"
public EndProtect as "_EndProtect@4"
	
section '.data' data readable writeable
begin_data:
; VM header table
vm_header:
; signatures (used to find the VM)
dd 0xb0fa3d74								; VM signature 1
dd 0xa53bcf98								; VM signature 2
dd 0xee02930e								; VM signature 3
dd 0										; VM major version
dd 2										; VM minor version
; --------------------------------
; File header
; --------------------------------
VM_FILEHEADER:
VM_FILEHEADER_BASE:
dd 0
VM_FILEHEADER_VA_TABLE:
dd 0
VM_FILEHEADER_VA_TABLE_VALUES:
dd 0
; --------------------------------
dd vm_code_start							; VM base
dd vm_code_end-vm_code_start				; VM total size
dd vm_init-vm_code_start					; Init offset
dd vm_fetch_decode-vm_code_start			; Fetch decode offset
dd vm_reenter-vm_code_start					; Reenter offset
dd vm_exit-vm_code_start					; Exit offset
dd vm_virtualalloc_patch_location_1+2-vm_code_start	; VirtualAlloc patch offset 1
dd vm_virtualalloc_patch_location_2+2-vm_code_start	; VirtualAlloc patch offset 2
dd vm_virtualfree_patch_location_1+2-vm_code_start	; VirtualFree patch offset 1
dd vm_virtualfree_patch_location_2+2-vm_code_start	; VirtualFree patch offset 2
dd vm_handlers_patch_instruction+1-vm_code_start	; Handlers patch location
dd vm_reenter_patch_location+1-vm_code_start
dd vm_exit_patch_location+1-vm_code_start
dd vm_exit_native-vm_code_start
dd vm_va_table_patch_location+1-vm_code_start
dd vm_va_table_values_patch_location+1-vm_code_start
dd vm_handlers
dd VM_HANDLER_COUNT
dd BeginProtect-vm_code_start
dd EndProtect-vm_code_start
; End

build_id:
dq 0x1450902833

_code:
dd 0
db 1 ; exit

; ================ Handlers ================
VM_HANDLER_COUNT = 28
vm_handlers:
dd vm_native_dispatch				; 0
dd vm_exit							; 1
dd vm_push							; 2
dd vm_repush
dd vm_push_reg						; 3
dd _push_dword						; 4
dd vm_pop_reg							; 5
dd _push_flag 						; 6
dd _move							; 7
dd _nand							; 8
dd _jump_rel						; 9
dd _jump_abs						; 10
dd _jump_rel_cond					; 11
dd 0 								; popmem, 12
dd _deref 							; deref, 13
dd _add 							; add, 14
dd _sub								; sub, 15
dd _mul 							; mul, 16
dd _xor								; xor, 17
dd _call_rel
dd _nop
dd vm_rebase						;
dd vm_retn
dd vm_pop_remove
dd vm_add
dd vm_sub
dd _push_reg
dd _pop_reg
dd 0
; ========================================

section '.text' code readable executable
text_start:
; VM context structure as pointed to through the context register
CONTEXT_VM_ESP			= 0
CONTEXT_FLAGS			= CONTEXT_VM_ESP		+ 4
CONTEXT_EDI				= CONTEXT_FLAGS			+ 4
CONTEXT_ESI				= CONTEXT_EDI			+ 4
CONTEXT_EBP				= CONTEXT_ESI			+ 4
CONTEXT_ESP				= CONTEXT_EBP			+ 4		
CONTEXT_EBX				= CONTEXT_ESP			+ 4		; do not use, use CONTEXT_NATIVE_ESP instead
CONTEXT_EDX				= CONTEXT_EBX			+ 4
CONTEXT_ECX				= CONTEXT_EDX			+ 4
CONTEXT_EAX				= CONTEXT_ECX			+ 4
CONTEXT_NATIVE_ESP		= CONTEXT_EAX			+ 4
CONTEXT_SCRATCH			= CONTEXT_NATIVE_ESP	+ 4

CONTEXT_SIZE = 0x30

; Stack size
VM_SCRATCH_SIZE	=	0x100
VM_STACK_SIZE	=	0x4000


vm_code_start:
GetBuild:
	mov eax, build_id
EndProtect:
	retn 4
BeginProtect:
	retn 4
vm_init:
; begin_obfuscate
; begin_shuffle
	lea esp, [esp+4]	; throw away the return value from the BeginProtect call
	pushad
	pushfd
	; Allocate scratch space
	push 0x40						; protection: PAGE_EXECUTE_READWRITE
	push 0x1000						; allocation: MEM_COMMIT
	push VM_SCRATCH_SIZE			; allocation size
	push 0							; desired address
vm_virtualalloc_patch_location_1:
	call dword [VirtualAlloc]
	push eax
	
	; Allocate stack
	push 0x04						; protection: PAGE_READWRITE
	push 0x1000						; allocation: MEM_COMMIT
	push VM_STACK_SIZE				; allocation size
	push 0							; desired address
vm_virtualalloc_patch_location_2:
	call dword [VirtualAlloc]

	add eax, VM_STACK_SIZE
	
	pop ebx						; VM scratch 
	
	; Copy flags and registers to VM stack
	pop dword [eax-0x2c]		; registers
	pop dword [eax-0x28]		
	pop dword [eax-0x24]
	pop dword [eax-0x20]
	pop dword [eax-0x1c]
	pop dword [eax-0x18]
	pop dword [eax-0x14]
	pop dword [eax-0x10]
	pop dword [eax-0x0c]		; flags
	
	pop edx						; VM IP
; end_obfuscate
	mov dword [eax-0x08], esp
; begin_obfuscate
	mov dword [eax-0x04], ebx	; VM scratch
	sub eax, CONTEXT_SIZE
	mov dword [eax], eax		; VM ESP
	mov ebx, eax
	mov esp, eax
	
	; Save current base address
	push 0
	call dword [GetModuleHandleA]
	mov dword [VM_FILEHEADER_BASE], eax
	add edx, eax	; add image base to VM IP
	
	jmp vm_fetch_decode
vm_native_dispatch:
	;int3
	mov eax, [ebx+CONTEXT_SCRATCH]
	mov ecx, [edx]
	inc edx
	and ecx, 0xff	; length
	xor esi,esi
	@@:
	cmp ecx,esi
	je @f
	push dword [edx+esi]
	pop dword [eax+esi]
	add esi,4
	jmp @b
	@@:
	add edx, ecx	; increase VM IP
	
	push edx
	push ebx
	
	; Save VM ESP at this state (don't push anything past this point)
; end_obfuscate
	mov dword [ebx+CONTEXT_VM_ESP], esp
; begin_obfuscate
	lea edi, [ebx+CONTEXT_NATIVE_ESP]
	
	; Add mov [<VM context>], esp
	mov dword [eax+esi+0], 0x25899090
	mov dword [eax+esi+4], edi
	
	add esi, 8
	
	; Add mov esp, <VM context> at end of code
	mov dword [eax+esi+0], 0xbc909090
	mov [eax+esi+4], edi
	add esi, 8
	
	; Add jump at end of code
	; (jmp  = vm_reenter - scratch - esi - 5)
	mov dword [eax+esi], 0xe9909090
	add esi, 4
vm_reenter_patch_location:
	mov edi, vm_reenter
	sub edi, eax
	sub edi, esi
	sub edi, 4
	mov dword [eax+esi], edi
	mov ecx, [ebx+CONTEXT_SCRATCH]
	push ecx
vm_native_dispatch_2:
	; Push address onto native stack
	pop ecx
	sub dword [ebx+CONTEXT_NATIVE_ESP], 4
	mov eax, [ebx+CONTEXT_NATIVE_ESP]
	mov [eax], ecx
; end_obfuscate
	lea esp, [ebx+CONTEXT_FLAGS]	; since we want offset 0, otherwise use lea
	popfd
	popad
	pop esp	
	retn
vm_reenter:
	pushad
	pushfd
	sub esp, 4
	pop esp
	pop ebx
	pop edx
	mov dword [ebx+CONTEXT_VM_ESP], esp	; update VM ESP (removing the pushed dispatched address etc.)
	jmp vm_fetch_decode
; begin_obfuscate
vm_exit:
	; Push the VM exit operand onto the native stack 
	sub dword [ebx+CONTEXT_NATIVE_ESP], 12
	mov eax, [ebx+CONTEXT_NATIVE_ESP]
		
	push dword [ebx+CONTEXT_SCRATCH]
	
	mov dword [eax+0], ebx				; move context pointer to native stack
	pop dword [eax+4]					; move scratch pointer to native stack
	pop dword [eax+8]					; vm exit return value?
vm_exit_patch_location:
	push vm_exit_native
	jmp vm_native_dispatch_2
vm_exit_native:
	pushad							;
	pushfd							; subtracts 36 from ESP
	
	push 0x8000						; free type: MEM_RELEASE
	push 0							; memory size
	push dword [esp+36+4+4]			; pointer to memory
vm_virtualfree_patch_location_1:
	call dword [VirtualFree]
	
	push 0x8000						; free type: MEM_RELEASE
	push 0							; memory size
	push dword [esp+40+4+4]			; pointer to memory
vm_virtualfree_patch_location_2:
	call dword [VirtualFree]
	
	popfd
	popad
	
	lea esp, [esp+8]				; remove the two VirtualFree addresses
	
	retn ; jump to vm_exit operand
vm_fetch_decode:
	mov eax, [edx]
	and eax, 0xff
	imul eax, 4
	inc edx
vm_handlers_patch_instruction:
	mov ebp, vm_handlers
	jmp dword [ebp+eax]
vm_int3:	; (used to inspect the native state by dispatching to this as a handler)
	int3
	jmp vm_fetch_decode
vm_retn:
	pop eax
	add eax, 4
	mov ecx, [ebx+CONTEXT_NATIVE_ESP]
	push dword [ecx]
	add dword [ebx+CONTEXT_NATIVE_ESP], eax
	jmp vm_exit
vm_push:
	push dword [edx]
	add edx, 4
	jmp vm_fetch_decode
vm_pop_remove:
	lea esp, [esp+4]
	jmp vm_fetch_decode
_push_dword:
	;int3
	sub dword [ebx+CONTEXT_NATIVE_ESP], 4
	mov eax, [ebx+CONTEXT_NATIVE_ESP]
	pop dword [eax]
	jmp vm_fetch_decode
vm_repush:
	push dword [esp+00]
	jmp vm_fetch_decode
vm_push_reg:
	;int3
	mov eax, [edx]
	inc edx
	and eax, 0xff
	cmp eax, 4
	jne @f
	push dword [ebx+CONTEXT_NATIVE_ESP]		; special case for now
	jmp vm_fetch_decode
	@@:
	neg eax
	shl eax, 2
	push dword [ebx+CONTEXT_EAX+eax]
	jmp vm_fetch_decode
vm_pop_reg:
	;int3
	mov eax, [edx]
	inc edx
	and eax, 0xff
	cmp eax, 4
	jne @f
	pop dword [ebx+CONTEXT_NATIVE_ESP]		; special case for now
	jmp vm_fetch_decode
	@@:
	neg eax
	shl eax, 2
	pop dword [ebx+CONTEXT_EAX+eax]
	jmp vm_fetch_decode
_push_reg:
	;int3
	mov eax, [edx]
	inc edx
	and eax, 0xff
	sub dword [ebx+CONTEXT_NATIVE_ESP], 4
	mov ecx, [ebx+CONTEXT_NATIVE_ESP]
	cmp eax, 4
	jne @f
	push dword [ebx+CONTEXT_NATIVE_ESP]		; special case for now
	pop dword [ecx]
	jmp vm_fetch_decode
	@@:
	neg eax
	shl eax, 2
	push dword [ebx+CONTEXT_EAX+eax]
	pop dword [ecx]
	jmp vm_fetch_decode
_pop_reg:
	;int3
	mov eax, [edx]
	inc edx
	and eax, 0xff
	mov ecx, [ebx+CONTEXT_NATIVE_ESP]
	mov ecx, [ecx]
	cmp eax, 4
	jne @f
	mov [ebx+CONTEXT_NATIVE_ESP], ecx		; special case for now
	add dword [ebx+CONTEXT_NATIVE_ESP], 4
	jmp vm_fetch_decode
	@@:
	neg eax
	shl eax, 2
	mov [ebx+CONTEXT_EAX+eax], ecx
	add dword [ebx+CONTEXT_NATIVE_ESP], 4
	jmp vm_fetch_decode
_move:
	pop eax
	pop dword [eax]
	jmp vm_fetch_decode
_nand:
	pop eax
	pop ecx
	push dword [ebx+CONTEXT_FLAGS]
	popf
	and eax, ecx
	not eax
	and eax, 0xffffffff; hack to update flags according to NOT result (prior state not needed)
	pushf
	pop dword [ebx+CONTEXT_FLAGS]
	push eax
	jmp vm_fetch_decode
_jump_rel_cond:
	pop eax
	cmp eax, 0
	jnz _jump_rel
	add edx, 4
	jmp vm_fetch_decode
_jump_rel:
	;int3
	nop
	nop
	nop
	mov eax, [edx]
	add edx, 4
	add edx, eax
	jmp vm_fetch_decode
_jump_abs:
	pop eax
vm_va_table_patch_location:
	mov ecx, [VM_FILEHEADER_VA_TABLE]
vm_va_table_values_patch_location:
	mov edx, [VM_FILEHEADER_VA_TABLE_VALUES]
	mov esi, ecx
	@@:
	cmp dword [esi], -1
	je no_va_found
	cmp [esi], eax
	je @f
	add esi, 4
	jmp @b
	@@:
	sub esi, ecx
	mov edi, [edx + esi]
	add edi, [VM_FILEHEADER_BASE]
	; and that the jump does not lead out of a virtualized chunk
	mov edx, edi
	jmp vm_fetch_decode
vm_rebase:
	mov eax, [VM_FILEHEADER_BASE]
	add [esp], eax
	jmp vm_fetch_decode
no_va_found:
	mov edx, eax	; use original VA
	jmp vm_fetch_decode
_push_flag:
	mov eax, [edx]
	add edx, 4
	mov ecx, [ebx+CONTEXT_FLAGS]
	and eax, ecx
	push eax
	jmp vm_fetch_decode
_deref:
	pop eax
	push dword [eax]
	jmp vm_fetch_decode
vm_add:
	pop eax
	pop ecx
	add eax, ecx
	push eax
	jmp vm_fetch_decode
vm_sub:
	pop eax
	pop ecx
	sub eax, ecx
	push eax
	jmp vm_fetch_decode
_add:
	pop eax
	pop ecx
	push dword [ebx+CONTEXT_FLAGS]
	popf
	add eax, ecx
	pushf
	pop dword [ebx+CONTEXT_FLAGS]
	push eax
	jmp vm_fetch_decode
_sub:
	pop eax
	pop ecx
	push dword [ebx+CONTEXT_FLAGS]
	popf
	sub eax, ecx
	pushf
	pop dword [ebx+CONTEXT_FLAGS]
	push eax
	jmp vm_fetch_decode
_mul:
	pop eax
	pop ecx
	imul eax, ecx
	push eax
	jmp vm_fetch_decode
_xor:	; no flags...
	pop eax
	pop ecx
	xor eax, ecx
	push eax
	jmp vm_fetch_decode
_call_rel:	; oh dear
	int3
_nop:
	nop
	jmp vm_fetch_decode
; 32 bit Fowler-Noll-Vo hashing
fnv_32:
	mov esi, [esp + 0x04]			; buffer
	mov ecx, [esp + 0x08]			; length
	mov eax, 0x0ce942fa			; basis
	mov edi, 0x01000193			; fnv_32_prime
@@:
   mul edi
   xor al, [esi]
   inc esi
   dec ecx
   jnz @b
   retn 8
vm_code_end:
; end_obfuscate
; end_shuffle
; Lookup table for VAs that have been manually added as jump targets
; These are placeholders only, the actual tables are patched in at runtime
va_table:
va_table_values:

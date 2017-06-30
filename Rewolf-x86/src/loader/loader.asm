; * x86.Virtualizer
; * Copyright 2007 ReWolf
; * Contact:
; * rewolf@rewolf.pl
; * http://rewolf.pl
; *
; * This program is free software; you can redistribute it and/or modify
; * it under the terms of the GNU General Public License as published by
; * the Free Software Foundation; either version 2 of the License, or
; * (at your option) any later version.
; * 
; * This program is distributed in the hope that it will be useful,
; * but WITHOUT ANY WARRANTY; without even the implied warranty of
; * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; * GNU General Public License for more details.
; * 
; * You should have received a copy of the GNU General Public License
; * along with this program; if not, write to the Free Software
; * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
; *


.386
.model flat, stdcall
option casemap:none

include \masm32\include\windows.inc
include \masm32\include\kernel32.inc
include \masm32\include\user32.inc
includelib \masm32\lib\kernel32
includelib \masm32\lib\user32


CF equ 00000001h
PF equ 00000004h
AF equ 00000010h
ZF equ 00000040h
SF equ 00000080h
OF equ 00000800h

storeESP macro where, corr
	push	eax
	call	$+5
	pop	eax
	mov	dword ptr [eax - ($ - offset where - 1)], esp
	add	dword ptr [eax - ($ - offset where - 4) + corr], 4
	pop	eax
endm

storeRealESP macro _dt
	push	eax
	getVar	eax, vm_esp
	lea	eax, dword ptr [eax + _dt + 2Ch]
	mov	dword ptr [eax], esp
	add	dword ptr [eax], 4
	pop	eax
endm

reStoreRealESP macro _dt
        mov	esp, dword ptr [esp + _dt + 28h]
endm

reStoreESP macro _from
	call	$+5
	pop	esp
	mov	esp, dword ptr [esp - ($ - offset _from - 1)]
endm

setVar macro var, val
	push	eax
	call	$+5
	pop	eax
	mov	dword ptr [eax - ($ - offset var - 1)], val
	pop	eax
endm

getVar macro reg, var
	call	$+5
	pop	reg
	mov	reg, dword ptr [reg - ($ - offset var - 1)]
endm

getVarAddr macro reg, var
	call	$+5
	pop	reg
	lea	reg, dword ptr [reg - ($ - offset var - 1)]
endm

.data
        temp	dd 0
        vm_stack dd 0
.code
start:
dd	vm_end_label - _vm_jump		;vm_size
dd	_vm_jump - 401000h		;vm code start (not entry)
dd	_vm_init - 401000h
dd	_vm_start - 401000h
dd	poly - 401000h
dd	offset vm_prefix - 401000h
dd	offset vm_opcode_tab - 401000h
dd	8	;data counter ;p

dd	22		;_prot_rec_cnt
;dd	_vm_jump, _vm_call
dd	_vm_call, _call_003
dd	_vm_init, _vm_init_end
dd	_mark_000, _next_instr
dd	_next_instr, _mark_001
dd	_mark_002, _mark_003
;dd	_mark_003, _mark_004
;dd	_mark_005, _mark_006
dd	_vm_instr, _mark_007
dd	_01_COND_JMP, _mark_008
dd	_03_CALL_REL, _mark_009
dd	_02_JECX, _mark_010
dd	_06_LOOPcc, _mark_011
dd	_04_VM_END, _mark_012
dd	_05_RET, _mark_013
dd	_08_VM_MOV_REG, _mark_014

dd	_20_CALL, _mark_022
dd	_21_JMP, _mark_021
dd	_22_PUSH, _mark_020
dd	_25_FAKE_CALL, _mark_019
dd	_23_POP, _mark_018
dd	_24_RELOC, _mark_017
dd	_0C_VM_REAL, _mark_016
dd	_real_tramp, _mark_015

;dd	poly, _vm_init

dd	_memmov, vm_end_label

;---------------------------------------------------------------------------
;in  edx = jump number
;out edx = 0 - jump not taken; 1 - jump taken
_vm_jump:

push	eax
pushfd
pop	eax


push	ebx
call	$+5
pop	ebx

lea	edx, dword ptr [ebx + edx*2 + 8]
pop	ebx
jmp	edx

jmp	_0x70
jmp	_0x71
jmp	_0x72
jmp	_0x73
jmp	_0x74
jmp	_0x75
jmp	_0x76
jmp	_0x77
jmp	_0x78
jmp	__0x79
jmp	__0x7A
jmp	__0x7B
jmp	__0x7C
jmp	__0x7D
jmp	__0x7E
jmp	__0x7F


_0x70:
test  eax, OF		;A9 00080000
jz   __no_jump
jmp  _jump75

_0x71:
test  eax, OF		;A9 00080000
jnz   __no_jump
jmp   _jump75

_0x72:
test  eax, CF		;A9 01000000
jz    __no_jump
jmp   _jump75

__0x79: jmp  _0x79

_0x73:
test  eax, CF		;A9 01000000
jnz   __no_jump
jmp   _jump75

__0x7A: jmp  _0x7A

_0x74:
test  eax, ZF		;A9 40000000
jz    __no_jump
jmp   _jump75

__0x7B: jmp  _0x7B
__0x70: jmp  _0x70
__0x71: jmp  _0x71

_0x75:
test  eax, ZF		;A9 40000000
jnz   __no_jump
_jump75:
jmp   _jump79

__0x7C: jmp  _0x7C
__0x7F: jmp  ___0x7F

_0x76:
test  eax, CF		;A9 01000000
jz    _0x74
jmp   _jump79

__0x7D: jmp  _0x7D

_0x77:
test  eax, CF		;A9 01000000
jnz    __no_jump
jmp   _0x75

__0x7E: jmp  _0x7E
__no_jump: jmp _no_jump

_0x78:
test  eax, SF		;A9 80000000
jz   _no_jump
jmp  _jump79

___0x7F: jmp  _0x7F

_0x79:
test  eax, SF		;A9 80000000
jnz   _no_jump
_jump79:
jmp  _jump7E

_0x7A:
test  eax, PF		;A9 04000000
jz   _no_jump
jmp  _jump7E

_0x7B:
test  eax, PF		;A9 04000000
jnz   _no_jump
jmp  _jump7E

_0x7C:
test  eax, SF or OF	;A9 80080000
jz    _no_jump
test  eax, SF
jz    __0x70
jmp   __0x71

_0x7D:
test  eax, SF
jnz   __0x70
jmp   __0x71

_0x7E:
test  eax, ZF
jz    _0x7C
_jump7E:
jmp   _jump

_0x7F:
test  eax, ZF
jnz   _no_jump
jmp   _0x7D

_jump:
	xor	edx, edx
	jmp	_jump_real
_no_jump:
	xor	edx, edx
	jmp	_no_jump_real

_jump_real:
	inc	edx

_no_jump_real:
	push	eax
	popfd
	pop	eax
	ret


;---------------------------------------------------------------------------
;call variants	([esp+4] address)
;---------------------------------------
_vm_call:
	;push	eax
	call	$+5
	_call_000 equ $
	pop	eax
	push	dword ptr [esp+4]
	pop	dword ptr [eax+(_call_001 - _call_000 + 1)]
	sub	dword ptr [eax+(_call_001 - _call_000 + 1)], eax
	sub	dword ptr [eax+(_call_001 - _call_000 + 1)], (_call_001 - _call_000 + 5)

	mov	eax, dword ptr [esp]
	;mov	dword ptr [esp+8], eax

	;pop	eax
	add	esp, 8


	pushad
	_call_003 equ $
	storeESP vm_esp, 3		;switching to original application
	;reStoreESP r_esp		;context
	reStoreRealESP 10*4
	mov	dword ptr [esp + 28h], eax
	popfd
	popad
	mov	dword ptr [esp], eax
	pop	eax


	_call_001 equ $
	db	0E9h
	dd	0


;poly enc/dec skeleton
;[esp+4] address
;[esp+8] size
;[esp+C] pos
;---------------------------------------------------------------------------
poly:
push	esi
push	eax
push	ecx
mov	esi, dword ptr [esp + 10h]
xor	ecx, ecx
_loop:
mov	al, byte ptr [esi]


;...
db 60 DUP (90h)	;commands 2-bytes
db 30 DUP (90h)	;junks 3-bytes
;...

xor	eax, dword ptr [esp + 18h]	;!!! temp. disabled (only for tests)
mov	byte ptr [esi], al
inc	ecx
inc	esi
cmp	ecx, dword ptr [esp + 14h]
jnz	_loop
pop	ecx
pop	eax
pop	esi
ret	0Ch

;---------------------------------------------------------------------------
;vm init
;[esp] ret addr
;[esp+4] vm stack buffer (rwx)
;[esp+8] module handle


_vm_init:
	corr	equ 3

	push	ebx
	getVar	ebx, vm_esp
	test	ebx, ebx
	jne	_ss
	mov	ebx, dword ptr [esp+8]
	setVar	vm_esp, ebx
	mov	ebx, dword ptr [esp+0Ch]
	setVar	module_handle, ebx
_ss:	pop	ebx
        ret	8
_vm_init_end	equ $
;---------------------------------------------------------------------------
	r_esp	dd 0
	vm_esp	dd 0
	vm_prefix dd 0FFFFh
	module_handle dd 0
	vm_opcode_tab db 3, 4, 254 DUP (0)
;---------------------------------------------------------------------------
;vm run
;[esp] ret addr
;[esp+4] vm'ed code
;---------------------------------------------------------------------------

_vm_start:
	pushad
	pushfd
	pushfd
	pop	ecx
	;storeESP r_esp, 3

	delta	equ 24h
        mov	eax, esp

	mov	ebx, dword ptr [esp + delta]	;ret addr
	mov	edx, dword ptr [esp + delta + 4];data addr

	reStoreESP vm_esp

	;DWORD	orig eflags
	;BYTE	20h DUP execution buffer
	;DWORD	vm_reg
	;DWORD	original esp ???
_mark_000	equ $
	push	ebp
	mov	ebp, esp
	sub	esp, 2Ch	;24h

	mov	dword ptr [esp + 28h], eax		;setting original esp


	;ebx	ret address (from vm)			(const)
	;esi	vm instr pointer			(*)
	;ecx	instr pos relative to vm'ed code begin	(*)
	;edi	execution buffer poninter		(const)
	;eax	current instr length			(*)
	;edx	current vm instr address		(*)

	mov	dword ptr [esp], ecx		;ecx == original eflags
	xor	ecx, ecx
	lea	edi, dword ptr [esp + 4]

_next_instr:
	mov	esi, edx
	inc	esi
	mov	al, byte ptr [edx]
	xor	al, byte ptr [edx + 1]
	movzx	eax, al


        push	eax
        push	esi
        push	edi
        call	_memmov

        push	ecx		;position
        push	eax		;size
        push	edi		;address
        call	poly


        push	ecx
        getVar	ecx, vm_prefix
        cmp	word ptr [edi], cx
_mark_001	equ $
        je	_vm_instr			;VM instr ->
_mark_002	equ $
        pop	ecx
        ;else	original x86 instr

	;generateing ret jmp (push ret):
	push	ecx
	mov	byte ptr [edi + eax], 68h
        call	$+5
        pop	ecx
        add	ecx, vm_ret - $ + 1
	mov	dword ptr [edi + eax + 1], ecx
	mov	byte ptr [edi + eax + 5], 0C3h
	pop	ecx
_mark_003	equ $

_real_call:
	;vm_call correcting
	setVar	(vm_call + 1), edi			;corr += 3

	pushad
	storeESP vm_esp, 3		;switching to original application
	;reStoreESP r_esp		;context
	reStoreRealESP 20h
	popfd
	popad

	mov	dword ptr [esp + 4], eax
	pop	eax
	pop	eax
	;pushfd
	;add	esp, 8
	;popfd
_mark_004	equ $

vm_call	equ	$			;calling not supported instruction
	;call	edi
	db	68h
	dd	0
	ret
vm_ret	equ	$
_mark_005	equ $
	push	0			;stack correction
	push	0
	pushad
	pushfd
	pushfd                          ;\ preserve eflags
	pop	eax			;/
	;storeESP r_esp, 3		;restoring vm context
	storeRealESP 1Ch					;!!!
	reStoreESP vm_esp
	mov	dword ptr [esp + 20h], eax	;storing new eflags
	popad
_mark_006	equ $

_pre_next_instr:
	add	ecx, eax
	add	edx, eax
	inc	edx
	inc	ecx
	jmp	_next_instr


_vm_instr:
	pop	ecx

	push	eax
	push	edx

	getVarAddr eax, vm_opcode_tab
	movzx	edx, byte ptr [edi + 2]
	movzx	edx, byte ptr [eax + edx]

	;
	call	$+5
	pop	eax					;1
	add	eax, dword ptr [eax + 4*edx + 10]	;4
	add	eax, 10
_mark_007	equ $
	jmp	eax
_vm_dispatch	equ $
dd _01_COND_JMP - _vm_dispatch
dd _01_COND_JMP - _vm_dispatch
dd _02_JECX - _vm_dispatch
dd _03_CALL_REL - _vm_dispatch
dd _04_VM_END - _vm_dispatch
dd _05_RET - _vm_dispatch
dd _06_LOOPcc - _vm_dispatch
dd _07_VM_MOV_IMM - _vm_dispatch
dd _08_VM_MOV_REG - _vm_dispatch
dd _09_VM_ADD_IMM - _vm_dispatch
dd _0A_VM_ADD_REG - _vm_dispatch
dd _0B_VM_SHL_REG - _vm_dispatch
dd _0C_VM_REAL - _vm_dispatch
dd _0D_JMP - _vm_dispatch
dd _0D_JMP - _vm_dispatch
dd _0F_ROL - _vm_dispatch
dd _10_ROR - _vm_dispatch
dd _11_RCL - _vm_dispatch
dd _12_RCR - _vm_dispatch
dd _13_SHL - _vm_dispatch
dd _14_SHR - _vm_dispatch
dd _15_SAR - _vm_dispatch
dd _16_SAL - _vm_dispatch
dd _17_ADD - _vm_dispatch
dd _18_OR - _vm_dispatch
dd _19_ADC - _vm_dispatch
dd _1A_SBB - _vm_dispatch
dd _1B_AND - _vm_dispatch
dd _1C_SUB - _vm_dispatch
dd _1D_XOR - _vm_dispatch
dd _1E_CMP - _vm_dispatch
dd _1F_MOV - _vm_dispatch
dd _20_CALL - _vm_dispatch
dd _21_JMP - _vm_dispatch
dd _22_PUSH - _vm_dispatch
dd _23_POP - _vm_dispatch
dd _24_RELOC - _vm_dispatch
dd _25_FAKE_CALL - _vm_dispatch
dd _26_VM_NOP - _vm_dispatch


_26_VM_NOP:
_vm_instr_end:
        pop	edx
        pop	eax

	jmp	_pre_next_instr




_01_COND_JMP:
	movzx	edx, byte ptr [edi + 3]
	push	dword ptr [esp + 8]
	popfd				;temp restoring original eflags
        call	_vm_jump
        test	edx, edx
_mark_008	equ $
        je	_vm_instr_end
	pop	edx
	add	edx, dword ptr [edi + 4]
	add	ecx, dword ptr [edi + 4]
	pop	eax
	jmp	_next_instr

_0D_JMP:
	pop	edx
	add	edx, dword ptr [edi + 3]
	add	ecx, dword ptr [edi + 3]
	pop	eax
	jmp	_next_instr

_03_CALL_REL:
	mov	edx, dword ptr [edi + 3]
	push	edx
	getVar	edx, module_handle
	add	dword ptr [esp], edx
	call	_vm_call
	push	0
	push	0
	pushad
	pushfd
	pushfd
	pop	eax
	;storeESP r_esp, 3		;restoring vm context
	storeRealESP 24h
_mark_009	equ $
	reStoreESP vm_esp
	mov	dword ptr [esp + 28h], eax
	popad
;_mark_009	equ $
	jmp	_vm_instr_end

_02_JECX:					;TO TEST
	getVar	edx, r_esp
        mov	edx, dword ptr [edx + 1Ch]
        test	edx, edx
_mark_010	equ $
        jnz	_vm_instr_end
	pop	edx
	add	edx, dword ptr [edi + 3]
	add	ecx, dword ptr [edi + 3]
	pop	eax
	jmp	_next_instr


_06_LOOPcc:						;TO TEST
        ;movzx	edx, byte ptr [edi + 3]
        ;cmp	edx, 2
        ;jne	_loopcc_n
	;LOOP
        getVar	eax, r_esp
        mov	edx, dword ptr [eax + 1Ch]
        test	edx, edx
_mark_011	equ $
        jnz	_vm_instr_end
        movzx	edx, byte ptr [edi + 3]
        cmp	edx, 2
        jne	_loopcc_n
_loop_ok:
        pop	edx
        add	edx, dword ptr [edi + 4]
        add	ecx, dword ptr [edi + 4]
        pop	eax
        jmp	_next_instr
_loopcc_n:	;loopz/loopnz
	mov	eax, dword ptr [eax]		;flags
	test	edx, edx
	je	_loopnz
	test	eax, ZF
	jnz	_loop_ok
	jmp	_vm_instr_end
_loopnz:
	test	eax, ZF
	jz	_loop_ok
	jmp	_vm_instr_end

_04_VM_END:
	pop	edx
	pop	eax
	mov	eax, dword ptr [esp + 28h]
	leave
	storeESP vm_esp, 3
	;reStoreESP r_esp
	mov	esp, eax
	mov	dword ptr [esp + 24h], ebx
	popfd
	popad
	ret	4
_mark_012	equ $

_05_RET:
	pop	edx
	pop	eax
	mov	eax, dword ptr [esp + 28h]
	movzx	edx, word ptr [edi + 3]
	setVar	retret, edx
	leave
	storeESP vm_esp, 3
	;reStoreESP r_esp
	mov	esp, eax
	;mov	dword ptr [esp + 24h], ebx
	mov	ebx, dword ptr [esp + 24h + 8]
	mov	dword ptr [esp + 24h + 8 + edx], ebx
	popfd
	popad
	mov	dword ptr [esp + 4], eax
	pop	eax
	pop	eax
_mark_013	equ $
	;ret	4
	db	0C2h
retret	dd	0

_07_VM_MOV_IMM:
	mov	eax, dword ptr [edi + 3]
	mov	dword ptr [esp + 24h + 8], eax
	jmp	_vm_instr_end
_08_VM_MOV_REG:
	movzx	eax, byte ptr [edi + 3]
	mov	edx, dword ptr [esp + 28h + 8]		;get real esp
	shl	eax, 2
	sub	eax, 20h
	neg	eax
	mov	edx, dword ptr [edx + eax]
	;
	cmp	eax, 10h	;ESP
	jnz	_08_n
	add	edx, 8
	_08_n:
	;
	mov	dword ptr [esp + 24h + 8], edx
_mark_014	equ $
        jmp     _vm_instr_end
_09_VM_ADD_IMM:
	mov	eax, dword ptr [edi + 3]
	add	dword ptr [esp + 24h + 8], eax
        jmp     _vm_instr_end
_0A_VM_ADD_REG:
	movzx	eax, byte ptr [edi + 3]
	mov	edx, dword ptr [esp + 28h + 8]		;get real esp
	shl	eax, 2
	sub	eax, 20h
	neg	eax
	mov	edx, dword ptr [edx + eax]
	;
	cmp	eax, 10h	;ESP
	jnz	_0A_n
	add	edx, 8
	_0A_n:
	;
	add	dword ptr [esp + 24h + 8], edx
        jmp     _vm_instr_end
_0B_VM_SHL_REG:
	mov	eax, ecx
	movzx	ecx, byte ptr [edi + 3]
	shl	dword ptr [esp + 24h + 8], cl
	mov	ecx, eax
        jmp     _vm_instr_end

SHIFTS macro itr
	mov	eax, ecx
	movzx	ecx, byte ptr [edi + 3]
	mov	edx, dword ptr [esp + 24h + 8]	;address from vm_reg
	pushfd
        push	dword ptr [edi - 4]		;eflags
        popfd
	itr	dword ptr [edx], cl
	pushfd
	pop	dword ptr [edi - 4]		;eflags
	popfd
	mov	ecx, eax
        jmp     _vm_instr_end
endm

_0F_ROL: SHIFTS	rol
_10_ROR: SHIFTS	ror
_11_RCL: SHIFTS	rcl
_12_RCR: SHIFTS	rcr
_13_SHL: SHIFTS	shl
_14_SHR: SHIFTS	shr
_15_SAR: SHIFTS	sar
_16_SAL: SHIFTS	sal

IMM32 macro itr
	;mov	eax, ecx
	mov	eax, dword ptr [edi + 3]
	mov	edx, dword ptr [esp + 24h + 8]	;address from vm_reg
	pushfd
        push	dword ptr [edi - 4]		;eflags
        popfd
	itr	dword ptr [edx], eax
	pushfd
	pop	dword ptr [edi - 4]		;eflags
	popfd
	;mov	ecx, eax
        jmp     _vm_instr_end
endm

_17_ADD: IMM32	add
_18_OR:  IMM32	or
_19_ADC: IMM32	adc
_1A_SBB: IMM32	sbb
_1B_AND: IMM32	and
_1C_SUB: IMM32	sub
_1D_XOR: IMM32	xor
_1E_CMP: IMM32	cmp
_1F_MOV: IMM32	mov

_20_CALL:
	mov	edx, dword ptr [esp + 24h + 8]
	mov	byte ptr [edi], 68h
	lea	eax, dword ptr [edi + 0Bh]
	mov	dword ptr [edi + 1], eax
	mov	byte ptr [edi + 5], 68h
	mov	edx, dword ptr [edx]
	mov	dword ptr [edi + 6], edx
	mov	byte ptr [edi + 0Ah], 0C3h
	mov	eax, 0Bh
_mark_022	equ $
	jmp	_real_tramp

_21_JMP:
	mov	edx, dword ptr [esp + 24h + 8]
	mov	byte ptr [edi], 68h
	mov	edx, dword ptr [edx]
	mov	dword ptr [edi + 1], edx
	mov	byte ptr [edi + 5], 0C3h
_mark_021	equ $
	jmp	_real_tramp


_22_PUSH:
	mov	edx, dword ptr [esp + 24h + 8]
	mov	byte ptr [edi], 68h
	mov	edx, dword ptr [edx]
	mov	dword ptr [edi + 1], edx
	mov	eax, 5
_mark_020	equ $
	jmp	_real_tramp

_25_FAKE_CALL:
	mov	edx, dword ptr [edi + 3]
	mov	byte ptr [edi], 68h
	getVar	eax, module_handle
	add	edx, eax
	mov	dword ptr [edi + 1], edx
	mov	eax, 5
_mark_019	equ $
	jmp	_real_tramp

_23_POP:
	mov	edx, dword ptr [esp + 24h + 8]
	mov	word ptr [edi], 058Fh
	mov	dword ptr [edi + 2], edx
	mov	eax, 6
_mark_018	equ $
	jmp	_real_tramp

_24_RELOC:
	movzx	eax, byte ptr [edi + 4]
	push	eax
	lea	edx, dword ptr [edi + 5]
	push	edx
        movzx	edx, byte ptr [edi + 3]
	push	edi
	call	_memmov
	push	eax
	mov	eax, edx
	getVar	edx, module_handle
	add	dword ptr [edi + eax], edx
	pop	eax
_mark_017	equ $
        jmp	_real_tramp


_0C_VM_REAL:
	mov	al, byte ptr [edi + 3]		;real opcode
	mov	ah, byte ptr [edi + 4]		;register
	shl	ah, 3
	or	ah, 5

	mov	word ptr [edi], ax
	push	dword ptr [esp + 24h + 8]
	pop	dword ptr [edi + 2]
	mov	eax, 6
_mark_016	equ	$

_real_tramp:
	;generateing ret jmp (push ret):
	push	ecx
	mov	byte ptr [edi + eax], 68h
        call	$+5
        pop	ecx
        add	ecx, vm_ret - $ + 1
	mov	dword ptr [edi + eax + 1], ecx
	mov	byte ptr [edi + eax + 5], 0C3h
	pop	ecx

	pop	edx
	pop	eax
_mark_015	equ $
	jmp	_real_call


;push	length
;push	offset Source
;push	offset Destination
;call	_memmov
_memmov:
	pushad
	mov	ecx,dword ptr [esp+11*4]
	mov	esi,dword ptr [esp+10*4]
	mov	edi,dword ptr [esp+9*4]
	rep	movsb
	popad
	ret	0Ch
vm_end_label	equ $
end start



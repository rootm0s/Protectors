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
; begin_shuffle
jmp lbl_2a446f0d_5290_4b56_8fd2_5696af059826

lbl_dc9f7f7b_96ee_405e_a536_14558dc39b2a:

push esi
sub dword [ebx++CONTEXT_NATIVE_ESP], 4
push 803
mov eax, [ebx++CONTEXT_NATIVE_ESP]
lea esp, [esp+12]	; increment junk SP
push 5988
mov [eax], ecx
lea esp, [esp+56]	; reset
lea esp, [ebx+CONTEXT_FLAGS]	; since we want offset 0, otherwise use lea
popfd
jmp lbl_83a04d58_c5c8_4eae_851a_29bf4845a173

lbl_d27a8aca_560d_4388_8748_db3299b2f1a2:

_call_rel:	;control flow -> resetting SP
int3 
lea esp, [esp+0]	; reset
_nop:	;control flow -> resetting SP
nop 
lea esp, [esp+-44]	; new junk SP
push ebx
lea esp, [esp+48]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-36]	; new junk SP
jmp lbl_c51338c6_c6e9_4a0c_a679_62ece01480c7

lbl_8a3fda83_3d7a_491f_8e37_133537ebeabe:

shl eax, 2
lea esp, [esp+52]	; reset
push dword [ebx+eax+CONTEXT_EAX]
lea esp, [esp+-40]	; new junk SP
lea esp, [esp+40]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-56]	; new junk SP
lea esp, [esp+56]	; reset
vm_pop_reg:	;control flow -> resetting SP
; int3
jmp lbl_64ebe533_b05a_48e5_aacf_8077a13d17e1

lbl_310dea17_2262_4229_9f1b_23ab7c5e688e:

jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
lbl_c64ec0c1_0325_4e6e_8be5_985de65e5c59:	;control flow -> resetting SP
lea esp, [esp+-36]	; new junk SP
push ecx
neg eax
lea esp, [esp+-8]	; decrement junk SP
push 7660
shl eax, 2
push dword [esp+52]
jmp lbl_18048634_d2c8_4553_9f73_d23c1d28455f

lbl_d3e48e63_5c5c_4b78_a8b0_1f6301699d38:

mov dword [esp+36], eax
lea esp, [esp+20]	; increment junk SP
push 1154
lea esp, [esp+20]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-36]	; new junk SP
push 751
lea esp, [esp+40]	; reset
_jump_rel_cond:	;control flow -> resetting SP
pop eax
jmp lbl_ad148190_7e37_4577_b49d_0daf2c33a1dc

lbl_68447c21_9141_40f1_a4fd_322c70d7d295:

lea esp, [esp+-60]	; new junk SP
add edi, [+VM_FILEHEADER_BASE]
; and that the jump does not lead out of a virtualized chunk
mov edx, edi
lea esp, [esp+60]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-44]	; new junk SP
lea esp, [esp+44]	; reset
vm_rebase:	;control flow -> resetting SP
lea esp, [esp+-64]	; new junk SP
jmp lbl_8f6ca9af_5127_4797_b164_e4fcbc766456

lbl_0986d64a_cdb0_4e11_aa87_3c01f6345310:

push dword [ebx++CONTEXT_FLAGS]
lea esp, [esp+0]	; reset
popf 	;control flow -> resetting SP
lea esp, [esp+-64]	; new junk SP
push 3680
add eax, ecx
lea esp, [esp+68]	; reset
pushf 	;control flow -> resetting SP
lea esp, [esp+-56]	; new junk SP
push dword [esp+56]
jmp lbl_8c126541_a9b0_4e87_b87b_d8dd87fbff40

lbl_6ffdc82f_4b0c_4ed0_b622_9587bcdd6c24:

call dword [+VirtualAlloc]	;control flow -> resetting SP
lea esp, [esp+-36]	; new junk SP
push ecx
mov dword [esp+36], eax
push 7550
; Allocate stack
lea esp, [esp+32]	; increment junk SP
mov dword [esp+4], 0x04
push edi
mov dword [esp+4], 0x1000
jmp lbl_9d595953_a8d9_4e9d_bda4_8c13c9a67371

lbl_abc217b3_fb47_4802_bc0c_2291ff43b424:

push 6209
lea esp, [esp+40]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
vm_repush:	;control flow -> resetting SP
lea esp, [esp+-48]	; new junk SP
lea esp, [esp+48]	; reset
push dword [esp++00]
lea esp, [esp+-56]	; new junk SP
lea esp, [esp+56]	; reset
jmp lbl_27c19fdc_174b_41d0_b116_903fdb2827d6

lbl_f577dd3f_9653_45f5_abd4_3d9178988ce1:

lea esp, [esp+-60]	; new junk SP
mov eax, [esp+60]
push dword [esp+64]
pop dword [eax]
push 1543
lea esp, [esp+72]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-48]	; new junk SP
lea esp, [esp+48]	; reset
_nand:	;control flow -> resetting SP
jmp lbl_c7b010ed_77f1_4b59_9f05_e4e77c8561eb

lbl_9376ae38_32e2_4b88_83f8_d2339fd87709:

lea esp, [esp+-48]	; new junk SP
lea esp, [esp+48]	; reset
lbl_e78feab4_f082_4605_8a20_67a49ff579d4:	;control flow -> resetting SP
lea esp, [esp+-40]	; new junk SP
push 6264
neg eax
lea esp, [esp+36]	; increment junk SP
push esi
shl eax, 2
mov [ebx+eax+CONTEXT_EAX], ecx
jmp lbl_7ad23270_5202_4250_8203_d8bcdf3a9ecc

lbl_e7bebe28_d6e3_4016_bdd9_b2d666a93b03:

push 8153
add eax, VM_STACK_SIZE
lea esp, [esp+40]	; increment junk SP
mov ebx, [esp+16]
push ebx
; Copy flags and registers to VM stack
push dword [esp+24]
pop dword [eax++0x2c]	;registers
lea esp, [esp+24]	; increment junk SP
push dword [esp+4]
jmp lbl_3fcea45c_b7c7_45af_ab88_de15c98d470a

lbl_01f1c837_0bbb_408d_984b_58c8e9401004:

mov dword [eax++0x04], ebx	;VM scratch
sub eax, CONTEXT_SIZE
mov dword [eax], eax	;VM ESP
mov ebx, eax
mov esp, eax
; Save current base address
lea esp, [esp+-52]	; new junk SP
mov dword [esp+48], 0
lea esp, [esp+48]	; reset
call dword [+GetModuleHandleA]	;control flow -> resetting SP
jmp lbl_28924539_afee_4f71_97c4_cf004ac35820

lbl_6fa600a0_6804_4aa7_bb7d_48f3812517bb:

push esi
mov eax, [ebx++CONTEXT_SCRATCH]
lea esp, [esp+-28]	; decrement junk SP
mov ecx, [edx]
push 2856
inc edx
lea esp, [esp+32]	; increment junk SP
push edx
and ecx, 0xff	;length
xor esi, esi
jmp lbl_bbbce1bd_a3db_4c08_a31c_532a8980b604

lbl_283581fd_7d8b_4f64_bf95_463c54ef3f0c:

and eax, 0xff
lea esp, [esp+-64]	; new junk SP
push 7189
sub dword [ebx++CONTEXT_NATIVE_ESP], 4
push 2793
mov ecx, [ebx++CONTEXT_NATIVE_ESP]
lea esp, [esp+-8]	; decrement junk SP
push 3507
cmp eax, 4
lea esp, [esp+-20]	; decrement junk SP
jmp lbl_02288d6e_8241_4dc4_a579_038b4adcc913

lbl_daef9012_aa83_42aa_add3_b0087223bb50:

_mul:	;control flow -> resetting SP
pop eax
lea esp, [esp+-48]	; new junk SP
push 2714
mov ecx, [esp+52]
imul eax, ecx
lea esp, [esp+0]	; increment junk SP
push ebx
mov dword [esp+56], eax
lea esp, [esp+56]	; reset
jmp lbl_1d73c25a_3a35_47bc_8f0e_596659da6678

lbl_d034b97c_2db0_416a_b4d7_ba3ccd79d750:

mov ecx, [ebx++CONTEXT_FLAGS]
push ebx
and eax, ecx
push 7763
mov dword [esp+44], eax
lea esp, [esp+44]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
_deref:	;control flow -> resetting SP
pop eax
jmp lbl_748c7fdf_a7ba_467c_9ec7_0df69dfd69f0

lbl_72d1523d_c1f5_4ef0_81b1_4e62df0f6479:

nop 
nop 
nop 
mov eax, [edx]
push esi
add edx, 4
push 409
add edx, eax
lea esp, [esp+72]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
jmp lbl_cdbf58e7_2f82_4d0f_96b0_9101f156ad0a

lbl_652506df_d790_4e1a_b338_41dc55fa2837:

push edx
mov eax, [esp+48]
mov ecx, [esp+52]
add eax, ecx
lea esp, [esp+-32]	; decrement junk SP
push 5901
mov dword [esp+88], eax
push 1871
lea esp, [esp+92]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
jmp lbl_cd207c0e_e74f_4cf4_b8fb_582a463f4e05

lbl_dc522ecc_e6b8_4b32_8a8b_c73ada0d7e97:

xor eax, ecx
lea esp, [esp+8]	; increment junk SP
push edi
mov dword [esp+40], eax
lea esp, [esp+-4]	; decrement junk SP
lea esp, [esp+44]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-60]	; new junk SP
push eax
lea esp, [esp+64]	; reset
jmp lbl_d27a8aca_560d_4388_8748_db3299b2f1a2

lbl_4c4d6a3d_3709_4999_aa7f_ba7f0c794a2c:

lea esp, [esp+52]	; reset
push dword [esp++40+4+4]	;pointer to memory
lea esp, [esp+-40]	; new junk SP
push ebp
lea esp, [esp+44]	; reset
vm_virtualfree_patch_location_2:	;control flow -> resetting SP
lea esp, [esp+0]	; reset
call dword [+VirtualFree]	;control flow -> resetting SP
lea esp, [esp+-48]	; new junk SP
push edx
jmp lbl_c82581a4_4235_48e3_9c66_f4a8356f3dc3

lbl_cd207c0e_e74f_4cf4_b8fb_582a463f4e05:

lea esp, [esp+-40]	; new junk SP
lea esp, [esp+40]	; reset
vm_sub:	;control flow -> resetting SP
pop eax
pop ecx
sub eax, ecx
lea esp, [esp+-60]	; new junk SP
push 247
mov dword [esp+60], eax
lea esp, [esp+4]	; increment junk SP
jmp lbl_3d9ac79d_dcce_4e39_95bf_794f94edc7e2

lbl_d31a5404_9af1_4d75_91ae_2853c3d61d2c:

pop dword [eax++0x14]
push 4927
push dword [esp+32]
pop dword [eax++0x10]
lea esp, [esp+-4]	; decrement junk SP
push dword [esp+40]
pop dword [eax++0x0c]	;flags
mov edx, [esp+44]
lea esp, [esp+48]	; reset
mov dword [eax-0x08], esp
jmp lbl_01f1c837_0bbb_408d_984b_58c8e9401004

lbl_e720a360_a3e0_4338_a131_6d41157f1184:

push edi
; Add mov esp, <VM context> at end of code
push 1795
mov dword [eax+esi+0], 0xbc909090
lea esp, [esp+-24]	; decrement junk SP
mov [eax+esi+4], edi
add esi, 8
; Add jump at end of code
push 5801
; (jmp  = vm_reenter - scratch - esi - 5)
jmp lbl_eceac388_c7b6_4604_a0eb_d79e6e88c32f

lbl_4aeedcae_ab05_4aa2_ae64_96a0a47a032c:

and eax, ecx
lea esp, [esp+-52]	; new junk SP
not eax
and eax, 0xffffffff	;hack to update flags according to NOT result (prior state not needed)
lea esp, [esp+52]	; reset
pushf 	;control flow -> resetting SP
lea esp, [esp+-32]	; new junk SP
push esp
push dword [esp+36]
pop dword [ebx++CONTEXT_FLAGS]
jmp lbl_d3e48e63_5c5c_4b78_a8b0_1f6301699d38

lbl_0818c88d_29de_4d05_935c_bc588e481a8e:

push 2706
add esi, 4
lea esp, [esp+44]	; reset
jmp lbl_391d6e73_00a5_422a_99ca_2b8eef0c52e3	;control flow -> resetting SP
lea esp, [esp+-60]	; new junk SP
push 3433
lea esp, [esp+64]	; reset
lbl_6b3e907e_121d_4476_b504_581199cf26ec:	;control flow -> resetting SP
sub esi, ecx
mov edi, [edx+esi]
jmp lbl_68447c21_9141_40f1_a4fd_322c70d7d295

lbl_9d595953_a8d9_4e9d_bda4_8c13c9a67371:

lea esp, [esp+0]	; decrement junk SP
mov dword [esp+0], VM_STACK_SIZE
lea esp, [esp+-48]	; new junk SP
push ebp
mov dword [esp+48], 0
lea esp, [esp+48]	; reset
vm_virtualalloc_patch_location_2:	;control flow -> resetting SP
lea esp, [esp+0]	; reset
call dword [+VirtualAlloc]	;control flow -> resetting SP
lea esp, [esp+-52]	; new junk SP
jmp lbl_e7bebe28_d6e3_4016_bdd9_b2d666a93b03

lbl_b4cfd62e_61e4_4635_8bc8_a112fed0389d:

add eax, 4
mov ecx, [ebx++CONTEXT_NATIVE_ESP]
lea esp, [esp+-44]	; new junk SP
push 4961
lea esp, [esp+48]	; reset
push dword [ecx]
add dword [ebx++CONTEXT_NATIVE_ESP], eax
lea esp, [esp+-52]	; new junk SP
push edx
lea esp, [esp+56]	; reset
jmp lbl_3882c3d5_fc07_4d6f_9671_6ca9b58db7e9

lbl_b2db32a5_30ad_40f8_a247_cf76ebfae33f:

lea esp, [esp+-36]	; new junk SP
mov ecx, [esp+36]
lea esp, [esp+40]	; reset
push dword [ebx++CONTEXT_FLAGS]
lea esp, [esp+0]	; reset
popf 	;control flow -> resetting SP
sub eax, ecx
lea esp, [esp+-60]	; new junk SP
lea esp, [esp+60]	; reset
pushf 	;control flow -> resetting SP
jmp lbl_f148519a_11c1_4b04_b344_417dfe6159ef

lbl_18048634_d2c8_4553_9f73_d23c1d28455f:

pop dword [ebx+eax+CONTEXT_EAX]
lea esp, [esp+40]	; increment junk SP
push eax
lea esp, [esp+20]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
_push_reg:	;control flow -> resetting SP
; int3
mov eax, [edx]
inc edx
jmp lbl_283581fd_7d8b_4f64_bf95_463c54ef3f0c

lbl_35325125_f973_48db_8d54_7052adc93e52:

lea esp, [esp+36]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
_push_dword:	;control flow -> resetting SP
; int3
sub dword [ebx++CONTEXT_NATIVE_ESP], 4
lea esp, [esp+-32]	; new junk SP
mov eax, [ebx++CONTEXT_NATIVE_ESP]
push dword [esp+32]
pop dword [eax]
jmp lbl_abc217b3_fb47_4802_bc0c_2291ff43b424

lbl_83a04d58_c5c8_4eae_851a_29bf4845a173:

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
jmp lbl_e228a2ab_e9fc_40fe_8bf8_40df69793791

lbl_c7b010ed_77f1_4b59_9f05_e4e77c8561eb:

lea esp, [esp+-44]	; new junk SP
push eax
mov eax, [esp+48]
mov ecx, [esp+52]
push edi
lea esp, [esp+60]	; reset
push dword [ebx++CONTEXT_FLAGS]
lea esp, [esp+-52]	; new junk SP
lea esp, [esp+52]	; reset
popf 	;control flow -> resetting SP
jmp lbl_4aeedcae_ab05_4aa2_ae64_96a0a47a032c

lbl_490407f5_121b_492c_9029_411998c2648b:

push 302
lea esp, [esp+48]	; reset
lbl_f46ee2ae_b72a_4138_891b_3b47467a78c9:	;control flow -> resetting SP
add edx, ecx	;increase VM IP
lea esp, [esp+-64]	; new junk SP
push 1239
mov dword [esp+64], edx
lea esp, [esp+24]	; increment junk SP
push esi
mov dword [esp+40], ebx
jmp lbl_88466859_41a7_4db0_8759_4ac4f7027479

lbl_d1ba9bff_36a0_4e0f_9553_2168b3fa088c:

lea esp, [esp+32]	; increment junk SP
push edi
mov eax, [edx]
lea esp, [esp+-28]	; decrement junk SP
inc edx
push 6529
and eax, 0xff
push esp
mov ecx, [ebx++CONTEXT_NATIVE_ESP]
push eax
jmp lbl_fc76fc56_683c_4419_b8c7_4fc3ee8aff02

lbl_65a57896_cc5e_4fec_8a21_52822514c323:

mov eax, [edx]
lea esp, [esp+-12]	; decrement junk SP
inc edx
and eax, 0xff
push 2356
cmp eax, 4
lea esp, [esp+-16]	; decrement junk SP
push 6809
lea esp, [esp+52]	; reset
jne lbl_fe1e5aa0_5781_4a05_a514_3e20f155a315	;control flow -> resetting SP
jmp lbl_084ac380_2e05_476b_813e_8c59fbfd0b9a

lbl_78d6c8a9_05bb_45aa_88da_87fca7034763:

pop dword [ecx]
push eax
lea esp, [esp+68]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-44]	; new junk SP
lea esp, [esp+44]	; reset
_pop_reg:	;control flow -> resetting SP
lea esp, [esp+-56]	; new junk SP
push 7994
; int3
jmp lbl_d1ba9bff_36a0_4e0f_9553_2168b3fa088c

lbl_e8053b5e_5d4e_4ac6_a991_73b7f9d9637d:

lea esp, [esp+-40]	; new junk SP
push 3015
int3 
lea esp, [esp+44]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-64]	; new junk SP
push 739
lea esp, [esp+68]	; reset
vm_retn:	;control flow -> resetting SP
pop eax
jmp lbl_b4cfd62e_61e4_4635_8bc8_a112fed0389d

lbl_7ad23270_5202_4250_8203_d8bcdf3a9ecc:

lea esp, [esp+8]	; increment junk SP
push eax
add dword [ebx++CONTEXT_NATIVE_ESP], 4
lea esp, [esp+0]	; decrement junk SP
lea esp, [esp+8]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-32]	; new junk SP
push edx
lea esp, [esp+36]	; reset
_move:	;control flow -> resetting SP
jmp lbl_f577dd3f_9653_45f5_abd4_3d9178988ce1

lbl_1d73c25a_3a35_47bc_8f0e_596659da6678:

jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-56]	; new junk SP
push edx
lea esp, [esp+60]	; reset
_xor:	;control flow -> resetting SP
lea esp, [esp+-32]	; new junk SP
push esi
mov eax, [esp+36]
push 5130
mov ecx, [esp+44]
jmp lbl_dc522ecc_e6b8_4b32_8a8b_c73ada0d7e97

lbl_aa64cc4a_a3ae_4bc6_90cc_8fc348a94b7c:

lea esp, [esp+-28]	; decrement junk SP
push 7288
lea esp, [esp+80]	; reset
vm_handlers_patch_instruction:	;control flow -> resetting SP
mov ebp, vm_handlers
lea esp, [esp+0]	; reset
jmp dword [ebp+eax]	;control flow -> resetting SP
lea esp, [esp+-56]	; new junk SP
lea esp, [esp+56]	; reset
vm_int3:	;control flow -> resetting SP
jmp lbl_e8053b5e_5d4e_4ac6_a991_73b7f9d9637d

lbl_b7471e70_b193_4dcb_b604_cfdcea851fd3:

pushfd 	;control flow -> resetting SP
lea esp, [esp+-36]	; new junk SP
push esi
; Allocate scratch space
lea esp, [esp+-20]	; decrement junk SP
mov dword [esp+56], 0x40
push edi
mov dword [esp+56], 0x1000
lea esp, [esp+-48]	; decrement junk SP
push ebp
jmp lbl_760caa0a_0c60_4e4c_8f4d_3cf1691bd120

lbl_1e9f5133_3483_41c3_935f_f06700fb3b6c:

push 1638
lea esp, [esp+72]	; reset
jne lbl_c64ec0c1_0325_4e6e_8be5_985de65e5c59	;control flow -> resetting SP
lea esp, [esp+-40]	; new junk SP
push 2405
push dword [esp+44]
pop dword [ebx++CONTEXT_NATIVE_ESP]	;special case for now
lea esp, [esp+-28]	; decrement junk SP
push ebp
lea esp, [esp+80]	; reset
jmp lbl_310dea17_2262_4229_9f1b_23ab7c5e688e

lbl_551bc3ce_f5fc_4506_8620_dc125dd326d9:

jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-36]	; new junk SP
push 955
lea esp, [esp+40]	; reset
vm_pop_remove:	;control flow -> resetting SP
lea esp, [esp+-44]	; new junk SP
push 6738
lea esp, [esp+48]	; reset
lea esp, [esp++4]
lea esp, [esp+-36]	; new junk SP
jmp lbl_35325125_f973_48db_8d54_7052adc93e52

lbl_02288d6e_8241_4dc4_a579_038b4adcc913:

lea esp, [esp+104]	; reset
jne lbl_7c5c7bd4_6cfa_4c1d_bcc5_7413c19c05fc	;control flow -> resetting SP
push dword [ebx++CONTEXT_NATIVE_ESP]	;special case for now
pop dword [ecx]
lea esp, [esp+-60]	; new junk SP
lea esp, [esp+60]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-36]	; new junk SP
push 3278
lea esp, [esp+40]	; reset
jmp lbl_8064d5d6_2036_403e_9e78_e42b92aa4048

lbl_218a9c20_4188_4360_aeaa_1e7bd48544a8:

lea esp, [esp+-92]	; decrement junk SP
mov dword [esp+216], ecx
push 3841
lea esp, [esp+220]	; reset
vm_native_dispatch_2:	;control flow -> resetting SP
; Push address onto native stack
lea esp, [esp+-40]	; new junk SP
push 4288
mov ecx, [esp+44]
lea esp, [esp+-8]	; decrement junk SP
jmp lbl_dc9f7f7b_96ee_405e_a536_14558dc39b2a

lbl_3227dd4a_a430_49ac_a640_7c0a300312db:

lea esp, [esp+0]	; reset
lbl_a1b97a5e_275a_45ba_b306_9b6eb4303421:	;control flow -> resetting SP
mul edi
lea esp, [esp+-60]	; new junk SP
push 2171
xor al, [esi]
inc esi
dec ecx
lea esp, [esp+64]	; reset
jnz lbl_a1b97a5e_275a_45ba_b306_9b6eb4303421	;control flow -> resetting SP
jmp lbl_0fb57007_bce2_4b08_87d4_ae9839973bb6

lbl_67bdaa4d_c95f_4061_8d48_dc71b333bca9:

lea esp, [esp+-40]	; new junk SP
push 3694
push dword [esp+44]
pop dword [eax+esi]
push esi
add esi, 4
push esp
lea esp, [esp+56]	; reset
jmp lbl_95eca7d8_6b5b_4f5a_9d54_ba34e3578126	;control flow -> resetting SP
lea esp, [esp+-44]	; new junk SP
jmp lbl_490407f5_121b_492c_9029_411998c2648b

lbl_2a446f0d_5290_4b56_8fd2_5696af059826:

lea esp, [esp+-40]	; new junk SP
lea esp, [esp+40]	; reset
lea esp, [esp++4]	;throw away the return value from the BeginProtect call
lea esp, [esp+-56]	; new junk SP
push 1804
lea esp, [esp+60]	; reset
pushad 	;control flow -> resetting SP
lea esp, [esp+-40]	; new junk SP
push ebx
lea esp, [esp+44]	; reset
jmp lbl_b7471e70_b193_4dcb_b604_cfdcea851fd3

lbl_748c7fdf_a7ba_467c_9ec7_0df69dfd69f0:

lea esp, [esp+-36]	; new junk SP
push esi
lea esp, [esp+40]	; reset
push dword [eax]
lea esp, [esp+-52]	; new junk SP
lea esp, [esp+52]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
vm_add:	;control flow -> resetting SP
lea esp, [esp+-44]	; new junk SP
jmp lbl_652506df_d790_4e1a_b338_41dc55fa2837

lbl_ad148190_7e37_4577_b49d_0daf2c33a1dc:

cmp eax, 0
lea esp, [esp+0]	; reset
jnz _jump_rel	;control flow -> resetting SP
add edx, 4
lea esp, [esp+0]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
_jump_rel:	;control flow -> resetting SP
lea esp, [esp+-64]	; new junk SP
; int3
jmp lbl_72d1523d_c1f5_4ef0_81b1_4e62df0f6479

lbl_5639e86b_851c_4b83_a4be_7e82d7eedb12:

push dword [esp++36+4+4]	;pointer to memory
lea esp, [esp+0]	; reset
vm_virtualfree_patch_location_1:	;control flow -> resetting SP
lea esp, [esp+-36]	; new junk SP
lea esp, [esp+36]	; reset
call dword [+VirtualFree]	;control flow -> resetting SP
push 0x8000	;free type: MEM_RELEASE
lea esp, [esp+-52]	; new junk SP
mov dword [esp+48], 0
push 5155
jmp lbl_4c4d6a3d_3709_4999_aa7f_ba7f0c794a2c

lbl_8f6ca9af_5127_4797_b164_e4fcbc766456:

mov eax, [+VM_FILEHEADER_BASE]
add [esp], eax
push 4259
lea esp, [esp+68]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
no_va_found:	;control flow -> resetting SP
lea esp, [esp+-40]	; new junk SP
push ecx
mov edx, eax	;use original VA
jmp lbl_7a5dc999_dc22_4283_a210_775112b0a723

lbl_910eec8d_a63a_47c2_b130_7dcc7e0f2fc7:

pushad 	;control flow -> resetting SP
lea esp, [esp+-40]	; new junk SP
lea esp, [esp+40]	; reset
pushfd 	;control flow -> resetting SP
push 0x8000	;free type: MEM_RELEASE
lea esp, [esp+-44]	; new junk SP
push 5979
mov dword [esp+44], 0
push edi
lea esp, [esp+48]	; reset
jmp lbl_5639e86b_851c_4b83_a4be_7e82d7eedb12

lbl_e228a2ab_e9fc_40fe_8bf8_40df69793791:

mov dword [ebx+CONTEXT_VM_ESP], esp	; update VM ESP (removing the pushed dispatched address etc.)
jmp vm_fetch_decode
lea esp, [esp+0]	; reset
vm_exit:	;control flow -> resetting SP
lea esp, [esp+-48]	; new junk SP
; Push the VM exit operand onto the native stack
sub dword [ebx++CONTEXT_NATIVE_ESP], 12
mov eax, [ebx++CONTEXT_NATIVE_ESP]
lea esp, [esp+48]	; reset
push dword [ebx++CONTEXT_SCRATCH]
jmp lbl_1553c355_b7fd_4c55_9726_ac77ed765caa

lea esp, [esp+-36]	; new junk SP
lea esp, [esp+36]	; reset
retn 8	;control flow -> resetting SP
lea esp, [esp+0]	; reset
vm_code_end:	;control flow -> resetting SP
lea esp, [esp+0]	; reset
lbl_eceac388_c7b6_4604_a0eb_d79e6e88c32f:

lea esp, [esp+-84]	; decrement junk SP
push edx
mov dword [eax+esi], 0xe9909090
push ebx
add esi, 4
lea esp, [esp+192]	; reset
vm_reenter_patch_location:	;control flow -> resetting SP
lea esp, [esp+-52]	; new junk SP
mov edi, vm_reenter
sub edi, eax
jmp lbl_4cbaf66e_044b_48be_b44e_569147061020

lbl_88466859_41a7_4db0_8759_4ac4f7027479:

lea esp, [esp+-28]	; decrement junk SP
; Save VM ESP at this state (don't push anything past this point)
lea esp, [esp+68]	; reset
mov dword [ebx+CONTEXT_VM_ESP], esp
lea edi, [ebx++CONTEXT_NATIVE_ESP]
; Add mov [<VM context>], esp
mov dword [eax+esi+0], 0x25899090
lea esp, [esp+-64]	; new junk SP
mov dword [eax+esi+4], edi
add esi, 8
jmp lbl_e720a360_a3e0_4338_a131_6d41157f1184

lbl_7a5dc999_dc22_4283_a210_775112b0a723:

lea esp, [esp+-12]	; decrement junk SP
lea esp, [esp+56]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-44]	; new junk SP
lea esp, [esp+44]	; reset
_push_flag:	;control flow -> resetting SP
mov eax, [edx]
lea esp, [esp+-36]	; new junk SP
push ecx
add edx, 4
jmp lbl_d034b97c_2db0_416a_b4d7_ba3ccd79d750

lbl_27c19fdc_174b_41d0_b116_903fdb2827d6:

jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-48]	; new junk SP
push 2121
lea esp, [esp+52]	; reset
vm_push_reg:	;control flow -> resetting SP
lea esp, [esp+-36]	; new junk SP
push ebp
; int3
lea esp, [esp+28]	; increment junk SP
push edi
jmp lbl_65a57896_cc5e_4fec_8a21_52822514c323

lbl_f148519a_11c1_4b04_b344_417dfe6159ef:

lea esp, [esp+-40]	; new junk SP
push dword [esp+40]
pop dword [ebx++CONTEXT_FLAGS]
push 7740
mov dword [esp+44], eax
lea esp, [esp+44]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-44]	; new junk SP
push 2048
lea esp, [esp+48]	; reset
jmp lbl_daef9012_aa83_42aa_add3_b0087223bb50

lbl_1553c355_b7fd_4c55_9726_ac77ed765caa:

lea esp, [esp+-60]	; new junk SP
push eax
mov dword [eax++0], ebx	;move context pointer to native stack
lea esp, [esp+20]	; increment junk SP
push eax
push dword [esp+48]
pop dword [eax++4]	;move scratch pointer to native stack
lea esp, [esp+-4]	; decrement junk SP
push dword [esp+56]
pop dword [eax++8]	;vm exit return value?
jmp lbl_1aec8077_0bdc_4d4b_adfc_52039f0410e8

lbl_64ebe533_b05a_48e5_aacf_8077a13d17e1:

lea esp, [esp+-60]	; new junk SP
mov eax, [edx]
push edi
inc edx
lea esp, [esp+16]	; increment junk SP
push 5449
and eax, 0xff
push 3000
cmp eax, 4
lea esp, [esp+-12]	; decrement junk SP
jmp lbl_1e9f5133_3483_41c3_935f_f06700fb3b6c

lbl_cdbf58e7_2f82_4d0f_96b0_9101f156ad0a:

lea esp, [esp+0]	; reset
_jump_abs:	;control flow -> resetting SP
lea esp, [esp+-40]	; new junk SP
push 6648
mov eax, [esp+44]
lea esp, [esp+48]	; reset
vm_va_table_patch_location:	;control flow -> resetting SP
lea esp, [esp+-48]	; new junk SP
mov ecx, [+VM_FILEHEADER_VA_TABLE]
push ebp
jmp lbl_e35642e2_7fc5_405e_bcb6_45110077d809

lbl_c82581a4_4235_48e3_9c66_f4a8356f3dc3:

lea esp, [esp+52]	; reset
popfd 	;control flow -> resetting SP
lea esp, [esp+-48]	; new junk SP
push esi
lea esp, [esp+52]	; reset
popad 	;control flow -> resetting SP
lea esp, [esp++8]	;remove the two VirtualFree addresses
lea esp, [esp+0]	; reset
retn 	;control flow -> resetting SP
lea esp, [esp+-60]	; new junk SP
jmp lbl_1db9a8a0_eefc_417c_81e3_34d97eb972e9

lbl_fc76fc56_683c_4419_b8c7_4fc3ee8aff02:

mov ecx, [ecx]
lea esp, [esp+48]	; increment junk SP
cmp eax, 4
lea esp, [esp+24]	; reset
jne lbl_e78feab4_f082_4605_8a20_67a49ff579d4	;control flow -> resetting SP
lea esp, [esp+-40]	; new junk SP
mov [ebx++CONTEXT_NATIVE_ESP], ecx	;special case for now
add dword [ebx++CONTEXT_NATIVE_ESP], 4
lea esp, [esp+40]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
jmp lbl_9376ae38_32e2_4b88_83f8_d2339fd87709

lbl_8c126541_a9b0_4e87_b87b_d8dd87fbff40:

pop dword [ebx++CONTEXT_FLAGS]
mov dword [esp+56], eax
push ebx
lea esp, [esp+60]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+-56]	; new junk SP
push ebp
lea esp, [esp+60]	; reset
_sub:	;control flow -> resetting SP
pop eax
jmp lbl_b2db32a5_30ad_40f8_a247_cf76ebfae33f

lbl_1db9a8a0_eefc_417c_81e3_34d97eb972e9:

lea esp, [esp+60]	; reset
vm_fetch_decode:	;control flow -> resetting SP
lea esp, [esp+-52]	; new junk SP
mov eax, [edx]
and eax, 0xff
push esi
imul eax, 4
lea esp, [esp+12]	; increment junk SP
push ebp
inc edx
jmp lbl_aa64cc4a_a3ae_4bc6_90cc_8fc348a94b7c

lbl_e35642e2_7fc5_405e_bcb6_45110077d809:

lea esp, [esp+52]	; reset
vm_va_table_values_patch_location:	;control flow -> resetting SP
mov edx, [+VM_FILEHEADER_VA_TABLE_VALUES]
lea esp, [esp+-56]	; new junk SP
push 7984
mov esi, ecx
lea esp, [esp+60]	; reset
lbl_391d6e73_00a5_422a_99ca_2b8eef0c52e3:	;control flow -> resetting SP
cmp dword [esi], -1
lea esp, [esp+-56]	; new junk SP
jmp lbl_d741e7a5_d287_45bd_b60b_da2d4064016d

lbl_3d9ac79d_dcce_4e39_95bf_794f94edc7e2:

push 2947
lea esp, [esp+60]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
_add:	;control flow -> resetting SP
pop eax
lea esp, [esp+-36]	; new junk SP
mov ecx, [esp+36]
push ebx
lea esp, [esp+44]	; reset
jmp lbl_0986d64a_cdb0_4e11_aa87_3c01f6345310

lbl_760caa0a_0c60_4e4c_8f4d_3cf1691bd120:

mov dword [esp+104], VM_SCRATCH_SIZE
push 8147
mov dword [esp+104], 0
lea esp, [esp+60]	; increment junk SP
push ebx
lea esp, [esp+48]	; reset
vm_virtualalloc_patch_location_1:	;control flow -> resetting SP
lea esp, [esp+-56]	; new junk SP
push edx
lea esp, [esp+60]	; reset
jmp lbl_6ffdc82f_4b0c_4ed0_b622_9587bcdd6c24

lbl_28924539_afee_4f71_97c4_cf004ac35820:

mov dword [+VM_FILEHEADER_BASE], eax
lea esp, [esp+-60]	; new junk SP
add edx, eax	;add image base to VM IP
push 2063
lea esp, [esp+64]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
vm_native_dispatch:	;control flow -> resetting SP
; int3
lea esp, [esp+-48]	; new junk SP
jmp lbl_6fa600a0_6804_4aa7_bb7d_48f3812517bb

lbl_8064d5d6_2036_403e_9e78_e42b92aa4048:

lbl_7c5c7bd4_6cfa_4c1d_bcc5_7413c19c05fc:	;control flow -> resetting SP
lea esp, [esp+-60]	; new junk SP
push ebp
neg eax
shl eax, 2
push esi
lea esp, [esp+68]	; reset
push dword [ebx+eax+CONTEXT_EAX]
lea esp, [esp+-60]	; new junk SP
push dword [esp+60]
jmp lbl_78d6c8a9_05bb_45aa_88da_87fca7034763

lbl_3882c3d5_fc07_4d6f_9671_6ca9b58db7e9:

jmp vm_exit	;control flow -> resetting SP
lea esp, [esp+0]	; reset
vm_push:	;control flow -> resetting SP
lea esp, [esp+-64]	; new junk SP
push 3607
lea esp, [esp+68]	; reset
push dword [edx]
lea esp, [esp+-40]	; new junk SP
add edx, 4
lea esp, [esp+40]	; reset
jmp lbl_551bc3ce_f5fc_4506_8620_dc125dd326d9

lbl_4cbaf66e_044b_48be_b44e_569147061020:

push 7800
sub edi, esi
lea esp, [esp+-16]	; decrement junk SP
push ebp
sub edi, 4
push 5287
mov dword [eax+esi], edi
lea esp, [esp+-44]	; decrement junk SP
push eax
mov ecx, [ebx++CONTEXT_SCRATCH]
jmp lbl_218a9c20_4188_4360_aeaa_1e7bd48544a8

lbl_3fcea45c_b7c7_45af_ab88_de15c98d470a:

pop dword [eax++0x28]
push dword [esp+8]
pop dword [eax++0x24]
push dword [esp+12]
pop dword [eax++0x20]
push dword [esp+16]
pop dword [eax++0x1c]
push dword [esp+20]
pop dword [eax++0x18]
push dword [esp+24]
jmp lbl_d31a5404_9af1_4d75_91ae_2853c3d61d2c

lbl_d741e7a5_d287_45bd_b60b_da2d4064016d:

push 6896
lea esp, [esp+60]	; reset
je no_va_found	;control flow -> resetting SP
lea esp, [esp+-44]	; new junk SP
push 4807
cmp [esi], eax
lea esp, [esp+12]	; increment junk SP
lea esp, [esp+36]	; reset
je lbl_6b3e907e_121d_4476_b504_581199cf26ec	;control flow -> resetting SP
lea esp, [esp+-40]	; new junk SP
jmp lbl_0818c88d_29de_4d05_935c_bc588e481a8e

lbl_bbbce1bd_a3db_4c08_a31c_532a8980b604:

lea esp, [esp+56]	; increment junk SP
lea esp, [esp+0]	; reset
lbl_95eca7d8_6b5b_4f5a_9d54_ba34e3578126:	;control flow -> resetting SP
lea esp, [esp+-56]	; new junk SP
cmp ecx, esi
lea esp, [esp+56]	; reset
je lbl_f46ee2ae_b72a_4138_891b_3b47467a78c9	;control flow -> resetting SP
lea esp, [esp+-48]	; new junk SP
lea esp, [esp+48]	; reset
push dword [edx+esi]
jmp lbl_67bdaa4d_c95f_4061_8d48_dc71b333bca9

lbl_084ac380_2e05_476b_813e_8c59fbfd0b9a:

push dword [ebx++CONTEXT_NATIVE_ESP]	;special case for now
lea esp, [esp+-40]	; new junk SP
push 6470
lea esp, [esp+44]	; reset
jmp vm_fetch_decode	;control flow -> resetting SP
lea esp, [esp+0]	; reset
lbl_fe1e5aa0_5781_4a05_a514_3e20f155a315:	;control flow -> resetting SP
neg eax
lea esp, [esp+-48]	; new junk SP
push ebp
jmp lbl_8a3fda83_3d7a_491f_8e37_133537ebeabe

lbl_1aec8077_0bdc_4d4b_adfc_52039f0410e8:

push esp
lea esp, [esp+64]	; reset
vm_exit_patch_location:	;control flow -> resetting SP
push vm_exit_native
lea esp, [esp+0]	; reset
jmp vm_native_dispatch_2	;control flow -> resetting SP
lea esp, [esp+-60]	; new junk SP
lea esp, [esp+60]	; reset
vm_exit_native:	;control flow -> resetting SP
lea esp, [esp+0]	; reset
jmp lbl_910eec8d_a63a_47c2_b130_7dcc7e0f2fc7

lbl_c51338c6_c6e9_4a0c_a679_62ece01480c7:

push esp
; 32 bit Fowler-Noll-Vo hashing
lea esp, [esp+0]	; increment junk SP
push edi
lea esp, [esp+44]	; reset
fnv_32:	;control flow -> resetting SP
mov esi, [esp++0x04]	;buffer
mov ecx, [esp++0x08]	;length
mov eax, 0x0ce942fa	;basis
mov edi, 0x01000193	;fnv_32_prime
jmp lbl_3227dd4a_a430_49ac_a640_7c0a300312db

lbl_0fb57007_bce2_4b08_87d4_ae9839973bb6:

; end_shuffle
; Lookup table for VAs that have been manually added as jump targets
; These are placeholders only, the actual tables are patched in at runtime
va_table:
va_table_values:

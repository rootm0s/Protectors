/* x86.Virtualizer
 * Copyright 2007 ReWolf
 * Contact:
 * rewolf@rewolf.pl
 * http://rewolf.pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef _MACROS_H_
#define _MACROS_H_

//flags
#define CF 0x00000001 
#define PF 0x00000004
#define ZF 0x00000040
#define SF 0x00000080
#define OF 0x00000800

//registers
#define _EAX 0
#define _ECX 1
#define _EDX 2
#define _EBX 3
#define _ESP 4
#define _EBP 5
#define _ESI 6
#define _EDI 7

//vm opcodes
#define I_COND_JMP_SHORT	0x00			//00: SHORT Jcc (0x7x) 
#define I_COND_JMP_LONG		0x01			//01: LONG Jcc	(0x0F, 0x8x)
#define I_JECX				0x02			//02: J(E)CXZ	(0xE3)
#define I_CALL_REL			0x03			//03: CALL		(0xE8)
#define I_VM_END			0x04			//04: VM_END	(macro)
#define I_RET				0x05			//05: RET		(ret, ret v)
#define I_LOOPxx			0x06			//06: LOOP(Z/NZ)(0xE0/0xE1/0xE2)
#define I_VM_MOV_IMM		0x07
#define I_VM_MOV_REG		0x08
#define I_VM_ADD_IMM		0x09
#define I_VM_ADD_REG		0x0A
#define I_VM_SHL_IMM		0x0B
#define I_VM_REAL			0x0C
#define I_JMP_LONG			0x0D
#define I_JMP_SHORT			0x0E
//rol, ror, rcl, rcr, sal, shl, shr, sar
#define I_VM_ROL			0x0F
#define I_VM_ROR			0x10
#define I_VM_RCL			0x11
#define I_VM_RCR			0x12
#define I_VM_SAL			0x13
#define I_VM_SHL			0x14
#define I_VM_SHR			0x15
#define I_VM_SAR			0x16
//(add, or, adc, sbb, and, sub, xor, cmp)(mov)
#define I_VM_ADD			0x17
#define I_VM_OR				0x18
#define I_VM_ADC			0x19
#define I_VM_SBB			0x1A
#define I_VM_AND			0x1B
#define I_VM_SUB			0x1C
#define I_VM_XOR			0x1D
#define I_VM_CMP			0x1E
#define I_VM_MOV			0x1F
//(call, jmp, push)(pop)
#define I_VM_CALL			0x20
#define I_VM_JMP			0x21
#define I_VM_PUSH			0x22
#define I_VM_POP			0x23
//
#define I_VM_RELOC			0x24
#define I_VM_FAKE_CALL		0x25
#define I_VM_NOP			0x26

//----------------------------------------------------------------------------------------
#define PUT_VM_PREFIX *(WORD*)(outCodeBuf + outPos + 1) = vm_instr_prefix
#define PUT_VM_OP_SIZE(a) *(outCodeBuf + outPos) = a
#define PUT_VM_OPCODE(a) *(outCodeBuf + outPos + 3) = opcodeTab[a]
//(codeBase + curPos + prefSize)[0] instead of dis.disasm_opcode
#define OPCODE_BEGIN(a) else if ((codeBase + curPos + prefSize)[0] == a) { if (outCodeBuf)
#define OPCODE_BEGIN_3(a, b, c) else if (((codeBase + curPos + prefSize)[0] == a) || ((codeBase + curPos + prefSize)[0] == b) || ((codeBase + curPos + prefSize)[0] == c)) { if (outCodeBuf)
#define OPCODE_BEGIN_MAP_B else if (0
#define OPCODE_MAP_ENTRY(a) || ((codeBase + curPos + prefSize)[0] == a)
#define OPCODE_BEGIN_MAP_E ) { /*if (outCodeBuf)*/
#define OPCODE_END(a) outPos += a; }
//----------------------------------------------------------------------------------------
#define MAKE_XXX_IMM(imm, vmop) \
	if (outCodeBuf) \
	{ \
	PUT_VM_OP_SIZE(7); \
	PUT_VM_PREFIX; \
	PUT_VM_OPCODE(vmop); \
	*(DWORD*)(outCodeBuf + outPos + 4) = imm; \
	} \
	outPos += 8;
//----------------------------------------------------------------------------------------
#define MAKE_XXX_REG(reg, vmop) \
	if (outCodeBuf) \
	{ \
	PUT_VM_OP_SIZE(4); \
	PUT_VM_PREFIX; \
	PUT_VM_OPCODE(vmop); \
	*(outCodeBuf + outPos + 4) = reg; \
	} \
	outPos += 5;
//----------------------------------------------------------------------------------------
#define MAKE_MOV_IMM(a) MAKE_XXX_IMM(a, I_VM_MOV_IMM)
#define MAKE_MOV_REG(a) MAKE_XXX_REG(a, I_VM_MOV_REG)
#define MAKE_ADD_IMM(a) MAKE_XXX_IMM(a, I_VM_ADD_IMM)
#define MAKE_ADD_REG(a) MAKE_XXX_REG(a, I_VM_ADD_REG)
#define MAKE_SHL_IMM(a) MAKE_XXX_REG(a, I_VM_SHL_IMM)
#define MAKE_REAL_INSTR \
	if (outCodeBuf) \
	{ \
	PUT_VM_OP_SIZE(5); \
	PUT_VM_PREFIX; \
	PUT_VM_OPCODE(I_VM_REAL); \
	*(outCodeBuf + outPos + 4) = *(codeBase + curPos + prefSize); \
	*(outCodeBuf + outPos + 5) = _reg; \
	} \
	outPos += 6;
//----------------------------------------------------------------------------------------
#define MAKE_VM_PURE(a) \
	if (outCodeBuf) \
	{ \
	PUT_VM_OP_SIZE(3); \
	PUT_VM_PREFIX; \
	PUT_VM_OPCODE(a); \
	} \
	outPos += 4;
//----------------------------------------------------------------------------------------
#define MAKE_ORIG_INSTR \
	if (outCodeBuf) \
	{ \
	*(outCodeBuf + outPos) = dis.disasm_len; \
	memmove(outCodeBuf + outPos + 1, codeBase + curPos, dis.disasm_len); \
	} \
	outPos += dis.disasm_len + 1;
//----------------------------------------------------------------------------------------
#endif

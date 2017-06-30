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


#include "protect.h"

//jump table
BYTE condTab[16];
//TODO: random vm_instr prefix (0xFFFF)
//0xFFFE, 0xFFFD, 0xFFFC, 0xFFFB, 0xFFFA, 0xFFF9,  0xFFF8,
WORD vm_instr_prefix = 0xFFFF;
DWORD vm_key;
//vm opcode table
#define VM_INSTR_COUNT 256
BYTE opcodeTab[VM_INSTR_COUNT];
BYTE* hVMMemory = 0;
DWORD __vmSize;

int vm_init(BYTE** retMem, DWORD* _vmInit, DWORD* _vmStart)
{	
	//load vm image
	HANDLE hVMFile = CreateFile("loader.exe", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD vmFileSize = GetFileSize(hVMFile, 0) - 0x400;
	if (hVMMemory) GlobalFree(hVMMemory);
	hVMMemory = (BYTE*)GlobalAlloc(GMEM_FIXED, vmFileSize);
	SetFilePointer(hVMFile, 0x400, 0, FILE_BEGIN);
	DWORD tmp;
	ReadFile(hVMFile, hVMMemory, vmFileSize, &tmp, 0);
	CloseHandle(hVMFile);

	__vmSize = *(DWORD*)hVMMemory;
	DWORD vmSize = *(DWORD*)hVMMemory;
	DWORD vmCodeStart = *(DWORD*)(hVMMemory + 4);
	DWORD _ssss = (*(DWORD*)(hVMMemory + 28))*4 + (*(DWORD*)(hVMMemory + 32))*8 + 4;
	*_vmInit = *(DWORD*)(hVMMemory + 8) - _ssss;
	*_vmStart = *(DWORD*)(hVMMemory + 12) - _ssss;
	DWORD vmPoly = *(DWORD*)(hVMMemory + 16);
	DWORD vmPrefix = *(DWORD*)(hVMMemory + 20);
	DWORD vmOpcodeTab = *(DWORD*)(hVMMemory + 24);
	*retMem = hVMMemory + _ssss;

	genPolyEncDec();
	memmove(hVMMemory + vmPoly, _vm_poly_dec, sizeof(_vm_poly_dec));
	genPermutation(condTab, 16);
	BYTE invCondTab[16];
	memmove(invCondTab, condTab, 16);
	invPerm16(invCondTab);
	permutateJcc((WORD*)(hVMMemory + vmCodeStart + 17), 16, invCondTab);
	genPermutation(opcodeTab, VM_INSTR_COUNT);
	memmove(hVMMemory + vmOpcodeTab, opcodeTab, VM_INSTR_COUNT);
	invPerm256(hVMMemory + vmOpcodeTab);
	*(WORD*)(hVMMemory + vmPrefix) = vm_instr_prefix;

	return vmSize;
}

BYTE* vm_getVMImg()
{
	return hVMMemory;
}

DWORD vm_getVMSize()
{
	return __vmSize;
}

void vm_free()
{
	GlobalFree(hVMMemory);
}

int vm_protect(BYTE* codeBase, int codeSize, BYTE* outCodeBuf, DWORD inExeFuncRVA, BYTE* relocBuf, DWORD imgBase)
{
	//relocations
	DWORD* relocMap;
	int relocs;
	if (relocBuf)
	{
		relocs = genRelocMap(relocBuf, inExeFuncRVA, codeSize, 0);
		relocMap = (DWORD*)GlobalAlloc(GMEM_FIXED, 4*relocs);
		genRelocMap(relocBuf, inExeFuncRVA, codeSize, relocMap);
	}
	/*/
	char c_tmp[10];
	sprintf(c_tmp, "rel: %d", relocs);
	MessageBox(0, c_tmp, c_tmp, 0);
	for (int i = 0; i < relocs; i++)
	{
	sprintf(c_tmp, "%08X", relocMap[i]);
	MessageBox(0, c_tmp, c_tmp, 0);
	}
	//*/
	//disasm_struct dis;
	struct 
	{
		DWORD disasm_len;
	} dis; 
	int curPos = 0;
	int outPos = 0;
	DWORD index = 0;

	int instrCnt = genCodeMap(codeBase, codeSize, 0);
	if (instrCnt == -1) return -1;
	DWORD* codeMap = (DWORD*)GlobalAlloc(GMEM_FIXED, 4*instrCnt + 4);
	DWORD* outCodeMap = (DWORD*)GlobalAlloc(GMEM_FIXED, 4*instrCnt + 8);	//one byte more for vm_end
	genCodeMap(codeBase, codeSize, codeMap);
	codeMap[instrCnt] = 0;
	outCodeMap[instrCnt + 1] = 0;

	int relocPtr = 0;

	while (curPos != codeSize)
	{		
		//
		//if (outPos > 0xBD0) MessageBox(0, "d", "d", 0);

		/*
		if (!disasm(codeBase + curPos, &dis)) 
		{
		GlobalFree(outCodeMap);
		GlobalFree(codeMap);	
		return -1;
		}
		*/
		int opSplitSize = 0;
		//patch for LDE engine bug ;p (kurwa¿ jego maæ)
		//BYTE ldeExcept[3] = {0x8D, 0x84, 0x05};
		if ((curPos - 3 < codeSize) && 
			((((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x05848D) ||
			(((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x15948D) ||
			(((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x0D8C8D) ||
			(((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x1D9C8D)
			)) dis.disasm_len = 7;
		else dis.disasm_len = lde(codeBase + curPos);		
		//
		int prefSize = dis.disasm_len >> 8;
		dis.disasm_len &= 0xFF;
		if (!dis.disasm_len)
		{
			GlobalFree(outCodeMap);
			GlobalFree(codeMap);
			GlobalFree(relocMap);
			return -1;
		}
		//if (outCodeBuf)
		//{	
		outCodeMap[index] = outPos;

		//
		if (relocBuf && (codeMap[index] < relocMap[relocPtr] - inExeFuncRVA) &&
			(codeMap[index + 1] > relocMap[relocPtr] - inExeFuncRVA))
		{			
			//MessageBox(0, "rel", "", 0);
			if (outCodeBuf) 
			{ 
				PUT_VM_OP_SIZE(dis.disasm_len + 5);
				PUT_VM_PREFIX;
				PUT_VM_OPCODE(I_VM_RELOC);
				*(outCodeBuf + outPos + 4) = relocMap[relocPtr] - inExeFuncRVA - codeMap[index];
				*(outCodeBuf + outPos + 5) = dis.disasm_len; 
				memmove(outCodeBuf + outPos + 6, codeBase + curPos, dis.disasm_len);
				(*(DWORD*)(outCodeBuf + outPos + 6 + relocMap[relocPtr] - inExeFuncRVA - codeMap[index])) -= imgBase;
			} 
			outPos += dis.disasm_len + 6;
			relocPtr++;
		}
		//
		else if (!prefSize) {

			if ((((codeBase + curPos + prefSize)[0] & 0xF0) == 0x70) || (((codeBase + curPos + prefSize)[0] == 0x0F) && (((codeBase + curPos + prefSize)[1] & 0xF0) == 0x80)))
			{
				if (outCodeBuf)
				{
					//conditional jumps correct and generate
					bool shortjmp = true;
					if ((codeBase + curPos + prefSize)[0] == 0x0F) shortjmp = false;
					PUT_VM_OP_SIZE(8);
					PUT_VM_PREFIX;
					if (shortjmp) PUT_VM_OPCODE(I_COND_JMP_SHORT);	//				
					else PUT_VM_OPCODE(I_COND_JMP_LONG);			//put Jcc opcode
					BYTE condition;
					if (shortjmp) condition = (codeBase + curPos + prefSize)[0] & 0xF;
					else condition = (codeBase + curPos + prefSize)[1] & 0xF;
					*(outCodeBuf + outPos + 4) = condTab[condition];		//put condition
					DWORD delta;
					if (shortjmp) delta = (int)*(char*)(codeBase + curPos + 1);		//byte extended to dword with sign
					else delta = *(DWORD*)(codeBase + curPos + 2);
					*(DWORD*)(outCodeBuf + outPos + 5) = delta;
				}
				outPos += 9;	//fixed length for all conditional jumps (no short/long)

			}
			OPCODE_BEGIN_MAP_B
				OPCODE_MAP_ENTRY(0xE9)
				OPCODE_MAP_ENTRY(0xEB)
			OPCODE_BEGIN_MAP_E
			{
				if (outCodeBuf)
				{
					PUT_VM_OP_SIZE(7);
					PUT_VM_PREFIX;
					//PUT_VM_OPCODE(I_JMP);
					if ((codeBase + curPos + prefSize)[0] == 0xE9) 
					{
						PUT_VM_OPCODE(I_JMP_LONG);
						*(DWORD*)(outCodeBuf + outPos + 4) = *(DWORD*)(codeBase + curPos + 1);
					}
					else 
					{
						/*if ((codeBase + curPos + prefSize)[1] == 1)
						{
							PUT_VM_OPCODE(I_VM_NOP);
							*(DWORD*)(outCodeBuf + outPos + 4) = rand();
							curPos++;
						}
						else*/
						{
							PUT_VM_OPCODE(I_JMP_SHORT);
							*(DWORD*)(outCodeBuf + outPos + 4) = (int)*(char*)(codeBase + curPos + 1);
						}
					}
				}
			}
			OPCODE_END(8)
				OPCODE_BEGIN(0xE3)	//JECXZ/JCXZ
			{
				PUT_VM_OP_SIZE(7);
				PUT_VM_PREFIX;
				PUT_VM_OPCODE(I_JECX);
				*(DWORD*)(outCodeBuf + outPos + 5) = (int)*(char*)(codeBase + curPos + 1);
			}
			OPCODE_END(8)
				OPCODE_BEGIN(0xE8)	//relative direct calls
			{
				PUT_VM_OP_SIZE(7);
				PUT_VM_PREFIX;
				if (*(DWORD*)(codeBase + curPos + 1)) PUT_VM_OPCODE(I_CALL_REL);					
				else PUT_VM_OPCODE(I_VM_FAKE_CALL);
				*(DWORD*)(outCodeBuf + outPos + 4) = inExeFuncRVA + *(DWORD*)(codeBase + curPos + 1) + curPos + 5;
			}		
			OPCODE_END(8)
				OPCODE_BEGIN(0xC2)	//ret xxxx
			{
				PUT_VM_OP_SIZE(5);
				PUT_VM_PREFIX;
				PUT_VM_OPCODE(I_RET);
				*(WORD*)(outCodeBuf + outPos + 4) = *(WORD*)(codeBase + curPos + 1);
			}
			OPCODE_END(6)
				OPCODE_BEGIN(0xC3)	//ret
			{
				PUT_VM_OP_SIZE(5);
				PUT_VM_PREFIX;
				PUT_VM_OPCODE(I_RET);
				*(WORD*)(outCodeBuf + outPos + 4) = 0;
			}
			OPCODE_END(6)
				OPCODE_BEGIN_3(0xE0, 0xE1, 0xE2)	//loop, loope, loopne
			{
				PUT_VM_OP_SIZE(8);
				PUT_VM_PREFIX;
				PUT_VM_OPCODE(I_LOOPxx);
				*(outCodeBuf + outPos + 4) = (codeBase + curPos + prefSize)[0] & 0x0F;
				*(WORD*)(outCodeBuf + outPos + 5) = (int)*(char*)(codeBase + curPos + 1);
			}
			OPCODE_END(9)
				OPCODE_BEGIN_MAP_B
				OPCODE_MAP_ENTRY(0x01)	//ADD   mem, r32
				OPCODE_MAP_ENTRY(0x03)	//ADD   r32, mem
				OPCODE_MAP_ENTRY(0x09)	//OR    mem, r32
				OPCODE_MAP_ENTRY(0x0B)	//OR    r32, mem
				OPCODE_MAP_ENTRY(0x21)	//AND   mem, r32
				OPCODE_MAP_ENTRY(0x23)	//AND   r32, mem
				OPCODE_MAP_ENTRY(0x29)	//SUB   mem, r32
				OPCODE_MAP_ENTRY(0x2B)	//SUB   r32, mem
				OPCODE_MAP_ENTRY(0x31)	//XOR   mem, r32
				OPCODE_MAP_ENTRY(0x33)	//XOR   r32, mem
				OPCODE_MAP_ENTRY(0x39)	//CMP   mem, r32
				OPCODE_MAP_ENTRY(0x3B)	//CMP   r32, mem
				OPCODE_MAP_ENTRY(0x85)	//TEST  r32, mem
				OPCODE_MAP_ENTRY(0x89)	//MOV   mem, r32
				OPCODE_MAP_ENTRY(0x8B)	//MOV   r32, mem
				OPCODE_MAP_ENTRY(0x8D)	//LEA   r32, mem
				OPCODE_BEGIN_MAP_E
			{
				int _mod, _reg, _rm, _scale, _base, _index;
				_mod = ((codeBase + curPos + prefSize)[1] & 0xC0) >> 6;
				_reg = ((codeBase + curPos + prefSize)[1] & 0x38) >> 3;
				_rm = (codeBase + curPos + prefSize)[1] & 7;
				BYTE* instr = codeBase + curPos + prefSize;
				if ((instr[1] & 7) == 0x4)
				{
					_scale = (instr[2] & 0xC0) >> 6;
					_index = (instr[2] & 0x38) >> 3;
					_base = instr[2] & 7;
				}

				switch (_mod)
				{
				case 0:
					if (_rm == 4)	//SIB
					{
						if (_index != 4)
						{
							MAKE_MOV_REG(_index);
							MAKE_SHL_IMM(_scale);
						}
						else 
						{
							MAKE_MOV_IMM(0);
						}
						MAKE_ADD_REG(_base);
						MAKE_REAL_INSTR;
					}
					else if (_rm == 5)
					{
						MAKE_MOV_IMM(*(DWORD*)(instr + 2));
						MAKE_REAL_INSTR;
					}
					else
					{
						MAKE_MOV_REG(_rm);
						MAKE_REAL_INSTR;
					}
					break;
				case 1:
					if (_rm == 4)
					{
						if (_index != 4)
						{
							MAKE_MOV_REG(_index);
							MAKE_SHL_IMM(_scale);
						}
						else 
						{
							MAKE_MOV_IMM(0);
						}
						MAKE_ADD_REG(_base);
						MAKE_ADD_IMM((int)*((char*)instr + 3));
						MAKE_REAL_INSTR;
					}
					else
					{
						MAKE_MOV_REG(_rm);
						MAKE_ADD_IMM((int)*((char*)instr + 2));
						MAKE_REAL_INSTR;
					}
					break;
				case 2:
					if (_rm == 4)
					{
						if (_index != 4)
						{
							MAKE_MOV_REG(_index);
							MAKE_SHL_IMM(_scale);
						}
						else 
						{
							MAKE_MOV_IMM(0);
						}
						MAKE_ADD_REG(_base);
						MAKE_ADD_IMM(*(DWORD*)(instr + 3));
						MAKE_REAL_INSTR;
					}
					else
					{					
						MAKE_MOV_REG(_rm);
						MAKE_ADD_IMM(*(DWORD*)(instr + 2));
						MAKE_REAL_INSTR;
					}
					break;
				case 3:
					//original instruction processing
					MAKE_ORIG_INSTR;
					break;
				}
			}
			OPCODE_END(0)
				OPCODE_BEGIN_MAP_B
				OPCODE_MAP_ENTRY(0x81)		//INSTR    Mem, IMM32 (Rej 0..7)(add, or, adc, sbb, and, sub, xor, cmp)
				OPCODE_MAP_ENTRY(0x8F)		//POP      Mem        (Rej 0)(pop)
				OPCODE_MAP_ENTRY(0xC1)		//INSTR    Mem, Db    (Rej 0..7)(rol, ror, rcl, rcr, sal, shl, shr, sar)
				OPCODE_MAP_ENTRY(0xC7)		//MOV      Mem, IMM32 (Rej 0)(mov)
				OPCODE_MAP_ENTRY(0xD1)		//INSTR    Mem, 1     (Rej 0..7)(rol, ror, rcl, rcr, sal, shl, shr, sar)
				OPCODE_MAP_ENTRY(0xFF)		//INSTR    Mem        (Rej 2, 4, 6)(call, jmp, push)
				OPCODE_BEGIN_MAP_E
			{
				BYTE tmp_imm8;
				int _mod, _reg, _rm, _scale, _index, _base;
				BYTE* instr = codeBase + curPos + prefSize;
				_mod = (instr[1] & 0xC0) >> 6;
				_reg = (instr[1] & 0x38) >> 3;
				_rm = instr[1] & 7;			
				if (_rm == 0x4)
				{
					_scale = (instr[2] & 0xC0) >> 6;
					_index = (instr[2] & 0x38) >> 3;
					_base = instr[2] & 7;
				}
				switch (_mod)
				{
				case 0:
					if (_rm == 4)	//SIB
					{
						if (_index != 4)
						{
							MAKE_MOV_REG(_index);
							MAKE_SHL_IMM(_scale);
						}
						else 
						{
							MAKE_MOV_IMM(0);
						}
						MAKE_ADD_REG(_base);
						//MAKE_REAL_INSTR;
						tmp_imm8 = 3;
					}
					else if (_rm == 5)
					{
						MAKE_MOV_IMM(*(DWORD*)(instr + 2));
						//MAKE_REAL_INSTR;
						tmp_imm8 = 6;
					}
					else
					{
						MAKE_MOV_REG(_rm);
						//MAKE_REAL_INSTR;
						tmp_imm8 = 2;
					}
					break;
				case 1:
					if (_rm == 4)
					{
						if (_index != 4)
						{
							MAKE_MOV_REG(_index);
							MAKE_SHL_IMM(_scale);
						}
						else 
						{
							MAKE_MOV_IMM(0);
						}
						MAKE_ADD_REG(_base);
						MAKE_ADD_IMM((int)*((char*)instr + 3));
						//MAKE_REAL_INSTR;
						tmp_imm8 = 4;
					}
					else
					{
						MAKE_MOV_REG(_rm);
						MAKE_ADD_IMM((int)*((char*)instr + 2));
						//MAKE_REAL_INSTR;
						tmp_imm8 = 3;
					}
					break;
				case 2:
					if (_rm == 4)
					{
						if (_index != 4)
						{
							MAKE_MOV_REG(_index);
							MAKE_SHL_IMM(_scale);
						}
						else 
						{
							MAKE_MOV_IMM(0);
						}
						MAKE_ADD_REG(_base);
						MAKE_ADD_IMM(*(DWORD*)(instr + 3));
						//MAKE_REAL_INSTR;
						tmp_imm8 = 7;
					}
					else
					{					
						MAKE_MOV_REG(_rm);
						MAKE_ADD_IMM(*(DWORD*)(instr + 2));
						//MAKE_REAL_INSTR;
						tmp_imm8 = 6;
					}
					break;
				case 3:
					//original instruction processing
					MAKE_ORIG_INSTR;
					break;
				}

				if (_mod < 3)
				{
					if ((instr[0] == 0xC1) || (instr[0] == 0xD1))
					{

						if (instr[0] == 0xC1) tmp_imm8 = instr[tmp_imm8];
						else tmp_imm8 = 1;
						BYTE __op[8] = {I_VM_ROL, I_VM_ROR, I_VM_RCL, I_VM_RCR, I_VM_SHL, I_VM_SHR, I_VM_SAL, I_VM_SAR};					
						MAKE_XXX_REG(tmp_imm8, __op[_reg]);
					}
					else if (instr[0] == 0x81)
					{
						BYTE __op[8] = {I_VM_ADD, I_VM_OR, I_VM_ADC, I_VM_SBB, I_VM_AND, I_VM_SUB, I_VM_XOR, I_VM_CMP};
						MAKE_XXX_IMM((*(DWORD*)(instr + tmp_imm8)), __op[_reg]);
					}
					else if ((instr[0] == 0xC7) && (!_reg)) { MAKE_XXX_IMM((*(DWORD*)(instr + tmp_imm8)), I_VM_MOV); }
					else if ((instr[0] == 0x8F) && (!_reg)) { MAKE_VM_PURE(I_VM_POP); }				
					else if ((instr[0] == 0xFF) && (_reg == 2)) { MAKE_VM_PURE(I_VM_CALL); }
					else if ((instr[0] == 0xFF) && (_reg == 4)) { MAKE_VM_PURE(I_VM_JMP); }
					else if ((instr[0] == 0xFF) && (_reg == 6)) { MAKE_VM_PURE(I_VM_PUSH); }
					else { MAKE_ORIG_INSTR; }
				}

			}
			OPCODE_END(0)
			else { MAKE_ORIG_INSTR; }		
		}
		else { MAKE_ORIG_INSTR; }
		//}
		curPos += dis.disasm_len;
		index++;
		if (curPos == codeSize)
		{
			//vm end
			if (outCodeBuf)
			{
				PUT_VM_OP_SIZE(3);
				PUT_VM_PREFIX;
				PUT_VM_OPCODE(I_VM_END);
			}
			outCodeMap[index] = outPos;
			outPos += 4;
			index++;
		}
	}

	outCodeMap[index] = outPos;

	//cipher loop
	//Jcc correction loop
	if (outCodeBuf)
	{
		for (int i = 0; i < instrCnt + 1; i++)
		{
			if (*(WORD*)(outCodeBuf + outCodeMap[i] + 1) == vm_instr_prefix)
			{
				//test for Jcc
				if ((*(outCodeBuf + outCodeMap[i] + 3) == opcodeTab[I_COND_JMP_SHORT]) ||
					(*(outCodeBuf + outCodeMap[i] + 3) == opcodeTab[I_COND_JMP_LONG]) ||
					(*(outCodeBuf + outCodeMap[i] + 3) == opcodeTab[I_JECX]) ||
					(*(outCodeBuf + outCodeMap[i] + 3) == opcodeTab[I_LOOPxx]) ||
					(*(outCodeBuf + outCodeMap[i] + 3) == opcodeTab[I_JMP_LONG]) ||
					(*(outCodeBuf + outCodeMap[i] + 3) == opcodeTab[I_JMP_SHORT])
					)
				{
					int ttt = 0;
					int jecxcorr = 0;
					if (*(outCodeBuf + outCodeMap[i] + 3) == opcodeTab[I_COND_JMP_LONG]) ttt = 4;
					if (*(outCodeBuf + outCodeMap[i] + 3) == opcodeTab[I_JECX]) jecxcorr = 1;
					if (*(outCodeBuf + outCodeMap[i] + 3) == opcodeTab[I_JMP_LONG]) 
					{
						jecxcorr = 1;
						ttt = 3;
					}
					if (*(outCodeBuf + outCodeMap[i] + 3) == opcodeTab[I_JMP_SHORT]) jecxcorr = 1;
					DWORD outDest = codeMap[i] + *(DWORD*)(outCodeBuf + outCodeMap[i] + 5 - jecxcorr) + 2 + ttt;
					//search outDest in codeMap
					for (int j = 0; j < instrCnt; j++)
					{
						if (outDest == codeMap[j])
						{
							*(DWORD*)(outCodeBuf + outCodeMap[i] + 5 - jecxcorr) = outCodeMap[j] - outCodeMap[i];
							break;
						}
					}
				}
			}
			{		
				{
					int tmpChr = 0;
					do
					{
						polyEnc(outCodeBuf + outCodeMap[i] + tmpChr + 1, *(outCodeBuf + outCodeMap[i] + tmpChr), outCodeMap[i] + tmpChr);
						BYTE __tt = *(outCodeBuf + outCodeMap[i] + tmpChr);						
						*(outCodeBuf + outCodeMap[i] + tmpChr) ^= *(outCodeBuf + outCodeMap[i] + tmpChr + 1);
						tmpChr += __tt + 1;
					}
					while (outCodeMap[i + 1] != outCodeMap[i] + tmpChr);
				}
			}
		}
	}

	GlobalFree(relocMap);
	GlobalFree(codeMap);
	GlobalFree(outCodeMap);
	return outPos;
}
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


#include "common.h"
#include "poly_encdec.h"

extern BYTE _vm_poly_dec[121] = {0};

extern polyFunc polyEnc = (polyFunc)(BYTE*)_vm_poly_enc;
extern polyFunc polyDec = (polyFunc)(BYTE*)_vm_poly_dec;

DWORD WINAPI _lde(BYTE* off)
{
	HDE_STRUCT hdeStr;
	hde_disasm(off, &hdeStr);
	return ((hdeStr.p_66 | hdeStr.p_67 | hdeStr.p_lock | hdeStr.p_rep | hdeStr.p_seg) << 8) | hdeStr.len;
}

void genPolyEncDec()
{
	//xor eax, dword ptr [esp+18h]
	//0x18244433
	memmove(_vm_poly_dec, _vm_poly_enc, sizeof(_vm_poly_enc));
	*(DWORD*)(_vm_poly_enc + 0x65) = 0x18244433;
	*(DWORD*)(_vm_poly_dec + 11) = 0x18244433;

	//XOR val 0x34 val
	//SUB val 0x2C val
	//ADD val 0x04 val
	//XOR CL  0x32 0xC1
	//SUB CL  0x2A 0xC1
	//ADD CL  0x02 0xC1
	//INC     0xFE 0xC0
	//DEC     0xFE 0xC8
	//ROR CL  0xD2 0xC8
	//ROL CL  0xD2 0xC0
	//junk    0xEB 0x01 xx

	int instr = 30;
	int junk = 10;
	int ptr = 11;
	while (instr || junk)
	{
		int w = rand() & 1;
		if (w && junk)
		{
			_vm_poly_enc[ptr] = 0xEB;
			_vm_poly_enc[ptr+1] = 0x01;
			_vm_poly_enc[ptr+2] = rand();
			_vm_poly_dec[114 - ptr - 1] = 0xEB;
			_vm_poly_dec[114 - ptr] = 0x01;
			_vm_poly_dec[114 - ptr + 1] = rand();
			ptr += 3;
			junk--;
		}
		else
		{
			int cinstr = rand() % 10;
			switch (cinstr)
			{
			case 0:
				_vm_poly_enc[ptr] = 0x34;
				_vm_poly_enc[ptr+1] = rand();
				_vm_poly_dec[114 - ptr] = 0x34;
				_vm_poly_dec[114 - ptr+1] = _vm_poly_enc[ptr+1];
				break;
			case 1:
				_vm_poly_enc[ptr] = 0x2C;
				_vm_poly_enc[ptr+1] = rand();
				_vm_poly_dec[114 - ptr] = 0x04;
				_vm_poly_dec[114 - ptr+1] = _vm_poly_enc[ptr+1];
				break;
			case 2:
				_vm_poly_enc[ptr] = 0x04;
				_vm_poly_enc[ptr+1] = rand();
				_vm_poly_dec[114 - ptr] = 0x2C;
				_vm_poly_dec[114 - ptr+1] = _vm_poly_enc[ptr+1];
				break;
			case 3:
				_vm_poly_enc[ptr] = 0x32;
				_vm_poly_enc[ptr+1] = 0xC1;
				_vm_poly_dec[114 - ptr] = 0x32;
				_vm_poly_dec[114 - ptr+1] = 0xC1;
				break;
			case 4:
				_vm_poly_enc[ptr] = 0x2A;
				_vm_poly_enc[ptr+1] = 0xC1;
				_vm_poly_dec[114 - ptr] = 0x02;
				_vm_poly_dec[114 - ptr+1] = 0xC1;
				break;
			case 5:
				_vm_poly_enc[ptr] = 0x02;
				_vm_poly_enc[ptr+1] = 0xC1;
				_vm_poly_dec[114 - ptr] = 0x2A;
				_vm_poly_dec[114 - ptr+1] = 0xC1;
				break;
			case 6:
				_vm_poly_enc[ptr] = 0xFE;
				_vm_poly_enc[ptr+1] = 0xC0;
				_vm_poly_dec[114 - ptr] = 0xFE;
				_vm_poly_dec[114 - ptr+1] = 0xC8;
				break;
			case 7:
				_vm_poly_enc[ptr] = 0xFE;
				_vm_poly_enc[ptr+1] = 0xC8;
				_vm_poly_dec[114 - ptr] = 0xFE;
				_vm_poly_dec[114 - ptr+1] = 0xC0;
				break;
			case 8:
				_vm_poly_enc[ptr] = 0xD2;
				_vm_poly_enc[ptr+1] = 0xC8;
				_vm_poly_dec[114 - ptr] = 0xD2;
				_vm_poly_dec[114 - ptr+1] = 0xC0;
				break;
			case 9:
				_vm_poly_enc[ptr] = 0xD2;
				_vm_poly_enc[ptr+1] = 0xC0;
				_vm_poly_dec[114 - ptr] = 0xD2;
				_vm_poly_dec[114 - ptr+1] = 0xC8;
				break;
			}
			ptr += 2;
			instr--;
		}
	}
}

int genCodeMap(BYTE* codeBase, int codeSize, DWORD* codeMap)
{
	int curPos = 0;
	int instrCount = 0;
	//disasm_struct dis;
	struct  
	{
		DWORD disasm_len;
	} dis;
	while (curPos != codeSize)
	{		
		//dis.disasm_len = lde(codeBase + curPos) & 0xFF;
		if ((curPos - 3 < codeSize) && 
			((((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x05848D) ||
			(((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x15948D) ||
			(((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x0D8C8D) ||
			(((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x1D9C8D)
			)) dis.disasm_len = 7;
		//else if ((curPos - 2 < codeSize) && (*(WORD*)(codeBase + curPos) == 0x01EB)) dis.disasm_len = 3;
		else dis.disasm_len = lde(codeBase + curPos) & 0xFF;		
		//
		if (!dis.disasm_len) return -1;
		if (codeMap) codeMap[instrCount] = curPos;
		instrCount++;
		curPos += dis.disasm_len;
	}
	return instrCount;
}

void genPermutation(BYTE* buf, int size)
{
	memset(buf, 0, size);
	int i = 0;
	while (i < size)
	{
		BYTE rnd = rand() % size;
		if (!buf[rnd]) 
		{
			buf[rnd] = i;
			i++;
		}
	}
}

void invPerm256(BYTE* buf)
{
	BYTE tmp[256];
	for (int i = 0; i < 256; i++)
	{
		tmp[buf[i]] = i; 
	}
	memmove(buf, tmp, 256);
}

void invPerm16(BYTE* buf)
{
	BYTE tmp[16];
	for (int i = 0; i < 16; i++)
	{
		tmp[buf[i]] = i; 
	}
	memmove(buf, tmp, 16);
}

void permutateJcc(WORD* buf, int elemCount, BYTE* permutation)
{
	WORD temp[16];
	for (int i = 0; i < elemCount; i++)
	{
		temp[i] = buf[permutation[i]];
		if (i > permutation[i])
		{
			WORD tmp = i - permutation[i];
			tmp <<= 9;
			temp[i] -= tmp;
		}
		else
		{
			WORD tmp = permutation[i] - i;
			tmp <<= 9;
			temp[i] += tmp;
		}
	}
	memmove(buf, temp, 2*16);
}

int genRelocMap(BYTE* relocSeg, DWORD funcRVA, int funcSize, DWORD* relocMap)
{
	BYTE* relocPtr = relocSeg;
	//DWORD delta = (DWORD)newImageBase - inh->OptionalHeader.ImageBase;
	int relCnt = 0;
	while (*(DWORD*)relocPtr)
	{
		DWORD relocRVA = ((DWORD*)relocPtr)[0];
		DWORD blockSize = ((DWORD*)relocPtr)[1];		
		for (int i = 0; i < (blockSize - 8) / 2; i++)
		{
			//if (((WORD*)(relocPtr + 8))[i] & 0xF000)
			//{
				//*(DWORD*)(newImageBase + relocRVA + (((WORD*)(relocPtr + 8))[i] & 0xFFF)) += delta;
			//}
			if ((relocRVA + (((WORD*)(relocPtr + 8))[i] & 0xFFF) >= funcRVA) &&
				(relocRVA + (((WORD*)(relocPtr + 8))[i] & 0xFFF) < funcRVA + funcSize))
			{
				if (relocMap) relocMap[relCnt] = relocRVA + (((WORD*)(relocPtr + 8))[i] & 0xFFF);
				relCnt++;
			}
		}
		relocPtr += blockSize;
	}
	return relCnt;
}

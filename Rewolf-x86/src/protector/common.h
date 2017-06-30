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


#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include "hde.h"


DWORD WINAPI _lde(BYTE* off);
//extern "C" DWORD WINAPI lde(BYTE* off);
#define lde _lde

typedef void (__stdcall *polyFunc)(BYTE* buf, DWORD size, DWORD pos);
extern polyFunc polyEnc;
extern polyFunc polyDec;
extern BYTE _vm_poly_dec[121];

int genCodeMap(BYTE* codeBase, int codeSize, DWORD* codeMap);
void genPolyEncDec();
void genPermutation(BYTE* buf, int size);
void invPerm256(BYTE* buf);
void invPerm16(BYTE* buf);
void permutateJcc(WORD* buf, int elemCount, BYTE* permutation);
int genRelocMap(BYTE* relocSeg, DWORD funcRVA, int funcSize, DWORD* relocMap);

#endif
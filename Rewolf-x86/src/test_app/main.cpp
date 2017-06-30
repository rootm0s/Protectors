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


#include <Windows.h>
#include <cstdio>
#include "resource.h"

#pragma data_seg(".vm")
BYTE VM[0x8000] = {0};
#pragma data_seg()
//#pragma comment(linker, "/SECTION:.vm,RWS")

__declspec(naked) void __stdcall _md5(BYTE* sig, BYTE* data, int size)
{
	__asm
	{
             pushad
                 mov     esi, [esp+24h]
             mov     dword ptr [esi], 67452301h
                 mov     dword ptr [esi+4], 0EFCDAB89h
                 mov     dword ptr [esi+8], 98BADCFEh
                 mov     dword ptr [esi+0Ch], 10325476h
                 mov     eax, [esp+2Ch]
             push    eax
                 xor     edx, edx
                 mov     ecx, 40h
                 div     ecx
                 inc     eax
                 pop     edx
                 sub     esp, 40h
                 mov     ebx, esp
                 mov     esi, [esp+68h]
             xchg    eax, edx

 loc_40103A:                             ; CODE XREF: sub_401000+6E3j
                 mov     edi, ebx
                 dec     edx
                 jnz     short loc_401080
                 test    eax, eax
                 js      short loc_401049
                 mov     byte ptr [eax+ebx], 80h
                 jmp     short loc_40104C
 ; ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦

 loc_401049:                             ; CODE XREF: sub_401000+41j
                 xor     eax, eax
                 dec     eax

 loc_40104C:                             ; CODE XREF: sub_401000+47j
                 mov     ecx, 40h
                 sub     ecx, eax
                 add     edi, eax
                 push    eax
                 xor     eax, eax
                 inc     edi
                 dec     ecx
                 rep stosb
                 pop     eax
                 test    eax, eax
                 js      short loc_401066
                 cmp     eax, 38h
                 jnb     short loc_40107F

 loc_401066:                             ; CODE XREF: sub_401000+5Fj
                 push    eax
                  mov     eax, [esp+70h]
             push    edx
                 xor     edx, edx
                 mov     ecx, 8
                 mul     ecx
                 mov     [ebx+38h], eax
                 mov     [ebx+3Ch], edx
                 pop     edx
                 pop     eax
                 jmp     short loc_401080
 ; ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦

 loc_40107F:                             ; CODE XREF: sub_401000+64j
                 inc     edx

 loc_401080:                             ; CODE XREF: sub_401000+3Dj
                                         ; sub_401000+7Dj
                 test    eax, eax
                 js      short loc_40108B
                 cmp     eax, 40h
                 jnb     short loc_401091
                 jmp     short loc_40108D
 ; ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦

 loc_40108B:                             ; CODE XREF: sub_401000+82j
                 xor     eax, eax

 loc_40108D:                             ; CODE XREF: sub_401000+89j
                 mov     ecx, eax
                 jmp     short loc_401096
 ; ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦

 loc_401091:                             ; CODE XREF: sub_401000+87j
                 mov     ecx, 40h

 loc_401096:                             ; CODE XREF: sub_401000+8Fj
                 mov     edi, ebx
                 rep movsb
                 push    eax
                 push    edx
                 push    ebx
                 push    esi
                 lea     esi, [esp+10h]
              mov     edi, [esp+74h]
             push    edi
                 mov     eax, [edi]
             mov     ebx, [edi+4]
             mov     ecx, [edi+8]
             mov     edx, [edi+0Ch]
             mov     edi, ebx
                 mov     ebp, ebx
                 and     edi, ecx
                 not     ebp
                 and     ebp, edx
                 or      edi, ebp
                 lea     eax, [edi+eax-28955B88h]
             add     eax, [esi]
             rol     eax, 7
                 add     eax, ebx
                 mov     edi, eax
                 mov     ebp, eax
                 and     edi, ebx
                 not     ebp
                 and     ebp, ecx
                 or      edi, ebp
                 lea     edx, [edi+edx-173848AAh]
             add     edx, [esi+4]
             rol     edx, 0Ch
                 add     edx, eax
                 mov     edi, edx
                 mov     ebp, edx
                 and     edi, eax
                 not     ebp
                 and     ebp, ebx
                 or      edi, ebp
                 lea     ecx, [edi+ecx+242070DBh]
             add     ecx, [esi+8]
             rol     ecx, 11h
                 add     ecx, edx
                 mov     edi, ecx
                 mov     ebp, ecx
                 and     edi, edx
                 not     ebp
                 and     ebp, eax
                 or      edi, ebp
                 lea     ebx, [edi+ebx-3E423112h]
             add     ebx, [esi+0Ch]
             rol     ebx, 16h
                 add     ebx, ecx
                 mov     edi, ebx
                 mov     ebp, ebx
                 and     edi, ecx
                 not     ebp
                 and     ebp, edx
                 or      edi, ebp
                 lea     eax, [edi+eax-0A83F051h]
             add     eax, [esi+10h]
             rol     eax, 7
                 add     eax, ebx
                 mov     edi, eax
                 mov     ebp, eax
                 and     edi, ebx
                 not     ebp
                 and     ebp, ecx
                 or      edi, ebp
                 lea     edx, [edi+edx+4787C62Ah]
             add     edx, [esi+14h]
             rol     edx, 0Ch
                 add     edx, eax
                 mov     edi, edx
                 mov     ebp, edx
                 and     edi, eax
                 not     ebp
                 and     ebp, ebx
                 or      edi, ebp
                 lea     ecx, [edi+ecx-57CFB9EDh]
             add     ecx, [esi+18h]
             rol     ecx, 11h
                 add     ecx, edx
                 mov     edi, ecx
                 mov     ebp, ecx
                 and     edi, edx
                 not     ebp
                 and     ebp, eax
                 or      edi, ebp
                 lea     ebx, [edi+ebx-2B96AFFh]
             add     ebx, [esi+1Ch]
             rol     ebx, 16h
                 add     ebx, ecx
                 mov     edi, ebx
                 mov     ebp, ebx
                 and     edi, ecx
                 not     ebp
                 and     ebp, edx
                 or      edi, ebp
                 lea     eax, [edi+eax+698098D8h]
             add     eax, [esi+20h]
             rol     eax, 7
                 add     eax, ebx
                 mov     edi, eax
                 mov     ebp, eax
                 and     edi, ebx
                 not     ebp
                 and     ebp, ecx
                 or      edi, ebp
                 lea     edx, [edi+edx-74BB0851h]
             add     edx, [esi+24h]
             rol     edx, 0Ch
                 add     edx, eax
                 mov     edi, edx
                 mov     ebp, edx
                 and     edi, eax
                 not     ebp
                 and     ebp, ebx
                 or      edi, ebp
                 lea     ecx, [edi+ecx-0A44Fh]
             add     ecx, [esi+28h]
             rol     ecx, 11h
                 add     ecx, edx
                 mov     edi, ecx
                 mov     ebp, ecx
                 and     edi, edx
                 not     ebp
                 and     ebp, eax
                 or      edi, ebp
                 lea     ebx, [edi+ebx-76A32842h]
             add     ebx, [esi+2Ch]
             rol     ebx, 16h
                 add     ebx, ecx
                 mov     edi, ebx
                 mov     ebp, ebx
                 and     edi, ecx
                 not     ebp
                 and     ebp, edx
                 or      edi, ebp
                 lea     eax, [edi+eax+6B901122h]
             add     eax, [esi+30h]
             rol     eax, 7
                 add     eax, ebx
                 mov     edi, eax
                 mov     ebp, eax
                 and     edi, ebx
                 not     ebp
                 and     ebp, ecx
                 or      edi, ebp
                 lea     edx, [edi+edx-2678E6Dh]
             add     edx, [esi+34h]
             rol     edx, 0Ch
                 add     edx, eax
                 mov     edi, edx
                 mov     ebp, edx
                 and     edi, eax
                 not     ebp
                 and     ebp, ebx
                 or      edi, ebp
                 lea     ecx, [edi+ecx-5986BC72h]
             add     ecx, [esi+38h]
             rol     ecx, 11h
                 add     ecx, edx
                 mov     edi, ecx
                 mov     ebp, ecx
                 and     edi, edx
                 not     ebp
                 and     ebp, eax
                 or      edi, ebp
                 lea     ebx, [edi+ebx+49B40821h]
             add     ebx, [esi+3Ch]
             rol     ebx, 16h
                 add     ebx, ecx
                 mov     edi, edx
                 mov     ebp, edx
                 and     edi, ebx
                 not     ebp
                 and     ebp, ecx
                 or      edi, ebp
                 lea     eax, [edi+eax-9E1DA9Eh]
             add     eax, [esi+4]
             rol     eax, 5
                 add     eax, ebx
                 mov     edi, ecx
                 mov     ebp, ecx
                 and     edi, eax
                 not     ebp
                 and     ebp, ebx
                 or      edi, ebp
                 lea     edx, [edi+edx-3FBF4CC0h]
             add     edx, [esi+18h]
             rol     edx, 9
                 add     edx, eax
                 mov     edi, ebx
                 mov     ebp, ebx
                 and     edi, edx
                 not     ebp
                 and     ebp, eax
                 or      edi, ebp
                 lea     ecx, [edi+ecx+265E5A51h]
             add     ecx, [esi+2Ch]
             rol     ecx, 0Eh
                 add     ecx, edx
                 mov     edi, eax
                 mov     ebp, eax
                 and     edi, ecx
                 not     ebp
                 and     ebp, edx
                 or      edi, ebp
                 lea     ebx, [edi+ebx-16493856h]
             add     ebx, [esi]
             rol     ebx, 14h
                 add     ebx, ecx
                 mov     edi, edx
                 mov     ebp, edx
                 and     edi, ebx
                 not     ebp
                 and     ebp, ecx
                 or      edi, ebp
                 lea     eax, [edi+eax-29D0EFA3h]
             add     eax, [esi+14h]
             rol     eax, 5
                 add     eax, ebx
                 mov     edi, ecx
                 mov     ebp, ecx
                 and     edi, eax
                 not     ebp
                 and     ebp, ebx
                 or      edi, ebp
                 lea     edx, [edi+edx+2441453h]
             add     edx, [esi+28h]
             rol     edx, 9
                 add     edx, eax
                 mov     edi, ebx
                 mov     ebp, ebx
                 and     edi, edx
                 not     ebp
                 and     ebp, eax
                 or      edi, ebp
                 lea     ecx, [edi+ecx-275E197Fh]
             add     ecx, [esi+3Ch]
             rol     ecx, 0Eh
                 add     ecx, edx
                 mov     edi, eax
                 mov     ebp, eax
                 and     edi, ecx
                 not     ebp
                 and     ebp, edx
                 or      edi, ebp
                 lea     ebx, [edi+ebx-182C0438h]
             add     ebx, [esi+10h]
             rol     ebx, 14h
                 add     ebx, ecx
                 mov     edi, edx
                 mov     ebp, edx
                 and     edi, ebx
                 not     ebp
                 and     ebp, ecx
                 or      edi, ebp
                 lea     eax, [edi+eax+21E1CDE6h]
             add     eax, [esi+24h]
             rol     eax, 5
                 add     eax, ebx
                 mov     edi, ecx
                 mov     ebp, ecx
                 and     edi, eax
                 not     ebp
                 and     ebp, ebx
                 or      edi, ebp
                 lea     edx, [edi+edx-3CC8F82Ah]
             add     edx, [esi+38h]
             rol     edx, 9
                 add     edx, eax
                 mov     edi, ebx
                 mov     ebp, ebx
                 and     edi, edx
                 not     ebp
                 and     ebp, eax
                 or      edi, ebp
                 lea     ecx, [edi+ecx-0B2AF279h]
             add     ecx, [esi+0Ch]
             rol     ecx, 0Eh
                 add     ecx, edx
                 mov     edi, eax
                 mov     ebp, eax
                 and     edi, ecx
                 not     ebp
                 and     ebp, edx
                 or      edi, ebp
                 lea     ebx, [edi+ebx+455A14EDh]
             add     ebx, [esi+20h]
             rol     ebx, 14h
                 add     ebx, ecx
                 mov     edi, edx
                 mov     ebp, edx
                 and     edi, ebx
                 not     ebp
                 and     ebp, ecx
                 or      edi, ebp
                 lea     eax, [edi+eax-561C16FBh]
             add     eax, [esi+34h]
             rol     eax, 5
                 add     eax, ebx
                 mov     edi, ecx
                 mov     ebp, ecx
                 and     edi, eax
                 not     ebp
                 and     ebp, ebx
                 or      edi, ebp
                 lea     edx, [edi+edx-3105C08h]
             add     edx, [esi+8]
             rol     edx, 9
                 add     edx, eax
                 mov     edi, ebx
                 mov     ebp, ebx
                 and     edi, edx
                 not     ebp
                 and     ebp, eax
                 or      edi, ebp
                 lea     ecx, [edi+ecx+676F02D9h]
             add     ecx, [esi+1Ch]
             rol     ecx, 0Eh
                 add     ecx, edx
                 mov     edi, eax
                 mov     ebp, eax
                 and     edi, ecx
                 not     ebp
                 and     ebp, edx
                 or      edi, ebp
                 lea     ebx, [edi+ebx-72D5B376h]
             add     ebx, [esi+30h]
             rol     ebx, 14h
                 add     ebx, ecx
                 mov     ebp, ebx
                 xor     ebp, ecx
                 xor     ebp, edx
                 lea     eax, [ebp+eax-5C6BEh]
             add     eax, [esi+14h]
             rol     eax, 4
                 add     eax, ebx
                 mov     ebp, eax
                 xor     ebp, ebx
                 xor     ebp, ecx
                 lea     edx, [ebp+edx-788E097Fh]
             add     edx, [esi+20h]
             rol     edx, 0Bh
                 add     edx, eax
                 mov     ebp, edx
                 xor     ebp, eax
                 xor     ebp, ebx
                 lea     ecx, [ebp+ecx+6D9D6122h]
             add     ecx, [esi+2Ch]
             rol     ecx, 10h
                 add     ecx, edx
                 mov     ebp, ecx
                 xor     ebp, edx
                 xor     ebp, eax
                 lea     ebx, [ebp+ebx-21AC7F4h]
             add     ebx, [esi+38h]
             rol     ebx, 17h
                 add     ebx, ecx
                 mov     ebp, ebx
                 xor     ebp, ecx
                 xor     ebp, edx
                 lea     eax, [ebp+eax-5B4115BCh]
             add     eax, [esi+4]
             rol     eax, 4
                 add     eax, ebx
                 mov     ebp, eax
                 xor     ebp, ebx
                 xor     ebp, ecx
                 lea     edx, [ebp+edx+4BDECFA9h]
             add     edx, [esi+10h]
             rol     edx, 0Bh
                 add     edx, eax
                 mov     ebp, edx
                 xor     ebp, eax
                 xor     ebp, ebx
                 lea     ecx, [ebp+ecx-944B4A0h]
             add     ecx, [esi+1Ch]
             rol     ecx, 10h
                 add     ecx, edx
                 mov     ebp, ecx
                 xor     ebp, edx
                 xor     ebp, eax
                 lea     ebx, [ebp+ebx-41404390h]
             add     ebx, [esi+28h]
             rol     ebx, 17h
                 add     ebx, ecx
                 mov     ebp, ebx
                 xor     ebp, ecx
                 xor     ebp, edx
                 lea     eax, [ebp+eax+289B7EC6h]
             add     eax, [esi+34h]
             rol     eax, 4
                 add     eax, ebx
                 mov     ebp, eax
                 xor     ebp, ebx
                 xor     ebp, ecx
                 lea     edx, [ebp+edx-155ED806h]
             add     edx, [esi]
             rol     edx, 0Bh
                 add     edx, eax
                 mov     ebp, edx
                 xor     ebp, eax
                 xor     ebp, ebx
                 lea     ecx, [ebp+ecx-2B10CF7Bh]
             add     ecx, [esi+0Ch]
             rol     ecx, 10h
                 add     ecx, edx
                 mov     ebp, ecx
                 xor     ebp, edx
                 xor     ebp, eax
                 lea     ebx, [ebp+ebx+4881D05h]
             add     ebx, [esi+18h]
             rol     ebx, 17h
                 add     ebx, ecx
                 mov     ebp, ebx
                 xor     ebp, ecx
                 xor     ebp, edx
                 lea     eax, [ebp+eax-262B2FC7h]
             add     eax, [esi+24h]
             rol     eax, 4
                 add     eax, ebx
                 mov     ebp, eax
                 xor     ebp, ebx
                 xor     ebp, ecx
                 lea     edx, [ebp+edx-1924661Bh]
             add     edx, [esi+30h]
             rol     edx, 0Bh
                 add     edx, eax
                 mov     ebp, edx
                 xor     ebp, eax
                 xor     ebp, ebx
                 lea     ecx, [ebp+ecx+1FA27CF8h]
             add     ecx, [esi+3Ch]
             rol     ecx, 10h
                 add     ecx, edx
                 mov     ebp, ecx
                 xor     ebp, edx
                 xor     ebp, eax
                 lea     ebx, [ebp+ebx-3B53A99Bh]
             add     ebx, [esi+8]
             rol     ebx, 17h
                 add     ebx, ecx
                 mov     ebp, edx
                 not     ebp
                 or      ebp, ebx
                 xor     ebp, ecx
                 lea     eax, [ebp+eax-0BD6DDBCh]
             add     eax, [esi]
             rol     eax, 6
                 add     eax, ebx
                 mov     ebp, ecx
                 not     ebp
                 or      ebp, eax
                 xor     ebp, ebx
                 lea     edx, [ebp+edx+432AFF97h]
             add     edx, [esi+1Ch]
             rol     edx, 0Ah
                 add     edx, eax
                 mov     ebp, ebx
                 not     ebp
                 or      ebp, edx
                 xor     ebp, eax
                 lea     ecx, [ebp+ecx-546BDC59h]
             add     ecx, [esi+38h]
             rol     ecx, 0Fh
                 add     ecx, edx
                 mov     ebp, eax
                 not     ebp
                 or      ebp, ecx
                 xor     ebp, edx
                 lea     ebx, [ebp+ebx-36C5FC7h]
             add     ebx, [esi+14h]
             rol     ebx, 15h
                 add     ebx, ecx
                 mov     ebp, edx
                 not     ebp
                 or      ebp, ebx
                 xor     ebp, ecx
                 lea     eax, [ebp+eax+655B59C3h]
             add     eax, [esi+30h]
             rol     eax, 6
                 add     eax, ebx
                 mov     ebp, ecx
                 not     ebp
                 or      ebp, eax
                 xor     ebp, ebx
                 lea     edx, [ebp+edx-70F3336Eh]
             add     edx, [esi+0Ch]
             rol     edx, 0Ah
                 add     edx, eax
                 mov     ebp, ebx
                 not     ebp
                 or      ebp, edx
                 xor     ebp, eax
                 lea     ecx, [ebp+ecx-100B83h]
             add     ecx, [esi+28h]
             rol     ecx, 0Fh
                 add     ecx, edx
                 mov     ebp, eax
                 not     ebp
                 or      ebp, ecx
                 xor     ebp, edx
                 lea     ebx, [ebp+ebx-7A7BA22Fh]
             add     ebx, [esi+4]
             rol     ebx, 15h
                 add     ebx, ecx
                 mov     ebp, edx
                 not     ebp
                 or      ebp, ebx
                 xor     ebp, ecx
                 lea     eax, [ebp+eax+6FA87E4Fh]
             add     eax, [esi+20h]
             rol     eax, 6
                 add     eax, ebx
                 mov     ebp, ecx
                 not     ebp
                 or      ebp, eax
                 xor     ebp, ebx
                 lea     edx, [ebp+edx-1D31920h]
             add     edx, [esi+3Ch]
             rol     edx, 0Ah
                 add     edx, eax
                 mov     ebp, ebx
                 not     ebp
                 or      ebp, edx
                 xor     ebp, eax
                 lea     ecx, [ebp+ecx-5CFEBCECh]
             add     ecx, [esi+18h]
             rol     ecx, 0Fh
                 add     ecx, edx
                 mov     ebp, eax
                 not     ebp
                 or      ebp, ecx
                 xor     ebp, edx
                 lea     ebx, [ebp+ebx+4E0811A1h]
             add     ebx, [esi+34h]
             rol     ebx, 15h
                 add     ebx, ecx
                 mov     ebp, edx
                 not     ebp
                 or      ebp, ebx
                 xor     ebp, ecx
                 lea     eax, [ebp+eax-8AC817Eh]
             add     eax, [esi+10h]
             rol     eax, 6
                 add     eax, ebx
                 mov     ebp, ecx
                 not     ebp
                 or      ebp, eax
                 xor     ebp, ebx
                 lea     edx, [ebp+edx-42C50DCBh]
             add     edx, [esi+2Ch]
             rol     edx, 0Ah
                 add     edx, eax
                 mov     ebp, ebx
                 not     ebp
                 or      ebp, edx
                 xor     ebp, eax
                 lea     ecx, [ebp+ecx+2AD7D2BBh]
             add     ecx, [esi+8]
             rol     ecx, 0Fh
                 add     ecx, edx
                 mov     ebp, eax
                 not     ebp
                 or      ebp, ecx
                 xor     ebp, edx
                 lea     ebx, [ebp+ebx-14792C6Fh]
             add     ebx, [esi+24h]
             rol     ebx, 15h
                 add     ebx, ecx
                 pop     edi
                 add     [edi], eax
                 add     [edi+4], ebx
                 add     [edi+8], ecx
                 add     [edi+0Ch], edx
                 pop     esi
                 pop     ebx
                 pop     edx
                 pop     eax
                 sub     eax, 40h
                 test    edx, edx
                 jnz     loc_40103A
                 add     esp, 40h
                 popad
                 retn    0Ch


	}
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COMMAND:
			{
				switch (wParam & 0xFFFF)
				{
					case BTN_DRAW:
						{
							RECT wndRect;
							HWND tmpHwnd = GetDlgItem(hwndDlg, IDC_STATIC1);
							GetClientRect(tmpHwnd, &wndRect);
							HDC hdcWnd = GetWindowDC(tmpHwnd);
							//Rectangle(hdcWnd, wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
							DWORD baseColor = ((rand() % 256) << 16) | ((rand() % 256) << 7);
							for (int i = 0; i < wndRect.bottom; i++)
							{
								HGDIOBJ oldPen = SelectObject(hdcWnd, CreatePen(PS_SOLID, 1, baseColor + i % 256));
								MoveToEx(hdcWnd, 0, i, 0);
								LineTo(hdcWnd, wndRect.right, i);
								DeleteObject(SelectObject(hdcWnd, oldPen));								
							}
							ReleaseDC(tmpHwnd, hdcWnd);
						}
						break;
					case BTN_MSG:
						MessageBox(hwndDlg, "Sample MessageBox ;p", "Info", MB_ICONINFORMATION);
						break;
					case BTN_MD5:
						{
							BYTE md5buf[16];
							_md5(md5buf, (BYTE*)0x401000, 0x9000);
							char md5sig[40] = {0};
							char tmp[5];
							for (int i = 0; i < 16; i++)
							{
								sprintf(tmp, "%02X", md5buf[i]);
								strcat(md5sig, tmp);
							}
							MessageBox(hwndDlg, md5sig, "MD5", MB_ICONINFORMATION);
						}
						break;
					case BTN_EXIT:
						EndDialog(hwndDlg, 0);
						break;
				}
			}
			break;
		case WM_CLOSE:
			EndDialog(hwndDlg, 0);
			break;
	}
	return 0;
}

int CALLBACK WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd )
{
	srand(GetTickCount());
	DialogBoxParam(hInstance, (LPCSTR)IDD_DIALOG1, 0, DialogProc, 0);

	return 0;
}
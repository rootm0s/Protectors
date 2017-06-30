/*
*   Copyright © May-30-2010 by learn_more
*   SimpleObf.h is part of the project 'SimpleObfuscation'.
*
*   Please do not use this in payhacks.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY, without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*$UCC_HDR$*
*/

#define SIMPLE_OBF_H_VERSION		003

/******************************************************************************\
**                                                                            **
**                              Expansion macros                              **
**                      not recommended for direct usage                      **
**                                                                            **
\******************************************************************************/

#define PASTE(x,y)	x##y

#define OBFUSCATENOPS3(x,obf_start_nops,obf_end_nops) \
obf_start_nops: \
	x \
	__asm { \
	__asm push obf_end_nops \
	__asm push obf_start_nops \
	__asm call _AddNops \
} \
obf_end_nops: 

#define DEL_START3(del_start_lbl) \
del_start_lbl:

#define DEL_END3(del_start_lbl,del_end_lbl) \
	__asm { \
	__asm push del_end_lbl \
	__asm push del_start_lbl \
	__asm call _AddNops \
} \
del_end_lbl: 


#define OBFUSCATENOPS2(x,y)	OBFUSCATENOPS3(x,PASTE(obf_start_nops_,y),PASTE(obf_end_nops_,y))

#define DEL_START2(x,y)	DEL_START3(PASTE2(del_start_lbl_,x,y))
#define DEL_END2(x,y)	DEL_END3(PASTE2(del_start_lbl_,x,y),PASTE2(del_end_lbl_,x,y))



/******************************************************************************\
**                                                                            **
**                    Macros / functions for direct usage                     **
**                                                                            **
\******************************************************************************/

//use these nops with the macro OBFUSCATENOPS, or manually insert __asm nop
#define NOP5	__asm nop __asm nop __asm nop __asm nop __asm nop
#define NOP10	NOP5 NOP5
#define NOP20	NOP10 NOP10

#define OBFUSCATENOPS(x)	OBFUSCATENOPS2(x,__LINE__)

//everything between the _START and _ENDE macro will only be executed once, after that
//it is overwritten with random nops.
//use both arguments to create a unique identifier, this is very important!
#define RUNONCE_START(x)	DEL_START3(PASTE(del_start_lbl_,x))
#define RUNONCE_END(x)	DEL_END3(PASTE(del_start_lbl_,x),PASTE(del_end_lbl_,x))



void __stdcall _AddNops( PBYTE start, PBYTE end );
PBYTE __stdcall AllocateJumpgate( PBYTE target, int minlen, int maxlen );
void __stdcall FreeJumpgate( PBYTE target );

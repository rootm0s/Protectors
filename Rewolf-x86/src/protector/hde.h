/*
 *  Hacker Disassembler Engine
 *  Copyright (c) 2006-2007 Veacheslav Patkov
 *  aLL rights reserved.
 *
 *  hde.h : C/C++ header file
 *
 */

#ifndef _HDE_H_
#define _HDE_H_

#pragma pack(push,1)

typedef struct {
    unsigned char    len;         /* length of command                                   */
    unsigned char    p_rep;       /* rep & rep(n)z prefix: 0xF2 or 0xF3                  */
    unsigned char    p_lock;      /* lock prefix 0xF0                                    */
    unsigned char    p_seg;       /* segment prefix: 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65  */
    unsigned char    p_66;        /* prefix 0x66                                         */
    unsigned char    p_67;        /* prefix 0x67                                         */
    unsigned char    opcode;      /* opcode                                              */
    unsigned char    opcode2;     /* second opcode (if first opcode equal 0x0F)          */
    unsigned char    modrm;       /* ModR/M byte                                         */
    unsigned char    modrm_mod;   /*  - mod byte of ModR/M                               */
    unsigned char    modrm_reg;   /*  - reg byte of ModR/M                               */
    unsigned char    modrm_rm;    /*  - r/m byte of ModR/M                               */
    unsigned char    sib;         /* SIB byte                                            */
    unsigned char    sib_scale;   /*  - scale byte of SIB                                */
    unsigned char    sib_index;   /*  - index byte of SIB                                */
    unsigned char    sib_base;    /*  - base byte of SIB                                 */
    unsigned char    imm8;        /* immediate imm8                                      */
    unsigned short   imm16;       /* immediate imm16                                     */
    unsigned long    imm32;       /* immediate imm32                                     */
    unsigned char    disp8;       /* displacement disp8                                  */
    unsigned short   disp16;      /* displacement disp16 (if prefix 0x67 exist)          */
    unsigned long    disp32;      /* displacement disp32                                 */
    unsigned char    rel8;        /* relative address rel8                               */
    unsigned short   rel16;       /* relative address rel16 (if prefix 0x66 exist)       */
    unsigned long    rel32;       /* relative address rel32                              */
} HDE_STRUCT;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

unsigned long __stdcall hde_disasm(const void *pCode, HDE_STRUCT *pHDE_STRUCT);

#ifdef __cplusplus
}
#endif

#endif /* _HDE_H_ */

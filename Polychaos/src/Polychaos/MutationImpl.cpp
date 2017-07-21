#include "MutationImpl.h"
#include "Utils.hpp"

#include <windows.h>

namespace mut
{

MutationImpl::MutationImpl()
{
}


MutationImpl::~MutationImpl()
{
}

/// <summary>
/// Mutate code graph
/// </summary>
/// <param name="root">Graph root.</param>
/// <param name="flags">User-supplied mutation flags.</param>
/// <param name="context">User-supplied context</param>
/// <returns>true on success</returns>
bool MutationImpl::Mutate( InstructionData* root, int flags /*= NoFlags*/, void* /*context*/ /*= nullptr*/ )
{
    // Switch jcc
    if (flags & SwitchJcc)
    {
        for (auto* entry = root; entry; entry = entry->next)
            if (Utils::CheckProbability( 1.0 / 2 ))
            if (entry->len == 6)
                if (((*(uint16_t*)&entry->cmd) & 0xF0FF) == 0x800F)     // find jcc
                {
                    entry->cmd[1] ^= 0x01;                  // inverse condition (jz <--> jnz)
                    auto* t = entry->rel;                   // exchange nxt & rel pointers
                    entry->rel = entry->nxt;                //  (following, and label to jmp to)
                    entry->nxt = t;
                }
    }

    if (flags & Change)
    {
        for (auto* entry = root; entry; entry = entry->next) 
        {
            if (entry->len == 2)
            {
                // get opcodes
                uint8_t opcode1_0 = entry->cmd[0];
                uint8_t opcode1_1 = entry->cmd[1];

                // 10001001 11xxxyyy     ; mov r1,r2
                // 01010xxx 01011yyy     ; push r2 // pop r1
                // 10001011 11xxxyyy     ; mov r1,r2
                // 01010yyy 01011xxx     ; push r2 // pop r1
                if ((opcode1_0 & 0xFD) == 0x89)
                    if ((opcode1_1 & 0xC0) == 0xC0)
                        if (Utils::CheckProbability( 1.0 / 3 ))
                        {
                            uint8_t t = opcode1_0;
                            opcode1_0 = 0x50 | ((opcode1_1 >> (t == 0x89 ? 3 : 0)) & 7);
                            opcode1_1 = 0x58 | ((opcode1_1 >> (t == 0x89 ? 0 : 3)) & 7);
                        }

                // 00ttt001 11xxxyyy     ; ttt r1,r2 (ADD,ADC,AND,OR,SUB,SBB,XOR,CMP)
                // 00ttt011 11yyyxxx
                // 10001001 11xxxyyy     ; mov r1,r2
                // 10001011 11yyyxxx
                if ((opcode1_1 & 0xC0) == 0xC0)
                    if (((opcode1_0 & 0xC4) == 0x00) || ((opcode1_0 & 0xFC) == 0x88))
                        if (Utils::CheckProbability( 1.0 / 3 ))
                        {
                            opcode1_0 ^= 0x02;
                            opcode1_1 = 0xC0 | ((opcode1_1 >> 3) & 7) | ((opcode1_1 & 7) << 3);
                        }

                //if (xxx==yyy)
                //001100xx 11xxxyyy     ; xor r1,r1
                //001010xx 11xxxyyy     ; sub r1,r1
                if (((opcode1_0 & 0xFC) == 0x30) || ((opcode1_0 & 0xFC) == 0x28))
                    if (((opcode1_1 & 0xC0) == 0xC0) && (((opcode1_1 >> 3) & 7) == (opcode1_1 & 7)))
                        if (Utils::CheckProbability( 1.0 / 3 ))
                        {
                            opcode1_0 ^= 0x30 ^ 0x28;
                        }

                //if (xxx==yyy)
                //0000100x 11xxxyyy     ; or r1,r1
                //1000010x 11xxxyyy     ; test r1,r1
                if (((opcode1_0 & 0xFE) == 0x08) || ((opcode1_0 & 0xFE) == 0x84))
                    if (((opcode1_1 & 0xC0) == 0xC0) && (((opcode1_1 >> 3) & 7) == (opcode1_1 & 7)))
                        if (Utils::CheckProbability( 1.0 / 3 ))
                        {
                            opcode1_0 ^= 0x08 ^ 0x84;
                        }

                // store opcodes
                entry->cmd[0] = opcode1_0;
                entry->cmd[1] = opcode1_1;

                entry->flags |= Mutated;
            }


            if (entry->len == 1)
            {
                // push reg
                /*if (entry->cmd[0] >= 0x50 && entry->cmd[0] < 0x58)
                    if (Utils::CheckProbability( 1.0 / 3 ))
                    {
                        uint8_t rg = entry->cmd[0] - 0x50;

                        // sub esp, 4
                        entry->cmd[0] = 0x83; entry->cmd[1] = 0xEC; entry->cmd[2] = 0x04;
                        // mov [esp], reg
                        entry->cmd[3] = 0x89; entry->cmd[4] = 0x04 + (rg << 3); entry->cmd[5] = 0x24;

                        entry->len = 6;
                    }

                // pop reg
                if (entry->cmd[0] >= 0x58 && entry->cmd[0] < 0x5F)
                    if (Utils::CheckProbability( 1.0 / 3 ))
                    {
                        uint8_t rg = entry->cmd[0] - 0x58;

                        // mov reg, [esp]
                        entry->cmd[0] = 0x8B;  entry->cmd[1] = 0x04 + (rg << 3); entry->cmd[2] = 0x24;
                        // add esp, 4
                        entry->cmd[3] = 0x83; entry->cmd[4] = 0xC4; entry->cmd[5] = 0x04;

                        entry->len = 6;
                    }*/
            }
        }

    }

    /*if (flags & Mix)
    {
        for (auto *entry0 = root, *entry1 = entry0; entry0; entry1 = entry0, entry0 = entry0->next)
        {
            / * ... h1 h0 ... * /    
            // if h0->nxt is standard cmd
            if (!(entry1->flags & (Mutated | xRef)))
                if (!(entry0->flags & (Mutated | Stop | xRef)))
                    if ((entry0->nxt) && (!(entry0->nxt->flags & (HaveRel | Stop | xRef))))
                    {
                        int arg1_0, arg2_0, arg1_1, arg2_1;
                        int n1 = GetArgs( &entry0->cmd[0], &arg1_0, &arg2_0, entry0->len );
                        int n2 = GetArgs( &entry1->cmd[0], &arg1_1, &arg2_1, entry1->len );
                        if (n1 && n2 && (n1 + n2 <= 3))  // both ok, and only 1 may use stack
                            if (((arg1_0 != 5) && (arg1_1 != 5)) || (n1 + n2 == 2)) // check if ESP used
                                if ((arg1_0 == -1) || (arg1_1 == -1) || (arg1_0 != arg1_1))
                                    if ((arg1_0 == -1) || (arg2_1 == -1) || (arg1_0 != arg2_1))
                                        if ((arg2_0 == -1) || (arg1_1 == -1) || (arg2_0 != arg1_1))
                                            if (Utils::CheckProbability( 1.0 / 3 ))
                                            {
                                                // swap opcodes
                                                for (int i = 0; i < 15; i++) 
                                                {
                                                    entry0->cmd[i] ^= entry1->cmd[i];
                                                    entry1->cmd[i] ^= entry0->cmd[i];
                                                    entry0->cmd[i] ^= entry1->cmd[i];
                                                }

                                                // swap lengths
                                                entry0->len ^= entry1->len;               
                                                entry1->len ^= entry0->len;
                                                entry0->len ^= entry1->len;

                                                entry0->flags |= Mutated;
                                                entry1->flags |= Mutated;
                                            }

                    }
        }
    }*/

    // Add trash
    if (flags & AddTrash)
    {
        for (auto* entry = root; entry; entry = entry->next)
        {
            if (!(entry->flags & HaveRel))
                if (Utils::CheckProbability( 1.0 / 5 ))
                {
                    if (Utils::CheckProbability( 1.0 / 10 ))
                    {
                        entry->cmd[entry->len++] = 0x90;  // nop
                    }
                    else
                    {
                        entry->cmd[entry->len++] = 0x87 + (uint8_t)(Utils::GetRandomInt( 0, 1 )) * 2;   // 87, 89 --> xchg, mov
                        uint8_t y = (uint8_t)Utils::GetRandomInt( 0, 7 );
                        entry->cmd[entry->len++] = 0xC0 + (y << 3) + y;       // reg, reg
                    }
                }
        }
    }

    return true;
}

/// <summary>
/// Get instruction arguments
/// </summary>
/// <param name="instr">Instruction data.</param>
/// <param name="arg1">First argument</param>
/// <param name="arg2">Second argument</param>
/// <param name="len">Instruction length</param>
/// <returns>2 if stack used, 1 if not, 0 if non-mixable / unknown opcode</returns>
int MutationImpl::GetArgs( uint8_t* instr, int* arg1, int* arg2, int len )
{
    *arg1 = -1;
    *arg2 = -1;

    if ((instr[1] & 0xC0) == 0xC0)              // modrm: r, r
        if ((instr[1] & 7) != 4)                // esp md
            if (((instr[1] >> 3) & 7) != 4)     // esp md
            {
                if ((instr[0] & 0xFD) == 0x89)  // mov r, r
                {
                    *arg1 = (instr[1] >> 3) & 7;
                    *arg2 = instr[1] & 7;

                    // swap
                    if (instr[0] & 2) 
                    { 
                        *arg1 ^= *arg2; 
                        *arg2 ^= *arg1; 
                        *arg1 ^= *arg2;
                    }; 

                    return 1;
                }
            }

    if ((instr[0] & 0xF8) == 0x50)          // push r
        if ((instr[0] & 7) != 4)            // esp md
        {
            *arg2 = instr[0] & 7;
            return 2;                       // 2=stack used
        }

    if ((instr[0] & 0xF8) == 0x58)          // pop r
        if ((instr[0] & 7) != 4)            // esp md
        {
            *arg1 = instr[0] & 7;
            return 2;                       // 2=stack used
        }

    if ((instr[0] & 0xF8) == 0xB8)          // mov r, c
        if ((instr[0] & 7) != 4)            // esp md
        {
            *arg1 = instr[0] & 7;
            return 1;
        }

    if (len == 1)
        if ((instr[0] & 0xF0) == 0x40)          // inc/dec r
            if ((instr[0] & 7) != 4)            // esp md
            {
                *arg2 = *arg1 = instr[0] & 7;
                return 1;
            }

    return 0;
}


}
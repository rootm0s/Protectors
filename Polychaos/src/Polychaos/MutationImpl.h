#pragma once

#include "MutationDef.h"

namespace mut
{

enum MutationFlags
{
    NoMutation  = 0x00, // don't modify anything
    SwitchJcc   = 0x01, // switch jcc execution branches
    Change      = 0x02, // substitute another instruction
    //Mix         = 0x04, // swap instructions, buggy
    AddTrash    = 0x08, // add trash instructions

    MutateAll = SwitchJcc | Change /*| Mix*/ | AddTrash,
};


class MutationImpl
{
public:
    MutationImpl();
    virtual ~MutationImpl();

    /// <summary>
    /// Mutate code graph
    /// </summary>
    /// <param name="root">Graph root.</param>
    /// <param name="flags">User-supplied mutation flags.</param>
    /// <param name="context">User-supplied context</param>
    /// <returns>true on success</returns>
    virtual bool Mutate( InstructionData* root, int flags, void* context = nullptr );

private:
    /// <summary>
    /// Get instruction arguments
    /// </summary>
    /// <param name="instr">Instruction data.</param>
    /// <param name="arg1">First argument</param>
    /// <param name="arg2">Second argument</param>
    /// <param name="len">Instruction length</param>
    /// <returns>2 if stack used, 1 if not, 0 if non-mixable / unknown opcode</returns>
    int GetArgs( uint8_t* instr, int* arg1, int* arg2, int len );
};

}
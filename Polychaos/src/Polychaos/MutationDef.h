#pragma once

#include <stdint.h>
#include <memory>

namespace mut
{

// Code analyze state
enum State
{
    Empty       = '-',  // empty (no code here, or not processed yet)
    EntryIP     = 'E',  // entry point
    NextIP      = 'N',  // next address to be analyzed
    Code        = 'C',  // already analyzed, code
};

// Instruction flags
enum Flags
{
    NoNodeFlags = 0x00, // no flags
    Stop        = 0x01, // JMP/RET-alike instruction
    HaveRel     = 0x02, // have relative argument (JMP, JCC, CALL, etc.)
    ExtRel      = 0x04, // relative arg points to external label
    Assembled   = 0x08, // already assembled
    xRef        = 0x10, // label, i.e. have XREF     
    AbsRel      = 0x20, // absolute offset argument
    Mutated     = 0x40, // was mutated
};

inline void operator |= (Flags& a1, Flags a2)
{
    a1 = (Flags)((int)a1 | (int)a2);
};

inline void operator &= (Flags& a1, Flags a2)
{
    a1 = (Flags)((int)a1 & (int)a2);
};

inline void operator &= (Flags& a1, int a2)
{
    a1 = (Flags)((int)a1 & a2);
};

inline Flags operator | (Flags a1, Flags a2)
{
    return (Flags)((int)a1 | (int)a2);
};

/// <summary>
/// Single instruction information
/// </summary>
struct InstructionData
{
    uint8_t cmd[15];            // Instruction itself
    int32_t len = 0;            // Current instruction length
    int32_t orig_len = 0;       // Original instruction length
    uint8_t* ofs = nullptr;     // Pointer to instruction in buffer
    Flags flags = NoNodeFlags;  // Flags

    uint32_t old_rva = 0;       // Original RVA, relative to buffer start
    uint32_t new_rva = 0;       // RVA after assembling
    uint32_t abs_imm = 0;       // IMM argument, if any

    InstructionData* rel = nullptr;     // Relative branch, if any
    InstructionData* nxt = nullptr;     // Next instruction in execution flow
    InstructionData* next = nullptr;    // Next record in graph

    InstructionData()
    {
        memset( cmd, 0x00, sizeof(cmd) );
    }
};

struct FuncData
{
    uint32_t section_rva;       // RVA of section containing pointer
    uint32_t rva;               // RVA of pointer relative to section base, -1 to skip entry during fixup
    uint32_t ptr;               // Pointer value, relative to code section base

    FuncData( uint32_t sec_, uint32_t rva_, uint32_t ptr_ )
        : section_rva( sec_ )
        , rva( rva_ )
        , ptr( ptr_ ) { }
};

}

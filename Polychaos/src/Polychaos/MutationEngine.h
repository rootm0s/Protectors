#pragma once

#include "MutationImpl.h"
#include "LDasm.h"

#include <map>
#include <list>

namespace mut
{

struct DataEntry
{
    uint32_t old_rva = 0;
    uint32_t new_rva = 0;
    uint32_t val = 0;


    DataEntry() { }

    DataEntry( uint32_t rva_, uint32_t val_ )
        : old_rva( rva_ )
        , val( val_ ) { }
};

class MutationEngine
{
public:
    /// <summary>
    /// MutationEngine ctor
    /// </summary>
    /// <param name="pImpl">Code mutator inplementation</param>
    MutationEngine( MutationImpl* pImpl );

    ~MutationEngine();

    /// <summary>
    /// Mutate provided code
    /// Every single byte inside buffer is treated as code. 
    /// So any data mixed with code will corrupt code graph
    /// </summary>
    /// <param name="ptr">Code ptr.</param>
    /// <param name="size">Code size</param>
    /// <param name="rva_ep">Entry point relative to Code ptr</param>
    /// <param name="extDelta">New code section RVA - old code section RVA</param>
    /// <param name="extBase">Image base + code section RVA</param>
    /// <param name="obuf">Output buffer</param>
    /// <returns>Output buffer size</returns>
    size_t Mutate( uint8_t* ptr, size_t size, 
                   size_t& rva_ep, size_t extDelta,
                   const std::list<FuncData>& knownFuncs,
                   size_t extBase, uint8_t*& obuf );

    /// <summary>
    /// Get instruction data by RVA; relative to code base
    /// </summary>
    /// <param name="rva">Instruction RVA</param>
    /// <returns>Instruction data, if any</returns>
    InstructionData* GetIdataByRVA( uint32_t rva );

    inline const std::map<uint32_t, DataEntry>& dataList() { return _jumpFixups; }

private:
    /// <summary>
    /// Disassemble code
    /// </summary>
    /// <param name="extBase">Image base + code section RVA</param>
    /// <param name="noep">No entry point</param>
    void Disasm( size_t extBase, bool noep = false );

    /// <summary>
    /// Process code graph
    /// Link relative jumps and remove short jumps
    /// </summary>
    /// <returns>0 if success, non 0 if error</returns>
    size_t Process();

    /// <summary>
    /// Assemble mutated code graph and fix offsets
    ///
    /// Algo:
    ///
    /// while (there is commands)
    /// {
    ///     take random command, which is not stored yet
    ///     for (;;)
    ///     {
    ///         select random place in output buffer
    ///         store current command to output buffer
    ///         go to next command
    ///         if command is already stored, link with jmp and break
    ///     }
    /// }
    /// </summary>
    /// <param name="rva_ep">New entry point address</param>
    /// <param name="extDelta">New code section address - old code section address</param>
    /// <param name="extBase">Image base + code section RVA</param>
    void AssembleAndLink( size_t &rva_ep, size_t extDelta, size_t extBase, size_t& realSize );

    /// <summary>
    /// Reset internal buffers
    /// </summary>
    void Reset();

private:
    uint8_t* _ptr  = nullptr;
    uint8_t* _ibuf = nullptr;   // Input code
    uint8_t* _obuf = nullptr;   // Output buffer
    uint8_t* _imap = nullptr;   // Input code map
    uint8_t* _omap = nullptr;   // Output code map
    uint32_t _size = 0;         // Input code size
    uint32_t _osize = 0;        // Output buffer size

    InstructionData* _root = nullptr;       // Graph root
    MutationImpl* _pImpl = nullptr;         // Code mutation implementation

    std::map<uint32_t, DataEntry> _jumpFixups;       // Jumptable fixups
};

}

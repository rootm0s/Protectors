#include "MutationEngine.h"
#include "Utils.hpp"

#include <cassert>
#include <stdexcept>
#include <string>
#include <algorithm>

/* |<--SPACE_START--> [code] <--SPACE_BETWEEN--> [code] <--SPACE_END-->| */
#define SPACE_START     4
#define SPACE_BETWEEN   16
#define SPACE_END       128

// try to find new ip NTRY times
#define NTRY  1000 

//#define NO_MUTATION
//#define NO_RANDOMIZE

namespace mut
{

/// <summary>
/// MutationEngine ctor
/// </summary>
/// <param name="pImpl">Code mutator inplementation</param>
MutationEngine::MutationEngine( MutationImpl* pImpl )
    : _pImpl( pImpl )
{
}


MutationEngine::~MutationEngine()
{
    Reset();
    delete _pImpl;
}

/// <summary>
/// Reset internal buffers
/// </summary>
void MutationEngine::Reset()
{
    // Clear graph
    for (auto* entry = _root; entry != nullptr;)
    {
        auto* tmp = entry;
        entry = entry->next;
        delete tmp;
    }

    delete[] _imap;
    delete[] _ibuf;
    delete[] _omap;
    delete[] _obuf;

    _root = nullptr;
    _imap = _omap = _ibuf = _obuf = nullptr;
    _size = 0;
}

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
size_t MutationEngine::Mutate( uint8_t* ptr, size_t size, 
                               size_t& rva_ep, size_t extDelta,
                               const std::list<FuncData>& knownFuncs,
                               size_t extBase, uint8_t*& obuf )
{
    //
    // Initialize
    //
    Reset();

    _ptr = ptr;
    _size = size;
    _osize = size * 2;

#ifndef NO_MUTATION
#ifndef NO_RANDOMIZE
    _osize = size * 5;
#endif
#endif

    _imap = new uint8_t[size];
    _ibuf = new uint8_t[size];
    _obuf = new uint8_t[_osize];
    _omap = new uint8_t[_osize];

    memset( _imap, '----', size );
    memset( _omap, 0x00000000, _osize );
    memset( _obuf, 0xCCCCCCCC, _osize );

    memcpy( _ibuf, ptr, size );

    if (rva_ep != -1)
        _imap[rva_ep] = EntryIP;

    // Mark known functions
    for (auto& func : knownFuncs)
        _imap[func.ptr] = NextIP;

    // Prepare code graph
    Disasm( extBase, rva_ep == -1 );
    Process( );

#ifndef NO_MUTATION
    // Apply mutations
    if (_pImpl)
        _pImpl->Mutate( _root, MutateAll, nullptr );
#endif

    // Assemble
    size_t realSize = 0;
    AssembleAndLink( rva_ep, extDelta, extBase, realSize );
    obuf = _obuf;

    return realSize;
}

/// <summary>
/// Disassemble code
///
/// Algo:
///
/// mark entrypoint as NEXT
/// while (NEXT - marked opcode found)
/// {
///     find offset of the opcode marked as NEXT
///     for (;;)
///     {
///         get opcode length
///         mark opcode bytes as CODE( already analyzed )
///         if opcode is JMP, CALL or JCC mark label it points to as NEXT
///             if opcode is RET or JMP then break
///                 go to next opcode
///                 if opcode is already processed break
///     }
/// }
/// </summary>
/// <param name="extBase">Image base + code section RVA</param>
void MutationEngine::Disasm( size_t extBase, bool noep /*= false*/ )
{
    uint32_t ip;
    bool ep_passed = false;

    // Start form EP
    auto iter = std::find( _imap, _imap + _size, EntryIP );

    for (InstructionData** pEntry = &_root;;)
    {
        // Get next ip to analyze
        if (ep_passed || noep)
        {
            iter = std::find( _imap, _imap + _size, NextIP );
            if (iter == _imap + _size)
                break;
        }
        else
            ep_passed = true;

        ip = iter - _imap;

        for (;;)
        {
            ldasm_data data = { 0 };
            auto len = ldasm( &_ibuf[ip], &data, is_x64 );
            for (uint32_t i = 0; i < len; i++)
                _imap[ip + i] = Code;

            // analyze instruction
            uint32_t imm = 0;
            int32_t  rel = -1;
            uint32_t nxt = ip + len;

            uint8_t  opcode1 = _ibuf[ip];                                   // opcode, 1 byte
            uint16_t opcode2 = *(uint16_t*)&_ibuf[ip];                      // opcode, 2 bytes
            int32_t  relVA1 = ip + len + (int8_t)_ibuf[ip + len - 1];       // relative offset, 1 byte
            int32_t  relVA4 = ip + len + *(int32_t*)&_ibuf[ip + len - 4];   // relative offset, 4 bytes

            // jcc, jcxz, loop/z/nz
            if (((opcode1 & 0xF0) == 0x70) || ((opcode1 & 0xFC) == 0xE0))
                rel = relVA1;

            // jcc near
            if ((opcode2 & 0xF0FF) == 0x800F)
                rel = relVA4;

            // call relative
            if (opcode1 == 0xE8)
                rel = relVA4;

            // jmp short
            if (opcode1 == 0xEB)
            {
                rel = relVA1;
                nxt = 0xFFFFFFFF;
            };

            // jmp
            if (opcode1 == 0xE9)
            {
                rel = relVA4;
                nxt = 0xFFFFFFFF;
            };

            // ret/ret#/retf/retf#/iret/jmp modrm
            if (((opcode1 & 0xF6) == 0xC2) || (opcode1 == 0xCF) || ((opcode2 & 0x38FF) == 0x20FF))
                nxt = 0xFFFFFFFF;

            // absolute imm (e.g. mov eax, 0x401000)
            if (data.flags & F_IMM && data.imm_size == 4)
            {
                // imm points to old .text section
                imm = *(uint32_t*)&_ibuf[ip + data.imm_offset];

                // Mark for analysis
                if (imm >= extBase && imm < extBase + _size)
                {
                    if (_imap[imm - extBase] == Empty)
                        _imap[imm - extBase] = NextIP;
                }
                else
                    imm = 0;
            }

            // absolute disp (e.g. jmp [eax*4 + 0x401000])
            if (data.flags & F_DISP && data.disp_size == 4)
            {
                imm = *(uint32_t*)&_ibuf[ip + data.disp_offset];
                if (imm < extBase || imm > extBase + _size)
                    imm = 0;

                // Check jumptable
                if (imm && opcode1 == 0xFF)
                {
                    int idx = 0;
                    for (uint32_t* ptr = (uint32_t*)(_ibuf + imm - extBase);; ptr++, idx++)
                    {
                        if (*ptr < extBase || *ptr > extBase + _size)
                            break;

                        // Mark case
                        _imap[*ptr - extBase] = NextIP;
                        _jumpFixups[*ptr - extBase] = DataEntry( imm - extBase + idx * 4, *ptr - extBase );
                    }
                }
            }

            // in range
            if (rel >= 0 && rel < (int32_t)_size)
                if (_imap[rel] == Empty)        // if not processed yet
                    _imap[rel] = NextIP;        // mark as C_NEXT

            // store instruction
            InstructionData* tmp = *pEntry;
            if (*pEntry != NULL)
                pEntry = &(*pEntry)->next;

            *pEntry = new InstructionData();

            (*pEntry)->ofs = &_ibuf[ip];
            if (tmp)
                if (!tmp->nxt)
                    tmp->nxt = (InstructionData*)(*pEntry)->ofs;

            memcpy( (*pEntry)->cmd, (*pEntry)->ofs, len );

            (*pEntry)->orig_len = (*pEntry)->len = len;
            (*pEntry)->old_rva = ip;
            (*pEntry)->flags = NoNodeFlags;
            if (*pEntry == _root)
                (*pEntry)->flags |= xRef;

            if (rel != -1)
            {         
                (*pEntry)->flags |= HaveRel | ExtRel;

                // Determine if relative instruction lies within buffer
                if (rel >= 0)
                    (*pEntry)->rel = (InstructionData*)&_ibuf[rel];
                else
                    (*pEntry)->rel = (InstructionData*)*(int32_t*)&_ibuf[ip + len - 4];
            }

            if (imm != 0)
            {
                (*pEntry)->flags |= AbsRel;
                (*pEntry)->abs_imm = imm;
            }

            if (nxt == -1)
                (*pEntry)->flags |= Stop;

            // continue disasm cycle
            ip = nxt;               // go to next instruction
            if (ip >= _size)
                break;              // NONE/out of range?

            if (_imap[ip] == Code)
            {
                (*pEntry)->nxt = (InstructionData*)&_ibuf[ip];
                break;      // break if already code
            }
        }
    }

    return;
}

/// <summary>
/// Process code graph
/// Link relative jumps and remove short jumps
/// </summary>
/// <returns>0 if success, non 0 if error</returns>
size_t MutationEngine::Process( )
{
    //
    // Link nodes
    //
    auto structMap = new InstructionData*[_size * 4]();
    for (auto* entry = _root; entry; entry = entry->next)
    {
        uint32_t ofs = (uint32_t)entry->ofs - (uint32_t)_ibuf;
        if (ofs < _size)
            structMap[ofs] = entry;
    }

    auto prev = _root;
    for (auto entry = _root; entry; entry = entry->next)
    {
        if (entry->flags & Stop)
            entry->nxt = NULL;

        // Link next instruction
        uint32_t ofs = (uint32_t)entry->nxt - (uint32_t)_ibuf;
        if (ofs < _size)
        {
            auto q = structMap[ofs];
            if (!q)
            {
                assert( false && "Invalid ptr" );

                delete[] structMap;
                throw(std::runtime_error( "Invalid code pointer at offset " + std::to_string( ofs ) ));
            }

            if ((uint32_t)entry->nxt == (uint32_t)q->ofs)
                entry->nxt = q;
        }

        // Link relative instruction
        ofs = (uint32_t)entry->rel - (uint32_t)_ibuf;
        if (ofs < _size)
        {
            auto q = structMap[ofs];
            if (!q)
            {
                assert( false && "Invalid ptr" );

                delete[] structMap;
                throw(std::runtime_error( "Invalid code pointer at offset " + std::to_string( ofs ) ));
            }

            if ((uint32_t)entry->rel == (uint32_t)q->ofs)
            {
                entry->flags &= ~ExtRel;
                q->flags |= xRef;
                entry->rel = q;                    // replace VA with structure ptr
            }
        }

        prev = entry;
    }

    delete[] structMap;

    // Replace short jumps with long
    for (auto* entry = _root; entry; entry = entry->next)
    {
        uint8_t opcode1 = entry->cmd[0];

        // short jcc
        if ((opcode1 & 0xF0) == 0x70)                 
        {
            entry->cmd[0] = 0x0F;
            entry->cmd[1] = 0xF0 ^ opcode1;     // -> near (70-->80)
            entry->len = 6;
        }
        // short jmp 
        if (opcode1 == 0xEB) 
        {
            entry->cmd[0] = 0xE9;               // -> near
            entry->len = 5;
        }
        // loop / z / nz, jcxz
        if ((opcode1 & 0xFC) == 0xE0)  
        {
            // loop
            if (opcode1 == 0xE2)                      
            {
                entry->cmd[0] = 0x49;                   // dec ecx
                *(uint16_t*)&entry->cmd[1] = 0x850F;    // jnz near ...
                entry->len = 1 + 6;
            }
            // loopne
            else if (opcode1 == 0xE0)
            {
                entry->cmd[0] = 0x49;                   // dec ecx
                *(uint16_t*)&entry->cmd[1] = 0x840F;    // jz near ...
                entry->len = 1 + 6;
            }
            // jcxz
            else if (opcode1 == 0xE3)                      
            {
                *(uint16_t*)&entry->cmd[0] = 0xC909;      // or ecx, ecx
                *(uint16_t*)&entry->cmd[2] = 0x840F;      // jz near ...
                entry->len = 2 + 6;
            }
            else
            {
                assert( false && "Abnormal jump" );
                throw(std::runtime_error( "Abnormal jump. Opcode: " + std::to_string( opcode1 ) ));
            }
        }
    }

    return 0;
}

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
void MutationEngine::AssembleAndLink( size_t &rva_ep, size_t extDelta, size_t extBase, size_t& realSize )
{
    uint32_t ip = 0;
    rva_ep = ip = 0;

#ifndef NO_MUTATION
#ifndef NO_RANDOMIZE
    rva_ep = ip = SPACE_START + Utils::GetRandomInt( 0, _osize - SPACE_START - SPACE_END );
#endif
#endif

    //
    // Assemble
    //
    for (auto topentry = _root; topentry; topentry = topentry->next)
        if (!(topentry->flags & Assembled))
        {
            auto prev = topentry;
            // for nxt-linked entries
            for (auto entry = topentry; entry; entry = entry->nxt)
            {
#ifndef NO_MUTATION
#ifndef NO_RANDOMIZE
                // calculate space available
                int i;                                  
                for (i = 0; (i < SPACE_BETWEEN) && (ip + i < _osize - SPACE_END); i++)
                    if (_omap[ip + i])
                        break;

                // Randomize placement
                if (i < SPACE_BETWEEN || Utils::CheckProbability( 1.0 / 10))
                {
                    uint32_t newip;
                    int ntry = 0;

                    do 
                    {
                        if (ntry++ > NTRY)
                            throw(std::runtime_error( "Couldn't find suitable place for code" ));

                        newip = SPACE_START + Utils::GetRandomInt( 0, _osize - SPACE_START - SPACE_END );

                        for (i = 0; (i < SPACE_BETWEEN * 2) && (newip + i < _osize - SPACE_END); i++)
                            if (_omap[newip + i])
                                break;

                    } while (i < SPACE_BETWEEN * 2);

                    newip += SPACE_BETWEEN;
                    _omap[ip] = 1;
                    _obuf[ip++] = 0xE9;     // link with jmp

                    *(uint32_t*)&_omap[ip] = 0x01010101;
                    ip += 4;

                    *(uint32_t*)&_obuf[ip - 4] = newip - ip;
                    ip = newip;
                };
#endif // NO_RANDOMIZE
#endif // NO_MUTATION
                if (entry->flags & Assembled)
                {
                    _omap[ip] = 1;
                    _obuf[ip++] = 0xE9;        // link with jmp

                    *(uint32_t*)&_omap[ip] = 0x01010101;
                    ip += 4;

                    *(uint32_t*)&_obuf[ip - 4] = (uint32_t)entry->ofs - (uint32_t)&_obuf[ip];

                    break;
                }
                else
                {
                    entry->flags |= Assembled;

                    for (int32_t i = 0; i < entry->len; i++)
                        _omap[ip + i] = 1;

                    entry->ofs = &_obuf[ip];
                    entry->new_rva = ip;

                    for (int32_t i = 0; i < entry->len; i++)
                        entry->ofs[i] = entry->cmd[i];

                    ip += entry->len;
                }

                prev = entry;

                if (entry->flags & Stop)
                    break;                  // RET/JMP-alike
            }
        }

#ifndef NO_MUTATION
    // Find free space
    uint32_t nip;
    for (nip = _osize - _jumpFixups.size() * sizeof(uint32_t); nip > 0; nip--)
        if (_omap[nip] == 1)
            break;

    ip = nip + 1;
#endif // !NO_MUTATION


    // Align address
    ip = ip % 4 ? (((ip >> 2) + 1) << 2) : ip;

    //
    // Assemble jumptables
    //
    for (auto& entry : _jumpFixups)
    {
        auto pData = GetIdataByRVA( entry.first );
        if (pData)
        {
            *(uint32_t*)&_omap[ip] = 0x01010101;
            *(uint32_t*)&_obuf[ip] = extBase + extDelta + pData->new_rva;

            entry.second.new_rva = ip;
            ip += sizeof(entry.first);
        }
    }

    realSize = ip;

    //
    // Link
    //
    for (auto entry = _root; entry; entry = entry->next)
    {
        // Have relative argument
        if (entry->flags & HaveRel)
        {
            int32_t val = 0;
            if (entry->flags & ExtRel)
            {
                // External address lies in another code section
                if ((int32_t)entry->rel < 0)
                    val = (int32_t)entry->rel - extDelta + entry->old_rva - entry->new_rva;
                else
                    val = (int32_t)entry->rel + extDelta - (uint32_t)entry->ofs - entry->len;
            }
            else
                val = (uint32_t)entry->rel->ofs - (uint32_t)entry->ofs - entry->len;

            *(int32_t*)&entry->ofs[entry->len - 4] = val;
        }
        // Have absolute argument pointing to old code section
        else if (entry->flags & AbsRel)
        {
            auto rva = (uint32_t)entry->abs_imm - extBase;
            auto pData = GetIdataByRVA( rva );
            if (pData)
            {
                *(uint32_t*)&entry->ofs[entry->orig_len - 4] = (uint32_t)entry->abs_imm + extDelta - pData->old_rva + pData->new_rva;
            }
            // Search jump tables
            else
            {
                auto iter = std::find_if( _jumpFixups.begin(), _jumpFixups.end(), 
                                          [rva]( const std::pair<uint32_t, DataEntry>& de ) { return rva == de.second.old_rva; } );

                if (iter != _jumpFixups.end())
                    *(uint32_t*)&entry->ofs[entry->orig_len - 4] = (uint32_t)entry->abs_imm + extDelta - iter->second.old_rva + iter->second.new_rva;
            }
        }
    }

    // Erase original code
    memset( _ptr, 0xCC, _size );
}

/// <summary>
/// Get instruction data by RVA; relative to code base
/// </summary>
/// <param name="rva">Instruction RVA</param>
/// <returns>Instruction data, if any</returns>
InstructionData* MutationEngine::GetIdataByRVA( uint32_t rva )
{
    for (auto entry = _root; entry; entry = entry->next)
    {
        if (rva >= entry->old_rva && rva < entry->old_rva + entry->orig_len)
            return entry;
    }

    return nullptr;
}

}
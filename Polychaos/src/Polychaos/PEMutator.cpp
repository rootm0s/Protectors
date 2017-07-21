#include "PEMutator.h"

#include <fstream>
#include <cassert>
#include <algorithm>

namespace mut
{

PEMutator::PEMutator( MutationImpl* pImpl )
    : _mutator( pImpl )
{
}


PEMutator::~PEMutator()
{
}

/// <summary>
/// Mutate executable
/// </summary>
/// <param name="filePath">Executable path</param>
/// <param name="newPath">Output file</param>
/// <returns>Output file path</returns>
std::string PEMutator::Mutate( const std::string& filePath, std::string newPath /*= "" */ )
{
    std::ifstream file( filePath, std::ios::binary | std::ios::in );
    _image.reset( new pe_bliss::pe_base( pe_bliss::pe_factory::create_pe( file ) ) );

    size_t ep = _image->get_ep();
    bool hasEP = (ep != 0);
    if (!hasEP)
    {
        for (auto& sec : _image->get_image_sections())
        {
            // Filter data sections
            if (sec.get_characteristics() & (pe_bliss::pe_win::image_scn_mem_execute))
            {
                ep = sec.get_virtual_address();
                break;
            }
        }
    }

    auto& secRef = _image->section_from_rva( ep );
    auto oldText = secRef;
    auto newText = secRef;
    pe_bliss::section relocSec;
    std::list<FuncData> funcs;

    auto ep_rva = hasEP ? ep - oldText.get_virtual_address() : -1;
    uint8_t* obuf = nullptr;

    GetKnownFunctions( oldText, funcs );

    // Strip relocations section if required
    if (_image->has_reloc())
    {
        auto& relSec = _image->section_from_directory( pe_bliss::pe_win::image_directory_entry_basereloc );
        relocSec = relSec;
        _image->strip_section( relSec );
    }

    auto& lastSec = _image->get_image_sections().back();

    // Set new section VA
    newText.set_virtual_address( lastSec.get_virtual_address() +
                                 pe_bliss::pe_utils::align_up( lastSec.get_virtual_size(), _image->get_section_alignment() ) );

    // Mutate code section
    auto delta = newText.get_virtual_address() - secRef.get_virtual_address();
    auto sz = _mutator.Mutate( (uint8_t*)secRef.get_raw_data().c_str(),
                               secRef.get_raw_data().length(),
                               ep_rva, delta, funcs,
                               _image->get_image_base_32() + oldText.get_virtual_address(),
                               obuf );

    newText.set_raw_data( std::string( obuf, obuf + sz ) );
    secRef.set_name( ".pdata" );

    _image->add_section( newText );

    // Restore reloc section
    if (_image->has_reloc())
    {
        auto& lastSec = _image->get_image_sections().back();
        relocSec.set_virtual_address( lastSec.get_virtual_address() +
                                     pe_bliss::pe_utils::align_up( lastSec.get_virtual_size(), _image->get_section_alignment() ) );

        _image->add_section( relocSec );
        _image->set_directory_rva( pe_bliss::pe_win::image_directory_entry_basereloc, relocSec.get_virtual_address() );
    }

    _image->set_base_of_code( newText.get_virtual_address() );

    if (hasEP)
        _image->set_ep( ep_rva + oldText.get_virtual_address() + delta );

    // Perform PE fixups
    FixKnownFunctions( oldText, newText, funcs );
    FixSafeSEH( oldText, newText );
    FixExport( oldText, newText );
    FixRelocs( oldText, newText );
    
    // Build new name
    if (newPath.empty())
    {
        auto pt = filePath.rfind( '.' );
        if (pt != filePath.npos)
        {
            auto ext = filePath.substr( pt + 1 );
            auto name = filePath.substr( 0, pt );
            newPath = name + "_Mutated." + ext;
        }
        else
        {
            newPath = filePath + "_Mutated";
        }
    }

    // Write new file
    std::ofstream new_file( newPath, std::ios::binary );
    if (!new_file.is_open())
        throw std::runtime_error( "Couldn't create output file: " + newPath );

    pe_bliss::rebuild_pe( *_image, new_file );

    return newPath;
}

/// <summary>
/// Try to find functions from known pointers and data sections
/// </summary>
/// <param name="oldText">original code section</param>
/// <param name="funcs">Found functions</param>
/// <returns>Function count</returns>
size_t PEMutator::GetKnownFunctions( const pe_bliss::section& oldText, std::list<FuncData>& funcs )
{
    uint32_t textStart = oldText.get_virtual_address() + _image->get_image_base_32();
    uint32_t textEnd = textStart + oldText.get_virtual_size();

    // Parse export
    if (_image->has_exports())
        ParseExport( oldText, funcs );

    // Parse TLS
    if (_image->has_tls())
        ParseTls( oldText, funcs );

    // Parse SAFESEH
    if (_image->has_config())
        ParseSAFESEH( oldText, funcs );

    // Use relocations to get code pointers
    if (_image->has_reloc())
        ParseRelocs( oldText, textStart, textEnd, funcs );
    // Brute-force approach
    else
        ParseRawSections( textStart, textEnd, funcs );

    return funcs.size();
}


/// <summary>
/// Get functions form export directory
/// </summary>
/// <param name="oldText">Original code section</param>
/// <param name="funcs">Found functions</param>
void PEMutator::ParseExport( const pe_bliss::section &oldText, std::list<FuncData> &funcs )
{
    pe_bliss::export_info info;
    auto exports = pe_bliss::get_exported_functions( *_image, info );
    auto& expSec = _image->section_from_directory( pe_bliss::pe_win::image_directory_entry_export );

    for (auto& exp : exports)
    {
        if (exp.is_forwarded())
            continue;

        // Function inside old .text section
        if (exp.get_rva() >= oldText.get_virtual_address() && 
             exp.get_rva() <= oldText.get_virtual_address() + oldText.get_virtual_size())
        {
            auto rva = exp.get_rva() - oldText.get_virtual_address();
            funcs.emplace_back( FuncData( expSec.get_virtual_address(), 0xFFFFFFFF, rva ) );
        }
    }
}

/// <summary>
/// Get functions form TLS callbacks table
/// </summary>
/// <param name="oldText">Original code section</param>
/// <param name="funcs">Found functions</param>
void PEMutator::ParseTls( const pe_bliss::section &oldText, std::list<FuncData> &funcs )
{
    auto tls = pe_bliss::get_tls_info( *_image );

    if (tls.get_callbacks_rva() != 0)
    {
        auto& tlsSec = _image->section_from_rva( tls.get_callbacks_rva() );
        auto pSectionData = _image->section_data_from_rva( tls.get_callbacks_rva() );

        for (uint32_t* pCallback = (uint32_t*)pSectionData; *pCallback; pCallback++)
        {
            auto ptr = *pCallback - _image->get_image_base_32();

            // Callback belongs to old .text section
            if (ptr >= oldText.get_virtual_address() &&
                 ptr <= oldText.get_virtual_address() + oldText.get_virtual_size())
            {
                auto rva = (const char*)pCallback - tlsSec.get_raw_data().c_str();
                funcs.emplace_back( FuncData( tlsSec.get_virtual_address(), rva, ptr - oldText.get_virtual_address() ) );
            }
        }
    }
}

/// <summary>
/// Get functions form SAFESEH table
/// </summary>
/// <param name="oldText">Original code section</param>
/// <param name="funcs">Found functions</param>
void PEMutator::ParseSAFESEH( const pe_bliss::section &oldText, std::list<FuncData> &funcs )
{
    auto& cfgSec = _image->section_from_directory( pe_bliss::pe_win::image_directory_entry_load_config );
    auto cfg = pe_bliss::get_image_config( *_image );
    auto& handlers = cfg.get_se_handler_rvas();

    for (size_t i = 0; i < handlers.size(); i++)
    {
        // Handler inside old section
        if (handlers[i] >= oldText.get_virtual_address() &&
             handlers[i] <= oldText.get_virtual_address() + oldText.get_virtual_size())
        {
            auto rva = handlers[i] - oldText.get_virtual_address();

            // Exclude functions from generic fix
            funcs.emplace_back( FuncData( cfgSec.get_virtual_address(), 0xFFFFFFFF, rva ) );
        }
    }
}


/// <summary>
/// Get functions form relocations
/// </summary>
/// <param name="oldText">Original code section</param>
/// <param name="textStart">Text section RVA + ImageBase</param>
/// <param name="textEnd">textStart + text section size</param>
/// <param name="funcs">Found functions</param>
void PEMutator::ParseRelocs( const pe_bliss::section &oldText, 
                             uint32_t textStart,
                             uint32_t textEnd, 
                             std::list<FuncData> &funcs )
{
    auto relocs = pe_bliss::get_relocations( *_image );

    for (auto& relBlock : relocs)
    {
        // Exclude code section relocations
        if (relBlock.get_rva() >= oldText.get_virtual_address() &&
             relBlock.get_rva() < oldText.get_virtual_address() + oldText.get_virtual_size())
             continue;

        auto& sec = _image->section_from_rva( relBlock.get_rva() );
        auto pData = (uint8_t*)sec.get_raw_data().data();
        auto baseRVA = relBlock.get_rva() - sec.get_virtual_address();

        for (auto& rel : relBlock.get_relocations())
        {
            auto rva = baseRVA + rel.get_rva();
            if ((rva % 4))
                continue;

            auto ptr = (uint32_t*)(pData + rva);

            // Pointer belongs to code section
            if (*ptr >= textStart &&  *ptr < textEnd)
                funcs.emplace_back( FuncData( sec.get_virtual_address(), rva, *ptr - textStart ) );
        }
    }
}

/// <summary>
/// Get functions from section data
/// </summary>
/// <param name="textStart">Text section RVA + ImageBase</param>
/// <param name="textEnd">textStart + text section size</param>
/// <param name="funcs">Found functions</param>
void PEMutator::ParseRawSections( uint32_t textStart, uint32_t textEnd, std::list<FuncData> &funcs )
{
    // Prepare exclusion regions
    std::list<std::pair<uint32_t, uint32_t>> rgns;
    for (auto i = pe_bliss::pe_win::image_directory_entry_export; i <= pe_bliss::pe_win::image_directory_entry_com_descriptor; i++)
    {
        auto base = _image->get_directory_rva( i );
        auto size = _image->get_directory_size( i );

        if (base != 0 && size != 0)
            rgns.push_back( std::make_pair( base, base + size ) );
    }

    for (auto& sec : _image->get_image_sections())
    {
        // Filter data sections
        if (sec.get_characteristics() & (pe_bliss::pe_win::image_scn_cnt_initialized_data | pe_bliss::pe_win::image_scn_cnt_uninitialized_data))
        {
            uint32_t* start = (uint32_t*)sec.get_raw_data().c_str();
            uint32_t* end = (uint32_t*)(sec.get_raw_data().c_str() + sec.get_raw_data().length());

            for (uint32_t* ptr = start; ptr < end - 1; ptr++)
            {
                // Exclude some PE metadata regions
                auto rva = sec.get_virtual_address() + (ptr - start) * sizeof(uint32_t);
                auto iter = std::find_if( rgns.begin(), rgns.end(),
                                          [rva]( const std::pair<uint32_t, uint32_t>& val ) { return (rva >= val.first && rva < val.second); } );

                if (iter == rgns.end() && *ptr >= textStart && *ptr < textEnd)
                    funcs.emplace_back( FuncData( sec.get_virtual_address(), (ptr - start) * sizeof(uint32_t), *ptr - textStart ) );
            }
        }
    }
}


/// <summary>
/// Fix function returned by GetKnownFunctions
/// </summary>
/// <param name="oldText">Original .text section</param>
/// <param name="newText">New .text section</param>
/// <param name="funcs">Function list</param>
void PEMutator::FixKnownFunctions( const pe_bliss::section& /*oldText*/, 
                                   const pe_bliss::section& newText, 
                                   const std::list<FuncData>& funcs )
{
    for (auto& func : funcs)
    {
        // Skip function
        if (func.rva == -1)
            continue;

        auto& sec = _image->section_from_rva( func.section_rva );
        auto pData = _mutator.GetIdataByRVA( func.ptr );
        if (pData)
            *(uint32_t*)(sec.get_raw_data().c_str() + func.rva) = _image->get_image_base_32() + newText.get_virtual_address() + pData->new_rva;
    }
}


/// <summary>
/// Fix relocations
/// </summary>
/// <param name="oldText">Original .text section</param>
/// <param name="newText">New .text section</param>
void PEMutator::FixRelocs( const pe_bliss::section& oldText, const pe_bliss::section& newText )
{
    // No relocations
    if (!_image->has_reloc())
        return;

    auto relocs = pe_bliss::get_relocations( *_image );
    std::vector<std::pair<uint32_t, uint16_t>> tmpRelocs;

    for (size_t i = 0; i < relocs.size(); )
    {
        // Collect information about new relocation addresses
        if (relocs[i].get_rva() >= oldText.get_virtual_address() &&
             relocs[i].get_rva() < oldText.get_virtual_address() + oldText.get_virtual_size())
        {
            auto recBaseRVA = relocs[i].get_rva() - oldText.get_virtual_address();

            for (auto& rel : relocs[i].get_relocations())
            {
                auto pData = _mutator.GetIdataByRVA( rel.get_rva() + recBaseRVA );
                if (pData)
                    tmpRelocs.push_back( std::make_pair( rel.get_rva() - (pData->old_rva - recBaseRVA) + pData->new_rva, rel.get_type() ) );
               /* else
                    assert( false && "Invalid relocation RVA" );*/
            }

            relocs.erase( relocs.begin() + i );
        }
        else
            i++;
    }

    // Add jump tables
    for (auto& entry : _mutator.dataList())
        tmpRelocs.push_back( std::make_pair( entry.second.new_rva, pe_bliss::pe_win::image_rel_based_highlow ) );

    std::sort( tmpRelocs.begin(), tmpRelocs.end() );

    // Make new relocation table
    for (auto& rel : tmpRelocs)
    {
        auto page = rel.first >> 12;
        if (relocs.empty() || relocs.back().get_rva() != newText.get_virtual_address() + (page << 12))
        {
            relocs.push_back( pe_bliss::relocation_table() );
            relocs.back().set_rva( newText.get_virtual_address() + (page << 12) );
        }

        relocs.back().add_relocation( pe_bliss::relocation_entry( rel.first & 0xFFF, rel.second ) );
    }

    pe_bliss::rebuild_relocations( *_image, relocs, 
                                   _image->get_image_sections().back()/*, 
                                   0, true, false*/ );
}

/// <summary>
/// Fix export section
/// </summary>
/// <param name="oldText">Original .text section</param>
/// <param name="newText">New .text section</param>
void PEMutator::FixExport( const pe_bliss::section& oldText, const pe_bliss::section& newText )
{
    // No exports
    if (!_image->has_exports())
        return;

    pe_bliss::export_info info;
    auto exports = pe_bliss::get_exported_functions( *_image, info );
    auto ofst = _image->get_directory_rva( pe_bliss::pe_win::image_directory_entry_export )
              - _image->section_from_directory( pe_bliss::pe_win::image_directory_entry_export ).get_virtual_address();

    for (auto& exp : exports)
    {
        if (exp.is_forwarded())
            continue;

        // Function inside old .text section
        if (exp.get_rva() >= oldText.get_virtual_address() && 
             exp.get_rva() <= oldText.get_virtual_address() + oldText.get_virtual_size())
        {
            auto rva = exp.get_rva() - oldText.get_virtual_address();
            auto pData = _mutator.GetIdataByRVA( rva );
            if (pData)
                exp.set_rva( pData->new_rva + newText.get_virtual_address() );
            else
                assert( false && "Invalid export pointer" );
        }
    }

    pe_bliss::rebuild_exports( *_image, info, exports, _image->section_from_directory( pe_bliss::pe_win::image_directory_entry_export ), ofst );
}

/// <summary>
/// Fix SAFESEH table
/// </summary>
/// <param name="oldText">Original .text section</param>
/// <param name="newText">New .text section</param>
void PEMutator::FixSafeSEH( const pe_bliss::section& oldText, const pe_bliss::section& newText )
{
    if (!_image->has_config())
        return;

    auto cfg = pe_bliss::get_image_config( *_image );
    auto& handlers = cfg.get_se_handler_rvas();

    if (handlers.empty())
        return;

    for (size_t i = 0; i < handlers.size(); i++)
    {
        // Handler inside old section
        if (handlers[i] >= oldText.get_virtual_address() &&
             handlers[i] <= oldText.get_virtual_address() + oldText.get_virtual_size())
        {
            auto rva = handlers[i] - oldText.get_virtual_address();
            auto pData = _mutator.GetIdataByRVA( rva );
            if (pData)
                handlers[i] = pData->new_rva + newText.get_virtual_address();
            else
                assert( false && "Invalid handler" );
        }
    }

    std::sort( handlers.begin(), handlers.end() );

    auto pSectionData = _image->section_data_from_va( cfg.get_se_handler_table_va() );
    memcpy( pSectionData, &handlers[0], handlers.size() * sizeof(handlers[0]) );
}

}
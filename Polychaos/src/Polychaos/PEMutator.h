#pragma once

#include "MutationEngine.h"
#include "../../contrib/portable-executable-library/pe_lib/pe_bliss.h"

#include <list>

namespace mut
{

class PEMutator
{
public:
    PEMutator( MutationImpl* pImpl );
    ~PEMutator();

    /// <summary>
    /// Mutate executable
    /// </summary>
    /// <param name="filePath">Executable path</param>
    /// <param name="newPath">Output file</param>
    /// <returns>Output file path</returns>
    std::string Mutate( const std::string& filePath, std::string newPath = "" );

private:
    /// <summary>
    /// Try to find functions from known pointers and data sections
    /// </summary>
    /// <param name="oldText">original code section</param>
    /// <param name="funcs">Found functions</param>
    /// <returns>Function count</returns>
    size_t GetKnownFunctions( const pe_bliss::section& oldText, std::list<FuncData>& funcs );

    /// <summary>
    /// Get functions form export directory
    /// </summary>
    /// <param name="oldText">Original code section</param>
    /// <param name="funcs">Found functions</param>
    void ParseExport( const pe_bliss::section &oldText, std::list<FuncData> &funcs );

    /// <summary>
    /// Get functions form TLS callbacks table
    /// </summary>
    /// <param name="oldText">Original code section</param>
    /// <param name="funcs">Found functions</param>
    void ParseTls( const pe_bliss::section &oldText, std::list<FuncData> &funcs );

    /// <summary>
    /// Get functions form SAFESEH table
    /// </summary>
    /// <param name="oldText">Original code section</param>
    /// <param name="funcs">Found functions</param>
    void ParseSAFESEH( const pe_bliss::section &oldText, std::list<FuncData> &funcs );

    /// <summary>
    /// Get functions form relocations
    /// </summary>
    /// <param name="oldText">Original code section</param>
    /// <param name="textStart">Text section RVA + ImageBase</param>
    /// <param name="textEnd">textStart + text section size</param>
    /// <param name="funcs">Found functions</param>
    void ParseRelocs( const pe_bliss::section &oldText, 
                      uint32_t textStart,
                      uint32_t textEnd, 
                      std::list<FuncData> &funcs );

    /// <summary>
    /// Get functions from section data
    /// </summary>
    /// <param name="textStart">Text section RVA + ImageBase</param>
    /// <param name="textEnd">textStart + text section size</param>
    /// <param name="funcs">Found functions</param>
    void ParseRawSections( uint32_t textStart, uint32_t textEnd, std::list<FuncData> &funcs );

    /// <summary>
    /// Fix function returned by GetKnownFunctions
    /// </summary>
    /// <param name="oldText">Original .text section</param>
    /// <param name="newText">New .text section</param>
    /// <param name="funcs">Function list</param>
    void FixKnownFunctions( const pe_bliss::section& oldText, 
                            const pe_bliss::section& newText, 
                            const std::list<FuncData>& funcs );

    /// <summary>
    /// Fix relocations
    /// </summary>
    /// <param name="oldText">Original .text section</param>
    /// <param name="newText">New .text section</param>
    void FixRelocs( const pe_bliss::section& oldText, const pe_bliss::section& newText );

    /// <summary>
    /// Fix export section
    /// </summary>
    /// <param name="oldText">Original .text section</param>
    /// <param name="newText">New .text section</param>
    void FixExport( const pe_bliss::section& oldText, const pe_bliss::section& newText );

    /// <summary>
    /// Fix SAFESEH table
    /// </summary>
    /// <param name="oldText">Original .text section</param>
    /// <param name="newText">New .text section</param>
    void FixSafeSEH( const pe_bliss::section& oldText, const pe_bliss::section& newText );

private:
    std::unique_ptr<pe_bliss::pe_base> _image;
    MutationEngine _mutator;
};

}
#include "PeHeader.h"
#include <sstream>
#include <vector>
#include "../Library/SCommand.h"
#include "assert.h"
#include "../Library/Types.h"
#include "string.h"

using std::ostream;
using std::vector;
using std::exception;
using std::string;

namespace NPeProtector
{
namespace
{
   struct SMzHeader
   {
      IMAGE_DOS_HEADER mHeader;
      BYTE mDosStub[9];
      BYTE mDosMessage[47 + 8];
   };

   const SMzHeader sMzHeader =
   {
      {
         0x5a4d,				// WORD   e_magic;                     // Magic number
         0x0090,				// WORD   e_cblp;                      // Bytes on last page of file
         0x0003,				// WORD   e_cp;                        // Pages in file
         0x0000,				// WORD   e_crlc;                      // Relocations
         0x0004,				// WORD   e_cparhdr;                   // Size of header in paragraphs
         0x0000,				// WORD   e_minalloc;                  // Minimum extra paragraphs needed
         0xffff,				// WORD   e_maxalloc;                  // Maximum extra paragraphs needed
         0x0000,				// WORD   e_ss;                        // Initial (relative) SS value
         0x00b8,				// WORD   e_sp;                        // Initial SP value
         0x0000,				// WORD   e_csum;                      // Checksum
         0x0000,				// WORD   e_ip;                        // Initial IP value
         0x0000,				// WORD   e_cs;                        // Initial (relative) CS value
         0x0040,				// WORD   e_lfarlc;                    // File address of relocation table
         0x0000,				// WORD   e_ovno;                      // Overlay number
         0, 0, 0, 0,			// WORD   e_res[4];                    // Reserved words
         0,                // WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
         0,	               // WORD   e_oeminfo;                   // OEM information; e_oemid specific
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// WORD   e_res2[10];       // Reserved words
         0x00000080			// LONG   e_lfanew;                    // File address of new exe header
      },
      { 0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21 },
      { 'T', 'h', 'i', 's', ' ', 'p', 'r', 'o', 'g', 'r', 'a', 'm', ' ', 'c', 'a', 'n', 'n', 'o', 't', ' ', 'b', 'e', ' ', 'r', 'u', 'n', ' ', 'i', 'n', ' ', 'D', 'O', 'S', ' ', 'm', 'o', 'd', 'e', 0x2e, 0x0d, 0x0d, 0x0a, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
   };

   IMAGE_NT_HEADERS32 sPeHeader =
   {
      0x00004550,                          // PE
      { 
         // IMAGE_FILE_HEADER 
         IMAGE_FILE_MACHINE_I386,          // WORD    Machine;
         0/*zero sections*/,               // WORD    NumberOfSections;
         0,                                // DWORD   TimeDateStamp;
         0,                                // DWORD   PointerToSymbolTable;
         0,									       // DWORD   NumberOfSymbols;
         sizeof(IMAGE_OPTIONAL_HEADER32),	 // WORD    SizeOfOptionalHeader;
         IMAGE_FILE_EXECUTABLE_IMAGE |		 // WORD    Characteristics;
         IMAGE_FILE_LINE_NUMS_STRIPPED |
         IMAGE_FILE_LOCAL_SYMS_STRIPPED |
         IMAGE_FILE_DEBUG_STRIPPED |
         IMAGE_FILE_32BIT_MACHINE },
         {										
            // IMAGE_OPTIONAL_HEADER32
            IMAGE_NT_OPTIONAL_HDR32_MAGIC, // WORD    Magic;
            0x05,                          // BYTE    MajorLinkerVersion;
            0x00,	                         // BYTE    MinorLinkerVersion;
            0,	                            // DWORD   SizeOfCode /*размер секций .text*/
            0,                             // DWORD   SizeOfInitializedData /*размер всех секций с данными*/
            0,                             // DWORD   SizeOfUninitializedData;
            0x1000,                        // DWORD   AddressOfEntryPoint;
            0x1000,                        // DWORD   BaseOfCode;
            0/**/,                         // DWORD   BaseOfData;
            0x400000,                      // DWORD   ImageBase;
            0x00001000,	                   // DWORD   SectionAlignment;
            0x00000200,	                   // DWORD   FileAlignment;
            0x0004,                        // WORD    MajorOperatingSystemVersion;
            0x0000,                        // WORD    MinorOperatingSystemVersion;
            0x0000,                        // WORD    MajorImageVersion;
            0x0000,                        // WORD    MinorImageVersion;
            0x0004,                        // WORD    MajorSubsystemVersion;
            0x0000,                        // WORD    MinorSubsystemVersion;
            0,	                            // DWORD   Win32VersionValue;
            0,//??????                     // DWORD   SizeOfImage;
            0x400,                         // DWORD   SizeOfHeaders;
            0x0/*????*/,                   // DWORD   CheckSum;
            2,/*GUI TODO replace*/         // WORD    Subsystem; /*копировать*/
            0,                             // WORD    DllCharacteristics;
            0x100000,                      // DWORD   SizeOfStackReserve;
            0x1000,                        // DWORD   SizeOfStackCommit;
            0x100000,                      // DWORD   SizeOfHeapReserve;
            0x1000,                        // DWORD   SizeOfHeapCommit;
            0,	                            // DWORD   LoaderFlags;
            0x10                           // DWORD   NumberOfRvaAndSizes;
            // IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
         }
   };


   int findDirective(const vector<SCommand> & commands, const string & name)
   {
      for (unsigned int i = 0; i < commands.size(); ++i)
      {
         if (commands[i].mType == NCommand::DIRECTIVE && commands[i].mDirective.mName == name)
         {
            return i;
         }
      }
      return -1;
   }

   int findSection(const vector<SCommand> & commands, const int startIndex)
   {
      for (unsigned int i = startIndex + 1; i < commands.size(); ++i)
      {
         if (commands[i].mType == NCommand::SECTION || commands[i].mType == NCommand::END)
         {
            return i;
         }
      }
      return -1;
   }

   IMAGE_SECTION_HEADER getSection(const vector<SCommand> & commands, const int sectionIndex, const DWORD sizeOfImage)
   {
      int a = sizeof(IMAGE_NT_HEADERS32);
      int b = sizeof(IMAGE_NT_HEADERS32);
      int c = sizeof IMAGE_FILE_HEADER;
      IMAGE_NT_HEADERS32 * pp = 0;
      void * p1 = &pp->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

      IMAGE_SECTION_HEADER result = { 0 };
      strncpy_s((char*)result.Name, sizeof(result.Name), commands[sectionIndex].mSection.mName.c_str(), sizeof(result.Name));
      
      const int nextSection = findSection(commands, sectionIndex);
      
      assert(nextSection != -1);

      result.PointerToRawData = commands[sectionIndex].mRAW;
      result.VirtualAddress = commands[sectionIndex].mRVA;
      result.SizeOfRawData = commands[nextSection].mRAW - commands[sectionIndex].mRAW;
      if (commands[nextSection].mType == NCommand::END && commands[nextSection].mRVA < sizeOfImage)
      {
         result.Misc.VirtualSize = sizeOfImage - commands[sectionIndex].mRVA;
      }
      else
      {
         result.Misc.VirtualSize = commands[nextSection].mRVA - commands[sectionIndex].mRVA;
      }

      result.Characteristics = 0;
      
      if (commands[sectionIndex].mSection.mAttributes & NSectionAttributes::READ)
      {
         result.Characteristics |= IMAGE_SCN_MEM_READ;
      }
      if (commands[sectionIndex].mSection.mAttributes & NSectionAttributes::WRITE)
      {
         result.Characteristics |= IMAGE_SCN_MEM_WRITE;
      }
      if (commands[sectionIndex].mSection.mAttributes & NSectionAttributes::EXECUTE)
      {
         result.Characteristics |= IMAGE_SCN_MEM_EXECUTE;
      }
      if (commands[sectionIndex].mSection.mAttributes & NSectionAttributes::CODE)
      {
         result.Characteristics |= IMAGE_SCN_CNT_CODE;
      }
      if (commands[sectionIndex].mSection.mAttributes & NSectionAttributes::INITIALIZED)
      {
         result.Characteristics |= IMAGE_SCN_CNT_INITIALIZED_DATA;
      }

      return result;
   }

   vector<IMAGE_SECTION_HEADER> getSections(const vector<SCommand> & commands, DWORD SizeOfImage)
   {
      vector<IMAGE_SECTION_HEADER> sections;

      for (unsigned int i = 0; i < commands.size(); ++i)
      {
         if (commands[i].mType == NCommand::SECTION)
         {
            sections.push_back(getSection(commands, i, SizeOfImage));
         }
      }
      return sections;
   }
}

   int gImageBase = 0;

   int getHeaderSize()
   {
      return 0x400; //TODO
   }

   void putHeader(ostream & output, const vector<SCommand> & commands, const SClientFile & clientFile)
   {
      const vector<IMAGE_SECTION_HEADER> & sections = getSections(commands, clientFile.mImageSize);

      if (sections.empty())
      {
         throw exception("No sections");
      }

      sPeHeader.FileHeader.NumberOfSections = sections.size();
      sPeHeader.OptionalHeader.SizeOfImage = sections.back().VirtualAddress + sections.back().Misc.VirtualSize;
      sPeHeader.OptionalHeader.ImageBase = clientFile.mImageBase;
      
      const int importPosition = findDirective(commands, "IMPORT_DIRECTORY");
      if (importPosition != -1)
      {
         sPeHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = commands[importPosition].mRVA;
         sPeHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = commands[importPosition].mDirective.mDirectorySize;
      }
      const int resourcePosition = findDirective(commands, "RECOURCE_DIRECTORY");
      if (resourcePosition != -1)
      {
         sPeHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = commands[resourcePosition].mRVA;
         sPeHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = commands[resourcePosition].mDirective.mDirectorySize;
      }

      output.write((char*)&sMzHeader, sizeof(sMzHeader));
      output.write((char*)&sPeHeader, sizeof(sPeHeader));

      for (unsigned int i = 0; i < sections.size(); ++i)
      {
         output.write((char*)&sections[i], sizeof(sections[0]));
      }
   }
}
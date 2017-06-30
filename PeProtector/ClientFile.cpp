#include "ClientFile.h"
#include "imagehlp.h"
#include "assert.h"
#include "..\aplib\aplib.h"
#include "crtdbg.h"

using std::exception;
using std::vector;

namespace NPeProtector
{
namespace
{
   class CFileMapping
   {
   public:
      CFileMapping(const std::string & fileName)
         : mFileHandle(INVALID_HANDLE_VALUE)
         , mMapppingHandle(INVALID_HANDLE_VALUE)
         , mPointer()
         , mNtHeaderPtr()
         , mSize()
      {
         mFileHandle = CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
         if (mFileHandle != INVALID_HANDLE_VALUE)
         {
            mMapppingHandle = CreateFileMapping(mFileHandle, 0, PAGE_READONLY | SEC_IMAGE, 0, 0, 0);
            if (mMapppingHandle != INVALID_HANDLE_VALUE)
            {
               mPointer = MapViewOfFile(mMapppingHandle, FILE_MAP_READ, 0, 0, 0);
               if (mPointer != 0)
               {
                  mNtHeaderPtr = ImageNtHeader((void*)mPointer);
                  if (mNtHeaderPtr != 0)
                  {
                     mSize = mNtHeaderPtr->OptionalHeader.SizeOfImage;
                  }
                  else
                  {
                     throw exception("CFileMapping::ImageNtHeader returns error");
                  }
               }
               else
               {
                  throw exception("CFileMapping::MapViewOfFile returns error");
               }
            }
            else
            {
               throw exception("CFileMapping::CreateFileMapping returns error");
            }
         }
         else
         {
            throw exception("CFileMapping::CreateFileA returns error");
         }
      }

      const void * getPointer() const
      {
         return mPointer;
      }

      const IMAGE_NT_HEADERS & getNtHeader() const
      {
         assert(mNtHeaderPtr != 0);

         return *mNtHeaderPtr;
      }

      int getSize() const
      {
         return mSize;
      }

      ~CFileMapping()
      {
         if (mPointer != 0)
         {
            UnmapViewOfFile(mPointer);
         }
         if (mMapppingHandle != INVALID_HANDLE_VALUE)
         {
            CloseHandle(mMapppingHandle);
         }
         if (mFileHandle != INVALID_HANDLE_VALUE)
         {
            CloseHandle(mFileHandle);
         }
      }
   private:

      HANDLE mFileHandle;
      HANDLE mMapppingHandle;
      const void * mPointer;
      const IMAGE_NT_HEADERS * mNtHeaderPtr;
      int    mSize;
   };


   const IMAGE_RESOURCE_DIRECTORY_ENTRY * getDirectoryEntry(const IMAGE_RESOURCE_DIRECTORY * directory)
   {
      return reinterpret_cast<const IMAGE_RESOURCE_DIRECTORY_ENTRY *>(directory + 1);
   };

   const IMAGE_RESOURCE_DIRECTORY * findResource(void * base, const IMAGE_RESOURCE_DIRECTORY * const directory, const int id)
   {
      const IMAGE_RESOURCE_DIRECTORY_ENTRY * entries = getDirectoryEntry(directory);

      for (int i = directory->NumberOfNamedEntries; i < directory->NumberOfNamedEntries + directory->NumberOfIdEntries; i++)
      {
         if (entries[i].Id == id)
         {
            return (IMAGE_RESOURCE_DIRECTORY *)((char*)base + entries[i].OffsetToDirectory);
         }
      }
      return 0;
   }
   
   vector<vector<char> > getAllResource(const CFileMapping & fileMapping, const int id)
   {
      vector<vector<char> > result;
      // get resource directory
      const DWORD rvaResource = fileMapping.getNtHeader().OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
      if (rvaResource != 0)
      {
         const IMAGE_RESOURCE_DIRECTORY * const resource = (IMAGE_RESOURCE_DIRECTORY *)((char *)fileMapping.getPointer() + rvaResource);
         const IMAGE_RESOURCE_DIRECTORY * const iconDirectoryL2 = findResource((void*)resource, resource, id /*3 RT_ICON*/);
         if (iconDirectoryL2 != 0)
         {
            try
            {
               const IMAGE_RESOURCE_DIRECTORY_ENTRY * iconEntryL2 = getDirectoryEntry(iconDirectoryL2);
               for (int i = iconDirectoryL2->NumberOfNamedEntries; i < iconDirectoryL2->NumberOfIdEntries + iconDirectoryL2->NumberOfNamedEntries; ++i)
               {
                  const IMAGE_RESOURCE_DIRECTORY * iconDirectoryL3 = (IMAGE_RESOURCE_DIRECTORY *)((char *)resource + iconEntryL2->OffsetToDirectory);
                  const IMAGE_RESOURCE_DIRECTORY_ENTRY * iconEntryL3 = getDirectoryEntry(iconDirectoryL3);
                  IMAGE_RESOURCE_DATA_ENTRY * dataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)((char *)resource + iconEntryL3->OffsetToDirectory);

                  // dataEntry->OffsetToData = RVA
                  const char * data = ((char *)fileMapping.getPointer() + dataEntry->OffsetToData);
                  const int size = dataEntry->Size;

                  result.push_back(vector<char>(data, data + size));

                  iconEntryL2++;
               }
            }
            catch (...)
            {
               throw exception("Failed to get resources");
            }
         }
      }
      return result;
   }
   
   // get first resource!
   std::vector<char> getFirstResource(const CFileMapping & fileMapping, const int id)
   {
      std::vector<char> result;
      // get resource directory
      const DWORD rvaResource = fileMapping.getNtHeader().OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
      if (rvaResource != 0)
      {
         const IMAGE_RESOURCE_DIRECTORY * const resource = (IMAGE_RESOURCE_DIRECTORY *)((char *)fileMapping.getPointer() + rvaResource);
         const IMAGE_RESOURCE_DIRECTORY * const iconDirectoryL2 = findResource((void*)resource, resource, id /*3 RT_ICON*/);
         if (iconDirectoryL2 != 0)
         {
            try
            {
               const IMAGE_RESOURCE_DIRECTORY_ENTRY * iconEntryL2 = getDirectoryEntry(iconDirectoryL2);
               const IMAGE_RESOURCE_DIRECTORY * iconDirectoryL3 = (IMAGE_RESOURCE_DIRECTORY *)((char *)resource + iconEntryL2->OffsetToDirectory);
               const IMAGE_RESOURCE_DIRECTORY_ENTRY * iconEntryL3 = getDirectoryEntry(iconDirectoryL3);
               IMAGE_RESOURCE_DATA_ENTRY * dataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)((char *)resource + iconEntryL3->OffsetToDirectory);

               // dataEntry->OffsetToData = RVA
               const char * data = ((char *)fileMapping.getPointer() + dataEntry->OffsetToData);
               const int size = dataEntry->Size;

               result.insert(result.end(), data, data + size);
            }
            catch (...)
            {
               throw exception("Failed to get resources");
            }
         }
      }
      return result;
   }

   vector<char> compressFile(const CFileMapping & file)
   {
      /* allocate workmem and destination memory */
      char * workmem = (char *)malloc(aP_workmem_size(file.getSize()));
      char * compressed = (char *)malloc(aP_max_packed_size(file.getSize()));

      /* compress data[] to compressed[] */
      size_t outlength = aP_pack(file.getPointer(), compressed, file.getSize(), workmem, NULL, NULL);

      /* if APLIB_ERROR is returned, and error occured */
      if (outlength == APLIB_ERROR)
      {
         throw exception("aplib: an error occured!\n");
      }

      // copy
      vector<char> result(compressed, compressed + outlength);

      free(workmem);
      free(compressed);

      return result;
   }
}

   SClientFile getPeFileInfo(const char * filename)
   {
      SClientFile result;
      
      CFileMapping fileMapping(filename);
      if (fileMapping.getNtHeader().OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
      {
         if (fileMapping.getNtHeader().OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress == 0)
         {
            if (fileMapping.getNtHeader().OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress == 0)
            {
               result.mIcons = getAllResource(fileMapping, 3/*icon*/);
               result.mManifest = getFirstResource(fileMapping, 0x18/*manifest*/);
               result.mGroupIcons = getFirstResource(fileMapping, 0x0E/*group icons*/);
               result.mCompressed = compressFile(fileMapping);
               result.mImageBase = fileMapping.getNtHeader().OptionalHeader.ImageBase;
               result.mImageSize = fileMapping.getNtHeader().OptionalHeader.SizeOfImage;
               result.mOEP = fileMapping.getNtHeader().OptionalHeader.AddressOfEntryPoint;
            }
            else
            {
               throw exception("PeProtector doesn't support files with TLS");
            }
         }
         else
         {
            throw exception("IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR != 0");
         }
      }
      else
      {
         throw exception("OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC");
      }
      return result;
   }
}

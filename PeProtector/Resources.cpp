#include "Resources.h"
#include <ostream>
#include "algorithm"
#include "assert.h"

using std::ostream;
using std::vector;

namespace NPeProtector
{
   //TODO find good place!
   inline int align(int value, int al)
   {
      if (value % al)
      {
         value += al - (value % al);
      }
      return value;
   }

   class CEntry
   {
   public:
      enum EDirectoryType
      {
         ROOT,
         LEVEL_1,
         LEVEL_2,
         LEVEL_3, // keep DATA_ENTRY
         DATA_ENTRY = LEVEL_3,
      };

      CEntry(EDirectoryType level, int id)
         : mLevel(level)
         , mId(id)
         , mSelf()
         , mChildren()
      {
      }

      CEntry * get(int id)
      {
         for (unsigned int i = 0; i < mChildren.size(); ++i)
         {
            if (mChildren[i]->mId == id)
            {
               return mChildren[i];
            }
         }
         return 0;
      }

      void addChild(CEntry * entry)
      {
         mChildren.push_back(entry);
      }

      void setData(const std::vector<char> & data)
      {
         assert(mChildren.empty());

         mData = data;
      }

      void compile(char * buffer, DWORD offset/*from root resource*/, DWORD rva/*from root resource*/)
      {
         if (mLevel == DATA_ENTRY)
         {
            if (mSelf != 0)
            {
               mSelf->Id = mId;
               mSelf->DataIsDirectory = 0;
               mSelf->OffsetToDirectory = offset;
            }
            IMAGE_RESOURCE_DATA_ENTRY * dataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)buffer;
            dataEntry->OffsetToData = rva + offset + sizeof(IMAGE_RESOURCE_DATA_ENTRY);
            dataEntry->Size = mData.size();

            buffer += sizeof(IMAGE_RESOURCE_DATA_ENTRY);

            memcpy(buffer, &mData.front(), mData.size());
         }
         else
         {
            if (mSelf != 0)
            {
               mSelf->Id = mId;
               mSelf->DataIsDirectory = 1;
               mSelf->OffsetToDirectory = offset;
            }
            IMAGE_RESOURCE_DIRECTORY * directory = (IMAGE_RESOURCE_DIRECTORY *)buffer;
            directory->NumberOfIdEntries = mChildren.size();

            IMAGE_RESOURCE_DIRECTORY_ENTRY * directoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)(directory + 1);
            for (unsigned int i = 0; i < mChildren.size(); ++i)
            {
               mChildren[i]->mSelf = directoryEntry;
               directoryEntry++;
            }
         }
      }

      int getSize()
      {
         if (mLevel == DATA_ENTRY)
         {
            return sizeof(IMAGE_RESOURCE_DATA_ENTRY) + align(mData.size(), 0x10);
         }
         else
         {
            return sizeof(IMAGE_RESOURCE_DIRECTORY) + sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) * mChildren.size();
         }
      }

      bool operator<(const CEntry & right) const
      {
         return mLevel < right.mLevel;
      }
      friend bool isLess(CEntry * left, CEntry * right);
   private:
      const EDirectoryType mLevel;
      int mId;
      IMAGE_RESOURCE_DIRECTORY_ENTRY * mSelf; // parent should set it!
      std::vector<CEntry*> mChildren;
      std::vector<char> mData;
   };

   bool isLess(CEntry * left, CEntry * right)
   {
      return left->mLevel < right->mLevel;
   }

   class CResourceDirectory
   {
   public:

      CResourceDirectory()
         : mRoot(new CEntry(CEntry::ROOT, 0))
      {
         mList.push_back(mRoot);
      }

      void add(int id1, int id2, int id3, const std::vector<char> & data)
      {
         CEntry * entry1 = mRoot->get(id1);

         if (entry1 == 0)
         {
            entry1 = new CEntry(CEntry::LEVEL_1, id1);
            mRoot->addChild(entry1);
            mList.push_back(entry1);
         }

         CEntry * entry2 = entry1->get(id2);
         if (entry2 == 0)
         {
            entry2 = new CEntry(CEntry::LEVEL_2, id2);
            entry1->addChild(entry2);
            mList.push_back(entry2);
         }

         CEntry * entry3 = entry2->get(id3);
         if (entry3 == 0)
         {
            entry3 = new CEntry(CEntry::LEVEL_3, id3);
            entry2->addChild(entry3);
            mList.push_back(entry3);
         }
         entry3->setData(data);
      }

      std::vector<char> compile(const DWORD rva)
      {
         const int bufferSize = getSize();
         char * const buffer = new char[bufferSize];
         memset(buffer, 0, bufferSize);
         int offset = 0;
         for (unsigned int i = 0; i < mList.size(); ++i)
         {
            mList[i]->compile(buffer + offset, offset, rva);
            offset += mList[i]->getSize();
         }
         
         assert(offset == bufferSize);

         vector<char> result(buffer, buffer + bufferSize);

         delete[] buffer;

         return result;
      }

      int getSize()
      {
         std::sort(mList.begin(), mList.end(), isLess);
         int result = 0;
         for (unsigned int i = 0; i < mList.size(); ++i)
         {
            result += mList[i]->getSize();
         }
         return result;
      }

   private:
      void sort()
      {
         std::sort(mList.begin(), mList.end(), isLess);
      }
      CEntry * mRoot;
      std::vector<CEntry*> mList; //sort by level;
   };

   int getResourcesSize(const SClientFile & clientFile)
   {
      CResourceDirectory resourceDirectory;
      if (!clientFile.mIcons.empty())
      {
         for (unsigned int i = 0; i < clientFile.mIcons.size(); ++i)
         {
            resourceDirectory.add(3/*icon*/, i + 1, 0x409, clientFile.mIcons[i]);
         }
      }
      if (!clientFile.mGroupIcons.empty())
      {
         resourceDirectory.add(0x0E/*group icon*/, 1, 0x409, clientFile.mGroupIcons);
      }
      if (!clientFile.mManifest.empty())
      {
         resourceDirectory.add(0x18/*manifest*/, 1, 0x409, clientFile.mManifest);
      }
      return resourceDirectory.getSize();
   }

   void putResources(ostream & output, const vector<SCommand> & commands, const SClientFile & clientFile, const DWORD rva)
   {
      CResourceDirectory resourceDirectory;
      for (unsigned int i = 0; i < clientFile.mIcons.size(); ++i)
      {
         resourceDirectory.add(3/*icon*/, i + 1, 0x409, clientFile.mIcons[i]);
      }
      if (!clientFile.mGroupIcons.empty())
      {
         resourceDirectory.add(0x0E/*group icon*/, 1, 0x409, clientFile.mGroupIcons);
      }
      if (!clientFile.mManifest.empty())
      {
         resourceDirectory.add(0x18/*manifest*/, 1, 0x409, clientFile.mManifest);
      }

      const vector<char> & resources = resourceDirectory.compile(rva);

      if (!resources.empty())
      {
         output.write(&resources.front(), resources.size());
      }
   }
}
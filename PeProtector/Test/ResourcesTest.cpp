#include "CppUnitTest.h"
#include "../PeProtector/ClientFile.h"
#include "../PeProtector/Resources.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace NPeProtector;

namespace Test
{
	TEST_CLASS(ResourcesTest)
	{
	public:
		
      TEST_METHOD(testGetResourcesSizeEmpty)
		{
         SClientFile clientFile;
         const int size = getResourcesSize(clientFile);
         Assert::IsTrue(size == 16);
		}

      TEST_METHOD(testGetResourcesSizeManifest)
      {
         SClientFile clientFile;
         clientFile.mManifest = { 1, 2, 3 };

         const int size = getResourcesSize(clientFile);
         Assert::IsTrue(size == 104);
      }

      TEST_METHOD(testGetResourcesSizeIconds)
      {
         SClientFile clientFile;
         clientFile.mIcons = { { 1, 2, 3 }, { 4, 5, 6 } };
         clientFile.mGroupIcons = { 1, 2, 3 };

         const int size = getResourcesSize(clientFile);
         Assert::IsTrue(size == 256);
      }

      TEST_METHOD(testGetResourcesSizeAll)
      {
         SClientFile clientFile;
         clientFile.mIcons = { { 1, 2, 3 }, { 4, 5, 6 } };
         clientFile.mGroupIcons = { 1, 2, 3 };
         clientFile.mManifest = { 1, 2, 3 };

         const int size = getResourcesSize(clientFile);
         Assert::IsTrue(size == 344);
      }
   };
}

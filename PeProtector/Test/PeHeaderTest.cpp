#include "CppUnitTest.h"
#include "../PeProtector/PeHeader.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace NPeProtector;

namespace Test
{
	TEST_CLASS(PeHeaderTest)
	{
	public:
		
      TEST_METHOD(testGetHeaderSize)
		{
         Assert::IsTrue(0x400 == getHeaderSize());
		}
	};
}

#include "CppUnitTest.h"
#include "../Library/SCommand.h"
#include "../PeProtector/Data.h"
#include "iostream"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace NPeProtector;

namespace Test
{
	TEST_CLASS(DataTest)
	{
	public:
		
		TEST_METHOD(testGetDataSize1)
		{
         std::vector<SLabel> labels;
         std::vector<SConstant> constants = { SConstant(labels, 10) };

         SData data("", 1, constants, 2);

         const int size = getDataSize(data);

         Assert::IsTrue(size == 2);
		}

      TEST_METHOD(testGetDataSize2)
      {
         std::vector<SLabel> labels;
         std::vector<SConstant> constants = { SConstant(labels, 10) };

         SData data("", 4, constants, 2);

         const int size = getDataSize(data);

         Assert::IsTrue(size == 8);
      }

      TEST_METHOD(testPutData1)
      {
         std::stringstream expectedResult;
         char characters[] = { 33 };
         expectedResult.write(characters, sizeof(characters));

         std::vector<SLabel> labels;
         std::vector<SConstant> constants = { SConstant(labels, 33) };
         SData data("", 1, constants, 1);

         std::stringstream result;

         putData(result, data, std::vector<SCommand>());

         Assert::IsTrue(result.rdbuf()->str() == expectedResult.rdbuf()->str());
      }

      TEST_METHOD(testPutData2)
      {
         std::stringstream expectedResult;
         char characters[] = {33, 33, 33};
         expectedResult.write(characters, sizeof(characters));

         std::vector<SLabel> labels;
         std::vector<SConstant> constants = { SConstant(labels, 33) };
         SData data("", 1, constants, 3);

         std::stringstream result;

         putData(result, data, std::vector<SCommand>());

         Assert::IsTrue(result.rdbuf()->str() == expectedResult.rdbuf()->str());
      }

      TEST_METHOD(testPutData3)
      {
         std::stringstream expectedResult;
         char characters[] = { 33, 34, 35 };
         expectedResult.write(characters, sizeof(characters));

         std::vector<SLabel> labels;
         std::vector<SConstant> constants = { SConstant(labels, 33), SConstant(labels, 34), SConstant(labels, 35) };
         SData data("", 1, constants, 1);

         std::stringstream result;

         putData(result, data, std::vector<SCommand>());

         Assert::IsTrue(result.rdbuf()->str() == expectedResult.rdbuf()->str());
      }

      TEST_METHOD(testPutData4)
      {
         std::stringstream expectedResult;
         DWORD characters[] = { 33, 34, 35 };
         expectedResult.write((char*)characters, sizeof(characters));

         std::vector<SLabel> labels;
         std::vector<SConstant> constants = { SConstant(labels, 33), SConstant(labels, 34), SConstant(labels, 35) };
         SData data("", 4, constants, 1);

         std::stringstream result;

         putData(result, data, std::vector<SCommand>());

         Assert::IsTrue(result.rdbuf()->str() == expectedResult.rdbuf()->str());
      }

   };
}

#include "CppUnitTest.h"
#include "../Library/SCommand.h"
#include "../PeProtector/Import.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace NPeProtector;

namespace Test
{
	TEST_CLASS(ImportTest)
	{
	public:
		
      TEST_METHOD(testResolveImport1)
      {
         SCommand command1;
         command1.mType = NCommand::IMPORT;
         command1.mImport.mDllName = "dll1";
         command1.mImport.mFunctionName = "func1";

         std::vector<SCommand> commands = { command1, };

         resolveImport(commands, 0);

         Assert::IsTrue(commands[0].mRVA == 48);
      }

      TEST_METHOD(testResolveImport2)
		{
         SCommand command1;
         command1.mType = NCommand::IMPORT;
         command1.mImport.mDllName = "dll1";
         command1.mImport.mFunctionName = "func1";
         
         SCommand command2;
         command2.mType = NCommand::IMPORT;
         command2.mImport.mDllName = "dll2";
         command2.mImport.mFunctionName = "func2";

         std::vector<SCommand> commands = {command1, command2};

         resolveImport(commands, 0);

         Assert::IsTrue(commands[0].mRVA == 76);
         Assert::IsTrue(commands[1].mRVA == 84);
		}

      TEST_METHOD(testGetImportSize1)
      {
         SCommand command1;
         command1.mType = NCommand::IMPORT;
         command1.mImport.mDllName = "dll1";
         command1.mImport.mFunctionName = "func1";

         std::vector<SCommand> commands = { command1};

         const int size = getImportSize(commands);

         Assert::IsTrue(size == 69);
      }

      TEST_METHOD(testGetImportSize2)
      {
         SCommand command1;
         command1.mType = NCommand::IMPORT;
         command1.mImport.mDllName = "dll1";
         command1.mImport.mFunctionName = "func1";

         SCommand command2;
         command2.mType = NCommand::IMPORT;
         command2.mImport.mDllName = "dll2";
         command2.mImport.mFunctionName = "func2";

         std::vector<SCommand> commands = { command1, command2 };

         const int size = getImportSize(commands);

         Assert::IsTrue(size == 118);
      }
   };
}


#include "CppUnitTest.h"
#include "../Library/SCommand.h"
#include "../Compile/CCompile.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace NPeProtector;

namespace Test
{		
	TEST_CLASS(CompileTest)
	{
	public:
		
      TEST_METHOD(testCompileComments)
      {
         std::stringstream input;
         input << ";23\n  ;1\n  ;12";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands.size() == 1);
         Assert::IsTrue(commands[0].mType == NCommand::END);
      }

		TEST_METHOD(testCompileImport)
		{
         std::stringstream input;
         input << "IMPORT KERNEL32.GetTickCount";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::IMPORT);
         Assert::IsTrue(commands[0].mImport.mDllName == "KERNEL32.dll");
         Assert::IsTrue(commands[0].mImport.mFunctionName == "GetTickCount");
		}

      TEST_METHOD(testCompileData1)
      {
         std::stringstream input;
         input << "name DD 123";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::DATA);
         Assert::IsTrue(commands[0].mData.mName == "name");
         Assert::IsTrue(commands[0].mData.mCount == 1);
         Assert::IsTrue(commands[0].mData.mConstants[0].mValue == 123);
         Assert::IsTrue(commands[0].mData.mSizeData == 4);
      }

      TEST_METHOD(testCompileData2)
      {
         std::stringstream input;
         input << "name DWORD 123";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::DATA);
         Assert::IsTrue(commands[0].mData.mName == "name");
         Assert::IsTrue(commands[0].mData.mCount == 1);
         Assert::IsTrue(commands[0].mData.mConstants[0].mValue == 123);
         Assert::IsTrue(commands[0].mData.mSizeData == 4);
      }

      TEST_METHOD(testCompileDataString)
      {
         std::stringstream input;
         input << "szLoadLibrary DB \"LoadLibraryA\"";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::DATA);
         Assert::IsTrue(commands[0].mData.mName == "szLoadLibrary");
         Assert::IsTrue(commands[0].mData.mCount == 1);
         Assert::IsTrue(commands[0].mData.mConstants.size() == 12);
         Assert::IsTrue(commands[0].mData.mConstants[0].mValue == (DWORD)'L');
         Assert::IsTrue(commands[0].mData.mConstants[1].mValue == (DWORD)'o');
         Assert::IsTrue(commands[0].mData.mConstants[2].mValue == (DWORD)'a');
         Assert::IsTrue(commands[0].mData.mConstants[3].mValue == (DWORD)'d');
         Assert::IsTrue(commands[0].mData.mSizeData == 1);
      }

      TEST_METHOD(testCompileDataCount)
      {
         std::stringstream input;
         input << "name DB 1, 2, 10h";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::DATA);
         Assert::IsTrue(commands[0].mData.mName == "name");
         Assert::IsTrue(commands[0].mData.mCount == 1);
         Assert::IsTrue(commands[0].mData.mConstants[0].mValue == 1);
         Assert::IsTrue(commands[0].mData.mConstants[1].mValue == 2);
         Assert::IsTrue(commands[0].mData.mConstants[2].mValue == 0x10);
         Assert::IsTrue(commands[0].mData.mSizeData == 1);
      }

      TEST_METHOD(testCompileDataDup)
      {
         std::stringstream input;
         input << "testDup db 30 dup (80h)";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::DATA);
         Assert::IsTrue(commands[0].mData.mName == "testDup");
         Assert::IsTrue(commands[0].mData.mConstants.size() == 1);
         Assert::IsTrue(commands[0].mData.mConstants[0].mValue == 0x80);
         Assert::IsTrue(commands[0].mData.mSizeData == 1);
      }
      TEST_METHOD(testCompileSection)
      {
         std::stringstream input;
         input << "SECTION \".text\" crwei";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::SECTION);
         Assert::IsTrue(commands[0].mSection.mName == ".text");
         Assert::IsTrue((commands[0].mSection.mAttributes & NSectionAttributes::CODE) != 0);
         Assert::IsTrue((commands[0].mSection.mAttributes & NSectionAttributes::READ) != 0);
         Assert::IsTrue((commands[0].mSection.mAttributes & NSectionAttributes::EXECUTE) != 0);
         Assert::IsTrue((commands[0].mSection.mAttributes & NSectionAttributes::WRITE) != 0);
         Assert::IsTrue((commands[0].mSection.mAttributes & NSectionAttributes::INITIALIZED) != 0);
      }

      TEST_METHOD(testCompileInstruction)
      {
         std::stringstream input;
         input << "MOV EAX, DWORD PTR [EAX + 0CH]";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::INSTRUCTION);
         Assert::IsTrue(commands[0].mInstruction.mType == NInstruction::MOV);
         Assert::IsTrue(commands[0].mInstruction.mOperands.size() == 2);
         Assert::IsTrue(commands[0].mInstruction.mOperands[0].mType == NOperand::REG32);
         Assert::IsTrue(commands[0].mInstruction.mOperands[0].mRegister == NRegister::EAX);
         Assert::IsTrue(commands[0].mInstruction.mOperands[1].mType == NOperand::MEM32);
         Assert::IsTrue(commands[0].mInstruction.mOperands[1].mMemory.mRegisters[0] == NRegister::EAX);
         Assert::IsTrue(commands[0].mInstruction.mOperands[1].mMemory.mConstant.mValue == 0x0C);
      }

      TEST_METHOD(testCompileExtern)
      {
         std::stringstream input;
         input << "EXTERN DD externImageBase";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::EXTERN);
         Assert::IsTrue(commands[0].mNameLabel == "externImageBase");
         Assert::IsTrue(commands[0].mData.mSizeData == 4);
      }

      TEST_METHOD(testCompileLabel1)
      {
         std::stringstream input;
         input << "labelName: NOP";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::INSTRUCTION);
         Assert::IsTrue(commands[0].mNameLabel == "labelName");
         Assert::IsTrue(commands[0].mInstruction.mType == NInstruction::NOP);
      }

      TEST_METHOD(testCompileLabel2)
      {
         std::stringstream input;
         input << "labelName: \n\n NOP";

         const std::vector<SCommand> & commands = compile(input);

         Assert::IsTrue(commands[0].mType == NCommand::INSTRUCTION);
         Assert::IsTrue(commands[0].mNameLabel == "labelName");
         Assert::IsTrue(commands[0].mInstruction.mType == NInstruction::NOP);
      }

   };
}

#include "CppUnitTest.h"
#include "../Library/SCommand.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace NPeProtector;

namespace Test
{
	TEST_CLASS(CommandTest)
	{
	public:

      TEST_METHOD(testSerializationExtern)
      {
         SCommand command;
         command.mType = NCommand::EXTERN;
         command.mNameLabel = "externImageBase";
         command.mData.mSizeData = 4;

         std::stringstream result;
         serialize(result, command);

         SCommand command2;
         deserialize(result, command2);

         Assert::IsTrue(command.mType == command2.mType);
         Assert::IsTrue(command.mNameLabel == command2.mNameLabel);
         Assert::IsTrue(command.mData.mSizeData == command2.mData.mSizeData);
      }

      TEST_METHOD(testSerializationImport)
      {
         SCommand command;
         command.mType = NCommand::IMPORT;
         command.mImport.mDllName = "KERNEL32.DLL";
         command.mImport.mFunctionName = "f3";

         std::stringstream result;
         serialize(result, command);

         SCommand command2;
         deserialize(result, command2);

         Assert::IsTrue(command.mType == command2.mType);
         Assert::IsTrue(command.mImport.mDllName == command2.mImport.mDllName);
         Assert::IsTrue(command.mImport.mFunctionName == command2.mImport.mFunctionName);
      }

      TEST_METHOD(testSerializationData)
      {
         SCommand command;
         command.mType = NCommand::DATA;

         std::vector<SLabel> labels;
         std::vector<SConstant> constants = { SConstant(labels, 33), SConstant(labels, 34), SConstant(labels, 35) };
         command.mData = SData("", 4, constants, 1);

         std::stringstream result;
         serialize(result, command);

         SCommand command2;
         deserialize(result, command2);

         Assert::IsTrue(command.mType == command2.mType);
         Assert::IsTrue(command.mData.mSizeData == command2.mData.mSizeData);
      }

      TEST_METHOD(testSerializationInstruction)
      {
         SCommand command;
         command.mType = NCommand::INSTRUCTION;

         SOperand operand1;
         operand1.mType = NOperand::REG8;
         operand1.mRegister = NRegister::CH;

         SOperand operand2;
         operand2.mType = NOperand::CONSTANT;
         operand2.mConstant.mValue = 0x88;

         command.mInstruction = SInstruction(NPrefix::NON, NInstruction::ADD, { operand1, operand2 });

         std::stringstream result;
         serialize(result, command);

         SCommand command2;
         deserialize(result, command2);

         Assert::IsTrue(command.mType == command2.mType);
         Assert::IsTrue(command.mInstruction.mType == command2.mInstruction.mType);
         Assert::IsTrue(command.mInstruction.mOperands[0].mType == command2.mInstruction.mOperands[0].mType);
         Assert::IsTrue(command.mInstruction.mOperands[1].mType == command2.mInstruction.mOperands[1].mType);
      }
	};
}
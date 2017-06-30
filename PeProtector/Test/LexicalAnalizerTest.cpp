#include "CppUnitTest.h"
#include "../Compile/CLexicalAnalizer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace NPeProtector;

TEST_CLASS(LexicalAnalizerTest)
{
public:

   TEST_METHOD(testParseComma1)
   {
      std::stringstream input;
      input << ",";

      const std::vector<std::vector<SToken> > tokens = parse(input);
      Assert::IsTrue(tokens[0][0].mType == NCategory::COMMA);
   }

   TEST_METHOD(testParseComma2)
   {
      std::stringstream input;
      input << ",123";

      const std::vector<std::vector<SToken> > tokens = parse(input);
      Assert::IsTrue(tokens[0][0].mType == NCategory::COMMA);
      Assert::IsTrue(tokens[0][1].mType == NCategory::CONSTANT);
   }

   TEST_METHOD(testParseComma3)
   {
      std::stringstream input;
      input << "123,";

      const std::vector<std::vector<SToken> > tokens = parse(input);
      Assert::IsTrue(tokens[0][0].mType == NCategory::CONSTANT);
      Assert::IsTrue(tokens[0][1].mType == NCategory::COMMA);
   }
   
   TEST_METHOD(testParseKeywords)
   {
      std::stringstream input;
      input << "IMPORT EXTERN DUP PTR SECTION DIRECTIVE";

      const std::vector<std::vector<SToken> > tokens = parse(input);
      Assert::IsTrue(tokens[0][0].mType == NCategory::IMPORT);
      Assert::IsTrue(tokens[0][1].mType == NCategory::EXTERN);
      Assert::IsTrue(tokens[0][2].mType == NCategory::DUP);
      Assert::IsTrue(tokens[0][3].mType == NCategory::PTR);
      Assert::IsTrue(tokens[0][4].mType == NCategory::SECTION);
      Assert::IsTrue(tokens[0][5].mType == NCategory::DIRECTIVE);
   }
   
   TEST_METHOD(testParseTrancate)
   {
      std::stringstream input;
      input << "  1 \n  2 \n  3";

      const std::vector<std::vector<SToken> > tokens = parse(input);
      Assert::IsTrue(tokens[0][0].mType == NCategory::CONSTANT);
      Assert::IsTrue(tokens[1][0].mType == NCategory::CONSTANT);
      Assert::IsTrue(tokens[2][0].mType == NCategory::CONSTANT);
   }

   TEST_METHOD(testParseName)
   {
      std::stringstream input;
      input << "_name";

      const std::vector<std::vector<SToken> > tokens = parse(input);
      Assert::IsTrue(tokens[0][0].mType == NCategory::NAME);
      Assert::IsTrue(tokens[0][0].mData =="_name");
   }

   TEST_METHOD(testParseString)
   {
      std::stringstream input;
      input << "\"0-0\"";

      const std::vector<std::vector<SToken> > tokens = parse(input);
      Assert::IsTrue(tokens[0][0].mType == NCategory::STRING);
      Assert::IsTrue(tokens[0][0].mData == "0-0");
   }

   TEST_METHOD(testParseConstant)
   {
      std::stringstream input;
      input << "555";

      const std::vector<std::vector<SToken> > tokens = parse(input);
      Assert::IsTrue(tokens[0][0].mType == NCategory::CONSTANT);
      Assert::IsTrue(tokens[0][0].mConstant == 555);
   }
};
#include "CCompile.h"
#include "CLexicalAnalizer.h"
#include <iosfwd>
#include <sstream> 
#include "../LogLibrary/CLog.h"
#include <assert.h>

using std::string;
using std::vector;
using std::exception;
using std::basic_istream;
using std::char_traits;
using std::ostringstream;

namespace NPeProtector
{
namespace
{
   vector<SToken> getSubTokens(const vector<SToken> & tokens, int firstIndex, int length = -1);
   vector<SToken> getSubTokens(const vector<SToken> & tokens, int firstIndex, NCategory::EType type);

   int findLabel(const vector<SCommand> & commands, const string & label);
   int getDataSize(NDataType::EType dataType);
   int getScale(int value);
   int getDataTypeSize(NDataType::EType dataType);
   int getAttributes(const string & attributes);
   SConstant createConstant(const vector<SCommand> & commands, const vector<SToken> & tokens);

   SCommand createImport(const string & label, int numberLine, const string & dllName, const string & functionName);
   SCommand createExtern(const string & label, int numberLine, NDataType::EType dataType, const string & name);
   SCommand createDirective(const string & label, int numberLine, const string & name);
   SCommand createSection(const string & label, int numberLine, const string & nameSection, const string & attributes);
   SCommand createDummyCommand(const string & label, int numberLine);

   SOperand createOperandRegister(NRegister::EType reg);
   SOperand createOperandMemory(const vector<SCommand> & commands, const vector<SToken> & tokens, NSegment::EType segment, NDataType::EType dataType);
   SOperand createOperandConstant(const vector<SCommand> & commands, const vector<SToken> & tokens);
   SOperand createOperand(const vector<SCommand> & commands, const vector<SToken> & tokens);

   void fillInstruction(SCommand & command, const vector<SCommand> & commands, const vector<SToken> & tokens);
   
   vector<SConstant> createDataString(const vector<SToken> & tokens);
   vector<SConstant> createDataConstants(const vector<SCommand> & commands, const vector<SToken> & tokens);

   void fillData(SCommand & command, const vector<SCommand> & commands, const vector<SToken> & tokens);
   
   // implementations

   int findToken(const vector<SToken> & tokens, const int firstIndex, const NCategory::EType type)
   {
      for (unsigned int i = firstIndex; i < tokens.size(); ++i)
      {
         if (tokens[i].mType == type)
         {
            return i;
         }
      }
      return -1;
   }

   vector<SToken> getSubTokens(const vector<SToken> & tokens, const int firstIndex, const int length /*= -1*/) // подумать и сделать тут -1
   {
      return vector<SToken>(tokens.begin() + firstIndex, length == -1 ? tokens.end() : tokens.begin() + firstIndex + length);
   }

   vector<SToken> getSubTokens(const vector<SToken> & tokens, const int firstIndex, const NCategory::EType type)
   {
      for (unsigned int i = firstIndex; i < tokens.size(); ++i)
      {
         if (tokens[i].mType == type)
         {
            return getSubTokens(tokens, firstIndex/*startPosition*/, i - firstIndex/*length*/);
         }
      }
      //return tokens;
      return getSubTokens(tokens, firstIndex/*startPosition*/, -1/*length*/);
   }

   int findLabel(const vector<SCommand> & commands, const string & label)
   {
      for (unsigned int i = 0; i < commands.size(); ++i)
      {
         if (commands[i].mNameLabel == label ||
            (commands[i].mType == NCommand::DATA && commands[i].mData.mName == label)) //todo check case sensitive
         {
            return i;
         }
      }
      throw exception(("label \"" + label + "\" doesn't exist").c_str());
   }

   int getDataTypeSize(const NDataType::EType dataType)
   {
      switch (dataType)
      {
      case NDataType::DD:
      case NDataType::DWORD:
         return 4;
      case NDataType::DW:
      case NDataType::WORD:
         return 2;
      case NDataType::DB:
      case NDataType::BYTE:
         return 1;
      }
      return 0;
   }

   // we receive here: sign label sign label sign const
   SConstant createConstant(const vector<SCommand> & commands, const vector<SToken> & tokens)
   {
      SConstant result;

      if (!tokens.empty())
      {
         unsigned int i = 0;
         if (tokens[i].mType == NCategory::NAME)
         {
            result.mLabels.push_back(SLabel(NSign::PLUS, findLabel(commands, tokens[i].mData)));
            ++i;
         }
         else if (tokens[i].mType == NCategory::CONSTANT)
         {
            result.mValue = tokens[i].mConstant;
            ++i;
         }

         for (; i < tokens.size(); ++i)
         {
            NSign::EType sign;
            if (tokens[i].mType == NCategory::MINUS)
            {
               sign = NSign::MINUS;
            }
            else if (tokens[i].mType == NCategory::PLUS)
            {
               sign = NSign::PLUS;
            }
            else
            {
               throw exception("invalid constant");
            }

            ++i;

            if (tokens[i].mType == NCategory::NAME)
            {
               result.mLabels.push_back(SLabel(sign, findLabel(commands, tokens[i].mData)));
            }
            else if (tokens[i].mType == NCategory::CONSTANT)
            {
               if (sign == NSign::PLUS)
               {
                  result.mValue += tokens[i].mConstant;
               }
               else
               {
                  result.mValue -= tokens[i].mConstant;
               }
            }
         }
      }
      return result;
   }


   SOperand createOperandRegister(const NRegister::EType reg)
   {
      SOperand result;
      result.mRegister = reg;
      switch (reg)
      {
         case NRegister::EAX:
         case NRegister::EBX:
         case NRegister::ECX:
         case NRegister::EDX:
         case NRegister::EBP:
         case NRegister::ESP:
         case NRegister::EDI:
         case NRegister::ESI:
            result.mType = NOperand::REG32;
            break;
         case NRegister::AX:
         case NRegister::BX:
         case NRegister::CX:
         case NRegister::DX:
         case NRegister::BP:
         case NRegister::SP:
         case NRegister::DI:
         case NRegister::SI:
            result.mType = NOperand::REG16;
            break;
         case NRegister::AL:
         case NRegister::BL:
         case NRegister::CL:
         case NRegister::DL:
         case NRegister::AH:
         case NRegister::BH:
         case NRegister::CH:
         case NRegister::DH:
            result.mType = NOperand::REG8;
            break;
      }
      return result;
   }

   SOperand createOperandMemory(const vector<SCommand> & commands, const vector<SToken> & tokens, const NSegment::EType segment, const NDataType::EType dataType)
   {
      SOperand result;
      result.mMemory.mSegment = segment;

      switch (dataType)
      {
         case NDataType::DWORD:
         case NDataType::DD:
            result.mType = NOperand::MEM32;
            break;
         case NDataType::WORD:
         case NDataType::DW:
            result.mType = NOperand::MEM16;
            break;
         case NDataType::BYTE:
         case NDataType::DB:
            result.mType = NOperand::MEM8;
            break;
      }

      unsigned int position = 0;
      if (isMatch(tokens, {NCategory::CONSTANT, NCategory::ASTERIX, NCategory::REGISTER, NCategory::PLUS, NCategory::REGISTER}))
      {
         //result.mMemory.mScale = getScale(tokens[0].mConstant);
         result.mMemory.mScale = tokens[0].mConstant;
         result.mMemory.mRegisters.push_back(NRegister::EType(tokens[2].mIndex));
         result.mMemory.mRegisters.push_back(NRegister::EType(tokens[4].mIndex));
         position += 5;
      }
      else if (isMatch(tokens, {NCategory::REGISTER, NCategory::PLUS, NCategory::REGISTER}))
      {
         result.mMemory.mRegisters.push_back(NRegister::EType(tokens[0].mIndex));
         result.mMemory.mRegisters.push_back(NRegister::EType(tokens[2].mIndex));
         position += 3;
      }
      else if (isMatch(tokens, {NCategory::CONSTANT, NCategory::ASTERIX, NCategory::REGISTER}))
      {
         //result.mMemory.mScale = getScale(tokens[0].mConstant);
         result.mMemory.mScale = tokens[0].mConstant;
         result.mMemory.mRegisters.push_back(NRegister::EType(tokens[2].mIndex));
         position += 3;
      }
      else if (isMatch(tokens, {NCategory::REGISTER}))
      {
         result.mMemory.mRegisters.push_back(NRegister::EType(tokens[0].mIndex));
         position += 1;
      }

      if (position < tokens.size())
      {
         result.mMemory.mConstant = createConstant(commands, getSubTokens(tokens, position));
      }
      return result;
   }

   SOperand createOperandConstant(const vector<SCommand> & commands, const vector<SToken> & tokens)
   {
      SOperand result;
      result.mType = NOperand::CONSTANT;
      result.mConstant = createConstant(commands, tokens);
      return result;
   }

   SOperand createOperand(const vector<SCommand> & commands, const vector<SToken> & tokens)
   {
      SOperand result;
      if (tokens.size() == 1 && tokens[0].mType == NCategory::REGISTER)
      {
         result = createOperandRegister(NRegister::EType(tokens[0].mIndex));
      }
      else if (tokens.size() > 4 && 
         tokens[0].mType == NCategory::DATA_TYPE && 
         tokens[1].mType == NCategory::PTR &&
         tokens[2].mType == NCategory::OP_SQ_BRACKET && 
         tokens[tokens.size() - 1].mType == NCategory::CL_SQ_BRACKET)
      {
         result = createOperandMemory(commands, vector<SToken>(tokens.begin() + 3, tokens.end() - 1), NSegment::NON, NDataType::EType(tokens[0].mIndex));
      }
      // call dword ptr ds:[00923030h]
      else if (tokens.size() > 6 && 
         tokens[0].mType == NCategory::DATA_TYPE && 
         tokens[1].mType == NCategory::PTR &&
         tokens[2].mType == NCategory::SEGMENT && 
         tokens[3].mType == NCategory::COLON &&
         tokens[4].mType == NCategory::OP_SQ_BRACKET && 
         tokens[tokens.size() - 1].mType == NCategory::CL_SQ_BRACKET)
      {
         result = createOperandMemory(commands, vector<SToken>(tokens.begin() + 5, tokens.end() - 1), NSegment::EType(tokens[2].mIndex), NDataType::EType(tokens[0].mIndex));
      }
      else if (!tokens.empty() && 
         (tokens[0].mType == NCategory::MINUS || 
         tokens[0].mType == NCategory::PLUS ||
         tokens[0].mType == NCategory::CONSTANT || 
         tokens[0].mType == NCategory::NAME))
      {
         result = createOperandConstant(commands, tokens);
      }
      else
      {
         throw exception("invalid operand");
      }
      return result;
   }

   void fillInstruction(SCommand & command, const vector<SCommand> & commands, const vector<SToken> & tokens/*only operands!*/)
   {
      assert(command.mType == NCommand::INSTRUCTION);

      // split operands

      // get first
      if (!tokens.empty())
      {
         int beginPosition = 0;
         int commaPosition = -1;
         do 
         {
            commaPosition = findToken(tokens, beginPosition, NCategory::COMMA);

            command.mInstruction.mOperands.push_back(createOperand(commands, getSubTokens(tokens, beginPosition/*beginPosition*/, 
               (commaPosition == -1) ? commaPosition : commaPosition - beginPosition/*length*/)));
            
            beginPosition = (commaPosition == -1) ? commaPosition : commaPosition + 1;

         } while (beginPosition != -1);
      }
   }

   int getDataSize(const NDataType::EType dataType)
   {
      switch (dataType)
      {
      case NDataType::DWORD:
      case NDataType::DD:
         return 4;
      case NDataType::WORD:
      case NDataType::DW:
         return 2;
      case NDataType::BYTE:
      case NDataType::DB:
         return 2;
      }
      throw exception("invalid data type");
   }

   vector<SConstant> createDataString(const SToken & token)
   {
      vector<SConstant> result;
      for (unsigned int i = 0; i < token.mData.size(); ++i)
      {
          SConstant constant;
          constant.mValue = token.mData[i];
          result.push_back(constant);
      }
      return result;
   }

   vector<SConstant> createDataConstants(const vector<SCommand> & commands, const vector<SToken> & tokens)
   {
      vector<SConstant> result;
      if (!tokens.empty())
      {
         if (tokens[0].mType == NCategory::STRING)
         {
            result = createDataString(tokens[0]);
         }
         else
         {
            result.push_back(createConstant(commands, tokens));
         }
      }
      return result;
   }

   // new method const string & label, const int numberLine, 
   void fillData(SCommand & command, const vector<SCommand> & commands, const vector<SToken> & tokens/*only value*/)
   {
      assert(command.mType == NCommand::DATA);

      if (tokens.size() == 5 && 
         tokens[0].mType == NCategory::CONSTANT &&
         tokens[1].mType == NCategory::DUP && 
         tokens[2].mType == NCategory::OP_BRACKET && 
         tokens[3].mType == NCategory::CONSTANT && 
         tokens[4].mType == NCategory::CL_BRACKET)
      {
         command.mData.mCount = tokens[0].mConstant;
         SConstant constant;
         constant.mValue = tokens[3].mConstant;
         command.mData.mConstants.push_back(constant);
      }
      else if (!tokens.empty())
      {
         command.mData.mCount = 1;
         for (unsigned int i = 0; i < tokens.size();)
         {
            const vector<SToken> & subTokens = getSubTokens(tokens, i, NCategory::COMMA);

            const vector<SConstant> & constants = createDataConstants(commands, subTokens);
            // append
            command.mData.mConstants.insert(command.mData.mConstants.end(), constants.begin(), constants.end());

            i += subTokens.size();

            if (i < tokens.size() && tokens[i].mType == NCategory::COMMA)
            {
               ++i;
            }
         }
      }
   }
   
   SCommand createEndComamnd(const string & label)
   {
      SCommand result;
      result.mType = NCommand::END;
      result.mNameLabel = label;
      result.mNumberLine = -1;
      return result;
   }

   SCommand createInstruction(const string & label, const int numberLine, const NPrefix::EType prefix, const NInstruction::EType instruction)
   {
      SCommand result;
      result.mType = NCommand::INSTRUCTION;
      result.mNameLabel = label;
      result.mNumberLine = numberLine;
      result.mInstruction.mType = instruction;
      result.mInstruction.mPrefix = prefix;
      return result;
   }

   SCommand createData(const string & label, const int numberLine, const string & dataName, const NDataType::EType dataType)
   {
      SCommand result;
      result.mType = NCommand::DATA;
      result.mNameLabel = label;
      result.mNumberLine = numberLine;
      result.mData.mName = dataName;
      result.mData.mSizeData = getDataTypeSize(dataType);
      return result;
   }

   SCommand createImport(const string & label, const int numberLine, const string & functionName)
   {
      SCommand result;
      result.mType = NCommand::IMPORT;
      //result.mNameLabel = dllName + "." + functionName;
      result.mNameLabel = functionName;
      result.mNumberLine = numberLine;
      
      size_t dot = functionName.find('.');
      if (dot != string::npos)
      {
         result.mImport.mDllName = functionName.substr(0, dot) + ".dll";
         result.mImport.mFunctionName = functionName.substr(dot + 1);
      }
      else
      {
         throw exception("Wrong import name");
      }
      return result;
   }
   
   SCommand createExtern(const string & label, const int numberLine, const NDataType::EType dataType, const string & name)
   {
      SCommand result;
      result.mType = NCommand::EXTERN;
      result.mNameLabel = name;
      result.mNumberLine = numberLine;
      result.mData.mSizeData = getDataTypeSize(dataType);
      return result;
   }

   SCommand createDirective(const string & label, const int numberLine, const string & name)
   {
      SCommand result;
      result.mType = NCommand::DIRECTIVE;
      result.mNameLabel = label;
      result.mNumberLine = numberLine;
      result.mDirective.mName = name;
      return result;
   }

   int getAttributes(const string & attributes)
   {
      int result = 0;
      for (unsigned int i = 0; i < attributes.size(); ++i)
      {
         switch (attributes[i])
         {
         case 'r':
         case 'R':
            result |= NSectionAttributes::READ;
            break;
         case 'w':
         case 'W':
            result |= NSectionAttributes::WRITE;
            break;
         case 'e':
         case 'E':
            result |= NSectionAttributes::EXECUTE;
            break;
         case 'c':
         case 'C':
            result |= NSectionAttributes::CODE;
            break;
         case 'i':
         case 'I':
            result |= NSectionAttributes::INITIALIZED;
            break;
         }
      }
      return result;
   }
   
   SCommand createSection(const string & label, int numberLine, const string & nameSection, const string & attributes)
   {
      SCommand result;
      result.mType = NCommand::SECTION;
      result.mNameLabel = label;
      result.mNumberLine = numberLine;
      result.mSection.mName = nameSection;
      result.mSection.mAttributes = getAttributes(attributes);
      return result;
   }

   string toString(const NPeProtector::SToken & token)
   {
      ostringstream result;
      result << NPeProtector::NCategory::gStrings[token.mType];
      switch (token.mType)
      {
      case NCategory::COMMA:         // ','
      //case NCategory::DOT:           // '.'
      case NCategory::COLON:         // ':'
      case NCategory::MINUS:         // '-'
      case NCategory::PLUS:          // '+'
      case NCategory::OP_BRACKET:    // '('
      case NCategory::CL_BRACKET:    // ')'
      case NCategory::OP_SQ_BRACKET: // '['
      case NCategory::CL_SQ_BRACKET: // ']'
      case NCategory::ASTERIX:       // '*'
         // keywords
      case NCategory::IMPORT:
      case NCategory::EXTERN:
      case NCategory::DUP:
      case NCategory::PTR:
      case NCategory::SECTION:
         // ignore
         break;
         // groups
      case NCategory::DATA_TYPE: // map to type NDataType::EType
         result << " " << NPeProtector::NDataType::gStrings[token.mIndex];
         break;
      case NCategory::PREFIX: // map to type NPrefix::EType
         result << " " << NPeProtector::NPrefix::gStrings[token.mIndex];
         break;
      case NCategory::INSTRUCTION: // map to type NInstruction::EType
         result << " " << NPeProtector::NInstruction::gStrings[token.mIndex];
         break;
      case NCategory::REGISTER:  //map to type NRegister::EType
         result << " " << NPeProtector::NRegister::gStrings[token.mIndex];
         break;
      case NCategory::SEGMENT: // map to type NSegment::EType
         result << " " << NPeProtector::NSegment::gStrings[token.mIndex];
         break;
         // undefined string
      case NCategory::NAME:
         result << " " << token.mData;
         break;
      case NCategory::STRING:
         result << " \"" << token.mData << "\"";
         break;
      case NCategory::CONSTANT:
         result << " " << token.mConstant;
         break;
      }
      return result.str();
   }

   string toString(const vector<NPeProtector::SToken> & tokens)
   {
      ostringstream result;

      for (unsigned int i = 0; i < tokens.size(); ++i)
      {
         result << toString(tokens[i]) << ((i == (tokens.size() - 1)) ? "" : ", ");
      }

      return result.str();
   }

   void loggingTokens(const vector<vector<NPeProtector::SToken> > & tokens)
   {
      for (unsigned int i = 0; i < tokens.size(); ++i)
      {
         LOG_DEBUG("line %d : %s", i, toString(tokens[i]).c_str());
      }
   }
}

   vector<SCommand> compile(basic_istream<char, char_traits<char> > & input)
   {
      vector<SCommand> result;

      const vector<vector<NPeProtector::SToken> > & tokens = parse(input);
      
      loggingTokens(tokens);

      // first scan: processing import, extern, labels and stub for instruction and data
      string currentLabel;
      for (unsigned int indexLine = 0; indexLine < tokens.size(); ++indexLine)
      {
         if (tokens[indexLine].empty())
         {
            //skip, push empty line
         }
         else if (isMatch(tokens[indexLine], {NCategory::DIRECTIVE, NCategory::NAME}))
         {
            result.push_back(createDirective(currentLabel, indexLine, tokens[indexLine][1].mData));
            currentLabel.clear();
         }
         //else if (isMatch(tokens[indexLine], {NCategory::IMPORT, NCategory::NAME, NCategory::DOT, NCategory::NAME}))
         else if (isMatch(tokens[indexLine], {NCategory::IMPORT, NCategory::NAME}))
         {
            //result.push_back(createImport("", indexLine, tokens[indexLine][1].mData, tokens[indexLine][3].mData));
            result.push_back(createImport("", indexLine, tokens[indexLine][1].mData));
         }
         else if (isMatch(tokens[indexLine], {NCategory::EXTERN, NCategory::DATA_TYPE, NCategory::NAME}))
         {
            result.push_back(createExtern("", indexLine, NDataType::EType(tokens[indexLine][1].mIndex), tokens[indexLine][2].mData));
         }
         else if (isMatch(tokens[indexLine], {NCategory::SECTION, NCategory::STRING, NCategory::NAME}))
         {
            result.push_back(createSection("", indexLine, tokens[indexLine][1].mData, tokens[indexLine][2].mData));
         }
         else if (isMatch(tokens[indexLine], {NCategory::NAME, NCategory::COLON, NCategory::PREFIX, NCategory::INSTRUCTION}))
         {
            if (!currentLabel.empty())
            {
               // warning!
               currentLabel.clear();
            }
            result.push_back(createInstruction(tokens[indexLine][0].mData, indexLine, NPrefix::EType(tokens[indexLine][2].mIndex), NInstruction::EType(tokens[indexLine][3].mIndex)));
         }
         else if (isMatch(tokens[indexLine], {NCategory::NAME, NCategory::COLON, NCategory::INSTRUCTION}))
         {
            if (!currentLabel.empty())
            {
               // warning!
               currentLabel.clear();
            }
            result.push_back(createInstruction(tokens[indexLine][0].mData, indexLine, NPrefix::NON, NInstruction::EType(tokens[indexLine][2].mIndex)));
         }
         else if (isMatch(tokens[indexLine], {NCategory::PREFIX, NCategory::INSTRUCTION}))
         {
            result.push_back(createInstruction(currentLabel, indexLine, NPrefix::EType(tokens[indexLine][0].mIndex), NInstruction::EType(tokens[indexLine][1].mIndex)));
            currentLabel.clear();
         }
         else if (isMatch(tokens[indexLine], {NCategory::INSTRUCTION}))
         {
            result.push_back(createInstruction(currentLabel, indexLine, NPrefix::NON, NInstruction::EType(tokens[indexLine][0].mIndex)));
            currentLabel.clear();
         }
         else if (isMatch(tokens[indexLine], {NCategory::NAME, NCategory::DATA_TYPE}))
         {
            result.push_back(createData(currentLabel, indexLine, tokens[indexLine][0].mData, NDataType::EType(tokens[indexLine][1].mIndex)));
            currentLabel.clear();
         }
         else if (isMatch(tokens[indexLine], {NCategory::NAME, NCategory::COLON}))
         {
            // save label !
            currentLabel = tokens[indexLine][0].mData;
         }
         else
         {
            //error!!!
         }
      }
      // add ending command
      result.push_back(createEndComamnd(currentLabel));
      currentLabel.clear();

      LOG_DEBUG("First scan:");
      loggingCommands(result);

      // second scan: instruction and data
      for (unsigned int indexLine = 0, indexCommand = 0; indexLine < tokens.size(); ++indexLine)
      {
         // erase label in line
         const vector<SToken> & lineTokens = isMatch(tokens[indexLine], { NCategory::NAME, NCategory::COLON }) ?
            getSubTokens(tokens[indexLine], 2) : tokens[indexLine];

         if (!lineTokens.empty())
         {
            if (isMatch(lineTokens, { NCategory::PREFIX, NCategory::INSTRUCTION }))
            {
               fillInstruction(result[indexCommand], result, getSubTokens(lineTokens, 2));
            }
            else if (isMatch(lineTokens, { NCategory::INSTRUCTION }))
            {
               fillInstruction(result[indexCommand], result, getSubTokens(lineTokens, 1));
            }
            else if (isMatch(lineTokens, { NCategory::NAME, NCategory::DATA_TYPE }))
            {
               fillData(result[indexCommand], result, getSubTokens(lineTokens, 2));
            }
            ++indexCommand;
         }
      }

      LOG_DEBUG("Second scan:");
      loggingCommands(result);

      return result;
   }
}

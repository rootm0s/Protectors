#include "SCommand.h"
#include <sstream> 
#include "../LogLibrary/CLog.h"

using std::string;
using std::vector;
using std::ostringstream;
using std::basic_ostream;
using std::basic_istream;
using std::char_traits;

namespace NPeProtector
{
namespace
{
   string toString(const vector<NRegister::EType> & registers)
   {
      ostringstream result;
      for (unsigned int i = 0; i < registers.size(); ++i)
      {
         result << NRegister::gStrings[registers[i]] << " ";
      }
      return result.str();
   }

   string toString(const SConstant & constant)
   {
      ostringstream result;
      result << "labels {";
      for (unsigned int i = 0; i < constant.mLabels.size(); ++i)
      {
         result << NSign::gStrings[constant.mLabels[i].mSign] << " ";
         result << constant.mLabels[i].mIndex << " ";
      }
      result << "} value " << constant.mValue;

      return result.str();
   }

   string toString(const SMemory & memory)
   {
      ostringstream result;
      result << "reg {" << toString(memory.mRegisters) << "} ";
      result << "scale " << memory.mScale << " ";
      result << "seg " << NSegment::gStrings[memory.mSegment] << " ";
      result << "constant {" << toString(memory.mConstant) << "}";
      return result.str();
   }

   string toString(const vector<SOperand> & operands)
   {
      ostringstream result;
      for (unsigned int i = 0; i < operands.size(); ++i)
      {
         result << "type " << NOperand::gStrings[operands[i].mType];
         switch (operands[i].mType)
         {
            case NOperand::REG8:
            case NOperand::REG16:
            case NOperand::REG32:
               result << " reg " << NRegister::gStrings[operands[i].mRegister];
               break;
            case NOperand::MEM8:
            case NOperand::MEM16:
            case NOperand::MEM32:
               result << " mem {" << toString(operands[i].mMemory) << "}";
               break;
            case NOperand::CONSTANT:
               result << " constant {" << toString(operands[i].mConstant) << "}";
               break;
         }
         result << (i == operands.size() - 1 ? "" : ", ");
      }
      return result.str();
   }

   string toString(const SData & data)
   {
      ostringstream result;
      result << "name " << data.mName << " ";
      result << "sizeData " << data.mSizeData << " ";
      result << "constants {";
      for (unsigned int i = 0; i < data.mConstants.size(); ++i)
      {
         result << toString(data.mConstants[i]) << " ";// ??
      }
      result << "} ";
      result << "count " << data.mCount;

      return result.str();
   }

   string toString(const SImport & import)
   {
      ostringstream result;
      result << "dllName " << import.mDllName << " ";
      result << "function " << import.mFunctionName;
      return result.str();
   }

   string toString(const SSection & section)
   {
      ostringstream result;
      result << "name " << section.mName << " ";
      result << "attributes " << section.mAttributes; //TODO
      return result.str();
   }

   string toString(const SInstruction & instruction)
   {
      ostringstream result;
      result << "prefix " << NPrefix::gStrings[instruction.mPrefix] << " ";
      result << "type " << NInstruction::gStrings[instruction.mType] << " ";
      result << "operands {" << toString(instruction.mOperands) << "} ";
      result << "trash " << instruction.mTrash;
      return result.str();
   }
}
   SLabel::SLabel() 
      : mSign()
      , mIndex()
   {
   }

   SLabel::SLabel(const NSign::EType sign, const int index)
      : mSign(sign)
      , mIndex(index)
   {
   }

   SConstant::SConstant() 
      : mLabels()
      , mValue()
   {
   }

   SConstant::SConstant(const std::vector<SLabel> & labels, const DWORD value) : mLabels(labels)
      , mValue(value)
   {

   }

   SMemory::SMemory() 
      : mRegisters()
      , mScale()
      , mSegment(NSegment::NON)
      , mConstant()
   {
   }

   SMemory::SMemory(const std::vector<NRegister::EType> & registers, const int scale, const NSegment::EType segment, const SConstant & constant) : mRegisters(registers)
      , mScale(scale)
      , mSegment(segment)
      , mConstant(constant)
   {

   }

   SOperand::SOperand() 
      : mType(NOperand::NON)
      , mMemory()
      , mRegister()
      , mConstant()
   {
   }

   SOperand::SOperand(const NOperand::EType type, const SMemory & memory, const NRegister::EType _register, const SConstant & constant) : mType(type)
      , mMemory(memory)
      , mRegister(_register)
      , mConstant(constant)
   {

   }

   SInstruction::SInstruction() 
      : mPrefix(NPrefix::NON)
      , mType()
      , mOperands()
      , mTrash()
   {
   }

   SInstruction::SInstruction(const NPrefix::EType prefix, const NInstruction::EType type, const std::vector<SOperand> & operands) : mPrefix(prefix)
      , mType(type)
      , mOperands(operands)
      , mTrash()
   {

   }

   SData::SData() 
      : mName()
      , mSizeData()
      , mConstants()
      , mCount()
   {
   }

   SData::SData(const std::string & name, const int sizeData, const std::vector<SConstant> & constants, const int count) : mName(name)
      , mSizeData(sizeData)
      , mConstants(constants)
      , mCount(count)
   {

   }

   SImport::SImport() 
      : mDllName()
      , mFunctionName()
   {
   }

   SImport::SImport(const std::string & dllName, const std::string & functionName) : mDllName(dllName)
      , mFunctionName(functionName)
   {

   }

   SSection::SSection() 
      : mName()
      , mAttributes()
   {
   }

   SSection::SSection(const std::string & name, const int attributes) : mName(name)
      , mAttributes(attributes)
   {

   }

   SDirective::SDirective() 
      : mName()
      , mDirectorySize()
   {
   }

   SDirective::SDirective(const std::string & name, const int directorySize) : mName(name)
      , mDirectorySize(directorySize)
   {

   }

   SCommand::SCommand() 
      : mType(NCommand::END)
      , mRVA()
      , mRAW()
      , mNameLabel()
      , mNumberLine()
      , mInstruction()
      , mData()
      , mDirective()
      , mImport()
      , mSection()
   {
   }

   SCommand::SCommand(const NCommand::EType type, const DWORD RVA, const DWORD RAW, const std::string & nameLabel, const int numberLine, const SInstruction & instruction, const SData & data, const SDirective & directive, const SImport & import, const SSection & section) : mType(type)
      , mRVA(RVA)
      , mRAW(RAW)
      , mNameLabel(nameLabel)
      , mNumberLine(numberLine)
      , mInstruction(instruction)
      , mData(data)
      , mDirective(directive)
      , mImport(import)
      , mSection(section)
   {

   }

   void deserialize(basic_istream<char, char_traits<char> > & input, char & value)
   {
      input.read(&value, sizeof(value));
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const char value)
   {
      output.write(&value, sizeof(value));
   }

   void deserialize(basic_istream<char, char_traits<char> > & input, NRegister::EType & value)
   {
      input.read((char*)(&value), sizeof(value));
   }

   void deserialize(basic_istream<char, char_traits<char> > & input, int & value)
   {
      input.read((char*)(&value), sizeof(value));
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const int value)
   {
      output.write((char*)(&value), sizeof(value));
   }

   void deserialize(basic_istream<char, char_traits<char> > & input, string & values)
   {
      int size;
      deserialize(input, size);
      for (int i = 0; i < size; ++i)
      {
         char value;
         deserialize(input, value);
         values += value;
      }
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const string & values)
   {
      serialize(output, static_cast<int>(values.size()));

      for (unsigned int i = 0; i < values.size(); ++i)
      {
         serialize(output, values[i]);
      }
   }

   void deserialize(basic_istream<char, char_traits<char> > & input, SLabel & label)
   {
      deserialize(input, reinterpret_cast<int&>(label.mSign));
      deserialize(input, label.mIndex);
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const SLabel & label)
   {
      serialize(output, label.mSign);
      serialize(output, label.mIndex);
   }
   
   void deserialize(basic_istream<char, char_traits<char> > & input, SConstant & constant)
   {
      deserialize(input, constant.mLabels);
      deserialize(input, reinterpret_cast<int&>(constant.mValue));
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const SConstant & constant)
   {
      serialize(output, constant.mLabels);
      serialize(output, static_cast<int>(constant.mValue));
   }

   void deserialize(basic_istream<char, char_traits<char> > & input, SMemory & operandMemory)
   {
      deserialize(input, operandMemory.mRegisters);
      deserialize(input, operandMemory.mScale);
      deserialize(input, reinterpret_cast<int&>(operandMemory.mSegment));
      deserialize(input, operandMemory.mConstant);
   }
   void serialize(basic_ostream<char, char_traits<char> > & output, const SMemory & operandMemory)
   {
      serialize(output, operandMemory.mRegisters);
      serialize(output, operandMemory.mScale);
      serialize(output, operandMemory.mSegment);
      serialize(output, operandMemory.mConstant);
   }
   void deserialize(basic_istream<char, char_traits<char> > & input, SOperand & operand)
   {
      deserialize(input, reinterpret_cast<int&>(operand.mType));
      deserialize(input, operand.mMemory);
      deserialize(input, reinterpret_cast<int&>(operand.mRegister));
      deserialize(input, operand.mConstant);
   }
   void serialize(basic_ostream<char, char_traits<char> > & output, const SOperand & operand)
   {
      serialize(output, operand.mType);
      serialize(output, operand.mMemory);
      serialize(output, operand.mRegister);
      serialize(output, operand.mConstant);
   }
   
   void deserialize(basic_istream<char, char_traits<char> > & input, SInstruction & instruction)
   {
      deserialize(input, reinterpret_cast<int&>(instruction.mPrefix));
      deserialize(input, reinterpret_cast<int&>(instruction.mType));
      deserialize(input, instruction.mOperands);
      deserialize(input, instruction.mTrash);
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const SInstruction & instruction)
   {
      serialize(output, instruction.mPrefix);
      serialize(output, instruction.mType);
      serialize(output, instruction.mOperands);
      serialize(output, instruction.mTrash);
   }

   void deserialize(basic_istream<char, char_traits<char> > & input, SSection & section)
   {
      deserialize(input, section.mName);
      deserialize(input, section.mAttributes);
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const SSection & section)
   {
      serialize(output, section.mName);
      serialize(output, section.mAttributes);
   }
   
   void deserialize(basic_istream<char, char_traits<char> > & input, SImport & import)
   {
      deserialize(input, import.mDllName);
      deserialize(input, import.mFunctionName);
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const SImport & import)
   {
      serialize(output, import.mDllName);
      serialize(output, import.mFunctionName);
   }
   
   void deserialize(basic_istream<char, char_traits<char> > & input, SDirective & directive)
   {
      deserialize(input, directive.mName);
      deserialize(input, directive.mDirectorySize);
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const SDirective & directive)
   {
      serialize(output, directive.mName);
      serialize(output, directive.mDirectorySize);
   }

   void deserialize(basic_istream<char, char_traits<char> > & input, SData & data)
   {
      deserialize(input, data.mName);
      deserialize(input, data.mSizeData);
      deserialize(input, data.mConstants);
      deserialize(input, data.mCount);
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const SData & data)
   {
      serialize(output, data.mName);
      serialize(output, data.mSizeData);
      serialize(output, data.mConstants);
      serialize(output, data.mCount);
   }
   
   void deserialize(basic_istream<char, char_traits<char> > & input, SCommand & command)
   {
      deserialize(input, reinterpret_cast<int&>(command.mType));
      deserialize(input, reinterpret_cast<int&>(command.mRVA));
      deserialize(input, reinterpret_cast<int&>(command.mRAW));
      deserialize(input, command.mNameLabel);
      deserialize(input, command.mNumberLine);
      deserialize(input, command.mInstruction);
      deserialize(input, command.mData);
      deserialize(input, command.mDirective);
      deserialize(input, command.mImport);
      deserialize(input, command.mSection);
   }

   void serialize(basic_ostream<char, char_traits<char> > & output, const SCommand & command)
   {
      serialize(output, command.mType);
      serialize(output, static_cast<int>(command.mRVA));
      serialize(output, static_cast<int>(command.mRAW));
      serialize(output, command.mNameLabel);
      serialize(output, command.mNumberLine);
      serialize(output, command.mInstruction);
      serialize(output, command.mData);
      serialize(output, command.mDirective);
      serialize(output, command.mImport);
      serialize(output, command.mSection);
   }

   void loggingCommands(const vector<SCommand> & commands)
   {
      for (unsigned int i = 0; i < commands.size(); ++i)
      {
         switch (commands[i].mType)
         {
         case NCommand::INSTRUCTION:
            LOG_DEBUG("command[%d] type %s rva %p raw %p nameLabel %s numberLine %d instruction {%s}",
               i,
               NCommand::gStrings[commands[i].mType],
               commands[i].mRVA,
               commands[i].mRAW,
               commands[i].mNameLabel.c_str(),
               commands[i].mNumberLine,
               toString(commands[i].mInstruction).c_str());
            break;
         case NCommand::DATA:
            LOG_DEBUG("command[%d] type %s rva %p raw %p nameLabel %s numberLine %d data {%s}",
               i,
               NCommand::gStrings[commands[i].mType],
               commands[i].mRVA,
               commands[i].mRAW,
               commands[i].mNameLabel.c_str(),
               commands[i].mNumberLine,
               toString(commands[i].mData).c_str());
            break;
         case NCommand::DIRECTIVE:
            LOG_DEBUG("command[%d] type %s rva %p raw %p nameLabel %s numberLine %d directive {%s %d}",
               i,
               NCommand::gStrings[commands[i].mType],
               commands[i].mRVA,
               commands[i].mRAW,
               commands[i].mNameLabel.c_str(),
               commands[i].mNumberLine,
               commands[i].mDirective.mName.c_str(),
               commands[i].mDirective.mDirectorySize);
            break;
         case NCommand::IMPORT:
            LOG_DEBUG("command[%d] type %s rva %p raw %p nameLabel %s numberLine %d import {%s}",
               i,
               NCommand::gStrings[commands[i].mType],
               commands[i].mRVA,
               commands[i].mRAW,
               commands[i].mNameLabel.c_str(),
               commands[i].mNumberLine,
               toString(commands[i].mImport).c_str());
            break;
         case NCommand::EXTERN:
            LOG_DEBUG("command[%d] type %s rva %p raw %p nameLabel %s numberLine %d data {%s}",
               i,
               NCommand::gStrings[commands[i].mType],
               commands[i].mRVA,
               commands[i].mRAW,
               commands[i].mNameLabel.c_str(),
               commands[i].mNumberLine,
               toString(commands[i].mData).c_str());
            break;
         case NCommand::SECTION:
            LOG_DEBUG("command[%d] type %s rva %p raw %p nameLabel %s numberLine %d section {%s}",
               i,
               NCommand::gStrings[commands[i].mType],
               commands[i].mRVA,
               commands[i].mRAW,
               commands[i].mNameLabel.c_str(),
               commands[i].mNumberLine,
               toString(commands[i].mSection).c_str());
            break;
         case NCommand::END:
            LOG_DEBUG("command[%d] type %s rva %p raw %p nameLabel %s numberLine %d",
               i,
               NCommand::gStrings[commands[i].mType],
               commands[i].mRVA,
               commands[i].mRAW,
               commands[i].mNameLabel.c_str(),
               commands[i].mNumberLine);
            break;
         }
      }
   }
}
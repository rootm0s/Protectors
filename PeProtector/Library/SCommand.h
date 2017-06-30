#ifndef SCOMMAND_H
#define SCOMMAND_H

#include <vector>
#include <windows.h>
#include "..\Library\Types.h"
#include <istream>

namespace NPeProtector
{
   /**
    * @brief Contains reference to command in list<SCommand>
    */
   struct SLabel
   {
      /**
       * @brief Constructor
       */
      SLabel();
      /**
       * @brief Constructor
       * @param[in] sign sign of constant
       * @param[in] index index in list<SCommand>
       */
      SLabel(const NSign::EType sign, const int index);
      /**
       * @brief Label sign
       */
      NSign::EType mSign;
      /**
       * @brief Index in list<SCommand>
       */
      int mIndex;
   };

   /**
    * @brief Contains list of labels and constant value. 
    * It's used in memory operand, ex: DWORD PTR [label1 - label2 + 666]
    */
   struct SConstant
   {
      /**
       * @brief Constructor
       */
      SConstant();
      /**
       * @brief Constructor
       */
      SConstant(const std::vector<SLabel> & labels, const DWORD value);
      /**
       * @brief List of labels
       */
      std::vector<SLabel> mLabels;
      /**
       * @brief Value of constant
       */
      DWORD mValue;
   };

   /**
    * @brief Describes memory operand in assembler instruction
    */
   struct SMemory
   {
      /**
       * @brief Constructor
       */
      SMemory();
      /**
       * @brief Constructor
       */
      SMemory(const std::vector<NRegister::EType> & registers, const int scale, const NSegment::EType segment, const SConstant & constant);

      std::vector<NRegister::EType> mRegisters;

      int mScale;  // 1->>2; 2->>4; 3-->>>8; scale! //TODO int!!!

      NSegment::EType mSegment;

      SConstant mConstant;
   };

   /**
    * @brief Describes operand in assembler instruction
    */
   struct SOperand
   {
      /**
       * @brief Constructor
       */
      SOperand();
      /**
       * @brief Constructor
       */
      SOperand(const NOperand::EType type, const SMemory & memory, const NRegister::EType _register, const SConstant & constant);
      /**
       * @brief Type of operand (register, memory or constant).
       */
      NOperand::EType mType;
      /**
       * @brief It's used when type = memory
       */
      SMemory           mMemory;
      /**
       * @brief It's used when type = register
       */
      NRegister::EType  mRegister;
      /**
       * @brief It's used when type = constant
       */
      SConstant         mConstant;
   };

   /**
    * @brief Describes assembler instruction
    */
   struct SInstruction
   {
      /**
       * @brief Constructor
       */
      SInstruction();
      /**
       * @brief Constructor
       */
      SInstruction(const NPrefix::EType prefix, const NInstruction::EType type, const std::vector<SOperand> & operands);
      /**
       * @brief Prefix of instruction
       */
      NPrefix::EType mPrefix;
      /**
       * @brief Type of instruction
       */
      NInstruction::EType mType;
      /**
       * @brief Operands of instruction
       */
      std::vector<SOperand> mOperands;
      /**
       * @brief Currently it's not used
       */
      int mTrash;
   };

   /**
    * @brief Describes data in source code
    */
   struct SData
   {
      /**
       * @brief Constructor
       */
      SData();
      /**
       * @brief Constructor
       */
      SData(const std::string & name, const int sizeData, const std::vector<SConstant> & constants, const int count);
      /**
       * @brief Name of data, ex: API_ExAllocatePool dd 12345678h
       */
      std::string mName;
      /**
       * @brief Size of single item of data. Ex: API_ExAllocatePool dd 12345678h => mSizeData = 4 bytes
       * The total size = mSizeData * mCount
       */
      int mSizeData;
      /**
       * @brief if data is dup vector.size == 1 and count > 1. If data.size == count then it's array.
       */
      std::vector<SConstant> mConstants;
      /**
       * @brief Count of data
       */
      int mCount;
   };
  
   /**
    * @brief Describes import function
    */
   struct SImport
   {
      /**
       * @brief Constructor
       */
      SImport();
      /**
       * @brief Constructor
       */
      SImport(const std::string & dllName, const std::string & functionName);
      /**
       * @brief Dll name
       */
      std::string mDllName;
      /**
       * @brief Function name
       */
      std::string mFunctionName;
   };
  
   /**
    * @brief Describes pe section
    */
   struct SSection
   {
      /**
       * @brief Constructor
       */
      SSection();
      /**
       * @brief Constructor
       */
      SSection(const std::string & name, const int attributes);
      /**
       * @brief Name of section
       */
      std::string mName;
      /**
       * @brief Attributes of section, see NSectionAttributes.
       */
      int mAttributes;
   };
   
   /**
    * @brief Describes directive. It can be IMPORT_DIRECTORY, RECOURCE_DIRECTORY or COMPRESSED_FILE.
    */
   struct SDirective
   {
      /**
       * @brief Constructor
       */
      SDirective();
      /**
       * @brief Constructor
       */
      SDirective(const std::string & name, const int directorySize);
      /**
       * @brief Name of directive. 
       * It can be IMPORT_DIRECTORY, RECOURCE_DIRECTORY or COMPRESSED_FILE.
       */
      std::string mName;
      /**
       * @brief Size of directive
       */
      int mDirectorySize;
   };
   
   /**
    * @brief It's the key struct. 
    * It can contain: INSTRUCTION, DATA, DIRECTIVE, IMPORT, EXTERN, SECTION or END.
    */
   struct SCommand
   {
      /**
       * @brief Constructor
       */
      SCommand();
      /**
       * @brief Constructor
       */
      SCommand(const NCommand::EType type, 
         const DWORD RVA, 
         const DWORD RAW, 
         const std::string & nameLabel, 
         const int numberLine, 
         const SInstruction & instruction, 
         const SData & data, 
         const SDirective & directive, 
         const SImport & import, 
         const SSection & section);
      /**
       * @brief Type of command
       */
      NCommand::EType mType;
      /**
       * @brief RVA(offset in memory)
       */
      DWORD mRVA;
      /**
       * @brief RAW (offset in file)
       */
      DWORD mRAW;
      /**
       * @brief Label of command
       */
      std::string mNameLabel;
      /**
       * @brief Line number in source code. It's used for only for debug purpose.
       */
      int mNumberLine;
      /**
       * @brief Instruction, if type == INSTRUCTION
       */
      SInstruction mInstruction;
      /**
      * @brief Data, if type == DATA
      */
      SData mData;
      /**
       * @brief Directive, if type == DIRECTIVE
       */
      SDirective mDirective;
      /**
       * @brief Import, if type == IMPORT
       */
      SImport mImport;
      /**
       * @brief Section, if type == SECTION
       */
      SSection mSection;
   };
  
   /**
    * @brief Dump all command in trace file
    */
   void loggingCommands(const std::vector<NPeProtector::SCommand> & commands);

   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, char & value);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, NRegister::EType & value);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, int & value);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, std::string & values);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, SLabel & label);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, SConstant & constant);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, SMemory & operandMemory);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, SOperand & operand);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, SInstruction & instruction);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, SSection & section);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, SImport & import);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, SData & data);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, SData & data);
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, SCommand & command);

   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const char value);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const int value);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const std::string & values);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const SLabel & label);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const SConstant & constant);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const SMemory & operandMemory);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const SOperand & operand);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const SInstruction & instruction);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const SSection & section);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const SImport & import);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const SData & data);
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const SCommand & command);

   template <typename T>
   void deserialize(std::basic_istream<char, std::char_traits<char> > & input, std::vector<T> & values)
   {
      int size;
      deserialize(input, size);

      for (int i = 0; i < size; ++i)
      {
         T value;
         deserialize(input, value);
         values.push_back(value);
      }
   }

   template <typename T>
   void serialize(std::basic_ostream<char, std::char_traits<char> > & output, const std::vector<T> & values)
   {
      serialize(output, static_cast<int>(values.size()));

      for (unsigned int i = 0; i < values.size(); ++i)
      {
         serialize(output, values[i]);
      }
   }
}

#endif
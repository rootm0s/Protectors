#ifndef CLEXICAL_ANALIZER_H
#define CLEXICAL_ANALIZER_H

#include <vector>
#include <windows.h>

namespace NPeProtector
{
   namespace NCategory
   {
      /**
       * @brief Describes type of token or type of group tokens
       */
      enum EType
      {
         COMMA,         // ','
         COLON,         // ':'
         MINUS,         // '-'
         PLUS,          // '+'
         OP_BRACKET,    // '('
         CL_BRACKET,    // ')'
         OP_SQ_BRACKET, // '['
         CL_SQ_BRACKET, // ']'
         ASTERIX,       // '*'
         // keywords
         IMPORT,
         EXTERN,
         DUP,
         PTR,
         SECTION,
         DIRECTIVE,
         // groups
         DATA_TYPE,   // map to type NDataType::EType
         PREFIX,      // map to type NPrefix::EType
         INSTRUCTION, // map to type NInstruction::EType
         REGISTER,    // map to type NRegister::EType
         SEGMENT,     // map to type NSegment::EType
         // undefined string
         NAME,
         STRING,      // quote data quote, ex: "123"
         CONSTANT
      };
      /**
       * @brief Number of strings
       */
      extern const int gSize;
      /**
       * @brief String representation of EType
       */
      extern const char * const gStrings[];
   }

   /**
    * @brief Describes single token
    */
   struct SToken
   {
      /**
       * @brief Constructor
       */
      SToken();
      /**
       * @brief Constructor
       * @param[in] type type of token
       * @param[in] index it is used only for group tokens
       * @param[in] name it is used only for types: NAME or STRING
       * @param[in] constant it is used only for type CONSTANT
       */
      SToken(NCategory::EType type, int index = 0, std::string name = std::string(), DWORD constant = 0);
      /**
       * @brief Type of token
       */
      NCategory::EType mType;
      /**
       * @brief Index in group, it's used only for group types: DATA_TYPE, PREFIX, INSTRUCTION, REGISTER, SEGMENT
       */
      int mIndex;
      /**
       * @brief Contains name or string, it's used only for types: NAME or STRING
       */
      std::string mData;
      /**
       * @brief Contains constant value, it's used only for type CONSTANT
       */
      DWORD mConstant;
   };

   /**
    * @brief whether tokens match categories
    * @param[in] tokens tokens
    * @param[in] categories categories
    * @return True if they are matched, false otherwise
    */
   bool isMatch(const std::vector<SToken> & tokens, const std::vector<NCategory::EType> & categories);
   /**
    * @brief Parses source code and return list of tokens
    * @param[in] input source code
    * @return List of tokens
    */
   std::vector<std::vector<SToken> > parse(std::basic_istream<char, std::char_traits<char> > & input);
}

#endif

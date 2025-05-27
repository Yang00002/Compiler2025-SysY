
// Generated from SysY.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  SysYLexer : public antlr4::Lexer {
public:
  enum {
    INT = 1, FLOAT = 2, VOID = 3, IF = 4, ELSE = 5, WHILE = 6, CONTINUE = 7, 
    BREAK = 8, RETURN = 9, CONST = 10, ASSIGN = 11, ADD = 12, SUB = 13, 
    MUL = 14, DIV = 15, MOD = 16, EQ = 17, NE = 18, LT = 19, GT = 20, LE = 21, 
    GE = 22, OR = 23, AND = 24, NOT = 25, LPAREN = 26, RPAREN = 27, LBRACK = 28, 
    RBRACK = 29, LBRACE = 30, RBRACE = 31, COMMA = 32, SEMICO = 33, ID = 34, 
    IntConst = 35, FloatConst = 36, BlockComment = 37, LineComment = 38, 
    Whitespace = 39, Newline = 40
  };

  explicit SysYLexer(antlr4::CharStream *input);

  ~SysYLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};


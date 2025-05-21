
// Generated from SysY.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  SysYLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, INT = 2, FLOAT = 3, VOID = 4, IF = 5, ELSE = 6, WHILE = 7, 
    CONTINUE = 8, BREAK = 9, RETURN = 10, CONST = 11, ASSIGN = 12, ADD = 13, 
    SUB = 14, MUL = 15, DIV = 16, MOD = 17, EQ = 18, NE = 19, LT = 20, GT = 21, 
    LE = 22, GE = 23, OR = 24, AND = 25, NOT = 26, LPAREN = 27, RPAREN = 28, 
    LBRACK = 29, RBRACK = 30, LBRACE = 31, RBRACE = 32, COMMA = 33, ID = 34, 
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


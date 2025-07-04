#include "../antlr/SysYLexer.h"
#include "../antlr/SysYParser.h"
#include "../include/ast/Antlr2Ast.hpp"
#include "ast/Ast.hpp"

#include <CharStream.h>
#include <ostream>
#include <stdexcept>
#include <tree/ParseTree.h>
#include <tree/ParseTreeVisitor.h>
#include <tree/ParseTreeWalker.h>

#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  std::ifstream input_file(argv[1]);
  antlr4::ANTLRInputStream inputStream(input_file);
  SysYLexer lexer{&inputStream};
  antlr4::CommonTokenStream tokens(&lexer);
  SysYParser parser(&tokens);
  Antlr2AstVisitor visitor;
  antlr4::tree::ParseTree *ptree = parser.compUnit();
  ASTCompUnit *top = nullptr;
  try {
    top = visitor.astTree(ptree);
  } catch (std::runtime_error err) {
    std::cerr << err.what() << std::endl;
    top = nullptr;
  }
  input_file.close();
  std::ofstream output_file(argv[2]);
  output_file << ((top != nullptr) ? top->toString() : "error");
  output_file.close();
  if(top != nullptr) delete top;
  return 0;
}
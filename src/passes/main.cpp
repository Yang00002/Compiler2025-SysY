#include "../antlr/SysYLexer.h"
#include "../antlr/SysYParser.h"
#include "../include/ast/Antlr2Ast.hpp"
#include "../include/ast/Ast.hpp"
#include "../include/ir/AST2IR.hpp"
#include "../include/ir/Module.hpp"
#include "../include/passes/PassManager.hpp"
#include "DeadCode.hpp"
#include "Mem2Reg.hpp"

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
  if (top == nullptr) {
    output_file << "error";
    output_file.close();
    return 0;
  }
  AST2IRVisitor ast2ir;
  ast2ir.visit(top);
  delete top;
  auto module = ast2ir.getModule();
  PassManager* manager = new PassManager{module};
  manager->add_pass<Mem2Reg>();
  manager->add_pass<DeadCode>();
  manager->run();
  delete manager;
  output_file << module->print();
  output_file.close();
  delete module;
  return 0;
}
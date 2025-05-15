#include "../antlr/SysYBaseVisitor.h"
#include "../antlr/SysYLexer.h"
#include "../antlr/SysYParser.h"
#include <CharStream.h>
#include <iostream>
#include <tree/ParseTree.h>
#include <tree/ParseTreeVisitor.h>
#include <tree/ParseTreeWalker.h>

using namespace std;

class MyVisitor : public SysYBaseVisitor {
public:
  std::any visitCompUnit(SysYParser::CompUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  std::any visitDecl(SysYParser::DeclContext *ctx) override {
    return visitChildren(ctx);
  }
};

int main() {
  antlr4::ANTLRInputStream inputStream("int main(){}");
  SysYLexer lexer{&inputStream};
  antlr4::CommonTokenStream tokens(&lexer);
  SysYParser parser(&tokens);
  antlr4::tree::ParseTree *tree = parser.compUnit();
  MyVisitor visitor;
  visitor.visit(tree);
  return 0;
}
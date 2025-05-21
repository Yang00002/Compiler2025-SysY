#include "../include/SysYIRGenerator.hpp"
#include "../include/antlr/SysYLexer.h"
#include "../include/antlr/SysYParser.h"

#include <CharStream.h>
#include <tree/ParseTree.h>
#include <tree/ParseTreeVisitor.h>
#include <tree/ParseTreeWalker.h>

#include <fstream>
#include <iostream>


int main(int argc, char* argv[]) {
    std::ifstream input_file(argv[1]);
    antlr4::ANTLRInputStream inputStream(input_file);
    SysYLexer lexer{&inputStream};
    antlr4::CommonTokenStream tokens(&lexer);
    SysYParser parser(&tokens);
    antlr4::tree::ParseTree* ptree = parser.compUnit();
    SysYIRGenerator irgen;
    irgen.visit(ptree);
    return 0;
}
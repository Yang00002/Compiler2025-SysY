#include "../antlr/SysYLexer.h"
#include "../antlr/SysYParser.h"

#include "../include/ast/Antlr2Ast.hpp"
#include "../include/ir/AST2IR.hpp"

#include "../include/codegen/ARM_codegen.hpp"

#include <CharStream.h>
#include <tree/ParseTree.h>
#include <tree/ParseTreeVisitor.h>
#include <tree/ParseTreeWalker.h>

#include <fstream>
#include <iostream>
#include <tuple>
#include <string>

std::tuple<std::string, std::string, bool> parseArgs(int argc, char** argv)
{
    // compiler <testcase.sy> -S -o <testcase.s> [-O1]
     if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <testcase.sy> -S -o <testcase.s> [-O1]\n";
        std::exit(EXIT_FAILURE);
    }
    int i = 1;
    bool opt = false;
    auto input_filename = argv[i++];
    if (std::string(argv[i++]) != "-S") {
        std::cerr << "Expected -S after input.\n";
        std::exit(EXIT_FAILURE);
    }
    if (std::string(argv[i++]) != "-o") {
        std::cerr << "Expected -o for output file.\n";
        std::exit(EXIT_FAILURE);
    }
    if (i >= argc) {
        std::cerr << "Missing output file.\n";
        std::exit(EXIT_FAILURE);
    }
    auto output_filename = argv[i++];
    if (i < argc && std::string(argv[i]) == "-O1") {
        opt = true;
        ++i;
    }
    if (i != argc) {
        std::cerr << "Too many arguments provided.\n";
        std::exit(EXIT_FAILURE);
    }
    return std::make_tuple(input_filename,output_filename,opt);
}

int main(int argc, char* argv[]) {
    std::string infile, outfile;
    bool opt;
    std::tie(infile,outfile,opt) = parseArgs(argc, argv);

    std::ifstream input_file(infile);
    antlr4::ANTLRInputStream inputStream(input_file);
    SysYLexer lexer{&inputStream};
    antlr4::CommonTokenStream tokens(&lexer);
    input_file.close();
    SysYParser parser(&tokens);
    antlr4::tree::ParseTree* ptree = parser.compUnit();
    Antlr2AstVisitor MakeAst;
    auto ast = MakeAst.astTree(ptree);
    AST2IRVisitor MakeIR;
    MakeIR.visit(ast);
    auto m = MakeIR.getModule();

    if(opt){
        // Optimization Pass
    }
    
    auto c = ARMCodeGen(m);
    c.run();

    std::ofstream output_file(outfile);
    output_file << c.print();
    output_file.close();
    return 0;
}
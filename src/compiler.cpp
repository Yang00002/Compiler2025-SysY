#include <SysYLexer.h>
#include <SysYParser.h>

#include <Antlr2Ast.hpp>
#include <AST2IR.hpp>

#include <ARM_codegen.hpp>
#include "../include/passes/PassManager.hpp"
#include "../include/passes/DeadCode.hpp"
#include "../include/passes/LICM.hpp"
#include "../include/passes/Mem2Reg.hpp"

#include <CharStream.h>
#include <cstdlib>
#include <tree/ParseTree.h>
#include <tree/ParseTreeVisitor.h>
#include <tree/ParseTreeWalker.h>

#include <fstream>
#include <iostream>
#include <tuple>
#include <string>

std::tuple<std::string, std::string, bool> parseArgs(int argc, char** argv)
{
    // compiler -S -o <testcase.s> <testcase.sy> [-O1]
     if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " -S -o <testcase.s> <testcase.sy> [-O1]\n";
        std::exit(EXIT_FAILURE);
    }
    int i = 1;
    bool opt = false;
    std::string input_filename, output_filename;
    for(int i = 1; i < argc; ++i){
        std::string arg = argv[i];
        if(arg=="-S")
            continue;
        if(arg=="-o"){
            if( i + 1 < argc )
                output_filename = argv[++i];
            else {
                std::cerr << "Missing output filename.\n";
                std::exit(EXIT_FAILURE);
            }
        } else if(arg=="-O1")
            opt = true;
        else
            input_filename = arg;
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

    PassManager* pm = new PassManager{m};
    if(opt){
        // Optimization Pass
        pm->add_pass<Mem2Reg>();
        pm->add_pass<DeadCode>();
        pm->add_pass<LoopInvariantCodeMotion>();
        pm->add_pass<DeadCode>();
        pm->run();
    }
    
    auto c = ARMCodeGen(m);
    c.run();

    std::ofstream output_file(outfile);
    output_file << c.print();
    output_file.close();

    delete ast;
    delete m;
    
    return 0;
}
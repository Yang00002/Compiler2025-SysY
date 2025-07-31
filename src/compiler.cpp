#include <SysYLexer.h>
#include <SysYParser.h>

#include "Ast.hpp"
#include <AST2IR.hpp>
#include <Antlr2Ast.hpp>

#include "Arithmetic.hpp"
#include "CmpCombine.hpp"
#include "CodeGen.hpp"
#include "Config.hpp"
#include "CountLZ.hpp"
#include "CriticalEdgeRemove.hpp"
#include "DeadCode.hpp"
#include "GCM.hpp"
#include "GVN.hpp"
#include "LICM.hpp"
#include "MachineIR.hpp"
#include "Mem2Reg.hpp"
#include "Module.hpp"
#include "PassManager.hpp"
#include "SCCP.hpp"
#include <ARM_codegen.hpp>

#include "GraphColoring.hpp"
#include <CharStream.h>
#include <cstdlib>
#include <tree/ParseTree.h>
#include <tree/ParseTreeVisitor.h>
#include <tree/ParseTreeWalker.h>

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

std::tuple<std::string, std::string> parseArgs(int argc, char **argv) {
  // compiler -S -o <testcase.s> <testcase.sy> [-O1]
  if (argc < 5) {
    std::cerr << "Usage: " << argv[0]
              << " -S -o <testcase.s> <testcase.sy> [-O1] [-ast/ir/stack]\n";
    std::exit(EXIT_FAILURE);
  }
  int i = 1;
  std::string input_filename, output_filename;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-S")
      continue;
    if (arg == "-o") {
      if (i + 1 < argc)
        output_filename = argv[++i];
      else {
        std::cerr << "Missing output filename.\n";
        std::exit(EXIT_FAILURE);
      }
    } else if (arg == "-ast")
      emitAST = true;
    else if (arg == "-ir")
      emitIR = true;
    else if (arg == "-stack")
      useStack = true;
    else if (arg == "-O1")
      o1Optimization = true;
    else
      input_filename = arg;
  }
  return std::make_tuple(input_filename, output_filename);
}

void stack(std::string infile, std::string outfile) {
  std::ifstream input_file(infile);
  antlr4::ANTLRInputStream inputStream(input_file);
  SysYLexer lexer{&inputStream};
  antlr4::CommonTokenStream tokens(&lexer);
  input_file.close();
  SysYParser parser(&tokens);
  antlr4::tree::ParseTree *ptree = parser.compUnit();
  Antlr2AstVisitor MakeAst;
  auto ast = MakeAst.astTree(ptree);
  AST2IRVisitor MakeIR;
  MakeIR.visit(ast);
  delete ast;
  auto m = MakeIR.getModule();

  if (o1Optimization) {
    PassManager *pm = new PassManager{m};
    pm->add_pass<Mem2Reg>();
    pm->add_pass<DeadCode>();
    pm->add_pass<Arithmetic>();
    pm->add_pass<SCCP>();
    pm->add_pass<DeadCode>();
    pm->add_pass<LoopInvariantCodeMotion>();
    pm->add_pass<DeadCode>();
    pm->add_pass<GVN>();
    pm->add_pass<Arithmetic>();
    pm->add_pass<GlobalCodeMotion>();
    pm->add_pass<DeadCode>();
    pm->run();
    delete pm;
  }

  ARMCodeGen *cg = new ARMCodeGen{m};
  cg->run();
  std::ofstream output_file(outfile);
  output_file << cg->print();
  output_file.close();
  delete cg;
  delete m;
}

void ir(std::string infile, std::string outfile) {
  std::ifstream input_file(infile);
  antlr4::ANTLRInputStream inputStream(input_file);
  SysYLexer lexer{&inputStream};
  antlr4::CommonTokenStream tokens(&lexer);
  input_file.close();
  SysYParser parser(&tokens);
  antlr4::tree::ParseTree *ptree = parser.compUnit();
  Antlr2AstVisitor MakeAst;
  auto ast = MakeAst.astTree(ptree);
  AST2IRVisitor MakeIR;
  MakeIR.visit(ast);
  delete ast;
  auto m = MakeIR.getModule();

  if (o1Optimization) {
    PassManager *pm = new PassManager{m};
    pm->add_pass<Mem2Reg>();
    pm->add_pass<DeadCode>();
    pm->add_pass<Arithmetic>();
    pm->add_pass<SCCP>();
    pm->add_pass<DeadCode>();
    pm->add_pass<LoopInvariantCodeMotion>();
    pm->add_pass<DeadCode>();
    pm->add_pass<GVN>();
    pm->add_pass<Arithmetic>();
    pm->add_pass<GlobalCodeMotion>();
    pm->add_pass<DeadCode>();
    pm->run();
    delete pm;
  }

  std::ofstream output_file(outfile);
  output_file << m->print();
  output_file.close();
  delete m;
}

void ast(std::string infile, std::string outfile) {
  std::ifstream input_file(infile);
  antlr4::ANTLRInputStream inputStream(input_file);
  SysYLexer lexer{&inputStream};
  antlr4::CommonTokenStream tokens(&lexer);
  input_file.close();
  SysYParser parser(&tokens);
  antlr4::tree::ParseTree *ptree = parser.compUnit();
  Antlr2AstVisitor MakeAst;
  auto ast = MakeAst.astTree(ptree);
  std::ofstream output_file(outfile);
  output_file << ast->toString();
  output_file.close();
  delete ast;
}

void compiler(std::string infile, std::string outfile) {
  Module *m = nullptr;
  {
    ASTCompUnit *ast;
    std::ifstream input_file(infile);
    antlr4::ANTLRInputStream inputStream(input_file);
    SysYLexer lexer{&inputStream};
    antlr4::CommonTokenStream tokens(&lexer);
    input_file.close();
    SysYParser parser(&tokens);
    Antlr2AstVisitor MakeAst;
    ast = MakeAst.astTree(parser.compUnit());
    AST2IRVisitor MakeIR;
    MakeIR.visit(ast);
    delete ast;
    m = MakeIR.getModule();
  }

  PassManager *pm = new PassManager{m};
  if (o1Optimization) {
    // Optimization Pass
    pm->add_pass<Mem2Reg>();
    pm->add_pass<DeadCode>();
    pm->add_pass<Arithmetic>();
    pm->add_pass<SCCP>();
    pm->add_pass<DeadCode>();
    pm->add_pass<LoopInvariantCodeMotion>();
    pm->add_pass<DeadCode>();
    pm->add_pass<GVN>();
    pm->add_pass<Arithmetic>();
    pm->add_pass<GlobalCodeMotion>();
    pm->add_pass<DeadCode>();
  } else
    pm->add_pass<DeadCode>();
  pm->add_pass<CriticalEdgeERemove>();
  pm->add_pass<CmpCombine>();
  pm->run();
  delete pm;

  auto mir = new MModule();
  mir->accept(m);
  delete m;

  GraphColorSolver *solver = new GraphColorSolver{mir};
  solver->run();
  delete solver;

  CodeGen *cg = new CodeGen{mir};
  cg->run();

  std::ofstream output_file(outfile);
  output_file << cg->print();
  output_file.close();

  delete cg;
  delete mir;
}

void beforeRun() {
  if (sizeof(int) != 4)
    exit(-1);
  if (sizeof(long) != 4)
    exit(-2);
  if (sizeof(long long) != 8)
    exit(-3);
  if (sizeof(int *) != 8)
    exit(-4);
  /*

    if (m_countr_zero(2) != 1)
      exit(-5);
    if (!IS_SMALL_END)
      exit(-6);
  */
  /*
    std::ifstream input_file(R"(//test add
  int main(){
      int a, b;
      a = 10;
      b = -1;
      return a + b;
  })");
    antlr4::ANTLRInputStream inputStream(input_file);
    SysYLexer lexer{&inputStream};
    antlr4::CommonTokenStream tokens(&lexer);
    input_file.close();
    SysYParser parser(&tokens);
    antlr4::tree::ParseTree *ptree = parser.compUnit();
    Antlr2AstVisitor MakeAst;
    auto ast = MakeAst.astTree(ptree);
    delete ast;
  */
}

int main(int argc, char *argv[]) {
  if (testArchi)
    beforeRun();
  std::string infile, outfile;
  std::tie(infile, outfile) = parseArgs(argc, argv);
  if (emitAST)
    ast(infile, outfile);
  else if (emitIR)
    ir(infile, outfile);
  else if (useStack)
    stack(infile, outfile);
  else
    compiler(infile, outfile);
  return 0;
}
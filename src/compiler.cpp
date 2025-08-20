#include <SysYLexer.h>
#include <SysYParser.h>

#include "Ast.hpp"
#include <AST2IR.hpp>
#include <Antlr2Ast.hpp>
#include <cfloat>

#include "Arithmetic.hpp"
#include "BlockLayout.hpp"
#include "CleanCode.hpp"
#include "CmpCombine.hpp"
#include "CodeGen.hpp"
#include "Config.hpp"
#include "CountLZ.hpp"
#include "CriticalEdgeRemove.hpp"
#include "DeadCode.hpp"
#include "EmptyAntlrVisitor.hpp"
#include "FrameOffset.hpp"
#include "GCM.hpp"
#include "GVN.hpp"
#include "GetElementSplit.hpp"
#include "GlobalArrayReverse.hpp"
#include "Inline.hpp"
#include "InstructionSelect.hpp"
#include "LCSSA.hpp"
#include "LICM.hpp"
#include "LoopSimplify.hpp"
#include "MachineModule.hpp"
#include "Mem2Reg.hpp"
#include "Module.hpp"
#include "PassManager.hpp"
#include "Print.hpp"
#include "RegPrefill.hpp"
#include "RegisterAllocate.hpp"
#include "ReturnMerge.hpp"
#include "SCCP.hpp"
#include "LoopRotate.hpp"
#include "PhiEliminate.hpp"
#include <ARM_codegen.hpp>

#include "Ast.hpp"
#include "System.hpp"
#include <CharStream.h>
#include <climits>
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
  o1Optimization = false;
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

void toggleNO1DefaultSettings() {
  if (!o1Optimization) {
    replaceGlobalAddressWithRegisterNeedUseCount = INT_MAX;
    replaceAllocaAddressWithRegisterNeedTotalCost = FLT_MAX;
    mergeStackFrameSpilledWithGraphColoring = false;
    useCallerSaveRegsFirst = false;
    funcInlineGate = INT_MAX;
    epilogShouldMerge = INT_MAX;
    dangerousSignalInfer = false;
    ignoreNegativeArrayIndexes = false;
    useSinkForVirtualRegister = false;
  }
}

void addPasses4IR(PassManager *pm) {
  pm->add_pass<Mem2Reg>();
  pm->add_pass<DeadCode>();
  if (o1Optimization) {
	pm->add_pass<SCCP>();
	pm->add_pass<DeadCode>();
	pm->add_pass<Arithmetic>();
	pm->add_pass<DeadCode>();
	pm->add_pass<Inline>();
	pm->add_pass<LoopSimplify>();
	pm->add_pass<Print>();
	pm->add_pass<GlobalArrayReverse>();
	pm->add_pass<GetElementSplit>();
	pm->add_pass<LoopInvariantCodeMotion>();
	pm->add_pass<LCSSA>();
	pm->add_pass<LoopRotate>();
	pm->add_pass<SCCP>();
	pm->add_pass<DeadCode>();
	pm->add_pass<Arithmetic>();
	pm->add_pass<DeadCode>();
	pm->add_pass<PhiEliminate>();
	pm->add_pass<DeadCode>();
	pm->add_pass<GlobalCodeMotion>();
	pm->add_pass<Inline>();
	pm->add_pass<GVN>();
	pm->add_pass<DeadCode>();
  }
}

void addPasses4IR2MIR(PassManager *pm) {
  pm->add_pass<CriticalEdgeRemove>();
  pm->add_pass<CmpCombine>();
  if (o1Optimization)
    pm->add_pass<InstructionSelect>();
}

void stack(std::string infile, std::string outfile) {
  std::ifstream input_file(infile);
  antlr4::ANTLRInputStream inputStream(input_file);
  SysYLexer lexer{&inputStream};
  antlr4::CommonTokenStream tokens(&lexer);
  SysYParser parser(&tokens);
  antlr4::tree::ParseTree *ptree = parser.compUnit();
  Antlr2AstVisitor MakeAst;
  auto ast = MakeAst.astTree(ptree);
  input_file.close();
  AST2IRVisitor MakeIR;
  MakeIR.visit(ast);
  delete ast;
  auto m = MakeIR.getModule();

  PassManager *pm = new PassManager{m};
  addPasses4IR(pm);
  pm->run();
  delete pm;

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
  SysYParser parser(&tokens);
  antlr4::tree::ParseTree *ptree = parser.compUnit();
  Antlr2AstVisitor MakeAst;
  auto ast = MakeAst.astTree(ptree);
  input_file.close();
  AST2IRVisitor MakeIR;
  MakeIR.visit(ast);
  delete ast;
  auto m = MakeIR.getModule();

  PassManager *pm = new PassManager{m};
  addPasses4IR(pm);
  pm->run();
  delete pm;

  std::ofstream output_file(outfile);
  output_file << m->print();
  output_file.close();
  delete m;
}

void tree(std::string infile) {
  std::ifstream input_file(infile);
  antlr4::ANTLRInputStream inputStream(input_file);
  SysYLexer lexer{&inputStream};
  antlr4::CommonTokenStream tokens(&lexer);
  SysYParser parser(&tokens);
  antlr4::tree::ParseTree *ptree = parser.compUnit();
  EmptyAntlrVisitor MakeAst;
  MakeAst.tryVisit(ptree);
  input_file.close();
}

void ast(std::string infile, std::string outfile) {
  std::ifstream input_file(infile);
  antlr4::ANTLRInputStream inputStream(input_file);
  SysYLexer lexer{&inputStream};
  antlr4::CommonTokenStream tokens(&lexer);
  SysYParser parser(&tokens);
  antlr4::tree::ParseTree *ptree = parser.compUnit();
  Antlr2AstVisitor MakeAst;
  auto ast = MakeAst.astTree(ptree);
  input_file.close();
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
    SysYParser parser(&tokens);
    Antlr2AstVisitor MakeAst;
    ast = MakeAst.astTree(parser.compUnit());
    input_file.close();
    AST2IRVisitor MakeIR;
    MakeIR.visit(ast);
    delete ast;
    m = MakeIR.getModule();
  }

  PassManager *pm = new PassManager{m};
  addPasses4IR(pm);
  addPasses4IR2MIR(pm);
  pm->run();

  auto mir = new MModule();
  mir->accept(m);
  delete m;

  MachinePassManager *mng = new MachinePassManager{mir};

  if (o1Optimization) {
    mng->add_pass<RegPrefill>();
  }
  mng->add_pass<RegisterAllocate>();
  mng->add_pass<CleanCode>();
  mng->add_pass<FrameOffset>();
  mng->add_pass<CodeGen>();
  mng->add_pass<ReturnMerge>();
  if (o1Optimization) {
    mng->add_pass<BlockLayout>();
  }
  mng->run();
  delete mng;

  std::ofstream output_file(outfile);
  output_file << mir;
  output_file.close();

  delete mir;
}

void beforeRun() {}

int main(int argc, char *argv[]) {
  if (testArchi)
    beforeRun();
  std::string infile, outfile;
  std::tie(infile, outfile) = parseArgs(argc, argv);
  toggleNO1DefaultSettings();
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
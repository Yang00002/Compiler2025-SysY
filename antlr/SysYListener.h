
// Generated from SysY.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "SysYParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by SysYParser.
 */
class  SysYListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterCompUnit(SysYParser::CompUnitContext *ctx) = 0;
  virtual void exitCompUnit(SysYParser::CompUnitContext *ctx) = 0;

  virtual void enterCompDecl(SysYParser::CompDeclContext *ctx) = 0;
  virtual void exitCompDecl(SysYParser::CompDeclContext *ctx) = 0;

  virtual void enterDecl(SysYParser::DeclContext *ctx) = 0;
  virtual void exitDecl(SysYParser::DeclContext *ctx) = 0;

  virtual void enterConstDecl(SysYParser::ConstDeclContext *ctx) = 0;
  virtual void exitConstDecl(SysYParser::ConstDeclContext *ctx) = 0;

  virtual void enterBType(SysYParser::BTypeContext *ctx) = 0;
  virtual void exitBType(SysYParser::BTypeContext *ctx) = 0;

  virtual void enterConstDef(SysYParser::ConstDefContext *ctx) = 0;
  virtual void exitConstDef(SysYParser::ConstDefContext *ctx) = 0;

  virtual void enterConstInitVal(SysYParser::ConstInitValContext *ctx) = 0;
  virtual void exitConstInitVal(SysYParser::ConstInitValContext *ctx) = 0;

  virtual void enterConstArrayInitVal(SysYParser::ConstArrayInitValContext *ctx) = 0;
  virtual void exitConstArrayInitVal(SysYParser::ConstArrayInitValContext *ctx) = 0;

  virtual void enterVarDecl(SysYParser::VarDeclContext *ctx) = 0;
  virtual void exitVarDecl(SysYParser::VarDeclContext *ctx) = 0;

  virtual void enterVarDef(SysYParser::VarDefContext *ctx) = 0;
  virtual void exitVarDef(SysYParser::VarDefContext *ctx) = 0;

  virtual void enterInitVal(SysYParser::InitValContext *ctx) = 0;
  virtual void exitInitVal(SysYParser::InitValContext *ctx) = 0;

  virtual void enterArrayInitVal(SysYParser::ArrayInitValContext *ctx) = 0;
  virtual void exitArrayInitVal(SysYParser::ArrayInitValContext *ctx) = 0;

  virtual void enterFuncDef(SysYParser::FuncDefContext *ctx) = 0;
  virtual void exitFuncDef(SysYParser::FuncDefContext *ctx) = 0;

  virtual void enterFuncType(SysYParser::FuncTypeContext *ctx) = 0;
  virtual void exitFuncType(SysYParser::FuncTypeContext *ctx) = 0;

  virtual void enterFuncFParams(SysYParser::FuncFParamsContext *ctx) = 0;
  virtual void exitFuncFParams(SysYParser::FuncFParamsContext *ctx) = 0;

  virtual void enterFuncFParam(SysYParser::FuncFParamContext *ctx) = 0;
  virtual void exitFuncFParam(SysYParser::FuncFParamContext *ctx) = 0;

  virtual void enterBlock(SysYParser::BlockContext *ctx) = 0;
  virtual void exitBlock(SysYParser::BlockContext *ctx) = 0;

  virtual void enterBlockItem(SysYParser::BlockItemContext *ctx) = 0;
  virtual void exitBlockItem(SysYParser::BlockItemContext *ctx) = 0;

  virtual void enterStmt(SysYParser::StmtContext *ctx) = 0;
  virtual void exitStmt(SysYParser::StmtContext *ctx) = 0;

  virtual void enterExp(SysYParser::ExpContext *ctx) = 0;
  virtual void exitExp(SysYParser::ExpContext *ctx) = 0;

  virtual void enterCond(SysYParser::CondContext *ctx) = 0;
  virtual void exitCond(SysYParser::CondContext *ctx) = 0;

  virtual void enterLVal(SysYParser::LValContext *ctx) = 0;
  virtual void exitLVal(SysYParser::LValContext *ctx) = 0;

  virtual void enterPrimaryExp(SysYParser::PrimaryExpContext *ctx) = 0;
  virtual void exitPrimaryExp(SysYParser::PrimaryExpContext *ctx) = 0;

  virtual void enterNumber(SysYParser::NumberContext *ctx) = 0;
  virtual void exitNumber(SysYParser::NumberContext *ctx) = 0;

  virtual void enterUnaryExp(SysYParser::UnaryExpContext *ctx) = 0;
  virtual void exitUnaryExp(SysYParser::UnaryExpContext *ctx) = 0;

  virtual void enterRParen(SysYParser::RParenContext *ctx) = 0;
  virtual void exitRParen(SysYParser::RParenContext *ctx) = 0;

  virtual void enterFuncRParams(SysYParser::FuncRParamsContext *ctx) = 0;
  virtual void exitFuncRParams(SysYParser::FuncRParamsContext *ctx) = 0;

  virtual void enterMulExp(SysYParser::MulExpContext *ctx) = 0;
  virtual void exitMulExp(SysYParser::MulExpContext *ctx) = 0;

  virtual void enterAddExp(SysYParser::AddExpContext *ctx) = 0;
  virtual void exitAddExp(SysYParser::AddExpContext *ctx) = 0;

  virtual void enterRelExp(SysYParser::RelExpContext *ctx) = 0;
  virtual void exitRelExp(SysYParser::RelExpContext *ctx) = 0;

  virtual void enterEqExp(SysYParser::EqExpContext *ctx) = 0;
  virtual void exitEqExp(SysYParser::EqExpContext *ctx) = 0;

  virtual void enterLAndExp(SysYParser::LAndExpContext *ctx) = 0;
  virtual void exitLAndExp(SysYParser::LAndExpContext *ctx) = 0;

  virtual void enterLOrExp(SysYParser::LOrExpContext *ctx) = 0;
  virtual void exitLOrExp(SysYParser::LOrExpContext *ctx) = 0;

  virtual void enterConstExp(SysYParser::ConstExpContext *ctx) = 0;
  virtual void exitConstExp(SysYParser::ConstExpContext *ctx) = 0;


};


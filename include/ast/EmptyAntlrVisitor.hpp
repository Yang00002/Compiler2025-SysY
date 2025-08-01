#pragma once
#include "SysYBaseVisitor.h"

class EmptyAntlrVisitor final : SysYBaseVisitor
{
public:
	void tryVisit(antlr4::tree::ParseTree* tree);

	std::any visitCompUnit(SysYParser::CompUnitContext* context) override;

	std::any visitCompDecl(SysYParser::CompDeclContext* context) override;

	std::any visitDecl(SysYParser::DeclContext* context) override;

	std::any visitConstDecl(SysYParser::ConstDeclContext* context) override;

	std::any visitBType(SysYParser::BTypeContext* context) override;

	std::any visitConstDef(SysYParser::ConstDefContext* context) override;

	std::any visitConstInitVal(SysYParser::ConstInitValContext* context) override;

	std::any visitConstArrayInitVal(SysYParser::ConstArrayInitValContext* ctx) override;

	std::any visitVarDecl(SysYParser::VarDeclContext* context) override;

	std::any visitVarDef(SysYParser::VarDefContext* context) override;

	std::any visitInitVal(SysYParser::InitValContext* context) override;

	std::any visitArrayInitVal(SysYParser::ArrayInitValContext* ctx) override;

	std::any visitFuncDef(SysYParser::FuncDefContext* context) override;

	std::any visitFuncType(SysYParser::FuncTypeContext* context) override;

	std::any visitFuncFParams(SysYParser::FuncFParamsContext* context) override;

	std::any visitFuncFParam(SysYParser::FuncFParamContext* context) override;

	std::any visitBlock(SysYParser::BlockContext* context) override;

	std::any visitBlockItem(SysYParser::BlockItemContext* context) override;

	std::any visitStmt(SysYParser::StmtContext* context) override;

	std::any visitExp(SysYParser::ExpContext* context) override;

	std::any visitCond(SysYParser::CondContext* context) override;

	std::any visitLVal(SysYParser::LValContext* context) override;

	std::any visitPrimaryExp(SysYParser::PrimaryExpContext* context) override;

	std::any visitNumber(SysYParser::NumberContext* context) override;

	std::any visitUnaryExp(SysYParser::UnaryExpContext* context) override;

	std::any visitRParen(SysYParser::RParenContext* context) override;

	std::any visitFuncRParams(SysYParser::FuncRParamsContext* context) override;

	std::any visitMulExp(SysYParser::MulExpContext* context) override;

	std::any visitAddExp(SysYParser::AddExpContext* context) override;

	std::any visitRelExp(SysYParser::RelExpContext* context) override;

	std::any visitEqExp(SysYParser::EqExpContext* context) override;

	std::any visitLAndExp(SysYParser::LAndExpContext* context) override;

	std::any visitLOrExp(SysYParser::LOrExpContext* context) override;

	std::any visitConstExp(SysYParser::ConstExpContext* context) override;
};

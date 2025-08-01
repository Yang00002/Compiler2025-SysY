#include "EmptyAntlrVisitor.hpp"

void EmptyAntlrVisitor::tryVisit(antlr4::tree::ParseTree* tree)
{
	tree->accept(this);
}

// 主要节点 CompUnit
std::any EmptyAntlrVisitor::visitCompUnit(SysYParser::CompUnitContext* context)
{
	for (auto comp_decl : context->compDecl()) comp_decl->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitCompDecl(SysYParser::CompDeclContext* context)
{
	if (context->decl() == nullptr) context->funcDef()->accept(this);
	else context->decl()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitDecl(SysYParser::DeclContext* context)
{
	if (const auto const_dec = context->constDecl(); const_dec != nullptr)
		const_dec->accept(this);
	else context->varDecl()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitConstDecl(SysYParser::ConstDeclContext* context)
{
	for (auto const_def : context->constDef()) const_def->accept(this);
	context->bType()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitBType(SysYParser::BTypeContext* context)
{
	return {};
}

std::any EmptyAntlrVisitor::visitConstDef(SysYParser::ConstDefContext* context)
{
	for (const auto const_exp : context->constExp())
		const_exp->accept(this);
	context->constInitVal()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitConstInitVal(SysYParser::ConstInitValContext* context)
{
	if (context->constExp() != nullptr)
	{
		context->constExp()->accept(this);
		return {};
	}
	for (const auto& i : context->constArrayInitVal())
	{
		i->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitConstArrayInitVal(SysYParser::ConstArrayInitValContext* ctx)
{
	if (ctx->constExp() != nullptr)
	{
		ctx->constExp()->accept(this);
		return {};
	}
	for (const auto& i : ctx->constArrayInitVal())
	{
		i->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitVarDecl(SysYParser::VarDeclContext* context)
{
	context->bType()->accept(this);
	for (const auto& d : context->varDef())
	{
		d->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitVarDef(SysYParser::VarDefContext* context)
{
	for (const auto const_exp : context->constExp())
	{
		const_exp->accept(this);
	}
	if (context->initVal() == nullptr)
	{
		return {};
	}
	context->initVal()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitInitVal(SysYParser::InitValContext* context)
{
	if (context->exp() != nullptr)
	{
		context->exp()->accept(this);
		return {};
	}
	for (const auto& i : context->arrayInitVal())
	{
		i->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitArrayInitVal(SysYParser::ArrayInitValContext* ctx)
{
	if (ctx->exp() != nullptr)
	{
		ctx->exp()->accept(this);
		return {};
	}
	for (const auto& i : ctx->arrayInitVal())
	{
		i->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitFuncDef(SysYParser::FuncDefContext* context)
{
	context->funcType()->accept(this);
	if (context->funcFParams() != nullptr)
		context->funcFParams()->accept(this);
	context->block()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitFuncType(SysYParser::FuncTypeContext* context)
{
	return {};
}

std::any EmptyAntlrVisitor::visitFuncFParams(SysYParser::FuncFParamsContext* context)
{
	for (const auto& i : context->funcFParam())i->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitFuncFParam(SysYParser::FuncFParamContext* context)
{
	context->bType()->accept(this);

	for (const auto& i : context->exp())
	{
		i->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitBlock(SysYParser::BlockContext* context)
{
	for (const auto& i : context->blockItem())
	{
		i->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitBlockItem(SysYParser::BlockItemContext* context)
{
	if (context->decl() != nullptr)
	{
		context->decl()->accept(this);
	}
	else
	{
		context->stmt()->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitStmt(SysYParser::StmtContext* context)
{
	if (context->lVal() != nullptr)
	{
		context->lVal()->accept(this);
		context->exp()->accept(this);
		return {};
	}
	if (context->block() != nullptr)
	{
		context->block()->accept(this);
		return {};
	}
	if (context->IF() != nullptr)
	{
		context->cond()->accept(this);
		context->stmt()[0]->accept(this);
		if (context->ELSE() != nullptr)
			context->stmt()[1]->accept(this);
		return {};
	}
	if (context->WHILE() != nullptr)
	{
		context->cond()->accept(this);
		context->stmt()[0]->accept(this);
		return {};
	}
	if (context->BREAK() != nullptr)
	{
		return {};
	}
	if (context->CONTINUE() != nullptr)
	{
		return {};
	}
	if (context->RETURN() != nullptr)
	{
		if (context->exp() == nullptr)
		{
			return {};
		}
		context->exp()->accept(this);
		return {};
	}
	if (context->exp() != nullptr)
	{
		context->exp()->accept(this);
		return {};
	}
	return {};
}

std::any EmptyAntlrVisitor::visitExp(SysYParser::ExpContext* context)
{
	context->addExp()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitCond(SysYParser::CondContext* context)
{
	context->lOrExp()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitLVal(SysYParser::LValContext* context)
{
	for (const auto& i : context->exp())
	{
		i->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitPrimaryExp(SysYParser::PrimaryExpContext* context)
{
	if (context->exp() != nullptr)
		context->exp()->accept(this);
	else if (context->lVal() != nullptr)
		context->lVal()->accept(this);
	else context->number()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitNumber(SysYParser::NumberContext* context)
{
	return {};
}

std::any EmptyAntlrVisitor::visitUnaryExp(SysYParser::UnaryExpContext* context)
{
	if (context->primaryExp() != nullptr)
	{
		context->primaryExp()->accept(this);
		return {};
	}
	if (context->ID() != nullptr)
	{
		if (context->funcRParams() != nullptr)
			context->funcRParams()->accept(this);
		return {};
	}
	context->unaryExp()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitRParen(SysYParser::RParenContext* context)
{
	return {};
}

std::any EmptyAntlrVisitor::visitFuncRParams(SysYParser::FuncRParamsContext* context)
{
	for (const auto& i : context->exp())
		i->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitMulExp(SysYParser::MulExpContext* context)
{
	if (context->mulExp() == nullptr)
	{
		context->unaryExp()->accept(this);
		return {};
	}
	context->mulExp()->accept(this);
	context->unaryExp()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitAddExp(SysYParser::AddExpContext* context)
{
	if (context->addExp() == nullptr)
	{
		context->mulExp()->accept(this);
		return {};
	}
	context->addExp()->accept(this);
	context->mulExp()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitRelExp(SysYParser::RelExpContext* context)
{
	if (context->relExp() == nullptr)
	{
		context->addExp()->accept(this);
		return {};
	}
	context->relExp()->accept(this);
	context->addExp()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitEqExp(SysYParser::EqExpContext* context)
{
	if (context->eqExp() == nullptr)
	{
		context->relExp()->accept(this);
		return {};
	}
	context->eqExp()->accept(this);
	context->relExp()->accept(this);
	return {};
}

std::any EmptyAntlrVisitor::visitLAndExp(SysYParser::LAndExpContext* context)
{
	if (context->eqExp().size() == 1)
	{
		context->eqExp()[0]->accept(this);
		return {};
	}
	for (auto& i : context->eqExp())
	{
		i->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitLOrExp(SysYParser::LOrExpContext* context)
{
	if (context->lAndExp().size() == 1)
	{
		context->lAndExp()[0]->accept(this);
		return {};
	}
	for (auto& i : context->lAndExp())
	{
		i->accept(this);
	}
	return {};
}

std::any EmptyAntlrVisitor::visitConstExp(SysYParser::ConstExpContext* context)
{
	context->addExp()->accept(this);
	return {};
}

#include "../../include/ast/Antlr2Ast.hpp"


using namespace Types;
using namespace std;


ASTCompUnit* Antlr2AstVisitor::astTree(antlr4::tree::ParseTree* parseTree)
{
	return std::any_cast<ASTCompUnit*>(visit(parseTree));
}

// 主要节点 CompUnit
std::any Antlr2AstVisitor::visitCompUnit(SysYParser::CompUnitContext* context)
{
	auto comp_unit = new ASTCompUnit();
	structConstraint.emplace_back(comp_unit);
	for (const auto& decl : context->compDecl())
	{
		auto any_get = decl->accept(this);
		if (const auto fd = std::any_cast<std::list<ASTVarDecl*>>(&any_get); fd != nullptr)
		{
			auto l = *fd;
			for (auto& i : l)
				comp_unit->var_declarations_.emplace_back(i);
		}
	}
	auto main_ = dynamic_cast<ASTFuncDecl*>(comp_unit->findScope("main", true));
	if (main_ == nullptr) throw runtime_error("Expected a main function");
	if (!main_->args().empty()) throw runtime_error("Expected main function don't have args");
	if (main_->returnType() != INT) throw runtime_error("Expected main function return int");
	structConstraint.pop_back();
	return comp_unit;
}

std::any Antlr2AstVisitor::visitCompDecl(SysYParser::CompDeclContext* context)
{
	if (const auto decl = context->decl(); decl != nullptr)
	{
		return decl->accept(this);
	}
	return context->funcDef()->accept(this);
}

std::any Antlr2AstVisitor::visitDecl(SysYParser::DeclContext* context)
{
	if (const auto const_dec = context->constDecl(); const_dec != nullptr)
		return const_dec->accept(this);
	return context->varDecl()->accept(this);
}

std::any Antlr2AstVisitor::visitConstDecl(SysYParser::ConstDeclContext* context)
{
	list<ASTVarDecl*> defList;
	const auto block = structConstraint.back();
	for (const auto& d : context->constDef())
	{
		declarationTypeConstraint.push(std::any_cast<Type*>(context->bType()->accept(this)));
		auto decl = any_cast<ASTVarDecl*>(d->accept(this));
		defList.emplace_back(decl);
		if (!block->pushScope(decl))
			throw runtime_error("duplicate var declaration of id " + decl->id());
		decl->parent_ = block;
		declarationTypeConstraint.pop();
	}
	return defList;
}

std::any Antlr2AstVisitor::visitBType(SysYParser::BTypeContext* context)
{
	if (context->INT() != nullptr) return INT;
	return FLOAT;
}

std::any Antlr2AstVisitor::visitConstDef(SysYParser::ConstDefContext* context)
{
	vector<unsigned> dims;
	for (const auto const_exp : context->constExp())
	{
		const auto n = any_cast<ASTNumber*>(const_exp->accept(this));
		if (n->isFloat())throw runtime_error("Array can not have float dimension, context: " + const_exp->toString());
		if (n->toInteger() <= 0)
			throw runtime_error(
				"Array can not have dimension <= 0, context: " + const_exp->toString());
		dims.emplace_back(n->toInteger());
		delete n;
	}
	Type* t = declarationTypeConstraint.top();
	if (!dims.empty())
		t = arrayType(t, false, dims);
	auto decl = new ASTVarDecl(true, structConstraint.size() == 1, t);
	decl->id_ = context->ID()->toString();
	if (t->getTypeID() == TypeIDs::Array)
		t = dynamic_cast<ArrayType*>(t)->typeContained();
	if (t == INT)
		decl->initList_ = new Tensor{dims, InitializeValue{0}};
	else decl->initList_ = new Tensor{dims, InitializeValue{0.0f}};
	initTensorConstraint.push(decl->initList_->getData());
	context->constInitVal()->accept(this);
	initTensorConstraint.pop();
	return decl;
}

std::any Antlr2AstVisitor::visitConstInitVal(SysYParser::ConstInitValContext* context)
{
	const auto t = initTensorConstraint.top();
	const auto dft = t->tensorBelong()->defaultValue();
	if (context->constExp() != nullptr)
	{
		if (!t->tensorBelong()->getShape().empty())
			throw runtime_error(
				"designate an array to a number. context " + context->constExp()->toString());
		const auto exp = any_cast<ASTNumber*>(context->constExp()->accept(this));
		if (const auto exp2 = dynamic_cast<ASTNumber*>(exp->castTypeTo(dft.getExpressionType())); !t->append(
			exp2->toInitializeValue()))
			throw runtime_error(
				"fail to append value to initialize list. context " + context->constExp()->toString());
		delete exp;
		return {};
	}
	if (t->tensorBelong()->getShape().empty())
		throw runtime_error(
			"designate a number to an array. context " + context->toString());
	for (const auto& i : context->constArrayInitVal())
	{
		i->accept(this);
	}
	return {};
}

std::any Antlr2AstVisitor::visitConstArrayInitVal(SysYParser::ConstArrayInitValContext* ctx)
{
	const auto t = initTensorConstraint.top();
	const auto dft = t->tensorBelong()->defaultValue();
	if (ctx->constExp() != nullptr)
	{
		const auto exp = any_cast<ASTNumber*>(ctx->constExp()->accept(this));
		if (const auto exp2 = dynamic_cast<ASTNumber*>(exp->castTypeTo(dft.getExpressionType())); !t->append(
			exp2->toInitializeValue()))
			throw runtime_error(
				"fail to append value to initialize list. context " + ctx->constExp()->toString());
		delete exp;
		return {};
	}
	const auto tensor = t->makeSubTensor();
	if (tensor == nullptr)
		throw runtime_error(
			"fail to make new dimension in initialize list. context " + ctx->constExp()->toString());
	initTensorConstraint.push(tensor);
	for (const auto& i : ctx->constArrayInitVal())
	{
		i->accept(this);
	}
	initTensorConstraint.pop();
	return {};
}

std::any Antlr2AstVisitor::visitVarDecl(SysYParser::VarDeclContext* context)
{
	list<ASTVarDecl*> defList;
	const auto block = structConstraint.back();
	for (const auto& d : context->varDef())
	{
		declarationTypeConstraint.push(std::any_cast<Type*>(context->bType()->accept(this)));
		auto decl = any_cast<ASTVarDecl*>(d->accept(this));
		defList.emplace_back(decl);
		if (!block->pushScope(decl))
			throw runtime_error("duplicate var declaration of id " + decl->id());
		decl->parent_ = block;
		declarationTypeConstraint.pop();
	}
	return defList;
}

std::any Antlr2AstVisitor::visitVarDef(SysYParser::VarDefContext* context)
{
	vector<unsigned> dims;
	for (const auto const_exp : context->constExp())
	{
		const auto n = any_cast<ASTNumber*>(const_exp->accept(this));
		if (n->isFloat())throw runtime_error("Array can not have float dimension, context: " + const_exp->toString());
		if (n->toInteger() <= 0)
			throw runtime_error(
				"Array can not have dimension <= 0, context: " + const_exp->toString());
		dims.emplace_back(n->toInteger());
		delete n;
	}
	Type* t = declarationTypeConstraint.top();
	if (!dims.empty())
		t = arrayType(t, false, dims);
	auto decl = new ASTVarDecl(false, structConstraint.size() == 1, t);
	decl->id_ = context->ID()->toString();
	if (t->getTypeID() == TypeIDs::Array)
		t = dynamic_cast<ArrayType*>(t)->typeContained();
	if (context->initVal() == nullptr)
	{
		if (decl->is_global_)
		{
			if (t == INT)
				decl->initList_ = new Tensor{dims, InitializeValue{0}};
			else decl->initList_ = new Tensor{dims, InitializeValue{0.0f}};
		}
		return decl;
	}
	if (t == INT)
		decl->initList_ = new Tensor{dims, InitializeValue{0}};
	else decl->initList_ = new Tensor{dims, InitializeValue{0.0f}};
	initTensorConstraint.push(decl->initList_->getData());
	initVarNodeConstraint.push(decl);
	context->initVal()->accept(this);
	initVarNodeConstraint.pop();
	initTensorConstraint.pop();
	return decl;
}

std::any Antlr2AstVisitor::visitInitVal(SysYParser::InitValContext* context)
{
	const auto t = initTensorConstraint.top();
	const auto dft = t->tensorBelong()->defaultValue();
	if (context->exp() != nullptr)
	{
		if (!t->tensorBelong()->getShape().empty())
			throw runtime_error(
				"designate an array to a number. context " + context->exp()->toString());
		const auto exp = any_cast<ASTExpression*>(context->exp()->accept(this));
		if (structConstraint.size() == 1 && dynamic_cast<ASTNumber*>(exp) == nullptr)
			throw runtime_error(
				"global variable must have const init value. context " + context->exp()->toString());
		const auto exp2 = exp->castTypeTo(dft.getExpressionType());
		if (const auto num = dynamic_cast<ASTNumber*>(exp2); num != nullptr)
		{
			const InitializeValue ret = num->toInitializeValue();
			delete num;
			if (!t->append(ret))
				throw runtime_error(
					"fail to append value to initialize list. context " + context->exp()->toString());
		}
		else
		{
			if (!t->append(InitializeValue{exp2}))
				throw runtime_error(
					"fail to append value to initialize list. context " + context->exp()->toString());
			initVarNodeConstraint.top()->expressions_.emplace_back(exp2);
		}
		return {};
	}
	if (t->tensorBelong()->getShape().empty())
		throw runtime_error(
			"designate a number to an array. context " + context->toString());
	for (const auto& i : context->arrayInitVal())
	{
		i->accept(this);
	}
	return {};
}

std::any Antlr2AstVisitor::visitArrayInitVal(SysYParser::ArrayInitValContext* ctx)
{
	const auto t = initTensorConstraint.top();
	const auto dft = t->tensorBelong()->defaultValue();
	if (ctx->exp() != nullptr)
	{
		const auto exp = any_cast<ASTExpression*>(ctx->exp()->accept(this));
		if (structConstraint.size() == 1 && dynamic_cast<ASTNumber*>(exp) == nullptr)
			throw runtime_error(
				"global variable must have const init value. context " + ctx->exp()->toString());
		if (exp->getExpressionType() == FLOAT && dft.getExpressionType() == INT)
			throw runtime_error(
				"Can not case array init expression type from float to int");
		const auto exp2 = exp->castTypeTo(dft.getExpressionType());
		if (const auto num = dynamic_cast<ASTNumber*>(exp2); num != nullptr)
		{
			const InitializeValue ret = num->toInitializeValue();
			delete num;
			if (!t->append(ret))
				throw runtime_error(
					"fail to append value to initialize list. context " + ctx->exp()->toString());
		}
		else
		{
			if (!t->append(InitializeValue{exp2}))
				throw runtime_error(
					"fail to append value to initialize list. context " + ctx->exp()->toString());
			initVarNodeConstraint.top()->expressions_.emplace_back(exp2);
		}
	}
	const auto tensor = t->makeSubTensor();
	if (tensor == nullptr)
		throw runtime_error(
			"fail to make new dimension in initialize list. context " + ctx->exp()->toString());
	initTensorConstraint.push(tensor);
	for (const auto& i : ctx->arrayInitVal())
	{
		i->accept(this);
	}
	initTensorConstraint.pop();
	return {};
}

std::any Antlr2AstVisitor::visitFuncDef(SysYParser::FuncDefContext* context)
{
	const auto decl = new ASTFuncDecl(context->ID()->toString(), any_cast<Type*>(context->funcType()->accept(this)));
	const auto top = dynamic_cast<ASTCompUnit*>(structConstraint.front());
	decl->parent_ = top;
	top->func_declarations_.push_back(decl);
	if (!decl->pushScope(decl))
		throw runtime_error("duplicate function " + decl->id());
	structConstraint.emplace_back(decl->block_);
	vector<ASTVarDecl*> paras;
	if (context->funcFParams() != nullptr)
		paras = any_cast<vector<ASTVarDecl*>>(context->funcFParams()->accept(this));
	for (auto& i : paras)
	{
		i->parent_ = decl;
		decl->args_.emplace_back(i);
	}
	context->block()->accept(this);
	structConstraint.pop_back();
	return {};
}

std::any Antlr2AstVisitor::visitFuncType(SysYParser::FuncTypeContext* context)
{
	if (context->FLOAT()) return FLOAT;
	if (context->INT()) return INT;
	return VOID;
}

std::any Antlr2AstVisitor::visitFuncFParams(SysYParser::FuncFParamsContext* context)
{
	vector<ASTVarDecl*> decls;
	for (const auto& i : context->funcFParam()) decls.emplace_back(any_cast<ASTVarDecl*>(i->accept(this)));
	return decls;
}

std::any Antlr2AstVisitor::visitFuncFParam(SysYParser::FuncFParamContext* context)
{
	const auto id = context->ID()->toString();
	const auto btype = any_cast<Type*>(context->bType()->accept(this));
	vector<unsigned> dims;
	for (const auto& i : context->exp())
	{
		const auto exp = any_cast<ASTExpression*>(i->accept(this));
		const auto num = dynamic_cast<ASTNumber*>(exp);
		if (num == nullptr)
			throw runtime_error("function parameter array must have const dimension. context " + context->toString());
		if (num->getExpressionType() != INT)
			throw runtime_error(
				"function parameter array dimension size must be integer. context " + context->toString());
		int n = num->toInteger();
		if (n <= 0)
			throw runtime_error(
				"function parameter array dimension size must > 0. context " + context->toString());
		dims.emplace_back(n);
	}
	auto decl = new ASTVarDecl(false, false, btype);
	decl->id_ = id;
	if (!context->LBRACK().empty())
		decl->type_ = arrayType(btype, true, dims);
	if (!structConstraint.back()->pushScope(decl))
		throw runtime_error("duplicate scope " + decl->id());
	return decl;
}

std::any Antlr2AstVisitor::visitBlock(SysYParser::BlockContext* context)
{
	for (const auto& i : context->blockItem())
	{
		i->accept(this);
	}
	return {};
}

std::any Antlr2AstVisitor::visitBlockItem(SysYParser::BlockItemContext* context)
{
	const auto block = dynamic_cast<ASTBlock*>(structConstraint.back());
	if (context->decl() != nullptr)
	{
		auto decl = any_cast<list<ASTVarDecl*>>(context->decl()->accept(this));
		for (auto& d : decl)
			block->stmts_.emplace_back(d);
	}
	else
	{
		for (auto& stmt : any_cast<list<ASTStmt*>>(context->stmt()->accept(this)))
			block->stmts_.emplace_back(stmt);
	}
	return {};
}

std::any Antlr2AstVisitor::visitStmt(SysYParser::StmtContext* context)
{
	if (context->lVal() != nullptr)
	{
		const auto exp = any_cast<ASTExpression*>(context->lVal()->accept(this));
		const auto lv = dynamic_cast<ASTLVal*>(exp);
		if (lv == nullptr)
			throw runtime_error("assign value to not lvalue expression");
		if (lv->getDeclaration()->isConst())
			throw runtime_error("assign value to const value " + lv->getDeclaration()->id());
		auto v = any_cast<ASTExpression*>(context->exp()->accept(this));
		v = v->castTypeTo(lv->getExpressionType());
		const auto stmt = new ASTAssign(lv, v);
		lv->parent_ = stmt;
		v->parent_ = stmt;
		stmt->parent_ = structConstraint.back();
		return list<ASTStmt*>{stmt};
	}
	if (context->block() != nullptr)
	{
		auto block = new ASTBlock();
		block->parent_ = structConstraint.back();
		structConstraint.push_back(block);
		context->block()->accept(this);
		structConstraint.pop_back();
		if (block->isEmpty())
		{
			delete block;
			return list<ASTStmt*>{};
		}
		return list<ASTStmt*>{block};
	}
	if (context->IF() != nullptr)
	{
		auto cond = any_cast<ASTExpression*>(context->cond()->accept(this));
		list<ASTStmt*> if_st = any_cast<list<ASTStmt*>>(context->stmt()[0]->accept(this));
		list<ASTStmt*> else_st;
		if (context->ELSE() != nullptr)
			else_st = any_cast<list<ASTStmt*>>(context->stmt()[1]->accept(this));
		if (cond->getExpressionType() != BOOL)
		{
			auto condNum = dynamic_cast<ASTNumber*>(cond);
			if (condNum != nullptr)
			{
				if (condNum->getExpressionType() == INT)
				{
					condNum->type_ = BOOL;
					condNum->field_.b_value_ = condNum->field_.i_value_ != 0;
				}
				else
				{
					condNum->type_ = BOOL;
					condNum->field_.b_value_ = condNum->field_.f_value_ != 0.0f;
				}
			}
			else
			{
				auto n = new ASTMath2Logic(cond);
				n->not_ = false;
				cond->parent_ = n;
				cond = n;
			}
		}
		cond->parent_ = structConstraint.back();
		auto condNum = dynamic_cast<ASTNumber*>(cond);
		if (condNum != nullptr)
		{
			if (condNum->toBoolean())
			{
				delete cond;
				for (auto& i : else_st)delete i;
				return if_st;
			}
			for (auto& i : if_st)delete i;
			delete cond;
			return else_st;
		}
		auto condLogic = dynamic_cast<ASTLogicExp*>(cond);
		if (condLogic != nullptr)
		{
			if (condLogic->have_result_ != ASTLogicExp::UNDEFINE)
			{
				cond->parent_ = structConstraint.back();
				if (condLogic->have_result_ == ASTLogicExp::TRUE)
				{
					if_st.push_front(cond);
					for (auto& i : else_st) delete i;
					return if_st;
				}
				for (auto& i : if_st) delete i;
				else_st.push_front(cond);
				return else_st;
			}
		}
		if (if_st.empty())
		{
			if (else_st.empty())
			{
				auto l = AST::cutExpressionToOnlyLeaveFuncCall(condLogic);
				list<ASTStmt*> ret;
				for (auto& i : l) ret.emplace_back(i);
				return ret;
			}
			cond = new ASTEqual{true, condLogic, new ASTNumber{false}};
			condLogic->parent_ = cond;
			auto if_node = new ASTIf{cond};
			for (auto& i : else_st)
			{
				if_node->if_stmt_.emplace_back(i);
				i->parent_ = if_node;
			}
			cond->parent_ = if_node;
			return list<ASTStmt*>{if_node};
		}
		if (else_st.empty())
		{
			auto if_node = new ASTIf{cond};
			for (auto& i : if_st)
			{
				if_node->if_stmt_.emplace_back(i);
				i->parent_ = if_node;
			}
			cond->parent_ = if_node;
			return list<ASTStmt*>{if_node};
		}
		auto if_node = new ASTIf{cond};
		if_node->parent_ = structConstraint.back();
		for (auto& i : if_st)
		{
			if_node->if_stmt_.emplace_back(i);
			i->parent_ = if_node;
		}
		for (auto& i : else_st)
		{
			if_node->else_stmt_.emplace_back(i);
			i->parent_ = if_node;
		}
		cond->parent_ = if_node;
		return list<ASTStmt*>{if_node};
	}
	if (context->WHILE() != nullptr)
	{
		auto cond = any_cast<ASTExpression*>(context->cond()->accept(this));
		if (cond->getExpressionType() != BOOL)
		{
			auto condNum = dynamic_cast<ASTNumber*>(cond);
			if (condNum != nullptr)
			{
				if (condNum->getExpressionType() == INT)
				{
					condNum->type_ = BOOL;
					condNum->field_.b_value_ = condNum->field_.i_value_ != 0;
				}
				else
				{
					condNum->type_ = BOOL;
					condNum->field_.b_value_ = condNum->field_.f_value_ != 0.0f;
				}
			}
			else
			{
				auto n = new ASTMath2Logic(cond);
				n->not_ = false;
				cond->parent_ = n;
				cond = n;
			}
		}
		auto loop = new ASTWhile{cond};
		loop->parent_ = structConstraint.back();
		cond->parent_ = loop;
		structConstraint.push_back(loop);
		auto stmt = any_cast<list<ASTStmt*>>(context->stmt()[0]->accept(this));
		structConstraint.pop_back();
		for (auto& i : stmt) loop->stmt_.emplace_back(i);
		auto condNum = dynamic_cast<ASTNumber*>(cond);
		if (condNum != nullptr)
		{
			if (condNum->toBoolean())
				throw runtime_error("Infinite While Loop at context " + context->toString());
			delete loop;
			return list<ASTStmt*>{};
		}
		auto condLogic = dynamic_cast<ASTLogicExp*>(cond);
		if (condLogic != nullptr)
		{
			if (condLogic->have_result_ != ASTLogicExp::UNDEFINE)
			{
				if (condLogic->have_result_ == ASTLogicExp::TRUE)
					throw runtime_error("Infinite While Loop at context " + context->toString());
				cond->parent_ = structConstraint.back();
				loop->cond_ = nullptr;
				delete loop;
				return list<ASTStmt*>{condLogic};
			}
		}
		return list<ASTStmt*>{loop};
	}
	if (context->BREAK() != nullptr)
	{
		auto get = dynamic_cast<ASTWhile*>(structConstraint.back()->findParent([](const ASTNode* n)-> bool
		{
			return dynamic_cast<const ASTWhile*>(n) != nullptr;
		}));
		if (get == nullptr)
			throw runtime_error("break must be in an while loop!");
		auto ret = new ASTBreak{get};
		ret->parent_ = structConstraint.back();
		return list<ASTStmt*>{ret};
	}
	if (context->RETURN() != nullptr)
	{
		auto get = dynamic_cast<ASTFuncDecl*>(structConstraint.back()->findParent([](const ASTNode* n)-> bool
		{
			return dynamic_cast<const ASTFuncDecl*>(n) != nullptr;
		}));

		if (get == nullptr)
			throw runtime_error("break must be in an while loop!");
		if (context->exp() == nullptr)
		{
			if (get->returnType() != VOID)
			{
				throw runtime_error(
					"function with return type " + get->returnType()->toString() + " must return a value");
			}
			auto r = new ASTReturn{nullptr, get};
			r->parent_ = structConstraint.back();
			return list<ASTStmt*>{r};
		}
		if (get->returnType() == VOID)
		{
			throw runtime_error(
				"function with return type void should not return any value");
		}
		auto ret = any_cast<ASTExpression*>(context->exp()->accept(this));
		ret = ret->castTypeTo(get->returnType());
		auto r = new ASTReturn{ret, get};
		r->parent_ = structConstraint.back();
		return list<ASTStmt*>{r};
	}
	if (context->exp() != nullptr)
	{
		auto exp = any_cast<ASTExpression*>(context->exp()->accept(this));
		auto ret = AST::cutExpressionToOnlyLeaveFuncCall(exp);
		auto r = list<ASTStmt*>{};
		for (auto& i : ret)
		{
			i->parent_ = structConstraint.back();
			r.emplace_back(i);
		}
		return r;
	}
	return list<ASTStmt*>{};
}

std::any Antlr2AstVisitor::visitExp(SysYParser::ExpContext* context)
{
	return context->addExp()->accept(this);
}

std::any Antlr2AstVisitor::visitCond(SysYParser::CondContext* context)
{
	auto exp = any_cast<ASTExpression*>(context->lOrExp()->accept(this));
	if (exp->getExpressionType() == BOOL) return exp;
	auto num = dynamic_cast<ASTNumber*>(exp);
	if (num != nullptr)
	{
		if (num->getExpressionType() == INT)
		{
			num->field_.b_value_ = num->field_.i_value_ != 0;
			num->type_ = BOOL;
		}
		else
		{
			num->field_.b_value_ = num->field_.f_value_ != 0.0f;
			num->type_ = BOOL;
		}
		return static_cast<ASTExpression*>(num);
	}
	if (exp->getExpressionType() != INT && exp->getExpressionType() != BOOL)
	{
		throw runtime_error("Expression type " + exp->getExpressionType()->toString() + " can not convert to bool");
	}
	auto ret = new ASTMath2Logic{exp};
	ret->not_ = false;
	return static_cast<ASTExpression*>(ret);
}

std::any Antlr2AstVisitor::visitLVal(SysYParser::LValContext* context)
{
	const auto d = structConstraint.back()->findScope(context->ID()->toString(), false);
	if (d == nullptr)
		throw runtime_error("undefined scope " + context->ID()->toString() + " . context " + context->toString());
	const auto decl = dynamic_cast<ASTVarDecl*>(d);
	const auto indexCount = static_cast<int>(context->exp().size());
	if (decl->getType()->isArrayType())
	{
		const auto ar = dynamic_cast<ArrayType*>(decl->getType());
		const int ori = static_cast<int>(ar->dimensions().size()) +
		                (ar->getTypeID() == TypeIDs::ArrayInParameter ? 1 : 0);
		if (ori < indexCount)
			throw
				runtime_error(
					"Array " + context->ID()->toString() + " has " + to_string(ori) + " dimensions but get " +
					to_string(indexCount));
		vector<ASTExpression*> indexes;
		bool allConst = decl->isConst();
		int dimIdx = 0;
		for (const auto& i : context->exp())
		{
			const auto idx = any_cast<ASTExpression*>(i->accept(this));
			if (idx->getExpressionType() != INT)
				throw runtime_error("array index must be integer instead of " + idx->getExpressionType()->toString());
			if (const auto num = dynamic_cast<ASTNumber*>(idx); num != nullptr)
			{
				if (const int n = num->toInteger();
					n <= 0 || n >= static_cast<int>(ar->dimensions()[dimIdx]))
					throw runtime_error(
						"array index out of bound. expect [0, " + to_string(ar->dimensions()[dimIdx]) + "], get " +
						to_string(n));
			}
			else allConst = false;
			indexes.emplace_back(idx);
			dimIdx++;
		}
		if (allConst && ori == indexCount)
		{
			vector<int> constIndexes;
			for (const auto& i : indexes)
			{
				constIndexes.emplace_back(dynamic_cast<ASTNumber*>(i)->toInteger());
				delete i;
			}
			const auto l = decl->getInitList();
			return new ASTNumber(l->visitData()->getElement(constIndexes));
		}
		auto ret = static_cast<ASTExpression*>(new ASTLVal(decl, indexes));
		for (auto& i : indexes)
		{
			i->parent_ = ret;
			ret->haveFuncCall_ = ret->haveFuncCall_ || i->haveFuncCall_;
		}
		return ret;
	}
	if (indexCount > 0) throw runtime_error("Scalar " + context->ID()->toString() + " has no index");
	const auto exp = new ASTLVal(decl, {});
	return static_cast<ASTExpression*>(exp);
}

std::any Antlr2AstVisitor::visitPrimaryExp(SysYParser::PrimaryExpContext* context)
{
	if (context->exp() != nullptr)
		return context->exp()->accept(this);
	if (context->lVal() != nullptr)
		return context->lVal()->accept(this);
	return context->number()->accept(this);
}

std::any Antlr2AstVisitor::visitNumber(SysYParser::NumberContext* context)
{
	if (context->IntConst() != nullptr)
		return static_cast<ASTExpression*>(new ASTNumber(stoi(context->IntConst()->toString())));
	return static_cast<ASTExpression*>(new ASTNumber(stof(context->FloatConst()->toString())));
}

std::any Antlr2AstVisitor::visitUnaryExp(SysYParser::UnaryExpContext* context)
{
	if (context->primaryExp() != nullptr)
		return context->primaryExp()->accept(this);
	if (context->ID() != nullptr)
	{
		const auto id = context->ID()->toString();
		const auto f = structConstraint.front()->findScope(id, true);
		if (f == nullptr) throw runtime_error("Undeclared function " + id);
		const auto func = dynamic_cast<ASTFuncDecl*>(f);
		auto getArgs = any_cast<vector<ASTExpression*>>(context->funcRParams()->accept(this));
		const auto typeArgs = func->args();
		if (typeArgs.size() != getArgs.size())
			throw runtime_error("function call of " + id + " has incorrect args count. " + context->toString());
		const int size = static_cast<int>(getArgs.size());
		for (int i = 0; i < size; i++)
		{
			auto& get = getArgs[i];
			const auto& arg = typeArgs[i];
			const auto& type = arg->getType();
			const auto getType = get->getExpressionType();
			if (type == FLOAT || type == INT)
			{
				if (getType == FLOAT || getType == INT) get = get->castTypeTo(type);
			}
			if (type->getTypeID() == TypeIDs::ArrayInParameter)
			{
				if (getType->getTypeID() != TypeIDs::Array && getType->getTypeID() != TypeIDs::ArrayInParameter)
					throw runtime_error(
						"function " + id + " need a " + type->toString() + " instead of " + getType->toString() + " at "
						+ std::to_string(i));
				const auto arT = dynamic_cast<ArrayType*>(type);
				const auto arGT = dynamic_cast<ArrayType*>(getType);
				if (arT->typeContained() != arGT->typeContained())
					throw runtime_error(
						"function " + id + " need a " + type->toString() + " instead of " + getType->toString() + " at "
						+ std::to_string(i));
				// 库函数的输入参数并不满足维度要求
				if (func->isLibFunc()) continue;
				if (const auto tlv = dynamic_cast<ASTLVal*>(get); tlv != nullptr)
				{
					if (const auto lv = tlv->getDeclaration(); lv->isConst())
						throw runtime_error(
							"pass a const array " + lv->id() + " to function " + id + " at "
							+ std::to_string(i));
				}
				if (!arGT->canPassToFuncOf(arT))
					throw runtime_error(
						"function " + id + " need a " + type->toString() + " instead of " + getType->toString() + " at "
						+ std::to_string(i));
			}
		}
		const auto call = new ASTCall{func, getArgs};
		for (const auto& i : getArgs) i->parent_ = call;
		return static_cast<ASTExpression*>(call);
	}
	auto exp = any_cast<ASTExpression*>(context->unaryExp()->accept(this));
	if (context->ADD() != nullptr)
	{
		if (exp->getExpressionType() == BOOL)
			throw runtime_error("Bool expression exp can not use +exp");
		return exp;
	}
	if (context->SUB() != nullptr)
	{
		if (exp->getExpressionType() == BOOL)
			throw runtime_error("Bool expression exp can not use -exp");
		if (const auto n = dynamic_cast<ASTNeg*>(exp); n != nullptr)
		{
			exp = n->hold_;
			n->hold_ = nullptr;
			delete n;
			exp->parent_ = nullptr;
			return exp;
		}
		if (const auto n = dynamic_cast<ASTNumber*>(exp); n != nullptr)
		{
			if (n->getExpressionType() == INT)
			{
				n->field_.i_value_ = -n->field_.i_value_;
				return exp;
			}
			n->field_.f_value_ = -n->field_.f_value_;
			return exp;
		}
		const auto n = new ASTNeg(exp);
		exp->parent_ = n;
		n->haveFuncCall_ = exp->haveFuncCall_;
		return static_cast<ASTExpression*>(n);
	}
	if (const auto logic = dynamic_cast<ASTMath2Logic*>(exp); logic != nullptr)
	{
		logic->not_ = !logic->not_;
		return exp;
	}
	if (auto n = dynamic_cast<ASTNumber*>(exp); n != nullptr)
	{
		n = dynamic_cast<ASTNumber*>(n->castTypeTo(BOOL));
		n->field_.b_value_ = !n->field_.b_value_;
		return static_cast<ASTExpression*>(n);
	}
	const auto n = new ASTMath2Logic(exp);
	exp->parent_ = n;
	n->haveFuncCall_ = exp->haveFuncCall_;
	return static_cast<ASTExpression*>(n);
}

std::any Antlr2AstVisitor::visitFuncRParams(SysYParser::FuncRParamsContext* context)
{
	vector<ASTExpression*> exp;
	for (const auto& i : context->exp())
		exp.emplace_back(any_cast<ASTExpression*>(i->accept(this)));
	return exp;
}

std::any Antlr2AstVisitor::visitMulExp(SysYParser::MulExpContext* context)
{
	if (context->mulExp() == nullptr)
	{
		return context->unaryExp()->accept(this);
	}
	auto l = any_cast<ASTExpression*>(context->mulExp()->accept(this));
	auto r = any_cast<ASTExpression*>(context->unaryExp()->accept(this));
	const auto op = context->MUL() != nullptr ? MathOP::MUL : context->DIV() != nullptr ? MathOP::DIV : MathOP::MOD;
	const auto tyT = ASTExpression::maxType(l, r);
	if (const auto rnum = dynamic_cast<ASTNumber*>(r); rnum != nullptr)
	{
		if (op != MathOP::MUL)
		{
			const auto ty = r->getExpressionType();
			if (ty == FLOAT && rnum->toFloat() == 0.0f)
				throw runtime_error(
					"Divide by 0. context " + context->toString());
			if (ty == INT && rnum->toInteger() == 0)
				throw runtime_error(
					"Divide by 0. context " + context->toString());
		}
	}
	if (op == MathOP::MOD && tyT == FLOAT)
		throw runtime_error(
			"Float can not use %. context " + context->toString());
	const auto lnum = dynamic_cast<ASTNumber*>(l);
	const auto rnum = dynamic_cast<ASTNumber*>(r);
	if (lnum != nullptr && rnum != nullptr)
	{
		if (tyT == INT)
		{
			const int a = lnum->forceToInteger();
			const int b = rnum->forceToInteger();
			lnum->type_ = INT;
			delete rnum;
			if (op == MathOP::MUL)
			{
				lnum->field_.i_value_ = a * b;
				return static_cast<ASTExpression*>(lnum);
			}
			if (op == MathOP::DIV)
			{
				lnum->field_.i_value_ = a / b;
				return static_cast<ASTExpression*>(lnum);
			}
			lnum->field_.i_value_ = a % b;
			return static_cast<ASTExpression*>(lnum);
		}
		const float a = lnum->forceToFloat();
		const float b = rnum->forceToFloat();
		lnum->type_ = FLOAT;
		delete rnum;
		if (op == MathOP::MUL)
		{
			lnum->field_.f_value_ = a * b;
			return static_cast<ASTExpression*>(lnum);
		}
		lnum->field_.f_value_ = a / b;
		return static_cast<ASTExpression*>(lnum);
	}
	l = l->castTypeTo(tyT);
	r = r->castTypeTo(tyT);
	const auto ln = dynamic_cast<ASTNeg*>(l);
	if (const auto rn = dynamic_cast<ASTNeg*>(r); ln != nullptr && rn != nullptr)
	{
		l = ln->hold_;
		ln->hold_ = nullptr;
		delete ln;
		r = rn->hold_;
		rn->hold_ = nullptr;
		delete rn;
	}
	else if (ln != nullptr || rn != nullptr)
	{
		const auto n = ln != nullptr ? ln : rn;
		if (ln != nullptr)l = n->hold_;
		else r = n->hold_;
		const auto node = new ASTMathExp(tyT, op, l, r);
		l->setParent(node);
		r->setParent(node);
		node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
		node->setParent(n);
		n->hold_ = node;
		n->haveFuncCall_ = node->haveFuncCall_;
		return static_cast<ASTExpression*>(n);
	}
	const auto node = new ASTMathExp(tyT, op, l, r);
	node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
	l->setParent(node);
	r->setParent(node);
	return static_cast<ASTExpression*>(node);
}

std::any Antlr2AstVisitor::visitAddExp(SysYParser::AddExpContext* context)
{
	if (context->addExp() == nullptr)
	{
		return context->mulExp()->accept(this);
	}
	auto l = any_cast<ASTExpression*>(context->addExp()->accept(this));
	auto r = any_cast<ASTExpression*>(context->mulExp()->accept(this));
	const auto op = context->ADD() == nullptr ? MathOP::SUB : MathOP::ADD;
	const auto tyT = ASTExpression::maxType(l, r);
	const auto lm = dynamic_cast<ASTNumber*>(l);
	const auto rm = dynamic_cast<ASTNumber*>(l);
	if (lm != nullptr && rm != nullptr)
	{
		if (tyT == INT)
		{
			int a = lm->forceToInteger();
			int b = rm->forceToInteger();
			lm->type_ = INT;
			delete rm;
			if (op == MathOP::ADD) lm->field_.i_value_ = a + b;
			else lm->field_.i_value_ = a - b;
			return static_cast<ASTExpression*>(lm);
		}
		float a = lm->forceToFloat();
		float b = rm->forceToFloat();
		lm->type_ = FLOAT;
		delete rm;
		if (op == MathOP::ADD) lm->field_.f_value_ = a + b;
		else lm->field_.f_value_ = a - b;
		return static_cast<ASTExpression*>(lm);
	}
	const auto ln = dynamic_cast<ASTNeg*>(l);
	const auto rn = dynamic_cast<ASTNeg*>(r);
	const bool nl = ln != nullptr;
	const bool nr = rn != nullptr;
	if (nl)
	{
		if (nr)
		{
			if (op == MathOP::ADD)
			{
				l = ln->hold_;
				ln->hold_ = nullptr;
				delete ln;
				r = rn->hold_;
				l = l->castTypeTo(tyT);
				r = r->castTypeTo(tyT);
				const auto node = new ASTMathExp(tyT, op, l, r);
				l->setParent(node);
				r->setParent(node);
				node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
				node->setParent(rn);
				rn->hold_ = node;
				rn->haveFuncCall_ = node->haveFuncCall_;
				return static_cast<ASTExpression*>(rn);
			}
			l = ln->hold_;
			ln->hold_ = nullptr;
			delete ln;
			r = rn->hold_;
			rn->hold_ = nullptr;
			delete rn;
			l = l->castTypeTo(tyT);
			r = r->castTypeTo(tyT);
			const auto node = new ASTMathExp(tyT, op, r, l);
			l->setParent(node);
			r->setParent(node);
			node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
			return static_cast<ASTExpression*>(node);
		}
		if (op == MathOP::ADD)
		{
			l = ln->hold_;
			ln->hold_ = nullptr;
			delete ln;
			l = l->castTypeTo(tyT);
			r = r->castTypeTo(tyT);
			const auto node = new ASTMathExp(tyT, MathOP::SUB, r, l);
			l->setParent(node);
			r->setParent(node);
			node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
			return static_cast<ASTExpression*>(node);
		}
		l = ln->hold_;
		l = l->castTypeTo(tyT);
		r = r->castTypeTo(tyT);
		const auto node = new ASTMathExp(tyT, MathOP::ADD, l, r);
		l->setParent(node);
		r->setParent(node);
		node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
		node->setParent(ln);
		ln->haveFuncCall_ = node->haveFuncCall_;
		ln->hold_ = node;
		return static_cast<ASTExpression*>(ln);
	}
	if (nr)
	{
		if (op == MathOP::ADD)
		{
			r = rn->hold_;
			rn->hold_ = nullptr;
			delete rn;
			l = l->castTypeTo(tyT);
			r = r->castTypeTo(tyT);
			const auto node = new ASTMathExp(tyT, MathOP::SUB, l, r);
			l->setParent(node);
			r->setParent(node);
			node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
			return static_cast<ASTExpression*>(node);
		}
		r = rn->hold_;
		rn->hold_ = nullptr;
		delete rn;
		l = l->castTypeTo(tyT);
		r = r->castTypeTo(tyT);
		const auto node = new ASTMathExp(tyT, MathOP::ADD, l, r);
		l->setParent(node);
		r->setParent(node);
		node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
		return static_cast<ASTExpression*>(node);
	}
	l = l->castTypeTo(tyT);
	r = r->castTypeTo(tyT);
	const auto node = new ASTMathExp(tyT, op, l, r);
	l->setParent(node);
	r->setParent(node);
	node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
	return static_cast<ASTExpression*>(node);
}

std::any Antlr2AstVisitor::visitRelExp(SysYParser::RelExpContext* context)
{
	if (context->relExp() == nullptr)
	{
		return context->addExp()->accept(this);
	}
	auto l = any_cast<ASTExpression*>(context->relExp()->accept(this));
	auto r = any_cast<ASTExpression*>(context->addExp()->accept(this));
	auto tyT = ASTExpression::maxType(l, r);
	l = l->castTypeTo(tyT);
	r = r->castTypeTo(tyT);
	auto lNum = dynamic_cast<ASTNumber*>(l);
	auto rNum = dynamic_cast<ASTNumber*>(r);
	if (lNum != nullptr && rNum != nullptr)
	{
		bool res;
		if (context->LT() != nullptr)
		{
			if (lNum->isInteger())
				res = lNum->toInteger() < rNum->toInteger();
			else res = lNum->toFloat() < rNum->toFloat();
		}
		else if (context->GT() != nullptr)
		{
			if (lNum->isInteger())
				res = lNum->toInteger() > rNum->toInteger();
			else res = lNum->toFloat() > rNum->toFloat();
		}
		else if (context->LE() != nullptr)
		{
			if (lNum->isInteger())
				res = lNum->toInteger() <= rNum->toInteger();
			else res = lNum->toFloat() <= rNum->toFloat();
		}
		else
		{
			if (lNum->isInteger())
				res = lNum->toInteger() >= rNum->toInteger();
			else res = lNum->toFloat() >= rNum->toFloat();
		}
		delete rNum;
		lNum->field_.b_value_ = res;
		lNum->type_ = BOOL;
		lNum->parent_ = nullptr;
		return static_cast<ASTExpression*>(lNum);
	}
	RelationOP op;
	if (context->LT() != nullptr) op = RelationOP::LT;
	else if (context->GT() != nullptr) op = RelationOP::GT;
	else if (context->LE() != nullptr) op = RelationOP::LE;
	else op = RelationOP::GE;
	auto node = new ASTRelation{op, l, r};
	l->parent_ = node;
	r->parent_ = node;
	node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
	return static_cast<ASTExpression*>(node);
}

std::any Antlr2AstVisitor::visitEqExp(SysYParser::EqExpContext* context)
{
	if (context->eqExp() == nullptr)return context->relExp()->accept(this);
	auto l = any_cast<ASTExpression*>(context->eqExp()->accept(this));
	auto r = any_cast<ASTExpression*>(context->relExp()->accept(this));
	auto tyL = l->getExpressionType();
	auto tyR = r->getExpressionType();
	if (tyL == BOOL && tyR == BOOL)
	{
		auto lNum = dynamic_cast<ASTNumber*>(l);
		auto rNum = dynamic_cast<ASTNumber*>(r);
		if (lNum != nullptr && rNum != nullptr)
		{
			bool res = (context->EQ()
				            ? (lNum->toBoolean() == rNum->toBoolean())
				            : (lNum->toBoolean() != rNum->toBoolean()));
			delete rNum;
			lNum->field_.b_value_ = res;
			lNum->parent_ = nullptr;
			return static_cast<ASTExpression*>(lNum);
		}
		auto node = new ASTEqual{context->EQ() != nullptr, l, r};
		l->parent_ = node;
		r->parent_ = node;
		node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
		return static_cast<ASTExpression*>(node);
	}
	auto tyT = ASTExpression::maxType(l, r);
	l = l->castTypeTo(tyT);
	r = r->castTypeTo(tyT);
	auto lNum = dynamic_cast<ASTNumber*>(l);
	auto rNum = dynamic_cast<ASTNumber*>(r);
	if (lNum != nullptr && rNum != nullptr)
	{
		bool res;
		if (context->EQ() != nullptr)
		{
			if (lNum->isInteger())
				res = lNum->toInteger() == rNum->toInteger();
			else res = lNum->toFloat() == rNum->toFloat();
		}
		else if (lNum->isInteger())
			res = lNum->toInteger() != rNum->toInteger();
		else res = lNum->toFloat() != rNum->toFloat();
		delete rNum;
		lNum->field_.b_value_ = res;
		lNum->type_ = BOOL;
		lNum->parent_ = nullptr;
		return static_cast<ASTExpression*>(lNum);
	}
	auto node = new ASTEqual{context->EQ() != nullptr, l, r};
	l->parent_ = node;
	r->parent_ = node;
	node->haveFuncCall_ = l->haveFuncCall_ || r->haveFuncCall_;
	return static_cast<ASTExpression*>(node);
}

std::any Antlr2AstVisitor::visitLAndExp(SysYParser::LAndExpContext* context)
{
	if (context->eqExp().size() == 1) return context->eqExp()[0]->accept(this);
	bool cut = false;
	bool comp = false;
	vector<ASTExpression*> exps;
	for (auto& i : context->eqExp())
	{
		auto exp = any_cast<ASTExpression*>(i->accept(this));
		if (exp->getExpressionType() != BOOL)
			throw runtime_error(
				"logical expression have bool type but get " + exp->getExpressionType()->toString());
		if (auto num = dynamic_cast<ASTNumber*>(exp); num != nullptr)
		{
			if (num->toBoolean() == false)
			{
				// 既然可以编译期间计算得到结果, 就不存在副作用
				cut = true;
				delete exp;
			}
			else
			{
				// 既然可以编译期间计算得到结果, 就不存在副作用
				delete exp;
			}
		}
		else if (!cut)
		{
			exps.emplace_back(exp);
			if (exp->haveFuncCall_) comp = true;
		}
		else delete exp;
	}
	if (!comp)
	{
		if (cut)
		{
			for (auto& i : exps) delete i;
			return static_cast<ASTExpression*>(new ASTNumber(false));
		}
		if (exps.empty()) return static_cast<ASTExpression*>(new ASTNumber(true));
		if (exps.size() == 1) return exps[0];
		auto ret = new ASTLogicExp{LogicOP::AND, exps, ASTLogicExp::UNDEFINE};
		for (auto& i : exps) i->parent_ = ret;
		return static_cast<ASTExpression*>(ret);
	}
	if (cut)
	{
		vector<ASTExpression*> e;
		for (auto& i : exps)
		{
			if (i->haveFuncCall_)e.push_back(i);
			else delete i;
		}
		exps.emplace_back(new ASTNumber{false});
		auto ret = new ASTLogicExp{LogicOP::AND, exps, ASTLogicExp::FALSE};
		ret->haveFuncCall_ = true;
		for (auto& i : exps) i->parent_ = ret;
		return static_cast<ASTExpression*>(ret);
	}
	auto ret = new ASTLogicExp{LogicOP::AND, exps, ASTLogicExp::UNDEFINE};
	ret->haveFuncCall_ = true;
	for (auto& i : exps) i->parent_ = ret;
	return static_cast<ASTExpression*>(ret);
}

std::any Antlr2AstVisitor::visitLOrExp(SysYParser::LOrExpContext* context)
{
	if (context->lAndExp().size() == 1) return context->lAndExp()[0]->accept(this);
	bool cut = false;
	bool comp = false;
	bool undef = false;
	vector<ASTExpression*> exps;
	for (auto& i : context->lAndExp())
	{
		auto exp = any_cast<ASTExpression*>(i->accept(this));
		if (exp->getExpressionType() != BOOL)
			throw runtime_error("logical expression have bool type but get " + exp->getExpressionType()->toString());

		if (auto num = dynamic_cast<ASTNumber*>(exp); num != nullptr)
		{
			if (num->toBoolean() == true)
			{
				// 既然可以编译期间计算得到结果, 就不存在副作用
				cut = true;
				delete exp;
			}
			else
			{
				// 既然可以编译期间计算得到结果, 就不存在副作用
				delete exp;
			}
		}
		else if (!cut)
		{
			if (auto log = dynamic_cast<ASTLogicExp*>(exp); log != nullptr)
			{
				// 如果 have_result_, 一定 haveFuncCall_; 否则该是 Number
				if (log->have_result_ == ASTLogicExp::TRUE)
				{
					cut = true;
					comp = true;
				}
				else if (log->have_result_ == ASTLogicExp::UNDEFINE)
					undef = true;
			}
			else
				undef = true;
			exps.emplace_back(exp);
			if (exp->haveFuncCall_) comp = true;
		}
		else delete exp;
	}
	if (!comp)
	{
		if (cut)
		{
			for (auto& i : exps) delete i;
			return static_cast<ASTExpression*>(new ASTNumber(true));
		}
		if (exps.empty()) return static_cast<ASTExpression*>(new ASTNumber(false));
		if (exps.size() == 1) return exps[0];
		auto ret = new ASTLogicExp{LogicOP::OR, exps, ASTLogicExp::UNDEFINE};
		for (auto& i : exps) i->parent_ = ret;
		return static_cast<ASTExpression*>(ret);
	}
	if (cut)
	{
		vector<ASTExpression*> e;
		for (auto& i : exps)
		{
			if (i->haveFuncCall_)e.push_back(i);
			else delete i;
		}
		if (auto log = dynamic_cast<ASTLogicExp*>(exps.back()); log != nullptr)
		{
			if (log->have_result_ == ASTLogicExp::TRUE)
			{
				if (exps.size() == 1) return exps[0];
				auto ret = new ASTLogicExp{LogicOP::OR, exps, ASTLogicExp::TRUE};
				for (auto& i : exps) i->parent_ = ret;
				ret->haveFuncCall_ = true;
				return static_cast<ASTExpression*>(ret);
			}
		}
		exps.emplace_back(new ASTNumber{true});
		auto ret = new ASTLogicExp{LogicOP::OR, exps, ASTLogicExp::TRUE};
		ret->haveFuncCall_ = true;
		for (auto& i : exps) i->parent_ = ret;
		return static_cast<ASTExpression*>(ret);
	}
	auto ret = new ASTLogicExp{LogicOP::OR, exps, undef ? ASTLogicExp::UNDEFINE : ASTLogicExp::FALSE};
	ret->haveFuncCall_ = true;
	for (auto& i : exps) i->parent_ = ret;
	return static_cast<ASTExpression*>(ret);
}

std::any Antlr2AstVisitor::visitConstExp(SysYParser::ConstExpContext* context)
{
	const auto exp = any_cast<ASTExpression*>(context->addExp()->accept(this));
	const auto num = dynamic_cast<ASTNumber*>(exp);
	if (num == nullptr)
		throw runtime_error(
			"constExp should have const value, context " + context->addExp()->toString());
	return num;
}

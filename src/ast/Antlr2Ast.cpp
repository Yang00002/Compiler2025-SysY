#include "Antlr2Ast.hpp"
#include "Ast.hpp"
#include "Type.hpp"
#include "Tensor.hpp"

#define DEBUG 0
#include "Util.hpp"


using namespace Types;
using namespace std;


ASTCompUnit* Antlr2AstVisitor::astTree(antlr4::tree::ParseTree* parseTree)
{
	parseTree->accept(this);
	return dynamic_cast<ASTCompUnit*>(returnSlot_.front());
}

bool Antlr2AstVisitor::pushScope(ASTDecl* decl) const
{
	return _structConstraint.back()->pushScope(decl);
}

ASTDecl* Antlr2AstVisitor::findScope(const std::string& id, bool isFunc)
{
	for (auto it = _structConstraint.rbegin(); it != _structConstraint.rend(); ++it)
	{
		const auto node = *it;
		if (const auto fd = node->findScope(id, isFunc); fd != nullptr) return fd;
	}
	return nullptr;
}

// 主要节点 CompUnit
std::any Antlr2AstVisitor::visitCompUnit(SysYParser::CompUnitContext* context)
{
	LOG(color::cyan("visitCompUnit"));
	PUSH;
	auto comp_unit = new ASTCompUnit();
	_structConstraint.emplace_back(comp_unit);
	for (const auto& decl : context->compDecl())
	{
		decl->accept(this);
		while (!returnSlot_.empty())
		{
			comp_unit->_var_declarations.emplace_back(dynamic_cast<ASTVarDecl*>(poll()));
		}
	}
	auto main_ = dynamic_cast<ASTFuncDecl*>(comp_unit->findScope("main", true));
	ASSERT(main_ != nullptr);
	ASSERT(main_->args().empty());
	ASSERT(main_->returnType() == INT);
	_structConstraint.pop_back();
	returnSlot_.emplace(comp_unit);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitCompDecl(SysYParser::CompDeclContext* context)
{
	LOG(color::cyan("visitCompDecl"));
	if (const auto decl = context->decl(); decl != nullptr)
	{
		decl->accept(this);
		POP;
		return {};
	}
	context->funcDef()->accept(this);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitDecl(SysYParser::DeclContext* context)
{
	LOG(color::cyan("visitDecl"));
	PUSH;
	if (const auto const_dec = context->constDecl(); const_dec != nullptr)
	{
		const_dec->accept(this);
		POP;
		return {};
	}
	context->varDecl()->accept(this);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitConstDecl(SysYParser::ConstDeclContext* context)
{
	LOG(color::cyan("visitConstDecl"));
	PUSH;
	list<ASTVarDecl*> defList;
	const auto block = _structConstraint.back();
	context->bType()->accept(this);
	_declarationTypeConstraint.push(typeReturnSlot_);
	for (const auto& d : context->constDef())
	{
		d->accept(this);
		auto decl = dynamic_cast<ASTVarDecl*>(poll());
		defList.emplace_back(decl);
		if (!block->pushScope(decl))
			throw runtime_error("duplicate var declaration of id " + decl->id());
	}
	_declarationTypeConstraint.pop();
	POP;
	for (auto i : defList) returnSlot_.emplace(i);
	return {};
}

std::any Antlr2AstVisitor::visitBType(SysYParser::BTypeContext* context)
{
	LOG(color::green("visitBType ") + (context->INT() != nullptr ? "INT" : "FLOAT"));
	if (context->INT() != nullptr) typeReturnSlot_ = INT;
	else typeReturnSlot_ = FLOAT;
	return {};
}

std::any Antlr2AstVisitor::visitConstDef(SysYParser::ConstDefContext* context)
{
	LOG(color::cyan("visitConstDef ") + context->ID()->toString());
	PUSH;
	vector<int> dims;
	for (const auto const_exp : context->constExp())
	{
		const_exp->accept(this);
		const auto n = dynamic_cast<ASTNumber*>(poll());
		ASSERT(!n->isFloat());
		ASSERT(n->toInteger() > 0);
		dims.emplace_back(n->toInteger());
		delete n;
	}
	Type* t = _declarationTypeConstraint.top();
	if (!dims.empty())
		t = arrayType(t, false, dims);
	auto decl = new ASTVarDecl(true, _structConstraint.size() == 1, t);
	decl->_id = context->ID()->toString();
	if (t->getTypeID() == TypeIDs::Array)
		t = dynamic_cast<ArrayType*>(t)->typeContained();
	if (t == INT)
		decl->_initList = new Tensor{dims, InitializeValue{0}};
	else decl->_initList = new Tensor{dims, InitializeValue{0.0f}};
	_initTensorConstraint.push(decl->_initList->getData());
	context->constInitVal()->accept(this);
	_initTensorConstraint.pop();
	returnSlot_.emplace(decl);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitConstInitVal(SysYParser::ConstInitValContext* context)
{
	LOG(color::cyan("visitConstInitVal"));
	PUSH;
	const auto t = _initTensorConstraint.top();
	const auto dft = t->tensorBelong()->defaultValue();
	if (context->constExp() != nullptr)
	{
		ASSERT(t->tensorBelong()->getShape().empty());
		context->constExp()->accept(this);
		const auto exp = dynamic_cast<ASTNumber*>(poll());
		if (const auto exp2 = dynamic_cast<ASTNumber*>(exp->castTypeTo(dft.getExpressionType())); !t->append(
			exp2->toInitializeValue()))
			throw runtime_error(
				"fail to append value to initialize list. context " + context->constExp()->toString());
		delete exp;
		POP;
		return {};
	}
	ASSERT(!t->tensorBelong()->getShape().empty());
	for (const auto& i : context->constArrayInitVal())
	{
		i->accept(this);
	}
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitConstArrayInitVal(SysYParser::ConstArrayInitValContext* ctx)
{
	LOG(color::cyan("visitConstArrayInitVal"));
	PUSH;
	const auto t = _initTensorConstraint.top();
	const auto dft = t->tensorBelong()->defaultValue();
	if (ctx->constExp() != nullptr)
	{
		ctx->constExp()->accept(this);
		const auto exp = dynamic_cast<ASTNumber*>(poll());
		if (const auto exp2 = dynamic_cast<ASTNumber*>(exp->castTypeTo(dft.getExpressionType(),
		                                                               TypeCastEnvironment::INITIALIZE_LIST)); !t->
			append(
				exp2->toInitializeValue()))
			throw runtime_error(
				"fail to append value to initialize list. context " + ctx->constExp()->toString());
		delete exp;
		POP;
		return {};
	}
	const auto tensor = t->makeSubTensor();
	ASSERT(tensor != nullptr);
	_initTensorConstraint.push(tensor);
	for (const auto& i : ctx->constArrayInitVal())
	{
		i->accept(this);
	}
	_initTensorConstraint.pop();
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitVarDecl(SysYParser::VarDeclContext* context)
{
	LOG(color::cyan("visitVarDecl"));
	PUSH;
	list<ASTVarDecl*> defList;
	const auto block = _structConstraint.back();
	context->bType()->accept(this);
	auto ty = typeReturnSlot_;
	_declarationTypeConstraint.push(ty);
	for (const auto& d : context->varDef())
	{
		d->accept(this);
		auto decl = dynamic_cast<ASTVarDecl*>(poll());
		defList.emplace_back(decl);
		if (!block->pushScope(decl))
			throw runtime_error("duplicate var declaration of id " + decl->id());
	}
	_declarationTypeConstraint.pop();
	POP;
	for (auto i : defList)returnSlot_.emplace(i);
	return {};
}

std::any Antlr2AstVisitor::visitVarDef(SysYParser::VarDefContext* context)
{
	LOG(color::cyan("visitVarDef ") + context->ID()->toString());
	PUSH;
	vector<int> dims;
	for (const auto const_exp : context->constExp())
	{
		const_exp->accept(this);
		const auto n = dynamic_cast<ASTNumber*>(poll());
		ASSERT(!n->isFloat());
		ASSERT(n->toInteger() > 0);
		dims.emplace_back(n->toInteger());
		delete n;
	}
	Type* t = _declarationTypeConstraint.top();
	if (!dims.empty())
		t = arrayType(t, false, dims);
	auto decl = new ASTVarDecl(false, _structConstraint.size() == 1, t);
	decl->_id = context->ID()->toString();
	if (t->getTypeID() == TypeIDs::Array)
		t = dynamic_cast<ArrayType*>(t)->typeContained();
	if (context->initVal() == nullptr)
	{
		if (t == INT)
			decl->_initList = new Tensor{dims, InitializeValue{0}};
		else decl->_initList = new Tensor{dims, InitializeValue{0.0f}};
		returnSlot_.emplace(decl);
		POP;
		return {};
	}
	if (t == INT)
		decl->_initList = new Tensor{dims, InitializeValue{0}};
	else decl->_initList = new Tensor{dims, InitializeValue{0.0f}};
	_initTensorConstraint.push(decl->_initList->getData());
	_initVarNodeConstraint.push(decl);
	context->initVal()->accept(this);
	_initVarNodeConstraint.pop();
	_initTensorConstraint.pop();
	returnSlot_.emplace(decl);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitInitVal(SysYParser::InitValContext* context)
{
	LOG(color::cyan("visitInitVal"));
	PUSH;
	const auto t = _initTensorConstraint.top();
	const auto dft = t->tensorBelong()->defaultValue();
	if (context->exp() != nullptr)
	{
		ASSERT(t->tensorBelong()->getShape().empty());
		context->exp()->accept(this);
		const auto exp = dynamic_cast<ASTExpression*>(poll());
		ASSERT(_structConstraint.size() != 1 || dynamic_cast<ASTNumber*>(exp) != nullptr);
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
			_initVarNodeConstraint.top()->_expressions.emplace_back(exp2);
		}
		POP;
		return {};
	}
	ASSERT(!t->tensorBelong()->getShape().empty());
	for (const auto& i : context->arrayInitVal())
	{
		i->accept(this);
	}
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitArrayInitVal(SysYParser::ArrayInitValContext* ctx)
{
	LOG(color::cyan("visitArrayInitVal"));
	PUSH;
	const auto t = _initTensorConstraint.top();
	const auto dft = t->tensorBelong()->defaultValue();
	if (ctx->exp() != nullptr)
	{
		ctx->exp()->accept(this);
		const auto exp = dynamic_cast<ASTExpression*>(poll());
		ASSERT(_structConstraint.size() != 1 || dynamic_cast<ASTNumber*>(exp) != nullptr);
		ASSERT(exp->getExpressionType() != FLOAT || dft.getExpressionType() != INT);
		const auto exp2 = exp->castTypeTo(dft.getExpressionType(),
		                                  TypeCastEnvironment::INITIALIZE_LIST);
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
			_initVarNodeConstraint.top()->_expressions.emplace_back(exp2);
		}
		POP;
		return {};
	}
	const auto tensor = t->makeSubTensor();
	ASSERT(tensor != nullptr);
	_initTensorConstraint.push(tensor);
	for (const auto& i : ctx->arrayInitVal())
	{
		i->accept(this);
	}
	_initTensorConstraint.pop();
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitFuncDef(SysYParser::FuncDefContext* context)
{
	LOG(color::cyan("visitFuncDef ") + context->ID()->toString());
	PUSH;
	context->funcType()->accept(this);
	const auto decl = new ASTFuncDecl(context->ID()->toString(), typeReturnSlot_);
	const auto top = dynamic_cast<ASTCompUnit*>(_structConstraint.front());
	top->_func_declarations.push_back(decl);
	_currentFunction = decl;
	if (!pushScope(decl))
		throw runtime_error("duplicate function " + decl->id());
	_structConstraint.emplace_back(decl);
	vector<ASTVarDecl*> paras;
	if (context->funcFParams() != nullptr)
	{
		context->funcFParams()->accept(this);
		while (!returnSlot_.empty())
		{
			paras.emplace_back(dynamic_cast<ASTVarDecl*>(poll()));
		}
	}
	for (auto& i : paras)
		decl->_args.emplace_back(i);
	_structConstraint.emplace_back(decl->_block);
	context->block()->accept(this);
	_structConstraint.pop_back();
	_structConstraint.pop_back();
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitFuncType(SysYParser::FuncTypeContext* context)
{
	LOG(color::green("visitFuncType ") + (context->INT() ? "INT" : (context->FLOAT() ? "FLOAT" : "VOID")));
	if (context->FLOAT()) typeReturnSlot_ = FLOAT;
	else if (context->INT()) typeReturnSlot_ = INT;
	else typeReturnSlot_ = VOID;
	return {};
}

std::any Antlr2AstVisitor::visitFuncFParams(SysYParser::FuncFParamsContext* context)
{
	LOG(color::cyan("visitFuncFParams"));
	PUSH;
	list<ASTVarDecl*> l;
	for (const auto& i : context->funcFParam())
	{
		i->accept(this);
		l.emplace_back(dynamic_cast<ASTVarDecl*>(poll()));
	}
	POP;
	for (auto i : l) returnSlot_.emplace(i);
	return {};
}

std::any Antlr2AstVisitor::visitFuncFParam(SysYParser::FuncFParamContext* context)
{
	LOG(color::cyan("visitFuncFParam ") + context->ID()->toString());
	PUSH;
	const auto id = context->ID()->toString();
	context->bType()->accept(this);
	const auto btype = typeReturnSlot_;
	vector<int> dims;
	for (const auto& i : context->exp())
	{
		i->accept(this);
		const auto exp = dynamic_cast<ASTExpression*>(poll());
		const auto num = dynamic_cast<ASTNumber*>(exp);
		ASSERT(num != nullptr);
		ASSERT(num->getExpressionType() == INT);
		int n = num->toInteger();
		ASSERT(n > 0);
		dims.emplace_back(n);
		delete num;
	}
	auto decl = new ASTVarDecl(false, false, btype);
	decl->_id = id;
	if (!context->LBRACK().empty())
		decl->_type = arrayType(btype, true, dims);
	if (!_structConstraint.back()->pushScope(decl))
		throw runtime_error("duplicate scope " + decl->id());
	POP;
	returnSlot_.emplace(decl);
	return {};
}

std::any Antlr2AstVisitor::visitBlock(SysYParser::BlockContext* context)
{
	LOG(color::yellow("visitBlock"));
	PUSH;
	for (const auto& i : context->blockItem())
	{
		i->accept(this);
	}
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitBlockItem(SysYParser::BlockItemContext* context)
{
	LOG(color::yellow("visitBlockItem"));
	PUSH;
	const auto block = dynamic_cast<ASTBlock*>(_structConstraint.back());
	if (context->decl() != nullptr)
	{
		context->decl()->accept(this);
	}
	else
	{
		context->stmt()->accept(this);
	}
	while (!returnSlot_.empty())
	{
		block->_stmts.emplace_back(poll());
	}
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitStmt(SysYParser::StmtContext* context)
{
	LOG(color::cyan("visitStmt"));
	PUSH;
	if (context->lVal() != nullptr)
	{
		context->lVal()->accept(this);
		ASTExpression* exp = dynamic_cast<ASTExpression*>(returnSlot_.front());
		returnSlot_.pop();
		const auto rv = dynamic_cast<ASTRVal*>(exp);
		ASSERT(rv != nullptr);
		ASSERT(!rv->getDeclaration()->isConst());
		const auto lv = rv->toLVal();
		context->exp()->accept(this);
		auto v = dynamic_cast<ASTExpression*>(poll());
		v = v->castTypeTo(rv->getExpressionType());
		rv->_index.clear();
		delete rv;
		const auto stmt = new ASTAssign(lv, v);
		POP;
		returnSlot_.emplace(stmt);
		return {};
	}
	if (context->block() != nullptr)
	{
		auto block = new ASTBlock();
		_structConstraint.push_back(block);
		context->block()->accept(this);
		_structConstraint.pop_back();
		if (block->isEmpty())
		{
			delete block;
			POP;
			return {};
		}
		POP;
		returnSlot_.emplace(block);
		return {};
	}
	if (context->IF() != nullptr)
	{
		context->cond()->accept(this);
		auto cond = dynamic_cast<ASTExpression*>(returnSlot_.front());
		returnSlot_.pop();
		context->stmt()[0]->accept(this);
		list<ASTStmt*> if_st;
		while (!returnSlot_.empty())
		{
			if_st.emplace_back(dynamic_cast<ASTStmt*>(returnSlot_.front()));
			returnSlot_.pop();
		}
		list<ASTStmt*> else_st;
		if (context->ELSE() != nullptr)
		{
			context->stmt()[1]->accept(this);
			while (!returnSlot_.empty())
			{
				else_st.emplace_back(dynamic_cast<ASTStmt*>(returnSlot_.front()));
				returnSlot_.pop();
			}
		}
		auto condNum = dynamic_cast<ASTNumber*>(cond);
		if (condNum != nullptr)
		{
			if (condNum->toBoolean())
			{
				delete cond;
				for (auto& i : else_st)delete i;
				POP;
				for (auto i : if_st) returnSlot_.emplace(i);
				return {};
			}
			for (auto& i : if_st) delete i;
			delete cond;
			POP;
			for (auto i : else_st) returnSlot_.emplace(i);
			return {};
		}
		auto condLogic = dynamic_cast<ASTLogicExp*>(cond);
		if (condLogic != nullptr)
		{
			if (condLogic->_have_result != ASTLogicExp::UNDEFINE)
			{
				if (condLogic->_have_result == ASTLogicExp::TRUE)
				{
					for (auto& i : else_st) delete i;
					if_st.push_front(cond);
					POP;
					for (auto i : if_st) returnSlot_.emplace(i);
					return {};
				}
				for (auto& i : if_st) delete i;
				else_st.push_front(cond);
				POP;
				for (auto i : else_st) returnSlot_.emplace(i);
				return {};
			}
		}
		if (if_st.empty())
		{
			if (else_st.empty())
			{
				auto l = AST::cutExpressionToOnlyLeaveFuncCall(cond);
				list<ASTStmt*> ret;
				for (auto& i : l) ret.emplace_back(i);
				POP;
				for (auto i : ret) returnSlot_.emplace(i);
				return {};
			}
			if (dynamic_cast<ASTNot*>(cond) != nullptr)
			{
				auto not_ = dynamic_cast<ASTNot*>(cond);
				cond = not_->_hold;
				cond = cond->castTypeTo(BOOL, TypeCastEnvironment::LOGIC_EXPRESSION);
				not_->_hold = nullptr;
				delete not_;
			}
			else
				cond = new ASTNot{cond};
			auto if_node = new ASTIf{cond};
			for (auto& i : else_st)
				if_node->_if_stmt.emplace_back(i);
			POP;
			returnSlot_.emplace(if_node);
			return {};
		}
		if (else_st.empty())
		{
			auto if_node = new ASTIf{cond};
			for (auto& i : if_st)
				if_node->_if_stmt.emplace_back(i);
			POP;
			returnSlot_.emplace(if_node);
			return {};
		}
		auto if_node = new ASTIf{cond};
		for (auto& i : if_st)
			if_node->_if_stmt.emplace_back(i);
		for (auto& i : else_st)
			if_node->_else_stmt.emplace_back(i);
		POP;
		returnSlot_.emplace(if_node);
		return {};
	}
	if (context->WHILE() != nullptr)
	{
		context->cond()->accept(this);
		auto cond = dynamic_cast<ASTExpression*>(returnSlot_.front());
		returnSlot_.pop();
		auto loop = new ASTWhile{cond};
		_whileConstraint.push(loop);
		context->stmt()[0]->accept(this);
		list<ASTStmt*> stmt;
		while (!returnSlot_.empty())
		{
			stmt.emplace_back(dynamic_cast<ASTStmt*>(returnSlot_.front()));
			returnSlot_.pop();
		}
		_whileConstraint.pop();
		for (auto& i : stmt) loop->_stmt.emplace_back(i);
		auto condNum = dynamic_cast<ASTNumber*>(cond);
		if (condNum != nullptr)
		{
			if (condNum->toBoolean() == false)
			{
				delete loop;
				POP;
				return {};
			}
		}
		auto condLogic = dynamic_cast<ASTLogicExp*>(cond);
		if (condLogic != nullptr)
		{
			if (condLogic->_have_result != ASTLogicExp::UNDEFINE)
			{
				if (condLogic->_have_result == ASTLogicExp::FALSE)
				{
					loop->_cond = nullptr;
					delete loop;
					returnSlot_.emplace(condLogic);
					POP;
					return {};
				}
			}
		}
		if (loop->_stmt.empty())
		{
			if (!cond->_haveFuncCall)
			{
				delete loop;
				POP;
				return {};
			}
			POP;
			returnSlot_.emplace(loop);
			return {};
		}
		context->cond()->accept(this);
		auto cond2 = dynamic_cast<ASTExpression*>(poll());
		auto ifNode = new ASTIf{cond2};
		ifNode->_if_stmt.emplace_back(loop);
		POP;
		returnSlot_.emplace(ifNode);
		return {};
	}
	if (context->BREAK() != nullptr)
	{
		auto get = _whileConstraint.top();
		returnSlot_.emplace(new ASTBreak{get});
		POP;
		return {};
	}
	if (context->CONTINUE() != nullptr)
	{
		auto get = _whileConstraint.top();
		returnSlot_.emplace(new ASTContinue{get});
		POP;
		return {};
	}
	if (context->RETURN() != nullptr)
	{
		if (context->exp() == nullptr)
		{
			ASSERT(_currentFunction->returnType() == VOID);
			returnSlot_.emplace(new ASTReturn{nullptr, _currentFunction});
			POP;
			return {};
		}
		ASSERT(_currentFunction->returnType() != VOID);
		context->exp()->accept(this);
		auto ret = dynamic_cast<ASTExpression*>(returnSlot_.front());
		returnSlot_.pop();
		ret = ret->castTypeTo(_currentFunction->returnType());
		returnSlot_.emplace(new ASTReturn{ret, _currentFunction});
		POP;
		return {};
	}
	if (context->exp() != nullptr)
	{
		context->exp()->accept(this);
		auto exp = dynamic_cast<ASTExpression*>(returnSlot_.front());
		returnSlot_.pop();
		auto ret = AST::cutExpressionToOnlyLeaveFuncCall(exp);
		for (auto i : ret) returnSlot_.emplace(i);
		POP;
		return {};
	}
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitExp(SysYParser::ExpContext* context)
{
	LOG(color::cyan("visitExp"));
	PUSH;
	context->addExp()->accept(this);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitCond(SysYParser::CondContext* context)
{
	LOG(color::cyan("visitCond"));
	PUSH;
	_allowLogic = true;
	context->lOrExp()->accept(this);
	auto cond = dynamic_cast<ASTExpression*>(returnSlot_.front());
	returnSlot_.pop();
	_allowLogic = false;
	cond = cond->castTypeTo(BOOL, TypeCastEnvironment::LOGIC_EXPRESSION);
	returnSlot_.emplace(cond);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitLVal(SysYParser::LValContext* context)
{
	LOG(color::cyan("visitLVal ") + context->ID()->toString());
	PUSH;
	const auto d = findScope(context->ID()->toString(), false);
	ASSERT(d != nullptr);
	const auto decl = dynamic_cast<ASTVarDecl*>(d);
	const auto indexCount = u2iNegThrow(context->exp().size());
	if (decl->getType()->isArrayType())
	{
		const auto ar = dynamic_cast<ArrayType*>(decl->getType());
		const int ori = u2iNegThrow(ar->dimensions().size()) +
		                (ar->getTypeID() == TypeIDs::ArrayInParameter ? 1 : 0);
		ASSERT(ori >= indexCount);
		vector<ASTExpression*> indexes;
		bool allConst = decl->isConst();
		int dimIdx = 0;
		for (const auto& i : context->exp())
		{
			i->accept(this);
			const auto idx = dynamic_cast<ASTExpression*>(returnSlot_.front());
			returnSlot_.pop();
			ASSERT(idx->getExpressionType() == INT);
			if (const auto num = dynamic_cast<ASTNumber*>(idx); num != nullptr)
			{
				int m = INT_MAX;
				if (ar->getTypeID() == TypeIDs::Array)
				{
					m = ar->dimensions()[dimIdx];
				}
				else if (dimIdx > 0)
					m = ar->dimensions()[dimIdx - 1];
				ASSERT(num->toInteger() >= 0 && num->toInteger() < m);
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
			returnSlot_.emplace(new ASTNumber(l->visitData()->getElement(constIndexes)));
			POP;
			return {};
		}
		auto ret = static_cast<ASTExpression*>(new ASTRVal(decl, indexes));
		for (auto& i : indexes)
			ret->_haveFuncCall = ret->_haveFuncCall || i->_haveFuncCall;
		returnSlot_.emplace(ret);
		POP;
		return {};
	}
	ASSERT(indexCount <= 0);
	if (decl->isConst())
	{
		returnSlot_.emplace(new ASTNumber{decl->_initList->getData()->getElement({})});
		POP;
		return {};
	}
	returnSlot_.emplace(new ASTRVal(decl, {}));
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitPrimaryExp(SysYParser::PrimaryExpContext* context)
{
	LOG(color::cyan("visitPrimaryExp"));
	PUSH;
	if (context->exp() != nullptr)
	{
		context->exp()->accept(this);
		POP;
		return {};
	}
	if (context->lVal() != nullptr)
	{
		context->lVal()->accept(this);
		POP;
		return {};
	}
	context->number()->accept(this);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitNumber(SysYParser::NumberContext* context)
{
	LOG(color::green("visitNumber ") + (context->IntConst() != nullptr ? context->IntConst()->toString() : context->
		FloatConst()->toString()));
	PUSH;
	if (context->IntConst() != nullptr)
	{
		int c;
		auto str = context->IntConst()->toString();
		if (str.size() >= 2)
		{
			if (str[0] == '0')
			{
				if (str[1] == 'x' || str[1] == 'X')
					c = std::stoi(str, nullptr, 16);
				else if (str[1] == 'b' || str[1] == 'B')
					c = std::stoi(str, nullptr, 2);
				else c = std::stoi(str, nullptr, 8);
			}
			else c = std::stoi(str);
		}
		else c = std::stoi(str);
		returnSlot_.emplace(new ASTNumber(c));
		POP;
		return {};
	}
	returnSlot_.emplace(new ASTNumber(stof(context->FloatConst()->toString())));
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitUnaryExp(SysYParser::UnaryExpContext* context)
{
	LOG(color::cyan("visitUnaryExp"));
	PUSH;
	if (context->primaryExp() != nullptr)
	{
		context->primaryExp()->accept(this);
		POP;
		return {};
	}
	if (context->ID() != nullptr)
	{
		bool special = false;
		auto id = context->ID()->toString();
		if (id == "starttime")
		{
			id = "_sysy_starttime";
			special = true;
		}
		else if (id == "stoptime")
		{
			id = "_sysy_stoptime";
			special = true;
		}
		const auto f = findScope(id, true);
		if (f == nullptr) throw runtime_error("Undeclared function " + id);
		const auto func = dynamic_cast<ASTFuncDecl*>(f);
		if (context->funcRParams()) context->funcRParams()->accept(this);
		vector<ASTExpression*> getArgs;
		while (!returnSlot_.empty())
		{
			getArgs.emplace_back(dynamic_cast<ASTExpression*>(returnSlot_.front()));
			returnSlot_.pop();
		}
		if (special)
		{
			ASSERT(getArgs.empty());
			const int line = u2iNegThrow(context->rParen()->getStart()->getLine());
			auto arg = new ASTNumber{line};
			getArgs.emplace_back(arg);
		}
		const auto typeArgs = func->args();
		ASSERT(typeArgs.size() == getArgs.size());
		const int size = u2iNegThrow(getArgs.size());
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
				ASSERT(getType->getTypeID() == TypeIDs::Array || getType->getTypeID() == TypeIDs::ArrayInParameter);
				const auto arT = dynamic_cast<ArrayType*>(type);
				const auto arGT = dynamic_cast<ArrayType*>(getType);
				ASSERT(arT->typeContained() == arGT->typeContained());
				// 库函数的输入参数并不满足维度要求
				if (func->isLibFunc()) continue;
				if (const auto tlv = dynamic_cast<ASTLVal*>(get); tlv != nullptr)
				{
					ASSERT(!tlv->getDeclaration()->isConst());
				}
				ASSERT(arGT->canPassToFuncOf(arT));
			}
		}
		const auto call = new ASTCall{func, getArgs};
		returnSlot_.emplace(call);
		POP;
		return {};
	}
	context->unaryExp()->accept(this);
	auto exp = dynamic_cast<ASTExpression*>(poll());
	if (context->ADD() != nullptr)
	{
		if (exp->getExpressionType() == BOOL)
			exp = exp->castTypeTo(INT, TypeCastEnvironment::LOGIC_EXPRESSION);
		returnSlot_.emplace(exp);
		POP;
		return {};
	}
	if (context->SUB() != nullptr)
	{
		if (exp->getExpressionType() == BOOL)
			exp = exp->castTypeTo(INT, TypeCastEnvironment::LOGIC_EXPRESSION);
		if (auto n = dynamic_cast<ASTNeg*>(exp); n != nullptr)
		{
			exp = n->_hold;
			n->_hold = nullptr;
			delete n;
			returnSlot_.emplace(exp);
			POP;
			return {};
		}
		if (const auto n = dynamic_cast<ASTNumber*>(exp); n != nullptr)
		{
			if (n->getExpressionType() == INT)
			{
				n->_field = ConstantValue{-n->_field.getIntConstant()};
				returnSlot_.emplace(exp);
				return {};
			}
			n->_field = ConstantValue{-n->_field.getFloatConstant()};
			returnSlot_.emplace(exp);
			POP;
			return {};
		}
		ASTNeg* n = new ASTNeg(exp);
		n->_haveFuncCall = exp->_haveFuncCall;
		returnSlot_.emplace(n);
		POP;
		return {};
	}
	if (!_allowLogic) throw runtime_error("math expression don't allow logic!");
	if (const auto logic = dynamic_cast<ASTNot*>(exp); logic != nullptr)
	{
		auto ret = logic->_hold;
		logic->_hold = nullptr;
		delete logic;
		returnSlot_.emplace(ret);
		POP;
		return {};
	}
	if (auto n = dynamic_cast<ASTNumber*>(exp); n != nullptr)
	{
		n = dynamic_cast<ASTNumber*>(n->castTypeTo(BOOL, TypeCastEnvironment::LOGIC_EXPRESSION));
		n->_field = ConstantValue{!n->_field.getBoolConstant()};
		returnSlot_.emplace(n);
		POP;
		return {};
	}
	exp = exp->castTypeTo(BOOL, TypeCastEnvironment::LOGIC_EXPRESSION);
	const auto n = new ASTNot(exp);
	n->_haveFuncCall = exp->_haveFuncCall;
	returnSlot_.emplace(n);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitRParen(SysYParser::RParenContext* context)
{
	return {};
}

std::any Antlr2AstVisitor::visitFuncRParams(SysYParser::FuncRParamsContext* context)
{
	LOG(color::cyan("visitFuncRParams"));
	PUSH;
	list<ASTNode*> buffer;
	for (const auto& i : context->exp())
	{
		i->accept(this);
		while (!returnSlot_.empty())
		{
			buffer.emplace_back(poll());
		}
	}
	POP;
	for (auto i : buffer) returnSlot_.emplace(i);
	return {};
}

std::any Antlr2AstVisitor::visitMulExp(SysYParser::MulExpContext* context)
{
	LOG(color::cyan("visitMulExp"));
	PUSH;
	if (context->mulExp() == nullptr)
	{
		context->unaryExp()->accept(this);
		POP;
		return {};
	}
	context->mulExp()->accept(this);
	auto l = dynamic_cast<ASTExpression*>(returnSlot_.front());
	returnSlot_.pop();
	context->unaryExp()->accept(this);
	auto r = dynamic_cast<ASTExpression*>(returnSlot_.front());
	returnSlot_.pop();
	const auto op = context->MUL() != nullptr ? MathOP::MUL : context->DIV() != nullptr ? MathOP::DIV : MathOP::MOD;
	auto tyT = ASTExpression::maxType(l, r, _allowLogic
		                                        ? TypeCastEnvironment::LOGIC_EXPRESSION
		                                        : TypeCastEnvironment::MATH_EXPRESSION);
	if (tyT == BOOL) tyT = INT;
	l = l->castTypeTo(tyT, _allowLogic ? TypeCastEnvironment::LOGIC_EXPRESSION : TypeCastEnvironment::MATH_EXPRESSION);
	r = r->castTypeTo(tyT, _allowLogic ? TypeCastEnvironment::LOGIC_EXPRESSION : TypeCastEnvironment::MATH_EXPRESSION);
	if (const auto rnum = dynamic_cast<ASTNumber*>(r); rnum != nullptr)
	{
		if (op != MathOP::MUL)
		{
			const auto ty = r->getExpressionType();
			ASSERT(ty != FLOAT || rnum->toFloat() != 0.0f);
			ASSERT(ty != INT || rnum->toInteger() != 0);
		}
	}
	ASSERT(op != MathOP::MOD || tyT != FLOAT);
	const auto lnum = dynamic_cast<ASTNumber*>(l);
	const auto rnum = dynamic_cast<ASTNumber*>(r);
	if (lnum != nullptr && rnum != nullptr)
	{
		if (tyT == INT)
		{
			const int a = lnum->forceToInteger();
			const int b = rnum->forceToInteger();
			delete rnum;
			if (op == MathOP::MUL)
			{
				lnum->_field = ConstantValue{a * b};
				returnSlot_.emplace(lnum);
				POP;
				return {};
			}
			if (op == MathOP::DIV)
			{
				lnum->_field = ConstantValue{a / b};
				returnSlot_.emplace(lnum);
				POP;
				return {};
			}
			lnum->_field = ConstantValue{a % b};
			returnSlot_.emplace(lnum);
			POP;
			return {};
		}
		const float a = lnum->forceToFloat();
		const float b = rnum->forceToFloat();
		delete rnum;
		if (op == MathOP::MUL)
		{
			lnum->_field = ConstantValue{a * b};
			returnSlot_.emplace(lnum);
			POP;
			return {};
		}
		lnum->_field = ConstantValue{a / b};
		returnSlot_.emplace(lnum);
		POP;
		return {};
	}
	auto ln = dynamic_cast<ASTNeg*>(l);
	if (auto rn = dynamic_cast<ASTNeg*>(r); ln != nullptr && rn != nullptr)
	{
		l = ln->_hold;
		ln->_hold = nullptr;
		delete ln;
		r = rn->_hold;
		rn->_hold = nullptr;
		delete rn;
	}
	else if (ln != nullptr || rn != nullptr)
	{
		auto n = ln != nullptr ? ln : rn;
		if (ln != nullptr)l = n->_hold;
		else r = n->_hold;
		const auto node = new ASTMathExp(tyT, op, l, r);
		node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
		n->_hold = node;
		n->_haveFuncCall = node->_haveFuncCall;
		returnSlot_.emplace(n);
		POP;
		return {};
	}
	const auto node = new ASTMathExp(tyT, op, l, r);
	node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
	returnSlot_.emplace(node);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitAddExp(SysYParser::AddExpContext* context)
{
	LOG(color::cyan("visitAddExp"));
	PUSH;
	if (context->addExp() == nullptr)
	{
		context->mulExp()->accept(this);
		POP;
		return {};
	}
	context->addExp()->accept(this);
	auto l = dynamic_cast<ASTExpression*>(returnSlot_.front());
	returnSlot_.pop();
	context->mulExp()->accept(this);
	auto r = dynamic_cast<ASTExpression*>(returnSlot_.front());
	returnSlot_.pop();
	const auto op = context->ADD() == nullptr ? MathOP::SUB : MathOP::ADD;
	auto tyT = ASTExpression::maxType(l, r, _allowLogic
		                                        ? TypeCastEnvironment::LOGIC_EXPRESSION
		                                        : TypeCastEnvironment::MATH_EXPRESSION);
	if (tyT == BOOL) tyT = INT;
	l = l->castTypeTo(tyT, _allowLogic ? TypeCastEnvironment::LOGIC_EXPRESSION : TypeCastEnvironment::MATH_EXPRESSION);
	r = r->castTypeTo(tyT, _allowLogic ? TypeCastEnvironment::LOGIC_EXPRESSION : TypeCastEnvironment::MATH_EXPRESSION);
	const auto lm = dynamic_cast<ASTNumber*>(l);
	const auto rm = dynamic_cast<ASTNumber*>(r);
	if (lm != nullptr && rm != nullptr)
	{
		if (tyT == INT)
		{
			int a = lm->forceToInteger();
			int b = rm->forceToInteger();
			delete rm;
			if (op == MathOP::ADD)
				lm->_field = ConstantValue{a + b};
			else lm->_field = ConstantValue{a - b};
			returnSlot_.emplace(lm);
			POP;
			return {};
		}
		float a = lm->forceToFloat();
		float b = rm->forceToFloat();
		delete rm;
		if (op == MathOP::ADD) lm->_field = ConstantValue{a + b};
		else lm->_field = ConstantValue{a - b};
		returnSlot_.emplace(lm);
		POP;
		return {};
	}
	auto ln = dynamic_cast<ASTNeg*>(l);
	auto rn = dynamic_cast<ASTNeg*>(r);
	const bool nl = ln != nullptr;
	const bool nr = rn != nullptr;
	if (nl)
	{
		if (nr)
		{
			if (op == MathOP::ADD)
			{
				l = ln->_hold;
				ln->_hold = nullptr;
				delete ln;
				r = rn->_hold;
				l = l->castTypeTo(tyT);
				r = r->castTypeTo(tyT);
				const auto node = new ASTMathExp(tyT, op, l, r);
				node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
				rn->_hold = node;
				rn->_haveFuncCall = node->_haveFuncCall;
				returnSlot_.emplace(rn);
				POP;
				return {};
			}
			l = ln->_hold;
			ln->_hold = nullptr;
			delete ln;
			r = rn->_hold;
			rn->_hold = nullptr;
			delete rn;
			l = l->castTypeTo(tyT);
			r = r->castTypeTo(tyT);
			const auto node = new ASTMathExp(tyT, op, r, l);
			node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
			returnSlot_.emplace(node);
			POP;
			return {};
		}
		if (op == MathOP::ADD)
		{
			l = ln->_hold;
			ln->_hold = nullptr;
			delete ln;
			l = l->castTypeTo(tyT);
			r = r->castTypeTo(tyT);
			const auto node = new ASTMathExp(tyT, MathOP::SUB, r, l);
			node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
			returnSlot_.emplace(node);
			POP;
			return {};
		}
		l = ln->_hold;
		l = l->castTypeTo(tyT);
		r = r->castTypeTo(tyT);
		const auto node = new ASTMathExp(tyT, MathOP::ADD, l, r);
		node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
		ln->_haveFuncCall = node->_haveFuncCall;
		ln->_hold = node;
		returnSlot_.emplace(ln);
		POP;
		return {};
	}
	if (nr)
	{
		if (op == MathOP::ADD)
		{
			r = rn->_hold;
			rn->_hold = nullptr;
			delete rn;
			l = l->castTypeTo(tyT);
			r = r->castTypeTo(tyT);
			const auto node = new ASTMathExp(tyT, MathOP::SUB, l, r);
			node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
			returnSlot_.emplace(node);
			POP;
			return {};
		}
		r = rn->_hold;
		rn->_hold = nullptr;
		delete rn;
		l = l->castTypeTo(tyT);
		r = r->castTypeTo(tyT);
		const auto node = new ASTMathExp(tyT, MathOP::ADD, l, r);
		node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
		returnSlot_.emplace(node);
		POP;
		return {};
	}
	const auto node = new ASTMathExp(tyT, op, l, r);
	node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
	returnSlot_.emplace(node);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitRelExp(SysYParser::RelExpContext* context)
{
	LOG(color::cyan("visitRelExp"));
	PUSH;
	if (context->relExp() == nullptr)
	{
		context->addExp()->accept(this);
		POP;
		return {};
	}
	context->relExp()->accept(this);
	auto l = dynamic_cast<ASTExpression*>(returnSlot_.front());
	returnSlot_.pop();
	context->addExp()->accept(this);
	auto r = dynamic_cast<ASTExpression*>(returnSlot_.front());
	returnSlot_.pop();
	auto tyT = ASTExpression::maxType(l, r, TypeCastEnvironment::LOGIC_EXPRESSION);
	if (tyT == BOOL) tyT = INT;
	l = l->castTypeTo(tyT, TypeCastEnvironment::LOGIC_EXPRESSION);
	r = r->castTypeTo(tyT, TypeCastEnvironment::LOGIC_EXPRESSION);
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
		lNum->_field = ConstantValue{res};
		returnSlot_.emplace(lNum);
		POP;
		return {};
	}
	RelationOP op;
	if (context->LT() != nullptr) op = RelationOP::LT;
	else if (context->GT() != nullptr) op = RelationOP::GT;
	else if (context->LE() != nullptr) op = RelationOP::LE;
	else op = RelationOP::GE;
	auto node = new ASTRelation{op, l, r};
	node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
	returnSlot_.emplace(node);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitEqExp(SysYParser::EqExpContext* context)
{
	LOG(color::cyan("visitEqExp"));
	PUSH;
	if (context->eqExp() == nullptr)
	{
		context->relExp()->accept(this);
		POP;
		return {};
	}
	context->eqExp()->accept(this);
	auto l = dynamic_cast<ASTExpression*>(returnSlot_.front());
	returnSlot_.pop();
	context->relExp()->accept(this);
	auto r = dynamic_cast<ASTExpression*>(returnSlot_.front());
	returnSlot_.pop();
	auto tyT = ASTExpression::maxType(l, r, TypeCastEnvironment::LOGIC_EXPRESSION);
	if (tyT == BOOL) tyT = INT;
	l = l->castTypeTo(tyT, TypeCastEnvironment::LOGIC_EXPRESSION);
	r = r->castTypeTo(tyT, TypeCastEnvironment::LOGIC_EXPRESSION);
	auto lNum = dynamic_cast<ASTNumber*>(l);
	auto rNum = dynamic_cast<ASTNumber*>(r);
	if (lNum != nullptr && rNum != nullptr)
	{
		bool res;
		if (context->EQ() != nullptr)
		{
			if (lNum->isInteger())
				res = lNum->toInteger() == rNum->toInteger();
			else if (lNum->isFloat()) res = lNum->toFloat() == rNum->toFloat();
			else res = lNum->toBoolean() == rNum->toBoolean();
		}
		else
		{
			if (lNum->isInteger())
				res = lNum->toInteger() != rNum->toInteger();
			else if (lNum->isFloat()) res = lNum->toFloat() != rNum->toFloat();
			else res = lNum->toBoolean() != rNum->toBoolean();
		}
		delete rNum;
		lNum->_field = ConstantValue{res};
		returnSlot_.emplace(lNum);
		POP;
		return {};
	}
	auto node = new ASTEqual{context->EQ() != nullptr, l, r};
	node->_haveFuncCall = l->_haveFuncCall || r->_haveFuncCall;
	returnSlot_.emplace(node);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitLAndExp(SysYParser::LAndExpContext* context)
{
	LOG(color::cyan("visitLAndExp"));
	PUSH;
	if (context->eqExp().size() == 1)
	{
		context->eqExp()[0]->accept(this);
		POP;
		return {};
	}
	bool cut = false;
	bool comp = false;
	vector<ASTExpression*> exps;
	for (auto& i : context->eqExp())
	{
		i->accept(this);
		auto exp = dynamic_cast<ASTExpression*>(returnSlot_.front());
		returnSlot_.pop();
		ASSERT(exp != nullptr);
		exp = exp->castTypeTo(BOOL, TypeCastEnvironment::LOGIC_EXPRESSION);
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
			if (exp->_haveFuncCall) comp = true;
		}
		else delete exp;
	}
	if (!comp)
	{
		if (cut)
		{
			for (auto& i : exps) delete i;
			returnSlot_.emplace(new ASTNumber(false));
			POP;
			return {};
		}
		if (exps.empty())
		{
			returnSlot_.emplace(new ASTNumber(true));
			POP;
			return {};
		}
		if (exps.size() == 1)
		{
			returnSlot_.emplace(exps[0]);
			POP;
			return {};
		}
		auto ret = new ASTLogicExp{LogicOP::AND, exps, ASTLogicExp::UNDEFINE};
		returnSlot_.emplace(ret);
		POP;
		return {};
	}
	if (cut)
	{
		vector<ASTExpression*> e;
		for (auto& i : exps)
		{
			if (i->_haveFuncCall)e.push_back(i);
			else delete i;
		}
		exps.emplace_back(new ASTNumber{false});
		auto ret = new ASTLogicExp{LogicOP::AND, exps, ASTLogicExp::FALSE};
		ret->_haveFuncCall = true;
		returnSlot_.emplace(ret);
		POP;
		return {};
	}
	auto ret = new ASTLogicExp{LogicOP::AND, exps, ASTLogicExp::UNDEFINE};
	ret->_haveFuncCall = true;
	returnSlot_.emplace(ret);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitLOrExp(SysYParser::LOrExpContext* context)
{
	LOG(color::cyan("visitLOrExp"));
	PUSH;
	if (context->lAndExp().size() == 1)
	{
		context->lAndExp()[0]->accept(this);
		POP;
		return {};
	}
	bool cut = false;
	bool comp = false;
	bool undef = false;
	vector<ASTExpression*> exps;
	for (auto& i : context->lAndExp())
	{
		i->accept(this);
		auto exp = dynamic_cast<ASTExpression*>(returnSlot_.front());
		ASSERT(exp != nullptr);
		returnSlot_.pop();
		exp = exp->castTypeTo(BOOL, TypeCastEnvironment::LOGIC_EXPRESSION);
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
				if (log->_have_result == ASTLogicExp::TRUE)
				{
					cut = true;
					comp = true;
				}
				else if (log->_have_result == ASTLogicExp::UNDEFINE)
					undef = true;
			}
			else
				undef = true;
			exps.emplace_back(exp);
			if (exp->_haveFuncCall) comp = true;
		}
		else delete exp;
	}
	if (!comp)
	{
		if (cut)
		{
			for (auto& i : exps) delete i;
			POP;
			returnSlot_.emplace(new ASTNumber{true});
			return {};
		}
		if (exps.empty())
		{
			POP;
			returnSlot_.emplace(new ASTNumber{false});
			return {};
		}
		if (exps.size() == 1)
		{
			POP;
			returnSlot_.emplace(exps[0]);
			return {};
		}
		auto ret = new ASTLogicExp{LogicOP::OR, exps, ASTLogicExp::UNDEFINE};
		POP;
		returnSlot_.emplace(ret);
		return {};
	}
	if (cut)
	{
		vector<ASTExpression*> e;
		for (auto& i : exps)
		{
			if (i->_haveFuncCall) e.push_back(i);
			else delete i;
		}
		if (auto log = dynamic_cast<ASTLogicExp*>(exps.back()); log != nullptr)
		{
			if (log->_have_result == ASTLogicExp::TRUE)
			{
				if (exps.size() == 1)
				{
					returnSlot_.emplace(exps[0]);
					POP;
					return {};
				}
				auto ret = new ASTLogicExp{LogicOP::OR, exps, ASTLogicExp::TRUE};
				ret->_haveFuncCall = true;
				returnSlot_.emplace(ret);
				POP;
				return {};
			}
		}
		exps.emplace_back(new ASTNumber{true});
		auto ret = new ASTLogicExp{LogicOP::OR, exps, ASTLogicExp::TRUE};
		ret->_haveFuncCall = true;
		returnSlot_.emplace(ret);
		POP;
		return {};
	}
	auto ret = new ASTLogicExp{LogicOP::OR, exps, undef ? ASTLogicExp::UNDEFINE : ASTLogicExp::FALSE};
	ret->_haveFuncCall = true;
	returnSlot_.emplace(ret);
	POP;
	return {};
}

std::any Antlr2AstVisitor::visitConstExp(SysYParser::ConstExpContext* context)
{
	LOG(color::cyan("visitConstExp"));
	PUSH;
	context->addExp()->accept(this);
	ASSERT(dynamic_cast<ASTNumber*>(returnSlot_.front()) != nullptr);
	POP;
	return {};
}

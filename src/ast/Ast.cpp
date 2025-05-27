#include <charconv>
#include <iomanip>

#include "../../include/ast/ast.hpp"

using namespace Types;
using namespace std;

namespace
{
	bool isSmallEnd()
	{
		int a = 1;
		char b = *reinterpret_cast<char*>(&a);
		if (b == 0) return false;
		return true;
	}

	bool SMALL_END = isSmallEnd();
	int LOGICAL_LEFT_END_8 = SMALL_END ? 0 : 7;
	int LOGICAL_LEFT_END_4 = SMALL_END ? 0 : 3;
	int LOGICAL_LEFT_END_2 = SMALL_END ? 0 : 1;
	int LOGICAL_RIGHT_END_8 = SMALL_END ? 7 : 0;
	int LOGICAL_RIGHTT_END_4 = SMALL_END ? 3 : 0;
	int LOGICAL_RIGHT_END_2 = SMALL_END ? 1 : 0;
}


bool TopScopeTableInAST::pushScope(ASTDecl* decl)
{
	if (auto f = dynamic_cast<ASTFuncDecl*>(decl); f != nullptr)
	{
		if (func_scopes.find(f->id()) != func_scopes.end())return false;
		func_scopes.emplace(decl->id(), f);
	}
	else
	{
		auto v = dynamic_cast<ASTVarDecl*>(decl);
		if (var_scopes.find(v->id()) != var_scopes.end())return false;
		var_scopes.emplace(decl->id(), v);
	}
	return true;
}

ASTDecl* TopScopeTableInAST::findScope(const std::string& id, const bool isFunc)
{
	if (isFunc)
	{
		if (const auto idx = func_scopes.find(id); idx != func_scopes.end()) return idx->second;
		return nullptr;
	}
	if (const auto idx = var_scopes.find(id); idx != var_scopes.end()) return idx->second;
	return nullptr;
}

bool BlockScopeTableInAST::pushScope(ASTDecl* decl)
{
	if (const auto f = dynamic_cast<ASTFuncDecl*>(decl); f != nullptr)return false;
	auto v = dynamic_cast<ASTVarDecl*>(decl);
	if (var_scopes.find(v->id()) != var_scopes.end())return false;
	var_scopes.emplace(decl->id(), v);
	return true;
}

ASTDecl* BlockScopeTableInAST::findScope(const std::string& id, const bool isFunc)
{
	if (isFunc) return nullptr;
	if (const auto idx = var_scopes.find(id); idx != var_scopes.end()) return idx->second;
	return nullptr;
}

bool InitializeValue::isExpression() const
{
	return field_.segment_[LOGICAL_LEFT_END_8] == static_cast<char>(0x00);
}

bool InitializeValue::isConstant() const
{
	return field_.segment_[LOGICAL_LEFT_END_8] != static_cast<char>(0x00);
}

bool InitializeValue::isIntConstant() const
{
	return field_.segment_[LOGICAL_LEFT_END_8] == static_cast<char>(0x10);
}

bool InitializeValue::isBoolConstant() const
{
	return field_.segment_[LOGICAL_LEFT_END_8] == static_cast<char>(0x11);
}

bool InitializeValue::isFloatConstant() const
{
	return field_.segment_[LOGICAL_LEFT_END_8] == static_cast<char>(0x01);
}

ASTExpression* InitializeValue::getExpression() const
{
	if (!isExpression()) throw std::runtime_error("InitializeValue Not Expression");
	return field_.expression_;
}

int InitializeValue::getIntConstant() const
{
	if (!isIntConstant()) throw std::runtime_error("InitializeValue Not Int Constant");
	return field_.int_value_[LOGICAL_RIGHT_END_2];
}

bool InitializeValue::getBoolConstant() const
{
	if (!isBoolConstant()) throw std::runtime_error("InitializeValue Not Bool Constant");
	return field_.bool_value_[LOGICAL_RIGHT_END_8];
}

float InitializeValue::getFloatConstant() const
{
	if (!isFloatConstant()) throw std::runtime_error("InitializeValue Not Float Constant");
	return field_.float_value_[LOGICAL_RIGHT_END_2];
}

InitializeValue::InitializeValue()
{
	field_.expression_ = nullptr;
}

InitializeValue::InitializeValue(ASTExpression* expression)
{
	field_.expression_ = expression;
}

InitializeValue::InitializeValue(const int intConstant)
{
	field_.expression_ = nullptr;
	field_.int_value_[LOGICAL_RIGHT_END_2] = intConstant;
	field_.segment_[LOGICAL_LEFT_END_8] = static_cast<char>(0x10);
}

InitializeValue::InitializeValue(const float floatConstant)
{
	field_.expression_ = nullptr;
	field_.float_value_[LOGICAL_RIGHT_END_2] = floatConstant;
	field_.segment_[LOGICAL_LEFT_END_8] = static_cast<char>(0x01);
}

InitializeValue::InitializeValue(const bool boolConstant)
{
	field_.expression_ = nullptr;
	field_.bool_value_[LOGICAL_RIGHT_END_8] = boolConstant;
	field_.segment_[LOGICAL_LEFT_END_8] = static_cast<char>(0x11);
}

Type* InitializeValue::getExpressionType() const
{
	if (isExpression()) return field_.expression_->getExpressionType();
	if (isIntConstant()) return INT;
	if (isFloatConstant()) return FLOAT;
	return BOOL;
}

std::string InitializeValue::toString() const
{
	if (isExpression()) return field_.expression_->toString();
	if (isIntConstant()) return to_string(field_.int_value_[LOGICAL_RIGHT_END_2]);
	if (isFloatConstant())
	{
		std::ostringstream oss;
		oss << std::scientific << std::setprecision(8) << field_.float_value_[LOGICAL_RIGHT_END_2];
		return oss.str() + 'f';
	}
	return field_.bool_value_[LOGICAL_RIGHT_END_8] ? "true" : "false";
}

std::string ASTNode::toString()
{
	string str;
	for (auto& i : toStringList())
	{
		str += i + '\n';
	}
	if (!str.empty()) str.pop_back();
	return str;
}

ScopeTableInAST* ASTNode::getScopeTable()
{
	return nullptr;
}

bool ASTNode::pushScope(ASTDecl* decl)
{
	auto node = this;
	while (node->getScopeTable() == nullptr)
	{
		node = node->parent_;
		if (node == nullptr)
			throw runtime_error(
				"AST tree should have scope table in root, in that case, this point can not be reach.");
	}
	return node->getScopeTable()->pushScope(decl);
}

ASTDecl* ASTNode::findScope(const std::string& id, const bool isFunc)
{
	auto node = this;
	while (node != nullptr)
	{
		while (node->getScopeTable() == nullptr)
			node = node->parent_;
		if (node == nullptr)
			throw runtime_error(
				"AST tree should have scope table in root, in that case, this point can not be reach.");
		if (const auto fd = node->getScopeTable()->findScope(id, isFunc); fd != nullptr) return fd;
		node = node->parent_;
	}
	return nullptr;
}

void ASTNode::setParent(ASTNode* parent)
{
	parent_ = parent;
}

std::list<std::string> ASTNode::toStringList()
{
	return {};
}

ASTNode* ASTNode::getParent() const
{
	return parent_;
}

ASTNode* ASTNode::findParent(const std::function<bool(const ASTNode*)>& func)
{
	auto node = this;
	while (node != nullptr)
	{
		if (func(node)) return node;
		node = node->parent_;
	}
	return nullptr;
}

ASTCompUnit::~ASTCompUnit()
{
	for (auto& i : var_declarations_) delete i;
	for (auto& i : func_declarations_) delete i;
	for (auto& i : lib_) delete i;
	delete scopeTable_;
}

ASTCompUnit::ASTCompUnit(): scopeTable_(new TopScopeTableInAST{})
{
	ASTFuncDecl* decls[] = {
		new ASTFuncDecl("getint", INT, {}),
		new ASTFuncDecl("getch", INT, {}),
		new ASTFuncDecl("getarray", INT, {arrayType(INT, true, {})}),
		new ASTFuncDecl("getfarray", INT, {arrayType(FLOAT, true, {})}),
		new ASTFuncDecl("putint", VOID, {INT}),
		new ASTFuncDecl("putch", VOID, {INT}),
		new ASTFuncDecl("putarray", VOID, {INT, arrayType(INT, true, {})}),
		new ASTFuncDecl("putfloat", VOID, {FLOAT}),
		new ASTFuncDecl("putfarray", VOID, {INT, arrayType(FLOAT, true, {})})
	};
	// 加载库函数
	for (auto& decl : decls)
	{
		decl->parent_ = this;
		decl->in_lib_ = true;
		lib_.emplace_back(decl);
		scopeTable_->pushScope(decl);
	}
}

std::list<std::string> ASTCompUnit::toStringList()
{
	list<string> ret;
	for (auto& i : var_declarations_)
	{
		ret.emplace_back(i->toString() + ';');
	}
	for (auto& i : func_declarations_)
	{
		for (auto& j : i->toStringList())
		{
			ret.emplace_back(j);
		}
	}
	return ret;
}

ScopeTableInAST* ASTCompUnit::getScopeTable()
{
	return scopeTable_;
}

const std::string& ASTDecl::id()
{
	return id_;
}

std::list<std::string> ASTVarDecl::toStringList()
{
	string ret;
	if (isConst()) ret += "const ";
	auto t = type_;
	if (type_->isArrayType())
	{
		t = dynamic_cast<ArrayType*>(type_)->typeContained();
	}
	ret += t->toString();
	ret += ' ';
	ret += id();
	if (type_->isArrayType())
	{
		ret += dynamic_cast<ArrayType*>(type_)->suffixToString();
	}
	if (initList_ != nullptr)
	{
		ret += " = ";
		ret += initList_->toString([](const InitializeValue& v)-> std::string
		{
			return v.toString();
		});
	}
	return {ret};
}

ASTVarDecl::~ASTVarDecl()
{
	delete initList_;
	for (auto& i : expressions_)delete i;
}

bool ASTVarDecl::isConst() const
{
	return is_const_;
}

bool ASTVarDecl::isGlobal() const
{
	return is_global_;
}

const Tensor<InitializeValue>* ASTVarDecl::getInitList() const
{
	return initList_;
}

Type* ASTVarDecl::getType() const
{
	return type_;
}

std::list<std::string> ASTFuncDecl::toStringList()
{
	list<string> l;
	string str;
	str += return_type_->toString();
	str += " ";
	str += id();
	str += '(';
	for (auto& i : args_)
	{
		str += i->toString();
		str += ", ";
	}
	if (str.back() == ' ')
	{
		str.pop_back();
		str.pop_back();
	}
	str += ')';
	l.emplace_back(str);
	auto b = block_->toStringList();
	for (auto& i : b) l.emplace_back(i);
	return l;
}

ASTFuncDecl::~ASTFuncDecl()
{
	for (auto& i : args_) delete i;
	delete block_;
}

ASTFuncDecl::ASTFuncDecl(const std::string& id, Type* return_type)
	: return_type_(return_type), block_(new ASTBlock())
{
	block_->parent_ = this;
	id_ = id;
}

ASTFuncDecl::ASTFuncDecl(const std::string& id, Type* return_type, const std::vector<Type*>& innerArgs)
	: return_type_(return_type), block_(new ASTBlock())
{
	block_->parent_ = this;
	id_ = id;
	const string idTemplate = id_ + "Parameter";
	int idx = 0;
	for (auto& arg : innerArgs)
	{
		const auto d = new ASTVarDecl(false, false, arg);
		d->parent_ = this;
		d->id_ = idTemplate + to_string(idx);
		block_->pushScope(d);
		args_.emplace_back(d);
		idx++;
	}
}

Type* ASTFuncDecl::returnType() const
{
	return return_type_;
}

const std::vector<ASTVarDecl*>& ASTFuncDecl::args() const
{
	return args_;
}

bool ASTFuncDecl::isLibFunc() const
{
	return in_lib_;
}

Type* ASTExpression::maxType(const ASTExpression* a, const ASTExpression* b)
{
	if (a->getExpressionType() == INT)
	{
		if (b->getExpressionType() == INT) return INT;
		if (b->getExpressionType() == FLOAT)return FLOAT;
		throw runtime_error("can not resolve maxType of " + b->getExpressionType()->toString());
	}
	if (a->getExpressionType() == FLOAT)
	{
		if (b->getExpressionType() == INT || b->getExpressionType() == FLOAT)return FLOAT;
		throw runtime_error("can not resolve maxType of " + b->getExpressionType()->toString());
	}
	throw runtime_error("can not resolve maxType of " + a->getExpressionType()->toString());
}

ASTExpression* ASTExpression::castTypeTo(Type* type)
{
	if (type == getExpressionType()) return this;
	if (type != FLOAT && type != INT)
		throw runtime_error(
			"Can not case expression type from " + getExpressionType()->toString() + " to " + type->toString());
	const auto c = new ASTCast(this, type);
	this->parent_ = c;
	c->haveFuncCall_ = this->haveFuncCall_;
	return c;
}

std::list<std::string> ASTLVal::toStringList()
{
	string str;
	str += decl_->id();
	if (!index_.empty())
	{
		for (auto& i : index_)
		{
			str += '[';
			str += i->toString();
			str += ']';
		}
	}
	return {str};
}

std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input)
{
	std::list<ASTExpression*> ret;
	if (!input->haveFuncCall_)
	{
		delete input;
		return ret;
	}
	{
		auto toASTCast = dynamic_cast<ASTCast*>(input);
		if (toASTCast != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTCast->source_))
			{
				ret.emplace_back(i);
			}
			toASTCast->source_ = nullptr;
			delete toASTCast;
			return ret;
		}
	}
	{
		auto toASTMathExp = dynamic_cast<ASTMathExp*>(input);
		if (toASTMathExp != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTMathExp->l_))
			{
				ret.emplace_back(i);
			}
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTMathExp->r_))
			{
				ret.emplace_back(i);
			}
			toASTMathExp->l_ = nullptr;
			toASTMathExp->r_ = nullptr;
			delete toASTMathExp;
			return ret;
		}
	}
	{
		auto toASTMathExp = dynamic_cast<ASTMathExp*>(input);
		if (toASTMathExp != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTMathExp->l_))
			{
				ret.emplace_back(i);
			}
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTMathExp->r_))
			{
				ret.emplace_back(i);
			}
			toASTMathExp->l_ = nullptr;
			toASTMathExp->r_ = nullptr;
			delete toASTMathExp;
			return ret;
		}
	}
	{
		auto toASTCall = dynamic_cast<ASTCall*>(input);
		if (toASTCall != nullptr)
		{
			ret.emplace_back(toASTCall);
			return ret;
		}
	}
	{
		auto toASTNeg = dynamic_cast<ASTNeg*>(input);
		if (toASTNeg != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTNeg->hold_))
			{
				ret.emplace_back(i);
			}
			toASTNeg->hold_ = nullptr;
			delete toASTNeg;
			return ret;
		}
	}
	{
		auto toASTLVal = dynamic_cast<ASTLVal*>(input);
		if (toASTLVal != nullptr)
		{
			for (auto& j : toASTLVal->index_)
				for (auto& i : cutExpressionToOnlyLeaveFuncCall(j))
				{
					ret.emplace_back(i);
				}
			toASTLVal->index_.clear();
			delete toASTLVal;
			return ret;
		}
	}
	{
		auto toASTMath2Logic = dynamic_cast<ASTMath2Logic*>(input);
		if (toASTMath2Logic != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTMath2Logic->contained_))
			{
				ret.emplace_back(i);
			}
			toASTMath2Logic->contained_ = nullptr;
			delete toASTMath2Logic;
			return ret;
		}
	}
	{
		auto toASTLogicExp = dynamic_cast<ASTLogicExp*>(input);
		if (toASTLogicExp != nullptr)
		{
			ret.emplace_back(toASTLogicExp);
			return ret;
		}
	}
	{
		auto toASTEqual = dynamic_cast<ASTEqual*>(input);
		if (toASTEqual != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTEqual->l_))
			{
				ret.emplace_back(i);
			}
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTEqual->r_))
			{
				ret.emplace_back(i);
			}
			toASTEqual->l_ = nullptr;
			toASTEqual->r_ = nullptr;
			delete toASTEqual;
			return ret;
		}
	}
	{
		auto toASTRelation = dynamic_cast<ASTRelation*>(input);
		if (toASTRelation != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTRelation->l_))
			{
				ret.emplace_back(i);
			}
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTRelation->r_))
			{
				ret.emplace_back(i);
			}
			toASTRelation->l_ = nullptr;
			toASTRelation->r_ = nullptr;
			delete toASTRelation;
			return ret;
		}
	}
	ret.emplace_back(input);
	return ret;
}

ASTLVal::~ASTLVal()
{
	for (auto& i : index_) delete i;
}

ASTLVal::ASTLVal(ASTVarDecl* target, const std::vector<ASTExpression*>& index)
	: decl_(target),
	  index_(index)
{
}

Type* ASTLVal::getExpressionType() const
{
	const auto ty = decl_->getType();
	if (ty->isArrayType())
	{
		const auto ar = dynamic_cast<ArrayType*>(ty);
		return ar->getSubType(static_cast<int>(index_.size()));
	}
	return ty;
}

ASTVarDecl* ASTLVal::getDeclaration() const
{
	return decl_;
}

ASTExpression* ASTLVal::findChild(const std::function<bool(const ASTExpression*)> func)
{
	if (func(this) == true) return this;
	for (const auto& i : index_)
	{
		if (const auto ret = i->findChild(func); ret != nullptr) return ret;
	}
	return nullptr;
}

std::list<std::string> ASTNumber::toStringList()
{
	if (type_ == INT) return {to_string(field_.i_value_)};
	if (type_ == FLOAT)
	{
		std::ostringstream oss;
		oss << std::scientific << std::setprecision(8) << field_.f_value_;
		return {oss.str() + 'f'};
	}
	return {field_.b_value_ ? "true" : "false"};
}

ASTNeg::ASTNeg(ASTExpression* hold)
	: hold_(hold)
{
}

Type* ASTNeg::getExpressionType() const
{
	return hold_->getExpressionType();
}

ASTExpression* ASTNeg::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this) == true) return this;
	return hold_->findChild(func);
}

std::list<std::string> ASTMath2Logic::toStringList()
{
	string ret;
	if (not_) ret += "!";
	else ret += "(bool)";
	ret += contained_->toString();
	return {ret};
}

ASTMath2Logic::~ASTMath2Logic()
{
	delete contained_;
}


ASTMath2Logic::ASTMath2Logic(ASTExpression* contained)
	: contained_(contained), not_(true)
{
}

Type* ASTMath2Logic::getExpressionType() const
{
	return BOOL;
}

ASTExpression* ASTMath2Logic::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	return contained_->findChild(func);
}

std::list<std::string> ASTBlock::toStringList()
{
	list<string> ret;
	ret.emplace_back("{");
	for (auto& i : stmts_)
	{
		if (dynamic_cast<ASTBlock*>(i) != nullptr || dynamic_cast<ASTIf*>(i) != nullptr || dynamic_cast<ASTWhile*>(i) !=
		    nullptr)
		{
			auto l = i->toStringList();
			for (auto& j : l)
			{
				auto s = "    " + j;
				ret.emplace_back(s);
			}
		}
		else
		{
			ret.emplace_back("    " + i->toString() + ';');
		}
	}
	ret.emplace_back("}");
	return ret;
}

ASTBlock::~ASTBlock()
{
	delete scope_table_;
	for (auto& i : stmts_) delete i;
}

ScopeTableInAST* ASTBlock::getScopeTable()
{
	return scope_table_;
}

bool ASTBlock::isEmpty() const
{
	return stmts_.empty();
}

std::list<std::string> ASTAssign::toStringList()
{
	string ret;
	ret += assign_to_->toString();
	ret += " = ";
	ret += assign_value_->toString();
	return {ret};
}

ASTAssign::~ASTAssign()
{
	delete assign_to_;
	delete assign_value_;
}

ASTAssign::ASTAssign(ASTLVal* assign_to, ASTExpression* assign_value)
	: assign_to_(assign_to),
	  assign_value_(assign_value)
{
}

std::list<std::string> ASTIf::toStringList()
{
	list<string> ret;
	string str = "if(";
	str += cond_->toString();
	str += ')';
	ret.emplace_back(str);
	if (if_stmt_.size() == 1 && dynamic_cast<ASTBlock*>(if_stmt_[0]) != nullptr)
	{
		for (auto& i : if_stmt_[0]->toStringList())
		{
			ret.emplace_back(i);
		}
	}
	else
	{
		ret.emplace_back("{");
		for (auto& i : if_stmt_)
		{
			if (dynamic_cast<ASTBlock*>(i) != nullptr || dynamic_cast<ASTIf*>(i) != nullptr || dynamic_cast<ASTWhile*>(
				    i) !=
			    nullptr)
			{
				auto l = i->toStringList();
				for (auto& j : l)
				{
					auto s = "    " + j;
					ret.emplace_back(s);
				}
			}
			else
			{
				ret.emplace_back("    " + i->toString() + ';');
			}
		}
		ret.emplace_back("}");
	}
	if (else_stmt_.empty()) return ret;
	ret.emplace_back("else");
	if (else_stmt_.size() == 1 && dynamic_cast<ASTBlock*>(else_stmt_[0]) != nullptr)
	{
		for (auto& i : else_stmt_[0]->toStringList())
		{
			ret.emplace_back(i);
		}
	}
	else
	{
		ret.emplace_back("{");
		for (auto& i : else_stmt_)
		{
			if (dynamic_cast<ASTBlock*>(i) != nullptr || dynamic_cast<ASTIf*>(i) != nullptr || dynamic_cast<ASTWhile*>(
				    i) !=
			    nullptr)
			{
				auto l = i->toStringList();
				for (auto& j : l)
				{
					auto s = "    " + j;
					ret.emplace_back(s);
				}
			}
			else
			{
				ret.emplace_back("    " + i->toString() + ';');
			}
		}
		ret.emplace_back("}");
	}
	return ret;
}

ASTIf::~ASTIf()
{
	delete cond_;
	for (auto& i : if_stmt_) delete i;
	for (auto& i : else_stmt_) delete i;
}

std::list<std::string> ASTWhile::toStringList()
{
	string str = "while(";
	str += cond_->toString();
	str += ')';
	if (stmt_.empty())
	{
		str += ';';
		return {str};
	}
	list<string> ret;
	ret.emplace_back(str);
	if (stmt_.size() == 1 && dynamic_cast<ASTBlock*>(stmt_[0]) != nullptr)
	{
		for (auto& i : stmt_[0]->toStringList())
		{
			ret.emplace_back(i);
		}
	}
	else
	{
		ret.emplace_back("{");
		for (auto& i : stmt_)
		{
			if (dynamic_cast<ASTBlock*>(i) != nullptr || dynamic_cast<ASTIf*>(i) != nullptr || dynamic_cast<ASTWhile*>(
				    i) !=
			    nullptr)
			{
				auto l = i->toStringList();
				for (auto& j : l)
				{
					auto s = "    " + j;
					ret.emplace_back(s);
				}
			}
			else
			{
				ret.emplace_back("    " + i->toString() + ';');
			}
		}
		ret.emplace_back("}");
	}
	return ret;
}

ASTWhile::~ASTWhile()
{
	delete cond_;
	for (auto& i : stmt_) delete i;
}

std::list<std::string> ASTBreak::toStringList()
{
	return {"break"};
}

std::list<std::string> ASTReturn::toStringList()
{
	string ret = "return";
	if (return_value_ != nullptr)
	{
		ret += ' ';
		ret += return_value_->toString();
	}
	return {ret};
}

ASTReturn::~ASTReturn()
{
	delete return_value_;
}

ASTNumber::ASTNumber(const int i)
{
	type_ = INT;
	field_.i_value_ = i;
}

ASTNumber::ASTNumber(const float i)
{
	type_ = FLOAT;
	field_.f_value_ = i;
}

ASTNumber::ASTNumber(const bool i)
{
	type_ = BOOL;
	field_.b_value_ = i;
}

ASTNumber::ASTNumber(InitializeValue v)
{
	if (v.isIntConstant())
	{
		type_ = INT;
		field_.i_value_ = v.getIntConstant();
	}
	else if (v.isFloatConstant())
	{
		type_ = FLOAT;
		field_.f_value_ = v.getFloatConstant();
	}
	else if (v.isBoolConstant())
	{
		type_ = BOOL;
		field_.b_value_ = v.getBoolConstant();
	}
	else throw runtime_error("can not initialize a number with not constant initialize value.");
}


bool ASTNumber::isFloat() const
{
	return type_ == FLOAT;
}

bool ASTNumber::isInteger() const
{
	return type_ == INT;
}

bool ASTNumber::isBoolean() const
{
	return type_ == BOOL;
}

int ASTNumber::toInteger() const
{
	if (type_ != INT)
		throw
			runtime_error(getExpressionType()->toString() + " Number do not have int value");
	return field_.i_value_;
}

float ASTNumber::toFloat() const
{
	if (type_ != FLOAT)
		throw runtime_error(
			getExpressionType()->toString() + " Number do not have float value");
	return field_.f_value_;
}

bool ASTNumber::toBoolean() const
{
	if (type_ != BOOL)
		throw runtime_error(
			getExpressionType()->toString() + " Number do not have bool value");
	return field_.b_value_;
}

int ASTNumber::forceToInteger() const
{
	if (type_ == INT) return field_.i_value_;
	if (type_ == FLOAT)
		return static_cast<int>(field_.f_value_);
	throw runtime_error(
		getExpressionType()->toString() + " Number do not have int value");
}

float ASTNumber::forceToFloat() const
{
	if (type_ == FLOAT)
		return field_.f_value_;
	if (type_ == INT)
		return static_cast<float>(field_.i_value_);
	throw runtime_error(
		getExpressionType()->toString() + " Number do not have float value");
}

ASTExpression* ASTNumber::castTypeTo(Type* type)
{
	if (type_ == FLOAT)
	{
		if (type == INT)
		{
			type_ = INT;
			field_.i_value_ = static_cast<int>(field_.f_value_);
		}
		else if (type == BOOL)
		{
			type_ = BOOL;
			field_.b_value_ = field_.f_value_ != 0.0f;
		}
		else if (type != FLOAT)
			throw runtime_error(
				"Can not case expression type from " + type_->toString() + " to " + type->toString());
	}
	else if (type_ == INT)
	{
		if (type == FLOAT)
		{
			type_ = FLOAT;
			field_.f_value_ = static_cast<float>(field_.i_value_);
		}
		else if (type == BOOL)
		{
			type_ = BOOL;
			field_.b_value_ = field_.i_value_ != 0;
		}
		else if (type != INT)
			throw runtime_error(
				"Can not case expression type from " + type_->toString() + " to " + type->toString());
	}
	else if (type != BOOL)
		throw runtime_error(
			"Can not case expression type from " + type_->toString() + " to " + type->toString());
	return this;
}

ASTNumber* ASTNumber::toArrayInitValue(const Type* t)
{
	if (type_ == INT)
	{
		if (t == FLOAT)
		{
			type_ = FLOAT;
			field_.f_value_ = static_cast<float>(field_.i_value_);
		}
		else if (t != INT)
			throw runtime_error(
				"Can not case array init expression type from " + type_->toString() + " to " + t->toString());
	}
	else if (type_ == FLOAT && t == FLOAT) return this;
	else
		throw runtime_error(
			"Can not case array init expression type from " + type_->toString() + " to " + t->toString());
	return this;
}

Type* ASTNumber::getExpressionType() const
{
	return type_;
}

InitializeValue ASTNumber::toInitializeValue() const
{
	if (type_ == INT) return InitializeValue{field_.i_value_};
	if (type_ == FLOAT) return InitializeValue{field_.f_value_};
	return InitializeValue{field_.b_value_};
}

ASTExpression* ASTNumber::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this))return this;
	return nullptr;
}

std::list<std::string> ASTCast::toStringList()
{
	string str;
	str += '(';
	str += castTo_->toString();
	str += ')';
	str += source_->toString();
	return {str};
}

ASTCast::~ASTCast()
{
	delete source_;
}

ASTCast::ASTCast(ASTExpression* source, Type* cast_to)
	: source_(source),
	  castTo_(cast_to)
{
}

Type* ASTCast::getExpressionType() const
{
	return castTo_;
}

ASTExpression* ASTCast::castTypeTo(Type* type)
{
	throw runtime_error("recast");
}

ASTExpression* ASTCast::findChild(const std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	if (const auto ret = source_->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTMathExp::toStringList()
{
	string ret = "(";
	ret += l_->toString();
	ret += ' ';
	if (op_ == MathOP::ADD) ret += '+';
	else if (op_ == MathOP::SUB) ret += '-';
	else if (op_ == MathOP::MUL) ret += '*';
	else if (op_ == MathOP::DIV) ret += '/';
	else ret += '%';
	ret += ' ';
	ret += r_->toString();
	ret += ')';
	return {ret};
}

ASTMathExp::~ASTMathExp()
{
	delete l_;
	delete r_;
}

ASTMathExp::ASTMathExp(Type* result_type, const MathOP op, ASTExpression* l, ASTExpression* r)
	: result_type_(result_type),
	  op_(op),
	  l_(l),
	  r_(r)
{
}

Type* ASTMathExp::getExpressionType() const
{
	return result_type_;
}

ASTExpression* ASTMathExp::findChild(const std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	if (const auto ret = l_->findChild(func); ret != nullptr) return ret;
	if (const auto ret = r_->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTLogicExp::toStringList()
{
	string ret = "(";
	for (auto& i : exps_)
	{
		ret += i->toString();
		ret += ' ';
		if (op_ == LogicOP::OR) ret += "||";
		else ret += "&&";
		ret += ' ';
	}
	if (ret.back() == ' ')
	{
		ret.pop_back();
		ret.pop_back();
		ret.pop_back();
		ret.pop_back();
	}
	ret += ')';
	return {ret};
}

ASTLogicExp::~ASTLogicExp()
{
	for (auto& i : exps_) delete i;
}

Type* ASTLogicExp::getExpressionType() const
{
	return BOOL;
}

ASTExpression* ASTLogicExp::findChild(const std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	for (auto& i : exps_)
		if (const auto ret = i->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTEqual::toStringList()
{
	string ret = "(";
	ret += l_->toString();
	ret += ' ';
	if (op_equal_) ret += "==";
	else ret += "!=";
	ret += ' ';
	ret += r_->toString();
	ret += ')';
	return {ret};
}

ASTEqual::~ASTEqual()
{
	delete l_;
	delete r_;
}

Type* ASTEqual::getExpressionType() const
{
	return BOOL;
}

ASTExpression* ASTEqual::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	if (const auto ret = l_->findChild(func); ret != nullptr) return ret;
	if (const auto ret = r_->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTRelation::toStringList()
{
	string ret = "(";
	ret += l_->toString();
	ret += ' ';
	if (op_ == RelationOP::LT) ret += '<';
	else if (op_ == RelationOP::GT) ret += '>';
	else if (op_ == RelationOP::LE) ret += "<=";
	else ret += ">=";
	ret += ' ';
	ret += r_->toString();
	ret += ')';
	return {ret};
}

ASTRelation::~ASTRelation()
{
	delete l_;
	delete r_;
}

Type* ASTRelation::getExpressionType() const
{
	return BOOL;
}

ASTExpression* ASTRelation::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	if (const auto ret = l_->findChild(func); ret != nullptr) return ret;
	if (const auto ret = r_->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTCall::toStringList()
{
	string ret = function_->id();
	ret += '(';
	for (auto& i : parameters_)
	{
		ret += i->toString();
		ret += ", ";
	}
	if (ret.back() == ' ')
	{
		ret.pop_back();
		ret.pop_back();
	}
	ret += ')';
	return {ret};
}

ASTCall::~ASTCall()
{
	for (auto& i : parameters_) delete i;
}

ASTCall::ASTCall(ASTFuncDecl* function, const std::vector<ASTExpression*>& parameters)
	: function_(function),
	  parameters_(parameters)
{
	haveFuncCall_ = true;
}

Type* ASTCall::getExpressionType() const
{
	return function_->returnType();
}

ASTExpression* ASTCall::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	for (auto& i : parameters_)
		if (const auto ret = i->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTNeg::toStringList()
{
	string ret = "-";
	ret += hold_->toString();
	return {ret};
}

ASTNeg::~ASTNeg()
{
	delete hold_;
}

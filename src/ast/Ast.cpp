#include <charconv>
#include <iomanip>

#include "../../include/ast/Ast.hpp"

using namespace Types;
using namespace std;

namespace error
{
	namespace
	{
		// 无法恰当转换类型
		std::runtime_error ASTTypeCast(const Type* from, const Type* to, TypeCastEnvironment environment)
		{
			return std::runtime_error{
				"ASTTypeCast Error: Can not cast type from " + from->toString() + " to " + to->toString() +
				" in context " +
				to_string(environment)
			};
		}


		// 无法恰当统一类型
		std::runtime_error ASTTypeUnify(const Type* ta, const Type* tb, TypeCastEnvironment environment)
		{
			return std::runtime_error{
				"ASTTypeUnify Error: Can not unify type " + ta->toString() + " and " + tb->toString() + " in context " +
				to_string(environment)
			};
		}
	}
}

namespace
{
	bool isSmallEnd()
	{
		int a = 1;
		char b = *reinterpret_cast<char*>(&a);
		if (b == 0) return false;
		return true;
	}

	void checkForTypeCast(const Type* from, const Type* to, const TypeCastEnvironment& environment)
	{
		if (!from->isBasicValueType() || !to->isBasicValueType())
			throw error::ASTTypeCast(from, to, environment);
		switch (environment)
		{
			case TypeCastEnvironment::INITIALIZE_LIST:
				if (from == BOOL || to == BOOL || (from == FLOAT && to == INT))
					throw error::ASTTypeCast(from, to, environment);
				break;
			case TypeCastEnvironment::MATH_EXPRESSION:
				if (from == BOOL || to == BOOL)
					throw error::ASTTypeCast(from, to, environment);
				break;
			case TypeCastEnvironment::LOGIC_EXPRESSION:
				break;
		}
	}


	bool SMALL_END = isSmallEnd();
	int LOGICAL_LEFT_END_8 = SMALL_END ? 7 : 0;
	int LOGICAL_RIGHT_END_8 = SMALL_END ? 0 : 7;
	int LOGICAL_RIGHT_END_2 = SMALL_END ? 0 : 1;
}

std::string to_string(const TypeCastEnvironment e)
{
	switch (e)
	{
		case TypeCastEnvironment::INITIALIZE_LIST:
			return "initialize list";
		case TypeCastEnvironment::MATH_EXPRESSION:
			return "math expression";
		case TypeCastEnvironment::LOGIC_EXPRESSION:
			return "logic expression";
	}
	return "";
}

bool TopScopeTableInAST::pushScope(ASTDecl* decl)
{
	if (auto f = dynamic_cast<ASTFuncDecl*>(decl); f != nullptr)
	{
		if (_func_scopes.find(f->id()) != _func_scopes.end())return false;
		_func_scopes.emplace(decl->id(), f);
	}
	else
	{
		auto v = dynamic_cast<ASTVarDecl*>(decl);
		if (_var_scopes.find(v->id()) != _var_scopes.end())return false;
		_var_scopes.emplace(decl->id(), v);
	}
	return true;
}

ASTDecl* TopScopeTableInAST::findScope(const std::string& id, const bool isFunc)
{
	if (isFunc)
	{
		if (const auto idx = _func_scopes.find(id); idx != _func_scopes.end()) return idx->second;
		return nullptr;
	}
	if (const auto idx = _var_scopes.find(id); idx != _var_scopes.end()) return idx->second;
	return nullptr;
}

bool BlockScopeTableInAST::pushScope(ASTDecl* decl)
{
	if (const auto f = dynamic_cast<ASTFuncDecl*>(decl); f != nullptr)return false;
	auto v = dynamic_cast<ASTVarDecl*>(decl);
	if (_var_scopes.find(v->id()) != _var_scopes.end())return false;
	_var_scopes.emplace(decl->id(), v);
	return true;
}

ASTDecl* BlockScopeTableInAST::findScope(const std::string& id, const bool isFunc)
{
	if (isFunc) return nullptr;
	if (const auto idx = _var_scopes.find(id); idx != _var_scopes.end()) return idx->second;
	return nullptr;
}

bool InitializeValue::isExpression() const
{
	return _field.segment_[LOGICAL_LEFT_END_8] == static_cast<char>(0x00);
}

bool InitializeValue::isConstant() const
{
	return _field.segment_[LOGICAL_LEFT_END_8] != static_cast<char>(0x00);
}

bool InitializeValue::isIntConstant() const
{
	return _field.segment_[LOGICAL_LEFT_END_8] == static_cast<char>(0x10);
}

bool InitializeValue::isBoolConstant() const
{
	return _field.segment_[LOGICAL_LEFT_END_8] == static_cast<char>(0x11);
}

bool InitializeValue::isFloatConstant() const
{
	return _field.segment_[LOGICAL_LEFT_END_8] == static_cast<char>(0x01);
}

ASTExpression* InitializeValue::getExpression() const
{
	if (!isExpression()) throw std::runtime_error("InitializeValue Not Expression");
	return _field.expression_;
}

int InitializeValue::getIntConstant() const
{
	if (!isIntConstant()) throw std::runtime_error("InitializeValue Not Int Constant");
	return _field.int_value_[LOGICAL_RIGHT_END_2];
}

bool InitializeValue::getBoolConstant() const
{
	if (!isBoolConstant()) throw std::runtime_error("InitializeValue Not Bool Constant");
	return _field.bool_value_[LOGICAL_RIGHT_END_8];
}

float InitializeValue::getFloatConstant() const
{
	if (!isFloatConstant()) throw std::runtime_error("InitializeValue Not Float Constant");
	return _field.float_value_[LOGICAL_RIGHT_END_2];
}

InitializeValue::InitializeValue()
{
	_field.expression_ = nullptr;
}

InitializeValue::InitializeValue(ASTExpression* expression)
{
	_field.expression_ = expression;
}

InitializeValue::InitializeValue(const int intConstant)
{
	_field.expression_ = nullptr;
	_field.int_value_[LOGICAL_RIGHT_END_2] = intConstant;
	_field.segment_[LOGICAL_LEFT_END_8] = static_cast<char>(0x10);
}

InitializeValue::InitializeValue(const float floatConstant)
{
	_field.expression_ = nullptr;
	_field.float_value_[LOGICAL_RIGHT_END_2] = floatConstant;
	_field.segment_[LOGICAL_LEFT_END_8] = static_cast<char>(0x01);
}

InitializeValue::InitializeValue(const bool boolConstant)
{
	_field.expression_ = nullptr;
	_field.bool_value_[LOGICAL_RIGHT_END_8] = boolConstant;
	_field.segment_[LOGICAL_LEFT_END_8] = static_cast<char>(0x11);
}

Type* InitializeValue::getExpressionType() const
{
	if (isExpression()) return _field.expression_->getExpressionType();
	if (isIntConstant()) return INT;
	if (isFloatConstant()) return FLOAT;
	return BOOL;
}

std::string InitializeValue::toString() const
{
	if (isExpression()) return _field.expression_->toString();
	if (isIntConstant()) return to_string(_field.int_value_[LOGICAL_RIGHT_END_2]);
	if (isFloatConstant())
	{
		std::ostringstream oss;
		oss << std::scientific << std::setprecision(8) << _field.float_value_[LOGICAL_RIGHT_END_2];
		return oss.str() + 'f';
	}
	return _field.bool_value_[LOGICAL_RIGHT_END_8] ? "true" : "false";
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
		node = node->_parent;
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
			node = node->_parent;
		if (node == nullptr)
			throw runtime_error(
				"AST tree should have scope table in root, in that case, this point can not be reach.");
		if (const auto fd = node->getScopeTable()->findScope(id, isFunc); fd != nullptr) return fd;
		node = node->_parent;
	}
	return nullptr;
}

void ASTNode::setParent(ASTNode* parent)
{
	_parent = parent;
}

std::list<std::string> ASTNode::toStringList()
{
	return {};
}

ASTNode* ASTNode::getParent() const
{
	return _parent;
}

ASTNode* ASTNode::findParent(const std::function<bool(const ASTNode*)>& func)
{
	auto node = this;
	while (node != nullptr)
	{
		if (func(node)) return node;
		node = node->_parent;
	}
	return nullptr;
}

ASTCompUnit::~ASTCompUnit()
{
	for (auto& i : _var_declarations) delete i;
	for (auto& i : _func_declarations) delete i;
	for (auto& i : _lib) delete i;
	delete _scopeTable;
}

ASTCompUnit::ASTCompUnit(): _scopeTable(new TopScopeTableInAST{})
{
	ASTFuncDecl* decls[] = {
		new ASTFuncDecl("starttime", VOID, {}),
		new ASTFuncDecl("stoptime", VOID, {}),
		new ASTFuncDecl("getint", INT, {}),
		new ASTFuncDecl("getfloat", FLOAT, {}),
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
		decl->_parent = this;
		decl->_in_lib = true;
		_lib.emplace_back(decl);
		_scopeTable->pushScope(decl);
	}
}

std::list<std::string> ASTCompUnit::toStringList()
{
	list<string> ret;
	for (auto& i : _var_declarations)
	{
		ret.emplace_back(i->toString() + ';');
	}
	for (auto& i : _func_declarations)
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
	return _scopeTable;
}

const std::string& ASTDecl::id()
{
	return _id;
}

std::list<std::string> ASTVarDecl::toStringList()
{
	string ret;
	if (isConst()) ret += "const ";
	auto t = _type;
	if (_type->isArrayType())
	{
		t = dynamic_cast<ArrayType*>(_type)->typeContained();
	}
	ret += t->toString();
	ret += ' ';
	ret += id();
	if (_type->isArrayType())
	{
		ret += dynamic_cast<ArrayType*>(_type)->suffixToString();
	}
	if (_initList != nullptr)
	{
		ret += " = ";
		ret += _initList->toString([](const InitializeValue& v)-> std::string
		{
			return v.toString();
		});
	}
	return {ret};
}

ASTVarDecl::~ASTVarDecl()
{
	delete _initList;
	for (auto& i : _expressions)delete i;
}

bool ASTVarDecl::isConst() const
{
	return _is_const;
}

bool ASTVarDecl::isGlobal() const
{
	return _is_global;
}

const Tensor<InitializeValue>* ASTVarDecl::getInitList() const
{
	return _initList;
}

Type* ASTVarDecl::getType() const
{
	return _type;
}

std::list<std::string> ASTFuncDecl::toStringList()
{
	list<string> l;
	string str;
	str += _return_type->toString();
	str += " ";
	str += id();
	str += '(';
	for (auto& i : _args)
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
	auto b = _block->toStringList();
	for (auto& i : b) l.emplace_back(i);
	return l;
}

ASTFuncDecl::~ASTFuncDecl()
{
	for (auto& i : _args) delete i;
	delete _block;
}

ASTFuncDecl::ASTFuncDecl(const std::string& id, Type* return_type)
	: _return_type(return_type), _block(new ASTBlock())
{
	_block->_parent = this;
	_id = id;
}

ASTFuncDecl::ASTFuncDecl(const std::string& id, Type* return_type, const std::vector<Type*>& innerArgs)
	: _return_type(return_type), _block(new ASTBlock())
{
	_block->_parent = this;
	_id = id;
	const string idTemplate = _id + "Parameter";
	int idx = 0;
	for (auto& arg : innerArgs)
	{
		const auto d = new ASTVarDecl(false, false, arg);
		d->_parent = this;
		d->_id = idTemplate + to_string(idx);
		_block->pushScope(d);
		_args.emplace_back(d);
		idx++;
	}
}

Type* ASTFuncDecl::returnType() const
{
	return _return_type;
}

const std::vector<ASTVarDecl*>& ASTFuncDecl::args() const
{
	return _args;
}

bool ASTFuncDecl::isLibFunc() const
{
	return _in_lib;
}

Type* ASTExpression::maxType(const ASTExpression* a, const ASTExpression* b, const
                             TypeCastEnvironment environment)
{
	auto ta = a->getExpressionType();
	auto tb = b->getExpressionType();
	if (!ta->isBasicValueType() || !tb->isBasicValueType())
		throw error::ASTTypeUnify(ta, tb, environment);
	if (environment != TypeCastEnvironment::LOGIC_EXPRESSION)
	{
		if (ta == BOOL && tb == BOOL) return BOOL;
		if (ta == BOOL || tb == BOOL)
			throw error::ASTTypeUnify(ta, tb, environment);
	}
	auto ty = ta;
	if (tb == FLOAT) ty = FLOAT;
	else if (tb == INT && ty != FLOAT) ty = INT;
	return ty;
}

ASTExpression* ASTExpression::castTypeTo(
	Type* type, const TypeCastEnvironment environment)
{
	if (type == getExpressionType()) return this;
	auto from = getExpressionType();
	checkForTypeCast(from, type, environment);
	const auto c = new ASTCast(this, type);
	this->_parent = c;
	c->_haveFuncCall = this->_haveFuncCall;
	return c;
}

std::list<std::string> ASTLVal::toStringList()
{
	string str;
	str += _decl->id();
	if (!_index.empty())
	{
		for (auto& i : _index)
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
	if (!input->_haveFuncCall)
	{
		delete input;
		return ret;
	}
	{
		auto toASTCast = dynamic_cast<ASTCast*>(input);
		if (toASTCast != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTCast->_source))
			{
				ret.emplace_back(i);
			}
			toASTCast->_source = nullptr;
			delete toASTCast;
			return ret;
		}
	}
	{
		auto toASTMathExp = dynamic_cast<ASTMathExp*>(input);
		if (toASTMathExp != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTMathExp->_l))
			{
				ret.emplace_back(i);
			}
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTMathExp->_r))
			{
				ret.emplace_back(i);
			}
			toASTMathExp->_l = nullptr;
			toASTMathExp->_r = nullptr;
			delete toASTMathExp;
			return ret;
		}
	}
	{
		auto toASTMathExp = dynamic_cast<ASTMathExp*>(input);
		if (toASTMathExp != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTMathExp->_l))
			{
				ret.emplace_back(i);
			}
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTMathExp->_r))
			{
				ret.emplace_back(i);
			}
			toASTMathExp->_l = nullptr;
			toASTMathExp->_r = nullptr;
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
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTNeg->_hold))
			{
				ret.emplace_back(i);
			}
			toASTNeg->_hold = nullptr;
			delete toASTNeg;
			return ret;
		}
	}
	{
		auto toASTLVal = dynamic_cast<ASTLVal*>(input);
		if (toASTLVal != nullptr)
		{
			for (auto& j : toASTLVal->_index)
				for (auto& i : cutExpressionToOnlyLeaveFuncCall(j))
				{
					ret.emplace_back(i);
				}
			toASTLVal->_index.clear();
			delete toASTLVal;
			return ret;
		}
	}
	{
		auto toASTMath2Logic = dynamic_cast<ASTNot*>(input);
		if (toASTMath2Logic != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTMath2Logic->_hold))
			{
				ret.emplace_back(i);
			}
			toASTMath2Logic->_hold = nullptr;
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
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTEqual->_l))
			{
				ret.emplace_back(i);
			}
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTEqual->_r))
			{
				ret.emplace_back(i);
			}
			toASTEqual->_l = nullptr;
			toASTEqual->_r = nullptr;
			delete toASTEqual;
			return ret;
		}
	}
	{
		auto toASTRelation = dynamic_cast<ASTRelation*>(input);
		if (toASTRelation != nullptr)
		{
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTRelation->_l))
			{
				ret.emplace_back(i);
			}
			for (auto& i : cutExpressionToOnlyLeaveFuncCall(toASTRelation->_r))
			{
				ret.emplace_back(i);
			}
			toASTRelation->_l = nullptr;
			toASTRelation->_r = nullptr;
			delete toASTRelation;
			return ret;
		}
	}
	ret.emplace_back(input);
	return ret;
}

ASTLVal::~ASTLVal()
{
	for (auto& i : _index) delete i;
}

ASTLVal::ASTLVal(ASTVarDecl* target, const std::vector<ASTExpression*>& index)
	: _decl(target),
	  _index(index)
{
}

Type* ASTLVal::getExpressionType() const
{
	const auto ty = _decl->getType();
	if (ty->isArrayType())
	{
		const auto ar = dynamic_cast<ArrayType*>(ty);
		return ar->getSubType(static_cast<int>(_index.size()));
	}
	return ty;
}

ASTVarDecl* ASTLVal::getDeclaration() const
{
	return _decl;
}

ASTExpression* ASTLVal::findChild(const std::function<bool(const ASTExpression*)> func)
{
	if (func(this) == true) return this;
	for (const auto& i : _index)
	{
		if (const auto ret = i->findChild(func); ret != nullptr) return ret;
	}
	return nullptr;
}

std::list<std::string> ASTNumber::toStringList()
{
	if (_type == INT) return {to_string(_field._i_value)};
	if (_type == FLOAT)
	{
		std::ostringstream oss;
		oss << std::scientific << std::setprecision(8) << _field._f_value;
		return {oss.str() + 'f'};
	}
	return {_field._b_value ? "true" : "false"};
}

ASTNeg::ASTNeg(ASTExpression* hold)
	: _hold(hold)
{
}

Type* ASTNeg::getExpressionType() const
{
	return _hold->getExpressionType();
}

ASTExpression* ASTNeg::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this) == true) return this;
	return _hold->findChild(func);
}

std::list<std::string> ASTNot::toStringList()
{
	return {"!" + _hold->toString()};
}

ASTNot::~ASTNot()
{
	delete _hold;
}

ASTNot::ASTNot(ASTExpression* contained)
	: _hold(contained)
{
}

Type* ASTNot::getExpressionType() const
{
	return BOOL;
}

ASTExpression* ASTNot::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	return _hold->findChild(func);
}

std::list<std::string> ASTBlock::toStringList()
{
	list<string> ret;
	ret.emplace_back("{");
	for (auto& i : _stmts)
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
	delete _scope_table;
	for (auto& i : _stmts) delete i;
}

ScopeTableInAST* ASTBlock::getScopeTable()
{
	return _scope_table;
}

bool ASTBlock::isEmpty() const
{
	return _stmts.empty();
}

std::list<std::string> ASTAssign::toStringList()
{
	string ret;
	ret += _assign_to->toString();
	ret += " = ";
	ret += _assign_value->toString();
	return {ret};
}

ASTAssign::~ASTAssign()
{
	delete _assign_to;
	delete _assign_value;
}

ASTAssign::ASTAssign(ASTLVal* assign_to, ASTExpression* assign_value)
	: _assign_to(assign_to),
	  _assign_value(assign_value)
{
}

std::list<std::string> ASTIf::toStringList()
{
	list<string> ret;
	string str = "if(";
	str += _cond->toString();
	str += ')';
	ret.emplace_back(str);
	if (_if_stmt.size() == 1 && dynamic_cast<ASTBlock*>(_if_stmt[0]) != nullptr)
	{
		for (auto& i : _if_stmt[0]->toStringList())
		{
			ret.emplace_back(i);
		}
	}
	else
	{
		ret.emplace_back("{");
		for (auto& i : _if_stmt)
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
	if (_else_stmt.empty()) return ret;
	ret.emplace_back("else");
	if (_else_stmt.size() == 1 && dynamic_cast<ASTBlock*>(_else_stmt[0]) != nullptr)
	{
		for (auto& i : _else_stmt[0]->toStringList())
		{
			ret.emplace_back(i);
		}
	}
	else
	{
		ret.emplace_back("{");
		for (auto& i : _else_stmt)
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
	delete _cond;
	for (auto& i : _if_stmt) delete i;
	for (auto& i : _else_stmt) delete i;
}

std::list<std::string> ASTWhile::toStringList()
{
	string str = "while(";
	str += _cond->toString();
	str += ')';
	if (_stmt.empty())
	{
		str += ';';
		return {str};
	}
	list<string> ret;
	ret.emplace_back(str);
	if (_stmt.size() == 1 && dynamic_cast<ASTBlock*>(_stmt[0]) != nullptr)
	{
		for (auto& i : _stmt[0]->toStringList())
		{
			ret.emplace_back(i);
		}
	}
	else
	{
		ret.emplace_back("{");
		for (auto& i : _stmt)
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
	delete _cond;
	for (auto& i : _stmt) delete i;
}

std::list<std::string> ASTBreak::toStringList()
{
	return {"break"};
}

std::list<std::string> ASTReturn::toStringList()
{
	string ret = "return";
	if (_return_value != nullptr)
	{
		ret += ' ';
		ret += _return_value->toString();
	}
	return {ret};
}

ASTReturn::~ASTReturn()
{
	delete _return_value;
}

ASTNumber::ASTNumber(const int i)
{
	_type = INT;
	_field._i_value = i;
}

ASTNumber::ASTNumber(const float i)
{
	_type = FLOAT;
	_field._f_value = i;
}

ASTNumber::ASTNumber(const bool i)
{
	_type = BOOL;
	_field._b_value = i;
}

ASTNumber::ASTNumber(InitializeValue v)
{
	if (v.isIntConstant())
	{
		_type = INT;
		_field._i_value = v.getIntConstant();
	}
	else if (v.isFloatConstant())
	{
		_type = FLOAT;
		_field._f_value = v.getFloatConstant();
	}
	else if (v.isBoolConstant())
	{
		_type = BOOL;
		_field._b_value = v.getBoolConstant();
	}
	else throw runtime_error("can not initialize a number with not constant initialize value.");
}

bool ASTNumber::isFloat() const
{
	return _type == FLOAT;
}

bool ASTNumber::isInteger() const
{
	return _type == INT;
}

bool ASTNumber::isBoolean() const
{
	return _type == BOOL;
}

int ASTNumber::toInteger() const
{
	if (_type != INT)
		throw
			runtime_error(getExpressionType()->toString() + " Number do not have int value");
	return _field._i_value;
}

float ASTNumber::toFloat() const
{
	if (_type != FLOAT)
		throw runtime_error(
			getExpressionType()->toString() + " Number do not have float value");
	return _field._f_value;
}

bool ASTNumber::toBoolean() const
{
	if (_type != BOOL)
		throw runtime_error(
			getExpressionType()->toString() + " Number do not have bool value");
	return _field._b_value;
}

int ASTNumber::forceToInteger() const
{
	if (_type == INT) return _field._i_value;
	if (_type == FLOAT)
		return static_cast<int>(_field._f_value);
	throw runtime_error(
		getExpressionType()->toString() + " Number do not have int value");
}

float ASTNumber::forceToFloat() const
{
	if (_type == FLOAT)
		return _field._f_value;
	if (_type == INT)
		return static_cast<float>(_field._i_value);
	throw runtime_error(
		getExpressionType()->toString() + " Number do not have float value");
}

ASTExpression* ASTNumber::castTypeTo(Type* type, TypeCastEnvironment environment)
{
	if (type == getExpressionType()) return this;
	auto from = getExpressionType();
	checkForTypeCast(from, type, environment);
	if (from == FLOAT)
	{
		if (type == INT)
		{
			_type = INT;
			_field._i_value = static_cast<int>(_field._f_value);
		}
		else if (type == BOOL)
		{
			_type = BOOL;
			_field._b_value = _field._f_value != 0.0f;
		}
	}
	else if (_type == INT)
	{
		if (type == FLOAT)
		{
			_type = FLOAT;
			_field._f_value = static_cast<float>(_field._i_value);
		}
		else if (type == BOOL)
		{
			_type = BOOL;
			_field._b_value = _field._i_value != 0;
		}
	}
	else
	{
		if (type == INT)
		{
			_type = INT;
			_field._i_value = _field._b_value ? 1 : 0;
		}
		else if (type == FLOAT)
		{
			_type = FLOAT;
			_field._f_value = _field._b_value ? 1.0f : 0.0f;
		}
	}
	return this;
}

Type* ASTNumber::getExpressionType() const
{
	return _type;
}

InitializeValue ASTNumber::toInitializeValue() const
{
	if (_type == INT) return InitializeValue{_field._i_value};
	if (_type == FLOAT) return InitializeValue{_field._f_value};
	return InitializeValue{_field._b_value};
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
	str += _castTo->toString();
	str += ')';
	str += _source->toString();
	return {str};
}

ASTCast::~ASTCast()
{
	delete _source;
}

ASTCast::ASTCast(ASTExpression* source, Type* cast_to)
	: _source(source),
	  _castTo(cast_to)
{
}

Type* ASTCast::getExpressionType() const
{
	return _castTo;
}

ASTExpression* ASTCast::findChild(const std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	if (const auto ret = _source->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTMathExp::toStringList()
{
	string ret = "(";
	ret += _l->toString();
	ret += ' ';
	if (_op == MathOP::ADD) ret += '+';
	else if (_op == MathOP::SUB) ret += '-';
	else if (_op == MathOP::MUL) ret += '*';
	else if (_op == MathOP::DIV) ret += '/';
	else ret += '%';
	ret += ' ';
	ret += _r->toString();
	ret += ')';
	return {ret};
}

ASTMathExp::~ASTMathExp()
{
	delete _l;
	delete _r;
}

ASTMathExp::ASTMathExp(Type* result_type, const MathOP op, ASTExpression* l, ASTExpression* r)
	: _result_type(result_type),
	  _op(op),
	  _l(l),
	  _r(r)
{
}

Type* ASTMathExp::getExpressionType() const
{
	return _result_type;
}

ASTExpression* ASTMathExp::findChild(const std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	if (const auto ret = _l->findChild(func); ret != nullptr) return ret;
	if (const auto ret = _r->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTLogicExp::toStringList()
{
	string ret = "(";
	for (auto& i : _exps)
	{
		ret += i->toString();
		ret += ' ';
		if (_op == LogicOP::OR) ret += "||";
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
	for (auto& i : _exps) delete i;
}

Type* ASTLogicExp::getExpressionType() const
{
	return BOOL;
}

ASTExpression* ASTLogicExp::findChild(const std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	for (auto& i : _exps)
		if (const auto ret = i->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTEqual::toStringList()
{
	string ret = "(";
	ret += _l->toString();
	ret += ' ';
	if (_op_equal) ret += "==";
	else ret += "!=";
	ret += ' ';
	ret += _r->toString();
	ret += ')';
	return {ret};
}

ASTEqual::~ASTEqual()
{
	delete _l;
	delete _r;
}

Type* ASTEqual::getExpressionType() const
{
	return BOOL;
}

ASTExpression* ASTEqual::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	if (const auto ret = _l->findChild(func); ret != nullptr) return ret;
	if (const auto ret = _r->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTRelation::toStringList()
{
	string ret = "(";
	ret += _l->toString();
	ret += ' ';
	if (_op == RelationOP::LT) ret += '<';
	else if (_op == RelationOP::GT) ret += '>';
	else if (_op == RelationOP::LE) ret += "<=";
	else ret += ">=";
	ret += ' ';
	ret += _r->toString();
	ret += ')';
	return {ret};
}

ASTRelation::~ASTRelation()
{
	delete _l;
	delete _r;
}

Type* ASTRelation::getExpressionType() const
{
	return BOOL;
}

ASTExpression* ASTRelation::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	if (const auto ret = _l->findChild(func); ret != nullptr) return ret;
	if (const auto ret = _r->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTCall::toStringList()
{
	string ret = _function->id();
	ret += '(';
	for (auto& i : _parameters)
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
	for (auto& i : _parameters) delete i;
}

ASTCall::ASTCall(ASTFuncDecl* function, const std::vector<ASTExpression*>& parameters)
	: _function(function),
	  _parameters(parameters)
{
	_haveFuncCall = true;
}

Type* ASTCall::getExpressionType() const
{
	return _function->returnType();
}

ASTExpression* ASTCall::findChild(std::function<bool(const ASTExpression*)> func)
{
	if (func(this)) return this;
	for (auto& i : _parameters)
		if (const auto ret = i->findChild(func); ret != nullptr) return ret;
	return nullptr;
}

std::list<std::string> ASTNeg::toStringList()
{
	string ret = "-";
	ret += _hold->toString();
	return {ret};
}

ASTNeg::~ASTNeg()
{
	delete _hold;
}

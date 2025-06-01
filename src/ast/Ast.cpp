#include <charconv>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "../../include/ast/Ast.hpp"
#include "../../include/util/System.hpp"
#include "../../include/util/Tensor.hpp"
#include "../../include/util/Type.hpp"
#include "../../include/ir/AST2IR.hpp"
#include "../../include/ir/Constant.hpp"

using namespace system_about;

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


		// 初始化值不是常量
		std::runtime_error InitializeValueNotConst(const InitializeValue& value)
		{
			return std::runtime_error{
				"InitializeValueNotConst Error: Can not convert InitializeValue " + value.toString() +
				" to constant because it is not constant."
			};
		}
	}
}

namespace
{
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

bool BlockScopeTableInAST::pushScope(ASTDecl* decl)
{
	if (const auto f = dynamic_cast<ASTFuncDecl*>(decl); f != nullptr)return false;
	auto v = dynamic_cast<ASTVarDecl*>(decl);
	if (_var_scopes.find(v->id()) != _var_scopes.end())return false;
	_var_scopes.emplace(decl->id(), v);
	return true;
}

ASTDecl* BlockScopeTableInAST::findScope(const std::string& id)
{
	if (const auto idx = _var_scopes.find(id); idx != _var_scopes.end()) return idx->second;
	return nullptr;
}

bool InitializeValue::isExpression() const
{
	return _field._segment[LOGICAL_LEFT_END_8] == static_cast<char>(0x00);
}

bool InitializeValue::isConstant() const
{
	return _field._segment[LOGICAL_LEFT_END_8] != static_cast<char>(0x00);
}

bool InitializeValue::isIntConstant() const
{
	return _field._segment[LOGICAL_LEFT_END_8] == static_cast<char>(0x10);
}

bool InitializeValue::isBoolConstant() const
{
	return _field._segment[LOGICAL_LEFT_END_8] == static_cast<char>(0x11);
}

bool InitializeValue::isFloatConstant() const
{
	return _field._segment[LOGICAL_LEFT_END_8] == static_cast<char>(0x01);
}

ASTExpression* InitializeValue::getExpression() const
{
	if (!isExpression()) throw std::runtime_error("InitializeValue Not Expression");
	return _field._expression;
}

int InitializeValue::getIntConstant() const
{
	if (!isIntConstant()) throw std::runtime_error("InitializeValue Not Int ConstantValue");
	return _field._int_value[LOGICAL_RIGHT_END_2];
}

bool InitializeValue::getBoolConstant() const
{
	if (!isBoolConstant()) throw std::runtime_error("InitializeValue Not Bool ConstantValue");
	return _field._bool_value[LOGICAL_RIGHT_END_8];
}

float InitializeValue::getFloatConstant() const
{
	if (!isFloatConstant()) throw std::runtime_error("InitializeValue Not Float ConstantValue");
	return _field._float_value[LOGICAL_RIGHT_END_2];
}

InitializeValue::InitializeValue()
{
	_field._expression = nullptr;
}

InitializeValue::InitializeValue(ASTExpression* expression)
{
	_field._expression = expression;
}

InitializeValue::InitializeValue(const int intConstant)
{
	_field._expression = nullptr;
	_field._int_value[LOGICAL_RIGHT_END_2] = intConstant;
	_field._segment[LOGICAL_LEFT_END_8] = static_cast<char>(0x10);
}

InitializeValue::InitializeValue(const float floatConstant)
{
	_field._expression = nullptr;
	_field._float_value[LOGICAL_RIGHT_END_2] = floatConstant;
	_field._segment[LOGICAL_LEFT_END_8] = static_cast<char>(0x01);
}

InitializeValue::InitializeValue(const bool boolConstant)
{
	_field._expression = nullptr;
	_field._bool_value[LOGICAL_RIGHT_END_8] = boolConstant;
	_field._segment[LOGICAL_LEFT_END_8] = static_cast<char>(0x11);
}

Type* InitializeValue::getExpressionType() const
{
	if (isExpression()) return _field._expression->getExpressionType();
	if (isIntConstant()) return INT;
	if (isFloatConstant()) return FLOAT;
	return BOOL;
}

std::string InitializeValue::toString() const
{
	if (isExpression()) return _field._expression->toString();
	if (isIntConstant()) return to_string(_field._int_value[LOGICAL_RIGHT_END_2]);
	if (isFloatConstant())
	{
		std::ostringstream oss;
		oss << std::scientific << std::setprecision(8) << _field._float_value[LOGICAL_RIGHT_END_2];
		return oss.str() + 'f';
	}
	return _field._bool_value[LOGICAL_RIGHT_END_8] ? "true" : "false";
}

ConstantValue InitializeValue::toConstant() const
{
	if (isExpression()) throw error::InitializeValueNotConst(*this);
	ConstantValue ret;
	if (isBoolConstant()) ret = ConstantValue{getBoolConstant()};
	else if (isIntConstant()) ret = ConstantValue{getIntConstant()};
	else ret = ConstantValue{getFloatConstant()};
	return ret;
}


bool ConstantValue::isIntConstant() const
{
	return _field._segment[LOGICAL_LEFT_END_8] == static_cast<char>(0x10);
}

bool ConstantValue::isBoolConstant() const
{
	return _field._segment[LOGICAL_LEFT_END_8] == static_cast<char>(0x11);
}

bool ConstantValue::isFloatConstant() const
{
	return _field._segment[LOGICAL_LEFT_END_8] == static_cast<char>(0x01);
}

int ConstantValue::getIntConstant() const
{
	if (!isIntConstant()) throw std::runtime_error("ConstantValue Not Int ConstantValue");
	return _field._int_value[LOGICAL_RIGHT_END_2];
}

bool ConstantValue::getBoolConstant() const
{
	if (!isBoolConstant()) throw std::runtime_error("ConstantValue Not Bool ConstantValue");
	return _field._bool_value[LOGICAL_RIGHT_END_8];
}

float ConstantValue::getFloatConstant() const
{
	if (!isFloatConstant()) throw std::runtime_error("ConstantValue Not Float ConstantValue");
	return _field._float_value[LOGICAL_RIGHT_END_2];
}

ConstantValue::ConstantValue(const int intConstant)
{
	_field._int_value[LOGICAL_RIGHT_END_2] = intConstant;
	_field._segment[LOGICAL_LEFT_END_8] = static_cast<char>(0x10);
}

ConstantValue::ConstantValue(const float floatConstant)
{
	_field._float_value[LOGICAL_RIGHT_END_2] = floatConstant;
	_field._segment[LOGICAL_LEFT_END_8] = static_cast<char>(0x01);
}

ConstantValue::ConstantValue(const bool boolConstant)
{
	_field._bool_value[LOGICAL_RIGHT_END_8] = boolConstant;
	_field._segment[LOGICAL_LEFT_END_8] = static_cast<char>(0x11);
}

Type* ConstantValue::getType() const
{
	if (isIntConstant()) return INT;
	if (isFloatConstant()) return FLOAT;
	return BOOL;
}

std::string ConstantValue::toString() const
{
	if (isIntConstant()) return to_string(_field._int_value[LOGICAL_RIGHT_END_2]);
	if (isFloatConstant())
	{
		std::ostringstream oss;
		oss << std::scientific << std::setprecision(8) << _field._float_value[LOGICAL_RIGHT_END_2];
		return oss.str() + 'f';
	}
	return _field._bool_value[LOGICAL_RIGHT_END_8] ? "true" : "false";
}

std::string ConstantValue::print() const
{
	if (isIntConstant()) return to_string(_field._int_value[LOGICAL_RIGHT_END_2]);
	if (isFloatConstant())
	{
		double val = _field._float_value[LOGICAL_RIGHT_END_2];
		int64_t intRep;
		memcpy(&intRep, &val, sizeof(double));
		std::ostringstream oss;
		oss << std::hex << intRep;
		return "0x" + oss.str();
	}
	return _field._bool_value[LOGICAL_RIGHT_END_8] ? "1" : "0";
}

InitializeValue ConstantValue::toInitializeValue() const
{
	if (isIntConstant()) return InitializeValue{_field._int_value[LOGICAL_RIGHT_END_2]};
	if (isFloatConstant()) return InitializeValue{_field._float_value[LOGICAL_RIGHT_END_2]};
	return InitializeValue{_field._bool_value[LOGICAL_RIGHT_END_8]};
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


std::list<std::string> ASTNode::toStringList()
{
	return {};
}

ASTCompUnit::~ASTCompUnit()
{
	for (auto& i : _var_declarations) delete i;
	for (auto& i : _func_declarations) delete i;
	for (auto& i : _lib) delete i;
}

ASTCompUnit::ASTCompUnit()
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
		decl->_in_lib = true;
		_lib.emplace_back(decl);
		pushScope(decl);
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

Value* ASTCompUnit::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
}

bool ASTCompUnit::pushScope(ASTDecl* decl)
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

ASTDecl* ASTCompUnit::findScope(const std::string& id, bool isFunc)
{
	if (isFunc)
	{
		if (const auto idx = _func_scopes.find(id); idx != _func_scopes.end()) return idx->second;
		return nullptr;
	}
	if (const auto idx = _var_scopes.find(id); idx != _var_scopes.end()) return idx->second;
	return nullptr;
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

Value* ASTVarDecl::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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
	_id = id;
}

ASTFuncDecl::ASTFuncDecl(const std::string& id, Type* return_type, const std::vector<Type*>& innerArgs)
	: _return_type(return_type), _block(new ASTBlock())
{
	_id = id;
	const string idTemplate = _id + "Parameter";
	int idx = 0;
	for (auto& arg : innerArgs)
	{
		const auto d = new ASTVarDecl(false, false, arg);
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

Value* ASTFuncDecl::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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
		auto toASTRVal = dynamic_cast<ASTRVal*>(input);
		if (toASTRVal != nullptr)
		{
			for (auto& j : toASTRVal->_index)
				for (auto& i : cutExpressionToOnlyLeaveFuncCall(j))
				{
					ret.emplace_back(i);
				}
			toASTRVal->_index.clear();
			delete toASTRVal;
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

ASTVarDecl* ASTLVal::getDeclaration() const
{
	return _decl;
}

Value* ASTLVal::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
}

std::list<std::string> ASTRVal::toStringList()
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

ASTRVal::~ASTRVal()
{
	for (auto& i : _index) delete i;
}

ASTRVal::ASTRVal(ASTVarDecl* target, const std::vector<ASTExpression*>& index)
	: _decl(target),
	  _index(index)
{
}

Type* ASTRVal::getExpressionType() const
{
	const auto ty = _decl->getType();
	if (ty->isArrayType())
	{
		const auto ar = dynamic_cast<ArrayType*>(ty);
		return ar->getSubType(static_cast<int>(_index.size()));
	}
	return ty;
}

ASTVarDecl* ASTRVal::getDeclaration() const
{
	return _decl;
}

Value* ASTRVal::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
}

ASTLVal* ASTRVal::toLVal() const
{
	ASTLVal* l_val = new ASTLVal{_decl, _index};
	return l_val;
}

std::list<std::string> ASTNumber::toStringList()
{
	return {_field.toString()};
}

ASTNeg::ASTNeg(ASTExpression* hold)
	: _hold(hold)
{
}

Type* ASTNeg::getExpressionType() const
{
	return _hold->getExpressionType();
}


Value* ASTNeg::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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

Value* ASTNot::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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
	for (auto& i : _stmts) delete i;
}

bool ASTBlock::isEmpty() const
{
	return _stmts.empty();
}

Value* ASTBlock::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
}

bool ASTBlock::pushScope(ASTDecl* decl)
{
	if (const auto f = dynamic_cast<ASTFuncDecl*>(decl); f != nullptr)return false;
	auto v = dynamic_cast<ASTVarDecl*>(decl);
	if (_var_scopes.find(v->id()) != _var_scopes.end())return false;
	_var_scopes.emplace(decl->id(), v);
	return true;
}

ASTDecl* ASTBlock::findScope(const std::string& id, bool isFunc)
{
	if (isFunc) return nullptr;
	if (const auto idx = _var_scopes.find(id); idx != _var_scopes.end()) return idx->second;
	return nullptr;
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

Value* ASTAssign::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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

Value* ASTIf::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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

Value* ASTWhile::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
}

std::list<std::string> ASTBreak::toStringList()
{
	return {"break"};
}

Value* ASTBreak::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
}

std::list<std::string> ASTContinue::toStringList()
{
	return {"continue"};
}

Value* ASTContinue::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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

Value* ASTReturn::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
}

ASTNumber::ASTNumber(const int i)
{
	_field = ConstantValue{i};
}

ASTNumber::ASTNumber(const float i)
{
	_field = ConstantValue{i};
}

ASTNumber::ASTNumber(const bool i)
{
	_field = ConstantValue{i};
}

ASTNumber::ASTNumber(InitializeValue v)
{
	_field = v.toConstant();
}

bool ASTNumber::isFloat() const
{
	return _field.isFloatConstant();
}

bool ASTNumber::isInteger() const
{
	return _field.isIntConstant();
}

bool ASTNumber::isBoolean() const
{
	return _field.isBoolConstant();
}

int ASTNumber::toInteger() const
{
	return _field.getIntConstant();
}

float ASTNumber::toFloat() const
{
	return _field.getFloatConstant();
}

bool ASTNumber::toBoolean() const
{
	return _field.getBoolConstant();
}

int ASTNumber::forceToInteger() const
{
	if (_field.isIntConstant()) return _field.getIntConstant();
	if (_field.isFloatConstant())
		return static_cast<int>(_field.getFloatConstant());
	throw runtime_error(
		getExpressionType()->toString() + " Number do not have int value");
}

float ASTNumber::forceToFloat() const
{
	if (_field.isFloatConstant()) return _field.getFloatConstant();
	if (_field.isIntConstant())
		return static_cast<float>(_field.getIntConstant());
	throw runtime_error(
		getExpressionType()->toString() + " Number do not have float value");
}

ASTExpression* ASTNumber::castTypeTo(Type* type, TypeCastEnvironment environment)
{
	if (type == getExpressionType()) return this;
	auto from = getExpressionType();
	auto _type = _field.getType();
	checkForTypeCast(from, type, environment);
	if (from == FLOAT)
	{
		if (type == INT)
		{
			_field = ConstantValue{static_cast<int>(_field.getFloatConstant())};
		}
		else if (type == BOOL)
		{
			_field = ConstantValue{_field.getFloatConstant() != 0.0f};
		}
	}
	else if (_type == INT)
	{
		if (type == FLOAT)
		{
			_field = ConstantValue{static_cast<float>(_field.getIntConstant())};
		}
		else if (type == BOOL)
		{
			_field = ConstantValue{_field.getIntConstant() != 0};
		}
	}
	else
	{
		if (type == INT)
		{
			_field = ConstantValue{_field.getBoolConstant() ? 1 : 0};
		}
		else if (type == FLOAT)
		{
			_field = ConstantValue{_field.getBoolConstant() ? 1.0f : 0.0f};
		}
	}
	return this;
}

Type* ASTNumber::getExpressionType() const
{
	return _field.getType();
}

InitializeValue ASTNumber::toInitializeValue() const
{
	return _field.toInitializeValue();
}

Value* ASTNumber::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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


Value* ASTCast::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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

Value* ASTMathExp::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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

Value* ASTLogicExp::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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

Value* ASTEqual::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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

Value* ASTRelation::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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

Value* ASTCall::accept(AST2IRVisitor* visitor)
{
	return visitor->visit(this);
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

#pragma once
#include <functional>
#include <cstdint>
#include <map>
#include <list>
#include <string>

class Value;
template <typename T>
class Tensor;
class Type;

class SysYVisitor;
class ASTBlock;
class ASTLVal;
class ASTNeg;
class ASTNot;
class ASTLogicExp;
class ASTCall;
class ASTMathExp;
class ASTCast;
class ASTExpression;
class ASTNumber;
class ASTDecl;
class ASTVarDecl;
class ASTFuncDecl;
class ConstantValue;
class AST2IRVisitor;

// 机器是小端架构
extern bool IS_SMALL_END;

// 定义不同的表达式等级, 不同的等级导致不同的表达式转换规则
// INITIALIZE_LIST: 初始化列表, 只允许从 INT 向 FLOAT 转换, 不允许 FLOAT 向 INT 转换, 不允许任何 BOOL 转换
// MATH_EXPRESSION: 数学表达式, 允许 INT FLOAT 间转换, 不允许任何 BOOL 转换
// LOGIC_EXPRESSION: 逻辑表达式, 允许 INT FLOAT BOOL 的转换
enum class TypeCastEnvironment : uint8_t
{
	INITIALIZE_LIST,
	MATH_EXPRESSION,
	LOGIC_EXPRESSION
};

std::string to_string(TypeCastEnvironment e);

// 数学操作符
enum class MathOP : uint8_t
{
	// 加
	ADD,
	// 减
	SUB,
	// 乘
	MUL,
	// 除, r 非 0
	DIV,
	// 余数, 只能对整型操作, r 非 0
	MOD
};

// 逻辑操作符
enum class LogicOP : uint8_t
{
	// 且
	AND,
	// 或
	OR
};

// 逻辑操作符
enum class RelationOP : uint8_t
{
	// 小于
	LT,
	// 大于
	GT,
	// 小于等于
	LE,
	// 大于等于
	GE
};

// 对有符号表对象的抽象
class HaveScope
{
public:
	HaveScope(const HaveScope&) = delete;
	HaveScope(HaveScope&&) = delete;
	HaveScope& operator=(const HaveScope&) = delete;
	HaveScope& operator=(HaveScope&&) = delete;
	virtual ~HaveScope() = default;
	HaveScope() = default;
	// 插入符号. 尝试插入符号; 若重名则返回 false; 否则插入并返回 true.
	virtual bool pushScope(ASTDecl* decl) = 0;
	// 寻找符号. 尝试, 若没有符号表则向上寻找; 所有表均未找到则返回 false.
	virtual ASTDecl* findScope(const std::string& id, bool isFunc) = 0;
};

// 用于初始化列表, 是一个小的(只有一个指针大小)的类型, 可以存储常量或 ASTExpression*.
// 鉴于它并不真正维护 ASTExpression*, 需要将 ASTExpression* 挂载在另外的地方以供删除.
class InitializeValue
{
	friend bool operator==(const InitializeValue& lhs, const InitializeValue& rhs)
	{
		return lhs._field._expression == rhs._field._expression;
	}

	friend bool operator!=(const InitializeValue& lhs, const InitializeValue& rhs)
	{
		return lhs._field._expression != rhs._field._expression;
	}

	union Field
	{
		ASTExpression* _expression = nullptr;
		int _int_value[2];
		float _float_value[2];
		bool _bool_value[8];
		char _segment[8];
	};

	Field _field;

public:
	[[nodiscard]] bool isExpression() const;

	[[nodiscard]] bool isConstant() const;

	[[nodiscard]] bool isIntConstant() const;

	[[nodiscard]] bool isBoolConstant() const;

	[[nodiscard]] bool isFloatConstant() const;

	[[nodiscard]] ASTExpression* getExpression() const;

	[[nodiscard]] int getIntConstant() const;

	[[nodiscard]] bool getBoolConstant() const;

	[[nodiscard]] float getFloatConstant() const;

	explicit InitializeValue();

	explicit InitializeValue(ASTExpression* expression);

	explicit InitializeValue(int intConstant);

	explicit InitializeValue(float floatConstant);

	explicit InitializeValue(bool boolConstant);

	[[nodiscard]] Type* getExpressionType() const;

	[[nodiscard]] std::string toString() const;

	// 尝试将 InitializeValue 转换为 ConstantValue
	[[nodiscard]] ConstantValue toConstant() const;
};

// 用于常量, 是一个小的(只有一个指针大小)的类型, 可以存储常量
class ConstantValue
{
	friend bool operator==(const ConstantValue& lhs, const ConstantValue& rhs)
	{
		return lhs._field._value == rhs._field._value;
	}

	friend bool operator!=(const ConstantValue& lhs, const ConstantValue& rhs)
	{
		return lhs._field._value != rhs._field._value;
	}

	friend class InitializeValue;

	union Field
	{
		int _int_value[2] = {};
		float _float_value[2];
		bool _bool_value[8];
		char _segment[8];
		// 用于比较
		long long _value;
	};

	Field _field;

public:
	[[nodiscard]] bool isIntConstant() const;

	[[nodiscard]] bool isBoolConstant() const;

	[[nodiscard]] bool isFloatConstant() const;

	[[nodiscard]] int getIntConstant() const;

	[[nodiscard]] bool getBoolConstant() const;

	[[nodiscard]] float getFloatConstant() const;

	explicit ConstantValue() = default;

	explicit ConstantValue(int intConstant);

	explicit ConstantValue(float floatConstant);

	explicit ConstantValue(bool boolConstant);

	[[nodiscard]] Type* getType() const;

	[[nodiscard]] std::string toString() const;
	// IR
	[[nodiscard]] std::string print() const;

	[[nodiscard]] InitializeValue toInitializeValue() const;

	[[nodiscard]] unsigned bits2Unsigned() const;
};


// AST 树节点, 定义了一些公共方法. 该类型自身不会出现于树中. 只能是
// ASTCompUnit, ASTDecl, ASTStmt
class ASTNode
{
	friend class Antlr2AstVisitor;

public:
	// 输出为字符串列表, 默认是以行分割
	virtual std::list<std::string> toStringList();
	// 输出为字符串, 默认以 C/C++ 源代码的格式, 能够复现 SysY 程序的结果, 但不是合法的 SysY 程序.
	virtual std::string toString();
	virtual ~ASTNode() = default;
	ASTNode() = default;
	ASTNode(const ASTNode&) = delete;
	ASTNode& operator=(const ASTNode&) = delete;
	ASTNode(const ASTNode&&) = delete;
	ASTNode& operator=(const ASTNode&&) = delete;
	virtual Value* accept(AST2IRVisitor* visitor) = 0;
};

// AST 树的根节点. 同时管理了整棵树的额外数据 *
class ASTCompUnit final : public ASTNode, public HaveScope
{
public:
	[[nodiscard]] const std::vector<ASTVarDecl*>& var_declarations() const
	{
		return _var_declarations;
	}

	[[nodiscard]] const std::vector<ASTFuncDecl*>& func_declarations() const
	{
		return _func_declarations;
	}

	[[nodiscard]] const std::vector<ASTFuncDecl*>& lib() const
	{
		return _lib;
	}

	~ASTCompUnit() override;
	ASTCompUnit();

	ASTCompUnit(const ASTCompUnit&) = delete;
	ASTCompUnit& operator=(const ASTCompUnit&) = delete;
	ASTCompUnit(const ASTCompUnit&&) = delete;
	ASTCompUnit& operator=(const ASTCompUnit&&) = delete;

	std::list<std::string> toStringList() override;

private:
	friend class Antlr2AstVisitor;
	friend class ASTNumber;
	// 变量声明 *
	std::vector<ASTVarDecl*> _var_declarations;
	// 函数声明 *
	std::vector<ASTFuncDecl*> _func_declarations;
	// 库函数, 在创建 ASTCompUnit 时自动加入. *
	std::vector<ASTFuncDecl*> _lib;
	// 全局变量声明符号表
	std::map<std::string, ASTVarDecl*> _var_scopes;
	// 函数声明符号表
	std::map<std::string, ASTFuncDecl*> _func_scopes;

public:
	Value* accept(AST2IRVisitor* visitor) override;
	bool pushScope(ASTDecl* decl) override;
	ASTDecl* findScope(const std::string& id, bool isFunc) override;
};

// 声明, 该类型自身不会出现于树中. 只能是 ASTVarDecl 或 ASTFuncDecl
class ASTDecl : public ASTNode
{
	friend class ASTCompUnit;
	friend class Antlr2AstVisitor;

protected:
	// 声明 ID
	std::string _id;

public:
	// 声明 ID
	const std::string& id();
};

// 变量声明, 有变量类型, 变量名和初始化列表
class ASTVarDecl final : public ASTDecl
{
	std::list<std::string> toStringList() override;
	friend class Antlr2AstVisitor;

public:
	ASTVarDecl(const ASTVarDecl& other) = delete;
	ASTVarDecl(ASTVarDecl&& other) = delete;
	ASTVarDecl& operator=(const ASTVarDecl& other) = delete;
	ASTVarDecl& operator=(ASTVarDecl&& other) = delete;

	ASTVarDecl(bool is_const, bool is_global, Type* type)
		: _is_const(is_const),
		  _is_global(is_global),
		  _type(type)
	{
	}

	~ASTVarDecl() override;

private:
	friend class Antlr2AstVisitor;
	friend class ASTFuncDecl;

	// 是否是常量
	bool _is_const;
	// 是否是全局变量
	bool _is_global;
	// 变量的类型
	Type* _type{};
	// 初始值列表, 没有则为空 *
	Tensor<InitializeValue>* _initList = nullptr;
	// 需要被计算的非常量初始化表达式, 用于维护内存 *
	std::vector<ASTExpression*> _expressions;

public:
	// 是否是常量
	[[nodiscard]] bool isConst() const;
	// 是否是全局变量
	[[nodiscard]] bool isGlobal() const;
	// 获取初始化列表
	[[nodiscard]] const Tensor<InitializeValue>* getInitList() const;
	// 变量的类型
	[[nodiscard]] Type* getType() const;
	Value* accept(AST2IRVisitor* visitor) override;
};

// 函数声明, 有函数返回值, 参数声明
class ASTFuncDecl final : public ASTDecl, public HaveScope
{
public:
	[[nodiscard]] ASTBlock* block() const
	{
		return _block;
	}

private:
	// 符号表
	std::map<std::string, ASTVarDecl*> _var_scopes;
	std::list<std::string> toStringList() override;
	~ASTFuncDecl() override;
	friend class Antlr2AstVisitor;

public:
	ASTFuncDecl(const ASTFuncDecl& other) = delete;
	ASTFuncDecl(ASTFuncDecl&& other) = delete;
	ASTFuncDecl& operator=(const ASTFuncDecl& other) = delete;
	ASTFuncDecl& operator=(ASTFuncDecl&& other) = delete;
	// 初始化函数, 参数留空
	ASTFuncDecl(const std::string& id, Type* return_type);
	// 匿名初始化参数
	ASTFuncDecl(const std::string& id, Type* return_type, const std::vector<Type*>& innerArgs);

	// 返回值类型
	[[nodiscard]] Type* returnType() const;
	// 参数列表
	[[nodiscard]] const std::vector<ASTVarDecl*>& args() const;

private:
	friend class ASTCompUnit;

	// 是否是库函数
	bool _in_lib = false;
	// 返回值类型
	Type* _return_type{};
	// 参数列表, 参数由函数管理(虽然挂载在其基本块符号表中) *
	std::vector<ASTVarDecl*> _args;
	// 对应基本块 *
	ASTBlock* _block{};

public:
	// 是否是库函数
	[[nodiscard]] bool isLibFunc() const;
	Value* accept(AST2IRVisitor* visitor) override;
	bool pushScope(ASTDecl* decl) override;
	ASTDecl* findScope(const std::string& id, bool isFunc) override;
};

// 语句. 该类型自身不会出现于树中.
// 只能是 ASTBlock, ASTAssign, ASTExpression, ASTIf, ASTWhile, ASTBreak, ASTContinue, ASTReturn
class ASTStmt : public ASTNode
{
};

namespace AST
{
	// 裁剪表达式, 留下尽可能简介的包含等价副作用的表达式列表.
	std::list<ASTExpression*> cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
}

// 表达式, 该类型自身不会出现于树中. 只能是
// ASTCast, ASTMathExp, ASTCall, ASTNeg, ASTRVal, ASTNot,  ASTLogicExp, ASTEqual, ASTRelation, ASTNumber
class ASTExpression : public ASTStmt
{
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	friend class Antlr2AstVisitor;

protected:
	// 该表达式是否调用了函数
	bool _haveFuncCall{};

public:
	// 表达式值的类型
	[[nodiscard]] virtual Type* getExpressionType() const = 0;
	// 获取两个表达式类型提升公共类型
	// 如果无公共类型, 它会抛出异常
	static Type* maxType(const ASTExpression* a, const ASTExpression* b,
	                     TypeCastEnvironment environment = TypeCastEnvironment::MATH_EXPRESSION);

protected:
	// 转换表达式类型, 对于常量, 返回对应转换后常量S
	// 对于表达式, 插入一个 ASTCast; 如果允许转换为 bool, 将会插入 Math2Logic
	virtual ASTExpression* castTypeTo(
		Type* type, TypeCastEnvironment environment = TypeCastEnvironment::MATH_EXPRESSION);
};

// 左值, 包含其引用的声明和使用的数组索引
class ASTLVal final : public ASTNode
{
public:
	[[nodiscard]] const std::vector<ASTExpression*>& index() const
	{
		return _index;
	}

private:
	std::list<std::string> toStringList() override;
	// 引用声明
	ASTVarDecl* _decl{};
	// 索引, 若没有则为空 *
	std::vector<ASTExpression*> _index;

public:
	ASTLVal(const ASTLVal& other) = delete;
	ASTLVal(ASTLVal&& other) = delete;
	ASTLVal& operator=(const ASTLVal& other) = delete;
	ASTLVal& operator=(ASTLVal&& other) = delete;
	~ASTLVal() override;
	ASTLVal(ASTVarDecl* target, const std::vector<ASTExpression*>& index);

	// 引用声明
	[[nodiscard]] ASTVarDecl* getDeclaration() const;
	Value* accept(AST2IRVisitor* visitor) override;
};

// 右值表达式, 包含其引用的声明和使用的数组索引. 区分于左值, 右值是表达式而左值不是.
class ASTRVal final : public ASTExpression
{
	friend class Antlr2AstVisitor;

public:
	[[nodiscard]] const std::vector<ASTExpression*>& index() const
	{
		return _index;
	}

private:
	std::list<std::string> toStringList() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	// 引用声明
	ASTVarDecl* _decl{};
	// 索引, 若没有则为空 *
	std::vector<ASTExpression*> _index{};

	// 尽管逻辑上是左值可以转换为右值, 但是在 AST 中由于右值是表达式, 对数据结构更友好
	[[nodiscard]] ASTLVal* toLVal() const;

public:
	ASTRVal(const ASTRVal& other) = delete;
	ASTRVal(ASTRVal&& other) = delete;
	ASTRVal& operator=(const ASTRVal& other) = delete;
	ASTRVal& operator=(ASTRVal&& other) = delete;
	~ASTRVal() override;
	ASTRVal(ASTVarDecl* target, const std::vector<ASTExpression*>& index);

	[[nodiscard]] Type* getExpressionType() const override;
	// 引用声明
	[[nodiscard]] ASTVarDecl* getDeclaration() const;
	Value* accept(AST2IRVisitor* visitor) override;
};

// 常数, 可以是 bool / float 或 int
class ASTNumber final : public ASTExpression
{
	std::list<std::string> toStringList() override;
	friend class Antlr2AstVisitor;
	friend class ASTCompUnit;

	ConstantValue _field{};

public:
	explicit ASTNumber(int i);
	explicit ASTNumber(float i);
	explicit ASTNumber(bool i);
	// v 为 Exp 时报错
	explicit ASTNumber(InitializeValue v);

	// 是否为浮点型
	[[nodiscard]] bool isFloat() const;
	// 是否为整型
	[[nodiscard]] bool isInteger() const;
	// 是否为布尔型
	[[nodiscard]] bool isBoolean() const;
	// 如果常量为整型, 则返回值, 否则抛出异常
	[[nodiscard]] int toInteger() const;
	// 如果常量为浮点型, 则返回值, 否则抛出异常
	[[nodiscard]] float toFloat() const;
	// 如果常量为布尔型, 则返回值, 否则抛出异常
	[[nodiscard]] bool toBoolean() const;
	// 如果常量为整型, 则返回值, 如果为浮点型则转换为整型并返回值, 否则抛出异常
	[[nodiscard]] int forceToInteger() const;
	// 如果常量为浮点型, 则返回值, 如果为浮点型否则转换为浮点型并返回值, 否则抛出异常
	[[nodiscard]] float forceToFloat() const;
	// 类型转换会转换其自身
	ASTExpression*
	castTypeTo(Type* type, TypeCastEnvironment environment = TypeCastEnvironment::MATH_EXPRESSION) override;
	[[nodiscard]] Type* getExpressionType() const override;
	//转化为初始化张量数据
	[[nodiscard]] InitializeValue toInitializeValue() const;
	Value* accept(AST2IRVisitor* visitor) override;
};

// 强制类型转换, 包含源表达式和转换后类型
class ASTCast final : public ASTExpression
{
public:
	[[nodiscard]] ASTExpression* source() const
	{
		return _source;
	}

	[[nodiscard]] Type* cast_to() const
	{
		return _castTo;
	}

private:
	std::list<std::string> toStringList() override;
	~ASTCast() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);

public:
	ASTCast(const ASTCast& other) = delete;
	ASTCast(ASTCast&& other) = delete;
	ASTCast& operator=(const ASTCast& other) = delete;
	ASTCast& operator=(ASTCast&& other) = delete;

	ASTCast(ASTExpression* source, Type* cast_to);

	[[nodiscard]] Type* getExpressionType() const override;
	Value* accept(AST2IRVisitor* visitor) override;

private:
	// 源表达式 *
	ASTExpression* _source{};
	// 目标类型
	Type* _castTo{};
};

// 算数表达式, 包含左右操作数(同一类型)和算符
class ASTMathExp final : public ASTExpression
{
public:
	[[nodiscard]] MathOP op() const
	{
		return _op;
	}

	[[nodiscard]] ASTExpression* l() const
	{
		return _l;
	}

	[[nodiscard]] ASTExpression* r() const
	{
		return _r;
	}

private:
	std::list<std::string> toStringList() override;
	~ASTMathExp() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);

public:
	ASTMathExp(const ASTMathExp& other) = delete;
	ASTMathExp(ASTMathExp&& other) = delete;
	ASTMathExp& operator=(const ASTMathExp& other) = delete;
	ASTMathExp& operator=(ASTMathExp&& other) = delete;

	ASTMathExp(Type* result_type, MathOP op, ASTExpression* l, ASTExpression* r);

	[[nodiscard]] Type* getExpressionType() const override;
	Value* accept(AST2IRVisitor* visitor) override;

private:
	// 结果类型
	Type* _result_type{};
	// 算符
	MathOP _op{};
	// 左操作数 *
	ASTExpression* _l{};
	// 右操作数 *
	ASTExpression* _r{};
};

// 逻辑表达式, 包含所有操作数(bool 类型)和算符
class ASTLogicExp final : public ASTExpression
{
public:
	[[nodiscard]] LogicOP op() const
	{
		return _op;
	}

	[[nodiscard]] const std::vector<ASTExpression*>& exps() const
	{
		return _exps;
	}

private:
	std::list<std::string> toStringList() override;
	~ASTLogicExp() override;

	enum LogicResult : uint8_t
	{
		UNDEFINE, TRUE, FALSE
	};

	friend class Antlr2AstVisitor;
	// 算符
	LogicOP _op{};
	// 操作数 *
	std::vector<ASTExpression*> _exps;
	// 是否在编译期间就已经能够短路运算得到结果
	// 通常代表该表达式可以得到结果, 但是考虑到副作用对表达式进行保留; 会改变此后表达式的短路策略.
	LogicResult _have_result{};

public:
	ASTLogicExp(const ASTLogicExp& other) = delete;
	ASTLogicExp(ASTLogicExp&& other) = delete;
	ASTLogicExp& operator=(const ASTLogicExp& other) = delete;
	ASTLogicExp& operator=(ASTLogicExp&& other) = delete;

	ASTLogicExp(LogicOP op, const std::vector<ASTExpression*>& exps, LogicResult haveResult): _op(op), _exps(exps),
		_have_result(haveResult)
	{
	}

	[[nodiscard]] Type* getExpressionType() const override;
	Value* accept(AST2IRVisitor* visitor) override;
};

// 相等表达式, 包含所有操作数和算符, 其操作数不能为 bool 类型
class ASTEqual final : public ASTExpression
{
public:
	[[nodiscard]] bool op_equal() const
	{
		return _op_equal;
	}

	[[nodiscard]] ASTExpression* l() const
	{
		return _l;
	}

	[[nodiscard]] ASTExpression* r() const
	{
		return _r;
	}

private:
	std::list<std::string> toStringList() override;
	~ASTEqual() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	friend class Antlr2AstVisitor;

public:
	ASTEqual(const ASTEqual& other) = delete;
	ASTEqual(ASTEqual&& other) = delete;
	ASTEqual& operator=(const ASTEqual& other) = delete;
	ASTEqual& operator=(ASTEqual&& other) = delete;

	ASTEqual(bool op_equal, ASTExpression* l, ASTExpression* r)
		: _op_equal(op_equal),
		  _l(l),
		  _r(r)
	{
	}

private:
	friend class Antlr2AstVisitor;
	// 算符 true 为 ==; false 为 !=
	bool _op_equal{};
	// 左操作数 *
	ASTExpression* _l{};
	// 右操作数 *
	ASTExpression* _r{};

public:
	[[nodiscard]] Type* getExpressionType() const override;
	Value* accept(AST2IRVisitor* visitor) override;
};

// 比较表达式, 包含所有操作数和算符, 其操作数不能为 bool 类型
class ASTRelation final : public ASTExpression
{
public:
	[[nodiscard]] RelationOP op() const
	{
		return _op;
	}

	[[nodiscard]] ASTExpression* l() const
	{
		return _l;
	}

	[[nodiscard]] ASTExpression* r() const
	{
		return _r;
	}

private:
	std::list<std::string> toStringList() override;
	~ASTRelation() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	friend class Antlr2AstVisitor;

public:
	ASTRelation(const ASTRelation& other) = delete;
	ASTRelation(ASTRelation&& other) = delete;
	ASTRelation& operator=(const ASTRelation& other) = delete;
	ASTRelation& operator=(ASTRelation&& other) = delete;

	ASTRelation(const RelationOP op, ASTExpression* l, ASTExpression* r)
		: _op(op),
		  _l(l),
		  _r(r)
	{
	}

private:
	friend class Antlr2AstVisitor;
	// 比较算符
	RelationOP _op{};
	// 左操作数 *
	ASTExpression* _l{};
	// 右操作数 *
	ASTExpression* _r{};

public:
	[[nodiscard]] Type* getExpressionType() const override;
	Value* accept(AST2IRVisitor* visitor) override;
};

// 函数调用, 包含函数声明和输入参数
class ASTCall final : public ASTExpression
{
public:
	[[nodiscard]] ASTFuncDecl* function() const
	{
		return _function;
	}

	[[nodiscard]] const std::vector<ASTExpression*>& parameters() const
	{
		return _parameters;
	}

private:
	std::list<std::string> toStringList() override;
	~ASTCall() override;

public:
	ASTCall(const ASTCall& other) = delete;
	ASTCall(ASTCall&& other) = delete;
	ASTCall& operator=(const ASTCall& other) = delete;
	ASTCall& operator=(ASTCall&& other) = delete;
	ASTCall(ASTFuncDecl* function, const std::vector<ASTExpression*>& parameters);
	[[nodiscard]] Type* getExpressionType() const override;
	Value* accept(AST2IRVisitor* visitor) override;

private:
	// 函数声明
	ASTFuncDecl* _function{};
	// 参数 *
	// 对于数组类型, AST 的检查保证参数一定可以传入函数, 但不代表它们的类型相同
	// 例如函数参数是 integer[][2], 那它可能输入类型 integer[][2], integer[3][2]
	// 无论如何, 已经不需要额外检查保证其正确性
	std::vector<ASTExpression*> _parameters;
};

// 取负号表达式, 包含操作数. 该节点会自动简化
class ASTNeg final : ASTExpression
{
public:
	[[nodiscard]] ASTExpression* hold() const
	{
		return _hold;
	}

private:
	std::list<std::string> toStringList() override;
	~ASTNeg() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	friend class Antlr2AstVisitor;
	// 操作数 *
	ASTExpression* _hold{};

public:
	ASTNeg(const ASTNeg& other) = delete;
	ASTNeg(ASTNeg&& other) = delete;
	ASTNeg& operator=(const ASTNeg& other) = delete;
	ASTNeg& operator=(ASTNeg&& other) = delete;
	explicit ASTNeg(ASTExpression* hold);

	[[nodiscard]] Type* getExpressionType() const override;
	Value* accept(AST2IRVisitor* visitor) override;
};

// 非表达式, 只能作用于逻辑表达式. 两个 ! 会折叠. 包含符号和所含表达式
class ASTNot final : public ASTExpression
{
public:
	[[nodiscard]] ASTExpression* hold() const
	{
		return _hold;
	}

private:
	std::list<std::string> toStringList() override;
	~ASTNot() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	friend class Antlr2AstVisitor;
	friend class ASTExpression;
	friend class ASTCast;

public:
	ASTNot(const ASTNot& other) = delete;
	ASTNot(ASTNot&& other) = delete;
	ASTNot& operator=(const ASTNot& other) = delete;
	ASTNot& operator=(ASTNot&& other) = delete;
	// 作为 not 被创建
	explicit ASTNot(ASTExpression* contained);
	[[nodiscard]] Type* getExpressionType() const override;
	Value* accept(AST2IRVisitor* visitor) override;

protected:
	// 所含表达式 *
	ASTExpression* _hold{};
};

// 基本块. 包含一个语句列表和一个符号表. *
class ASTBlock final : public ASTStmt, public HaveScope
{
public:
	[[nodiscard]] std::vector<ASTNode*>& stmts()
	{
		return _stmts;
	}

private:
	std::list<std::string> toStringList() override;

	~ASTBlock() override;
	friend class Antlr2AstVisitor;
	friend class ASTFuncDecl;

protected:
	// 符号表
	std::map<std::string, ASTVarDecl*> _var_scopes;
	// 语句列表, 可以是 ASTStmt 或 ASTVarDecl, 考虑到初始化时使用的函数调用, 声明列表无法与语句列表独立 *
	std::vector<ASTNode*> _stmts;
	// 块是否为空, 具体来说, 其中不包含任何语句与任何声明
	[[nodiscard]] bool isEmpty() const;

public:
	ASTBlock() = default;
	ASTBlock(const ASTBlock& other) = delete;
	ASTBlock(ASTBlock&& other) = delete;
	ASTBlock& operator=(const ASTBlock& other) = delete;
	ASTBlock& operator=(ASTBlock&& other) = delete;
	Value* accept(AST2IRVisitor* visitor) override;

private:
	bool pushScope(ASTDecl* decl) override;
	ASTDecl* findScope(const std::string& id, bool isFunc) override;
};

// 赋值语句. 包含赋值目标左值和所赋值
class ASTAssign final : public ASTStmt
{
public:
	[[nodiscard]] ASTLVal* assign_to() const
	{
		return _assign_to;
	}

	[[nodiscard]] ASTExpression* assign_value() const
	{
		return _assign_value;
	}

private:
	std::list<std::string> toStringList() override;

	~ASTAssign() override;

public:
	ASTAssign(const ASTAssign& other) = delete;
	ASTAssign(ASTAssign&& other) = delete;
	ASTAssign& operator=(const ASTAssign& other) = delete;
	ASTAssign& operator=(ASTAssign&& other) = delete;
	ASTAssign(ASTLVal* assign_to, ASTExpression* assign_value);
	Value* accept(AST2IRVisitor* visitor) override;

private:
	// 目标左值 *
	ASTLVal* _assign_to{};
	// 所赋值 *
	ASTExpression* _assign_value{};
};

// IF 语句. 包含条件, IF 子句和 ELSE 子句(没有则为空)
class ASTIf final : public ASTStmt
{
public:
	[[nodiscard]] ASTExpression* cond() const
	{
		return _cond;
	}

	[[nodiscard]] const std::vector<ASTStmt*>& if_stmt() const
	{
		return _if_stmt;
	}

	[[nodiscard]] const std::vector<ASTStmt*>& else_stmt() const
	{
		return _else_stmt;
	}

private:
	std::list<std::string> toStringList() override;
	~ASTIf() override;
	friend class Antlr2AstVisitor;

public:
	ASTIf(const ASTIf& other) = delete;
	ASTIf(ASTIf&& other) = delete;
	ASTIf& operator=(const ASTIf& other) = delete;
	ASTIf& operator=(ASTIf&& other) = delete;

	ASTIf(ASTExpression* cond) : _cond(cond)
	{
	}

	Value* accept(AST2IRVisitor* visitor) override;

private:
	// 条件 *
	ASTExpression* _cond{};
	// if 子句, 可能有 [1, ] 个元素. 它们与 cond 处于同一块中, 这通常由于优化, 是无法在代码中常规体现的. *
	std::vector<ASTStmt*> _if_stmt;
	// else 子句, 可能有 [0, ] 个元素. 它们与 cond 处于同一块中 *
	std::vector<ASTStmt*> _else_stmt;
};

// WHILE 语句. 包含条件和子句
class ASTWhile final : public ASTStmt
{
public:
	[[nodiscard]] ASTExpression* cond() const
	{
		return _cond;
	}

	[[nodiscard]] const std::vector<ASTStmt*>& stmt() const
	{
		return _stmt;
	}

private:
	std::list<std::string> toStringList() override;
	~ASTWhile() override;
	friend class Antlr2AstVisitor;

public:
	ASTWhile(const ASTWhile& other) = delete;
	ASTWhile(ASTWhile&& other) = delete;
	ASTWhile& operator=(const ASTWhile& other) = delete;
	ASTWhile& operator=(ASTWhile&& other) = delete;

	ASTWhile(ASTExpression* cond) : _cond(cond)
	{
	}

	Value* accept(AST2IRVisitor* visitor) override;

private:
	// 条件 *
	ASTExpression* _cond{};
	// 子句, 可能有 [0, ] 个元素. 它们与 cond 处于同一块中, 这通常由于优化, 是无法在代码中常规体现的. *
	std::vector<ASTStmt*> _stmt;
};

// BREAK 语句. 包含目标 While 节点
class ASTBreak final : public ASTStmt
{
	std::list<std::string> toStringList() override;
	friend class Antlr2AstVisitor;

public:
	ASTBreak(ASTWhile* target) : _target(target)
	{
	}

	Value* accept(AST2IRVisitor* visitor) override;

private:
	// 目标节点, 仅是为了方便定位, 同样只能跳出最近循环
	ASTWhile* _target;
};

// CONTINUE 语句. 包含目标 While 节点
class ASTContinue final : public ASTStmt
{
	std::list<std::string> toStringList() override;
	friend class Antlr2AstVisitor;

public:
	ASTContinue(ASTWhile* target) : _target(target)
	{
	}

	Value* accept(AST2IRVisitor* visitor) override;

private:
	// 目标节点
	ASTWhile* _target{};
};

// RETURN 语句. 包含返回值和目标返回函数
// 鉴于删除符号表的复杂性, 没有去除 RETURN 语句后代码
// 这考虑到即使去除代码, AST 也不得不读完全部代码来进行语义分析
class ASTReturn final : public ASTStmt
{
public:
	[[nodiscard]] ASTExpression* return_value() const
	{
		return _return_value;
	}

	[[nodiscard]] ASTFuncDecl* function() const
	{
		return _function;
	}

	ASTReturn(const ASTReturn& other) = delete;
	ASTReturn(ASTReturn&& other) = delete;
	ASTReturn& operator=(const ASTReturn& other) = delete;
	ASTReturn& operator=(ASTReturn&& other) = delete;

private:
	std::list<std::string> toStringList() override;

	~ASTReturn() override;
	friend class Antlr2AstVisitor;

public:
	ASTReturn(ASTExpression* return_value, ASTFuncDecl* function) : _return_value(return_value), _function(function)
	{
	}

	Value* accept(AST2IRVisitor* visitor) override;

private:
	// 返回值, 与函数返回值类型一致, 故可能为 nullptr *
	ASTExpression* _return_value{};
	// 目标函数, 仅是为了方便定位
	ASTFuncDecl* _function{};
};

#pragma once
#include "Tensor.hpp"
#include "Type.hpp"
#include "../../antlr/SysYVisitor.h"

class ASTBlock;
class ASTLVal;
class ASTNeg;
class ASTMath2Logic;
class ASTLogicExp;
class ASTCall;
class ASTMathExp;
class ASTCast;
class ASTExpression;
class ASTNumber;
class ASTDecl;
class ASTVarDecl;
class ASTFuncDecl;

// 机器是小端架构
extern bool IS_SMALL_END;

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

// AST 树内置符号表
class ScopeTableInAST
{
public:
	ScopeTableInAST(const ScopeTableInAST& other) = delete;
	ScopeTableInAST(ScopeTableInAST&& other) = delete;
	ScopeTableInAST& operator=(const ScopeTableInAST& other) = delete;
	ScopeTableInAST& operator=(ScopeTableInAST&& other) = delete;
	ScopeTableInAST() = default;
	virtual ~ScopeTableInAST() = default;
	// 尝试插入符号, 成功则返回 true. 存在同名符号则返回 false.
	// 函数符号表跟变量符号表是独立的, 允许函数与变量同名.
	virtual bool pushScope(ASTDecl* decl) = 0;
	// 尝试查找符号, 成功则返回声明, 否则返回 nullptr, 需要指定是否为函数.
	virtual ASTDecl* findScope(const std::string& id, bool isFunc) = 0;
};

// AST 顶层符号表, 管理函数和全局变量
class TopScopeTableInAST final : public ScopeTableInAST
{
	// 全局变量声明
	std::map<std::string, ASTVarDecl*> var_scopes;
	// 函数声明
	std::map<std::string, ASTFuncDecl*> func_scopes;

public:
	TopScopeTableInAST(const TopScopeTableInAST& other) = delete;
	TopScopeTableInAST(TopScopeTableInAST&& other) = delete;
	TopScopeTableInAST& operator=(const TopScopeTableInAST& other) = delete;
	TopScopeTableInAST& operator=(TopScopeTableInAST&& other) = delete;
	TopScopeTableInAST() = default;
	~TopScopeTableInAST() override = default;
	bool pushScope(ASTDecl* decl) override;
	ASTDecl* findScope(const std::string& id, bool isFunc) override;
};

// AST 块符号表, 管理局部变量
class BlockScopeTableInAST final : public ScopeTableInAST
{
	// 变量声明
	std::map<std::string, ASTVarDecl*> var_scopes;

public:
	BlockScopeTableInAST(const BlockScopeTableInAST& other) = delete;
	BlockScopeTableInAST(BlockScopeTableInAST&& other) = delete;
	BlockScopeTableInAST& operator=(const BlockScopeTableInAST& other) = delete;
	BlockScopeTableInAST& operator=(BlockScopeTableInAST&& other) = delete;
	~BlockScopeTableInAST() override = default;
	BlockScopeTableInAST() = default;
	bool pushScope(ASTDecl* decl) override;
	ASTDecl* findScope(const std::string& id, bool isFunc) override;
};

// 用于初始化列表, 是一个小的(只有一个指针大小)的类型, 可以存储常量或 ASTExpression*.
// 鉴于它并不真正维护 ASTExpression*, 需要将 ASTExpression* 挂载在另外的地方以供删除.
class InitializeValue
{
	union Field
	{
		ASTExpression* expression_ = nullptr;
		int int_value_[2];
		float float_value_[2];
		bool bool_value_[8];
		char segment_[8];
	};

	Field field_;

public:
	bool isExpression() const;

	bool isConstant() const;

	bool isIntConstant() const;

	bool isBoolConstant() const;

	bool isFloatConstant() const;

	ASTExpression* getExpression() const;

	int getIntConstant() const;

	bool getBoolConstant() const;

	float getFloatConstant() const;

	explicit InitializeValue();

	explicit InitializeValue(ASTExpression* expression);

	explicit InitializeValue(int intConstant);

	explicit InitializeValue(float floatConstant);

	explicit InitializeValue(bool boolConstant);

	[[nodiscard]] Type* getExpressionType() const;

	std::string toString() const;
};

// AST 树节点, 定义了一些公共方法. 该类型自身不会出现于树中. 只能是
// ASTCompUnit, ASTDecl, ASTStmt
class ASTNode
{
	friend class Antlr2AstVisitor;

protected:
	// 树中父亲节点
	ASTNode* parent_ = nullptr;
	// 获取符号表, 只有 CompUnit 和 Block 才有符号表
	virtual ScopeTableInAST* getScopeTable();
	// 插入符号. 尝试向该节点插入符号, 若没有符号表则向上寻找; 若重名则返回 false; 否则插入并返回 true.
	bool pushScope(ASTDecl* decl);
	// 寻找符号. 尝试从该节点插入符号, 若没有符号表则向上寻找; 所有表均未找到则返回 false.
	ASTDecl* findScope(const std::string& id, bool isFunc);
	virtual void setParent(ASTNode* parent);

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
	ASTNode* getParent() const;
	// 寻找第一个具有特定性质的父节点, 也包括自己
	ASTNode* findParent(const std::function<bool(const ASTNode*)>& func);
};

// AST 树的根节点. 同时管理了整棵树的额外数据 *
class ASTCompUnit final : public ASTNode
{
public:
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
	std::vector<ASTVarDecl*> var_declarations_;
	// 函数声明 *
	std::vector<ASTFuncDecl*> func_declarations_;
	// 库函数, 在创建 ASTCompUnit 时自动加入. *
	std::vector<ASTFuncDecl*> lib_;
	// 符号表. 拥有全局符号. 包括库函数, 全局变量和函数 *
	TopScopeTableInAST* scopeTable_;

protected:
	ScopeTableInAST* getScopeTable() override;
};

// 声明, 该类型自身不会出现于树中. 只能是 ASTVarDecl 或 ASTFuncDecl
class ASTDecl : public ASTNode
{
	friend class ASTCompUnit;
	friend class Antlr2AstVisitor;

protected:
	// 声明 ID
	std::string id_;

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
	ASTVarDecl(bool is_const, bool is_global, Type* type)
		: is_const_(is_const),
		  is_global_(is_global),
		  type_(type)
	{
	}

	~ASTVarDecl() override;

private:
	friend class Antlr2AstVisitor;
	friend class ASTFuncDecl;

	// 是否是常量
	bool is_const_;
	// 是否是全局变量
	bool is_global_;
	// 变量的类型
	Type* type_;
	// 初始值列表, 没有则为空 *
	Tensor<InitializeValue>* initList_ = nullptr;
	// 需要被计算的非常量初始化表达式, 用于维护内存 *
	std::vector<ASTExpression*> expressions_;

public:
	// 是否是常量
	[[nodiscard]] bool isConst() const;
	// 是否是全局变量
	[[nodiscard]] bool isGlobal() const;
	// 获取初始化列表
	[[nodiscard]] const Tensor<InitializeValue>* getInitList() const;
	// 变量的类型
	[[nodiscard]] Type* getType() const;
};

// 函数声明, 有函数返回值, 参数声明
class ASTFuncDecl final : public ASTDecl
{
	std::list<std::string> toStringList() override;
	~ASTFuncDecl() override;
	friend class Antlr2AstVisitor;

public:
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
	bool in_lib_ = false;
	// 返回值类型
	Type* return_type_;
	// 参数列表, 参数由函数管理(虽然挂载在其基本块符号表中) *
	std::vector<ASTVarDecl*> args_;
	// 对应基本块 *
	ASTBlock* block_;

public:
	// 是否是库函数
	[[nodiscard]] bool isLibFunc() const;
};

// 语句. 该类型自身不会出现于树中.
// 只能是 ASTBlock, ASTAssign, ASTExpression, ASTIf, ASTWhile, ASTBreak, ASTReturn
class ASTStmt : public ASTNode
{
};

namespace AST
{
	// 裁剪表达式, 留下尽可能简介的包含等价副作用的表达式列表.
	std::list<ASTExpression*> cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
}

// 表达式, 该类型自身不会出现于树中. 只能是
// ASTCast, ASTMathExp, ASTCall, ASTNeg, ASTLVal, ASTMath2Logic,  ASTLogicExp, ASTEqual, ASTRelation, ASTNumber
class ASTExpression : public ASTStmt
{
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	friend class Antlr2AstVisitor;

protected:
	// 该表达式是否调用了函数
	bool haveFuncCall_;

public:
	// 表达式值的类型
	[[nodiscard]] virtual Type* getExpressionType() const = 0;
	// 获取两个表达式类型提升公共类型, 仅提供将 int 自动提升为 float
	// 如果无公共类型, 它会抛出异常
	static Type* maxType(const ASTExpression* a, const ASTExpression* b);

protected:
	// 转换表达式类型, 对于常量, 返回对应转换后常量
	// 对于表达式, 插入一个 ASTCast
	virtual ASTExpression* castTypeTo(Type* type);

public:
	// 寻找具有某类性质的第一个子表达式. 将父节点包含在内. 只会考虑同样为 ASTExpression 的子节点
	// 不会进入初始化列表搜索
	virtual ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) = 0;
};

// 左值表达式, 包含其引用的声明和使用的数组索引
class ASTLVal final : public ASTExpression
{
	std::list<std::string> toStringList() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	// 引用声明
	ASTVarDecl* decl_;
	// 索引, 若没有则为空 *
	std::vector<ASTExpression*> index_;

public:
	~ASTLVal() override;
	ASTLVal(ASTVarDecl* target, const std::vector<ASTExpression*>& index);

	[[nodiscard]] Type* getExpressionType() const override;
	// 引用声明
	[[nodiscard]] ASTVarDecl* getDeclaration() const;
	ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) override;
};

// 常数, 可以是 bool / float 或 int
class ASTNumber final : public ASTExpression
{
	std::list<std::string> toStringList() override;
	friend class Antlr2AstVisitor;
	friend class ASTCompUnit;

	union Field
	{
		int i_value_ = 0;
		float f_value_;
		bool b_value_;
	};

	Field field_;
	// 类型
	Type* type_ = Types::INT;

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
	ASTExpression* castTypeTo(Type* type) override;
	// 进行类型转换, 以变为符合 t(INT / FLOAT) 的常量, 作为数组初始值.
	// 只支持 int 转 float
	// 类型转换会转换其自身
	ASTNumber* toArrayInitValue(const Type* t);
	[[nodiscard]] Type* getExpressionType() const override;
	//转化为初始化张量数据
	[[nodiscard]] InitializeValue toInitializeValue() const;
	ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) override;
};

// 强制类型转换, 包含源表达式和转换后类型
class ASTCast final : public ASTExpression
{
	std::list<std::string> toStringList() override;
	~ASTCast() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);

public:
	ASTCast(ASTExpression* source, Type* cast_to);

	[[nodiscard]] Type* getExpressionType() const override;

	// 不能二次转换, 理论上不应该有这种冗余操作
	ASTExpression* castTypeTo(Type* type) override;
	ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) override;

private:
	// 源表达式 *
	ASTExpression* source_;
	// 目标类型
	Type* castTo_;
};

// 算数表达式, 包含左右操作数(同一类型)和算符
class ASTMathExp final : public ASTExpression
{
	std::list<std::string> toStringList() override;
	~ASTMathExp() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);

public:
	ASTMathExp(Type* result_type, MathOP op, ASTExpression* l, ASTExpression* r);

	[[nodiscard]] Type* getExpressionType() const override;
	ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) override;

private:
	// 结果类型
	Type* result_type_;
	// 算符
	MathOP op_;
	// 左操作数 *
	ASTExpression* l_;
	// 右操作数 *
	ASTExpression* r_;
};

// 逻辑表达式, 包含所有操作数(bool 类型)和算符
class ASTLogicExp final : public ASTExpression
{
	std::list<std::string> toStringList() override;
	~ASTLogicExp() override;

	enum LogicResult : uint8_t
	{
		UNDEFINE, TRUE, FALSE
	};

	friend class Antlr2AstVisitor;
	// 算符
	LogicOP op_;
	// 操作数 *
	std::vector<ASTExpression*> exps_;
	// 是否在编译期间就已经能够短路运算得到结果
	// 通常代表该表达式可以得到结果, 但是考虑到副作用对表达式进行保留; 会改变此后表达式的短路策略.
	LogicResult have_result_;

public:
	ASTLogicExp(LogicOP op, const std::vector<ASTExpression*>& exps, LogicResult haveResult): op_(op), exps_(exps),
		have_result_(haveResult)
	{
	}

	[[nodiscard]] Type* getExpressionType() const override;
	ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) override;
};

// 相等表达式, 包含所有操作数和算符, 区别于比较表达式, 其操作数可以全为 bool 类型
// AST 中结构化方法与文法不同, ASTEqual 下可能包含两个 OR 表达式.
// AST 保证这种嵌套只会在表达式最外层发生一次, 并且其下的 ASTLogicExp 一定是 UNDEFINE 状态.
class ASTEqual final : public ASTExpression
{
	std::list<std::string> toStringList() override;
	~ASTEqual() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	friend class Antlr2AstVisitor;

public:
	ASTEqual(bool op_equal, ASTExpression* l, ASTExpression* r)
		: op_equal_(op_equal),
		  l_(l),
		  r_(r)
	{
	}

private:
	friend class Antlr2AstVisitor;
	// 算符 true 为 ==; false 为 !=
	bool op_equal_;
	// 左操作数 *
	ASTExpression* l_;
	// 右操作数 *
	ASTExpression* r_;

public:
	[[nodiscard]] Type* getExpressionType() const override;
	ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) override;
};

// 比较表达式, 包含所有操作数和算符, 区别于相等表达式, 其操作数不能为 bool 类型
class ASTRelation final : public ASTExpression
{
	std::list<std::string> toStringList() override;
	~ASTRelation() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	friend class Antlr2AstVisitor;

public:
	ASTRelation(const RelationOP op, ASTExpression* l, ASTExpression* r)
		: op_(op),
		  l_(l),
		  r_(r)
	{
	}

private:
	friend class Antlr2AstVisitor;
	// 比较算符
	RelationOP op_;
	// 左操作数 *
	ASTExpression* l_;
	// 右操作数 *
	ASTExpression* r_;

public:
	[[nodiscard]] Type* getExpressionType() const override;
	ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) override;
};

// 函数调用, 包含函数声明和输入参数
class ASTCall final : public ASTExpression
{
	std::list<std::string> toStringList() override;
	~ASTCall() override;

public:
	ASTCall(ASTFuncDecl* function, const std::vector<ASTExpression*>& parameters);
	[[nodiscard]] Type* getExpressionType() const override;
	ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) override;

private:
	// 函数声明
	ASTFuncDecl* function_;
	// 参数 *
	// 对于数组类型, AST 的检查保证参数一定可以传入函数, 但不代表它们的类型相同
	// 例如函数参数是 integer[][2], 那它可能输入类型 integer[][2], integer[3][2]
	// 无论如何, 已经不需要额外检查保证其正确性
	std::vector<ASTExpression*> parameters_;
};

// 取负号表达式, 包含操作数. 该节点会自动简化
class ASTNeg final : ASTExpression
{
	std::list<std::string> toStringList() override;
	~ASTNeg() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	friend class Antlr2AstVisitor;
	// 操作数 *
	ASTExpression* hold_;

public:
	explicit ASTNeg(ASTExpression* hold);

	[[nodiscard]] Type* getExpressionType() const override;
	ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) override;
};

// 算数转逻辑表达式. 一般来说是 ! 运算导致. 两个 ! 会折叠. 包含符号和所含表达式
class ASTMath2Logic final : public ASTExpression
{
	std::list<std::string> toStringList() override;
	~ASTMath2Logic() override;
	friend std::list<ASTExpression*> AST::cutExpressionToOnlyLeaveFuncCall(ASTExpression* input);
	friend class Antlr2AstVisitor;

public:
	// 作为 not 被创建
	explicit ASTMath2Logic(ASTExpression* contained);
	[[nodiscard]] Type* getExpressionType() const override;
	ASTExpression* findChild(std::function<bool(const ASTExpression*)> func) override;

protected:
	// 所含表达式 *
	ASTExpression* contained_;
	// 是否进行 ! 运算
	bool not_;
};

// 基本块. 包含一个语句列表和一个符号表. *
class ASTBlock final : public ASTStmt
{
	std::list<std::string> toStringList() override;

	~ASTBlock() override;
	friend class Antlr2AstVisitor;
	friend class ASTFuncDecl;

protected:
	ScopeTableInAST* getScopeTable() override;

	// 符号表 *
	BlockScopeTableInAST* scope_table_ = new BlockScopeTableInAST();
	// 语句列表, 可以是 ASTStmt 或 ASTVarDecl, 考虑到初始化时使用的函数调用, 声明列表无法与语句列表独立 *
	std::vector<ASTNode*> stmts_;
	// 块是否为空, 具体来说, 其中不包含任何语句与任何声明
	[[nodiscard]] bool isEmpty() const;
};

// 赋值语句. 包含赋值目标左值和所赋值
class ASTAssign final : public ASTStmt
{
	std::list<std::string> toStringList() override;

	~ASTAssign() override;

public:
	ASTAssign(ASTLVal* assign_to, ASTExpression* assign_value);

private:
	// 目标左值 *
	ASTLVal* assign_to_;
	// 所赋值 *
	ASTExpression* assign_value_;
};

// IF 语句. 包含条件, IF 子句和 ELSE 子句(没有则为空)
class ASTIf final : public ASTStmt
{
	std::list<std::string> toStringList() override;
	~ASTIf() override;
	friend class Antlr2AstVisitor;

public:
	ASTIf(ASTExpression* cond) : cond_(cond)
	{
	}

private:
	// 条件 *
	ASTExpression* cond_;
	// if 子句, 可能有 [1, ] 个元素. 它们与 cond 处于同一块中, 这通常由于优化, 是无法在代码中常规体现的. *
	std::vector<ASTStmt*> if_stmt_;
	// else 子句, 可能有 [0, ] 个元素. 它们与 cond 处于同一块中 *
	std::vector<ASTStmt*> else_stmt_;
};

// WHILE 语句. 包含条件和子句
class ASTWhile final : public ASTStmt
{
	std::list<std::string> toStringList() override;
	~ASTWhile() override;
	friend class Antlr2AstVisitor;

public:
	ASTWhile(ASTExpression* cond) : cond_(cond)
	{
	}

private:
	// 条件 *
	ASTExpression* cond_;
	// 子句, 可能有 [0, ] 个元素. 它们与 cond 处于同一块中, 这通常由于优化, 是无法在代码中常规体现的. *
	std::vector<ASTStmt*> stmt_;
};

// BREAK 语句. 包含目标 While 节点
class ASTBreak final : public ASTStmt
{
	std::list<std::string> toStringList() override;
	friend class Antlr2AstVisitor;

public:
	ASTBreak(ASTWhile* target) : target_(target)
	{
	}

private:
	// 目标节点, 仅是为了方便定位, 同样只能跳出最近循环
	ASTWhile* target_;
};

// RETURN 语句. 包含返回值和目标返回函数
// 鉴于删除符号表的复杂性, 没有去除 RETURN 语句后代码
// 这考虑到即使去除代码, AST 也不得不读完全部代码来进行语义分析
class ASTReturn final : public ASTStmt
{
	std::list<std::string> toStringList() override;

	~ASTReturn() override;
	friend class Antlr2AstVisitor;

public:
	ASTReturn(ASTExpression* return_value, ASTFuncDecl* function) : return_value_(return_value), function_(function)
	{
	}

private:
	// 返回值, 与函数返回值类型一致, 故可能为 nullptr *
	ASTExpression* return_value_;
	// 目标函数, 仅是为了方便定位
	ASTFuncDecl* function_;
};

#pragma once
#include <cassert>
#include <map>
#include <string>
#include <vector>

class ASTContinue;
class ASTNode;
class ASTStmt;
class BasicBlock;
class ASTRVal;
class InitializeValue;
class Constant;
class Function;
class IRBuilder;
class ASTReturn;
class ASTBreak;
class ASTWhile;
class ASTIf;
class ASTAssign;
class ASTBlock;
class ASTNot;
class ASTNeg;
class ASTCall;
class ASTRelation;
class ASTEqual;
class ASTLogicExp;
class ASTMathExp;
class ASTCast;
class ASTNumber;
class ASTLVal;
class ASTFuncDecl;
class ASTVarDecl;
class Value;
class ASTCompUnit;
class Module;
template <typename Element>
class Tensor;

class Scope
{
public:
	// enter a new scope
	void enter() { inner.emplace_back(); }

	// exit a scope
	void exit() { inner.pop_back(); }

	[[nodiscard]] bool in_global() const { return inner.size() == 1; }

	// push a name to scope
	// return true if successful
	// return false if this name already exits
	bool push(const std::string& name, Value* val)
	{
		auto result = inner[inner.size() - 1].insert({name, val});
		return result.second;
	}


	// push a name to scope
	// return true if successful
	// return false if this name already exits
	bool pushFront(const std::string& name, Value* val)
	{
		auto result = inner[0].insert({name, val});
		return result.second;
	}

	Value* find(const std::string& name);

private:
	std::vector<std::map<std::string, Value*>> inner;
};

class AST2IRVisitor final
{
public:
	AST2IRVisitor(const AST2IRVisitor&) = delete;
	AST2IRVisitor(AST2IRVisitor&&) = delete;
	AST2IRVisitor& operator=(const AST2IRVisitor&) = delete;
	AST2IRVisitor& operator=(AST2IRVisitor&&) = delete;
	AST2IRVisitor();
	~AST2IRVisitor();

	[[nodiscard]] Module* getModule() const;

	Value* visit(const ASTCompUnit*);
	Value* visit(ASTVarDecl*);
	Value* visit(ASTFuncDecl*);
	Value* visit(ASTLVal*);
	Value* visit(ASTRVal*);
	Value* visit(ASTNumber*);
	Value* visit(ASTCast*);
	Value* visit(ASTMathExp*);
	Value* visit(ASTLogicExp*);
	Value* visit(ASTEqual*);
	Value* visit(ASTRelation*);
	Value* visit(ASTCall*);
	Value* visit(ASTNeg*);
	Value* visit(ASTNot*);
	Value* visit(ASTBlock*);
	Value* visit(ASTAssign*);
	Value* visit(ASTIf*);
	Value* visit(ASTWhile*);
	Value* visit(ASTBreak*);
	Value* visit(ASTContinue*);
	Value* visit(ASTReturn*);

	// 访问一个语句序列, 如果这个语句序列无论如何都会在中间退出, 则返回退出理由, 否则返回 NONE
	void visitStmts(const std::vector<ASTStmt*>& vec);
	// 访问一个语句序列, 如果这个语句序列无论如何都会在中间退出, 则返回退出理由, 否则返回 NONE
	void visitStmts(const std::vector<ASTNode*>& vec);

	std::string createPrivateGlobalVarID();

private:
	// *
	IRBuilder* _builder;
	// 变量符号表
	Scope _var_scope;
	// 函数符号表
	Scope _func_scope;
	// 与 AST2IR 独立
	Module* _module;

	// 用于辅助构建 IR
	Function* _functionBelong = nullptr;
	// 对 bool 的处理:
	// 若某节点孩子是 bool 类型, 其预先创建 false/true 块, 并连接好它们.
	// 在之前的基本块, 孩子创建 br 指令; 然后定位到之后基本块插入其它命令.
	std::vector<BasicBlock*> _false_targets;
	std::vector<BasicBlock*> _true_targets;
	std::vector<BasicBlock*> _while_nexts;
	std::vector<BasicBlock*> _while_conds;

	int _globalInitIdx = 0;

	// 根据形状 shape 将 index 转换为结构化的数组索引. paddings: 前面填充 0 的个数
	[[nodiscard]] std::vector<Value*> index(const std::vector<int>& shape, int idx, int paddings = 0) const;
};

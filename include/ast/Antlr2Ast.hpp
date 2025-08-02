#pragma once
#include <queue>

#include "SysYBaseVisitor.h"

class ASTFuncDecl;
class HaveScope;
class ASTDecl;
class ASTWhile;
class ASTCompUnit;
class Type;
class ASTNode;
class ASTVarDecl;
class InitializeValue;
template<typename T>
class TensorData;

class Antlr2AstVisitor final : SysYBaseVisitor
{
	// 定义约束, 用于使父节点为子节点的语义添加限制. 约束调用要求谁插入, 谁弹出

	// 为定义指明其基本类型的约束, 是 Float 或 Integer
	std::stack<Type*> _declarationTypeConstraint;
	// 为初始化的约束, 标记初始化张量
	std::stack<TensorData<InitializeValue>*> _initTensorConstraint;
	// 为初始化的约束, 标记初始化节点, 用于列表中维护 ASTExpression 的内存
	std::stack<ASTVarDecl*> _initVarNodeConstraint;
	// 结构约束, 目前的前置节点. 这些节点是拥有符号表的节点
	std::vector<HaveScope*> _structConstraint;
	// 结构约束, 目前的循环节点. 用于 break 等
	std::stack<ASTWhile*> _whileConstraint;
	// 目前所在函数
	ASTFuncDecl* _currentFunction = nullptr;

	std::queue<ASTNode*> returnSlot_;
	Type* typeReturnSlot_ = nullptr;
	// 逻辑约束, 表达式是否允许使用 ! 运算
	bool _allowLogic = false;

	// 插入符号. 尝试在 _structConstraint 最右侧插入符号, 若重名则返回 false; 否则插入并返回 true.
	bool pushScope(ASTDecl* decl) const;
	// 寻找符号. 尝试在 _structConstraint 从右往左寻找符号; 所有表均未找到则返回 false.
	ASTDecl* findScope(const std::string& id, bool isFunc);

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

	ASTNode* poll()
	{
		auto ret = returnSlot_.front();
		returnSlot_.pop();
		return ret;
	}

public:
	ASTCompUnit* astTree(antlr4::tree::ParseTree* parseTree);
};

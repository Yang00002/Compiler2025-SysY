#ifndef SYSY_IR_GENERATOR_HPP
#define SYSY_IR_GENERATOR_HPP

#include "IR/IR.hpp"
#include "antlr/SysYVisitor.h"

#include <any>
#include <cstddef>
#include <string>
#include <map>
#include <memory>
#include <tree/ParseTreeType.h>
#include <vector>

#include "antlr4-runtime.h"


class Scope {
    std::vector<std::map<std::string, Value*>> symtable;
public:
    void enter() { symtable.emplace_back(); }
    void exit() { symtable.pop_back(); }
    bool in_global_scope() { return symtable.size()==1; }
    bool add_symbol(const std::string& name, Value* val) // returns false if name already exists
    {
        auto result = symtable[symtable.size()-1].emplace(name, val);
        return result.second;
    }
    Value* find(const std::string& name) 
    {
        for (auto s = symtable.rbegin(); s != symtable.rend(); s++) {
            auto iter = s->find(name);
            if (iter != s->end())
                return iter->second;
        }
        return nullptr; // name not found
    }
};


class SysYIRGenerator : public SysYVisitor {
public:
    SysYIRGenerator() {
        module = std::make_unique<Module>();
        builder = std::make_unique<IRBuilder>(nullptr,module.get());
    }
    std::unique_ptr<Module> get_module() { return std::move(module); }
    
    // 语义分析需要跟踪的信息
    std::vector<BasicBlock*> true_targets;
    std::vector<BasicBlock*> false_targets;

private:
    std::unique_ptr<IRBuilder> builder;
    std::unique_ptr<Module> module;
    Scope scope;

public:
    // 访问者模式
    std::any visitBType(SysYParser::BTypeContext *context) override;
    std::any visitCompUnit(SysYParser::CompUnitContext* ctx) override;
    std::any visitDecl(SysYParser::DeclContext* ctx) override;
    std::any visitConstDecl(SysYParser::ConstDeclContext* ctx) override;
    std::any visitConstDef(SysYParser::ConstDefContext* ctx) override;
    std::any visitConstInitVal(SysYParser::ConstInitValContext* ctx) override;
    std::any visitVarDecl(SysYParser::VarDeclContext* ctx) override;
    std::any visitVarDef(SysYParser::VarDefContext* ctx) override;
    std::any visitInitVal(SysYParser::InitValContext* ctx) override;
    std::any visitFuncType(SysYParser::FuncTypeContext *context) override;
    std::any visitFuncDef(SysYParser::FuncDefContext* ctx) override;
    std::any visitFuncFParams(SysYParser::FuncFParamsContext* ctx) override;
    std::any visitFuncFParam(SysYParser::FuncFParamContext* ctx) override;
    std::any visitBlock(SysYParser::BlockContext* ctx) override;
    std::any visitBlockItem(SysYParser::BlockItemContext* ctx) override;
    std::any visitStmt(SysYParser::StmtContext* ctx) override;
    std::any visitExp(SysYParser::ExpContext* ctx) override;
    std::any visitCond(SysYParser::CondContext* ctx) override;
    std::any visitLVal(SysYParser::LValContext* ctx) override;
    std::any visitPrimaryExp(SysYParser::PrimaryExpContext* ctx) override;
    std::any visitNumber(SysYParser::NumberContext* ctx) override;
    std::any visitUnaryExp(SysYParser::UnaryExpContext* ctx) override;
    std::any visitUnaryOp(SysYParser::UnaryOpContext* ctx) override;
    std::any visitFuncRParams(SysYParser::FuncRParamsContext* ctx) override;
    std::any visitMulExp(SysYParser::MulExpContext* ctx) override;
    std::any visitAddExp(SysYParser::AddExpContext* ctx) override;
    std::any visitRelExp(SysYParser::RelExpContext* ctx) override;
    std::any visitEqExp(SysYParser::EqExpContext* ctx) override;
    std::any visitLAndExp(SysYParser::LAndExpContext* ctx) override;
    std::any visitLOrExp(SysYParser::LOrExpContext* ctx) override;
    std::any visitConstExp(SysYParser::ConstExpContext* ctx) override;
};

#endif
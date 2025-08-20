# pragma once

#include "PassManager.hpp"
#include "Instruction.hpp"
#include "Value.hpp"
#include "Type.hpp"
#include "Constant.hpp"
#include "BasicBlock.hpp"

#include <iostream>
#include <set>
#include <map>
#include <memory>
#include <vector>

#define IS_UNARY(i) ((i->is_fp2si()||i->is_si2fp()||i->is_zext())&&(i->get_num_operand()==1))
#define IS_BINARY(i) ((i->isBinary())||((i->is_cmp()||i->is_fcmp())&&(i->get_num_operand()==2)))

struct ValStatus{
    /*
     *  INIT: 一个表达式的初始值为该状态，尚未被定值
     *  CONST: 已定为常值
     *  UNDEF: 值未访问或不确定 | NAC: 已确定不是常值
     */
    enum Status { NAC=0, UNDEF=1, CONST=2, INIT=3 }; 
    Status status_{UNDEF};
    Constant* const_val_{nullptr};
    
    bool not_const() const { return status_==Status::NAC; }
    bool is_const() const { return status_==Status::CONST; }
    bool not_defined() const { return status_==Status::UNDEF; }

    std::string print() const {
        return  status_ == Status::INIT ? "not evaluated" :
                status_ == Status::CONST ? "const" : 
                status_ == Status::UNDEF ? "undefined" : "not const";
    }

    bool operator != (ValStatus& v) const {
        if(status_!=v.status_)
            return true;
        if(status_!=Status::CONST)
            return false;
        if( const_val_->isIntConstant() ){
            auto x = const_val_->getIntConstant();
            auto y = v.const_val_->getIntConstant();
            return x != y;
        } else if( const_val_->isBoolConstant() ){
            auto x = const_val_->getBoolConstant();
            auto y = v.const_val_->getBoolConstant();
            return x != y;
        } else if( const_val_->isFloatConstant() ){
            auto x = const_val_->getFloatConstant();
            auto y = v.const_val_->getFloatConstant();
            return x != y;
        } 
        return false;
    }

    void operator &= (const ValStatus& v) { // 交操作(最大下界)
        if( v.status_ < status_ ){
            status_ = v.status_;
            const_val_ = v.const_val_;
        } else if ( v.status_==status_ && status_==CONST ) {
            if( const_val_->isIntConstant() ){
                auto x = const_val_->getIntConstant();
                auto y = v.const_val_->getIntConstant();
                if( x != y ){
                    status_ = NAC;
                    const_val_ = nullptr;
                }
            } else if( const_val_->isFloatConstant() ){
                auto x = const_val_->getFloatConstant();
                auto y = v.const_val_->getFloatConstant();
                if( x != y ){
                    status_ = NAC;
                    const_val_ = nullptr;
                }
            } else if( const_val_->isBoolConstant() ){
                auto x = const_val_->getBoolConstant();
                auto y = v.const_val_->getBoolConstant();
                if( x != y ){
                    status_ = NAC;
                    const_val_ = nullptr;
                }
            } 
        }
    }
};

class ValueMap {
  public:
    void clear() { value_map_.clear(); }
    ValStatus get(Value* key) {
        if (auto constant = dynamic_cast<Constant*>(key))
            value_map_[key] = {ValStatus::CONST, constant};
        return value_map_[key];
    }
    void set(Value* key, ValStatus value) { value_map_[key] = value; }

  private:
    std::map<Value*, ValStatus> value_map_;
};

class SCCPVisitor;
class SCCP final : public Pass
{
    ValueMap value_map;
    std::set<std::pair<BasicBlock*, BasicBlock*>> visited;
    std::vector<std::pair<BasicBlock*, BasicBlock*>> flow_worklist; // first->second由控制流可达
    std::vector<Instruction*> value_worklist;
    std::unique_ptr<SCCPVisitor> visitor_;

public:
	SCCP(const SCCP&) = delete;
	SCCP(SCCP&&) = delete;
	SCCP& operator=(const SCCP&) = delete;
	SCCP& operator=(SCCP&&) = delete;
	explicit SCCP(PassManager* mng,Module* m) : Pass(mng, m) { visitor_ = std::make_unique<SCCPVisitor>(*this); }
	~SCCP() override = default;

	void run() override;
    void run(Function* f);
    ValueMap& get_map() { return value_map; }
    ValStatus get_mapped_val(Value* key) { return value_map.get(key); }

    void replace_with_constant(Function* f);
	void convert_cond_br(Instruction* i, BasicBlock* target, BasicBlock* invalid) const;

    auto& get_visited() { return visited; }
    auto& get_flow_worklist() { return flow_worklist; }
    auto& get_value_worklist() { return value_worklist; }

    // 常量折叠
    Constant* constFold(Instruction* i);
    Constant* constFold(const Instruction* i, const Constant* op1, const Constant* op2) const;
    Constant* constFold(const Instruction* i, const Constant* op) const;
};

class SCCPVisitor {
public:
    explicit SCCPVisitor(SCCP &sccp_pass)
        : sccp(sccp_pass), val_map(sccp_pass.get_map()),
          flow_worklist(sccp_pass.get_flow_worklist()),
          value_worklist(sccp_pass.get_value_worklist()) {}
    void visit(Instruction* inst);

private:
    void visit_br(const BranchInst* inst); // 仅检查基本块的可达性
    void visit_phi(PhiInst* inst);
    void visit_fold(Instruction* inst);

    SCCP& sccp;
    ValueMap& val_map;
    std::vector<std::pair<BasicBlock *, BasicBlock *>>& flow_worklist;
    std::vector<Instruction *>& value_worklist;

    Instruction* inst_;
    BasicBlock* bb_;
    ValStatus prev_;
    ValStatus cur_;
};
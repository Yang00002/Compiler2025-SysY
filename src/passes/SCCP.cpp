#include "SCCP.hpp"

#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "Color.hpp"
#include "BasicBlock.hpp"
#include "Function.hpp"
#include "GlobalVariable.hpp"
#include "Instruction.hpp"
#include "Module.hpp"
#include "Value.hpp"

#define DEBUG 0

#if DEBUG == 1
namespace
{
	namespace debug
	{
		std::string tab_counts;

		void _0__counts_pop()
		{
			if (!tab_counts.empty()) tab_counts.pop_back();
		}

		void _0__counts_push()
		{
			tab_counts += ' ';
		}

		void _0__log_if_func(const std::string& str, const bool cond)
		{
			if (cond)
				std::cout << tab_counts << str << '\n';
		}

		void _0__log_func(const std::string& str)
		{
			std::cout << tab_counts << str << '\n';
		}

		void _0__gap_func()
		{
			std::cout << "==============================\n";
		}
	}
}

#define LOG(a) debug::_0__log_func(a)
#define LOGIF(a,b) debug::_0__log_if_func((a), (b))
#define GAP debug::_0__gap_func()
#define PUSH debug::_0__counts_push()
#define RUN(a) (a)
#define POP debug::_0__counts_pop()
#endif
#if DEBUG != 1
#define LOG(a)
#define LOGIF(a,b)
#define GAP
#define RUN(a)
#define PUSH
#define POP
#endif

void SCCP::run()
{
    LOG(color::blue("Run SCCP Pass"));
	// 以函数为单元遍历实现 SCCP 算法
    m_->set_print_name();
	for (const auto f : m_->get_functions())
	{
		if (f->is_declaration()||f->is_lib_)
			continue;
		run(f);
	}
    GAP;
    LOG(m_->print());
    GAP;
    LOG(color::blue("SCCP Done"));
}

void SCCP::run(Function* f)
{
    GAP;
    LOG(color::green("Run SCCP on function "+f->get_name()));
	value_map.clear();
    visited.clear();
    flow_worklist.clear();
    value_worklist.clear();
    flow_worklist.emplace_back(nullptr,f->get_entry_block());
    for(auto bb:f->get_basic_blocks()) {
        auto& args = f->get_args();
        for(auto i:bb->get_instructions()) {
            if(dynamic_cast<GlobalVariable*>(i)||i->is_call())
                value_map.set(i,{ValStatus::NAC,nullptr});
            else
                value_map.set(i,{ValStatus::UNDEF,nullptr});
            for(auto v:i->get_operands()){
                if(auto gv = dynamic_cast<GlobalVariable*>(v))
                    value_map.set(v,{ValStatus::NAC,nullptr});
            }
        }
    }

    unsigned value_id = 0;
    unsigned flow_id = 0;
    while( value_id < value_worklist.size() || flow_id < flow_worklist.size() ){
        // 计算控制流可达的基本块
        while( flow_id < flow_worklist.size() ){
            BasicBlock *pre=nullptr, *cur=nullptr;
            std::tie(pre,cur) = flow_worklist[flow_id++];
            LOG(color::pink("Visiting basic block "+std::to_string(flow_id)+
            "/"+std::to_string(flow_worklist.size())));
            if(visited.count({pre,cur}))
                continue;
            visited.insert({pre,cur});
            for(auto i:cur->get_instructions())
                visitor_->visit(i);
        }
        // 对每个基本块计算常量
        while( value_id < value_worklist.size() ){
            auto inst = value_worklist[value_id++];
            auto bb = inst->get_parent();
            for(auto pre:bb->get_pre_basic_blocks()){
                if(visited.count({pre,bb})){
                    visitor_->visit(inst);
                    break;
                }
            }
        }
    }
    // 常量传播
    replace_with_constant(f);
}

Constant* SCCP::constFold(Instruction* i, Constant* op1, Constant* op2)
{
    auto op = i->get_instr_type();
    switch (op) {
    case Instruction::add: 
        return Constant::create(m_,op1->getIntConstant()+op2->getIntConstant());
    case Instruction::sub: 
        return Constant::create(m_,op1->getIntConstant()-op2->getIntConstant());
    case Instruction::mul: 
        return Constant::create(m_,op1->getIntConstant()*op2->getIntConstant());
    case Instruction::sdiv: 
        return Constant::create(m_,op1->getIntConstant()/op2->getIntConstant());
    case Instruction::srem: 
        return Constant::create(m_,op1->getIntConstant()%op2->getIntConstant());
    case Instruction::lt:
        return Constant::create(m_,op1->getIntConstant()<op2->getIntConstant());
    case Instruction::le:
        return Constant::create(m_,op1->getIntConstant()<=op2->getIntConstant());
    case Instruction::gt:
        return Constant::create(m_,op1->getIntConstant()>op2->getIntConstant());
    case Instruction::ge:
        return Constant::create(m_,op1->getIntConstant()>=op2->getIntConstant());
    case Instruction::eq:
        return Constant::create(m_,op1->getIntConstant()==op2->getIntConstant());
    case Instruction::ne:
        return Constant::create(m_,op1->getIntConstant()!=op2->getIntConstant());
    case Instruction::fadd: 
        return Constant::create(m_,op1->getFloatConstant()+op2->getFloatConstant());
    case Instruction::fsub:
        return Constant::create(m_,op1->getFloatConstant()-op2->getFloatConstant());
    case Instruction::fmul:
        return Constant::create(m_,op1->getFloatConstant()*op2->getFloatConstant());
    case Instruction::fdiv:
        return Constant::create(m_,op1->getFloatConstant()/op2->getFloatConstant());
    case Instruction::flt:
        return Constant::create(m_,op1->getFloatConstant()<op2->getFloatConstant());
    case Instruction::fle:
        return Constant::create(m_,op1->getFloatConstant()<=op2->getFloatConstant());
    case Instruction::fgt:
        return Constant::create(m_,op1->getFloatConstant()>op2->getFloatConstant());
    case Instruction::fge:
        return Constant::create(m_,op1->getFloatConstant()>=op2->getFloatConstant());
    case Instruction::feq:
        return Constant::create(m_,op1->getFloatConstant()==op2->getFloatConstant());
    case Instruction::fne:
        return Constant::create(m_,op1->getFloatConstant()!=op2->getFloatConstant());
    default:
        return nullptr;
    }
}

Constant* SCCP::constFold(Instruction* i, Constant* op)
{
    auto ty = i->get_instr_type();
    switch (ty) {
    case Instruction::zext:
        return Constant::create(m_,op->getIntConstant());
    case Instruction::sitofp:
        return Constant::create(m_,(float)op->getIntConstant());
    case Instruction::fptosi:
        return Constant::create(m_,(int)op->getFloatConstant());
    default:
        return nullptr;
    }
}

void SCCP::replace_with_constant(Function* f)
{    
    std::vector<Instruction *> del_;
    for (auto b : f->get_basic_blocks()) {
        for (auto i : b->get_instructions()) {
            if (auto c = get_mapped_val(i).const_val_) {
                i->replace_all_use_with(c);
                del_.emplace_back(i);
            }
        }
    }
    auto c = del_.size();
    if(c!=0)
        LOG(color::cyan("Replaced "+std::to_string(c)+" use(s) with constant(s)"));
    for (auto d : del_)
        d->get_parent()->erase_instr(d);

    // 处理条件跳转
    for (auto b : f->get_basic_blocks()) {
        auto br = dynamic_cast<BranchInst*>(b->get_terminator_or_null());
        if (!br||!br->is_cond_br())
            continue;
        auto cond = br->get_operand(0);
        auto cond_s = value_map.get(cond);
        if (!cond_s.is_const())
            continue;
        auto true_bb = static_cast<BasicBlock*>(br->get_operand(1));
        auto false_bb = static_cast<BasicBlock*>(br->get_operand(2));
        if (cond_s.const_val_->getBoolConstant())
            convert_cond_br(br, true_bb, false_bb);
        else
            convert_cond_br(br, false_bb, true_bb);
    }
}

void SCCP::convert_cond_br(Instruction* i, BasicBlock* target, BasicBlock* invalid)
{
    LOG(color::cyan("Visiting branch: ")+i->print());
    auto br = static_cast<BranchInst*>(i);
    auto bb = br->get_parent();
    br->remove_all_operands();
    br->add_operand(target);
    if(target==invalid)
        return;
    /* 此处删除前驱/后继有bug
    bb->remove_succ_basic_block(invalid);
    invalid->remove_pre_basic_block(bb);
    */
}

void SCCPVisitor::visit(Instruction* i)
{
    inst_ = i;
    bb_ = i->get_parent();
    prev_ = val_map.get(i);
    cur_ = prev_;

    if(i->is_br())
        visit_br(static_cast<BranchInst*>(i));
    else if(i->is_phi())
        visit_phi(static_cast<PhiInst*>(i));
    else if(IS_BINARY(i)||IS_UNARY(i))
        visit_fold(i);
    else
        cur_ = {ValStatus::NAC, nullptr};

    if( cur_ != prev_ ){
        val_map.set(i,cur_);
        for(auto u:i->get_use_list()){
            if(auto inst=dynamic_cast<Instruction*>(u.val_))
                value_worklist.emplace_back(inst);
        }
    }
}

void SCCPVisitor::visit_fold(Instruction* i)
{
    LOG(color::cyan("Visiting unary/binary: ")+i->print());
    auto fold = sccp.constFold(i);
    if(fold){
        cur_ = {ValStatus::CONST, fold};
        LOG(color::green(i->get_name())+" is const with value "
            + ( fold->get_type()==Types::INT ? std::to_string(fold->getIntConstant()) :
                fold->get_type()==Types::BOOL ? (fold->getBoolConstant() ? "True" : "False") :
                std::to_string(fold->getFloatConstant())));
        return;
    }
    cur_ = {ValStatus::INIT,nullptr};
    for(auto op:i->get_operands()){
        auto status = val_map.get(op);
        LOG("current status: "+cur_.print()+" | "+color::yellow((op->get_name()=="" ? op->print() : op->get_name()))
            +" is "+ status.print());
        cur_ &= status;
        if(status.not_const()){
            LOG(color::green(i->get_name())+" is "+ cur_.print());
            return;
        }
    }
    LOG(color::green(i->get_name())+" is "+ cur_.print());
}

void SCCPVisitor::visit_phi(PhiInst* i)
{
    LOG(color::cyan("Visiting Phi: ")+i->print());
    cur_ = {ValStatus::INIT,nullptr};
    for(int j=1;j<i->get_num_operand();j+=2){
        auto pre_bb = static_cast<BasicBlock*>(i->get_operand(j));
        auto op = i->get_operand(j-1);
        auto status = val_map.get(op);
        LOG("current status: "+cur_.print()+" | "+color::yellow((op->get_name()=="" ? op->print() : op->get_name()))
            +" is "+ status.print());
        cur_ &= status;
        if(status.not_const()){
            LOG(color::green(i->get_name())+" is "+ cur_.print());
            return;
        }
    }
    LOG(color::green(i->get_name())+" is "+ cur_.print());
}

void SCCPVisitor::visit_br(BranchInst* i)
{
    if(!i->is_cond_br()){
        flow_worklist.emplace_back(bb_, static_cast<BasicBlock *>(i->get_operand(0)));
        return;
    }
    auto true_bb = static_cast<BasicBlock*>(i->get_operand(1));
    auto false_bb = static_cast<BasicBlock*>(i->get_operand(2));
    auto const_cond = val_map.get(i->get_operand(0)).const_val_;
    if (const_cond) {
        const_cond->getBoolConstant() ? flow_worklist.emplace_back(bb_, true_bb)
                                      : flow_worklist.emplace_back(bb_, false_bb);
    } else {
        flow_worklist.emplace_back(bb_, true_bb);
        flow_worklist.emplace_back(bb_, false_bb);
    }
}

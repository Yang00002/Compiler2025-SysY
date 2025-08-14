#include "SCCP.hpp"

#include <memory>
#include <queue>
#include <set>
#include <vector>

#include "BasicBlock.hpp"
#include "Function.hpp"
#include "GlobalVariable.hpp"
#include "Instruction.hpp"
#include "Module.hpp"
#include "Value.hpp"

#define DEBUG 0
#include "Util.hpp"

void SCCP::run()
{
	LOG(color::blue("Run SCCP Pass"));
	// 以函数为单元遍历实现 SCCP 算法
	m_->set_print_name();
	for (const auto f : m_->get_functions())
	{
		if (f->is_declaration() || f->is_lib_)
			continue;
		run(f);
	}
	PASS_SUFFIX;
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
	flow_worklist.emplace_back(nullptr, f->get_entry_block());
	for (auto bb : f->get_basic_blocks())
	{
		for (auto i : bb->get_instructions())
		{
			if (dynamic_cast<GlobalVariable*>(i) || i->is_call())
				value_map.set(i, {ValStatus::NAC, nullptr});
			else
				value_map.set(i, {ValStatus::UNDEF, nullptr});
			for (auto v : i->get_operands())
			{
				if (dynamic_cast<GlobalVariable*>(v))
					value_map.set(v, {ValStatus::NAC, nullptr});
			}
		}
	}

	int value_id = 0;
	int flow_id = 0;
	while (value_id < u2iNegThrow(value_worklist.size()) || flow_id < u2iNegThrow(flow_worklist.size()))
	{
		// 计算控制流可达的基本块
		while (flow_id < u2iNegThrow(flow_worklist.size()))
		{
			BasicBlock *pre = nullptr, *cur = nullptr;
			std::tie(pre, cur) = flow_worklist[flow_id++];
			LOG(color::pink("Visiting basic block "+std::to_string(flow_id)+
				"/"+std::to_string(flow_worklist.size())));
			if (visited.count({pre, cur}))
				continue;
			visited.insert({pre, cur});
			for (auto i : cur->get_instructions())
				visitor_->visit(i);
		}
		// 对每个基本块计算常量
		while (value_id < u2iNegThrow(value_worklist.size()))
		{
			auto inst = value_worklist[value_id++];
			auto bb = inst->get_parent();
			for (auto pre : bb->get_pre_basic_blocks())
			{
				if (visited.count({pre, bb}))
				{
					visitor_->visit(inst);
					break;
				}
			}
		}
	}
	// 常量传播
	replace_with_constant(f);
}

Constant* SCCP::constFold(Instruction* i) {
	if (IS_BINARY(i)) {
		auto v1 = get_mapped_val(i->get_operand(0));
		auto v2 = get_mapped_val(i->get_operand(1));
		if (v1.const_val_ && v2.const_val_) {
			ASSERT(v1.is_const() && v2.is_const() && "Trying to fold variables");
			return constFold(i, v1.const_val_, v2.const_val_);
		}
	}
	else if (IS_UNARY(i)) {
		auto v = get_mapped_val(i->get_operand(0));
		if (v.const_val_) {
			ASSERT(v.is_const() && "Trying to fold variables");
			return constFold(i, v.const_val_);
		}
	}
	return nullptr;
}

Constant* SCCP::constFold(const Instruction* i, const Constant* op1, const Constant* op2) const
{
	switch (i->get_instr_type()) // NOLINT(clang-diagnostic-switch-enum)
	{
		case Instruction::add:
			return Constant::create(m_, op1->getIntConstant() + op2->getIntConstant());
		case Instruction::sub:
			return Constant::create(m_, op1->getIntConstant() - op2->getIntConstant());
		case Instruction::mul:
			return Constant::create(m_, op1->getIntConstant() * op2->getIntConstant());
		case Instruction::sdiv:
			return Constant::create(m_, op1->getIntConstant() / op2->getIntConstant());
		case Instruction::srem:
			return Constant::create(m_, op1->getIntConstant() % op2->getIntConstant());
		case Instruction::shl:
			return Constant::create(m_, op1->getIntConstant() << op2->getIntConstant());
		case Instruction::ashr:
			return Constant::create(m_,  op1->getIntConstant() >> op2->getIntConstant());
		case Instruction::and_:
			return Constant::create(m_, op1->getIntConstant() & op2->getIntConstant());
		case Instruction::lt:
			return Constant::create(m_, op1->getIntConstant() < op2->getIntConstant());
		case Instruction::le:
			return Constant::create(m_, op1->getIntConstant() <= op2->getIntConstant());
		case Instruction::gt:
			return Constant::create(m_, op1->getIntConstant() > op2->getIntConstant());
		case Instruction::ge:
			return Constant::create(m_, op1->getIntConstant() >= op2->getIntConstant());
		case Instruction::eq:
			return Constant::create(m_, op1->getIntConstant() == op2->getIntConstant());
		case Instruction::ne:
			return Constant::create(m_, op1->getIntConstant() != op2->getIntConstant());
		case Instruction::fadd:
			return Constant::create(m_, op1->getFloatConstant() + op2->getFloatConstant());
		case Instruction::fsub:
			return Constant::create(m_, op1->getFloatConstant() - op2->getFloatConstant());
		case Instruction::fmul:
			return Constant::create(m_, op1->getFloatConstant() * op2->getFloatConstant());
		case Instruction::fdiv:
			return Constant::create(m_, op1->getFloatConstant() / op2->getFloatConstant());
		case Instruction::flt:
			return Constant::create(m_, op1->getFloatConstant() < op2->getFloatConstant());
		case Instruction::fle:
			return Constant::create(m_, op1->getFloatConstant() <= op2->getFloatConstant());
		case Instruction::fgt:
			return Constant::create(m_, op1->getFloatConstant() > op2->getFloatConstant());
		case Instruction::fge:
			return Constant::create(m_, op1->getFloatConstant() >= op2->getFloatConstant());
		case Instruction::feq:
			return Constant::create(m_, op1->getFloatConstant() == op2->getFloatConstant());
		case Instruction::fne:
			return Constant::create(m_, op1->getFloatConstant() != op2->getFloatConstant());
		default:
			return nullptr;
	}
}

Constant* SCCP::constFold(const Instruction* i, const Constant* op) const
{
	switch (i->get_instr_type()) // NOLINT(clang-diagnostic-switch-enum)
	{
		case Instruction::zext:
			return Constant::create(m_, op->getIntConstant());
		case Instruction::sitofp:
			return Constant::create(m_, static_cast<float>(op->getIntConstant()));
		case Instruction::fptosi:
			return Constant::create(m_, static_cast<int>(op->getFloatConstant()));
		default:
			return nullptr;
	}
}

void SCCP::replace_with_constant(Function* f)
{
	std::vector<Instruction*> del_;
	for (auto b : f->get_basic_blocks())
	{
		for (auto i : b->get_instructions())
		{
			if (auto c = get_mapped_val(i).const_val_)
			{
				i->replace_all_use_with(c);
				del_.emplace_back(i);
			}
		}
	}
	auto c = del_.size();

	if (c != 0)
	{
		LOG(color::cyan("Replaced "+std::to_string(c)+" use(s) with constant(s)"));
	}

	for (auto d : del_)
	{
		d->get_parent()->erase_instr(d);
		delete d;
	}

	// 处理条件跳转
	for (auto b : f->get_basic_blocks())
	{
		auto br = dynamic_cast<BranchInst*>(b->get_terminator_or_null());
		if (!br || !br->is_cond_br())
			continue;
		auto cond = br->get_operand(0);
		auto cond_s = value_map.get(cond);
		if (!cond_s.is_const())
			continue;
		auto true_bb = dynamic_cast<BasicBlock*>(br->get_operand(1));
		auto false_bb = dynamic_cast<BasicBlock*>(br->get_operand(2));
		if (cond_s.const_val_->getBoolConstant())
			convert_cond_br(br, true_bb, false_bb);
		else
			convert_cond_br(br, false_bb, true_bb);
	}
}

void SCCP::convert_cond_br(Instruction* i, BasicBlock* target, BasicBlock* invalid)
{
	LOG(color::cyan("Visiting branch: ")+i->print());
	auto br = dynamic_cast<BranchInst*>(i);
	br->remove_all_operands();
	br->add_operand(target);
	auto bb = br->get_parent();
	if (target == invalid) return;
	bb->remove_succ_basic_block(invalid);
	invalid->remove_pre_basic_block(bb);
	for (auto phi : invalid->get_instructions().phi_and_allocas())
	{
		auto p = dynamic_cast<PhiInst*>(phi);
		p->remove_phi_operand(bb);
	}
}

void SCCPVisitor::visit(Instruction* i)
{
	inst_ = i;
	bb_ = i->get_parent();
	prev_ = val_map.get(i);
	cur_ = prev_;

	if (i->is_br())
		visit_br(dynamic_cast<BranchInst*>(i));
	else if (i->is_phi())
		visit_phi(dynamic_cast<PhiInst*>(i));
	else if (IS_BINARY(i) || IS_UNARY(i))
		visit_fold(i);
	else
		cur_ = {ValStatus::NAC, nullptr};

	if (cur_ != prev_)
	{
		val_map.set(i, cur_);
		for (auto u : i->get_use_list())
		{
			if (auto inst = dynamic_cast<Instruction*>(u.val_))
				value_worklist.emplace_back(inst);
		}
	}
}

void SCCPVisitor::visit_fold(Instruction* i)
{
	LOG(color::cyan("Visiting unary/binary: ")+i->print());
	if (auto fold = sccp.constFold(i))
	{
		cur_ = {ValStatus::CONST, fold};
		LOG(color::green(i->get_name())+" is const with value "
			+ ( fold->get_type()==Types::INT ? std::to_string(fold->getIntConstant()) :
				fold->get_type()==Types::BOOL ? (fold->getBoolConstant() ? "True" : "False") :
				std::to_string(fold->getFloatConstant())));
		return;
	}
	cur_ = {ValStatus::INIT, nullptr};
	for (auto op : i->get_operands())
	{
		auto status = val_map.get(op);
		LOG("current status: "+cur_.print()+" | "+color::yellow((op->get_name().empty() ? op->print() : op->get_name()))
			+" is "+ status.print());
		cur_ &= status;
		if (status.not_const())
		{
			LOG(color::green(i->get_name())+" is "+ cur_.print());
			return;
		}
	}
	LOG(color::green(i->get_name())+" is "+ cur_.print());
}

void SCCPVisitor::visit_phi(PhiInst* i)
{
	LOG(color::cyan("Visiting Phi: ")+ i->print());
	cur_ = {ValStatus::INIT, nullptr};
	for (int j = 1; j < i->get_num_operand(); j += 2)
	{
		auto op = i->get_operand(j - 1);
		auto status = val_map.get(op);
		LOG("current status: "+cur_.print()+" | "+color::yellow((op->get_name().empty() ? op->print() : op->get_name()))
			+" is "+ status.print());
		cur_ &= status;
		if (status.not_const())
		{
			LOG(color::green(i->get_name())+" is "+ cur_.print());
			return;
		}
	}
	LOG(color::green(i->get_name())+" is "+ cur_.print());
}

void SCCPVisitor::visit_br(const BranchInst* i)
{
	if (!i->is_cond_br())
	{
		flow_worklist.emplace_back(bb_, dynamic_cast<BasicBlock*>(i->get_operand(0)));
		return;
	}
	auto true_bb = dynamic_cast<BasicBlock*>(i->get_operand(1));
	auto false_bb = dynamic_cast<BasicBlock*>(i->get_operand(2));
	if (auto const_cond = val_map.get(i->get_operand(0)).const_val_)
	{
		const_cond->getBoolConstant()
			? flow_worklist.emplace_back(bb_, true_bb)
			: flow_worklist.emplace_back(bb_, false_bb);
	}
	else
	{
		flow_worklist.emplace_back(bb_, true_bb);
		flow_worklist.emplace_back(bb_, false_bb);
	}
}

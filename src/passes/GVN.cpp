#include "GVN.hpp"

#include <unordered_set>
#include <vector>

#include "Instruction.hpp"
#include <iostream>

#define DEBUG 0
#include "Util.hpp"

void GVN::run()
{
	LOG(color::blue("Run GVN Pass"));
	m_->set_print_name();
	for (auto f : m_->get_functions())
	{
		if (!f->is_declaration() && !f->is_lib_)
			run(f);
	}
	LOG(color::blue("GVN Pass Done"));
}

void GVN::run(Function* f)
{
	LOG(color::green("Visiting function "+f->get_name()));
	expr_val_map_.clear();
	dom_ = manager_->getFuncInfo<Dominators>(f);
	const auto& PostOrder = dom_->get_dom_post_order(f);
	// 逆拓扑序访问基本块
	for (auto it = PostOrder.rbegin(); it != PostOrder.rend(); ++it)
	{
		auto bb = *it;
		std::unordered_set<Instruction*> visited_insts_;
		std::vector<Instruction*> to_delete_;
		for (auto i : bb->get_instructions())
		{
			auto ret = visit_inst(i);
			if (ret == firstdef)
				visited_insts_.emplace(i);
			else if (ret == redundant)
				to_delete_.emplace_back(i);
		}
		expr_val_map_.clear();
		LOG(color::yellow("Replaced "+std::to_string(to_delete_.size())+
			" redundant expression(s) from basic block " + bb->get_name()));
		for (auto i : to_delete_)
		{
			i->get_parent()->erase_instr(i);
			LOG("Replaced expression: " + i->print());
			delete i;
		}
	}
}

GVN::gvn_state_t GVN::visit_inst(Instruction* i)
{
	auto hash = ValueHash(i);
	if (hash.vc_ == 0) return invalid;
	auto get = expr_val_map_.find(hash);
	if (get == expr_val_map_.end())
	{
		expr_val_map_[hash] = i;
		return firstdef;
	}
	i->replace_all_use_with(get->second);
	return redundant;
}


GVN::ValueHash::ValueHash(Value** vals, int vc, Instruction::OpID ty) : vals_(vals), vc_(vc), type_(ty)
{
}

GVN::ValueHash::ValueHash()
{
	vals_ = nullptr;
	vc_ = 0;
	type_ = Instruction::alloca_;
}

GVN::ValueHash::ValueHash(const Instruction* inst)
{
	switch (auto op = inst->get_instr_type())
	{
		case Instruction::ret:
		case Instruction::br:
		case Instruction::alloca_:
		case Instruction::load:
		case Instruction::store:
		case Instruction::phi:
		case Instruction::call: // TODO 可以使纯函数也参与消除, 不清楚有没有效果
		case Instruction::memcpy_:
		case Instruction::memclear_:
			vals_ = nullptr;
			vc_ = 0;
			type_ = op;
			return;
		case Instruction::add:
		case Instruction::mul:
		case Instruction::and_:
		case Instruction::fadd:
		case Instruction::fmul:
		case Instruction::eq:
		case Instruction::ne:
		case Instruction::feq:
		case Instruction::fne:
			{
				Value* op1 = inst->get_operand(0);
				Value* op2 = inst->get_operand(1);
				if (op1 > op2)
				{
					auto buf = op1;
					op1 = op2;
					op2 = buf;
				}
				vals_ = new Value*[2]{op1, op2};
				vc_ = 2;
				type_ = op;
				return;
			}
		case Instruction::sub:
		case Instruction::sdiv:
		case Instruction::srem:
		case Instruction::shl:
		case Instruction::ashr:
		case Instruction::fsub:
		case Instruction::fdiv:
		case Instruction::ge:
		case Instruction::gt:
		case Instruction::le:
		case Instruction::lt:
		case Instruction::fge:
		case Instruction::fgt:
		case Instruction::fle:
		case Instruction::flt:
			{
				Value* op1 = inst->get_operand(0);
				Value* op2 = inst->get_operand(1);
				vals_ = new Value*[2]{op1, op2};
				vc_ = 2;
				type_ = op;
				return;
			}
		case Instruction::getelementptr:
			{
				vc_ = u2iNegThrow(inst->get_operands().size());
				vals_ = new Value*[vc_];
				int idx = 0;
				for (auto i : inst->get_operands())
					vals_[idx++] = i;
				type_ = op;
				return;
			}
		case Instruction::zext:
		case Instruction::fptosi:
		case Instruction::sitofp:
		case Instruction::nump2charp:
		case Instruction::global_fix:
			{
				Value* op1 = inst->get_operand(0);
				vals_ = new Value*[1]{op1};
				vc_ = 1;
				type_ = op;
				return;
			}
	}
}

GVN::ValueHash::ValueHash(const ValueHash& other)
	: vc_(other.vc_),
	  type_(other.type_)
{
	if (vc_ == 0) vals_ = nullptr;
	else
	{
		vals_ = new Value*[vc_];
		memcpy(static_cast<void*>(vals_), static_cast<void*>(other.vals_), vc_ << 3);
	}
}

GVN::ValueHash::ValueHash(ValueHash&& other) noexcept
	: vals_(other.vals_),
	  vc_(other.vc_),
	  type_(other.type_)
{
	other.vals_ = nullptr;
	other.vc_ = 0;
}

GVN::ValueHash& GVN::ValueHash::operator=(const ValueHash& other)
{
	if (this == &other)
		return *this;
	type_ = other.type_;
	if (other.vc_ == 0)
	{
		vc_ = other.vc_;
		delete vals_;
		vals_ = nullptr;
	}
	else
	{
		if (other.vc_ > vc_)
		{
			delete vals_;
			vals_ = new Value*[vc_];
		}
		vc_ = other.vc_;
		memcpy(static_cast<void*>(vals_), static_cast<void*>(other.vals_), vc_ << 3);
	}
	return *this;
}

GVN::ValueHash& GVN::ValueHash::operator=(ValueHash&& other) noexcept
{
	if (this == &other)
		return *this;
	delete vals_;
	vals_ = other.vals_;
	vc_ = other.vc_;
	type_ = other.type_;
	other.vals_ = nullptr;
	other.vc_ = 0;
	return *this;
}

GVN::ValueHash::~ValueHash()
{
	delete vals_;
}

bool GVN::ValueHash::operator<(const ValueHash& r) const
{
	if (vc_ < r.vc_) return true;
	if (vc_ > r.vc_) return false;
	if (vc_ == 0) return false;
	if (type_ < r.type_) return true;
	if (type_ > r.type_) return false;
	int v = vc_;
	for (int i = 0; i < v; i++)
	{
		auto a = vals_[i];
		auto b = r.vals_[i];
		if (a < b) return true;
		if (a > b) return false;
	}
	return false;
}
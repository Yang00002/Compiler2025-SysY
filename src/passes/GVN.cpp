#include "GVN.hpp"

#include <string>
#include <unordered_set>
#include <vector>

#include "Instruction.hpp"
#include <iostream>

#define DEBUG 0
#include "Util.hpp"

#define IS_BINARY(i) (((i)->isBinary())||(((i)->is_cmp()||(i)->is_fcmp())&&((i)->get_num_operand()==2)))

bool GVN::is_commutative(const Instruction* i)
{
	return i->is_add() || i->is_fadd() || i->is_mul() || i->is_fmul() ||
	       i->get_instr_type() == Instruction::OpID::eq ||
	       i->get_instr_type() == Instruction::OpID::feq ||
	       i->get_instr_type() == Instruction::OpID::ne ||
	       i->get_instr_type() == Instruction::OpID::fne;
}

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
	dom_->run_on_func(f);
	const auto& PostOrder = dom_->get_dom_post_order(f);
	// 逆拓扑序访问基本块
	for (auto it = PostOrder.rbegin(); it != PostOrder.rend(); ++it)
	{
		auto bb = *it;
		std::unordered_set<Instruction*> visited_insts_;
		std::vector<Instruction*> to_delete_;
		for (const auto& i : bb->get_instructions())
		{
			if (i && to_be_visited(i))
			{
				auto ret = visit_inst(i);
				if (ret == firstdef)
					visited_insts_.emplace(i);
				else if (ret == redundant)
					to_delete_.emplace_back(i);
			}
		}
		for (auto i : visited_insts_) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
			erase_inst(i);
		LOG(color::yellow("Replaced "+std::to_string(to_delete_.size())+
			" redundant expression(s) from basic block " + bb->get_name()));
		for (auto i : to_delete_)
		{
			i->get_parent()->erase_instr(i);
			LOG("Replaced expression: " + i->print());
		}
	}
}

GVN::gvn_state_t GVN::visit_inst(Instruction* i)
{
	if (IS_BINARY(i))
	{
		auto expr = get_expr(i);
		if (expr_val_map_.count(expr))
		{
			i->replace_all_use_with(expr_val_map_.at(expr));
			return redundant;
		}
		if (is_commutative(i))
		{
			auto eq_expr = get_equiv_expr(i);
			if (expr_val_map_.count(eq_expr))
			{
				i->replace_all_use_with(expr_val_map_.at(eq_expr));
				return redundant;
			}
			expr_val_map_[expr] = i;
			expr_val_map_[eq_expr] = i;
		}
		else
		{
			expr_val_map_[expr] = i;
		}
		return firstdef;
	}
	else if (i->is_fp2si() || i->is_zext() || i->is_nump2charp() || i->get_instr_type() == Instruction::global_fix)
	{
		auto expr = get_expr(i);
		if (expr_val_map_.count(expr))
		{
			i->replace_all_use_with(expr_val_map_.at(expr));
			return redundant;
		}
		expr_val_map_[expr] = i;
		return firstdef;
	}
	return invalid;
}

void GVN::erase_inst(Instruction* i)
{
	auto expr = get_expr(i);
	expr_val_map_.erase(expr);
	if (is_commutative(i))
	{
		expr = get_equiv_expr(i);
		expr_val_map_.erase(expr);
	}
}

bool GVN::to_be_visited(const Instruction* i)
{
	return IS_BINARY(i) || i->is_si2fp() || i->is_fp2si() || i->is_zext() || i->is_nump2charp() ||
	       i->get_instr_type() == Instruction::global_fix;
}

std::string GVN::get_expr(const Instruction* i)
{
	if (IS_BINARY(i))
	{
		auto opnd0 = i->get_operand(0)->print();
		auto op = i->get_instr_op_name();
		auto opnd1 = i->get_operand(1)->print();
		return op + "," + opnd0 + "," + opnd1;
	}
	if (i->is_si2fp() || i->is_fp2si() || i->is_zext() || i->is_nump2charp() ||
	    i->get_instr_type() == Instruction::global_fix)
	{
		auto op = i->get_instr_op_name();
		auto opnd0 = i->get_operand(0)->print();
		return op + "," + opnd0;
	}
	return "";
}

// 交换操作数顺序不改变结果的指令
std::string GVN::get_equiv_expr(const Instruction* i)
{
	auto opnd0 = i->get_operand(1)->print();
	auto op = i->get_instr_op_name();
	auto opnd1 = i->get_operand(0)->print();
	return op + "," + opnd0 + "," + opnd1;
}

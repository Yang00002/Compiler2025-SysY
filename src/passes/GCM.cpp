#include "GCM.hpp"

#define DEBUG 0
#include "Util.hpp"

bool GlobalCodeMotion::is_pinned(const Instruction* i)
{
	return i->is_br() || i->is_call() || i->is_ret() || i->is_phi() || i->is_load() || i->is_store();
}

BasicBlock* GlobalCodeMotion::LCA(BasicBlock* bb1, BasicBlock* bb2) const
{
	if (bb1 == bb2) return bb1;
	if (dom_->is_dominate(bb1, bb2)) return bb1;
	if (dom_->is_dominate(bb2, bb1)) return bb2; // NOLINT(readability-suspicious-call-argument)
	std::unordered_set<BasicBlock*> visited;
	while (bb1 || bb2)
	{
		if (bb1)
		{
			if (visited.count(bb1)) return bb1;
			visited.emplace(bb1);
			bb1 = dom_->get_idom(bb1);
		}
		if (bb2)
		{
			if (visited.count(bb2)) return bb2;
			visited.emplace(bb2);
			bb2 = dom_->get_idom(bb2);
		}
	}
	return nullptr;
}

void GlobalCodeMotion::run(Function* f)
{
	visited_.clear();
	auto& po = dom_->get_dom_post_order(f);
	std::vector<Instruction*> instructions;
	for (auto bb : po)
	{
		for (auto i : bb->get_instructions())
			instructions.emplace_back(i);
	}
	for (auto i : instructions)
		moveEarly(i, f);
	visited_.clear();
	for (auto j = instructions.rbegin(); j != instructions.rend(); ++j)
		postpone(*j);
}

void GlobalCodeMotion::moveEarly(Instruction* i, Function* f)
{
	if (is_pinned(i) || visited_.count(i))
		return;
	visited_.emplace(i);
	i->get_parent()->erase_instr(i);
	f->get_entry_block()->get_instructions().emplace_common_inst_from_end(i, 1);

	for (auto op : i->get_operands())
	{
		if (auto input_inst = dynamic_cast<Instruction*>(op))
		{
			moveEarly(input_inst, f);
			// 恢复支配关系
			if (input_inst->get_parent() != i->get_parent() &&
			    dom_->is_dominate(i->get_parent(), input_inst->get_parent())
			)
			{
				i->get_parent()->erase_instr(i);
				input_inst->get_parent()->get_instructions().emplace_common_inst_from_end(i, 1);
			}
		}
	}
}

void GlobalCodeMotion::postpone(Instruction* i)
{
	if (is_pinned(i) || visited_.count(i))
		return;
	visited_.emplace(i);

	BasicBlock* lca = nullptr;
	for (auto& user : i->get_use_list())
	{
		if (auto user_inst = dynamic_cast<Instruction*>(user.val_))
		{
			postpone(user_inst);
			BasicBlock* use_bb = nullptr;
			if (auto phi = dynamic_cast<PhiInst*>(user_inst))
			{
				for (int j = 0; j < phi->get_num_operand(); j += 2)
				{
					auto phi_op = dynamic_cast<Instruction*>(phi->get_operand(j));
					if (phi_op && phi_op == i)
						use_bb = dynamic_cast<BasicBlock*>(phi->get_operand(j + 1));
				}
			}
			else use_bb = user_inst->get_parent();
			lca = LCA(lca, use_bb);
		}
	}

	if (!i->get_use_list().empty())
	{
		BasicBlock* best = lca;
		while (lca != i->get_parent())
		{
			lca = dom_->get_idom(lca);
			auto& lca_succ = lca->get_succ_basic_blocks();
			if (lca_succ.size() == 1 && std::find(lca_succ.begin(), lca_succ.end(), best) != lca_succ.end())
				best = lca;
		}
		i->get_parent()->erase_instr(i);
		best->get_instructions().emplace_common_inst_from_end(i, 1);
	}

	BasicBlock* best = i->get_parent();
	for (auto inst : best->get_instructions())
	{
		if (i != inst)
		{
			if (!inst->is_phi())
			{
				auto& phi_use = inst->get_use_list();
				if (std::find_if(phi_use.begin(), phi_use.end(),
				                 [&](const Use& u) { return u.val_ == i; }) != phi_use.end()
				)
				{
					i->get_parent()->erase_instr(i);
					auto prev_iter = best->get_instructions().begin();
					for (auto j = best->get_instructions().begin(); j != best->get_instructions().end(); ++j)
					{
						if (*j == inst)
							break;
						prev_iter = j;
					}
					best->get_instructions().emplace_common_inst_after(i, prev_iter);
					break;
				}
			}
		}
	}
}

void GlobalCodeMotion::run()
{
	for (auto f : m_->get_functions())
	{
		if (!f->is_lib_ && f->get_num_basic_blocks() > 1)
		{
			dom_ = manager_->getFuncInfo<Dominators>(f);
			run(f);
		}
	}
}

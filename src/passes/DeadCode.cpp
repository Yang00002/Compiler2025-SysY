#include "DeadCode.hpp"
#include "BasicBlock.hpp"
#include "Instruction.hpp"
#include <unordered_set>
#include <vector>

// 处理流程：两趟处理，mark 标记有用变量，sweep 删除无用指令
void DeadCode::run()
{
	bool changed;
	func_info->run();
	do
	{
		changed = false;
		for (const auto& func : m_->get_functions())
		{
			changed |= clear_basic_blocks(func);
			mark(func);
			changed |= sweep(func);
		}
	}
	while (changed);
}

bool DeadCode::clear_basic_blocks(Function* func)
{
	bool changed = false;
	std::vector<BasicBlock*> to_erase;
	for (auto& bb : func->get_basic_blocks())
	{
		if (bb->get_pre_basic_blocks().empty() && bb != func->get_entry_block())
		{
			to_erase.push_back(bb);
			changed = true;
		}
	}
	for (const auto& bb : to_erase)
	{
		bb->erase_from_parent();
	}
	return changed;
}

void DeadCode::mark(Function* func)
{
	work_list.clear();
	marked.clear();

	for (auto& bb : func->get_basic_blocks())
	{
		for (auto& ins : bb->get_instructions())
		{
			if (is_critical(ins))
			{
				marked[ins] = true;
				work_list.push_back(ins);
			}
		}
	}

	while (work_list.empty() == false)
	{
		auto now = work_list.front();
		work_list.pop_front();

		mark(now);
	}
}

void DeadCode::mark(const Instruction* ins)
{
	for (auto op : ins->get_operands())
	{
		auto def = dynamic_cast<Instruction*>(op);
		if (def == nullptr)
			continue;
		if (marked[def])
			continue;
		if (def->get_function() != ins->get_function())
			continue;
		marked[def] = true;
		work_list.push_back(def);
	}
}

bool DeadCode::sweep(Function* func)
{
	std::unordered_set<Instruction*> wait_del{};
	for (auto& bb : func->get_basic_blocks())
	{
		for (auto it = bb->get_instructions().begin();
		     it != bb->get_instructions().end();)
		{
			if (marked[*it])
			{
				++it;
				continue;
			}
			wait_del.insert(*it);
			++it;
		}
	}
	for (const auto inst : wait_del)
		inst->remove_all_operands();
	for (auto inst : wait_del)
		inst->get_parent()->get_instructions().remove(inst);
	ins_count += static_cast<int>(wait_del.size());
	return not wait_del.empty(); // changed
}

bool DeadCode::is_critical(Instruction* ins) const
{
	// 对纯函数的无用调用也可以在删除之列
	if (ins->is_call())
	{
		auto call_inst = dynamic_cast<CallInst*>(ins);
		auto callee = dynamic_cast<Function*>(call_inst->get_operand(0));
		if (func_info->is_pure_function(callee))
			return false;
		return true;
	}
	if (ins->is_memcpy() || ins->is_memclear())
		return true;
	if (ins->is_br() || ins->is_ret())
		return true;
	if (ins->is_store())
		return true;
	return false;
}

void DeadCode::sweep_globally() const
{
	std::vector<Function*> unused_funcs;
	std::vector<GlobalVariable*> unused_globals;
	for (auto& f_r : m_->get_functions())
	{
		if (f_r->get_use_list().empty() and f_r->get_name() != "main")
			unused_funcs.push_back(f_r);
	}
	for (auto& glob_var_r : m_->get_global_variable())
	{
		if (glob_var_r->get_use_list().empty())
			unused_globals.push_back(glob_var_r);
	}
	for (auto func : unused_funcs)
		m_->get_functions().remove(func);
	for (auto glob : unused_globals)
		m_->get_global_variable().remove(glob);
}

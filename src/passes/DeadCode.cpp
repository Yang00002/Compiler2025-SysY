#include "DeadCode.hpp"

#include <iostream>

#include "BasicBlock.hpp"
#include "Instruction.hpp"
#include <unordered_set>
#include <vector>

#include "Color.hpp"


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


// 处理流程：两趟处理，mark 标记有用变量，sweep 删除无用指令
void DeadCode::run()
{
	LOG(color::cyan("Run DeadCode Pass"));
	PUSH;
	bool changed;
	func_info->run();
	LOG(color::green("Function Info Collected "));
	do
	{
		changed = false;
		for (const auto& func : m_->get_functions())
		{
			if (func->is_lib_) continue;
			changed |= clear_basic_blocks(func);
			mark(func);
			changed |= sweep(func);
		}
	}
	while (changed);
	POP;
	LOG(color::cyan("DeadCode Done"));
}

bool DeadCode::clear_basic_blocks(Function* func)
{
	LOG(color::blue("Begin Clean Basic Block of ") + func->get_name());
	PUSH;
	bool changed = false;
	std::vector<BasicBlock*> to_erase;
	for (auto& bb : func->get_basic_blocks())
	{
		if (bb->get_pre_basic_blocks().empty() && bb != func->get_entry_block())
		{
			LOG(color::pink("Block ") + bb->get_name() + color::pink(" empty, remove"));
			to_erase.push_back(bb);
			changed = true;
		}
	}
	for (const auto& bb : to_erase)
	{
		bb->erase_from_parent();
	}
	POP;
	return changed;
}

void DeadCode::mark(Function* func)
{
	LOG(color::blue("Begin Mark Unused Instructions of ") + func->get_name());
	PUSH;
	work_list.clear();
	marked.clear();

	for (auto& bb : func->get_basic_blocks())
	{
		for (auto ins : bb->get_instructions())
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

	POP;
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
	LOG(color::blue("Begin Sweep ") + func->get_name());
	PUSH;
	int count = 0;
	for (const auto& bb : func->get_basic_blocks())
	{
		LOG(color::pink("BasicBlock ") + bb->get_name());
		auto& instructions = bb->get_instructions();
		auto it = instructions.begin();
		PUSH;
		while (it != instructions.end())
		{
			auto n = it.get_and_add();
			if (marked[n]) continue;
			LOG(color::pink("Remove Instruction ") + n->print());
			n->remove_all_operands();
			// ReSharper disable once CppNoDiscardExpression
			it.remove_pre();
			delete n;
			count++;
		}
		POP;
	}
	ins_count += count;
	POP;
	return count;
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

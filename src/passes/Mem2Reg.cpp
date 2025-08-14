#include "Mem2Reg.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stack>

#include "Color.hpp"
#include "BasicBlock.hpp"
#include "Function.hpp"
#include "GlobalVariable.hpp"
#include "Instruction.hpp"
#include "Module.hpp"
#include "Type.hpp"
#include "Value.hpp"

#define DEBUG 0
#include "Util.hpp"

Mem2Reg::Mem2Reg(Module* m) : Pass(m)
{
	func_ = nullptr;
}

/**
 * 该函数执行内存到寄存器的提升过程，将栈上的局部变量提升到SSA格式。
 * 主要步骤：
 * 1. 创建并运行支配树分析
 * 2. 对每个非声明函数：
 *    - 清空相关数据结构
 *    - 插入必要的phi指令
 *    - 执行变量重命名
 *
 * 注意：函数执行后，冗余的局部变量分配指令将由后续的死代码删除Pass处理
 */
void Mem2Reg::run()
{
	LOG(color::cyan("Run Mem2Reg Pass"));
	// 创建支配树分析 Pass 的实例
	dominators_ = std::make_unique<Dominators>(m_);
	// 建立支配树
	dominators_->run();
	LOG(color::green("Dominators Built"));
	// 以函数为单元遍历实现 Mem2Reg 算法
	for (const auto& f : m_->get_functions())
	{
		if (f->is_declaration())
			continue;
		LOG(color::blue("Working on Function " ) + f->get_name());
		GAP;
		RUN(dominators_->print_dominance_frontier(f));
		GAP;
		RUN(dominators_->print_idom(f));
		GAP;
		PUSH;
		// dominators_->dump_dominator_tree(&f);
		func_ = f;
		var_val_stack.clear();
		phi_lval.clear();
		if (!func_->get_basic_blocks().empty())
		{
			// 对应伪代码中 phi 指令插入的阶段
			generate_phi();
			// 对应伪代码中重命名阶段
			rename(func_->get_entry_block());
			//dominators_->dump_dominator_tree(func_);
			//dominators_->dump_cfg(func_);
		}
		else
		{
			LOG(color::yellow("Function ") + f->get_name() + color::yellow(" is empty, passed."));
		}
		POP;
		// 后续 DeadCode 将移除冗余的局部变量的分配空间
	}
	PASS_SUFFIX;
	LOG(color::cyan("Mem2Reg Done"));
	for(auto f : m_->get_functions()){
		if(f->is_lib_) continue;
		for(auto bb : f->get_basic_blocks()){
			for(auto inst : bb->get_instructions()){
				if(inst->is_store()){
					assert(!inst->get_operand(0)->get_type()->isPointerType());
				}
			}
		}
	}
}

/**
 * @brief 在必要的位置插入phi指令
 *
 * 该函数实现了经典的phi节点插入算法：
 * 1. 收集全局活跃变量：
 *    - 扫描所有store指令
 *    - 识别在多个基本块中被赋值的变量
 *
 * 2. 插入phi指令：
 *    - 对每个全局活跃变量
 *    - 在其定值点的支配边界处插入phi指令
 *    - 使用工作表法处理迭代式的phi插入
 *
 * phi指令的插入遵循最小化原则，只在必要的位置插入phi节点
 */
void Mem2Reg::generate_phi()
{
	LOG(color::blue("Begin Generate Phi"));
	PUSH;
	// global_live_var_name 是全局名字集合，以 alloca 出的局部变量来统计。
	// 步骤一：找到活跃在多个 block 的全局名字集合，以及它们所属的 bb 块
	std::set<Value*> global_live_var_name;
	std::map<Value*, std::set<BasicBlock*>> live_var_2blocks;
	for (auto& bb : func_->get_basic_blocks())
	{
		LOG(color::blue("Searching removable allocated ops in BasicBlock ") + bb->get_name());
		PUSH;
		std::set<Value*> var_is_killed;
		for (const auto instr : bb->get_instructions())
		{
			if (instr->is_store())
			{
				// store i32 a, i32 *b
				// a is r_val, b is l_val
				if (auto l_val = dynamic_cast<StoreInst*>(instr)->get_lval(); is_valid_ptr(l_val))
				{
					LOG("Find Lval " + l_val->print());
					global_live_var_name.insert(l_val);
					live_var_2blocks[l_val].insert(bb);
				}
			}
		}
		POP;
	}

	LOG(color::green("All Ops Found"));
	// 步骤二：从支配树获取支配边界信息，并在对应位置插入 phi 指令
	std::map<std::pair<BasicBlock*, Value*>, bool> bb_has_var_phi; // bb has phi for var
	for (auto var : global_live_var_name)
	{
		LOG(color::blue("Begin Insert Phi For " ) + var->print());
		PUSH;
		std::vector<BasicBlock*> work_list;
		work_list.assign(live_var_2blocks[var].begin(),
		                 live_var_2blocks[var].end());
		for (int i = 0; i < u2iNegThrow(work_list.size()); i++)
		{
			auto bb = work_list[i];
			LOGIF(color::yellow("BasicBlock ") + bb->get_name() + color::yellow(" have no dominance frontier"),
			      dominators_->get_dominance_frontier(bb).empty());
			auto g = dominators_->get_dominance_frontier(bb);
			for (auto bb_dominance_frontier_bb :
			     dominators_->get_dominance_frontier(bb))
			{
				if (bb_has_var_phi.find({bb_dominance_frontier_bb, var}) ==
				    bb_has_var_phi.end())
				{
					// generate phi for bb_dominance_frontier_bb & add
					// bb_dominance_frontier_bb to work list
					auto phi = PhiInst::create_phi(
						var->get_type()->toPointerType()->typeContained(),
						bb_dominance_frontier_bb);
					phi_lval.emplace(phi, var);
					bb_dominance_frontier_bb->add_instruction(phi);
					work_list.push_back(bb_dominance_frontier_bb);
					bb_has_var_phi[{bb_dominance_frontier_bb, var}] = true;
					LOG(color::pink("Add phi for ") + var->
						print() + color::pink(" from block ") + bb_dominance_frontier_bb->get_name());
				}
				else
				{
					LOG(color::yellow("BasicBlock ") + bb->get_name() + color::yellow(" already have phi for ") + var->
						print() + color::yellow(" from block ") + bb_dominance_frontier_bb->get_name());
				}
			}
		}
		POP;
	}

	POP;
	LOG(color::green("All Phi Inserted"));
}

class rename_helper
{
public:
	std::map<Value*, std::stack<Value*>> name_stack_map;
	std::stack<std::map<Value*, int>> count_map;
	std::map<Value*, Value*> replace_map;
	std::map<PhiInst*, Value*>* phi_lval;

	rename_helper(std::map<PhiInst*, Value*>& m) : phi_lval(&m)
	{
	}

	std::map<Value*, int> idmap;
	std::set<Value*> remove_id;
	std::vector<Instruction*> wait_delete;
	int id = 0;

	void remove(Instruction* v)
	{
		wait_delete.push_back(v);
		remove_id.insert(v);
	}

	int get_id(Value* v)
	{
		if (idmap.count(v) == 0)
		{
			idmap[v] = id++;
		}
		return idmap[v];
	}

	std::string show_instruction(Instruction* v)
	{
		std::string ret = std::to_string(get_id(v)) + " " + v->get_instr_op_name() + " ";
		for (auto& i : v->get_operands())
		{
			if (remove_id.count(i))
				ret += "\033[10;31;49m" + std::to_string(get_id(i)) + "\033[10;39;49m ";
			else
				ret += std::to_string(get_id(i)) + " ";
		}
		return ret;
	}

	void run(BasicBlock* bb)
	{
		std::set<BasicBlock*> visited;
		std::stack<BasicBlock*> dfsWorkList;
		std::stack<std::list<BasicBlock*>::iterator> dfsVisitList;
		dfsWorkList.emplace(bb);
		dfsVisitList.emplace(bb->get_succ_basic_blocks().begin());
		run_inner(bb);
		visited.insert(bb);
		while (!dfsWorkList.empty())
		{
			auto b = dfsWorkList.top();
			auto& it = dfsVisitList.top();
			if (it == b->get_succ_basic_blocks().end())
			{
				dfsVisitList.pop();
				if (!dfsVisitList.empty())
				{
					auto& ic = count_map.top();
					for (auto& [v, j] : ic)
					{
						auto& focus = name_stack_map[v];
						for (int k = 0; k < j; k++)
						{
							focus.pop();
						}
					}
					count_map.pop();
				}
				dfsWorkList.pop();
				continue;
			}
			auto get = *it;
			if (visited.count(get) == 0)
			{
				run_inner(get);
				visited.insert(get);
				dfsVisitList.push(get->get_succ_basic_blocks().begin());
				dfsWorkList.push(get);
			}
			++it;
		}
		/*
		 
		run_inner(bb);
		visited.insert(bb);
		for (auto& i : bb->get_succ_basic_blocks())
		{
			if (visited.count(i) == 0)
			{
				visited.insert(i);
				run(i);
				auto ic = count_map.top();
				count_map.pop();
				for (auto& [v, j] : ic)
				{
					auto& focus = name_stack_map[v];
					for (int k = 0; k < j; k++)
					{
						focus.pop();
					}
				}
			}
		}
		 */
	}

	void run_inner(BasicBlock* bb)
	{
		LOG(color::blue("Begin Rename ") + bb->get_name() + color::blue(" in ") + bb->get_parent()->get_name());
		PUSH;
		std::map<Value*, int> inner_count_map;
		for (const auto ins : bb->get_instructions())
		{
			LOG(color::pink("Instruction ") + show_instruction(ins) + color::pink(" [ ") + ins->print() + color::pink(
				" ] "));
			PUSH;
			if (ins->is_alloca())
			{
				if (const auto ai = dynamic_cast<AllocaInst*>(ins); !ai->get_alloca_type()->isArrayType() &&
				                                                    !ai->get_alloca_type()->isPointerType())
				{
					name_stack_map[ins] = {};
					remove(ins);
					LOG("Remove alloca.");
				}
				else
				{
					LOG("Alloca array or pointer, not remove.");
				}
			}
			else if (ins->is_store())
			{
				auto ai = dynamic_cast<GetElementPtrInst*>(ins->get_operand(1));
				auto ag = dynamic_cast<GlobalVariable*>(ins->get_operand(1));
				if (ai == nullptr && ag == nullptr)
				{
					auto op = ins->get_operand(0);
					if (replace_map.find(op) != replace_map.end())
						op = replace_map[op];
					auto& s = name_stack_map[op];
					if (s.empty() || s.top() != op)
					{
						auto index = ins->get_operand(1);
						name_stack_map[index].push(op);
						if (inner_count_map.count(index))
							inner_count_map[index]++;
						else
							inner_count_map[index] = 1;
					}
					remove(ins);
					LOG("Remove store");
				}
				else
				{
					int size = u2iNegThrow(ins->get_operands().size());
					for (int idx = 0; idx < size; idx++)
					{
						auto op = ins->get_operand(idx);
						if (replace_map.find(op) != replace_map.end())
						{
							ins->set_operand(idx, replace_map[op]);
						}
					}
					LOG("To " + show_instruction(ins));
				}
			}
			else if (ins->is_load())
			{
				if (!name_stack_map[ins->get_operand(0)].empty())
				{
					replace_map[ins] =
						name_stack_map[ins->get_operand(0)].top();
					remove(ins);

					LOG("Remove load, linking " + std::to_string(get_id(ins)) + " to " + std::to_string(get_id(
						replace_map[ins])));
				}
				else
				{
					LOG("Load from array or pointer, not remove");
				}
			}
			else if (ins->is_phi())
			{
				auto& l = *phi_lval;
				auto pi = dynamic_cast<PhiInst*>(ins);
				if (auto f = l.find(pi); f != l.end())
				{
					auto value = f->second;
					name_stack_map[value].push(ins);
					if (inner_count_map.count(value))
						inner_count_map[value]++;
					else
						inner_count_map[value] = 1;
				}
			}
			else
			{
				int size = u2iNegThrow(ins->get_operands().size());
				for (int idx = 0; idx < size; idx++)
				{
					auto op = ins->get_operand(idx);
					if (replace_map.find(op) != replace_map.end())
					{
						ins->set_operand(idx, replace_map[op]);
					}
				}
				LOG("To " + show_instruction(ins));
			}
			POP;
		}
		count_map.push(inner_count_map);
		for (auto succ : bb->get_succ_basic_blocks())
		{
			for (auto inst : succ->get_instructions())
			{
				if (inst->is_phi())
				{
					auto p = dynamic_cast<PhiInst*>(inst);
					if (auto f = phi_lval->find(p); f != phi_lval->end())
					{
						if (!name_stack_map[f->second].empty())
							p->add_phi_pair_operand(name_stack_map[f->second].top(),
							                        bb);
					}
				}
				else
					break;
			}
		}
		POP;
	}

	void showWD() const
	{
		for (const auto instr : wait_delete)
			LOG(color::yellow("Remove " + instr->print()));
	}

	void clean() const
	{
		RUN(showWD());
		for (const auto instr : wait_delete)
		{
			instr->get_parent()->erase_instr(instr);
			delete instr;
		}
	}
};

void Mem2Reg::rename(BasicBlock* bb)
{
	LOG(color::blue("Begin Rename ") + bb->get_parent()->get_name());
	PUSH;
	rename_helper helper(phi_lval);
	helper.run(bb);
	POP;
	LOG(color::blue("Begin Clean ") + bb->get_parent()->get_name());
	PUSH;
	helper.clean();
	POP;
	LOG(color::green("Rename Done"));
	// DONE
	// 步骤一：将 phi 指令作为 lval 的最新定值，lval 即是为局部变量 alloca
	// 出的地址空间
	// 步骤二：用 lval 最新的定值替代对应的load指令
	// 步骤三：将
	// store 指令的 rval，也即被存入内存的值，作为 lval 的最新定值
	// 步骤四：为 lval 对应的 phi 指令参数补充完整
	// 步骤五：对 bb 在支配树上的所有后继节点，递归执行 re_name 操作
	// 步骤六：pop出 lval 的最新定值
	// 步骤七：清除冗余的指令
}

bool Mem2Reg::is_global_variable(Value* l_val)
{
	return dynamic_cast<GlobalVariable*>(l_val) != nullptr;
}

bool Mem2Reg::is_gep_instr(Value* l_val)
{
	return dynamic_cast<GetElementPtrInst*>(l_val) != nullptr;
}

bool Mem2Reg::is_valid_ptr(Value* l_val)
{
	return !is_global_variable(l_val) && !is_gep_instr(l_val);
}

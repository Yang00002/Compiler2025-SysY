#include "LICM.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <stdexcept>
#include <vector>

#include "BasicBlock.hpp"
#include "Color.hpp"
#include "Constant.hpp"
#include "FuncInfo.hpp"
#include "Function.hpp"
#include "GlobalVariable.hpp"
#include "Instruction.hpp"
#include "PassManager.hpp"
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


/**
 * @brief 循环不变式外提Pass的主入口函数
 *
 */
void LoopInvariantCodeMotion::run()
{
	LOG(color::cyan("Run LICM Pass"));
	PUSH;
	loop_detection_ = std::make_unique<LoopDetection>(m_);
	loop_detection_->run();
	LOG(color::green("Loop Dection Done"));
	GAP;
	RUN(loop_detection_->print());
	GAP;
	func_info_ = std::make_unique<FuncInfo>(m_);
	func_info_->run();
	for (auto& loop : loop_detection_->get_loops())
	{
		is_loop_done_[loop] = false;
	}

	for (auto& loop : loop_detection_->get_loops())
	{
		traverse_loop(loop);
	}
	POP;
	LOG(color::cyan("LICM Done"));
}

/**
 * @brief 遍历循环及其子循环
 * @param loop 当前要处理的循环
 *
 */
void LoopInvariantCodeMotion::traverse_loop(Loop* loop)
{
	if (is_loop_done_[loop])
	{
		return;
	}
	is_loop_done_[loop] = true;
	for (auto& sub_loop : loop->get_sub_loops())
	{
		traverse_loop(sub_loop);
	}
	run_on_loop(loop);
}


// 1. 遍历当前循环及其子循环的所有指令
// 2. 收集所有指令到loop_instructions中
// 3. 检查store指令是否修改了全局变量，如果是则添加到updated_global中
// 4. 检查是否包含非纯函数调用，如果有则设置contains_impure_call为true
void LoopInvariantCodeMotion::collect_loop_info(
	Loop* loop, std::set<Value*>& loop_instructions, bool& contains_impure_call) const
{
	for (auto bb : loop->get_blocks())
	{
		for (auto ins : bb->get_instructions())
		{
			loop_instructions.emplace(ins);
		}
	}
	for (auto ins : loop_instructions)
	{
		auto inst = dynamic_cast<Instruction*>(ins);
		if (!contains_impure_call)
		{
			if (inst->is_call())
			{
				if (const auto func = dynamic_cast<CallInst*>(inst)->get_function();
					func_info_->is_pure_function(func) == false)
					contains_impure_call = true;
			}
		}
	}
}

class InstVec : public std::vector<Instruction*>
{
public:
	bool contain(const Value* v) const
	{
		return std::any_of(begin(), end(), [v](const Value* i)
		{
			return i == v;
		});
	}

	void insert(Value* v) { push_back(dynamic_cast<Instruction*>(v)); }
};

/**
 * @brief 对单个循环执行不变式外提优化
 * @param loop 要优化的循环
 *
 */
void LoopInvariantCodeMotion::run_on_loop(Loop* loop) const
{
	LOG(color::blue("Handling Loop Begin At ") + loop->get_header()->get_name());
	PUSH;
	std::set<Value*> loop_instructions;
	std::set<BasicBlock*> blocks;
	for (auto b : loop->get_blocks())
		blocks.insert(b);
	bool contains_impure_call = false;
	collect_loop_info(loop, loop_instructions,
	                  contains_impure_call);

	InstVec loop_invariant;

	// - 如果指令已被标记为不变式则跳过
	// - 跳过 store、load、ret、br、phi 等指令与非纯函数调用
	// - 特殊处理全局变量的  指令
	// - 检查指令的所有操作数是否都是循环不变的
	// - 如果有新的不变式被添加则注意更新 changed 标志，继续迭代
	std::set<Function*> func_set;
	std::set<Function*> impure_set;
	bool changed;
	do
	{
		changed = false;
		for (auto& v : loop_instructions)
		{
			LOG(color::pink("Checking ") + v->print());
			if (loop_invariant.contain(v))
			{
				PUSH;
				LOG(color::yellow("Already Invariant, next"));
				POP;
				continue;
			}
			const auto inst = dynamic_cast<Instruction*>(v);
			int idx = 0;
			if (inst->is_store() || inst->is_load() || inst->is_ret() || inst->is_br() ||
			    inst->is_phi() || inst->is_memcpy() || inst->is_memclear())
			{
				PUSH;
				LOG(color::yellow("Instruction type can not lead to invariant"));
				POP;
				continue;
			}
			if (inst->is_call() && contains_impure_call)
			{
				const auto call = dynamic_cast<CallInst*>(inst);
				if (auto func = call->get_function(); func_set.count(func) == 0)
				{
					func_set.insert(func);
					if (!func_info_->is_pure_function(func))
					{
						impure_set.insert(func);
						PUSH;
						LOG(color::yellow("Impure function call"));
						POP;
						continue;
					}
				}
				else if (impure_set.count(func))
				{
					PUSH;
					LOG(color::yellow("Impure function call"));
					POP;
					continue;
				}
				idx = 1;
			}
			auto& ops = inst->get_operands();
			int size = static_cast<int>(ops.size());
			bool c = true;
			for (int i = idx; i < size; i++)
			{
				auto& op = ops[i];
				if (dynamic_cast<Constant*>(op) == nullptr &&
				    loop_instructions.count(op) != 0 &&
				    !loop_invariant.contain(op))
				{
					c = false;
					break;
				}
			}
			PUSH;
			if (c)
			{
				loop_invariant.insert(inst);
				changed = true;
				LOG(color::green("Invariant"));
			}
			else
				LOG("Can not decide");
			POP;
		}
		if (changed == true)
			LOG(color::green("Detect Invariant, try again"));
	}
	while (changed);

	if (loop_invariant.empty())
	{
		POP;
		LOG(color::green("No Invariant Detected"));
		return;
	}

	LOG(color::green("Find All Invariants"));
	if (loop->get_preheader() == nullptr)
	{
		LOG(color::pink("Create PreHeader"));
		loop->set_preheader(
			BasicBlock::create(m_, "", loop->get_header()->get_parent()));
	}

	auto preheader = loop->get_preheader();

	for (auto phi_inst_ : loop->get_header()->get_instructions())
	{
		if (phi_inst_->get_instr_type() != Instruction::phi)
			break;
		LOG(color::pink("Processing ") + phi_inst_->print());
		auto phi = dynamic_cast<PhiInst*>(phi_inst_);
		std::map<BasicBlock*, Value*> phiMap;
		Value* v = nullptr;
		int preCount = 0;
		for (auto& [i, j] : phi->get_phi_pairs())
		{
			phiMap[j] = i;
			v = i;
			if (!blocks.count(j))
				preCount++;
		}
		phi->remove_all_operands();
		if (preCount > 1)
		{
			auto phi2 = PhiInst::create_phi(v->get_type(), preheader);
			preheader->add_instruction(phi2);
			for (auto [bb, value] : phiMap)
			{
				if (blocks.count(bb))
				{
					phi->add_phi_pair_operand(value, bb);
				}
				else
				{
					phi2->add_phi_pair_operand(value, bb);
				}
				phi->add_phi_pair_operand(phi2, preheader);
			}
			PUSH;
			LOG("To " + phi2->print());
			LOG("And " + phi->print());
			POP;
		}
		else
		{
			for (auto& [bb, value] : phiMap)
			{
				if (blocks.count(bb))
				{
					phi->add_phi_pair_operand(value, bb);
				}
				else
				{
					phi->add_phi_pair_operand(value, preheader);
				}
			}
			PUSH;
			LOG("To " + phi->print());
			POP;
		}
	}

	// 将所有非 latch 的 header 前驱块的跳转指向 preheader
	// 并将 preheader 的跳转指向 header
	// 注意这里需要更新前驱块的后继和后继的前驱

	std::vector<BasicBlock*> pred_to_remove;
	for (auto& pred : loop->get_header()->get_pre_basic_blocks())
	{
		if (blocks.count(pred) == 0)
			pred_to_remove.emplace_back(pred);
	}
	LOGIF(color::pink("Handling PreHeader Relation"), !pred_to_remove.empty());
	for (auto& pred : pred_to_remove)
	{
		PUSH;
		LOG("Remove " + pred->get_name() + " -> " + loop->get_header()->get_name());
		LOG("Add " + pred->get_name() + " -> " + preheader->get_name());
		POP;
		loop->get_header()->remove_pre_basic_block(pred);
		pred->remove_succ_basic_block(loop->get_header());
		pred->add_succ_basic_block(preheader);
		preheader->add_pre_basic_block(pred);
		const auto ins = pred->get_instructions().back();
		const int size = static_cast<int>(ins->get_operands().size());
		for (int i = 0; i < size; i++)
		{
			if (ins->get_operand(i) == loop->get_header())
			{
				ins->set_operand(i, preheader);
			}
		}
	}
	LOG(color::pink("Moving Invariants"));
	PUSH;
	for (auto ins : loop_invariant)
	{
		LOG("From " + ins->get_parent()->get_name() + " to " + preheader->get_name());
		ins->get_parent()->erase_instr(ins);
		preheader->add_instruction(ins);
		ins->set_parent(preheader);
	}
	POP;

	// insert preheader br to header
	LOG(color::pink("Linking ") + preheader->get_name() + color::pink(" -> ") + loop->get_header()->get_name());

	BranchInst::create_br(loop->get_header(), preheader);

	// insert preheader to parent loop
	if (loop->get_parent() != nullptr)
	{
		loop->get_parent()->add_block(preheader);
	}
	POP;
}

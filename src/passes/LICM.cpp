#include "LICM.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "FuncInfo.hpp"
#include "Function.hpp"
#include "Instruction.hpp"
#include "PassManager.hpp"
#include "Value.hpp"

#define DEBUG 0
#include "Util.hpp"

void LoopInvariantCodeMotion::run()
{
	PREPARE_PASS_MSG;
	PASS_SUFFIX;
	LOG(color::cyan("Run LICM Pass"));
	PUSH;
	func_info_ = manager_->getGlobalInfo<FuncInfo>();
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_)continue;
		loop_detection_ = manager_->getFuncInfo<LoopDetection>(f);
		for (auto& loop : loop_detection_->get_loops())
		{
			is_loop_done_[loop] = false;
		}
		for (auto& loop : loop_detection_->get_loops())
		{
			traverse_loop(loop);
		}
	}
	POP;
	PASS_SUFFIX;
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


void LoopInvariantCodeMotion::collect_loop_info(
	Loop* loop, InstructionList& loop_instructions)
{
	for (auto bb : loop->get_blocks())
	{
		loop_instructions.addAll(bb->get_instructions());
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
	InstructionList loop_instructions;
	std::unordered_set<BasicBlock*> blocks;
	for (auto b : loop->get_blocks())
		blocks.insert(b);
	collect_loop_info(loop, loop_instructions);
	std::unordered_set<Instruction*> in_loop_instructions;
	for (auto i : loop_instructions) in_loop_instructions.insert(i);
	std::unordered_set<Instruction*> loop_invariant;
	std::list<Instruction*> invariant_as_list;
	std::unordered_set<Instruction*> loop_variant;

	// - 如果指令已被标记为不变式则跳过
	// - 跳过 store、load、ret、br、phi 等指令与非纯函数调用
	// - 特殊处理全局变量的指令
	// - 检查指令的所有操作数是否都是循环不变的
	// - 如果有新的不变式被添加则注意更新 changed 标志，继续迭代

	// 所有在循环中被存储过的值源(局部变量, 参数, 全局变量)
	std::unordered_set<Value*> dirtyValues;
	// 值的来源(各种变量 -> 局部变量, 参数, 全局变量)
	std::unordered_map<Value*, Value*> valueSrc;

	for (auto bb : loop->get_blocks())
	{
		for (auto inst : bb->get_instructions())
		{
			if (inst->is_store() || inst->is_memcpy())
			{
				auto op = inst->get_operand(1);
				auto src = valueSrc[op];
				if (src == nullptr)
				{
					src = ptrFrom(op);
					valueSrc[op] = src;
				}
				dirtyValues.emplace(src);
				continue;
			}
			if (inst->is_memclear())
			{
				auto op = inst->get_operand(0);
				auto src = valueSrc[op];
				if (src == nullptr)
				{
					src = ptrFrom(op);
					valueSrc[op] = src;
				}
				dirtyValues.emplace(src);
				continue;
			}
			if (inst->is_call() && !dynamic_cast<Function*>(inst->get_operand(0))->is_lib_)
			{
				auto& storeDetails = func_info_->storeDetail(dynamic_cast<Function*>(inst->get_operand(0)));
				for (auto g : storeDetails.globals_) dirtyValues.emplace(g);
				for (auto arg : storeDetails.arguments_)
				{
					auto op = inst->get_operand(arg->get_arg_no() + 1);
					auto src = valueSrc[op];
					if (src == nullptr)
					{
						src = ptrFrom(op);
						valueSrc[op] = src;
					}
					dirtyValues.emplace(src);
				}
			}
		}
	}

	bool changed;
	do
	{
		changed = false;
		auto it = loop_instructions.begin();
		while (it != loop_instructions.end())
		{
			auto inst = it.get_and_add();
			LOG(color::pink("Checking ") + inst->print());
			int idx = 0;
			if (inst->is_store() || inst->is_ret() || inst->is_br() ||
			    inst->is_phi() || inst->is_memcpy() || inst->is_memclear())
			{
				loop_variant.insert(inst);
				it.remove_pre();
				PUSH;
				LOG(color::yellow("Instruction type can not lead to invariant"));
				POP;
				continue;
			}
			// load 的指针是循环不变量, 并且内容没有在循环中变过
			if (inst->is_load())
			{
				if (dirtyValues.count(ptrFrom(inst->get_operand(0))))
				{
					loop_variant.insert(inst);
					it.remove_pre();
					PUSH;
					LOG(color::yellow("Dirty load from"));
					POP;
					continue;
				}
				auto from = dynamic_cast<Instruction*>(inst->get_operand(0));
				if (from != nullptr)
				{
					if (loop_variant.count(from))
					{
						loop_variant.insert(inst);
						it.remove_pre();
						PUSH;
						LOG(color::yellow("Impure load index"));
						POP;
						continue;
					}
					if (!loop_invariant.count(from))
					{
						LOG("Can not decide");
						continue;
					}
				}
				loop_invariant.insert(inst);
				invariant_as_list.emplace_back(inst);
				it.remove_pre();
				changed = true;
				LOG(color::green("Invariant"));
				continue;
			}
			// (可能循环不变) -> 未使用 impure 库, call 未存储任何值, 并且加载的值在循环内未更改
			if (inst->is_call())
			{
				const auto call = dynamic_cast<CallInst*>(inst);
				auto func = dynamic_cast<Function*>(call->get_operand(0));
				if (func_info_->useOrIsImpureLib(func))
				{
					PUSH;
					LOG(color::yellow("Impure function call"));
					POP;
					it.remove_pre();
					loop_variant.insert(inst);
					continue;
				}
				if (!func_info_->storeDetail(func).empty())
				{
					PUSH;
					LOG(color::yellow("Dirty function call"));
					POP;
					it.remove_pre();
					loop_variant.insert(inst);
					continue;
				}
				bool ok = true;
				auto& ld = func_info_->loadDetail(func);
				for (auto l : ld.globals_)
				{
					if (dirtyValues.count(l))
					{
						ok = false;
						break;
					}
				}
				if (ok)
				{
					for (auto arg : ld.arguments_)
					{
						auto op = inst->get_operand(arg->get_arg_no() + 1);
						auto src = valueSrc[op];
						if (src == nullptr)
						{
							src = ptrFrom(op);
							valueSrc[op] = src;
						}
						if (dirtyValues.count(src))
						{
							ok = false;
							break;
						}
					}
				}
				if (!ok)
				{
					PUSH;
					LOG(color::yellow("Load dirty function call"));
					POP;
					it.remove_pre();
					loop_variant.insert(inst);
					continue;
				}
				idx = 1;
			}
			auto& ops = inst->get_operands();
			int size = u2iNegThrow(ops.size());
			int c = 1;
			for (int i = idx; i < size; i++)
			{
				auto& op = ops[i];
				if (dynamic_cast<Constant*>(op) != nullptr) continue;
				auto iop = dynamic_cast<Instruction*>(op);
				if (iop == nullptr) continue;
				if (in_loop_instructions.count(iop) != 0 && loop_invariant.count(iop) == 0)
				{
					if (loop_variant.count(iop) != 0)
					{
						PUSH;
						LOG(color::yellow("Contain variant, no invariant"));
						POP;
						c = -1;
						break;
					}
					c = 0;
				}
			}
			PUSH;
			if (c == 1)
			{
				loop_invariant.insert(inst);
				invariant_as_list.emplace_back(inst);
				it.remove_pre();
				changed = true;
				LOG(color::green("Invariant"));
			}
			else if (c == 0)
			{
				LOG("Can not decide");
			}
			else
			{
				loop_variant.insert(inst);
				it.remove_pre();
			}
			POP;
		}
		if (changed == true)
		{
			LOG(color::green("Detect Invariant, try again"));
		}
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
		auto preheader = loop->get_preheader();

		loop->get_header()->add_block_before(preheader, loop->get_latches());

		LOG(color::pink("Moving Invariants"));
		PUSH;
		for (auto ins : invariant_as_list) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
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
	}
	else
	{
		auto preheader = loop->get_preheader();
		Instruction* pb = preheader->get_terminator();
		preheader->get_instructions().pop_back();
		PUSH;
		for (auto ins : invariant_as_list) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
		{
			LOG("From " + ins->get_parent()->get_name() + " to " + preheader->get_name());
			ins->get_parent()->erase_instr(ins);
			preheader->add_instruction(ins);
			ins->set_parent(preheader);
		}
		POP;
		preheader->add_instruction(pb);
	}
	POP;
}

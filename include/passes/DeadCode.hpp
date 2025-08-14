#pragma once

#include <deque>
#include <queue>

#include "FuncInfo.hpp"
#include "PassManager.hpp"

/**
 * 死代码消除
 **/
class DeadCode final : public Pass
{
	struct WorkList
	{
		std::deque<Instruction*> work_list{};
		std::unordered_map<Instruction*, bool> marked{};
		std::queue<Function*> clear_basic_blocks_;
		std::unordered_set<Function*> in_clear_basic_blocks_;
		std::queue<Function*> clear_parameters_;
		std::unordered_set<Function*> in_clear_parameters_;
		std::queue<Function*> clear_not_use_allocas_;
		std::unordered_set<Function*> in_clear_not_use_allocas_;
		std::queue<Function*> clear_instructions_;
		std::unordered_set<Function*> in_clear_instructions_;

		void addClearBasicBlock(Function* f)
		{
			if (in_clear_basic_blocks_.count(f))
			{
				in_clear_basic_blocks_.emplace(f);
				clear_basic_blocks_.emplace(f);
			}
		}

		void addClearParameters(Function* f);

		void addClearNotUseAllocas(Function* f);

		void addClearInstructions(Function* f)
		{
			if (in_clear_instructions_.count(f))
			{
				in_clear_instructions_.emplace(f);
				clear_instructions_.emplace(f);
			}
		}
	};

public:
	explicit DeadCode(Module* m) : Pass(m), func_info(std::make_shared<FuncInfo>(m)), work_list(new WorkList{})
	{
	}

	~DeadCode() override;

	void run() override;

private:
	std::shared_ptr<FuncInfo> func_info;
	WorkList* work_list;

	void mark(Function* func) const;
	void mark(const Instruction* ins) const;
	bool sweep(Function* func) const;
	// 删除不可达基本块
	// 不可达基本块的指令删除:
	// 删除可达基本块中 Phi 包括不可达基本块的定值
	// 可达基本块无法被不可达支配 -> 可达基本块中包含不可达的定值一定有 Phi 定值 -> 删除 Phi 定值就删除了所有的定值
	void clear_basic_blocks(Function* func) const;
	// 消除函数用不到的参数
	void clear_parameters(Function* func) const;
	// 删除无用的 alloca
	// 无用的 alloca 不存在任何的 load, 其也不作为函数参数
	void clear_not_use_allocas(Function* func) const;
	// 删除无用的指令(只保留关键指令)
	void clear_instructions(Function* func) const;
	bool is_critical(Instruction* ins) const;
	void sweep_globally() const;
	void eraseValue(const Value* val) const;
};

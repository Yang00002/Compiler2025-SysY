#include "Inline.hpp"

#include <queue>
#include <unordered_set>
#include <vector>

#include "BasicBlock.hpp"
#include "Instruction.hpp"

#define DEBUG 1
#include "Util.hpp"

// 处理流程：两趟处理，mark 标记有用变量，sweep 删除无用指令
void Inline::run()
{
	LOG(color::cyan("Run Inline Pass"));
	PUSH;
	for (const auto& func : m_->get_functions())
	{
		if (func->is_lib_) continue;
		removeEmptyBasicBlock(func);
	}
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("Inline Done"));
}

bool Inline::removeEmptyBasicBlock(Function* function)
{
	LOG(color::blue("Begin Remove Empty Block of ") + function->get_name());
	PUSH;
	const auto blocks = function->get_basic_blocks();
	bool rm = false;
	for (const auto block : blocks)
	{
		LOG(color::pink("Check Block ") + block->get_name());
		if (block->get_instructions().size() == 1)
		{
			if (const auto inst = block->get_instructions().back(); inst->is_br())
			{
				if (block->is_entry_block()) continue;
				if (const auto br = dynamic_cast<BranchInst*>(inst); !br->is_cond_br())
				{
					if (const auto to = dynamic_cast<BasicBlock*>(br->get_operand(0)); block->
						replace_self_with_block(to))
					{
						PUSH;
						LOG("Return Instruction " + br->print());
						LOG("Redirect use with " + to->get_name());
						POP;
						rm = true;
						function->remove(block);
					}
				}
			}
			else if (inst->is_ret())
			{
				const auto ret = dynamic_cast<ReturnInst*>(inst);
				auto& uses = block->get_use_list();
				bool stillUse = false;
				for (const auto use : uses)
				{
					const auto user = use.val_;
					if (const auto br = dynamic_cast<BranchInst*>(user); br != nullptr && !br->is_cond_br())
					{
						PUSH;
						LOG(br->get_parent()->get_name() + " Return Instruction " + br->print());
						LOG(ret->is_void_ret() ? "Replace with Return Value " : "Replace with Return Value " + ret->
							get_operand(0)->print());
						POP;
						const auto bb = br->get_parent();
						bb->replace_terminate_with_return_value(ret->is_void_ret() ? nullptr : ret->get_operand(0));
					}
					else stillUse = true;
				}
				if (!stillUse)
				{
					if (block->is_entry_block())
					{
						POP;
						return false;
					}
					function->remove(block);
					rm = true;
				}
			}
		}
	}
	POP;
	return rm;
}

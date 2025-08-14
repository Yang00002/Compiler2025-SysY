#include "InstructionSelect.hpp"

#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Instruction.hpp"

#define DEBUG 1
#include "Util.hpp"

using namespace std;

void InstructionSelect::runInner()
{
	LOG(color::cyan("InstructionSelect On ") + f_->get_name() + color::cyan(" Block ") + b_->get_name());
	auto& insts = b_->get_instructions();
	auto it = insts.begin();
	auto ed = insts.end();
	while (it != ed)
	{
		auto inst = it.get_and_add();
		if (inst->get_use_list().size() != 1) continue;
		auto u = inst->get_use_list().front();
		auto use = dynamic_cast<Instruction*>(u.val_);
		if (use->get_parent() != b_) continue;
		int id = u.arg_no_;
		if (inst->is_gep())
		{
			if (!use->is_gep()) continue;
			LOG(color::yellow("Merge"));
			LOG(inst->print());
			LOG(use->print());
			auto& opu = use->get_operands();
			assert(
				dynamic_cast<Constant*>(opu[1]) && dynamic_cast<Constant*>(opu[1])->
				getIntConstant() == 0);
			int size = u2iNegThrow(opu.size());
			for (int i = 2; i < size; i++) inst->add_operand(opu[i]);
			use->replace_all_use_with(inst);
			LOG(color::green("Get"));
			LOG(inst->print());
			LOG("");
			--it;
			b_->erase_instr(use);
			delete use;
		}
	}
}

void InstructionSelect::run()
{
	PREPARE_PASS_MSG;
	LOG(color::cyan("Run InstructionSelect Pass"));
	PUSH;
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_) continue;
		f_ = f;
		for (auto bb : f->get_basic_blocks())
		{
			b_ = bb;
			runInner();
		}
	}
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("InstructionSelect Done"));
}

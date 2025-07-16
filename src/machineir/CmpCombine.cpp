#include "CmpCombine.hpp"

#include "BasicBlock.hpp"
#include "Instruction.hpp"

#define DEBUG 1
#include "Constant.hpp"
#include "Util.hpp"

using namespace std;

enum UseType : uint8_t
{
	USE_ONCE_IN_BB,
	USE_MORE_IN_BB,
	USE_OUT_BB
};

namespace
{
	UseType getUseType(bool useOnce, bool inBB)
	{
		if (!inBB) return UseType::USE_OUT_BB;
		if (useOnce) return UseType::USE_ONCE_IN_BB;
		return USE_MORE_IN_BB;
	}
}

void CmpCombine::run()
{
	LOG(color::cyan("Run CmpCombine Pass"));
	PUSH;
	for (auto& func : m_->get_functions())
	{
		if (func->is_lib_) continue;
		std::map<Instruction*, std::pair<InstructionListIterator, UseType>> useList;
		for (auto& bb : func->get_basic_blocks())
		{
			auto& insts = bb->get_instructions();
			auto back = insts.back();
			Instruction* backUse = nullptr;
			if (back->is_br())
			{
				auto condBr = dynamic_cast<BranchInst*>(back);
				if (condBr->is_cond_br())
				{
					backUse = dynamic_cast<Instruction*>(condBr->get_operand(0));
				}
			}
			auto it = insts.begin();
			while (it != insts.end())
			{
				auto inst = *it;
				if (inst->is_cmp())
				{
					bool useOnce = inst->get_use_list().size() <= 1;
					bool inBB = backUse == inst;
					useList.emplace(inst, pair{it, getUseType(useOnce, inBB)});
				}
				++it;
			}
		}
		for (auto& [inst, pair] : useList)
		{
			LOG(color::pink("Handle Cmp ") + inst->print());
			PUSH;
			auto& [it, type] = pair;
			switch (type)
			{
				case USE_ONCE_IN_BB:
					{
						auto& l = inst->get_parent()->get_instructions();
						if (l.get_common_instruction_from_end(1) != inst)
						{
							++it;
							it.remove_pre();
							l.emplace_common_inst_from_end(inst, 1);
							LOG(color::yellow("Move to br in bb ") + inst->get_parent()->get_name());
						}
						break;
					}
				case USE_MORE_IN_BB:
					{
						auto& l = inst->get_parent()->get_instructions();
						if (l.get_common_instruction_from_end(1) != inst)
						{
							++it;
							it.remove_pre();
							l.emplace_common_inst_from_end(inst, 1);
							LOG(color::yellow("Move to br in bb ") + inst->get_parent()->get_name());
						}
						auto uses = inst->get_use_list();
						auto store = ZextInst::create_zext_to_i32(inst, nullptr);
						LOG(color::yellow("Append Store"));
						store->set_parent(inst->get_parent());
						l.emplace_common_inst_from_end(store, 1);
						for (auto use : uses)
						{
							auto usr = dynamic_cast<BranchInst*>(use.val_);
							auto bb = usr->get_parent();
							if (bb != inst->get_parent())
							{
								auto load = ICmpInst::create_eq(store, Constant::create(m_, 1), nullptr);
								load->set_parent(bb);
								bb->get_instructions().emplace_common_inst_from_end(load, 1);
								usr->set_operand(0, load);
								LOG(color::yellow("Add load in bb ") + bb->get_name());
							}
						}
						break;
					}
				case USE_OUT_BB:
					{
						auto& l = inst->get_parent()->get_instructions();
						auto uses = inst->get_use_list();
						auto store = ZextInst::create_zext_to_i32(inst, nullptr);
						store->set_parent(inst->get_parent());
						l.emplace_common_inst_after(store, it);
						LOG(color::yellow("Append Store"));
						for (auto use : uses)
						{
							auto usr = dynamic_cast<BranchInst*>(use.val_);
							auto bb = usr->get_parent();
							auto load = ICmpInst::create_eq(store, Constant::create(m_, 1), nullptr);
							load->set_parent(bb);
							bb->get_instructions().emplace_common_inst_from_end(load, 1);
							usr->set_operand(0, load);
							LOG(color::yellow("Add load in bb ") + bb->get_name());
						}
						break;
					}
			}
			POP;
		}
	}
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("CmpCombine Done"));
}

CmpCombine::CmpCombine(Module* m): Pass(m)
{
}

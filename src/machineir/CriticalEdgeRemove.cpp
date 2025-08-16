#include "CriticalEdgeRemove.hpp"

#include "BasicBlock.hpp"
#include "Instruction.hpp"

#define DEBUG 0
#include "Util.hpp"

using namespace std;

void CriticalEdgeRemove::run()
{
	LOG(color::cyan("Run CriticalEdgeRemove Pass"));
	PUSH;
	for (auto& func : m_->get_functions())
	{
		if (func->is_lib_) continue;
		auto bblist = func->get_basic_blocks();
		for (auto& bb : bblist)
		{
			if (bb->is_entry_block()) continue;
			auto phis = bb->get_instructions().phi_and_allocas();
			for (auto inst : phis)
			{
				auto phi = dynamic_cast<PhiInst*>(inst);
				auto pairs = phi->get_phi_pairs();
				for (auto& [v, preBB] : pairs)
				{
					if (preBB->get_succ_basic_blocks().size() > 1)
					{
						LOG(color::pink("Remove Critical Edge ") + preBB->get_name() + color::pink(" -> ") + bb->
							get_name());
						BasicBlock* newBB = new BasicBlock{m_, "", func};
						BranchInst::create_br(bb, newBB);
						auto br = preBB->get_instructions().back();
						br->replaceAllOperandMatchs(bb, newBB);
						bb->remove_pre_basic_block(preBB);
						preBB->remove_succ_basic_block(bb);
						preBB->add_succ_basic_block(newBB);
						newBB->add_pre_basic_block(preBB);
						for (const auto inst2 : phis)
							inst2->replaceAllOperandMatchs(preBB, newBB);
					}
				}
			}
		}
	}
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("CriticalEdgeRemove Done"));
}

CriticalEdgeRemove::CriticalEdgeRemove(PassManager* mng, Module* m): Pass(mng,m)
{
}

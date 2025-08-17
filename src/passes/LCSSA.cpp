#include "LCSSA.hpp"
#include "LoopDetection.hpp"

#define DEBUG 0
#include "Instruction.hpp"
#include "Type.hpp"
#include "Util.hpp"

using namespace std;

void LCSSA::run()
{
	LOG(m_->print());
	PREPARE_PASS_MSG;
	LOG(color::cyan("Run LCSSA Pass"));
	PUSH;
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_)continue;
		f_ = f;
		loops_ = manager_->getFuncInfo<LoopDetection>(f_);
		dominators_ = manager_->getFuncInfo<Dominators>(f_);
		runOnFunc();
	}
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("LCSSA Done"));
}

void LCSSA::runOnFunc()
{
	LOG(color::cyan("Run LCSSA On ") + f_->get_name());
	auto lps = loops_->get_loops();
	for (auto l : lps)
	{
		if (l->get_parent() == nullptr)
		{
			runOnLoops(l->get_sub_loops());
			loop_ = l;
			runOnLoop();
		}
	}
}

void LCSSA::runOnLoops(const std::vector<Loop*>& loops)
{
	for (auto loop : loops)
	{
		runOnLoops(loop->get_sub_loops());
		loop_ = loop;
		exitBlocks_.clear();
		runOnLoop();
	}
}

void LCSSA::runOnLoop()
{
	LOG("");
	LOG(color::blue("Run On Loop ") + loop_->print());
	PUSH;
	vector<Instruction*> defs;
	for (auto bb : loop_->get_blocks())
	{
		for (auto inst : bb->get_instructions())
		{
			if (inst->get_type() != Types::VOID)
			{
				defs.emplace_back(inst);
			}
		}
	}
	for (auto [i, j] : loop_->exits()) exitBlocks_.emplace(j);
	// 收集已经有的 Phi
	for (auto def : defs)
	{
		bphiMap_.clear();
		handledDef_ = def;
		auto uses = def->get_use_list();
		for (auto& use : uses)
		{
			auto useInst = dynamic_cast<PhiInst*>(use.val_);
			if (useInst == nullptr) continue;
			if (phiOnlyForLoopAtExit(useInst))
			{
				bphiMap_[useInst->get_parent()] = useInst;
			}
		}
		for (auto& use : uses)
		{
			auto useInst = dynamic_cast<Instruction*>(use.val_);
			replaceDefFor(useInst);
		}
	}

	while (!createdPhis_.empty())
	{
		auto p = createdPhis_.top();
		createdPhis_.pop();
		p->get_parent()->add_instr_begin(p);
	}

	POP;
}

bool LCSSA::phiOnlyForLoopAtExit(PhiInst* phi) const
{
	if (!exitBlocks_.count(phi->get_parent())) return false;
	int size = u2iNegThrow(phi->get_operands().size());
	for (int i = 1; i < size; i += 2)
	{
		if (handledDef_ != phi->get_operand(i)) return false;
	}
	return true;
}

void LCSSA::replaceDefFor(Instruction* target)
{
	auto parent = target->get_parent();
	if (loop_->have(parent)) return;
	if (bphiMap_[parent] == target) return;
	LOG(color::yellow("Replace Operand for ") + target->print() + color::yellow(" at ") + target->get_parent()->get_name
		());
	if (target->is_phi() && !exitBlocks_.count(parent))
	{
		int size = u2iNegThrow(target->get_operands().size()) >> 1;
		for (int i = 0; i < size; i++)
		{
			auto val = target->get_operand(i << 1);
			if (val == handledDef_)
			{
				auto bb = dynamic_cast<BasicBlock*>(target->get_operand((i << 1) + 1));
				placePhiAt(bb);
				target->set_operand(i << 1, bphiMap_[bb]);
			}
		}
	}
	else
	{
		placePhiAt(parent);
		target->replaceAllOperandMatchs(handledDef_, bphiMap_[parent]);
	}
	LOG(color::yellow("To ") + target->print());
}

void LCSSA::placePhiAt(BasicBlock* bb)
{
	if (bphiMap_[bb] != nullptr) return;
	if (exitBlocks_.count(bb))
	{
		auto phi = PhiInst::create_phi(handledDef_->get_type(), nullptr);
		LOG(color::green("Place Phi at exit ") + bb->get_name() + color::green(" for ") + handledDef_->print());
		phi->set_parent(bb);
		bb->add_instruction(phi);
		for (auto parent : bb->get_pre_basic_blocks())
		{
			phi->add_phi_pair_operand(handledDef_, parent);
		}
		bphiMap_[bb] = phi;
		return;
	}
	auto idom = dominators_->get_idom(bb);
	if (!loop_->have(idom))
	{
		placePhiAt(idom);
		bphiMap_[bb] = bphiMap_[idom];
		return;
	}
	for (auto parent : bb->get_pre_basic_blocks())
	{
		placePhiAt(parent);
	}
	LOG(color::green("Place Phi at ") + bb->get_name() + color::green(" for ") + handledDef_->print());
	auto phi = PhiInst::create_phi(handledDef_->get_type(), nullptr);
	phi->set_parent(bb);
	createdPhis_.emplace(phi);
	for (auto parent : bb->get_pre_basic_blocks())
	{
		phi->add_phi_pair_operand(bphiMap_[parent], parent);
	}
	bphiMap_[bb] = phi;
}

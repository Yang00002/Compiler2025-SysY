#include "ReturnMerge.hpp"

#include "CodeString.hpp"
#include "Config.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"


bool ReturnMerge::shoudlMerge() const
{
	int suffixLen = f_->funcSuffix_->lines();
	int count = 0;
	for (auto i : f_->blocks())
	{
		if (i->suc_bbs().empty()) count++;
	}
	if (count == 1) return false;
	int allLen = suffixLen * count;
	if (allLen <= suffixLen + 1) return false;
	return allLen >= epilogShouldMerge;
}

void ReturnMerge::inlineEpilog() const
{
	auto sf = f_->funcSuffix_;
	for (auto i : f_->blocks())
	{
		if (i->suc_bbs().empty())
		{
			auto ret = i->instructions().back()->str;
			ret->copyBefore(sf);
		}
	}
	delete sf;
	f_->funcSuffix_ = nullptr;
}

void ReturnMerge::mergeEpilog() const
{
	auto sf = f_->funcSuffix_;
	auto bb = new MBasicBlock{f_->name() + "_epilog", f_};
	bb->id_ = u2iNegThrow(f_->blocks().size());
	auto addr = BlockAddress::get(bb);
	for (auto i : f_->blocks())
	{
		if (i->suc_bbs().empty())
		{
			auto& insts = i->instructions();
			delete insts.back();
			auto inst = new MB{i, addr};
			inst->str = new CodeString{};
			insts.back() = inst;
			i->suc_bbs_.emplace(bb);
			bb->pre_bbs_.emplace(i);
		}
	}
	auto inst = new MRet{ bb };
	bb->instructions_.emplace_back(inst);
	inst->str = sf;
	sf->addInstruction("RET");
	f_->blocks().emplace_back(bb);
	f_->funcSuffix_ = nullptr;
}

void ReturnMerge::run()
{
	for (auto i : m_->functions())
	{
		f_ = i;
		if (shoudlMerge()) mergeEpilog();
		else inlineEpilog();
	}
}

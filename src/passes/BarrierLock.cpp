#include "BarrierLock.hpp"

#include "Dominators.hpp"
#include "Instruction.hpp"

void BarrierLock::run()
{
	Dominators* dom = manager_->getFuncInfo<Dominators>(f_);
	auto& fs = f_->get_parent()->get_functions();
	auto start = fs.front();
	Function* f = nullptr;
	auto it = fs.begin();
	++it;
	f = *it;
	if (start->get_use_list().empty())
	{
		for (auto b : f_->get_basic_blocks()) inner.emplace(b);
		return;
	}
	auto sb = dynamic_cast<Instruction*>(start->get_use_list().front().val_)->get_parent();
	if (sb->get_parent() != f_)
	{
		for (auto b : f_->get_basic_blocks()) inner.emplace(b);
		return;
	}
	auto eb = dynamic_cast<Instruction*>(f->get_use_list().front().val_)->get_parent();
	inner.emplace(sb);
	inner.emplace(eb);
	for (auto bb : f_->get_basic_blocks())
	{
		if (bb == sb) continue;
		if (bb == eb) continue;
		if (dom->is_dominate(sb, bb))
		{
			if (dom->is_dominate(eb, bb)) down.emplace(bb);
			else inner.emplace(bb);
		}
		else up.emplace(bb);
	}
}

bool BarrierLock::isInner(BasicBlock* bb) const
{
	return inner.count(bb);
}

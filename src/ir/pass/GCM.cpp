#include "GCM.hpp"

#include "FuncInfo.hpp"
#include "LoopDetection.hpp"

#define DEBUG 0
#include "Util.hpp"

bool GlobalCodeMotion::is_pinned(const Instruction* i) const
{
	if (i->is_call())
	{
		Function* f = dynamic_cast<Function*>(i->get_operand(0));
		if (info_->useOrIsImpureLib(f) || !info_->storeDetail(f).empty()) return true;
		return false;
	}
	return i->is_alloca() || i->is_br() || i->is_ret() || i->is_phi() || i->is_store() || i->is_memcpy() || i->
	       is_memclear();
}

BasicBlock* GlobalCodeMotion::lcaOfUse(Instruction* i) const
{
	std::unordered_set<Instruction*> afterMe;
	for (auto use : i->get_use_list())
	{
		// phi 使用应该将位置提前
		auto inst = dynamic_cast<Instruction*>(use.val_);
		if (inst->is_phi())
		{
			auto phi = dynamic_cast<PhiInst*>(inst);
			int size = u2iNegThrow(phi->get_operands().size());
			for (int j = 0; j < size; j += 2)
			{
				if (phi->get_operand(j) == i)
				{
					auto bb = dynamic_cast<BasicBlock*>(phi->get_operand(j + 1));
					afterMe.emplace(bb->get_instructions().back());
				}
			}
		}
		else
			afterMe.emplace(inst);
	}
	if (loadInst_.count(i))
	{
		auto& lds = loadInst_.at(i);
		for (auto ld : lds)
		{
			if (storeInst_.count(ld))
			{
				for (auto st : storeInst_.at(ld))
				{
					if (dom_->is_dominate(i->get_parent(), st->get_parent()))
						afterMe.emplace(st);
				}
			}
		}
	}
	// 该位置可能位于某个循环内, 需要拿出来
	auto ret = dom_->lca(afterMe);
	auto rl = loops_->loopOfBlock(ret);
	auto ro = loops_->loopOfBlock(i->get_parent());
	if (rl != nullptr)
	{
		while (rl != ro)
		{
			ret = dom_->get_idom(rl->get_header());
			rl = loops_->loopOfBlock(ret);
		}
	}
	return ret;
}

void GlobalCodeMotion::runInner()
{
	visited_.clear();
	collectLoadStores();
	for (auto bb : f_->get_basic_blocks())
	{
		for (auto inst : bb->get_instructions())
		{
			if (!is_pinned(inst))
			{
				visited_.emplace(inst);
				worklist_.emplace(inst);
			}
		}
	}
	while (!worklist_.empty())
	{
		auto i = worklist_.front();
		worklist_.pop();
		visited_.erase(i);
		auto bb = lcaOfUse(i);
		if (bb == i->get_parent()) continue;
		postpone(i, bb);
	}
}

void GlobalCodeMotion::collectLoadStores()
{
	loadInst_.clear();
	storeInst_.clear();
	for (auto bb : f_->get_basic_blocks())
	{
		for (auto inst : bb->get_instructions())
		{
			if (inst->is_call())
			{
				auto& sts = info_->storeDetail(dynamic_cast<Function*>(inst->get_operand(0)));
				for (auto s : sts.arguments_)
				{
					auto argIn = ptrFrom(inst->get_operand(s->get_arg_no() + 1));
					if (argIn != nullptr) storeInst_[argIn].emplace(inst);
				}
				for (auto s : sts.globals_)
					storeInst_[s].emplace(inst);
				// 不考虑被 pin 的
				if (!is_pinned(inst))
				{
					auto& lds = info_->loadDetail(dynamic_cast<Function*>(inst->get_operand(0)));
					for (auto s : lds.arguments_)
					{
						auto argIn = ptrFrom(inst->get_operand(s->get_arg_no() + 1));
						if (argIn != nullptr) loadInst_[inst].emplace(argIn);
					}
					for (auto s : lds.globals_)
						loadInst_[inst].emplace(s);
				}
			}
			else if (inst->is_store() || inst->is_memcpy())
			{
				auto argIn = ptrFrom(inst->get_operand(1));
				if (argIn != nullptr) storeInst_[argIn].emplace(inst);
			}
			else if (inst->is_memclear())
			{
				auto argIn = ptrFrom(inst->get_operand(0));
				if (argIn != nullptr) storeInst_[argIn].emplace(inst);
			}
			else if (inst->is_load())
			{
				auto argIn = ptrFrom(inst->get_operand(0));
				if (argIn != nullptr) loadInst_[inst].emplace(argIn);
			}
		}
	}
}

void GlobalCodeMotion::postpone(Instruction* i, BasicBlock* bb)
{
	LOG(color::yellow("PostPone ") + i->print() + color::yellow(" from ") + i->get_parent()->get_name() + color::yellow(" to ") + bb->get_name());
	auto origin = i->get_parent();
	origin->erase_instr(i);
	bb->add_instr_begin(i);
	i->set_parent(bb);
	for (auto op : i->get_operands())
	{
		auto inst = dynamic_cast<Instruction*>(op);
		if (inst != nullptr && !is_pinned(inst) && !visited_.count(inst))
		{
			visited_.emplace(inst);
			worklist_.emplace(inst);
		}
	}
}

void GlobalCodeMotion::run()
{
	LOG(m_->print());
	PREPARE_PASS_MSG;
	LOG(color::cyan("Run GCM Pass"));
	PUSH;
	info_ = manager_->getGlobalInfo<FuncInfo>();
	for (auto f : m_->get_functions())
	{
		if (!f->is_lib_ && f->get_num_basic_blocks() > 1)
		{
 			dom_ = manager_->getFuncInfo<Dominators>(f);
			loops_ = manager_->getFuncInfo<LoopDetection>(f);
			GAP;
			RUN(loops_->print());
			GAP;
			f_ = f;
			runInner();
		}
	}
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("GCM Done"));
}

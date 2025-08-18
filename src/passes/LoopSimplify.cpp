#include "LoopSimplify.hpp"

#include "LoopDetection.hpp"

#define DEBUG 0
#include <algorithm>
#include <queue>

#include "Instruction.hpp"
#include "Util.hpp"

using namespace std;

LoopSimplify::LoopSimplify(PassManager* mng, Module* m)
	: Pass(mng, m), loops_(nullptr), f_(nullptr)
{
}

void LoopSimplify::run()
{
	LOG(m_->print());
	PREPARE_PASS_MSG;
	LOG(color::cyan("Run LoopSimplify Pass"));
	PUSH;
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_)continue;
		f_ = f;
		loops_ = manager_->getFuncInfo<LoopDetection>(f_);
		runOnFunc();
	}
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("LoopSimplify Done"));
}

void LoopSimplify::runOnFunc()
{
	LOG(color::cyan("Run LoopSimplify On ") + f_->get_name());
	LOG(color::green("Detect Loops:"));
	GAP;
	RUN(loops_->print());
	GAP;
	auto lps = loops_->get_loops();
	bool change = false;
	for (auto l : lps)
	{
		if (l->get_parent() == nullptr)
		{
			change |= createPreHeaderAndLatchOnLoops(l->get_sub_loops());
			change |= createPreHeaderAndLatchOnLoop(l);
		}
	}
	for (auto l : loops_->get_loops())
	{
		if (l->get_parent() == nullptr)
		{
			change |= createExitOnLoops(l->get_sub_loops());
			change |= createExitOnLoop(l);
		}
	}
	for (auto l : loops_->get_loops())
	{
		if (l->get_parent() == nullptr)
		{
			change |= createLatchOnLoops(l->get_sub_loops());
			change |= createLatchOnLoop(l);
		}
	}
	if (change) manager_->flushFuncInfo<Dominators>(f_);
}

bool LoopSimplify::createPreHeaderAndLatchOnLoops(const std::vector<Loop*>& loops)
{
	bool ret = false;
	for (auto loop : loops)
	{
		ret |= createPreHeaderAndLatchOnLoops(loop->get_sub_loops());
		ret |= createPreHeaderAndLatchOnLoop(loop);
	}
	return ret;
}

bool LoopSimplify::createPreHeaderAndLatchOnLoop(Loop* loop) const
{
	bool ret = false;
	LOG(color::pink("Create PreHeader and Latch on Loop ") + loop->print());
	auto head = loop->get_header();
	vector<pair<int, BasicBlock*>> latches;
	// 给子循环排序
	for (auto i : loop->get_latches())
	{
		latches.emplace_back(loops_->costOfLatch(loop, i, manager_->getFuncInfo<Dominators>(f_)), i);
	}
	sort(latches.begin(), latches.end(), [](const pair<int, BasicBlock*>& l, const pair<int, BasicBlock*>& r)-> bool
	{
		return l.first < r.first;
	});
	for (auto& [i, block] : latches)
	{
		LOG(color::yellow("Handling Latches ") + block->get_name());
		int preSize = u2iNegThrow(head->get_pre_basic_blocks().size());
		ASSERT(preSize >= 2);
		ret = true;
		auto preHeader = new BasicBlock{m_, "", f_};
		head->add_block_before(preHeader, block);
		auto lp = loop->get_parent();
		while (lp != nullptr)
		{
			lp->add_block(preHeader);
			lp = lp->get_parent();
		}
		if (loop->get_latches().size() == 1)
		{
			loop->set_preheader(preHeader);
			loop->add_block_casecade(preHeader, false);
			auto dm = manager_->getFuncInfo<Dominators>(f_);
			dm->set_idom(preHeader, dm->get_idom(head));
			dm->set_idom(head, preHeader);
			break;
		}
		auto innerLoop = new Loop{loops_, head};
		loops_->collectInnerLoopMessage(loop, block, preHeader, innerLoop, manager_->getFuncInfo<Dominators>(f_));
		head = preHeader;
		LOG(color::green("Update Loop to ") + loop->print());
		LOG(color::green("Sub Get ") + innerLoop->print());
	}
	return ret;
}

bool LoopSimplify::createExitOnLoops(const std::vector<Loop*>& loops)
{
	bool ret = false;
	for (auto loop : loops)
	{
		ret |= createExitOnLoops(loop->get_sub_loops());
		ret |= createExitOnLoop(loop);
	}
	return ret;
}

bool LoopSimplify::createExitOnLoop(Loop* loop) const
{
	bool ret = false;
	LOG(color::pink("Create Exit on Loop ") + loop->print());
	auto& bbs = loop->get_blocks();
	for (auto bb : bbs)
	{
		for (auto suc : bb->get_succ_basic_blocks())
		{
			if (!bbs.count(suc))
			{
				unordered_set<BasicBlock*> pres;
				unordered_set<BasicBlock*> outerPres;
				bool ok = true;
				for (auto pb : suc->get_pre_basic_blocks())
				{
					if (loop->get_blocks().count(pb))pres.emplace(pb);
					else
					{
						outerPres.emplace(pb);
						ok = false;
					}
				}
				if (ok)
				{
					for (auto i : pres) loop->add_exit(i, suc); // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
					break;
				}
				auto nbb = new BasicBlock{m_, "", f_};
				suc->add_block_before(nbb, outerPres);
				for (auto i : pres) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
					LoopDetection::addNewExitTo(loop, i, nbb, suc);
				ret = true;
				break;
			}
		}
	}
	LOG(color::green("Update Loop to ") + loop->print());
	return ret;
}

bool LoopSimplify::createLatchOnLoops(const std::vector<Loop*>& loops)
{
	bool ret = false;
	for (auto loop : loops)
	{
		ret |= createLatchOnLoops(loop->get_sub_loops());
		ret |= createLatchOnLoop(loop);
	}
	return ret;
}

bool LoopSimplify::createLatchOnLoop(Loop* loop) const
{
	LOG(color::pink("Create Latch on Loop ") + loop->print());
	auto preLatch = loop->get_latch();
	if (preLatch->get_succ_basic_blocks().size() == 1) return false;
	auto nextLatch = BasicBlock::create(m_, "", f_);
	auto head = loop->get_header();
	head->add_block_before(nextLatch, loop->get_preheader());
	loop->add_block_casecade(nextLatch, true);
	loop->remove_latch(preLatch);
	loop->add_latch(nextLatch);
	LOG(color::green("Update Loop to ") + loop->print());
	return true;
}

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
		if (preSize == 2)
		{
			bool b = false;
			for (auto pre : head->get_pre_basic_blocks())
			{
				if (!loop->get_blocks().count(pre))
				{
					if (pre->get_succ_basic_blocks().size() == 1)
					{
						loop->set_preheader(pre);
						b = true;
					}
					break;
				}
			}
			if (b) break;
		}
		ret = true;
		auto preHeader = new BasicBlock{m_, "", f_};
		auto pres = head->get_pre_basic_blocks();
		for (auto pre : pres)
		{
			if (pre != block)
			{
				pre->get_instructions().back()->replaceAllOperandMatchs(head, preHeader);
				pre->remove_succ_basic_block(head);
				head->remove_pre_basic_block(pre);
				pre->add_succ_basic_block(preHeader);
				preHeader->add_pre_basic_block(pre);
			}
		}
		BranchInst::create_br(head, preHeader);
		auto insts = head->get_instructions().phi_and_allocas();
		auto it = insts.begin();
		while (it != insts.end())
		{
			auto phi = dynamic_cast<PhiInst*>(it.get_and_add());
			std::map<BasicBlock*, Value*> phiMap;
			Value* v = nullptr;
			Value* lv = nullptr;
			int preCount = 0;
			for (auto& [i1, j1] : phi->get_phi_pairs())
			{
				if (block != j1)
				{
					preCount++;
					phiMap[j1] = i1;
					v = i1;
				}
				else lv = i1;
			}
			phi->remove_all_operands();
			phi->add_phi_pair_operand(lv, block);
			if (preCount > 1)
			{
				auto phi2 = PhiInst::create_phi(v->get_type(), preHeader);
				preHeader->add_instruction(phi2);
				for (auto& [bb, value] : phiMap)
				{
					phi2->add_phi_pair_operand(value, bb);
				}
				phi->add_phi_pair_operand(phi2, preHeader);
			}
			else
			{
				for (auto& [bb, value] : phiMap)
				{
					phi->add_phi_pair_operand(value, preHeader);
				}
			}
		}
		auto lp = loop->get_parent();
		while (lp != nullptr)
		{
			lp->add_block(preHeader);
			lp = lp->get_parent();
		}
		if (loop->get_latches().size() == 1)
		{
			loop->set_preheader(preHeader);
			auto dm = manager_->getFuncInfo<Dominators>(f_);
			dm->set_idom(preHeader, dm->get_idom(head));
			dm->set_idom(head, preHeader);
			break;
		}
		auto innerLoop = new Loop{head};
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
			if (!bbs.count(suc) && !loop->exits().count(suc))
			{
				bool ok = true;
				unordered_set<BasicBlock*> pres;
				for (auto pb : suc->get_pre_basic_blocks())
				{
					if (loop->get_blocks().count(pb))pres.emplace(pb);
					else ok = false;
				}
				if (ok)
				{
					for (auto i : pres) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
						loop->addExit(i, suc);
				}
				else
				{
					auto nbb = new BasicBlock{m_, "", f_};
					for (auto pre : pres) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
					{
						pre->get_instructions().back()->replaceAllOperandMatchs(suc, nbb);
						pre->remove_succ_basic_block(suc);
						suc->remove_pre_basic_block(pre);
						pre->add_succ_basic_block(nbb);
						nbb->add_pre_basic_block(pre);
					}
					BranchInst::create_br(suc, nbb);
					for (auto inst : suc->get_instructions().phi_and_allocas())
					{
						auto phi = dynamic_cast<PhiInst*>(inst);
						std::map<BasicBlock*, Value*> nbbPhiMap;
						std::map<BasicBlock*, Value*> sucPhiMap;
						for (auto& [i1, j1] : phi->get_phi_pairs())
						{
							if (pres.count(j1))
							{
								nbbPhiMap.emplace(j1, i1);
							}
							else
							{
								sucPhiMap.emplace(j1, i1);
							}
						}
						if (nbbPhiMap.size() == 1)
						{
							phi->replaceAllOperandMatchs(bb, nbb);
						}
						else
						{
							auto nPhi = PhiInst::create_phi(phi->get_type(), nbb);
							sucPhiMap.emplace(nbb, nPhi);
							phi->remove_all_operands();
							for (auto [i, j] : nbbPhiMap) nPhi->add_phi_pair_operand(j, i);
							for (auto [i, j] : sucPhiMap) phi->add_phi_pair_operand(j, i);
						}
					}
					for (auto i : pres) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
						LoopDetection::addNewExitTo(loop, i, nbb, suc);
					ret = true;
				}
				break;
			}
		}
	}
	LOG(color::green("Update Loop to ") + loop->print());
	return ret;
}

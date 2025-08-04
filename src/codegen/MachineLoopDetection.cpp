#include "MachineLoopDetection.hpp"
#include "BasicBlock.hpp"
#include "Dominators.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <queue>
#include <stack>

#include "MachineFunction.hpp"

#define DEBUG 0
#include "Type.hpp"
#include "Util.hpp"


namespace
{
	class LTDominatorTreeSolver
	{
		MFunction* func_;
		int* edges0_;
		int* idToEdgeIdxs2_;
		int beginBlockId_, blockCount_;
		int *blockIdRankByDFSEnteringOrder1_, *DFSEnteringOrderOfblockId1_, *DFSParent1_;
		int *setParent1_, *mn1_;
		int *idom1_, *sdom1_;
		int edgeLinkingSize_;

		struct EL
		{
			int v, x;
		};

		EL* edgeLinking1_;
		int* edgeLinkingIdx1_;

		void dfs(const int u, int& idx)
		{
			DFSEnteringOrderOfblockId1_[u] = ++idx;
			blockIdRankByDFSEnteringOrder1_[idx] = u;
			for (int i = idToEdgeIdxs2_[u << 1]; i < idToEdgeIdxs2_[(u << 1) + 1]; i++)
			{
				if (const int v = edges0_[i]; !DFSEnteringOrderOfblockId1_[v])
				{
					dfs(v, idx);
					DFSParent1_[v] = u;
				}
			}
		}

		int findSetRootOf(int x)
		{
			if (setParent1_[x] == x)
			{
				return x;
			}
			int tmp = setParent1_[x];
			setParent1_[x] = findSetRootOf(setParent1_[x]);
			if (DFSEnteringOrderOfblockId1_[sdom1_[mn1_[tmp]]] < DFSEnteringOrderOfblockId1_[sdom1_[mn1_[x]]])
			{
				mn1_[x] = mn1_[tmp];
			}
			return setParent1_[x];
		}

		void add(int from, int to)
		{
			edgeLinking1_[++edgeLinkingSize_] = {to, edgeLinkingIdx1_[from]};
			edgeLinkingIdx1_[from] = edgeLinkingSize_;
		}

	public:
		LTDominatorTreeSolver(const LTDominatorTreeSolver&) = delete;
		LTDominatorTreeSolver(LTDominatorTreeSolver&&) = delete;
		LTDominatorTreeSolver& operator=(const LTDominatorTreeSolver&) = delete;
		LTDominatorTreeSolver& operator=(LTDominatorTreeSolver&&) = delete;

		LTDominatorTreeSolver(MFunction* function)
		{
			func_ = function;
			blockCount_ = u2iNegThrow(function->blocks().size());
			int edgeCount = 0;
			for (const auto& block : function->blocks())
			{
				edgeCount += u2iNegThrow(block->suc_bbs().size());
			}
			edges0_ = new int[edgeCount << 1];
			idToEdgeIdxs2_ = new int[(blockCount_ << 1) + 3];
			int allocateId = 0;
			for (int i = 1; i <= blockCount_; i++)
			{
				auto& block = function->blocks()[i - 1];
				idToEdgeIdxs2_[i << 1] = allocateId;
				for (auto& succ_basic_block : block->suc_bbs())
				{
					edges0_[allocateId++] = succ_basic_block->id() + 1;
				}
				idToEdgeIdxs2_[1 + (i << 1)] = allocateId;
				for (auto& succ_basic_block : block->pre_bbs())
				{
					edges0_[allocateId++] = succ_basic_block->id() + 1;
				}
			}
			idToEdgeIdxs2_[(blockCount_ << 1) + 2] = allocateId;
			beginBlockId_ = 1;
			blockIdRankByDFSEnteringOrder1_ = new int[blockCount_ + 1];
			DFSEnteringOrderOfblockId1_ = new int[blockCount_ + 1]{};
			DFSParent1_ = new int[blockCount_ + 1];
			setParent1_ = new int[blockCount_ + 1];
			edgeLinkingSize_ = 0;
			edgeLinking1_ = new EL[blockCount_ + 1];
			edgeLinkingIdx1_ = new int[blockCount_ + 1]{};
			mn1_ = new int[blockCount_ + 1];
			idom1_ = new int[blockCount_ + 1];
			sdom1_ = new int[blockCount_ + 1];
		}

		~LTDominatorTreeSolver()
		{
			delete[] edges0_;
			delete[] idToEdgeIdxs2_;
			delete[] blockIdRankByDFSEnteringOrder1_;
			delete[] DFSEnteringOrderOfblockId1_;
			delete[] DFSParent1_;
			delete[] setParent1_;
			delete[] mn1_;
			delete[] idom1_;
			delete[] sdom1_;
			delete[] edgeLinking1_;
			delete[] edgeLinkingIdx1_;
		}

		void solve()
		{
			int idx = 0;
			dfs(beginBlockId_, idx);
			for (int i = 1; i <= blockCount_; ++i)
			{
				idom1_[i] = setParent1_[i] = sdom1_[i] = mn1_[i] = i;
			}
			for (int i = blockCount_; i >= 2; --i)
			{
				int u = blockIdRankByDFSEnteringOrder1_[i], res = INT_MAX;
				for (int j = idToEdgeIdxs2_[(u << 1) + 1]; j < idToEdgeIdxs2_[(u << 1) + 2]; j++)
				{
					int v = edges0_[j];
					if (!DFSEnteringOrderOfblockId1_[v])
					{
						continue;
					}
					findSetRootOf(v);
					if (DFSEnteringOrderOfblockId1_[v] < DFSEnteringOrderOfblockId1_[u])
					{
						res = std::min(res, DFSEnteringOrderOfblockId1_[v]);
					}
					else
					{
						res = std::min(res, DFSEnteringOrderOfblockId1_[sdom1_[mn1_[v]]]);
					}
				}
				sdom1_[u] = blockIdRankByDFSEnteringOrder1_[res];
				setParent1_[u] = DFSParent1_[u];
				add(sdom1_[u], u);
				u = DFSParent1_[u];
				for (int j = edgeLinkingIdx1_[u]; j; j = edgeLinking1_[j].x)
				{
					int v = edgeLinking1_[j].v;
					findSetRootOf(v);
					if (sdom1_[mn1_[v]] == u)
					{
						idom1_[v] = u;
					}
					else
					{
						idom1_[v] = mn1_[v];
					}
				}
				edgeLinkingIdx1_[u] = 0;
			}
			for (int i = 2; i <= blockCount_; ++i)
			{
				int u = blockIdRankByDFSEnteringOrder1_[i];
				if (idom1_[u] != sdom1_[u])
				{
					idom1_[u] = idom1_[idom1_[u]];
				}
			}
		}

		void dumpIdom(std::vector<MBasicBlock*>& ret) const
		{
			ret.resize(blockCount_);
			for (int i = 1; i <= blockCount_; i++)
				ret[i - 1] = func_->blocks()[idom1_[i] - 1];
		}

		void dumpTreeSucc(std::vector<DynamicBitset>& ret) const
		{
			ret.clear();
			ret.reserve(blockCount_);
			for (int i = 0; i < blockCount_; i++) ret.emplace_back(blockCount_);
			for (int i = 1; i <= blockCount_; i++)
			{
				if (idom1_[i] == i) continue;
				ret[idom1_[i] - 1].set(func_->blocks()[i - 1]->id());
			}
		}

		void dumpFrontier(std::vector<DynamicBitset>& ret) const
		{
			ret.clear();
			ret.reserve(blockCount_);
			for (int i = 0; i < blockCount_; i++) ret.emplace_back(blockCount_);
			for (int i = 1; i <= blockCount_; i++)
			{
				int id = idom1_[i];
				int preBegin = idToEdgeIdxs2_[(i << 1) + 1];
				int preEnd = idToEdgeIdxs2_[(i << 1) + 2];
				if (preEnd - preBegin > 1)
				{
					for (; preBegin < preEnd; ++preBegin)
					{
						int p = edges0_[preBegin];
						while (p != id)
						{
							ret[p - 1].set(func_->blocks()[i - 1]->id());
							p = idom1_[p];
						}
					}
				}
			}
		}
	};
}


void MachineDominators::create_dom_dfs_order()
{
	// 分析得到 f 中各个基本块的支配树上的dfs序L,R
	int order = 0;
	std::stack<MBasicBlock*> dfsWorkList;
	std::stack<bool> dfsVisitList;
	dfsWorkList.emplace(func_->blocks()[0]);
	dfsVisitList.emplace(false);

	dom_tree_L_.resize(func_->blocks().size());
	dom_tree_R_.resize(func_->blocks().size());
	dom_dfs_order_.clear();

	// 这种非递归 DFS 只对树生效, 图上的去看 LiveMessage 的写法
	while (!dfsWorkList.empty())
	{
		if (dfsVisitList.top())
		{
			dfsVisitList.pop();
			auto bb = dfsWorkList.top();
			dfsWorkList.pop();
			dom_tree_R_[bb->id()] = order;
			continue;
		}
		dfsVisitList.top() = true;
		auto bb = dfsWorkList.top();
		dom_tree_L_[bb->id()] = ++order;
		dom_dfs_order_.emplace_back(bb);
		for (auto i : dom_tree_succ_blocks_[bb->id()])
		{
			dfsWorkList.emplace(func_->blocks()[i]);
			dfsVisitList.emplace(false);
		}
	}
	dom_post_order_ = std::vector(dom_dfs_order_.rbegin(),
	                              dom_dfs_order_.rend());
}

void MachineDominators::run()
{
}

void MachineDominators::run_on_func(MFunction* f)
{
	func_ = f;
	LTDominatorTreeSolver solver{f};
	solver.solve();
	solver.dumpIdom(idom_);
	solver.dumpTreeSucc(dom_tree_succ_blocks_);
	solver.dumpFrontier(dom_frontier_);
	create_dom_dfs_order();
}

MBasicBlock* MachineDominators::get_idom(const MBasicBlock* bb) const { return idom_[bb->id()]; }

const DynamicBitset& MachineDominators::get_dominance_frontier(const MBasicBlock* bb)
{
	return dom_frontier_[bb->id()];
}

const DynamicBitset& MachineDominators::get_dom_tree_succ_blocks(const MBasicBlock* bb)
{
	return dom_tree_succ_blocks_[bb->id()];
}

bool MachineDominators::is_dominate(const MBasicBlock* bb1, const MBasicBlock* bb2) const
{
	return dom_tree_L_[bb1->id()] <= dom_tree_L_[bb2->id()] &&
	       dom_tree_R_[bb1->id()] >= dom_tree_R_[bb2->id()];
}

const std::vector<MBasicBlock*>& MachineDominators::get_dom_dfs_order()
{
	return dom_dfs_order_;
}

const std::vector<MBasicBlock*>& MachineDominators::get_dom_post_order()
{
	return dom_post_order_;
}

MachineLoopDetection::~MachineLoopDetection()
{
	for (const auto& loop : loops_)
	{
		delete loop;
	}
}

void MachineLoopDetection::run()
{
}

void MachineLoopDetection::discover_loop_and_sub_loops(const MBasicBlock* bb, const DynamicBitset& latches,
                                                       MachineLoop* loop)
{
	std::stack<MBasicBlock*> work_list;
	for (auto i : latches) work_list.emplace(func_->blocks()[i]);

	while (!work_list.empty())
	{
		// 当工作表非空时继续处理
		auto subbb = work_list.top();
		work_list.pop();
		// DONE-1: 处理未分配给任何循环的节点
		if (bb_to_loop_[subbb->id()] == nullptr)
		{
			loop->add_block(subbb->id());
			bb_to_loop_[subbb->id()] = loop;
			for (auto pre : subbb->pre_bbs())
			{
				if (dominators_.is_dominate(bb, pre))
				{
					work_list.emplace(pre);
				}
			}
		}
		// DONE-2: 处理已属于其他循环的节点
		else if (bb_to_loop_[subbb->id()] != loop)
		{
			auto subloop = bb_to_loop_[subbb->id()];
			auto parent = subloop->get_parent();
			while (parent != nullptr)
			{
				subloop = parent;
				parent = parent->get_parent();
			}
			if (subloop != loop)
			{
				loop->add_sub_loop(subloop);
				subloop->set_parent(loop);
				for (auto pb : subloop->get_blocks())
				{
					loop->add_block(pb);
				}
				for (auto pre : subloop->get_header()->pre_bbs())
				{
					if (dominators_.is_dominate(bb, pre))
					{
						work_list.emplace(pre);
					}
				}
			}
		}
	}
}


void MachineLoopDetection::removeUnreachable() const
{
	auto& bs = func_->blocks();
	auto bc = u2iNegThrow(bs.size());
	DynamicBitset vis{bc};
	vis.set(0);
	std::queue<MBasicBlock*> blocks;
	blocks.emplace(bs[0]);
	while (!blocks.empty())
	{
		auto bb = blocks.front();
		blocks.pop();
		for (auto i : bb->suc_bbs())
		{
			if (!vis.test(i->id()))
			{
				vis.set(i->id());
				blocks.emplace(i);
			}
		}
	}
	LOG(vis.print());
	vis.reverse();
	LOG(vis.print());
	for (auto bb : bs)
	{
		for (auto i : vis)
		{
			auto bb0 = bs[i];
			bb->suc_bbs().erase(bb0);
			bb->pre_bbs().erase(bb0);
		}
	}
	for (int i = 0; i < bc; i++)
	{
		auto bb = func_->blocks()[i];
		if (!vis.test(bb->id()))
		{
			func_->blocks()[i]->id_ = i;
			func_->blocks()[i]->name_ = func_->name() + "_" + std::to_string(i);
		}
		else
		{
			func_->blocks().erase(func_->blocks().begin() + i);
			delete bb;
			bc--;
			i--;
		}
	}
}

void MachineLoopDetection::run_on_func(MFunction* f)
{
	func_ = f;
	removeUnreachable();
	for (auto i : loops_) delete i;
	loops_.clear();
	bb_to_loop_.resize(f->blocks().size());
	for (auto& i : bb_to_loop_) i = nullptr;
	dominators_.run_on_func(f);
	for (auto bb : dominators_.get_dom_post_order())
	{
		DynamicBitset latches{u2iNegThrow(f->blocks().size())};
		for (auto& pred : bb->pre_bbs())
		{
			if (dominators_.is_dominate(bb, pred))
			{
				latches.set(pred->id());
			}
		}
		if (latches.allZeros())
		{
			continue;
		}
		// create loop
		auto loop = new MachineLoop{bb};
		bb_to_loop_[bb->id()] = loop;
		// add latch nodes
		for (auto latch : latches)
		{
			loop->add_latch(latch);
		}
		loops_.push_back(loop);
		discover_loop_and_sub_loops(bb, latches, loop);
	}
}

void MachineLoopDetection::print() const
{
	std::cout << "Loop Detection Result:" << '\n';
	for (auto& loop : loops_)
	{
		std::cout << "\nLoop header: " << loop->get_header()->id()
			<< '\n';
		std::cout << "Loop blocks: ";
		for (auto bb : loop->get_blocks())
		{
			std::cout << func_->blocks()[bb]->id() << " ";
		}
		std::cout << '\n';
		std::cout << "Sub loops: ";
		for (auto& sub_loop : loop->get_sub_loops())
		{
			std::cout << sub_loop->get_header()->id() << " ";
		}
		std::cout << '\n';
	}
}

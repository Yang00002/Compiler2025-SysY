#include "MachineLoopDetection.hpp"
#include "BasicBlock.hpp"
#include "Dominators.hpp"
#include <iostream>
#include <ostream>
#include <queue>
#include <stack>

#include "MachineFunction.hpp"

#define DEBUG 0
#include "Util.hpp"


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

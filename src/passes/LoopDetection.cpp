#include "LoopDetection.hpp"
#include "BasicBlock.hpp"
#include "Dominators.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <set>

#include "Util.hpp"


LoopDetection::~LoopDetection()
{
	for (const auto& loop : loops_)
	{
		delete loop;
	}
}

void LoopDetection::run()
{
	Dominators* dominators = manager_->getFuncInfo<Dominators>(f_);
	for (auto bb : dominators->get_dom_post_order(f_))
	{
		BBset latches;
		for (auto& pred : bb->get_pre_basic_blocks())
		{
			if (dominators->is_dominate(bb, pred))
			{
				// pred is a back edge
				// pred -> bb , pred is the latch node
				latches.insert(pred);
			}
		}
		if (latches.empty())
		{
			continue;
		}
		// create loop
		auto loop = new Loop{bb};
		bb_to_loop_[bb] = loop;
		// add latch nodes
		for (auto& latch : latches)
		{
			loop->add_latch(latch);
		}
		loops_.push_back(loop);
		discover_loop_and_sub_loops(bb, latches, loop, dominators);
	}
}

void Loop::remove_sub_loop(const Loop* loop)
{
	auto it = sub_loops_.begin();
	auto ed = sub_loops_.end();
	while (it != ed)
	{
		if (*it == loop)
		{
			sub_loops_.erase(it);
			return;
		}
		++it;
	}
}

std::string Loop::print() const
{
	std::string ret;
	std::unordered_set<BasicBlock*> subs;
	for (auto l : sub_loops_) subs.emplace(l->header_);
	for (auto bb : blocks_)
	{
		if (bb->get_name().empty()) bb->get_module()->set_print_name();
		ret += bb->get_name();
		if (bb == header_) ret += "<H>";
		if (latches_.count(bb)) ret += "<L>";
		if (subs.count(bb)) ret += "<S>";
		if (exits_.count(bb)) ret += "<E>";
		ret += ", ";
	}
	if (!ret.empty())
	{
		ret.pop_back();
		ret.pop_back();
	}
	return ret;
}

/**
 * @brief 发现循环及其子循环
 * @param bb 循环的header块
 * @param latches 循环的回边终点(latch)集合
 * @param loop 当前正在处理的循环对象
 */
void LoopDetection::discover_loop_and_sub_loops(BasicBlock* bb, BBset& latches,
                                                Loop* loop, Dominators* dom)
{
	// DONE List:
	// 1. 初始化工作表，将所有latch块加入
	// 2. 实现主循环逻辑
	// 3. 处理未分配给任何循环的节点
	// 4. 处理已属于其他循环的节点
	// 5. 建立正确的循环嵌套关系

	BBvec work_list = {latches.begin(), latches.end()}; // 初始化工作表

	while (!work_list.empty())
	{
		// 当工作表非空时继续处理
		auto subbb = work_list.back();
		work_list.pop_back();
		// DONE-1: 处理未分配给任何循环的节点
		if (bb_to_loop_.find(subbb) == bb_to_loop_.end())
		{
			loop->add_block(subbb);
			bb_to_loop_[subbb] = loop;
			for (auto pre : subbb->get_pre_basic_blocks())
			{
				if (dom->is_dominate(bb, pre))
				{
					work_list.push_back(pre);
				}
			}
		}
		// DONE-2: 处理已属于其他循环的节点
		else if (bb_to_loop_[subbb] != loop)
		{
			auto subloop = bb_to_loop_[subbb];
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
				for (auto pre : subloop->get_header()->get_pre_basic_blocks())
				{
					if (dom->is_dominate(bb, pre))
					{
						work_list.push_back(pre);
					}
				}
			}
		}
	}
}

/**
 * @brief 打印循环检测的结果
 *
 * 为每个检测到的循环打印：
 * 1. 循环的header块
 * 2. 循环包含的所有基本块
 * 3. 循环的所有子循环
 */
void LoopDetection::print() const
{
	f_->get_parent()->set_print_name();
	std::cout << "Loop Detection Result:" << '\n';
	for (auto& loop : loops_)
	{
		std::cout << "\nLoop header: " << loop->get_header()->get_name()
			<< '\n';
		std::cout << "Loop blocks: ";
		for (auto& bb : loop->get_blocks())
		{
			std::cout << bb->get_name() << " ";
		}
		std::cout << '\n';
		std::cout << "Sub loops: ";
		for (auto& sub_loop : loop->get_sub_loops())
		{
			std::cout << sub_loop->get_header()->get_name() << " ";
		}
		std::cout << '\n';
	}
}

int LoopDetection::costOfLatch(Loop* loop, BasicBlock* bb, const Dominators* idoms)
{
	bool out = false;
	for (auto i : bb->get_succ_basic_blocks())
	{
		if (loop->get_blocks().count(i))
		{
			out = true;
			break;
		}
	}
	auto hd = loop->get_header();
	std::unordered_set<BasicBlock*> blocks;
	blocks.emplace(bb);
	while (bb != hd)
	{
		auto subLoop = bb_to_loop_[bb];
		if (subLoop != nullptr && subLoop->get_header() == bb && subLoop->get_parent() == loop)
			blocks.insert(subLoop->get_blocks().begin(), subLoop->get_blocks().end());
		bb = idoms->get_idom(bb);
		blocks.emplace(bb);
		if (!out)
		{
			for (auto i : bb->get_succ_basic_blocks())
			{
				if (loop->get_blocks().count(i))
				{
					out = true;
					break;
				}
			}
		}
	}
	int cost = u2iNegThrow(blocks.size());
	if (out) cost += INT_MAX >> 1;
	return cost;
}

void LoopDetection::collectInnerLoopMessage(Loop* loop, BasicBlock* bb, BasicBlock* preHeader, Loop* innerLoop, Dominators* idoms)
{
	innerLoop->set_preheader(preHeader);
	auto hd = loop->get_header();
	innerLoop->add_block(bb);
	innerLoop->add_latch(bb);
	auto pbb = bb;
	while (pbb != hd)
	{
		auto subLoop = bb_to_loop_[pbb];
		if (subLoop != nullptr && subLoop->get_header() == pbb && subLoop->get_parent() == loop)
		{
			innerLoop->get_blocks().insert(subLoop->get_blocks().begin(), subLoop->get_blocks().end());
			innerLoop->add_sub_loop(subLoop);
			subLoop->set_parent(innerLoop);
		}
		pbb = idoms->get_idom(pbb);
		innerLoop->add_block(pbb);
	}
	std::vector<Loop*> newSubs;
	for (auto sub : loop->get_sub_loops())
	{
		if (sub->get_parent() == loop)
		{
			newSubs.emplace_back(sub);
		}
	}
	loop->get_sub_loops() = newSubs;
	loop->add_sub_loop(innerLoop);
	loop->add_block(preHeader);
	innerLoop->set_parent(loop);
	loop->set_header(preHeader);
	bb_to_loop_[preHeader] = loop;
	bb_to_loop_[hd] = innerLoop;
	loops_.emplace_back(innerLoop);
	auto hid = idoms->get_idom(hd);
	idoms->set_idom(hd, preHeader);
	idoms->set_idom(preHeader, hid);
	loop->remove_latch(bb);
}

void LoopDetection::addNewExitTo(Loop* loop, BasicBlock* bb, BasicBlock* out, BasicBlock* preOut)
{
	loop->addExit(bb, out);
	auto lp = loop->get_parent();
	while (lp != nullptr)
	{
		if (lp->get_blocks().count(preOut))
			lp->add_block(out);
		lp = lp->get_parent();
	}
}

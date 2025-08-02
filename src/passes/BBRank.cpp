#include "BBRank.hpp"

#include "LoopDetection.hpp"

// 基本思路: 一个循环的基本块排在一起
// 去除回边, 进行拓扑排序
void BBRank::run()
{
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_) continue;
		if (f->get_basic_blocks().size() == 1) continue;
		f_ = f;
		runOnFunc();
	}
}

void BBRank::removeEdge(BBNode* from, BBNode* to)
{
	from->nc_ -= from->next_.resetAndGet(to->id_);
	to->pc_ -= to->pre_.resetAndGet(from->id_);
}

void BBRank::addEdge(BBNode* from, BBNode* to)
{
	from->nc_ += from->next_.setAndGet(to->id_);
	to->pc_ += to->pre_.setAndGet(from->id_);
}

void BBRank::runOnFunc()
{
	collectNodes();
	LoopDetection d{m_};
	d.only_run_on_func(f_);
	auto& loops = d.get_loops();
	std::unordered_set<Loop*> visitedLoops;
	loopId_ = 0;
	runOnLoops(visitedLoops, loops);
	std::vector<BBNode*> nodes;
	nodes.resize(nodes_.size());
	int idx = 0;
	auto begin = allNodes_[0];
	nodes[idx++] = begin;
	begin->loopId_ = loopId_;
	for (auto [i, j] : nodes_)
		if (j != begin)
		{
			j->loopId_ = loopId_;
			nodes[idx++] = j;
		}
	topoMerge(nodes);
	auto& bbs = f_->get_basic_blocks();
	bbs = std::move(allNodes_[0]->blocks_);
}

void BBRank::runOnLoop(Loop* loop)
{
	std::vector<BBNode*> nodes;
	auto begin = nodes_[loop->get_header()];
	begin->loopId_ = loopId_;
	nodes.emplace_back(begin);
	for (auto i : loop->get_blocks())
	{
		auto fd = nodes_.find(i);
		if (fd == nodes_.end()) continue;
		auto node = fd->second;
		removeEdge(node, begin);
		if (node != begin)
		{
			nodes_.erase(i);
			node->loopId_ = loopId_;
			nodes.emplace_back(node);
		}
	}
	topoMerge(nodes);
	loopId_++;
}

void BBRank::runOnLoops(std::unordered_set<Loop*>& visitedLoops, const std::vector<Loop*>& loops)
{
	for (auto i : loops)
	{
		if (visitedLoops.count(i)) continue;
		visitedLoops.emplace(i);
		auto& subs = i->get_sub_loops();
		if (!subs.empty())runOnLoops(visitedLoops, subs);
		runOnLoop(i);
	}
}

void BBRank::collectNodes()
{
	nodes_.clear();
	auto& bbs = f_->get_basic_blocks();
	int abc = u2iNegThrow(bbs.size());
	for (auto i : allNodes_) delete i;
	allNodes_.resize(abc);
	int bc = 0;
	for (auto i : bbs)
	{
		auto n = new BBNode{{i}, {abc}, {abc}, 0, 0, bc, -1};
		nodes_.emplace(i, n);
		allNodes_[bc++] = n;
	}
	for (auto i : bbs)
	{
		auto in = nodes_[i];
		for (auto s : i->get_succ_basic_blocks())
		{
			auto sn = nodes_[s];
			addEdge(in, sn);
		}
	}
}

void BBRank::topoMerge(const std::vector<BBNode*>& nodes)
{
	auto begin = nodes[0];
	int size = u2iNegThrow(nodes.size());
	for (int i = 1; i < size; i++)
	{
		auto node = nodes[i];
		for (auto j : node->next_)
		{
			auto next = allNodes_[j];
			if (next->loopId_ != loopId_)
			{
				removeEdge(node, next);
				addEdge(begin, next);
			}
		}
		for (auto j : node->pre_)
		{
			auto pre = allNodes_[j];
			if (pre->loopId_ != loopId_)
			{
				removeEdge(pre, node);
				addEdge(pre, begin);
			}
		}
	}
	int count = u2iNegThrow(nodes.size());
	dfsMerge(begin, nodes[0], count);
	if (count > 0)
	{
		auto worklist = DynamicBitset{count};
		worklist.set(1, count - 1);
		int pc = count;
		while (count > 0)
		{
			for (auto j : worklist)
			{
				auto i = allNodes_[j];
				if (i->pc_ == 0)
				{
					worklist.reset(j);
					if (i->loopId_ == loopId_)
						dfsMerge(begin, i, pc);
				}
			}
			if (pc == count)
			{
				auto i = allNodes_[*worklist.begin()];
				removeEdge(i, allNodes_[*i->pre_.begin()]);
			}
			count = pc;
		}
	}
}

void BBRank::dfsMerge(BBNode* parent, BBNode* node, int& count)
{
	node->loopId_ = -1;
	count--;
	auto n = node->next_;
	for (auto next : n)
	{
		auto nextNode = allNodes_[next];
		if (nextNode->loopId_ == loopId_) removeEdge(node, nextNode);
	}
	for (auto next : n)
	{
		auto nextNode = allNodes_[next];
		if (nextNode->pc_ == 0 && nextNode->loopId_ == loopId_)
		{
			parent->blocks_.splice(parent->blocks_.end(), nextNode->blocks_);
			dfsMerge(parent, nextNode, count);
		}
	}
}

BBRank::~BBRank()
{
	for (auto i : allNodes_) delete i;
}

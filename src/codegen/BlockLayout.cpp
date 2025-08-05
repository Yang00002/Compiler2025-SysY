#include "BlockLayout.hpp"

#include <stdexcept>

#include "MachineInstruction.hpp"
#include "MachineLoopDetection.hpp"
#include "MachineOperand.hpp"

#include <iostream>

#define OPEN_ASSERT 0
#define DEBUG 0
#include "Util.hpp"

using namespace std;


std::string BlockLayout::BBNode::print() const
{
	int count = 0;
	string ret;
	auto bb = block_;
	while (bb != nullptr)
	{
		ret += to_string(bb->id_) + " ";
		bb = bb->next_;
		count++;
	}
	if (!ret.empty())
		ret.pop_back();
	return to_string(count) + "{" + ret + "}";
}

namespace
{
	bool checkFunc(MFunction* f)
	{
		for (auto i : f->blocks())
		{
			ASSERT(i->id() == idx++);
		}
		return true;
	}
}

void BlockLayout::runOnFunc()
{
	LOG(color::blue("Run on Func ") + f_->name());
	PUSH;
	detect.run_on_func(f_);
	blockCount_ = u2iNegThrow(f_->blocks().size());
	ASSERT(checkFunc(f_));
	collectBlocks();
	auto& loops = detect.get_loops();
	if (!loops.empty())
	{
		vector<MachineLoop*> tops;
		for (auto i : loops) if (i->get_parent() == nullptr) tops.emplace_back(i);
		runOnLoops(tops);
	}
	auto nodes = DynamicBitset{blockCount_};
	nodes.rangeSet(0, blockCount_);
	runOnNodes(nodes);
	nodes.reset();
	auto n2 = blocks_[0]->block_;
	vector<MBasicBlock*> v2;
	int idx = 0;
	while (n2 != nullptr)
	{
		v2.emplace_back(n2);
		nodes.set(n2->id_);
		n2->id_ = idx++;
		n2 = n2->next_;
	}
	for (int i = 0; i < blockCount_; i++)
	{
		if (!nodes.test(i))
		{
			delete f_->blocks()[i];
		}
	}
	f_->blocks() = v2;
	POP;
}

void BlockLayout::runOnLoops(const std::vector<MachineLoop*>& loops)
{
	for (auto i : loops)
	{
		if (!i->get_sub_loops().empty()) runOnLoops(i->get_sub_loops());
		runOnLoop(i);
	}
}

void BlockLayout::runOnLoop(MachineLoop* loop)
{
	LOG(color::blue("runOnLoop ") + to_string(loop->get_header()->id()) + " contains " + loop->get_blocks().print());
	PUSH;
	auto& latches = loop->get_latches();
	auto pre = block(loop->get_header());
	for (auto l : latches)
	{
		auto next = block(l);
		removeEdge(next, pre);
	}
	runOnNodes(loop->get_blocks());
	POP;
}

std::string BlockLayout::logNodes(const DynamicBitset& nodes) const
{
	string ret;
	for (auto i : nodes)
	{
		ret += blocks_[i]->print() + "  ";
	}
	if (!ret.empty())
	{
		ret.pop_back();
		ret.pop_back();
	}
	return ret;
}

void BlockLayout::runOnNodes(DynamicBitset& nodes)
{
	for (auto i : nodes)
	{
		if (blocks_[i]->parent_ != nullptr)
		{
			// ASSERT(nodes.test(block(i)->id_));
			nodes.reset(i);
		}
	}

	LOG(color::blue("runOnNodes ") + logNodes(nodes));
	PUSH;

	while (true)
	{
		workListGate_ = 0;
		for (auto i : nodes)
		{
			ASSERT(block(i) == blocks_[i]);
			auto n = blocks_[i];
			auto l = i * blockCount_;
			for (auto next : n->next_)
			{
				if (nodes.test(next))
				{
					auto hash = l + next;
					auto cost = costMap_[hash];
					workList_.emplace(i, next, cost, block(next)->len_);
					if (workListGate_ < cost) workListGate_ = cost;
				}
			}
		}
		if (workList_.empty()) break;
		while (!workList_.empty())
		{
			auto edge = workList_.top();
			workList_.pop();
			auto from = block(edge.from_);
			auto to = block(edge.to_);
			if (from == to) continue;
			auto hash = from->id_ * blockCount_ + to->id_;
			auto cost = costMap_[hash];
			if (cost == edge.cost_ && to->len_ == edge.length_)
				merge(from, to, nodes);
		}
	}

	POP;
}

void BlockLayout::collectBlocks()
{
	delete costMap_;
	costMap_ = new unsigned[i2uKeepBits(blockCount_ * blockCount_)]{};
	for (auto i : blocks_) delete i;
	blocks_.resize(blockCount_);
	unsigned id = 0;
	for (auto i : f_->blocks())
	{
		unsigned size = 0;
		for (auto j : i->instructions()) size += j->str->lines();
		size += i->needBranchCount();
		auto node = new BBNode{i, {blockCount_}, {blockCount_}, nullptr, nullptr, size, id++};
		blocks_[i->id()] = node;
	}
	for (auto i : f_->blocks())
	{
		auto& suc = i->suc_bbs();
		if (suc.size() == 1)
		{
			auto next = *suc.begin();
			auto bl = block(i);
			auto br = block(next);
			addEdge(bl, br, 0);
		}
		else if (suc.size() == 2)
		{
			auto bl = block(i);
			for (auto next : suc)
			{
				auto br = block(next);
				addEdge(bl, br, 1);
			}
		}
	}
}

BlockLayout::BBNode* BlockLayout::block(const MBasicBlock* bb) const
{
	return blocks_[bb->id()]->parent();
}

BlockLayout::BBNode* BlockLayout::block(unsigned bb) const
{
	return blocks_[bb]->parent();
}

void BlockLayout::removeEdge(BBNode* from, BBNode* to)
{
	from->next_.reset(static_cast<int>(to->id_));
	to->pre_.reset(static_cast<int>(from->id_));
}

void BlockLayout::addEdge(BBNode* from, BBNode* to, unsigned cost) const
{
	from->next_.set(static_cast<int>(to->id_));
	to->pre_.set(static_cast<int>(from->id_));
	unsigned hash = from->id_ * blockCount_ + to->id_;
	costMap_[hash] = cost;
}

void BlockLayout::addEdge(BBNode* from, BBNode* to, unsigned hash, unsigned cost) const
{
	from->next_.set(static_cast<int>(to->id_));
	to->pre_.set(static_cast<int>(from->id_));
	costMap_[hash] = cost;
}

unsigned BlockLayout::cost(const BBNode* from, const BBNode* to) const
{
	if (from->next_.test(static_cast<int>(to->id_))) return costMap_[from->id_ * blockCount_ + to->id_];
	return INT_MAX;
}

namespace
{
	struct EdgeModifier
	{
		unsigned c_;
		unsigned l_;
	};
}

void BlockLayout::merge(BBNode* from, BBNode* to, const DynamicBitset& care)
{
	LOG(color::yellow("Merge Nodes ") + from->print() + color::yellow(" and ") + to->print());
	map<pair<unsigned, unsigned>, EdgeModifier> added;
	auto add2map = [&added](unsigned l, unsigned r, unsigned c, unsigned le)-> void
	{
		auto hash = pair{l, r};
		auto fd = added.find(hash);
		if (fd == added.end())
			added.emplace(hash, EdgeModifier{c, le});
		else
		{
			auto& s = fd->second;
			if (s.c_ > c) s.c_ = c;
			s.l_ = le;
		}
	};
	removeEdge(from, to);
	removeEdge(to, from);
	auto l = from->child()->block_;
	auto r = to->block_;
	if (r->empty() && r->next_ != nullptr) r = r->next_;
	l->next_ = r;

	queue<MBasicBlock*> needUpdates;
	needUpdates.emplace(l);
	int pn = l->needBranchCount();
	while (!needUpdates.empty())
	{
		auto up = needUpdates.front();
		needUpdates.pop();
		int p0 = up->needBranchCount();
		auto parent = block(up);
		parent->size_ -= up->collapseBranch();
		int p1 = up->needBranchCount();
		parent->size_ += p1 - p0;
		if (up->empty() && up->next_)
		{
			for (auto p : up->pre_bbs_) needUpdates.emplace(p);
		}
	}
	int nn = l->needBranchCount() - pn;

	unsigned fromHash = from->id_ * blockCount_;
	unsigned toHash = to->id_ * blockCount_;

	for (auto i : from->next_)
	{
		unsigned hash = fromHash + i;
		add2map(from->id_, i, costMap_[hash] + to->size_ + nn, block(i)->len_);
	}
	for (auto i : to->next_)
	{
		unsigned hash = toHash + i;
		add2map(from->id_, i,
		        costMap_[hash], block(i)->len_);
		removeEdge(to, blocks_[i]);
	}
	for (auto i : from->pre_)
	{
		unsigned hash = i * blockCount_ + from->id_;
		add2map(i, from->id_, costMap_[hash], to->len_);
	}
	bool isEntry = from->block_ == f_->blocks()[0];
	for (auto i : to->pre_)
	{
		if (!isEntry)
		{
			unsigned hash = i * blockCount_ + to->id_;
			add2map(i, from->id_, costMap_[hash] + from->size_, to->len_);
		}
		removeEdge(blocks_[i], to);
	}
	to->parent_ = from;
	from->child_ = to;
	for (auto [i,j] : added)
	{
		unsigned hash = i.first * blockCount_ + i.second;
		if (care.test(static_cast<int>(i.first)) && care.test(static_cast<int>(i.second)) && j.c_ <= workListGate_ && (
			    j.l_ != block(i.second)->len_ || costMap_[hash] != j.c_))
			workList_.emplace(i.first, i.second, j.c_, j.l_);
		addEdge(blocks_[i.first], blocks_[i.second], hash, j.c_);
	}
	from->size_ += to->size_;
	from->len_ = to->len_;
}

unsigned BlockLayout::lenOf(BBNode* node)
{
	auto n = node->child();
	auto inst = n->block_->instructions_.back();
	if (dynamic_cast<ReturnInst*>(inst)) return 2;
	auto b = n->block_->needBranchCount();
	if (b == 1) return 0;
	return 1;
}

BlockLayout::~BlockLayout()
{
	delete costMap_;
	for (auto i : blocks_) delete i;
}

void BlockLayout::run()
{
	LOG(color::cyan("Run BlockLayout Pass"));
	PUSH;
	for (auto i : m_->functions())
	{
		f_ = i;
		runOnFunc();
	}
	POP;
	LOG(color::cyan("BlockLayout Done"));
}

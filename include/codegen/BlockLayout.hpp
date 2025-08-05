#pragma once

#include <queue>

#include "InterfereGraph.hpp"
#include "MachineLoopDetection.hpp"
#include "MachinePassManager.hpp"
class Loop;


/**
 * 基本块排序
 * 将环中的基本块排在一起(并非自然循环)
 **/
class BlockLayout final : public MachinePass
{
	struct Edge;

	struct BBNode
	{
		MBasicBlock* block_;
		DynamicBitset next_;
		DynamicBitset pre_;
		BBNode* parent_;
		BBNode* child_;
		unsigned size_;
		unsigned id_;
		unsigned len_ = 0;

		BBNode* parent()
		{
			if (parent_ == nullptr) return this;
			auto p = parent_->parent();
			parent_ = p;
			return p;
		}

		BBNode* child()
		{
			if (child_ == nullptr) return this;
			auto p = child_->child();
			child_ = p;
			return p;
		}

		[[nodiscard]] std::string print() const;
	};

	struct Edge
	{
		unsigned from_;
		unsigned to_;
		unsigned cost_;
		unsigned length_;

		Edge(unsigned f, unsigned t, unsigned c, unsigned l): from_(f), to_(t), cost_(c), length_(l)
		{
		}
	};

	struct EdgeCMP
	{
		bool operator()(const Edge& l, const Edge& r) const
		{
			unsigned long long c1 = l.length_;
			unsigned long long c2 = r.length_;
			c1 |= static_cast<unsigned long long>(l.cost_) << 32;
			c2 |= static_cast<unsigned long long>(r.cost_) << 32;
			if (c1 > c2) return true;
			if (c1 < c2) return false;
			c1 = l.to_;
			c2 = r.to_;
			c1 |= static_cast<unsigned long long>(l.from_) << 32;
			c2 |= static_cast<unsigned long long>(r.from_) << 32;
			return c1 < c2;
		}
	};

	MFunction* f_ = nullptr;
	MachineLoopDetection detect;
	std::priority_queue<Edge, std::vector<Edge>, EdgeCMP> workList_;
	std::vector<BBNode*> blocks_;
	unsigned* costMap_ = nullptr;
	unsigned workListGate_ = 0;
	int blockCount_ = 0;
	void runOnFunc();
	void runOnLoops(const std::vector<MachineLoop*>& loops);
	void runOnLoop(MachineLoop* loop);
	[[nodiscard]] std::string logNodes(const DynamicBitset& nodes) const;
	void runOnNodes(DynamicBitset& nodes);
	void collectBlocks();
	BBNode* block(const MBasicBlock* bb) const;
	[[nodiscard]] BBNode* block(unsigned bb) const;
	static void removeEdge(BBNode* from, BBNode* to);
	void addEdge(BBNode* from, BBNode* to, unsigned cost) const;
	void addEdge(BBNode* from, BBNode* to, unsigned hash, unsigned cost) const;
	unsigned cost(const BBNode* from, const BBNode* to) const;
	void merge(BBNode* from, BBNode* to, const DynamicBitset& care);
	static unsigned lenOf(BBNode* node);

public:
	BlockLayout(const BlockLayout& other) = delete;
	BlockLayout(BlockLayout&& other) noexcept = delete;
	BlockLayout& operator=(const BlockLayout& other) = delete;
	BlockLayout& operator=(BlockLayout&& other) noexcept = delete;

	~BlockLayout() override;

	explicit BlockLayout(MModule* m)
		: MachinePass(m), detect(m)
	{
	}

	void run() override;
};

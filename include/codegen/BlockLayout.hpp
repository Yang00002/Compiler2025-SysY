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
		int nc_;
		int size_;
		int id_;

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
		int from_;
		int to_;
		int cost_;
		int length_;

		Edge(int f, int t, int c, int l): from_(f), to_(t), cost_(c), length_(l)
		{
		}
	};

	struct EdgeCMP
	{
		bool operator()(const Edge& l, const Edge& r) const
		{
			if (l.cost_ > r.cost_) return true;
			if (l.cost_ == r.cost_) return l.length_ < r.length_;
			return false;
		}
	};

	MFunction* f_ = nullptr;
	MachineLoopDetection detect;
	std::priority_queue<Edge, std::vector<Edge>, EdgeCMP> workList_;
	std::vector<BBNode*> blocks_;
	int* costMap_ = nullptr;
	int* lengthMap_ = nullptr;
	int blockCount_ = 0;
	void runOnFunc();
	void runOnLoops(const std::vector<MachineLoop*>& loops);
	void runOnLoop(MachineLoop* loop);
	[[nodiscard]] std::string logNodes(const DynamicBitset& nodes) const;
	void runOnNodes(DynamicBitset& nodes);
	void collectBlocks();
	BBNode* block(const MBasicBlock* bb) const;
	[[nodiscard]] BBNode* block(int bb) const;
	static void removeEdge(BBNode* from, BBNode* to);
	void addEdge(BBNode* from, BBNode* to, int cost, int length) const;
	int cost(const BBNode* from, const BBNode* to) const;
	void merge(BBNode* from, BBNode* to, const DynamicBitset& care);
	[[nodiscard]] int edgeHash(int f, int t) const;

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

#pragma once

#include <unordered_set>

#include "InterfereGraph.hpp"
#include "PassManager.hpp"

class Loop;

struct BBNode
{
	std::list<BasicBlock*> blocks_;
	DynamicBitset next_;
	DynamicBitset pre_;
	int nc_ = 0;
	int pc_ = 0;
	int id_ = 0;
	int loopId_ = -1;
};

/**
 * 基本块排序
 **/
class BBRank final : public Pass
{
public:
	BBRank(const BBRank& other) = delete;
	BBRank(BBRank&& other) noexcept = delete;
	BBRank& operator=(const BBRank& other) = delete;
	BBRank& operator=(BBRank&& other) noexcept = delete;

private:
	std::unordered_map<BasicBlock*, BBNode*> nodes_;
	std::vector<BBNode*> allNodes_;
	Function* f_ = nullptr;
	int loopId_ = 0;

	static void removeEdge(BBNode* from, BBNode* to);
	static void addEdge(BBNode* from, BBNode* to);
	void runOnFunc();
	void runOnLoop(Loop* loop);
	void runOnLoops(std::unordered_set<Loop*>& visitedLoops, const std::vector<Loop*>& loops);
	void collectNodes();
	void topoMerge(const std::vector<BBNode*>& nodes);
	void dfsMerge(BBNode* parent, BBNode* node, int& count);

public:
	~BBRank() override;

	explicit BBRank(Module* m)
		: Pass(m)
	{
	}

	void run() override;
};

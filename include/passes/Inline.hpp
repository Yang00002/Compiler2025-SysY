#pragma once

#include <queue>
#include <unordered_set>

#include "DynamicBitset.hpp"
#include "PassManager.hpp"


struct IBBNode
{
	IBBNode* type_;
	IBBNode* l_;
	IBBNode* r_;
	BasicBlock* block_;
	DynamicBitset next_;
	DynamicBitset pre_;
	int nc_ = 0;
	int pc_ = 0;
	int id_ = 0;
	void add(IBBNode* target);
	void remove(IBBNode* target) const;
	void clear();
	void discard();
	bool contain(const IBBNode* target) const;
	[[nodiscard]] bool empty() const;
	[[nodiscard]] IBBNode* top() const;
	[[nodiscard]] IBBNode* pop() const;
	bool haveNext(const IBBNode* n) const;
	bool havePre(const IBBNode* n) const;
};

// 函数和基本块内联
class Inline final : public Pass
{
public:
	Inline(const Inline& other) = delete;
	Inline(Inline&& other) noexcept = delete;
	Inline& operator=(const Inline& other) = delete;
	Inline& operator=(Inline&& other) noexcept = delete;

	explicit Inline(PassManager* manager, Module* m);

	void run() override;
	~Inline() override;

private:
	std::unordered_map<BasicBlock*, IBBNode*> nodes_;
	std::vector<IBBNode*> allNodes_;
	Function* f_ = nullptr;
	IBBNode* waitlist_;
	IBBNode* done_;
	IBBNode* return_;
	std::queue<Function*> funcWorkList_;
	std::unordered_set<Function*> funcVisited_;
	std::unordered_set<Function*> removedFunc_;
	void collectNodes();
	/**
	 * 去除函数中空的基本块(仅含一条指令, 这条指令有唯一目标)
	 */
	void runOnFunc();
	bool mergeBlocks();
	bool mergeReturns();
	bool mergeBranchs();
	void simplifyBranchs() const;
	void replaceGoTo(IBBNode* from, IBBNode* to) const;
	bool selectReturnNodes(std::unordered_map<Value*, IBBNode*>& nodeMap) const;
	void selectBranchNodes();
	void merge(IBBNode* pre, IBBNode* next) const;
	void done2WaitList();
	void mergeFunc();
	[[nodiscard]] IBBNode* node(int i) const;
	IBBNode* firstNext(const IBBNode* n) const;
	IBBNode* firstPre(const IBBNode* n) const;

	static void removeEdge(IBBNode* from, IBBNode* to);
	static void addEdge(IBBNode* from, IBBNode* to);
	static void initializeAddEdge(IBBNode* from, IBBNode* to);
};

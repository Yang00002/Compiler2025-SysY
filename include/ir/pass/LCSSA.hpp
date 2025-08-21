#pragma once
#include <stack>
#include <unordered_set>

#include "PassManager.hpp"

class Instruction;
class PhiInst;
class Dominators;
class Loop;
class LoopDetection;

class LCSSA : public Pass
{
	std::unordered_map<BasicBlock*, Instruction*> bphiMap_;
	std::unordered_set<BasicBlock*> exitBlocks_;
	std::stack<Instruction*> createdPhis_;
	LoopDetection* loops_;
	Function* f_;
	Dominators* dominators_;
	Instruction* handledDef_;
	Loop* loop_;

public:
	void run() override;

	LCSSA(PassManager* manager, Module* m)
		: Pass(manager, m)
	{
		loops_ = nullptr;
		f_ = nullptr;
		dominators_ = nullptr;
		handledDef_ = nullptr;
		loop_ = nullptr;
	}

private:
	void runOnFunc();
	void runOnLoops(const std::vector<Loop*>& loops);
	void runOnLoop();
	bool phiOnlyForLoopAtExit(PhiInst* phi) const;
	void replaceDefFor(Instruction* target);
	void placePhiAt(BasicBlock* bb);
};

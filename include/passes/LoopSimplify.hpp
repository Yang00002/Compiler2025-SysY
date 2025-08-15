#pragma once
#include <unordered_set>

#include "PassManager.hpp"

class Loop;
class LoopDetection;

class LoopSimplify : public Pass
{
	LoopDetection* loops_;
	Function* f_;

public:
	explicit LoopSimplify(Module* m);
	~LoopSimplify() override;

	void run() override;
private:
	void runOnFunc() ;
	void runOnLoops(std::vector<Loop*>& loops);
	void runOnLoop(Loop* loop);
	BasicBlock* preHeader(Loop* loop);
};

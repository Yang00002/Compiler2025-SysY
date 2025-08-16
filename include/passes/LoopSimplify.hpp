#pragma once
#include "PassManager.hpp"

class Loop;
class LoopDetection;

class LoopSimplify : public Pass
{
	LoopDetection* loops_;
	Function* f_;

public:
	explicit LoopSimplify(PassManager* mng,Module* m);

	void run() override;

private:
	void runOnFunc();
	bool createPreHeaderAndLatchOnLoops(const std::vector<Loop*>& loops);
	bool createPreHeaderAndLatchOnLoop(Loop* loop) const;
	bool createExitOnLoops(const std::vector<Loop*>& loops);
	bool createExitOnLoop(Loop* loop) const;
};

#pragma once
#include "PassManager.hpp"

class Loop;
class LoopDetection;

// 简化循环, 当化简后
// 每个循环都有它的专属 PreHeader, Head 只由 PreHeader 到达, 并且 PreHeader 只含 Phi 和 Br
// 循环的 exitBlock (跳出循环时进入的块) 可以由出边独有或共享, 但是 exitBlock 没有不在循环中的前驱, 不保证里面什么都没有
// 每个循环有且只有一个专属 Latch 块, 因此只有一个 Latch 边, Latch 块只有一条出边, 可以有任何指令
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
	bool createLatchOnLoops(const std::vector<Loop*>& loops);
	bool createLatchOnLoop(Loop* loop) const;
};

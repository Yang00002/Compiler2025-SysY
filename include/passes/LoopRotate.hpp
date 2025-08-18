#pragma once
#include "LoopDetection.hpp"
#include "PassManager.hpp"

class Dominators;
class Loop;
class LoopDetection;

// 进行循环旋转, 给一些可以插入 loop guard 的循环插入 guard
// 需要 LoopSimplify， LoopInvariantCodeMotion, 和 LCSSA 作为前置
// 目前还不能以 GVN 作为前置, 需要处理 指针 Phi 和 Cmp Phi 的问题
// GVN 会用循环头中的 getelement 替代循环内部的 getelement
// 当循环旋转后, getelement 的定义被旋转到 latch, 从而出现一个 getelement phi
// Cmp 的问题同理, 程序并没有考虑 i1 出现在 phi 中
class LoopRotate : public Pass
{
	LoopDetection* loops_;
	Function* f_;
	Dominators* dominators_;
	Loop* loop_;
	void runOnFunc();
	bool runOnLoops(const std::vector<Loop*>& loops);
	void splicePreheader() const;
	static void copyInst2(const BasicBlock* enter, BasicBlock* from, BasicBlock* to, std::unordered_map<Value*, Value*>& v_map);
	void moveHeadInst2Latch(const std::unordered_map<Value*, Value*>& v_map) const;
	static Value* getOrDefault(const std::unordered_map<Value*, Value*>& vmap, Value* val);
	void toWhileTrue(const Loop::Iterator& msg) const;
	void erasePhi(const Loop::Iterator& msg) const;
	void rotate(const Loop::Iterator& msg) const;
	bool runOnLoop() const;

public:
	void run() override;

	LoopRotate(PassManager* manager, Module* m)
		: Pass(manager, m)
	{
		loops_ = nullptr;
		f_ = nullptr;
		dominators_ = nullptr;
		loop_ = nullptr;
	}
};

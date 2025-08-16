#pragma once

#include "Instruction.hpp"
#include "Dominators.hpp"
#include "PassManager.hpp"
#include <unordered_set>

class GlobalCodeMotion final : public Pass
{
	Dominators* dom_;
	std::unordered_set<Instruction*> visited_;

	void run(Function* f);
	static bool is_pinned(const Instruction* i);
	BasicBlock* LCA(BasicBlock* bb1, BasicBlock* bb2) const;
	void moveEarly(Instruction* i, Function* f); // 计算最早可插入位置并移动指令
	void postpone(Instruction* i); // 尽可能延迟计算
public:
	GlobalCodeMotion(const GlobalCodeMotion&) = delete;
	GlobalCodeMotion(GlobalCodeMotion&&) = delete;
	GlobalCodeMotion& operator=(const GlobalCodeMotion&) = delete;
	GlobalCodeMotion& operator=(GlobalCodeMotion&&) = delete;

	explicit GlobalCodeMotion(PassManager* mng, Module* m) : Pass(mng, m)
	{
		dom_ = nullptr;
	}

	~GlobalCodeMotion() override = default;

	void run() override;
};

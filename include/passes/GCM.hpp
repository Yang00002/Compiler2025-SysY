#pragma once

#include <queue>

#include "Instruction.hpp"
#include "Dominators.hpp"
#include "PassManager.hpp"
#include <unordered_set>

class FuncInfo;
class LoopDetection;


// 尽可能推迟指令的计算
// store 指令等写入指令, 无法被推迟
// 其它指令可以被推迟到其所有使用在支配树上的 LCA 处
// load 不能被推迟到 store 后面, 所以它的使用还包含其支配的 store
// 需要前置 DeadCode, 因为 GCM 中求 LCA 不允许没有任何使用
class GlobalCodeMotion final : public Pass
{
	Dominators* dom_;
	LoopDetection* loops_;
	FuncInfo* info_;
	Function* f_;
	std::queue<Instruction*> worklist_;
	std::unordered_set<Instruction*> visited_;
	// 哪些指令进行了 store, store 了哪些值
	std::unordered_map<Value*, std::unordered_set<Instruction*>> storeInst_;
	// 哪些指令进行了 load, load 了哪些值
	std::unordered_map<Instruction*, std::unordered_set<Value*>> loadInst_;

	void runInner();
	void collectLoadStores();
	bool is_pinned(const Instruction* i) const;
	BasicBlock* lcaOfUse(Instruction* i) const;
	void postpone(Instruction* i, BasicBlock* bb); // 尽可能延迟计算
public:
	GlobalCodeMotion(const GlobalCodeMotion&) = delete;
	GlobalCodeMotion(GlobalCodeMotion&&) = delete;
	GlobalCodeMotion& operator=(const GlobalCodeMotion&) = delete;
	GlobalCodeMotion& operator=(GlobalCodeMotion&&) = delete;

	explicit GlobalCodeMotion(PassManager* mng, Module* m) : Pass(mng, m)
	{
		dom_ = nullptr;
		loops_ = nullptr;
		f_ = nullptr;
		info_ = nullptr;
	}

	~GlobalCodeMotion() override = default;

	void run() override;
};

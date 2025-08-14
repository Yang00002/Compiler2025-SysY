#pragma once

#include "FuncInfo.hpp"
#include "LoopDetection.hpp"
#include "PassManager.hpp"
#include <memory>
#include <unordered_map>

class LoopInvariantCodeMotion final : public Pass
{
public:
	LoopInvariantCodeMotion(const LoopInvariantCodeMotion&) = delete;
	LoopInvariantCodeMotion(LoopInvariantCodeMotion&&) = delete;
	LoopInvariantCodeMotion& operator=(const LoopInvariantCodeMotion&) = delete;
	LoopInvariantCodeMotion& operator=(LoopInvariantCodeMotion&&) = delete;

	explicit LoopInvariantCodeMotion(Module* m) : Pass(m)
	{
		loop_detection_ = nullptr;
		func_info_ = nullptr;
	}

	~LoopInvariantCodeMotion() override;

	void run() override;

private:
	std::unordered_map<Loop*, bool> is_loop_done_;
	LoopDetection* loop_detection_;
	FuncInfo* func_info_;
	void traverse_loop(Loop* loop);
	void run_on_loop(Loop* loop) const;
	static void collect_loop_info(Loop* loop,
	                              InstructionList& loop_instructions);
};

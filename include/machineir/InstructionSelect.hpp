#pragma once
#include "PassManager.hpp"

// 合并 IR 中的一些指令为新的指令
// 合并后的 IR 不再是合法的 LLVM IR
class InstructionSelect : public Pass
{
	Function* f_;
	BasicBlock* b_;
	void runInner() ;
public:

	explicit InstructionSelect(Module* m)
		: Pass(m)
	{
		f_ = nullptr;
		b_ = nullptr;
	}

	void run() override;
};

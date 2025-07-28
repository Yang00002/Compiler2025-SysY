#pragma once

#include "Instruction.hpp"
#include "BasicBlock.hpp"
#include "Type.hpp"
#include "PassManager.hpp"
#include "Constant.hpp"
#include <cmath>
#include <vector>

class Arithmetic final : public Pass
{
    std::vector<Instruction*> worklist_;
    void run(Function* f);
    void simplify(Instruction* i);
    void optimize_addsub(Instruction* i);
    void optimize_mul(Instruction* i);
    void optimize_fmul(Instruction* i);
    void optimize_div(Instruction* i);
    void optimize_fdiv(Instruction* i);
    void optimize_rem(Instruction* i);
public:
	Arithmetic(const Arithmetic&) = delete;
	Arithmetic(Arithmetic&&) = delete;
	Arithmetic& operator=(const Arithmetic&) = delete;
	Arithmetic& operator=(Arithmetic&&) = delete;

	explicit Arithmetic(Module* m) : Pass(m) {}

	~Arithmetic() override = default;

	void run() override;
};

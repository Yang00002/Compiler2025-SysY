#pragma once

#include "Instruction.hpp"
#include "PassManager.hpp"
#include <vector>

class Arithmetic final : public Pass
{
    std::vector<Instruction*> worklist_;
    void run(Function* f);
    void simplify(Instruction* i) const;
    void optimize_addsub(Instruction* i) const;
    void optimize_mul(Instruction* i) const;
    void optimize_fmul(Instruction* i) const;
    void optimize_div(Instruction* i) const;
    void optimize_fdiv(Instruction* i) const;
    void optimize_rem(Instruction* i) const;
public:
	Arithmetic(const Arithmetic&) = delete;
	Arithmetic(Arithmetic&&) = delete;
	Arithmetic& operator=(const Arithmetic&) = delete;
	Arithmetic& operator=(Arithmetic&&) = delete;

	explicit Arithmetic(Module* m) : Pass(m) {}

	~Arithmetic() override = default;

	void run() override;
};

#pragma once

#include "Dominators.hpp"
#include "Instruction.hpp"
#include "Value.hpp"

#include <map>
#include <memory>

class Mem2Reg final : public Pass
{
	Function* func_;
	Dominators* dominators_;
	std::map<Value*, Value*> phi_map;
	// 变量定值栈
	std::map<Value*, std::vector<Value*>> var_val_stack;
	// phi指令对应的左值(地址)
	std::map<PhiInst*, Value*> phi_lval;

public:
	Mem2Reg(const Mem2Reg&) = delete;
	Mem2Reg(Mem2Reg&&) = delete;
	Mem2Reg& operator=(const Mem2Reg&) = delete;
	Mem2Reg& operator=(Mem2Reg&&) = delete;

	explicit Mem2Reg(PassManager* mng,Module* m);

	~Mem2Reg() override = default;

	void run() override;

	void generate_phi();
	void rename(BasicBlock* bb);

	static bool is_global_variable(Value* l_val);

	static bool is_gep_instr(Value* l_val);

	static bool is_valid_ptr(Value* l_val);
};

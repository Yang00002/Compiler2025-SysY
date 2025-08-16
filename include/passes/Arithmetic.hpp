#pragma once

#include <queue>

#include "Instruction.hpp"
#include "PassManager.hpp"

class Arithmetic final : public Pass
{
	struct CCTuple
	{
		unsigned t;
		unsigned l;
		unsigned r;
	};

	std::queue<Instruction*> worklist_;
	std::unordered_set<Instruction*> inlist_;
	std::queue<Instruction*> sigWorklist_;
	std::unordered_set<Instruction*> inSigList_;
	std::unordered_map<Value*, unsigned> constType_;
	void run(Function* f);
	void simplify(Instruction* i);
	void optimize_addsub(Instruction* i);
	void addAroundToWorkList(const Value* i);
	void addAroundToSigWorkList(const Value* i);
	void addAroundToSigWorkList(const Value* i, const Value* exclude);
	void addToSignalWorkList(Value* i);
	void addToWorkList(Value* i);
	void optimize_mul(IBinaryInst* i);
	void optimize_fmul(FBinaryInst* i);
	void optimize_div(IBinaryInst* i);
	void optimize_fdiv(FBinaryInst* i);
	void optimize_rem(IBinaryInst* i);
	void optimize_shl(IBinaryInst* i);
	void optimize_ashr(IBinaryInst* i);
	void optimize_and(IBinaryInst* i);
	void decideSignal(Instruction* i);
	bool haveConstType(Value* val);
	static CCTuple inferAdd(unsigned t, unsigned l, unsigned r);
	static CCTuple inferShl(unsigned t, unsigned l, unsigned r);
	static CCTuple inferAshr(unsigned t, unsigned l, unsigned r);
	static CCTuple inferAnd(unsigned t, unsigned l, unsigned r);
	static CCTuple infer2Op(unsigned t, unsigned l, unsigned r, Instruction::OpID op);
	static CCTuple inferMul(unsigned t, unsigned l, unsigned r);
	static CCTuple inferRem(unsigned t, unsigned l, unsigned r);
	static CCTuple inferSdiv(unsigned t, unsigned l, unsigned r);
	static CCTuple inferFdiv(unsigned t, unsigned l, unsigned r);
	static unsigned sdivideSignal(unsigned l, unsigned r);
	static unsigned flipSignal(unsigned i);
	static unsigned multiplySignal(unsigned l, unsigned r);
	static unsigned andSignal(unsigned l, unsigned r);
	static unsigned andSignalL(unsigned l, unsigned r);
	static unsigned remSignal(unsigned l, unsigned r);
	static unsigned remSignalL(unsigned l, unsigned r);
	static unsigned addSignal(unsigned l, unsigned r);
	static unsigned mdivideCond(unsigned l, unsigned r);
	void designateSignal(Value* val, unsigned ty);
	unsigned signalOf(Value* val);

public:
	Arithmetic(const Arithmetic&) = delete;
	Arithmetic(Arithmetic&&) = delete;
	Arithmetic& operator=(const Arithmetic&) = delete;
	Arithmetic& operator=(Arithmetic&&) = delete;

	explicit Arithmetic(PassManager* mng, Module* m) : Pass(mng, m)
	{
	}

	~Arithmetic() override = default;

	void run() override;
};

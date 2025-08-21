#pragma once
#include <queue>
#include <unordered_set>

#include "PassManager.hpp"

class Instruction;

class SignalSpread : public GlobalInfoPass
{
	std::queue<Instruction*> sigWorklist_;
	std::unordered_set<Instruction*> inSigList_;

	void addToSignalWorkList(Value* i);
	void signalSpread();
	void designateSignal(Value* val, unsigned ty);
	void spreadRet(Instruction* inst);
	void spreadUseIfSetSig(Value* inst, unsigned sig);
	void spreadAdd(Instruction* inst);
	void spreadSub(Instruction* inst);
	unsigned signalOf2Spread(Value* val);
	void spreadMul(Instruction* inst);
	void spreadSdiv(Instruction* inst);
	void spreadSrem(Instruction* inst);
	void spreadShl(Instruction* inst);
	void spreadAshr(Instruction* inst);
	void spreadAnd(Instruction* inst);
	void spreadFDiv(Instruction* inst);
	void spreadLoad(Instruction* inst);
	void spreadPhi(Instruction* inst);
	void spreadFP2SI(Instruction* inst);
	void spreadSI2FP(Instruction* inst);
	void spreadCall(const Instruction* inst);

public:
	std::unordered_map<Value*, unsigned> constType_;

	SignalSpread(PassManager* mng, Module* m)
		: GlobalInfoPass(mng, m)
	{
	}

	static unsigned addSignal(unsigned l, unsigned r);
	static unsigned flipSignal(unsigned i);
	static unsigned multiplySignal(unsigned l, unsigned r);
	static unsigned sdivideSignal(unsigned l, unsigned r);
	static unsigned andSignal(unsigned l, unsigned r);

	void run() override;
	void decideConds();
};

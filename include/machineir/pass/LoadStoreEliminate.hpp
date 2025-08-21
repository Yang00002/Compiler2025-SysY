#pragma once
#include "MachinePassManager.hpp"

class MOperand;
class MBasicBlock;

class LoadStoreEliminate : public MachinePass
{
	MFunction* f_;
	MBasicBlock* b_;
	std::unordered_map<MOperand*, int> regData_;
	std::unordered_map<MOperand*, std::pair<MOperand*, int>> stackData_;

	void runInner();

public:
	void run() override;
	bool storeDataSame(MOperand* val, MOperand* stack);
	bool storeDataRegUnchange(MOperand* stack);
	bool storeDataSameReg(MOperand* val0, MOperand* stack);
	void updateStack(MOperand* val, MOperand* stack);
	void updateReg(MOperand* val);

	explicit LoadStoreEliminate(MModule* m)
		: MachinePass(m), f_(nullptr), b_(nullptr)
	{
	}
};

#pragma once
#include "MachinePassManager.hpp"

class RemoveEmptyBlocks : MachinePass
{
	MFunction* f_ = nullptr;
	bool removeEmptyBBs() const;
	void runOnFunc() const;
public:
	void run() override;

	explicit RemoveEmptyBlocks(MModule* m)
		: MachinePass(m)
	{
	}
};

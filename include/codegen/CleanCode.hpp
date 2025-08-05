#pragma once
#include "MachineInstruction.hpp"
#include "MachinePassManager.hpp"

class CleanCode : public MachinePass
{
public:
	static void removeInst(MInstruction* inst);
	void run() override;

	explicit CleanCode(MModule* m)
		: MachinePass(m)
	{
	}
};

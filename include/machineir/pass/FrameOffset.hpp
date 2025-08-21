#pragma once
#include "MachinePassManager.hpp"

class FrameOffset : public MachinePass
{
public:
	void run() override;

	explicit FrameOffset(MModule* m)
		: MachinePass(m)
	{
	}
};

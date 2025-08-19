#pragma once
#include "MachinePassManager.hpp"

class RegSpill : public MachinePass
{
	MFunction* f_;
	void runInner() const;

public:
	explicit RegSpill(MModule* m)
		: MachinePass(m), f_(nullptr)
	{
	}

	void run() override;
};

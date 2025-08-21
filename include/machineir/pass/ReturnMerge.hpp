#pragma once
#include "MachinePassManager.hpp"

class ReturnMerge: public MachinePass
{
	MFunction* f_ = nullptr;
	[[nodiscard]] bool shoudlMerge() const;
	void inlineEpilog() const;
	void mergeEpilog() const;
public:
	void run() override;

	explicit ReturnMerge(MModule* m)
		: MachinePass(m)
	{
	}
};

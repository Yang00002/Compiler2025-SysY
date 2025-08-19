#pragma once
#include <unordered_set>

#include "PassManager.hpp"


class BarrierLock : public FuncInfoPass
{
	std::unordered_set<BasicBlock*> up;
	std::unordered_set<BasicBlock*> inner;
	std::unordered_set<BasicBlock*> down;
public:
	BarrierLock(PassManager* mng, Function* f)
		: FuncInfoPass(mng, f)
	{
	}

	void run() override;
	bool isInner(BasicBlock* bb) const;
};

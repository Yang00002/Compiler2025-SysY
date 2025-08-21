#pragma once
#include <queue>
#include <unordered_set>

#include "PassManager.hpp"

class PhiInst;

class PhiEliminate : public Pass
{
	Function* f_;
	std::queue<PhiInst*> pq;
	std::unordered_set<PhiInst*> inq;

public:
	PhiEliminate(PassManager* manager, Module* m)
		: Pass(manager, m)
	{
		f_ = nullptr;
	}

	void run() override;

private:
	void runOnFunc();
};

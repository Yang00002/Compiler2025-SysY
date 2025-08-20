#pragma once
#include "PassManager.hpp"

class ConstGlobalEliminate : public Pass
{
	bool localize_;
public:
	void run() override;

	ConstGlobalEliminate(PassManager* manager, Module* m, bool localize)
		: Pass(manager, m), localize_(localize)
	{
	}
};

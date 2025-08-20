#pragma once
#include "PassManager.hpp"

class ConstGlobalEliminate : public Pass
{
public:
	void run() override;

	ConstGlobalEliminate(PassManager* manager, Module* m)
		: Pass(manager, m)
	{
	}
};

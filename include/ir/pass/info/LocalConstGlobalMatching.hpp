#pragma once
#include "PassManager.hpp"

class LocalConstGlobalMatching : public Pass
{
public:
	void run() override;

	LocalConstGlobalMatching(PassManager* manager, Module* m)
		: Pass(manager, m)
	{
	}
};

#pragma once
#include "PassManager.hpp"

class FuncInfo;

class GetElementSplit final : public Pass
{
	Function* f_;

	void runInner() const;
public:
	GetElementSplit(PassManager* manager, Module* m)
		: Pass(manager, m), f_(nullptr)
	{
	}

	void run() override;
};

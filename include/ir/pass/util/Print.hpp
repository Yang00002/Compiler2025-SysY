#pragma once
#include "PassManager.hpp"

// 插在某个 Pass 前面, 给这个 Pass 前所有基本块和操作数命名
class Print final : public Pass
{
public:
	explicit Print(PassManager* mng, Module* m) : Pass(mng, m)
	{
	}

	void run() override;
};

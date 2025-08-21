#pragma once
#include "MachinePassManager.hpp"

// 将栈地址, 全局变量地址, 常数提前加载到寄存器
class RegPrefill : public MachinePass
{
	MFunction* f_ = nullptr;
	void runInner() const;
public:
	explicit RegPrefill(MModule* m)
		: MachinePass(m)
	{
	}

	void run() override;
};

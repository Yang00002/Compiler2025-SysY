#pragma once
#include "PassManager.hpp"

/**
 * 转换 MachineIR 前置 Pass, 插入额外基本块, 消除关键边
 */
class CriticalEdgeERemove final : public Pass
{
public:
	void run() override;

	explicit CriticalEdgeERemove(Module* m);
};

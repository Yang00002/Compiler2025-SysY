#pragma once
#include "PassManager.hpp"

/**
 * 转换 MachineIR 前置 Pass, 插入额外基本块, 消除关键边
 */
class CriticalEdgeRemove final : public Pass
{
public:
	void run() override;

	explicit CriticalEdgeRemove(PassManager* mng, Module* m);
};

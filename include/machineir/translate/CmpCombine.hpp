#pragma once
#include "PassManager.hpp"

/**
 * 转换 MachineIR 前置 Pass, 插入 Zext, 排序 cmp, 使得 i1 值会被立刻使用, 无需保存
 */
class CmpCombine final : public Pass
{
public:
	void run() override;

	explicit CmpCombine(PassManager* mng ,Module* m);
};

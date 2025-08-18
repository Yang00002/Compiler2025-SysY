#pragma once
#include "InterfereGraph.hpp"
#include "LiveMessage.hpp"
#include "MachinePassManager.hpp"

class MachineDominators;

class RegisterAllocate : public MachinePass
{
public:
	[[nodiscard]] MachineDominators* dominators() const
	{
		return dominator_;
	}

	[[nodiscard]] LiveMessage* live_message() const
	{
		return liveMessage_;
	}

	[[nodiscard]] MFunction* currentFunc() const
	{
		return currentFunc_;
	}

private:
	InterfereGraph interfereGraph_;
	MModule* module_;
	LiveMessage* liveMessage_ = nullptr;
	MFunction* currentFunc_ = nullptr;
	MachineDominators* dominator_ = nullptr;
	void runOn(MFunction* function);
	// 尝试使用浮点寄存器进行 spill 操作, 目前只会使用 callee save 寄存器的 64 位
	void spillWithFR(MFunction* function) const;

public:
	[[nodiscard]] MModule* module() const
	{
		return module_;
	}

	RegisterAllocate(MModule* module);
	void run() override;
};

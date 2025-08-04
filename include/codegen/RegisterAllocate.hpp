#pragma once
#include "InterfereGraph.hpp"
#include "LiveMessage.hpp"
#include "MachinePassManager.hpp"

class RegisterAllocate : public MachinePass
{
public:
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
	void runOn(MFunction* function);

public:
	[[nodiscard]] MModule* module() const
	{
		return module_;
	}

	RegisterAllocate(MModule* module);
	void run() override;
};

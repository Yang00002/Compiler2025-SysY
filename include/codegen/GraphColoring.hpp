#pragma once
#include "InterfereGraph.hpp"
#include "LiveMessage.hpp"

class GraphColorSolver
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

	GraphColorSolver(MModule* module);
	void run();
};

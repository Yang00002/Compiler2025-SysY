#pragma once
#include "Value.hpp"

class MInstOp : public Value
{
public:
	explicit MInstOp(Type* ty, const std::string& name = "");
};
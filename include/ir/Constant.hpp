#pragma once
#include "Value.hpp"
#include "Ast.hpp"

class Module;

// 常量, 可以和 Value 混用
// 通常用于全局变量初始化的并非 Constant, 而是 PlainTensor<ConstantValue>
// 这是因为全局变量并不需要维护 Value(其不会与非常量操作数混用), 并且数组可能很大, 需要高效的存储.
class Constant final : public ConstantValue, public Value
{
public:
	Constant(const Constant&) = delete;
	Constant(Constant&&) = delete;
	Constant& operator=(const Constant&) = delete;
	Constant& operator=(Constant&&) = delete;

	explicit Constant(int i, const std::string& name = "");

	explicit Constant(float i, const std::string& name = "");

	explicit Constant(bool i, const std::string& name = "");

	static Constant* create(Module* m, int i);
	static Constant* create(Module* m, float i);
	static Constant* create(Module* m, bool i);
	~Constant() override = default;
	std::string print() override;
};

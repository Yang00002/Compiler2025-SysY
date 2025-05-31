#pragma once
#include "Value.hpp"

template <typename Element>
class Tensor;

class InitializeValue;
class ConstantValue;
template <typename Element>
class PlainTensor;

class Module;

// 全局变量, 全局变量的初始值一定是常数. 常数不必要维护使用列表, 所以全局变量也未实现和维护使用列表
// 正因如此, 你无法对其使用 get_operand 等操作
// 全局变量一定有一个初始值, 它一开始来自于 AST
class GlobalVariable : public Value
{
public:
	GlobalVariable(const GlobalVariable& other) = delete;
	GlobalVariable(GlobalVariable&& other) = delete;
	GlobalVariable& operator=(const GlobalVariable& other) = delete;
	GlobalVariable& operator=(GlobalVariable&& other) = delete;

private:
	bool is_const_;
	// *
	PlainTensor<ConstantValue>* init_val_ = nullptr;

public:
	// 创建全局常量, 自动将类型变为指针类型(尽管输出类型并非指针)
	static GlobalVariable* create(const std::string& name, Module* m, Type* ty, bool is_const,
	                              const Tensor<InitializeValue>* init);
	GlobalVariable(std::string name, Module* m, Type* ty, bool is_const,
	               const Tensor<InitializeValue>* init);

	~GlobalVariable() override;

	[[nodiscard]] PlainTensor<ConstantValue>* get_init() const { return init_val_; }
	[[nodiscard]] bool is_const() const { return is_const_; }
	std::string print() override;
	// 该全局变量因为初始化而导致的实际类型设置
	// 不同的初始化实际上对应不同的类型聚合
	std::string printInitType() const;
	// 该全局变量的初始化值
	std::string printInitValue() const;
};

#pragma once
#include "Value.hpp"

class Module;
// 提供常量的一个高效抽象, 用于 IR
// 它是常量中存储的值
class ConstantValue
{
	friend bool operator==(const ConstantValue& lhs, const ConstantValue& rhs)
	{
		return lhs._field._value == rhs._field._value;
	}

	friend bool operator!=(const ConstantValue& lhs, const ConstantValue& rhs)
	{
		return lhs._field._value != rhs._field._value;
	}

	friend class InitializeValue;

	union Field
	{
		int _int_value[2] = {};
		float _float_value[2];
		bool _bool_value[8];
		char _segment[8];
		// 用于比较
		long long _value;
	};

	Field _field;

public:
	[[nodiscard]] bool isIntConstant() const;

	[[nodiscard]] bool isBoolConstant() const;

	[[nodiscard]] bool isFloatConstant() const;

	[[nodiscard]] int getIntConstant() const;

	[[nodiscard]] bool getBoolConstant() const;

	[[nodiscard]] float getFloatConstant() const;

	explicit ConstantValue() = default;

	explicit ConstantValue(int intConstant);

	explicit ConstantValue(float floatConstant);

	explicit ConstantValue(bool boolConstant);

	[[nodiscard]] Type* getType() const;

	[[nodiscard]] std::string toString() const;
	// IR
	[[nodiscard]] std::string print() const;
};

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

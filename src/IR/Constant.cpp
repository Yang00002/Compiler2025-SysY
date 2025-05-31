#include "../../include/ir/Constant.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "../../include/ast/System.hpp"
#include "../../include/ast/Tensor.hpp"
#include "../../include/ast/Type.hpp"
#include "../../include/ir/Module.hpp"

using namespace system_about;
using namespace Types;
using namespace std;

bool ConstantValue::isIntConstant() const
{
	return _field._segment[LOGICAL_LEFT_END_8] == static_cast<char>(0x10);
}

bool ConstantValue::isBoolConstant() const
{
	return _field._segment[LOGICAL_LEFT_END_8] == static_cast<char>(0x11);
}

bool ConstantValue::isFloatConstant() const
{
	return _field._segment[LOGICAL_LEFT_END_8] == static_cast<char>(0x01);
}

int ConstantValue::getIntConstant() const
{
	if (!isIntConstant()) throw std::runtime_error("ConstantValue Not Int ConstantValue");
	return _field._int_value[LOGICAL_RIGHT_END_2];
}

bool ConstantValue::getBoolConstant() const
{
	if (!isBoolConstant()) throw std::runtime_error("ConstantValue Not Bool ConstantValue");
	return _field._bool_value[LOGICAL_RIGHT_END_8];
}

float ConstantValue::getFloatConstant() const
{
	if (!isFloatConstant()) throw std::runtime_error("ConstantValue Not Float ConstantValue");
	return _field._float_value[LOGICAL_RIGHT_END_2];
}

ConstantValue::ConstantValue(const int intConstant)
{
	_field._int_value[LOGICAL_RIGHT_END_2] = intConstant;
	_field._segment[LOGICAL_LEFT_END_8] = static_cast<char>(0x10);
}

ConstantValue::ConstantValue(const float floatConstant)
{
	_field._float_value[LOGICAL_RIGHT_END_2] = floatConstant;
	_field._segment[LOGICAL_LEFT_END_8] = static_cast<char>(0x01);
}

ConstantValue::ConstantValue(const bool boolConstant)
{
	_field._bool_value[LOGICAL_RIGHT_END_8] = boolConstant;
	_field._segment[LOGICAL_LEFT_END_8] = static_cast<char>(0x11);
}

Type* ConstantValue::getType() const
{
	if (isIntConstant()) return INT;
	if (isFloatConstant()) return FLOAT;
	return BOOL;
}

std::string ConstantValue::toString() const
{
	if (isIntConstant()) return to_string(_field._int_value[LOGICAL_RIGHT_END_2]);
	if (isFloatConstant())
	{
		std::ostringstream oss;
		oss << std::scientific << std::setprecision(8) << _field._float_value[LOGICAL_RIGHT_END_2];
		return oss.str() + 'f';
	}
	return _field._bool_value[LOGICAL_RIGHT_END_8] ? "true" : "false";
}

std::string ConstantValue::print() const
{
	if (isIntConstant()) return to_string(_field._int_value[LOGICAL_RIGHT_END_2]);
	if (isFloatConstant())
	{
		std::ostringstream oss;
		oss << std::scientific << std::setprecision(8) << _field._float_value[LOGICAL_RIGHT_END_2];
		return oss.str();
	}
	return _field._bool_value[LOGICAL_RIGHT_END_8] ? "true" : "false";
}

Constant::Constant(const int i, const std::string& name)
	: ConstantValue(i), Value(Types::INT, name)
{
}

Constant::Constant(const float i, const std::string& name)
	: ConstantValue(i), Value(Types::FLOAT, name)
{
}

Constant::Constant(const bool i, const std::string& name)
	: ConstantValue(i), Value(Types::BOOL, name)
{
}

Constant* Constant::create(Module* m, const int i)
{
	if (const auto it = m->int_constants_.find(i); it != m->int_constants_.end()) return it->second;
	auto ret = new Constant{i};
	m->int_constants_.emplace(i, ret);
	return ret;
}

Constant* Constant::create(Module* m, float i)
{
	if (const auto it = m->float_constants_.find(i); it != m->float_constants_.end()) return it->second;
	auto ret = new Constant{i};
	m->float_constants_.emplace(i, ret);
	return ret;
}

Constant* Constant::create(Module* m, const bool i)
{
	if (i) return m->true_constant_;
	return m->false_constant_;
}

std::string Constant::print()
{
	return ConstantValue::print();
}

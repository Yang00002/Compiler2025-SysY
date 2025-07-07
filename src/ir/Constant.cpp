#include "Constant.hpp"

#include <cstring>
#include <iomanip>
#include <string>

#include <System.hpp>
#include <Tensor.hpp>
#include <Type.hpp>
#include <Module.hpp>

using namespace system_about;
using namespace Types;
using namespace std;


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
	auto ret = new Constant{ i };
	m->int_constants_.emplace(i, ret);
	return ret;
}

Constant* Constant::create(Module* m, float i)
{
	if (const auto it = m->float_constants_.find(i); it != m->float_constants_.end()) return it->second;
	auto ret = new Constant{ i };
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

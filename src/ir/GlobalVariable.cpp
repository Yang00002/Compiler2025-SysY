#include <utility>

#include "../../include/ir/GlobalVariable.hpp"
#include "../../include/ir/IRPrinter.hpp"
#include "../../include/ir/Module.hpp"
#include "../../include/ir/Constant.hpp"
#include "../../include/util/Tensor.hpp"
#include "../../include/ast/Ast.hpp"
#include "../../include/util/Type.hpp"

using namespace std;
using namespace Types;

GlobalVariable* GlobalVariable::create(const std::string& name, Module* m, Type* ty, bool is_const,
                                       const Tensor<InitializeValue>* init)
{
	return new GlobalVariable{name, m, ty, is_const, init};
}

GlobalVariable::GlobalVariable(std::string name, Module* m, Type* ty, bool is_const,
                               const Tensor<InitializeValue>* init): Value(Types::pointerType(ty), std::move(name)),
                                                                     is_const_(is_const)
{
	m->add_global_variable(this);
	init_val_ = init->toPlain<ConstantValue>([](const InitializeValue& v)-> ConstantValue
	{
		return v.toConstant();
	}, 1);
}

GlobalVariable::~GlobalVariable()
{
	delete init_val_;
}

std::string GlobalVariable::print()
{
	std::string global_val_ir;
	global_val_ir += print_as_op(this, false);
	global_val_ir += " = ";
	global_val_ir += (this->is_const() ? "constant " : "global ");
	global_val_ir += printInitType();
	global_val_ir += " ";
	global_val_ir += printInitValue();
	return global_val_ir;
}

std::string GlobalVariable::printInitType() const
{
	auto ty = dynamic_cast<PointerType*>(this->get_type())->typeContained();
	if (ty->isBasicType())
	{
		return ty->print();
	}
	auto tys = ty->toArrayType()->typeContained()->print();
	int size = init_val_->segmentCount();
	if (size == 1)
	{
		auto [p, l] = init_val_->segment(0);
		if (p != nullptr)
		{
			int len = static_cast<int>(p->size());
			return '[' + std::to_string(len) + " x " + tys + ']';
		}
		return '[' + std::to_string(l) + " x " + tys + ']';
	}
	string ret = "<{";
	for (int i = 0; i < size; i++)
	{
		auto [p, l] = init_val_->segment(i);
		if (p != nullptr)
		{
			if (p->size() == 1)
			{
				ret += tys;
			}
			else ret += '[' + std::to_string(p->size()) + " x " + tys + ']';
		}
		else
		{
			ret += '[' + std::to_string(l) + " x " + tys + ']';
		}
		ret += ", ";
	}
	ret.pop_back();
	ret.pop_back();
	ret += "}>";
	return ret;
}

std::string GlobalVariable::printInitValue() const
{
	auto ty = dynamic_cast<PointerType*>(this->get_type())->typeContained();
	if (ty->isBasicType())
	{
		auto [p, l] = init_val_->segment(0);
		return (*p)[0].print();
	}
	auto tys = ty->toArrayType()->typeContained()->print();
	int size = init_val_->segmentCount();
	if (size == 1)
	{
		auto [p, l] = init_val_->segment(0);
		if (p != nullptr)
		{
			int len = static_cast<int>(p->size());
			string ret = "[";
			for (int i = 0; i < len; i++)
			{
				ret += tys;
				ret += " ";
				ret += (*p)[i].print();
				ret += ", ";
			}
			ret.pop_back();
			ret.pop_back();
			ret += ']';
			return ret;
		}
		return "zeroinitializer";
	}
	string ret = "<{";
	for (int i = 0; i < size; i++)
	{
		auto [p, l] = init_val_->segment(i);
		if (p != nullptr)
		{
			if (p->size() == 1)
			{
				ret += tys;
				ret += ' ';
				ret += (*p)[0].print();
			}
			else
			{
				int len = static_cast<int>(p->size());
				ret += '[' + std::to_string(len) + " x " + tys + "] [";
				for (int ii = 0; ii < len; ii++)
				{
					ret += tys;
					ret += " ";
					ret += (*p)[ii].print();
					ret += ", ";
				}
				ret.pop_back();
				ret.pop_back();
				ret += ']';
			}
		}
		else
		{
			ret += '[' + std::to_string(l) + " x " + tys + "] zeroinitializer";
		}
		ret += ", ";
	}
	ret.pop_back();
	ret.pop_back();
	ret += "}>";
	return ret;
}

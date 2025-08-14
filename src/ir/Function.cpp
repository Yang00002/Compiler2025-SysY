#include "Function.hpp"
#include <BasicBlock.hpp>
#include <IRPrinter.hpp>
#include <Module.hpp>
#include <Type.hpp>
#include <map>
#include <cassert>

#include "MachineBasicBlock.hpp"
#include "System.hpp"
#include "Util.hpp"

Function::Function(FuncType* ty, const std::string& name, Module* parent, const bool is_lib)
	: Value(ty, name), parent_(parent), seq_cnt_(0), is_lib_(is_lib)
{
	// num_args_ = ty->getNumParams();
	parent->add_function(this);
	// build args
	for (int i = 0; i < get_num_of_args(); i++)
	{
		arguments_.emplace_back(new Argument{ty->argumentType(i), "", this, i});
	}
}

Function::~Function()
{
	for (const auto& i : basic_blocks_) delete i;
	for (auto i : arguments_) delete i;
}

Function* Function::create(FuncType* ty, const std::string& name,
                           Module* parent, const bool is_lib)
{
	return new Function(ty, name, parent, is_lib);
}

FuncType* Function::get_function_type() const
{
	return dynamic_cast<FuncType*>(get_type());
}

Type* Function::get_return_type() const
{
	return get_function_type()->returnType();
}

int Function::get_num_of_args() const
{
	return get_function_type()->argumentCount();
}

int Function::get_num_basic_blocks() const { return u2iNegThrow(basic_blocks_.size()); }

Module* Function::get_parent() const { return parent_; }

void Function::remove(BasicBlock* bb)
{
	basic_blocks_.remove(bb);
	for (auto pre : bb->get_pre_basic_blocks())
	{
		pre->remove_succ_basic_block(bb);
	}
	for (auto succ : bb->get_succ_basic_blocks())
	{
		succ->remove_pre_basic_block(bb);
	}
	auto l = bb->get_use_list();
	for (auto& use_list : l)
	{
		if (auto phi_inst = dynamic_cast<PhiInst*>(use_list.val_); phi_inst != nullptr)
		{
			phi_inst->remove_phi_operand(bb, use_list.arg_no_);
		}
	}
	delete bb;
}

void Function::add_basic_block(BasicBlock* bb) { basic_blocks_.push_back(bb); }

Argument* Function::removeArg(Argument* arg)
{
	int id = arg->get_arg_no();
	removeArgUse(id);
	arguments_.erase(arguments_.begin() + id);
	for (int i = id, size = u2iNegThrow(arguments_.size()); i < size; i++)
	{
		arguments_[i]->set_arg_no(i);
	}
	return arg;
}

Argument* Function::removeArgWithoutUpdate(Argument* arg)
{
	int id = arg->get_arg_no();
	removeArgUse(id);
	arguments_.erase(arguments_.begin() + id);
	return arg;
}

void Function::set_instr_name()
{
	std::map<Value*, int> seq;
	for (auto& arg : this->get_args())
	{
		if (seq.find(arg) == seq.end())
		{
			auto seq_num = u2iNegThrow(seq.size()) + seq_cnt_;
			if (arg->set_name("arg" + std::to_string(seq_num)))
			{
				seq.emplace(arg, seq_num);
			}
		}
	}
	for (auto& bb : basic_blocks_)
	{
		if (seq.find(bb) == seq.end())
		{
			auto seq_num = u2iNegThrow(seq.size()) + seq_cnt_;
			if (bb->set_name("label" + std::to_string(seq_num)))
			{
				seq.emplace(bb, seq_num);
			}
		}
		for (auto instr : bb->get_instructions())
		{
			if (!instr->is_void() && seq.find(instr) == seq.end())
			{
				auto seq_num = u2iNegThrow(seq.size()) + seq_cnt_;
				if (instr->set_name("op" + std::to_string(seq_num)))
				{
					seq.emplace(instr, seq_num);
				}
			}
		}
	}
	seq_cnt_ += u2iNegThrow(seq.size());
}

std::string Function::print()
{
	set_instr_name();
	std::string func_ir;
	if (this->is_declaration())
	{
		func_ir += "declare ";
	}
	else
	{
		func_ir += "define ";
	}

	func_ir += this->get_return_type()->print();
	func_ir += " ";
	func_ir += print_as_op(this, false);
	func_ir += "(";

	// print arg
	if (this->is_declaration())
	{
		for (int i = 0; i < this->get_num_of_args(); i++)
		{
			if (i)
				func_ir += ", ";
			func_ir += dynamic_cast<FuncType*>(this->get_type())
			           ->argumentType(i)
			           ->print();
		}
	}
	else
	{
		for (auto& arg : get_args())
		{
			if (&arg != &*get_args().begin())
				func_ir += ", ";
			func_ir += arg->print();
		}
	}
	func_ir += ")";

	// print bb
	if (this->is_declaration())
	{
		func_ir += "\n";
	}
	else
	{
		func_ir += " {";
		func_ir += "\n";
		for (auto& bb : this->get_basic_blocks())
		{
			func_ir += bb->print();
		}
		func_ir += "}";
	}

	return func_ir;
}

float Function::opWeight(const AllocaInst* value, std::map<BasicBlock*, MBasicBlock*>& bmap)
{
	float r = 0;
	for (auto& use : value->get_use_list())
	{
		auto user = dynamic_cast<Instruction*>(use.val_);
		if (user != nullptr)
		{
			r += bmap[user->get_parent()]->weight();
		}
	}
	return r;
}

void Function::removeArgUse(int id) const
{
	for (auto& use : get_use_list())
	{
		auto call = dynamic_cast<CallInst*>(use.val_);
		call->remove_operand(id + 1);
	}
}

int Argument::get_arg_no() const
{
	ASSERT(parent_ && "can't get number of unparented arg");
	return arg_no_;
}

std::string Argument::print()
{
	std::string arg_ir;
	arg_ir += this->get_type()->print();
	arg_ir += " %";
	arg_ir += this->get_name();
	return arg_ir;
}

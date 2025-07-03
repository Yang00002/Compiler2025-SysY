#include <Module.hpp>
#include <Function.hpp>
#include <GlobalVariable.hpp>
#include <Constant.hpp>
#include <string>

Module::Module()
{
	true_constant_ = new Constant{true};
	false_constant_ = new Constant{false};
}

Module::~Module()
{
	for (const auto& i : global_list_) delete i;
	for (const auto& i : function_list_) delete i;
	delete true_constant_;
	delete false_constant_;
	for (auto& [i, j] : int_constants_)delete j;
	for (auto& [i, j] : float_constants_)delete j;
}

void Module::add_function(Function* f) { function_list_.push_back(f); }

std::list<Function*>& Module::get_functions() { return function_list_; }

void Module::add_global_variable(GlobalVariable* g)
{
	global_list_.push_back(g);
}

std::list<GlobalVariable*>& Module::get_global_variable()
{
	return global_list_;
}

void Module::set_print_name()
{
	for (auto& func : this->get_functions())
	{
		func->set_instr_name();
	}
	return;
}

std::string Module::print()
{
	set_print_name();
	std::string module_ir;
	for (auto& global_val : this->global_list_)
	{
		module_ir += global_val->print();
		module_ir += "\n";
	}
	for (auto& func : this->function_list_)
	{
		module_ir += func->print();
		module_ir += "\n";
	}
	module_ir += R"123(declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1))123";
	return module_ir;
}

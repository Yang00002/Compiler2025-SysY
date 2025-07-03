#pragma once

#include <ir/Function.hpp>
#include <ir/GlobalVariable.hpp>

#include <list>
#include <map>
#include <string>

class Constant;
class GlobalVariable;
class Function;

// IR 顶层
class Module
{
	friend class Constant;
public:
	Module(const Module& other) = delete;
	Module(Module&& other) = delete;
	Module& operator=(const Module& other) = delete;
	Module& operator=(Module&& other) = delete;

	Module();

	~Module();

	void add_function(Function* f);
	std::list<Function*>& get_functions();
	void add_global_variable(GlobalVariable* g);
	std::list<GlobalVariable*>& get_global_variable();

	void set_print_name();
	std::string print();

private:
	// The global variables in the module *
	std::list<GlobalVariable*> global_list_;
	// The functions in the module *
	std::list<Function*> function_list_;

	// 常数存储, 与 AST 不同, 常数不会自动删除, 只能放在 Module 中维护
	std::map<int, Constant*> int_constants_;
	std::map<float, Constant*> float_constants_;
	Constant* true_constant_;
	Constant* false_constant_;
};

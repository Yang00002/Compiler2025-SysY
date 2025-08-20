#pragma once

#include <cassert>
#include <list>
#include <map>
#include <unordered_set>

#include "Value.hpp"

class GlobalVariable;
class AllocaInst;
class MBasicBlock;
class FuncType;
class BasicBlock;
class Module;
class Argument;

// 函数, 函数管理它下面所有基本块的内存, 从函数删除基本块也会释放内存
class Function : public Value
{
public:
	Function(const Function& other) = delete;
	Function(Function&& other) = delete;
	Function& operator=(const Function& other) = delete;
	Function& operator=(Function&& other) = delete;
	Function(FuncType* ty, const std::string& name, Module* parent, bool is_lib = false);

	~Function() override;

	static Function* create(FuncType* ty, const std::string& name,
	                        Module* parent, bool is_lib = false);

	[[nodiscard]] FuncType* get_function_type() const;
	[[nodiscard]] Type* get_return_type() const;

	void add_basic_block(BasicBlock* bb);
	void updateArgType();

	[[nodiscard]] int get_num_of_args() const;
	[[nodiscard]] int get_num_basic_blocks() const;

	[[nodiscard]] Module* get_parent() const;
	/**
	 * 移除函数的基本块并删除它, 这同时也删除了其中所有指令
	 * @param bb 要移除的基本块
	 */
	void remove(BasicBlock* bb);
	[[nodiscard]] BasicBlock* get_entry_block() const { return *basic_blocks_.begin(); }

	std::list<BasicBlock*>& get_basic_blocks() { return basic_blocks_; }
	std::vector<Argument*>& get_args() { return arguments_; }
	[[nodiscard]] Argument* get_arg(int i) const { return arguments_[i]; }
	// 去除参数并返回(但不删除它)
	Argument* removeArg(Argument* arg);
	// 去除参数并返回, 并且不更新后续参数的序号(但不删除它)
	Argument* removeArgWithoutUpdate(Argument* arg);

	[[nodiscard]] bool is_declaration() const { return basic_blocks_.empty(); }

	void set_instr_name();
	std::string print() override;

	Value* ret_alloca_;


	static float opWeight(const AllocaInst* value, std::map<BasicBlock*, MBasicBlock*>& bmap);
	void checkBlockRelations() const;

private:
	// 去除所有 Call 的特定参数
	void removeArgUse(int id) const;
	std::list<BasicBlock*> basic_blocks_;
	std::vector<Argument*> arguments_;
	Module* parent_;
	int seq_cnt_; // print use
public:
	std::unordered_set<GlobalVariable*> constForSelf_;
	// 是否是库函数, 以对函数参数进行不同的处理
	bool is_lib_;
};

// Argument of Function, does not contain actual value
class Argument : public Value
{
public:
	Argument(const Argument& other) = delete;
	Argument(Argument&& other) = delete;
	Argument& operator=(const Argument& other) = delete;
	Argument& operator=(Argument&& other) = delete;

	explicit Argument(Type* ty, const std::string& name = "", Function* f = nullptr, int arg_no = 0)
		: Value(ty, name), parent_(f), arg_no_(arg_no)
	{
	}

	~Argument() override = default;

	[[nodiscard]] const Function* get_parent() const { return parent_; }
	Function* get_parent() { return parent_; }

	// For example in "void foo(int a, float b)" a is 0 and b is 1.
	[[nodiscard]] int get_arg_no() const;

	void set_arg_no(int arg_no)
	{
		arg_no_ = arg_no;
	}

	std::string print() override;

private:
	Function* parent_;
	int arg_no_; // argument No.
};

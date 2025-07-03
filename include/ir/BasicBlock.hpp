#pragma once

#include <Value.hpp>

#include <list>
#include <set>
#include <string>

class Function;
class Instruction;
class Module;

// 基本块
class BasicBlock : public Value
{
public:
	BasicBlock(const BasicBlock&) = delete;
	BasicBlock(BasicBlock&&) = delete;
	BasicBlock& operator=(const BasicBlock&) = delete;
	BasicBlock& operator=(BasicBlock&&) = delete;

	~BasicBlock() override;

	// 创建一个基本块, 为名字附加 label_ 前缀 
	static BasicBlock* create(Module* m, const std::string& name,
	                          Function* parent)
	{
		auto prefix = name.empty() ? "" : "label_";
		return new BasicBlock(m, prefix + name, parent);
	}

	/****************api about cfg****************/

	// 前驱块列表
	std::list<BasicBlock*>& get_pre_basic_blocks() { return pre_bbs_; }
	// 后继块列表
	std::list<BasicBlock*>& get_succ_basic_blocks() { return succ_bbs_; }

	// 添加前驱块
	void add_pre_basic_block(BasicBlock* bb) { pre_bbs_.push_back(bb); }
	// 添加后继块
	void add_succ_basic_block(BasicBlock* bb) { succ_bbs_.push_back(bb); }
	// 移除前驱块
	void remove_pre_basic_block(BasicBlock* bb) { pre_bbs_.remove(bb); }
	// 移除后继块
	void remove_succ_basic_block(BasicBlock* bb) { succ_bbs_.remove(bb); }

	// 该块是否被 ret/br 等指令终止
	[[nodiscard]] bool is_terminated() const;
	// 获取终止该块的指令, 没有则报错
	[[nodiscard]] Instruction* get_terminator() const;

	/****************api about Instruction****************/


	// 在末尾添加指令, 如果已经终止了就报错
	void add_instruction(Instruction* instr);
	// 在开头添加指令
	void add_instr_begin(Instruction* instr) { instr_list_.push_front(instr); }
	// 移除指令, 移除基本块中指令
	void erase_instr(const Instruction* instr);

	// 获取所有指令
	std::list<Instruction*>& get_instructions() { return instr_list_; }
	// 是否为空
	[[nodiscard]] bool empty() const { return instr_list_.empty(); }
	// 指令数
	[[nodiscard]] int get_num_of_instr() const { return static_cast<int>(instr_list_.size()); }

	/****************api about accessing parent****************/

	// 基本块属于的函数
	[[nodiscard]] Function* get_parent() const { return parent_; }
	// 基本块属于的模块
	[[nodiscard]] Module* get_module() const;
	// 从函数中移除基本块
	void erase_from_parent();

	std::string print() override;

	explicit BasicBlock(Module* m, const std::string& name, Function* parent);

private:
	std::list<BasicBlock*> pre_bbs_;
	std::list<BasicBlock*> succ_bbs_;
	// *
	std::list<Instruction*> instr_list_;
	Function* parent_;
};

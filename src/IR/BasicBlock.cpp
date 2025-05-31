#include "../../include/ir/BasicBlock.hpp"
#include "../../include/ir/Function.hpp"
#include "../../include/ir/IRprinter.hpp"
#include "../../include/ast/Type.hpp"

#include <cassert>

BasicBlock::BasicBlock(Module* m, const std::string& name = "",
                       Function* parent = nullptr)
	: Value(Types::LABEL, name), parent_(parent)
{
	assert(parent && "currently parent should not be nullptr");
	parent_->add_basic_block(this);
}

Module* BasicBlock::get_module() const { return get_parent()->get_parent(); }
void BasicBlock::erase_from_parent() { this->get_parent()->remove(this); }

BasicBlock::~BasicBlock()
{
	for (const auto& i : instr_list_)
		delete i;
}

bool BasicBlock::is_terminated() const
{
	if (instr_list_.empty())
	{
		return false;
	}
	switch (instr_list_.back()->get_instr_type())
	{
		case Instruction::ret:
		case Instruction::br:
			return true;
		default:
			return false;
	}
}

Instruction* BasicBlock::get_terminator() const
{
	assert(is_terminated() &&
		"Trying to get terminator from an bb which is not terminated");
	return instr_list_.back();
}

void BasicBlock::add_instruction(Instruction* instr)
{
	assert(not is_terminated() && "Inserting instruction to terminated bb");
	instr_list_.push_back(instr);
}

void BasicBlock::erase_instr(const Instruction* instr)
{
	instr_list_.erase(std::find(instr_list_.begin(), instr_list_.end(), instr));
	delete instr;
}

std::string BasicBlock::print()
{
	std::string bb_ir;
	bb_ir += this->get_name();
	bb_ir += ":";
	// print prebb
	if (!this->get_pre_basic_blocks().empty())
	{
		bb_ir += "                                                ; preds = ";
	}
	for (auto bb : this->get_pre_basic_blocks())
	{
		if (bb != *this->get_pre_basic_blocks().begin())
		{
			bb_ir += ", ";
		}
		bb_ir += print_as_op(bb, false);
	}

	// print prebb
	if (!this->get_parent())
	{
		bb_ir += "\n";
		bb_ir += "; Error: Block without parent!";
	}
	bb_ir += "\n";
	for (auto& instr : this->get_instructions())
	{
		bb_ir += "  ";
		bb_ir += instr->print();
		bb_ir += "\n";
	}

	return bb_ir;
}

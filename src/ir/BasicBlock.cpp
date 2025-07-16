#include "BasicBlock.hpp"
#include "Function.hpp"
#include "IRPrinter.hpp"
#include "Type.hpp"
#include "Util.hpp"



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
	for (const auto i : instr_list_)
		delete i;
}

bool BasicBlock::replace_self_with_block(BasicBlock* bb)
{
	if (bb == this) return false;
	const auto& pb = get_pre_basic_blocks();
	bool multiPre = pb.size() > 1;
	BasicBlock* preBB = pb.empty() ? nullptr : pb.front();
	// 收集使用该基本块定值的 phi 指令
	std::list<std::pair<PhiInst*, unsigned>> phiInsts;
	for (auto use : get_use_list())
	{
		auto phi = dynamic_cast<PhiInst*>(use.val_);
		{
			if (phi != nullptr)
			{
				// 理论上不该对这种基本块运行该指令
				assert(preBB != nullptr);
				// 替代会增加 phi 的长度
				if (multiPre) return false;
				auto& ops = phi->get_operands();
				int s = static_cast<int>(ops.size());
				for (int i = 1; i < s; i += 2)
				{
					// 依赖跳转进行定值
					if (ops[i] == preBB) return false;
				}
				phiInsts.emplace_back(phi, use.arg_no_);
			}
		}
	}
	if (!phiInsts.empty())
	{
		for (auto phi_inst : phiInsts)
		{
			auto phi = phi_inst.first;
			auto id = phi_inst.second;
			auto val = phi->get_operand(id - 1);
			phi->add_phi_pair_operand(val, preBB);
		}
	}
	for (auto pre_basic_block : get_pre_basic_blocks())
	{
		pre_basic_block->remove_succ_basic_block(this);
		pre_basic_block->add_succ_basic_block(bb);
		bb->add_pre_basic_block(pre_basic_block);
		auto term = pre_basic_block->get_terminator();
		unsigned size = static_cast<int>(term->get_operands().size());
		for (unsigned i = 0; i < size; i++)
		{
			auto op = term->get_operand(i);
			if (op == this)
			{
				term->set_operand(i, bb);
			}
		}
	}
	pre_bbs_.clear();
	return true;
}

void BasicBlock::replace_terminate_with_return_value(Value* value)
{
	assert(is_terminated() && "not terminated");
	auto pre = get_terminator();
	auto preRet = dynamic_cast<ReturnInst*>(pre);
	auto preBr = dynamic_cast<BranchInst*>(pre);
	if (!preRet)
	{
		if (preBr->is_cond_br())
		{
			auto t1 = dynamic_cast<BasicBlock*>(preBr->get_operand(1));
			auto t2 = dynamic_cast<BasicBlock*>(preBr->get_operand(2));
			t1->remove_pre_basic_block(this);
			t2->remove_pre_basic_block(this);
			remove_succ_basic_block(t1);
			remove_succ_basic_block(t2);
		}
		else
		{
			auto t1 = dynamic_cast<BasicBlock*>(preBr->get_operand(0));
			t1->remove_pre_basic_block(this);
			remove_succ_basic_block(t1);
		}
	}
	instr_list_.pop_back();
	ReturnInst::create_ret(value, this);
}

bool BasicBlock::is_terminated() const
{
	if (instr_list_.empty())
	{
		return false;
	}
	switch (instr_list_.back()->get_instr_type()) // NOLINT(clang-diagnostic-switch-enum)
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

Instruction* BasicBlock::get_terminator_or_null() const
{
	if (is_terminated()) return instr_list_.back();
	return nullptr;
}

void BasicBlock::add_instruction(Instruction* instr)
{
	ifThenOrThrow(instr->is_phi(), !is_entry_block(), "Phi can only insert to not entry block");
	ifThenOrThrow(instr->is_alloca(), is_entry_block(), "Alloca can only insert to entry block");
	ifThenOrThrow(!instr->is_alloca() && !instr->is_phi(), !is_terminated(), "Can not insert to terminated block");
	instr_list_.emplace_back(instr);
}

void BasicBlock::add_instr_begin(Instruction* instr)
{
	ifThenOrThrow(instr->is_phi(), !is_entry_block(), "Phi can only insert to not entry block");
	ifThenOrThrow(instr->is_alloca(), is_entry_block(), "Alloca can only insert to entry block");
	instr_list_.emplace_front(instr);
}

void BasicBlock::erase_instr(const Instruction* instr)
{
	instr_list_.remove_first(instr);
}

void BasicBlock::erase_instr_from_last(const Instruction* instr)
{
	instr_list_.remove_last(instr);
}

std::string BasicBlock::print()
{
	std::string bb_ir;
	bb_ir += this->get_name();
	bb_ir += ":";
	// print prebb
	if (!this->get_pre_basic_blocks().empty())
	{
		bb_ir += "\t\t; preds = ";
	}
	for (auto bb : this->get_pre_basic_blocks())
	{
		if (bb != *this->get_pre_basic_blocks().begin())
		{
			bb_ir += ", ";
		}
		bb_ir += print_as_op(bb, false);
	}
	if (!this->get_succ_basic_blocks().empty())
	{
		bb_ir += "\t\t; succs = ";
	}
	for (auto bb : this->get_succ_basic_blocks())
	{
		if (bb != *this->get_succ_basic_blocks().begin())
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
	for (const auto instr : this->get_instructions())
	{
		bb_ir += "  ";
		bb_ir += instr->print();
		bb_ir += "\n";
	}

	return bb_ir;
}

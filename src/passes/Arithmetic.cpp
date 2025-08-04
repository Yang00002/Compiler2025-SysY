#include "Arithmetic.hpp"
#include "Constant.hpp"

void Arithmetic::run()
{
	for (auto f : m_->get_functions())
		if (!f->is_declaration() && !f->is_lib_)
			run(f);
}

void Arithmetic::run(Function* f)
{
	worklist_.clear();
	for (auto b : f->get_basic_blocks())
	{
		for (auto i : b->get_instructions())
			worklist_.emplace_back(i);
	}
	for (auto i : worklist_)
		simplify(i);
}

void Arithmetic::simplify(Instruction* i) const
{
	switch (i->get_instr_type())
	{
		case Instruction::add:
		case Instruction::fadd:
		case Instruction::sub:
		case Instruction::fsub:
			return optimize_addsub(i);
		case Instruction::mul:
			return optimize_mul(i);
		case Instruction::fmul:
			return optimize_fmul(i);
		case Instruction::sdiv:
			return optimize_div(i);
		case Instruction::fdiv:
			return optimize_fdiv(i);
		case Instruction::srem:
			return optimize_rem(i);
		default: break;
	}
}

void Arithmetic::optimize_addsub(Instruction* i) const
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	if (op0 == op1)
	{
		if (i->is_fsub())
		{
			i->replace_all_use_with(Constant::create(m_, 0.0f));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (i->is_sub())
		{
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (auto constval = dynamic_cast<Constant*>(op0))
	{
		if (i->is_add() && (constval->isIntConstant() && constval->getIntConstant() == 0
		                    || constval->isFloatConstant() && constval->getFloatConstant() == 0.0f)
		)
		{
			i->replace_all_use_with(op1);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (auto constval = dynamic_cast<Constant*>(op1))
	{
		if (constval->isIntConstant() && constval->getIntConstant() == 0
		    || constval->isFloatConstant() && constval->getFloatConstant() == 0.0f
		)
		{
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
		}
	}
}

void Arithmetic::optimize_mul(Instruction* i) const
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	if (auto constval = dynamic_cast<Constant*>(op0))
	{
		auto const_op = constval->getIntConstant();
		if (const_op == 0)
		{
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (const_op == 1)
		{
			i->replace_all_use_with(op1);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (const_op == -1)
		{
			auto& instructions = i->get_parent()->get_instructions();
			for (auto it = instructions.begin(); it != instructions.end(); ++it)
			{
				if (*it == i)
				{
					auto subinst = IBinaryInst::create_sub(
						Constant::create(m_, 0), op1, nullptr);
					subinst->set_parent(i->get_parent());
					instructions.emplace_common_inst_after(subinst, it);
					i->replace_all_use_with(subinst);
					++it;
					it.remove_pre();
					delete i;
					break;
				}
			}
			return;
		}
	}
	if (auto constval = dynamic_cast<Constant*>(op1))
	{
		auto const_op = constval->getIntConstant();
		if (const_op == 0)
		{
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (const_op == 1)
		{
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (const_op == -1)
		{
			auto& instructions = i->get_parent()->get_instructions();
			for (auto it = instructions.begin(); it != instructions.end(); ++it)
			{
				if (*it == i)
				{
					auto subinst = IBinaryInst::create_sub(
						Constant::create(m_, 0), op0, nullptr);
					subinst->set_parent(i->get_parent());
					instructions.emplace_common_inst_after(subinst, it);
					i->replace_all_use_with(subinst);
					++it;
					it.remove_pre();
					delete i;
					break;
				}
			}
		}
	}
}

void Arithmetic::optimize_fmul(Instruction* i) const
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	if (auto constval = dynamic_cast<Constant*>(op0))
	{
		if (constval->getFloatConstant() == 0.0f)
		{
			i->replace_all_use_with(Constant::create(m_, 0.0f));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (constval->getFloatConstant() == 1.0f)
		{
			i->replace_all_use_with(op1);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (constval->getFloatConstant() == -1.0f)
		{
			auto& instructions = i->get_parent()->get_instructions();
			for (auto it = instructions.begin(); it != instructions.end(); ++it)
			{
				if (*it == i)
				{
					auto subinst = FBinaryInst::create_fsub(
						Constant::create(m_, 0.0f), op1, nullptr);
					subinst->set_parent(i->get_parent());
					instructions.emplace_common_inst_after(subinst, it);
					i->replace_all_use_with(subinst);
					i->get_parent()->erase_instr(i);
					delete i;
					break;
				}
			}
			return;
		}
	}
	if (auto constval = dynamic_cast<Constant*>(op1))
	{
		if (constval->getFloatConstant() == 0.0f)
		{
			i->replace_all_use_with(Constant::create(m_, 0.0f));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (constval->getFloatConstant() == 1.0f)
		{
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (constval->getFloatConstant() == -1.0f)
		{
			auto& instructions = i->get_parent()->get_instructions();
			for (auto it = instructions.begin(); it != instructions.end(); ++it)
			{
				if (*it == i)
				{
					auto subinst = FBinaryInst::create_fsub(
						Constant::create(m_, 0.0f), op0, nullptr);
					subinst->set_parent(i->get_parent());
					instructions.emplace_common_inst_after(subinst, it);
					i->replace_all_use_with(subinst);
					i->get_parent()->erase_instr(i);
					delete i;
					break;
				}
			}
		}
	}
}

void Arithmetic::optimize_rem(Instruction* i) const
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	if (op0 == op1)
	{
		i->replace_all_use_with(Constant::create(m_, 0));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (auto constval = dynamic_cast<Constant*>(op0))
	{
		auto const_op = constval->getIntConstant();
		if (const_op == 0)
		{
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (auto constval = dynamic_cast<Constant*>(op1))
	{
		auto const_op = constval->getIntConstant();
		if (const_op == 1 || const_op == -1)
		{
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
		}
	}
}

void Arithmetic::optimize_div(Instruction* i) const
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	if (op0 == op1)
	{
		i->replace_all_use_with(Constant::create(m_, 1));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (auto constval = dynamic_cast<Constant*>(op0))
	{
		if (constval->getIntConstant() == 0)
		{
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (auto constval = dynamic_cast<Constant*>(op1))
	{
		if (constval->getIntConstant() == 1)
		{
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (constval->getIntConstant() == -1)
		{
			auto& instructions = i->get_parent()->get_instructions();
			for (auto it = instructions.begin(); it != instructions.end(); ++it)
			{
				if (*it == i)
				{
					auto subinst = IBinaryInst::create_sub(
						Constant::create(m_, 0), op0, nullptr);
					subinst->set_parent(i->get_parent());
					instructions.emplace_common_inst_after(subinst, it);
					i->replace_all_use_with(subinst);
					++it;
					it.remove_pre();
					delete i;
					break;
				}
			}
		}
	}
}

void Arithmetic::optimize_fdiv(Instruction* i) const
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	if (op0 == op1)
	{
		i->replace_all_use_with(Constant::create(m_, 1.0f));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (auto constval = dynamic_cast<Constant*>(op0))
	{
		if (constval->getFloatConstant() == 0.0f)
		{
			i->replace_all_use_with(Constant::create(m_, 0.0f));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (auto constval = dynamic_cast<Constant*>(op1))
	{
		auto floatconst = constval->getFloatConstant();
		if (floatconst == 1.0f)
		{
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (floatconst == -1.0f)
		{
			auto& instructions = i->get_parent()->get_instructions();
			for (auto it = instructions.begin(); it != instructions.end(); ++it)
			{
				if (*it == i)
				{
					auto subinst = FBinaryInst::create_fsub(
						Constant::create(m_, 0.0f), op0, nullptr);
					subinst->set_parent(i->get_parent());
					instructions.emplace_common_inst_after(subinst, it);
					i->replace_all_use_with(subinst);
					++it;
					it.remove_pre();
					break;
				}
			}
		}
	}
}

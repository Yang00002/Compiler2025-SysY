#include "Arithmetic.hpp"

#include <stdexcept>

#include "Config.hpp"
#include "Constant.hpp"
#include "CountLZ.hpp"

#define DEBUG 0
#include "Util.hpp"

void Arithmetic::run()
{
	LOG(color::cyan("Run Arithmetic Pass"));
	PUSH;
	for (auto f : m_->get_functions())
		if (!f->is_declaration() && !f->is_lib_)
			run(f);
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("Arithmetic Done"));
}

void Arithmetic::run(Function* f)
{
	inlist_.clear();
	inSigList_.clear();
	for (auto b : f->get_basic_blocks())
	{
		for (auto i : b->get_instructions())
		{
			worklist_.emplace(i);
			inlist_.emplace(i);
			sigWorklist_.emplace(i);
			inSigList_.emplace(i);
		}
	}
	while (!worklist_.empty())
	{
		while (!sigWorklist_.empty())
		{
			auto j = sigWorklist_.front();
			sigWorklist_.pop();
			inSigList_.erase(j);
			decideSignal(j);
		}
		auto i = worklist_.front();
		worklist_.pop();
		inlist_.erase(i);
		simplify(i);
	}
}

void Arithmetic::simplify(Instruction* i)
{
	switch (i->get_instr_type())
	{
		case Instruction::add:
		case Instruction::fadd:
		case Instruction::sub:
		case Instruction::fsub:
			return optimize_addsub(i);
		case Instruction::mul:
			return optimize_mul(dynamic_cast<IBinaryInst*>(i));
		case Instruction::fmul:
			return optimize_fmul(dynamic_cast<FBinaryInst*>(i));
		case Instruction::sdiv:
			return optimize_div(dynamic_cast<IBinaryInst*>(i));
		case Instruction::fdiv:
			return optimize_fdiv(dynamic_cast<FBinaryInst*>(i));
		case Instruction::srem:
			return optimize_rem(dynamic_cast<IBinaryInst*>(i));
		case Instruction::shl:
			return optimize_shl(dynamic_cast<IBinaryInst*>(i));
		case Instruction::and_:
			return optimize_and(dynamic_cast<IBinaryInst*>(i));
		default: break;
	}
}

void Arithmetic::optimize_addsub(Instruction* i)
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	if (op0 == op1)
	{
		if (i->is_fsub())
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0.0f));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (i->is_sub())
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	auto c0 = dynamic_cast<Constant*>(op0);
	auto c1 = dynamic_cast<Constant*>(op1);
	if (c0 && c1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		if (i->is_add())
			i->replace_all_use_with(Constant::create(m_, c0->getIntConstant() + c1->getIntConstant()));
		else if (i->is_sub())
			i->replace_all_use_with(Constant::create(m_, c0->getIntConstant() - c1->getIntConstant()));
		else if (i->is_fadd())
			i->replace_all_use_with(Constant::create(m_, c0->getFloatConstant() + c1->getFloatConstant()));
		else
			i->replace_all_use_with(Constant::create(m_, c0->getFloatConstant() - c1->getFloatConstant()));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (c0)
	{
		if (i->is_add() && (c0->isIntConstant() && c0->getIntConstant() == 0
		                    || c0->isFloatConstant() && c0->getFloatConstant() == 0.0f)
		)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op1);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (c1)
	{
		if (c1->isIntConstant() && c1->getIntConstant() == 0
		    || c1->isFloatConstant() && c1->getFloatConstant() == 0.0f
		)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
		}
	}
}

void Arithmetic::addAroundToWorkList(const Value* i)
{
	for (auto& use : i->get_use_list())
	{
		auto inst = dynamic_cast<Instruction*>(use.val_);
		if (inst != i && !inlist_.count(inst))
		{
			inlist_.emplace(inst);
			worklist_.emplace(inst);
		}
	}
}

void Arithmetic::addAroundToSigWorkList(const Value* i)
{
	for (auto& use : i->get_use_list())
	{
		auto inst = dynamic_cast<Instruction*>(use.val_);
		if (inst != i && !inSigList_.count(inst))
		{
			inSigList_.emplace(inst);
			sigWorklist_.emplace(inst);
		}
	}
}

void Arithmetic::addAroundToSigWorkList(const Value* i, const Value* exclude)
{
	for (auto& use : i->get_use_list())
	{
		auto inst = dynamic_cast<Instruction*>(use.val_);
		if (inst != i && exclude != i && !inSigList_.count(inst))
		{
			inSigList_.emplace(inst);
			sigWorklist_.emplace(inst);
		}
	}
}

void Arithmetic::addToSignalWorkList(Value* i)
{
	auto c = dynamic_cast<Instruction*>(i);
	if (c != nullptr && !inSigList_.count(c))
	{
		inSigList_.emplace(c);
		sigWorklist_.emplace(c);
	}
}

void Arithmetic::addToWorkList(Value* i)
{
	auto c = dynamic_cast<Instruction*>(i);
	if (c != nullptr && !inlist_.count(c))
	{
		inlist_.emplace(c);
		worklist_.emplace(c);
	}
}

void Arithmetic::optimize_mul(IBinaryInst* i)
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	auto c0 = dynamic_cast<Constant*>(op0);
	auto c1 = dynamic_cast<Constant*>(op1);
	if (c0 && c1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, c0->getIntConstant() * c1->getIntConstant()));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (c0)
	{
		auto const_op = c0->getIntConstant();
		if (const_op == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (const_op == 1)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op1);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (const_op == -1)
		{
			LOG(color::green("simplify ") + i->print());
			addToSignalWorkList(i);
			i->set_operand(0, Constant::create(m_, 0));
			i->setOp(Instruction::sub);
			return;
		}
		if (const_op > 0 && (const_op & (const_op - 1)) == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addToSignalWorkList(i);
			i->set_operand(0, op1);
			i->set_operand(1, Constant::create(m_, m_countr_zero(i2uKeepBits(const_op))));
			i->setOp(Instruction::shl);
			return;
		}
		if (const_op < 0 && ((-const_op) & (-const_op - 1)) == 0)
		{
			int move = m_countr_zero(i2uKeepBits(-const_op));
			auto& instructions = i->get_parent()->get_instructions();
			for (auto it = instructions.begin(); it != instructions.end(); ++it)
			{
				if (*it == i)
				{
					LOG(color::green("simplify ") + i->print());
					auto shl = IBinaryInst::create_shl(
						i, Constant::create(m_, move), nullptr);
					shl->set_parent(i->get_parent());
					instructions.emplace_common_inst_after(shl, it);
					i->replace_all_use_with(shl);
					shl->set_operand(0, i);
					i->set_operand(0, Constant::create(m_, 0));
					i->setOp(Instruction::sub);
					addToSignalWorkList(shl);
					addToSignalWorkList(i);
					break;
				}
			}
			return;
		}
	}
	if (c1)
	{
		auto const_op = c1->getIntConstant();
		if (const_op == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (const_op == 1)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (const_op == -1)
		{
			LOG(color::green("simplify ") + i->print());
			addToSignalWorkList(i);
			i->set_operand(0, Constant::create(m_, 0));
			i->set_operand(1, op0);
			i->setOp(Instruction::sub);
			return;
		}
		if (const_op > 0 && (const_op & (const_op - 1)) == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addToSignalWorkList(i);
			i->set_operand(1, Constant::create(m_, m_countr_zero(i2uKeepBits(const_op))));
			i->setOp(Instruction::shl);
			return;
		}
		if (const_op < 0 && ((-const_op) & (-const_op - 1)) == 0)
		{
			int move = m_countr_zero(i2uKeepBits(-const_op));
			auto& instructions = i->get_parent()->get_instructions();
			for (auto it = instructions.begin(); it != instructions.end(); ++it)
			{
				if (*it == i)
				{
					LOG(color::green("simplify ") + i->print());
					auto shl = IBinaryInst::create_shl(
						i, Constant::create(m_, move), nullptr);
					shl->set_parent(i->get_parent());
					instructions.emplace_common_inst_after(shl, it);
					i->replace_all_use_with(shl);
					shl->set_operand(0, i);
					i->set_operand(0, Constant::create(m_, 0));
					i->set_operand(1, op0);
					i->setOp(Instruction::sub);
					addToSignalWorkList(shl);
					addToSignalWorkList(i);
					break;
				}
			}
		}
	}
}

void Arithmetic::optimize_fmul(FBinaryInst* i)
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	auto c0 = dynamic_cast<Constant*>(op0);
	auto c1 = dynamic_cast<Constant*>(op1);
	if (c0 && c1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, c0->getFloatConstant() * c1->getFloatConstant()));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (c0)
	{
		float constval = c0->getFloatConstant();
		if (constval == 0.0f)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0.0f));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (constval == 1.0f)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op1);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (constval == -1.0f)
		{
			LOG(color::green("simplify ") + i->print());
			addToSignalWorkList(i);
			i->set_operand(0, Constant::create(m_, 0.0f));
			i->setOp(Instruction::fsub);
			return;
		}
	}
	if (c1)
	{
		float constval = c1->getFloatConstant();
		if (constval == 0.0f)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0.0f));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (constval == 1.0f)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (constval == -1.0f)
		{
			LOG(color::green("simplify ") + i->print());
			addToSignalWorkList(i);
			i->set_operand(0, Constant::create(m_, 0.0f));
			i->set_operand(1, op0);
			i->setOp(Instruction::fsub);
		}
	}
}

void Arithmetic::optimize_rem(IBinaryInst* i)
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	if (op0 == op1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, 0));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	auto c0 = dynamic_cast<Constant*>(op0);
	auto c1 = dynamic_cast<Constant*>(op1);
	if (c0 && c1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, c0->getIntConstant() % c1->getIntConstant()));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (c0)
	{
		auto const_op = c0->getIntConstant();
		if (const_op == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (c1)
	{
		auto const_op = c1->getIntConstant();
		if (const_op == 1 || const_op == -1)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		int cop2 = const_op < 0 ? -const_op : const_op;
		if ((cop2 & (cop2 - 1)) == 0)
		{
			auto& ul = i->get_use_list();
			if (ul.size() == 1)
			{
				auto use = i->get_use_list().front();
				auto inst = dynamic_cast<ICmpInst*>(use.val_);
				if (inst != nullptr)
				{
					auto c = dynamic_cast<Constant*>(inst->get_operand(use.arg_no_ == 0 ? 1 : 0));
					if (c != nullptr && c->getIntConstant() == 0 && (
						inst->get_instr_type() == Instruction::eq || inst->get_instr_type() == Instruction::ne))
					{
						LOG(color::green("simplify ") + i->print());
						i->set_operand(1, Constant::create(m_, cop2 - 1));
						i->setOp(Instruction::and_);
						return;
					}
				}
			}
		}
		if ((signalOf(op0) & 1) == 0)
		{
			if (const_op < 0) const_op = -const_op;
			if ((const_op & (const_op - 1)) == 0)
			{
				LOG(color::green("simplify ") + i->print());
				addToSignalWorkList(i);
				int val = const_op - 1;
				i->set_operand(1, Constant::create(m_, val));
				i->setOp(Instruction::and_);
			}
		}
	}
}

void Arithmetic::optimize_shl(IBinaryInst* i)
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	auto c0 = dynamic_cast<Constant*>(op0);
	auto c1 = dynamic_cast<Constant*>(op1);
	if (c0 && c1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, c0->getIntConstant() << c1->getIntConstant()));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (c0)
	{
		auto const_op = c0->getIntConstant();
		if (const_op == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (c1)
	{
		auto const_op = c1->getIntConstant();
		if (const_op == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
		}
	}
}

void Arithmetic::optimize_ashr(IBinaryInst* i)
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	auto c0 = dynamic_cast<Constant*>(op0);
	auto c1 = dynamic_cast<Constant*>(op1);
	if (c0 && c1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, c0->getIntConstant() >> c1->getIntConstant()));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (c0)
	{
		auto const_op = c0->getIntConstant();
		if (const_op == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (c1)
	{
		auto const_op = c1->getIntConstant();
		if (const_op == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
		}
	}
}

void Arithmetic::optimize_and(IBinaryInst* i)
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	auto c0 = dynamic_cast<Constant*>(op0);
	auto c1 = dynamic_cast<Constant*>(op1);
	if (c0 && c1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, c0->getIntConstant() & c1->getIntConstant()));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (c0)
	{
		auto const_op = c0->getIntConstant();
		if (const_op == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (c1)
	{
		auto const_op = c1->getIntConstant();
		if (const_op == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (const_op == -1)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
		}
	}
}

void Arithmetic::decideSignal(Instruction* i)
{
	switch (const auto op = i->get_instr_type()) // NOLINT(clang-diagnostic-switch-enum)
	{
		case Instruction::add:
		case Instruction::sub:
		case Instruction::mul:
		case Instruction::sdiv:
		case Instruction::srem:
		case Instruction::shl:
		case Instruction::ashr:
		case Instruction::and_:
		case Instruction::fadd:
		case Instruction::fsub:
		case Instruction::fmul:
		case Instruction::fdiv:
			{
				LOG(color::yellow("decide signal of ") + i->print());
				auto l = i->get_operand(0);
				auto r = i->get_operand(1);
				auto ty = signalOf(i);
				auto tl = signalOf(l);
				auto tr = signalOf(r);
				auto spread = infer2Op(ty, tl, tr, op);
				bool c = false;
				if (spread.t != ty)
				{
					c = true;
					designateSignal(i, spread.t);
					addAroundToSigWorkList(i);
					addAroundToWorkList(i);
				}
				if (spread.l != tl)
				{
					c = true;
					designateSignal(l, spread.l);
					addAroundToSigWorkList(l, i);
					if (l != i)addToSignalWorkList(l);
					addAroundToWorkList(l);
				}
				if (spread.r != tr)
				{
					c = true;
					designateSignal(r, spread.r);
					addAroundToSigWorkList(r, i);
					if (r != i)addToSignalWorkList(r);
					addAroundToWorkList(r);
				}
				if (c)
				{
					LOG("original " + std::to_string(ty) + " " + std::to_string(tl) + " " + std::to_string(tr));
					LOG("to " + std::to_string(spread.t) + " " + std::to_string(spread.l) + " " + std::to_string(spread.
						r));
				}
				break;
			}
		case Instruction::phi:
			{
				LOG(color::yellow("decide signal of ") + i->print());
				auto phi = dynamic_cast<PhiInst*>(i);
				auto pairs = phi->get_phi_pairs();
				auto ty = signalOf(i);
				LOG("original " + std::to_string(ty));
				PUSH;
				auto nty = 0u;
				for (auto [v,j] : pairs)
				{
					auto ti = signalOf(v);
					auto t = ti & ty;
					nty |= t;
					if (dynamic_cast<Instruction*>(v) && t != ti)
					{
						LOG("parameter " + v->print() + " from " + std::to_string(ti) + " to " + std::to_string(t));
						designateSignal(v, t);
						addAroundToSigWorkList(v, i);
						if (v != i) addToSignalWorkList(v);
						addAroundToWorkList(v);
					}
				}
				POP;
				if (nty != ty)
				{
					LOG("to " + std::to_string(nty));
					designateSignal(i, nty);
					addAroundToSigWorkList(i);
					addAroundToWorkList(i);
				}
				break;
			}
		case Instruction::getelementptr:
			{
				auto& ops = i->get_operands();
				int size = u2iNegThrow(ops.size());
				for (int k = 1; k < size; k++)
				{
					auto get = ops[k];
					if (dynamic_cast<Instruction*>(get))
					{
						auto ty = signalOf(get);
						auto nt = ty & 6;
						if (nt != ty)
						{
							designateSignal(get, nt);
							addAroundToSigWorkList(get, i);
							if (get != i)
							{
								addToSignalWorkList(get);
								addAroundToWorkList(get);
							}
						}
					}
				}
				break;
			}
		default: break;
	}
}

bool Arithmetic::haveConstType(Value* val)
{
	auto c = dynamic_cast<Constant*>(val);
	if (c != nullptr) return true;
	auto fd = constType_.find(val);
	return (fd != constType_.end());
}

Arithmetic::CCTuple Arithmetic::inferAdd(unsigned t, unsigned l, unsigned r)
{
	unsigned up = 7;
	unsigned ntm;
	do
	{
		if (up & 1)
		{
			ntm = t & addSignal(l, r);
			if (t != ntm)
			{
				up = 6;
				t = ntm;
			}
			else up &= 6;
		}
		if (up & 2)
		{
			ntm = l & addSignal(t, flipSignal(r));
			if (l != ntm)
			{
				up = 5;
				l = ntm;
			}
			else up &= 5;
		}
		if (up & 4)
		{
			ntm = r & addSignal(t, flipSignal(l));
			if (r != ntm)
			{
				up = 3;
				r = ntm;
			}
			else up &= 3;
		}
	}
	while (up);
	return {t, l, r};
}

Arithmetic::CCTuple Arithmetic::inferShl(unsigned t, unsigned l, unsigned r)
{
	ASSERT((r & 1) == 0);
	l &= t;
	t &= l;
	return {t, l, r};
}

Arithmetic::CCTuple Arithmetic::inferAshr(unsigned t, unsigned l, unsigned r)
{
	ASSERT((r & 1) == 0);
	unsigned ll = l;
	if (l & 4u) ll |= 2u;
	t &= ll;
	l &= t;
	return {t, l, r};
}

Arithmetic::CCTuple Arithmetic::inferAnd(unsigned t, unsigned l, unsigned r)
{
	unsigned up = 7;
	unsigned ntm;
	do
	{
		if (up & 1)
		{
			ntm = t & andSignal(l, r);
			if (t != ntm)
			{
				up = 6;
				t = ntm;
			}
			else up &= 6;
		}
		if (up & 2)
		{
			ntm = l & andSignalL(t, r);
			if (l != ntm)
			{
				up = 5;
				l = ntm;
			}
			else up &= 5;
		}
		if (up & 4)
		{
			ntm = r & andSignalL(t, l);
			if (r != ntm)
			{
				up = 3;
				r = ntm;
			}
			else up &= 3;
		}
	}
	while (up);
	return {t, l, r};
}

Arithmetic::CCTuple Arithmetic::infer2Op(unsigned t, unsigned l, unsigned r, Instruction::OpID op)
{
	switch (op) // NOLINT(clang-diagnostic-switch-enum)
	{
		case Instruction::add:
		case Instruction::fadd:
			return inferAdd(t, l, r);
		case Instruction::sub:
		case Instruction::fsub:
			{
				auto ret = inferAdd(l, t, r);
				return {ret.l, ret.t, ret.r};
			}
		case Instruction::mul:
		case Instruction::fmul:
			return inferMul(t, l, r);
		case Instruction::sdiv:
			return inferSdiv(t, l, r);
		case Instruction::fdiv:
			return inferFdiv(t, l, r);
		case Instruction::srem:
			return inferRem(t, l, r);
		case Instruction::shl:
			return inferShl(t, l, r);
		case Instruction::ashr:
			return inferAshr(t, l, r);
		case Instruction::and_:
			return inferAnd(t, l, r);
		default: break;
	}
	throw std::runtime_error("unexpected");
}

Arithmetic::CCTuple Arithmetic::inferMul(unsigned t, unsigned l, unsigned r)
{
	unsigned up = 7;
	unsigned ntm;
	do
	{
		if (up & 1)
		{
			ntm = t & multiplySignal(l, r);
			if (t != ntm)
			{
				up = 6;
				t = ntm;
			}
			else up &= 6;
		}
		if (up & 2)
		{
			ntm = l & mdivideCond(t, r);
			if (l != ntm)
			{
				up = 5;
				l = ntm;
			}
			else up &= 5;
		}
		if (up & 4)
		{
			ntm = r & mdivideCond(t, l);
			if (r != ntm)
			{
				up = 3;
				r = ntm;
			}
			else up &= 3;
		}
	}
	while (up);
	return {t, l, r};
}

Arithmetic::CCTuple Arithmetic::inferRem(unsigned t, unsigned l, unsigned r)
{
	r &= 5;
	unsigned up = 3;
	unsigned ntm;
	do
	{
		if (up & 1)
		{
			ntm = t & remSignal(l, r);
			if (t != ntm)
			{
				up = 2;
				t = ntm;
			}
			else up &= 2;
		}
		if (up & 2)
		{
			ntm = l & remSignalL(t, r);
			if (l != ntm)
			{
				up = 1;
				l = ntm;
			}
			else up &= 1;
		}
	}
	while (up);
	return {t, l, r};
}

Arithmetic::CCTuple Arithmetic::inferSdiv(unsigned t, unsigned l, unsigned r)
{
	r &= 5;
	unsigned up = 7;
	unsigned ntm;
	do
	{
		if (up & 1)
		{
			ntm = t & sdivideSignal(l, r);
			if (t != ntm)
			{
				up = 6;
				t = ntm;
			}
			else up &= 6;
		}
		if (up & 2)
		{
			ntm = l & multiplySignal(t, r);
			if (l != ntm)
			{
				up = 5;
				l = ntm;
			}
			else up &= 5;
		}
		if (up & 4)
		{
			ntm = r & mdivideCond(l, t);
			if (r != ntm)
			{
				up = 3;
				r = ntm;
			}
			else up &= 3;
		}
	}
	while (up);
	return {t, l, r};
}

Arithmetic::CCTuple Arithmetic::inferFdiv(unsigned t, unsigned l, unsigned r)
{
	r &= 5;
	unsigned up = 7;
	unsigned ntm;
	do
	{
		if (up & 1)
		{
			ntm = t & mdivideCond(l, r);
			if (t != ntm)
			{
				up = 6;
				t = ntm;
			}
			else up &= 6;
		}
		if (up & 2)
		{
			ntm = l & multiplySignal(t, r);
			if (l != ntm)
			{
				up = 5;
				l = ntm;
			}
			else up &= 5;
		}
		if (up & 4)
		{
			ntm = r & mdivideCond(l, t);
			if (r != ntm)
			{
				up = 3;
				r = ntm;
			}
			else up &= 3;
		}
	}
	while (up);
	return {t, l, r};
}

unsigned Arithmetic::sdivideSignal(unsigned l, unsigned r)
{
	if ((r & 5u) == 0) return 0;
	unsigned ret = 0u;
	unsigned lra = l & r;
	if (lra & 5u) ret |= 6u;
	unsigned lrfa = l & flipSignal(r);
	if (lrfa & 5u) ret |= 3u;
	ret |= (l & 2u);
	return ret;
}

unsigned Arithmetic::multiplySignal(unsigned l, unsigned r)
{
	unsigned ret = (l & 4u) ? r : 0u;
	if (r) ret |= 2u;
	if (l & 1u) ret |= flipSignal(r);
	return ret;
}

unsigned Arithmetic::andSignal(unsigned l, unsigned r)
{
	unsigned ret = l & r;
	unsigned orr = l | r;
	if (orr & 4u) ret |= 2u;
	if (l & flipSignal(r) & 5u) ret |= 4u;
	return ret;
}

unsigned Arithmetic::andSignalL(unsigned l, unsigned r)
{
	static constexpr unsigned ret[3][8] = {
		{0u, 1u, 0u, 1u, 0u, 1u, 0u, 1u}, {0u, 6u, 7u, 7u, 7u, 7u, 7u, 7u}, {0u, 4u, 7u, 7u, 5u, 5u, 7u, 7u}
	};
	unsigned get = 0;
	if (l & 1u) get |= ret[0][r];
	if (l & 2u) get |= ret[1][r];
	if (l & 4u) get |= ret[2][r];
	return get;
}

unsigned Arithmetic::remSignal(unsigned l, unsigned r)
{
	if ((r & 5u) == 0u) return 0u;
	return l | 2u;
}

unsigned Arithmetic::remSignalL(unsigned l, unsigned r)
{
	if (dangerousSignalInfer && l == 6u && r == 4u) return 6u;
	if ((r & 5u) == 0u) return 0u;
	return (l & 2u) ? 7u : l;
}

unsigned Arithmetic::addSignal(unsigned l, unsigned r)
{
	static constexpr unsigned ret[3][8] = {
		{0u, 1u, 1u, 1u, 7u, 7u, 7u, 7u}, {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}, {0u, 7u, 4u, 7u, 4u, 7u, 4u, 7u}
	};
	unsigned get = 0;
	if (l & 1u) get |= ret[0][r];
	if (l & 2u) get |= ret[1][r];
	if (l & 4u) get |= ret[2][r];
	return get;
}

unsigned Arithmetic::mdivideCond(unsigned l, unsigned r)
{
	unsigned ret = (l & r) ? 4u : 0u;
	if (l & flipSignal(r)) ret |= 1u;
	if (l & 2u) ret |= 2u;
	return ret;
}

unsigned Arithmetic::flipSignal(unsigned i)
{
	static constexpr unsigned rets[8] = {0u, 4u, 2u, 6u, 1u, 5u, 3u, 7u};
	return rets[i];
}

void Arithmetic::designateSignal(Value* val, unsigned ty)
{
	constType_[val] = ty;
}

unsigned Arithmetic::signalOf(Value* val)
{
	auto c = dynamic_cast<Constant*>(val);
	if (c != nullptr)
	{
		if (c->isFloatConstant())
		{
			auto f = c->getFloatConstant();
			if (f > 0) return 4u;
			if (f == 0) return 2u;
			return 1;
		}
		auto f = c->getIntConstant();
		if (f > 0) return 4u;
		if (f == 0) return 2u;
		return 1u;
	}
	auto fd = constType_.find(val);
	if (fd != constType_.end()) return fd->second;
	return 7u;
}

void Arithmetic::optimize_div(IBinaryInst* i)
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	if (op0 == op1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, 1));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	auto c0 = dynamic_cast<Constant*>(op0);
	auto c1 = dynamic_cast<Constant*>(op1);
	if (c0 && c1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, c0->getIntConstant() / c1->getIntConstant()));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (c0)
	{
		auto constval = c0->getIntConstant();
		if (constval == 0)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (c1)
	{
		auto constval = c1->getIntConstant();
		if (constval == 1)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (constval == -1)
		{
			LOG(color::green("simplify ") + i->print());
			addToSignalWorkList(i);
			i->set_operand(0, Constant::create(m_, 0));
			i->set_operand(1, op0);
			i->setOp(Instruction::sub);
			return;
		}
		if (constval > 0)
		{
			if (((signalOf(op0) & 1u) == 0) && (constval & (constval - 1)) == 0)
			{
				LOG(color::green("simplify ") + i->print());
				addToSignalWorkList(i);
				i->set_operand(1, Constant::create(m_, m_countr_zero(i2uKeepBits(constval))));
				i->setOp(Instruction::ashr);
				return;
			}
		}
		else
		{
			if ((signalOf(op0) & 4u) == 0 && (-constval & (-constval - 1)) == 0)
			{
				int move = m_countr_zero(i2uKeepBits(-constval));
				auto& instructions = i->get_parent()->get_instructions();
				for (auto it = instructions.begin(); it != instructions.end(); ++it)
				{
					if (*it == i)
					{
						LOG(color::green("simplify ") + i->print());
						auto ashr = IBinaryInst::create_ashr(
							i, Constant::create(m_, move), nullptr);
						ashr->set_parent(i->get_parent());
						instructions.emplace_common_inst_after(ashr, it);
						i->replace_all_use_with(ashr);
						ashr->set_operand(0, i);
						i->set_operand(0, Constant::create(m_, 0));
						i->setOp(Instruction::sub);
						addToSignalWorkList(ashr);
						addToSignalWorkList(i);
						break;
					}
				}
			}
		}
	}
}

void Arithmetic::optimize_fdiv(FBinaryInst* i)
{
	auto op0 = i->get_operand(0);
	auto op1 = i->get_operand(1);
	if (op0 == op1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, 1.0f));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	auto c0 = dynamic_cast<Constant*>(op0);
	auto c1 = dynamic_cast<Constant*>(op1);
	if (c0 && c1)
	{
		LOG(color::green("simplify ") + i->print());
		addAroundToWorkList(i);
		addAroundToSigWorkList(i);
		i->replace_all_use_with(Constant::create(m_, c0->getFloatConstant() / c1->getFloatConstant()));
		i->get_parent()->erase_instr(i);
		delete i;
		return;
	}
	if (c0)
	{
		if (c0->getFloatConstant() == 0.0f)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(Constant::create(m_, 0.0f));
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
	}
	if (c1)
	{
		auto floatconst = c1->getFloatConstant();
		if (floatconst == 1.0f)
		{
			LOG(color::green("simplify ") + i->print());
			addAroundToWorkList(i);
			addAroundToSigWorkList(i);
			i->replace_all_use_with(op0);
			i->get_parent()->erase_instr(i);
			delete i;
			return;
		}
		if (floatconst == -1.0f)
		{
			LOG(color::green("simplify ") + i->print());
			addToSignalWorkList(i);
			i->set_operand(0, Constant::create(m_, 0));
			i->set_operand(1, op0);
			i->setOp(Instruction::fsub);
		}
	}
}

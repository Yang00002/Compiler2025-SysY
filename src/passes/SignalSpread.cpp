#include "SignalSpread.hpp"

#include "Arithmetic.hpp"
#include "BasicBlock.hpp"
#include "Instruction.hpp"
#include "Constant.hpp"
#include "Type.hpp"


#include "Util.hpp"

using namespace std;

namespace
{
	std::string printSig(unsigned s)
	{
		std::string ret;
		if (s & 1u) ret += "1";
		else ret += "0";
		if (s & 2u) ret += "1";
		else ret += "0";
		if (s & 4u) ret += "1";
		else ret += "0";
		return ret;
	}
}


void SignalSpread::run()
{
	signalSpread();
}


void SignalSpread::addToSignalWorkList(Value* i)
{
	auto c = dynamic_cast<Instruction*>(i);
	if (c != nullptr && !inSigList_.count(c))
	{
		inSigList_.emplace(c);
		sigWorklist_.emplace(c);
	}
}


void SignalSpread::signalSpread()
{
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_) continue;
		for (auto bb : f->get_basic_blocks())
		{
			for (auto inst : bb->get_instructions())
			{
				switch (inst->get_instr_type())
				{
					case Instruction::ret:
					case Instruction::add:
					case Instruction::sub:
					case Instruction::mul:
					case Instruction::mull:
					case Instruction::sdiv:
					case Instruction::srem:
					case Instruction::shl:
					case Instruction::ashr:
					case Instruction::and_:
					case Instruction::fadd:
					case Instruction::fsub:
					case Instruction::fmul:
					case Instruction::fdiv:
					case Instruction::load:
					case Instruction::phi:
					case Instruction::fptosi:
					case Instruction::sitofp:
					case Instruction::call:
						addToSignalWorkList(inst);
						break;
					case Instruction::br:
					case Instruction::alloca_:
					case Instruction::store:
					case Instruction::ge:
					case Instruction::gt:
					case Instruction::le:
					case Instruction::lt:
					case Instruction::eq:
					case Instruction::ne:
					case Instruction::fge:
					case Instruction::fgt:
					case Instruction::fle:
					case Instruction::flt:
					case Instruction::feq:
					case Instruction::fne:
					case Instruction::zext:
					case Instruction::memcpy_:
					case Instruction::memclear_:
					case Instruction::nump2charp:
					case Instruction::global_fix:
					case Instruction::msub:
					case Instruction::madd:
					case Instruction::mneg:
					case Instruction::getelementptr:
						break;
				}
			}
		}
	}
	while (!sigWorklist_.empty())
	{
		auto work = sigWorklist_.front();
		sigWorklist_.pop();
		inSigList_.erase(work);
		switch (work->get_instr_type())
		{
			case Instruction::ret:
				spreadRet(work);
				break;
			case Instruction::add:
			case Instruction::fadd:
				spreadAdd(work);
				break;
			case Instruction::sub:
			case Instruction::fsub:
				spreadSub(work);
				break;
			case Instruction::mul:
			case Instruction::mull:
			case Instruction::fmul:
				spreadMul(work);
				break;
			case Instruction::sdiv:
				spreadSdiv(work);
				break;
			case Instruction::srem:
				spreadSrem(work);
				break;
			case Instruction::shl:
				spreadShl(work);
				break;
			case Instruction::ashr:
				spreadAshr(work);
				break;
			case Instruction::and_:
				spreadAnd(work);
				break;
			case Instruction::fdiv:
				spreadFDiv(work);
				break;
			case Instruction::load:
				spreadLoad(work);
				break;
			case Instruction::phi:
				spreadPhi(work);
				break;
			case Instruction::fptosi:
				spreadFP2SI(work);
				break;
			case Instruction::sitofp:
				spreadSI2FP(work);
				break;
			case Instruction::call:
				spreadCall(work);
				break;
			case Instruction::br:
			case Instruction::alloca_:
			case Instruction::store:
			case Instruction::ge:
			case Instruction::gt:
			case Instruction::le:
			case Instruction::lt:
			case Instruction::eq:
			case Instruction::ne:
			case Instruction::fge:
			case Instruction::fgt:
			case Instruction::fle:
			case Instruction::flt:
			case Instruction::feq:
			case Instruction::fne:
			case Instruction::zext:
			case Instruction::memcpy_:
			case Instruction::memclear_:
			case Instruction::nump2charp:
			case Instruction::global_fix:
			case Instruction::msub:
			case Instruction::madd:
			case Instruction::mneg:
			case Instruction::getelementptr:
				break;
		}
	}
	for (auto i : constType_)
	{
		LOG(color::green("signal ") + printSig(i.second) + color::green(" for ") + i.first->print());
	}
}

void SignalSpread::designateSignal(Value* val, unsigned ty)
{
	constType_[val] = ty;
}

void SignalSpread::spreadRet(Instruction* inst)
{
	if (inst->get_type() == Types::VOID) return;
	auto sig = signalOf2Spread(inst->get_operand(0));
	if (sig == 0) return;
	for (auto use : inst->get_parent()->get_parent()->get_use_list())
	{
		auto call = dynamic_cast<CallInst*>(use.val_);
		auto oc = signalOf2Spread(call);
		auto oc2 = oc | sig;
		if (oc2 != oc)
		{
			designateSignal(call, oc2);
			for (auto use2 : call->get_use_list())
				addToSignalWorkList(use2.val_);
		}
	}
}

void SignalSpread::spreadUseIfSetSig(Value* inst, unsigned sig)
{
	auto sig0 = signalOf2Spread(inst);
	sig |= sig0;
	if (sig0 == sig) return;
	designateSignal(inst, sig);
	for (auto use : inst->get_use_list())
		addToSignalWorkList(use.val_);
}

namespace
{
	bool haveNegativeAndPositive(unsigned sig1)
	{
		return (sig1 & 0b101u) == 0b101u;
	}

	bool haveNegativeOrPositive(unsigned sig1)
	{
		return sig1 & 0b101u;
	}

	bool haveNegative(unsigned sig1)
	{
		return (sig1 & 0b001u) == 0b001u;
	}

	bool havePositive(unsigned sig1)
	{
		return (sig1 & 0b100u) == 0b100u;
	}

	bool haveZeroAndPositive(unsigned sig1)
	{
		return (sig1 & 0b011u) == 0b011u;
	}

	bool haveZeroAndNegative(unsigned sig1)
	{
		return (sig1 & 0b110u) == 0b110u;
	}

	bool onlyZero(unsigned sig1)
	{
		return sig1 == 0b010u;
	}

	unsigned allSig()
	{
		return 0b111u;
	}

	unsigned positiveSig()
	{
		return 0b100u;
	}

	unsigned negativeSig()
	{
		return 0b001u;
	}

	unsigned zeroSig()
	{
		return 0b010u;
	}

	bool haveZero(unsigned sig)
	{
		return sig & 0b010u;
	}

	unsigned removeZero(unsigned sig)
	{
		return sig & 0b101u;
	}

	bool greaterOrEqual(unsigned sig0, unsigned sig1)
	{
		if (sig0 & 1u)
		{
			if (sig1 == 1u) return true;
			return false;
		}
		if (sig0 & 2u)
		{
			if ((sig1 & 4u) == 0) return true;
			return false;
		}
		return true;
	}

	bool greaterThan(unsigned sig0, unsigned sig1)
	{
		if (sig0 & 1u) return false;
		if (sig0 & 2u)
		{
			if (sig1 == 1u) return true;
			return false;
		}
		return (sig1 & 4u) == 0;
	}

	bool equal(unsigned sig0, unsigned sig1)
	{
		if (sig0 == 1u)
		{
			if (sig1 == 1u) return true;
			return false;
		}
		if (sig0 == 2u)
		{
			if (sig1 == 2u) return true;
			return false;
		}
		if (sig0 == 4u)
		{
			if (sig1 == 4u) return true;
			return false;
		}
		return false;
	}

	bool notEqual(unsigned sig0, unsigned sig1)
	{
		return (sig0 & sig1) == 0;
	}
}

unsigned SignalSpread::addSignal(unsigned l, unsigned r)
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

void SignalSpread::spreadAdd(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	if (sig0 == 0) return;
	auto sig1 = signalOf2Spread(inst->get_operand(1));
	if (sig1 == 0) return;
	spreadUseIfSetSig(inst, addSignal(sig0, sig1));
}

void SignalSpread::spreadSub(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	if (sig0 == 0) return;
	auto sig1 = signalOf2Spread(inst->get_operand(1));
	if (sig1 == 0) return;
	sig1 = flipSignal(sig1);
	spreadUseIfSetSig(inst, addSignal(sig0, sig1));
}

unsigned SignalSpread::flipSignal(unsigned i)
{
	static constexpr unsigned rets[8] = {0u, 4u, 2u, 6u, 1u, 5u, 3u, 7u};
	return rets[i];
}


unsigned SignalSpread::signalOf2Spread(Value* val)
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
	return 0u;
}

void SignalSpread::spreadMul(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	auto sig1 = signalOf2Spread(inst->get_operand(1));
	if (sig0 == 0 || sig1 == 0)
	{
		if (haveZero(sig0) || haveZero(sig1)) spreadUseIfSetSig(inst, zeroSig());
		return;
	}
	spreadUseIfSetSig(inst, multiplySignal(sig0, sig1));
}

unsigned SignalSpread::multiplySignal(unsigned l, unsigned r)
{
	unsigned ret = (l & 4u) ? r : 0u;
	if (r) ret |= 2u;
	if (l & 1u) ret |= SignalSpread::flipSignal(r);
	return ret;
}

void SignalSpread::spreadSdiv(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	auto sig1 = signalOf2Spread(inst->get_operand(1));
	sig1 = removeZero(sig1);
	if (sig1 == 0)
	{
		if (haveZero(sig0)) spreadUseIfSetSig(inst, zeroSig());
		return;
	}
	spreadUseIfSetSig(inst, sdivideSignal(sig0, sig1));
}

unsigned SignalSpread::sdivideSignal(unsigned l, unsigned r)
{
	if ((r & 5u) == 0) return 0;
	unsigned ret = 0u;
	unsigned lra = l & r;
	if (lra & 5u) ret |= 6u;
	unsigned lrfa = l & SignalSpread::flipSignal(r);
	if (lrfa & 5u) ret |= 3u;
	ret |= (l & 2u);
	return ret;
}


void SignalSpread::spreadSrem(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	spreadUseIfSetSig(inst, sig0 | zeroSig());
}

void SignalSpread::spreadShl(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	if (sig0 == 0) return;
	spreadUseIfSetSig(inst, sig0);
}

void SignalSpread::spreadAshr(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	if (sig0 == 0) return;
	unsigned ll = sig0;
	if (sig0 & 4u) ll |= 2u;
	spreadUseIfSetSig(inst, ll);
}

void SignalSpread::spreadAnd(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	if (sig0 == 0) return;
	auto sig1 = signalOf2Spread(inst->get_operand(1));
	if (sig1 == 0) return;
	spreadUseIfSetSig(inst, andSignal(sig0, sig1));
}

unsigned SignalSpread::andSignal(unsigned l, unsigned r)
{
	unsigned ret = l & r;
	unsigned orr = l | r;
	if (orr & 4u) ret |= 2u;
	if (l & SignalSpread::flipSignal(r) & 5u) ret |= 4u;
	return ret;
}

void SignalSpread::spreadFDiv(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	auto sig1 = signalOf2Spread(inst->get_operand(1));
	sig1 = removeZero(sig1);
	if (sig0 == 0 || sig1 == 0)
	{
		if (haveZero(sig0)) spreadUseIfSetSig(inst, zeroSig());
		return;
	}
	spreadUseIfSetSig(inst, multiplySignal(sig0, sig1));
}

void SignalSpread::spreadLoad(Instruction* inst)
{
	spreadUseIfSetSig(inst, allSig());
}

void SignalSpread::spreadPhi(Instruction* inst)
{
	unsigned sig = 0;
	int size = u2iNegThrow(inst->get_operands().size());
	for (int i = 0; i < size; i += 2)
	{
		auto op = inst->get_operand(i);
		sig |= signalOf2Spread(op);
	}
	if (sig != 0) spreadUseIfSetSig(inst, sig);
}

void SignalSpread::spreadFP2SI(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	if (sig0 == 0) return;
	sig0 |= 2u;
	spreadUseIfSetSig(inst, sig0);
}

void SignalSpread::spreadSI2FP(Instruction* inst)
{
	auto sig0 = signalOf2Spread(inst->get_operand(0));
	if (sig0 == 0) return;
	spreadUseIfSetSig(inst, sig0);
}

void SignalSpread::spreadCall(const Instruction* inst)
{
	auto callF = dynamic_cast<Function*>(inst->get_operand(0));
	if (callF->is_lib_)
	{
		if (callF->get_type() != Types::VOID)
		{
			for (auto use : inst->get_parent()->get_parent()->get_use_list())
			{
				auto call = dynamic_cast<CallInst*>(use.val_);
				auto oc = signalOf2Spread(call);
				if (7u != oc)
				{
					designateSignal(call, 7u);
					for (auto use2 : call->get_use_list())
						addToSignalWorkList(use2.val_);
				}
			}
		}
		return;
	}
	int size = u2iNegThrow(inst->get_operands().size());
	for (int i = 1; i < size; i++)
	{
		auto op = inst->get_operand(i);
		if (!op->get_type()->isBasicType()) continue;
		auto sig = signalOf2Spread(op);
		if (sig != 0) spreadUseIfSetSig(callF->get_arg(i - 1), sig);
	}
}

void SignalSpread::decideConds()
{
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_) continue;
		for (auto bb : f->get_basic_blocks())
		{
			for (auto inst : bb->get_instructions())
			{
				if (inst->is_cmp())
				{
					auto icmp = dynamic_cast<ICmpInst*>(inst);
					if (icmp->get_parent()->get_parent()->get_name() == "revert")
						int iii = 0;
					auto sig0 = signalOf2Spread(icmp->get_operand(0));
					if (sig0 == 0u) sig0 = 7u;
					auto sig1 = signalOf2Spread(icmp->get_operand(1));
					if (sig1 == 0u) sig1 = 7u;
					switch (icmp->get_instr_type()) // NOLINT(clang-diagnostic-switch-enum)
					{
						case Instruction::gt:
						case Instruction::le:
							if (greaterThan(sig0, sig1))
							{
								icmp->set_operand(0, Constant::create(m_, 1));
								icmp->set_operand(1, Constant::create(m_, 0));
							}
							else if (greaterOrEqual(sig1, sig0)) // NOLINT(readability-suspicious-call-argument)
							{
								icmp->set_operand(1, Constant::create(m_, 1));
								icmp->set_operand(0, Constant::create(m_, 0));
							}
							break;
						case Instruction::lt:
						case Instruction::ge:
							if (greaterOrEqual(sig0, sig1))
							{
								icmp->set_operand(0, Constant::create(m_, 1));
								icmp->set_operand(1, Constant::create(m_, 0));
							}
							else if (greaterThan(sig1, sig0)) // NOLINT(readability-suspicious-call-argument)
							{
								icmp->set_operand(1, Constant::create(m_, 1));
								icmp->set_operand(0, Constant::create(m_, 0));
							}
							break;
						case Instruction::eq:
						case Instruction::ne:
							if (equal(sig0, sig1))
							{
								icmp->set_operand(0, Constant::create(m_, 0));
								icmp->set_operand(1, Constant::create(m_, 0));
							}
							else if (notEqual(sig0, sig1))
							{
								icmp->set_operand(0, Constant::create(m_, 0));
								icmp->set_operand(1, Constant::create(m_, 1));
							}
							break;
						default: break;
					}
				}
			}
		}
	}
}

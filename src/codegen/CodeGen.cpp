#include "CodeGen.hpp"

#include <cassert>

#include "Ast.hpp"
#include "Config.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineInstruction.hpp"
#include "MachineFunction.hpp"
#include "MachineOperand.hpp"
#include "CodeString.hpp"

#define DEBUG 0
#include <iomanip>

#include "CountLZ.hpp"
#include "Util.hpp"

using namespace std;

namespace
{
	string condName(Instruction::OpID cond)
	{
		switch (cond) // NOLINT(clang-diagnostic-switch-enum)
		{
			case Instruction::ge: return "GE";
			case Instruction::gt: return "GT";
			case Instruction::le: return "LE";
			case Instruction::lt: return "LT";
			case Instruction::eq: return "EQ";
			case Instruction::ne: return "NE";
			default: break;
		}
		throw runtime_error("unexpected");
	}

	long long upAlignTo16(long long of)
	{
		ASSERT(of >= 0);
		return ((of + 15) >> 4) << 4;
	}

	bool inUImm12(int i)
	{
		return i >= 0 && i < 4096;
	}

	bool inUImm12(long long i)
	{
		return i >= 0 && i < 4096;
	}

	bool inUImm24(int i)
	{
		return i >= 0 && (i >> 12) < 4096;
	}

	bool inUImm24(long long i)
	{
		return i >= 0 && (i >> 12) < 4096;
	}

	bool inUImm12L12(int i)
	{
		if (i < 0) return false;
		if (i & 4095) return false;
		i >>= 12;
		return i < 4096;
	}

	bool inUImm12L12(long long i)
	{
		if (i < 0) return false;
		if (i & 4095) return false;
		i >>= 12;
		return i < 4096;
	}

	bool inImm7L2(int i)
	{
		if (i < -256) return false;
		if (i > 252) return false;
		return !(i & 0b11);
	}

	bool inImm7L3(int i)
	{
		if (i < -512) return false;
		if (i > 504) return false;
		return !(i & 0b111);
	}

	bool inImm7L3(long long i)
	{
		if (i < -512) return false;
		if (i > 504) return false;
		return !(i & 0b111);
	}

	bool inImm9(long long i)
	{
		if (i < -256) return false;
		return i < 256;
	}

	bool align4(long long i)
	{
		return !(i & 0b11);
	}

	bool align8(long long i)
	{
		return !(i & 0b111);
	}

	bool floatMovSupport(unsigned u) noexcept
	{
		const unsigned exp = u & (0xFFu << 23u);
		const unsigned frac = u & 0x7FFFFFu;
		if (exp < 124u || exp > 131u)
			return false;
		if (frac & 0x780000u)
			return false;
		return true;
	}

	Instruction::OpID lrShiftOp(Instruction::OpID op)
	{
		// ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
		// ReSharper disable once CppIncompleteSwitchStatement
		switch (op) // NOLINT(clang-diagnostic-switch)
		{
			case Instruction::ge: return Instruction::le;
			case Instruction::gt: return Instruction::lt;
			case Instruction::le: return Instruction::ge;
			case Instruction::lt: return Instruction::gt;
			case Instruction::eq: return Instruction::eq;
			case Instruction::ne: return Instruction::ne;
		}
		throw runtime_error("invalid op");
	}
}


std::string CodeGen::word(const std::vector<ConstantValue>* v)
{
	string ret = "\t.word ";
	for (auto& i : *v)
	{
		ret += to_string(i.bits2Unsigned()) + ", ";
	}
	ret.pop_back();
	ret.pop_back();
	return ret;
}

std::string CodeGen::zero(int count)
{
	return "\t.zero " + to_string(count);
}

std::string CodeGen::size(const std::string& globName, int bytes)
{
	return "\t.size " + globName + ", " + to_string(bytes);
}

std::string CodeGen::size(const std::string& globName, long long bytes)
{
	return "\t.size " + globName + ", " + to_string(bytes);
}

void CodeGen::makeGlobal(const GlobalAddress* address) const
{
	auto allSize = logicalRightShift(address->size_, 3);
	m_->modulePrefix_->addAlign(allSize, true);
	m_->modulePrefix_->addGlobal(address->name_);
	m_->modulePrefix_->objectTypeDeclare(address->name_);
	m_->modulePrefix_->addLabel(address->name_);
	auto init = address->data_;
	for (int i = 0, segCount = init->segmentCount(); i < segCount; i++)
	{
		auto [iv, dc] = init->segment(i);
		if (dc == 0)
			m_->modulePrefix_->addCommonStr(word(iv));
		else m_->modulePrefix_->addCommonStr(zero(dc << 2));
	}
	m_->modulePrefix_->addCommonStr(size(address->name_, allSize));
}

void CodeGen::makeFunction()
{
	for (auto i : func_->blocks_) i->blockPrefix_ = new CodeString{};
	func_->funcPrefix_ = new CodeString{};
	func_->funcSuffix_ = new CodeString{};
	func_->funcPrefix_->addGlobal(func_->name_);
	func_->funcPrefix_->functionTypeDeclare(func_->name_);
	func_->funcPrefix_->addLabel(func_->name_);
	functionPrefix();
	functionSuffix();
	for (auto bb : func_->blocks())
	{
		LOG(color::yellow(bb->name()));
		PUSH;
		for (auto inst : bb->instructions())
			makeInstruction(inst);
		POP;
	}
	func_->sizeSuffix_ = functionSize(func_->name());
}

void CodeGen::functionPrefix()
{
	int lc = 0;
	int rc = 0;
	auto& calleeSaved = func_->callee_saved();
	for (auto [i,j] : calleeSaved)
	{
		if (i->isIntegerRegister()) lc++;
		else rc++;
	}
	auto toStr = func_->blocks_[0]->blockPrefix_;
	if (canLSInOneSPMove(calleeSaved))
	{
		sub64(sp(), sp(), func_->stack_move_offset(), toStr);
		if (lc & 1) str(calleeSaved[0].first, sp(), calleeSaved[0].second, 64, toStr);
		if (lc >= 2)
		{
			for (int i = lc & 1; i < lc; i += 2)
			{
				stp(calleeSaved[i].first, calleeSaved[i + 1].first, sp(), u2iNegThrow(calleeSaved[i].second),
				    64, toStr);
			}
		}
		if (rc & 1) str(calleeSaved[lc].first, sp(), calleeSaved[lc].second, 64, toStr);
		if (rc >= 2)
		{
			for (int i = rc & 1; i < rc; i += 2)
			{
				stp(calleeSaved[lc + i].first, calleeSaved[lc + i + 1].first, sp(),
				    u2iNegThrow(calleeSaved[lc + i].second),
				    64, toStr);
			}
		}
		return;
	}
	long long minOffset = upAlignTo16(func_->stack_move_offset() - calleeSaved[0].second);
	long long nextOffset = func_->stack_move_offset() - minOffset;
	sub64(sp(), sp(), minOffset, toStr);
	if (lc & 1) str(calleeSaved[0].first, sp(), calleeSaved[0].second - nextOffset, 64, toStr);
	if (lc >= 2)
	{
		for (int i = lc & 1; i < lc; i += 2)
		{
			stp(calleeSaved[i].first, calleeSaved[i + 1].first, sp(),
			    u2iNegThrow(calleeSaved[i].second - nextOffset), 64, toStr);
		}
	}
	if (rc & 1) str(calleeSaved[lc].first, sp(), calleeSaved[lc].second - nextOffset, 64, toStr);
	if (rc >= 2)
	{
		for (int i = rc & 1; i < rc; i += 2)
		{
			stp(calleeSaved[lc + i].first, calleeSaved[lc + i + 1].first, sp(),
			    u2iNegThrow(calleeSaved[lc + i].second - nextOffset), 64, toStr);
		}
	}
	sub64(sp(), sp(), nextOffset, toStr);
}

void CodeGen::functionSuffix()
{
	int lc = 0;
	int rc = 0;
	auto& calleeSaved = func_->callee_saved();
	for (auto [i,j] : calleeSaved)
	{
		if (i->isIntegerRegister()) lc++;
		else rc++;
	}
	auto toStr = func_->funcSuffix_;
	if (canLSInOneSPMove(calleeSaved))
	{
		if (rc >= 2)
		{
			for (int i = rc - 2; i >= 0; i -= 2)
			{
				ldp(calleeSaved[lc + i].first, calleeSaved[lc + i + 1].first, sp(),
				    u2iNegThrow(calleeSaved[lc + i].second),
				    64, toStr);
			}
		}
		if (rc & 1) ldr(calleeSaved[lc].first, sp(), calleeSaved[lc].second, 64, toStr);
		if (lc >= 2)
		{
			for (int i = lc - 2; i >= 0; i -= 2)
			{
				ldp(calleeSaved[i].first, calleeSaved[i + 1].first, sp(), u2iNegThrow(calleeSaved[i].second),
				    64, toStr);
			}
		}
		if (lc & 1)ldr(calleeSaved[0].first, sp(), calleeSaved[0].second, 64, toStr);
		add64(sp(), sp(), func_->stack_move_offset(), toStr);
		return;
	}
	long long minOffset = upAlignTo16(func_->stack_move_offset() - calleeSaved[0].second);
	long long nextOffset = func_->stack_move_offset() - minOffset;
	add64(sp(), sp(), nextOffset, toStr);
	if (rc >= 2)
	{
		for (int i = rc - 2; i >= 0; i -= 2)
		{
			ldp(calleeSaved[lc + i].first, calleeSaved[lc + i + 1].first, sp(),
			    u2iNegThrow(calleeSaved[lc + i].second - nextOffset), 64, toStr);
		}
	}
	if (rc & 1) ldr(calleeSaved[lc].first, sp(), calleeSaved[lc].second - nextOffset, 64, toStr);
	if (lc >= 2)
	{
		for (int i = lc - 2; i >= 0; i -= 2)
		{
			ldp(calleeSaved[i].first, calleeSaved[i + 1].first, sp(),
			    u2iNegThrow(calleeSaved[i].second - nextOffset), 64, toStr);
		}
	}
	if (lc & 1) ldr(calleeSaved[0].first, sp(), calleeSaved[0].second - nextOffset, 64, toStr);
	add64(sp(), sp(), minOffset, toStr);
}

void CodeGen::add(const Register* to, const Register* l, const Register* r, int len, CodeString* toStr)
{
	toStr->addInstruction("ADD", regName(to, len), regName(l, len), regName(r, len));
}

void CodeGen::fadd(const Register* to, const Register* l, const Register* r, CodeString* toStr)
{
	return toStr->addInstruction("FADD", regName(to, 32), regName(l, 32), regName(r, 32));
}

void CodeGen::fmul(const Register* to, const Register* l, const Register* r, CodeString* toStr)
{
	return toStr->addInstruction("FMUL", regName(to, 32), regName(l, 32), regName(r, 32));
}

void CodeGen::fdiv(const Register* to, const Register* l, const Register* r, CodeString* toStr)
{
	return toStr->addInstruction("FDIV", regName(to, 32), regName(l, 32), regName(r, 32));
}

void CodeGen::stp(const Register* a, const Register* b, const Register* c, int offset, int len, CodeString* toStr)
{
	ASSERT(len == 32 || len == 64);
	ASSERT(len == 32 ? inImm7L2(offset) : inImm7L3(offset));
	return toStr->addInstruction("STP", regName(a, len), regName(b, len), regDataOffset(c, offset));
}

void CodeGen::str(const Register* a, const Register* c, long long offset, int len, CodeString* toStr)
{
	ASSERT(len == 32 || len == 64 || len == 128);
	ASSERT(len == 32 ? align4(offset) : align8(offset));
	if (inImm9(offset)) return toStr->addInstruction("STR", regName(a, len), regDataOffset(c, u2iNegThrow(offset)));
	long long absOff = offset < 0 ? -offset : offset;
	int maxMov = m_countr_zero(ll2ullKeepBits(absOff));
	int lenLevel = m_countr_zero(i2uKeepBits(len)) - 3;
	if (maxMov > lenLevel) maxMov = lenLevel;
	offset = offset < 0 ? -(absOff >> maxMov) : (absOff >> maxMov);
	auto reg = getIP();
	makeI64Immediate(offset, reg, toStr);
	toStr->addInstruction("STR", regName(a, len), regDataRegLSLOffset(c, reg, maxMov));
	releaseIP(reg);
}

void CodeGen::str(const MOperand* regLike, const MOperand* stackLike, int len, CodeString* toStr, bool forFuncCall)
{
	if (const Register* i = dynamic_cast<const Register*>(stackLike); i != nullptr)
	{
		const Register* l = op2reg(regLike, len, toStr);
		str(l, i, 0ll, len, toStr);
		releaseIP(l);
		return;
	}
	ASSERT(dynamic_cast<const Immediate*>(stackLike) == nullptr);
	if (const GlobalAddress* i = dynamic_cast<const GlobalAddress*>(stackLike); i != nullptr)
	{
		const Register* l = op2reg(regLike, len, toStr);
		auto r = op2reg(i, 64, toStr);
		str(l, r, 0ll, len, toStr);
		releaseIP(l);
		releaseIP(r);
		return;
	}
	if (const FrameIndex* i = dynamic_cast<const FrameIndex*>(stackLike); i != nullptr)
	{
		auto offset = frameOffset(i, forFuncCall, toStr);
		const Register* l = op2reg(regLike, len, toStr);
		str(l, sp(), offset, len, toStr);
		releaseIP(l);
		return;
	}
	throw runtime_error("unexpected");
}

void CodeGen::simdcp(const MOperand* regLike, const MOperand* fregLike, int len, int lane, bool ld, CodeString* toStr)
{
	auto ll = dynamic_cast<const Register*>(regLike);
	auto rr = dynamic_cast<const Register*>(fregLike);
	ASSERT(ll && rr);
	if (len == 64)
	{
		if (ld) copy(regLike, fregLike, 64, toStr);
		else copy(fregLike, regLike, 64, toStr);
	}
	else
	{
		if (ld)
		{
			if (ll->isIntegerRegister())
				toStr->addInstruction("UMOV", regName(ll, 32), simd32RegName(rr, lane));
			else
				toStr->addInstruction("MOV", regName(ll, 32), simd32RegName(rr, lane));
		}
		else
		{
			if (ll->isIntegerRegister())
				toStr->addInstruction("INS", simd32RegName(rr, lane), regName(ll, 32));
			else
				toStr->addInstruction("INS", simd32RegName(rr, lane), simd32RegName(ll, 0));
		}
	}
}

const Register* CodeGen::op2reg(const MOperand* op, int len, bool useIntReg, CodeString* toStr)
{
	if (const Register* fromr = dynamic_cast<const Register*>(op); fromr != nullptr)
	{
		if (len == 128)
		{
			ASSERT(!fromr->isIntegerRegister());
			return fromr;
		}
		if (fromr->isIntegerRegister() != useIntReg)
		{
			auto reg = useIntReg ? getIP() : getFIP();
			copy(reg, fromr, len, toStr);
			return reg;
		}
		return fromr;
	}
	ASSERT(len == 32 || len == 64);
	auto r = useIntReg ? getIP() : getFIP();
	copy(r, op, len, toStr);
	return r;
}

const Register* CodeGen::op2reg(const MOperand* op, int len, CodeString* appendSlot)
{
	if (const Register* fromr = dynamic_cast<const Register*>(op); fromr != nullptr)
	{
		if (len == 128)
		{
			ASSERT(!fromr->isIntegerRegister());
			return fromr;
		}
		return fromr;
	}
	ASSERT(len == 32 || len == 64);
	auto r = getIP();
	copy(r, op, len, appendSlot);
	return r;
}

void CodeGen::ldp(const Register* a, const Register* b, const Register* c, int offset, int len, CodeString* toStr)
{
	ASSERT(len == 32 || len == 64);
	ASSERT(len == 32 ? inImm7L2(offset) : inImm7L3(offset));
	return toStr->addInstruction("LDP", regName(a, len), regName(b, len), regDataOffset(c, offset));
}

void CodeGen::ldr(const Register* a, const Register* baseOffsetReg, long long offset, int len, CodeString* toStr)
{
	ASSERT(len == 32 || len == 64 || len == 128);
	ASSERT(len == 32 ? align4(offset) : align8(offset));
	if (inImm9(offset))
		return toStr->addInstruction("LDR", regName(a, len),
		                             regDataOffset(baseOffsetReg, u2iNegThrow(offset)));
	long long absOff = offset < 0 ? -offset : offset;
	int maxMov = m_countr_zero(ll2ullKeepBits(absOff));
	int lenLevel = m_countr_zero(i2uKeepBits(len)) - 3;
	if (maxMov > lenLevel) maxMov = lenLevel;
	offset = offset < 0 ? -(absOff >> maxMov) : (absOff >> maxMov);
	auto reg = getIP();
	makeI64Immediate(offset, reg, toStr);
	toStr->addInstruction("LDR", regName(a, len), regDataRegLSLOffset(baseOffsetReg, reg, maxMov)); // NOLINT(readability-suspicious-call-argument)
	releaseIP(reg);
}

int CodeGen::ldrNeedInstCount(long long offset, int len)
{
	if (inImm9(offset)) return 4;
	long long absOff = offset < 0 ? -offset : offset;
	int maxMov = m_countr_zero(ll2ullKeepBits(absOff));
	int lenLevel = m_countr_zero(i2uKeepBits(len)) - 3;
	if (maxMov > lenLevel) maxMov = lenLevel;
	offset = offset < 0 ? -(absOff >> maxMov) : (absOff >> maxMov);
	return 4 + makeI64ImmediateNeedInstCount(offset);
}

int CodeGen::copyFrameNeedInstCount(long long offset)
{
	if (offset == 0) return 0;
	if (inUImm12(offset) || inUImm12L12(offset)) return 1;
	return 1 + makeI64ImmediateNeedInstCount(offset);
}

void CodeGen::ldr(const MOperand* a, const MOperand* stackLike, int len, CodeString* toStr)
{
	list<string> ret;
	auto l = dynamic_cast<const Register*>(a);
	ASSERT(l != nullptr);
	if (const Register* i = dynamic_cast<const Register*>(stackLike); i != nullptr)
	{
		ldr(l, i, 0, len, toStr);
		return;
	}
	ASSERT(dynamic_cast<const Immediate*>(stackLike) == nullptr);
	if (const GlobalAddress* i = dynamic_cast<const GlobalAddress*>(stackLike); i != nullptr)
	{
		auto r = op2reg(i, 64, toStr);
		ldr(l, r, 0, len, toStr);
		releaseIP(r);
		return;
	}
	if (const FrameIndex* i = dynamic_cast<const FrameIndex*>(stackLike); i != nullptr)
	{
		auto offset = frameOffset(i, false, toStr);
		ldr(l, sp(), offset, len, toStr);
		return;
	}
	throw runtime_error("unexpected");
}

void CodeGen::ld1(const MOperand* stackLike, int count, int offset, CodeString* toStr)
{
	if (const Register* i = dynamic_cast<const Register*>(stackLike); i != nullptr)
	{
		return ld1(i, count, offset, toStr);
	}
	throw runtime_error("unexpected");
}

void CodeGen::clearV(int count, CodeString* toStr)
{
	for (int i = 0; i < count; i++)
	{
		auto reg = "V" + to_string(i) + ".16B";
		toStr->addInstruction("EOR", reg, reg, reg);
	}
}

void CodeGen::mathInst(const MOperand* t, const MOperand* l, const MOperand* r,
                       Instruction::OpID op,
                       int len, CodeString* toStr)
{
	auto target = dynamic_cast<const Register*>(t);
	ASSERT(op >= Instruction::add && op <= Instruction::fdiv);
	ASSERT(op <= Instruction::srem || len != 64);
	ASSERT(target != nullptr);
	bool flt = op >= Instruction::fadd;
	auto immL = dynamic_cast<const Immediate*>(l);
	auto immR = dynamic_cast<const Immediate*>(r);
	if (immR && immR->isZero(flt, len))
	{
		if (op == Instruction::sdiv || op == Instruction::srem || op == Instruction::fdiv)
		{
			copy(t, zeroRegister(), len, toStr);
			return;
		}
	}
	if (immL && immR)
	{
		ASSERT(op <= Instruction::and_ || len != 64);
		if (op <= Instruction::and_)
		{
			if (len == 32)
			{
				int i;
				switch (op) // NOLINT(clang-diagnostic-switch-enum)
				{
					case Instruction::add:
						i = immL->asInt() + immR->asInt();
						break;
					case Instruction::sub:
						i = immL->asInt() - immR->asInt();
						break;
					case Instruction::mul:
						i = immL->asInt() * immR->asInt();
						break;
					case Instruction::sdiv:
						i = immL->asInt() / immR->asInt();
						break;
					case Instruction::srem:
						i = immL->asInt() % immR->asInt();
						break;
					case Instruction::shl:
						i = immL->asInt() << immR->asInt();
						break;
					case Instruction::ashr:
						i = immL->asInt() >> immR->asInt();
						break;
					case Instruction::and_:
						i = immL->asInt() & immR->asInt();
						break;
					default:
						throw runtime_error("unexpected");
				}
				makeI32Immediate(i, target, toStr);
				return;
			}
			long long i;
			switch (op) // NOLINT(clang-diagnostic-switch-enum)
			{
				case Instruction::add:
					i = immL->as64BitsInt() + immR->as64BitsInt();
					break;
				case Instruction::sub:
					i = immL->as64BitsInt() - immR->as64BitsInt();
					break;
				case Instruction::mul:
					i = immL->as64BitsInt() * immR->as64BitsInt();
					break;
				case Instruction::sdiv:
					i = immL->as64BitsInt() / immR->as64BitsInt();
					break;
				case Instruction::srem:
					i = immL->as64BitsInt() % immR->as64BitsInt();
					break;
				case Instruction::shl:
					i = immL->as64BitsInt() << immR->as64BitsInt();
					break;
				case Instruction::ashr:
					i = immL->as64BitsInt() >> immR->as64BitsInt();
					break;
				case Instruction::and_:
					i = immL->as64BitsInt() & immR->as64BitsInt();
					break;
				default:
					throw runtime_error("unexpected");
			}
			makeI64Immediate(i, target, toStr);
			return;
		}
		float f;
		switch (op) // NOLINT(clang-diagnostic-switch-enum)
		{
			case Instruction::fadd:
				f = immL->asFloat() + immR->asFloat();
				break;
			case Instruction::fsub:
				f = immL->asFloat() - immR->asFloat();
				break;
			case Instruction::fmul:
				f = immL->asFloat() * immR->asFloat();
				break;
			case Instruction::fdiv:
				f = immL->asFloat() / immR->asFloat();
				break;
			default:
				throw runtime_error("unexpected");
		}
		makeF32Immediate(f, target, toStr);
		return;
	}
	if (immL && !immR)
	{
		if (op == Instruction::add || op == Instruction::mul || op == Instruction::fadd || op == Instruction::fmul)
		{
			immR = immL;
			l = r;
			r = immL;
			immL = nullptr;
		}
	}
	if (immL)
	{
		if (immL->isZero(flt, len))
		{
			if (op == Instruction::add || op == Instruction::fadd)
			{
				copy(target, r, len, toStr);
				return;
			}
			if (op == Instruction::mul || op == Instruction::sdiv || op == Instruction::srem || op == Instruction::fmul
			    ||
			    op == Instruction::fdiv || op == Instruction::shl || op == Instruction::and_ || op == Instruction::ashr)
			{
				copy(target, zeroRegister(), len, toStr);
				return;
			}
		}
		if (immL->isNotFloatOne(flt, len))
		{
			if (op == Instruction::mul)
			{
				copy(target, r, len, toStr);
				return;
			}
		}
	}
	if (immR)
	{
		if (immR->isZero(flt, len))
		{
			if (op == Instruction::add || op == Instruction::fadd || op == Instruction::shl || op == Instruction::ashr)
			{
				copy(target, l, len, toStr);
				return;
			}
			if (op == Instruction::mul || op == Instruction::fmul || op == Instruction::and_)
			{
				copy(target, zeroRegister(), len, toStr);
				return;
			}
		}
		if (immR->isNotFloatOne(flt, len))
		{
			if (op == Instruction::mul || op == Instruction::sdiv || op == Instruction::srem)
			{
				return copy(target, l, len, toStr);
			}
		}
	}
	auto regL = dynamic_cast<const Register*>(l);
	auto regR = dynamic_cast<const Register*>(r);
	if (flt)
	{
		if (immL)
		{
			regL = getFIP();
			makeF32Immediate(immL->asFloat(), regL, toStr);
			immL = nullptr;
		}
		else if (immR)
		{
			regR = getFIP();
			makeF32Immediate(immR->asFloat(), regR, toStr);
			immR = nullptr;
		}
	}
	bool orl = regL || immL;
	bool orr = regR || immR;
	if ((!(orl || orr)) || (!(orl || orr) && op != Instruction::add && op != Instruction::sub && len != 64))
	{
		releaseIP(regL);
		releaseIP(regR);
		return;
	}
	if (flt)
	{
		regL = op2reg(regL, len, false, toStr);
		regR = op2reg(regR, len, false, toStr);
		switch (op) // NOLINT(clang-diagnostic-switch-enum)
		{
			case Instruction::fadd:
				fadd(target, regL, regR, toStr);
				break;
			case Instruction::fsub:
				fsub(target, regL, regR, toStr);
				break;
			case Instruction::fmul:
				fmul(target, regL, regR, toStr);
				break;
			case Instruction::fdiv:
				fdiv(target, regL, regR, toStr);
				break;
			default:
				throw runtime_error("unexpected");
		}
		releaseIP(regL);
		releaseIP(regR);
		return;
	}
	auto fl = dynamic_cast<const FrameIndex*>(l);
	auto fr = dynamic_cast<const FrameIndex*>(r);
	if (fl && immR)
	{
		long long offset = frameOffset(fl, false, toStr);
		immR = Immediate::getImmediate(immR->as64BitsInt() + offset, m_);
		r = immR;
		regL = sp();
		l = sp();
		fl = nullptr;
	}
	if (fr && immL)
	{
		long long offset = frameOffset(fr, false, toStr);
		immL = Immediate::getImmediate(immL->as64BitsInt() + offset, m_);
		l = immL;
		r = sp();
		fr = nullptr;
	}
	if (!regL)
	{
		regL = op2reg(l, len, true, toStr);
		fl = nullptr;
	}
	if (op == Instruction::shl)
	{
		ASSERT(immR);
		if (len == 32) lsl32(target, regL, immR->asInt(), toStr);
		else lsl64(target, regL, immR->as64BitsInt(), toStr);
		releaseIP(regL);
		return;
	}
	if (op == Instruction::ashr)
	{
		ASSERT(immR);
		if (len == 32) asr32(target, regL, immR->asInt(), toStr);
		else asr64(target, regL, immR->as64BitsInt(), toStr);
		releaseIP(regL);
		return;
	}
	if (op == Instruction::and_)
	{
		ASSERT(immR);
		if (len == 32) and32(target, regL, immR->asInt(), toStr);
		else and64(target, regL, immR->as64BitsInt(), toStr);
		releaseIP(regL);
		return;
	}
	if (immR && op == Instruction::srem)
	{
		if (len == 32)
		{
			int val = immR->asInt();
			int av = val < 0 ? -val : val;
			if ((av & (av - 1)) == 0)
			{
				auto rr = getIP();
				auto ip = getIP();
				makeImmediate(immR, false, 32, rr, toStr);
				toStr->addInstruction("SDIV", regName(ip, 32), regName(regL, 32),
				                      regName(rr, 32));
				lsl32(ip, ip, m_countr_zero(av), toStr);
				if (val < 0)
					mathRRInst(target, regL, ip, Instruction::add, 32, toStr);
				else
					mathRRInst(target, regL, ip, Instruction::sub, 32, toStr);
				releaseIP(rr);
				releaseIP(ip);
				return;
			}
		}
		else
		{
			long long val = immR->asInt();
			long long av = val < 0 ? -val : val;
			if ((av & (av - 1)) == 0)
			{
				auto rr = getIP();
				auto ip = getIP();
				makeImmediate(immR, false, 64, rr, toStr);
				toStr->addInstruction("SDIV", regName(ip, 64), regName(regL, 64),
				                      regName(rr, 64));
				lsl64(ip, ip, m_countr_zero(av), toStr);
				if (val < 0)
					mathRRInst(target, regL, ip, Instruction::add, 64, toStr);
				else
					mathRRInst(target, regL, ip, Instruction::sub, 64, toStr);
				releaseIP(rr);
				releaseIP(ip);
				return;
			}
		}
	}
	if (immR && (op == Instruction::add || op == Instruction::sub))
	{
		if (op == Instruction::add)
		{
			if (len == 32)
				add32(target, regL, immR->asInt(), toStr);
			else add64(target, regL, immR->as64BitsInt(), toStr);
			releaseIP(regL);
			return;
		}
		if (len == 32)
			sub32(target, regL, immR->asInt(), toStr);
		else sub64(target, regL, immR->as64BitsInt(), toStr);
		releaseIP(regL);
		return;
	}
	regR = op2reg(r, len, true, toStr);
	fr = nullptr;
	mathRRInst(target, regL, regR, op, len, toStr);
	releaseIP(regL);
	releaseIP(regR);
}

bool CodeGen::mathInstImmediateCanInline(const MOperand* l, const MOperand* r,
                                         Instruction::OpID op, int len)
{
	ASSERT(op >= Instruction::add && op <= Instruction::fdiv);
	ASSERT(op <= Instruction::srem || len != 64);
	bool flt = op >= Instruction::fadd;
	auto immL = dynamic_cast<const Immediate*>(l);
	auto immR = dynamic_cast<const Immediate*>(r);
	if (immR && immR->isZero(flt, len)) return true;
	if (immL && immR) return true;
	if (immL && !immR)
	{
		if (op == Instruction::add || op == Instruction::mul || op == Instruction::fadd || op == Instruction::fmul)
		{
			immR = immL;
			l = r;
			r = immL;
			immL = nullptr;
		}
	}
	if (immL)
	{
		if (immL->isZero(flt, len))
		{
			if (op == Instruction::add || op == Instruction::fadd) return true;
			if (op == Instruction::mul || op == Instruction::sdiv || op == Instruction::srem || op == Instruction::fmul
			    ||
			    op == Instruction::fdiv || op == Instruction::shl || op == Instruction::and_ || op == Instruction::ashr)
				return true;
		}
		if (immL->isNotFloatOne(flt, len))
		{
			if (op == Instruction::mul) return true;
		}
	}
	if (immR)
	{
		if (immR->isZero(flt, len))
		{
			if (op == Instruction::add || op == Instruction::fadd || op == Instruction::shl || op == Instruction::ashr)
				return true;
			if (op == Instruction::mul || op == Instruction::fmul || op == Instruction::and_)return true;
		}
		if (immR->isNotFloatOne(flt, len))
		{
			if (op == Instruction::mul || op == Instruction::sdiv || op == Instruction::srem)return true;
		}
	}
	auto regL = dynamic_cast<const Register*>(l);
	auto regR = dynamic_cast<const Register*>(r);
	if (flt)
	{
		if (immL || immR) return false;
		return true;
	}
	bool orl = regL || immL;
	bool orr = regR || immR;
	if ((!(orl || orr)) || (!(orl || orr) && op != Instruction::add && op != Instruction::sub && len != 64))
		return true;
	if (op == Instruction::shl) return true;
	if (op == Instruction::ashr) return true;
	if (op == Instruction::and_) return true;
	if (immR && op == Instruction::srem) return false;
	if (immR && (op == Instruction::add || op == Instruction::sub))
	{
		if (len == 32)
			return immCanInlineInAddSub(immR->asInt());
		return immCanInlineInAddSub(immR->as64BitsInt());
	}
	return false;
}

void CodeGen::ld1(const Register* stackLike, int count, int offset, CodeString* toStr)
{
	if (count == 1) return ldr(floatRegister(0), stackLike, 128, toStr);
	string ret = "LD1 {";
	for (int i = 0; i < count; i++)
		ret += "V" + to_string(i) + ".16B,";
	ret.pop_back();
	ret += "}, ";
	ret += regData(stackLike);
	if (offset != 0)
	{
		ret += ", " + immediate(offset);
	}
	toStr->addInstruction(ret);
}

void CodeGen::st1(const MOperand* stackLike, int count, int offset, CodeString* toStr)
{
	if (const Register* i = dynamic_cast<const Register*>(stackLike); i != nullptr)
	{
		return st1(i, count, offset, toStr);
	}
	throw runtime_error("unexpected");
}

bool CodeGen::immCanInlineInAddSub(int imm)
{
	if (imm < 0) imm = -imm;
	return imm == 0 || inUImm12(imm) || inUImm12L12(imm);
}

bool CodeGen::immCanInlineInAddSub(long long imm)
{
	if (imm < 0) imm = -imm;
	return imm == 0 || inUImm12(imm) || inUImm12L12(imm);
}

void CodeGen::add32(const Register* to, const Register* l, int imm, CodeString* toStr)
{
	if (imm < 0) return sub32(to, l, -imm, toStr);
	if (imm == 0) return copy(to, l, 32, toStr);
	if (inUImm12(imm))
		return toStr->addInstruction("ADD", regName(to, 32), regName(l, 32), immediate(imm));
	if (inUImm12L12(imm))
		return toStr->addInstruction("ADD", regName(to, 32), regName(l, 32), immediate(imm >> 12), leftShift12());
	if (inUImm24(imm))
	{
		add32(to, l, imm & 4095, toStr);
		add32(to, to, imm & 16773120, toStr);
		return;
	}
	auto reg = getIP();
	makeI32Immediate(imm, reg, toStr);
	add(to, l, reg, 32, toStr);
	releaseIP(reg);
}

void CodeGen::lsl32(const Register* to, const Register* l, int imm, CodeString* toStr)
{
	ASSERT(imm >= 0);
	if (imm == 0) return copy(to, l, 32, toStr);
	toStr->addInstruction("LSL", regName(to, 32), regName(l, 32), immediate(imm));
}

void CodeGen::lsl64(const Register* to, const Register* l, long long imm, CodeString* toStr)
{
	ASSERT(imm >= 0);
	if (imm == 0) return copy(to, l, 64, toStr);
	toStr->addInstruction("LSL", regName(to, 64), regName(l, 64), immediate(imm));
}

void CodeGen::asr32(const Register* to, const Register* l, int imm, CodeString* toStr)
{
	ASSERT(imm >= 0);
	if (imm == 0) return copy(to, l, 32, toStr);
	toStr->addInstruction("ASR", regName(to, 32), regName(l, 32), immediate(imm));
}

void CodeGen::asr64(const Register* to, const Register* l, long long imm, CodeString* toStr)
{
	ASSERT(imm >= 0);
	if (imm == 0) return copy(to, l, 64, toStr);
	toStr->addInstruction("ASR", regName(to, 64), regName(l, 64), immediate(imm));
}

void CodeGen::and32(const Register* to, const Register* l, int imm, CodeString* toStr)
{
	ASSERT(imm >= 0);
	if (imm == 0) return copy(to, l, 32, toStr);
	if (inUImm12(imm))
	{
		toStr->addInstruction("AND", regName(to, 32), regName(l, 32), immediate(imm));
		return;
	}
	if (inUImm12L12(imm))
	{
		toStr->addInstruction("AND", regName(to, 32), regName(l, 32), immediate(imm >> 12), leftShift12());
		return;
	}
	auto ip = getIP();
	makeI64Immediate(imm, ip, toStr);
	toStr->addInstruction("AND", regName(to, 32), regName(l, 32), regName(ip, 32));
	releaseIP(ip);
}

void CodeGen::and64(const Register* to, const Register* l, long long imm, CodeString* toStr)
{
	ASSERT(imm >= 0);
	if (imm == 0) return copy(to, l, 64, toStr);
	if (inUImm12(imm))
	{
		toStr->addInstruction("AND", regName(to, 64), regName(l, 64), immediate(imm));
		return;
	}
	if (inUImm12L12(imm))
	{
		toStr->addInstruction("AND", regName(to, 64), regName(l, 64), immediate(imm >> 12), leftShift12());
		return;
	}
	auto ip = getIP();
	makeI64Immediate(imm, ip, toStr);
	toStr->addInstruction("AND", regName(to, 64), regName(l, 64), regName(ip, 64));
	releaseIP(ip);
}

void CodeGen::add64(const Register* to, const Register* l, long long imm, CodeString* toStr)
{
	if (imm < 0)
	{
		sub64(to, l, -imm, toStr);
		return;
	}
	if (imm == 0) return copy(to, l, 64, toStr);
	if (inUImm12(imm))
	{
		toStr->addInstruction("ADD", regName(to, 64), regName(l, 64), immediate(imm));
		return;
	}
	if (inUImm12L12(imm))
	{
		toStr->addInstruction("ADD", regName(to, 64), regName(l, 64), immediate(imm >> 12), leftShift12());
		return;
	}
	if (inUImm24(imm))
	{
		add64(to, l, imm & 4095, toStr);
		add64(to, to, imm & 16773120, toStr);
		return;
	}
	auto reg = getIP();
	makeI64Immediate(imm, reg, toStr);
	add(to, l, reg, 64, toStr);
	releaseIP(reg);
}

void CodeGen::st1(const Register* stackLike, int count, int offset, CodeString* toStr)
{
	if (count == 1) return str(floatRegister(0), stackLike, 128, toStr, false);
	string ret = "ST1 {";
	for (int i = 0; i < count; i++)
		ret += "V" + to_string(i) + ".16B,";
	ret.pop_back();
	ret += "}, ";
	ret += regData(stackLike);
	if (offset != 0)
	{
		ret += ", " + immediate(offset);
	}
	toStr->addInstruction(ret);
}

// X = A + IMM
// ---
// X = A + IMM LSL
// ---
// T = MOV IMM
// X = A + T
// ---
// X = A + IMM0
// X = X + IMM1 LSL

void CodeGen::sub64(const Register* to, const Register* l, long long imm, CodeString* toStr)
{
	if (imm < 0)
	{
		add64(to, l, -imm, toStr);
		return;
	}
	if (imm == 0) return copy(to, l, 64, toStr);
	if (inUImm12(imm))
	{
		toStr->addInstruction("SUB", regName(to, 64), regName(l, 64), immediate(imm));
		return;
	}
	if (inUImm12L12(imm))
	{
		toStr->addInstruction("SUB", regName(to, 64), regName(l, 64), immediate(imm >> 12), leftShift12());
		return;
	}
	if (inUImm24(imm))
	{
		sub64(to, l, imm & 4095, toStr);
		sub64(to, to, imm & 16773120, toStr);
		return;
	}
	auto reg = getIP();
	makeI64Immediate(imm, reg, toStr);
	sub(to, l, reg, 64, toStr);
	releaseIP(reg);
}

void CodeGen::call(const MOperand* address, CodeString* toStr)
{
	auto f = dynamic_cast<const FuncAddress*>(address);
	ASSERT(f != nullptr);
	toStr->addInstruction("BL", f->function()->name());
	if (func2Call_ != nullptr)
	{
		ASSERT(f->function() == func2Call_);
		add64(sp(), sp(), func2Call_->fix_move_offset(), toStr);
		LOG(color::cyan("call, eliminate func2Call ") + func2Call_->print());
		func2Call_ = nullptr;
	}
}

void CodeGen::ret(CodeString* toStr)
{
	toStr->addInstruction("RET");
}

void CodeGen::cset(const MOperand* to, Instruction::OpID cond, CodeString* toStr)
{
	if (cond == Instruction::add)
		return makeI32Immediate(1, dynamic_cast<const Register*>(to), toStr);
	if (cond == Instruction::sub)
		return makeI32Immediate(0, dynamic_cast<const Register*>(to), toStr);
	auto bb = dynamic_cast<const Register*>(to);
	ASSERT(bb != nullptr);
	return toStr->addInstruction("CSET", regName(bb, 32), condName(cond));
}


void CodeGen::extend32To64(const MOperand* from, const MOperand* to, CodeString* toStr)
{
	auto f = dynamic_cast<const Register*>(from);
	auto t = dynamic_cast<const Register*>(to);
	ASSERT(t);
	if (auto imm = dynamic_cast<const Immediate*>(from); imm != nullptr)
	{
		long long i = imm->asInt();
		return makeI64Immediate(i, t, toStr);
	}
	ASSERT(f);
	return toStr->addInstruction("SXTW", regName(t, 64), regName(f, 32));
}

void CodeGen::f2i(const MOperand* from, const MOperand* to, CodeString* toStr)
{
	auto f = dynamic_cast<const Register*>(from);
	auto t = dynamic_cast<const Register*>(to);
	ASSERT(t);
	if (auto imm = dynamic_cast<const Immediate*>(from); imm != nullptr)
	{
		int i = static_cast<int>(imm->asFloat());
		return makeI32Immediate(i, t, toStr);
	}
	ASSERT(f);
	return toStr->addInstruction("FCVTZS", regName(t, 32), regName(f, 32));
}

void CodeGen::i2f(const MOperand* from, const MOperand* to, CodeString* toStr)
{
	auto f = dynamic_cast<const Register*>(from);
	auto t = dynamic_cast<const Register*>(to);
	ASSERT(t);
	if (auto imm = dynamic_cast<const Immediate*>(from); imm != nullptr)
	{
		float i = static_cast<float>(imm->asInt());
		return makeF32Immediate(i, t, toStr);
	}
	ASSERT(f);
	return toStr->addInstruction("SCVTF", regName(t, 32), regName(f, 32));
}

void CodeGen::compare(const MCMP* inst, const MOperand* l, const MOperand* r, bool flt, CodeString* toStr)
{
	auto lm = dynamic_cast<const Immediate*>(l);
	auto rm = dynamic_cast<const Immediate*>(r);
	auto lr = dynamic_cast<const Register*>(l);
	auto rr = dynamic_cast<const Register*>(r);
	ASSERT(lm != nullptr || lr != nullptr);
	ASSERT(rm != nullptr || rr != nullptr);
	if (flt)
	{
		if (lm && rm)
		{
			bool b = lm->asFloat() > rm->asFloat();
			bool e = lm->asFloat() == rm->asFloat();
			if (b)
			{
				if (inst->tiedB_)
					decideCond(inst->tiedB_, 1);
				if (inst->tiedC_)
					decideCond(inst->tiedC_, 1);
			}
			else if (e)
			{
				if (inst->tiedB_)
					decideCond(inst->tiedB_, 0);
				if (inst->tiedC_)
					decideCond(inst->tiedC_, 0);
			}
			else
			{
				if (inst->tiedB_)
					decideCond(inst->tiedB_, -1);
				if (inst->tiedC_)
					decideCond(inst->tiedC_, -1);
			}
			return;
		}
		if (lm && lm->isZero(true, 32))
		{
			reverseCmpOp(inst);
			auto reg = getFIP();
			copy(reg, zeroRegister(), 32, toStr);
			toStr->addInstruction("FCMP", regName(rr, 32), regName(reg, 32));
			releaseIP(reg);
			return;
		}
		if (rm && rm->isZero(true, 32))
		{
			auto reg = getFIP();
			copy(reg, zeroRegister(), 32, toStr);
			toStr->addInstruction("FCMP", regName(lr, 32), regName(reg, 32));
			releaseIP(reg);
			return;
		}
		lr = op2reg(l, 32, false, toStr);
		rr = op2reg(r, 32, false, toStr);
		toStr->addInstruction("FCMP", regName(lr, 32), regName(rr, 32));
		releaseIP(lr);
		releaseIP(rr);
		return;
	}
	if (lm && rm)
	{
		bool b = lm->asInt() > rm->asInt();
		bool e = lm->asInt() == rm->asInt();
		if (b)
		{
			if (inst->tiedB_)
				decideCond(inst->tiedB_, 1);
			if (inst->tiedC_)
				decideCond(inst->tiedC_, 1);
		}
		else if (e)
		{
			if (inst->tiedB_)
				decideCond(inst->tiedB_, 0);
			if (inst->tiedC_)
				decideCond(inst->tiedC_, 0);
		}
		else
		{
			if (inst->tiedB_)
				decideCond(inst->tiedB_, -1);
			if (inst->tiedC_)
				decideCond(inst->tiedC_, -1);
		}
		return;
	}
	if (lm && (inUImm12(lm->asInt()) || inUImm12L12(lm->asInt())))
	{
		reverseCmpOp(inst);
		if (inst->tiedC_)
		{
			toStr->addInstruction("SUBS", regName(dynamic_cast<const Register*>(inst->tiedC_->def(0)), 32),
			                      regName(rr, 32), immediate(lm->asInt()));
			inst->tiedC_->disable_ = true;
		}
		else
			toStr->addInstruction("CMP", regName(rr, 32), immediate(lm->asInt()));
		return;
	}
	if (rm && (inUImm12(rm->asInt()) || inUImm12L12(rm->asInt())))
	{
		if (inst->tiedC_)
		{
			toStr->addInstruction("SUBS", regName(dynamic_cast<const Register*>(inst->tiedC_->def(0)), 32),
			                      regName(lr, 32), immediate(rm->asInt()));
			inst->tiedC_->disable_ = true;
		}
		else
			toStr->addInstruction("CMP", regName(lr, 32), immediate(rm->asInt()));
		return;
	}
	lr = op2reg(l, 32, true, toStr);
	rr = op2reg(r, 32, true, toStr);
	if (inst->tiedC_)
	{
		toStr->addInstruction("SUBS", regName(dynamic_cast<const Register*>(inst->tiedC_->def(0)), 32),
		                      regName(lr, 32), regName(rr, 32));
		inst->tiedC_->disable_ = true;
	}
	toStr->addInstruction("CMP", regName(lr, 32), regName(rr, 32));
	releaseIP(lr);
	releaseIP(rr);
}

void CodeGen::reverseCmpOp(const MCMP* inst)
{
	auto cset = inst->tiedC_;
	if (cset != nullptr)
	{
		cset->op_ = lrShiftOp(cset->op_);
		return;
	}
	auto bcc = inst->tiedB_;
	if (bcc != nullptr)
	{
		bcc->op_ = lrShiftOp(bcc->op_);
		return;
	}
	ASSERT(false);
}

void CodeGen::sub32(const Register* to, const Register* l, int imm, CodeString* toStr)
{
	if (imm < 0) return add32(to, l, -imm, toStr);
	if (imm == 0) return copy(to, l, 32, toStr);
	if (inUImm12(imm))
		return toStr->addInstruction("SUB", regName(to, 32), regName(l, 32), immediate(imm));
	if (inUImm12L12(imm))
		return toStr->addInstruction("SUB", regName(to, 32), regName(l, 32), immediate(imm >> 12), leftShift12());
	if (inUImm24(imm))
	{
		sub32(to, l, imm & 4095, toStr);
		sub32(to, to, imm & 16773120, toStr);
		return;
	}
	auto reg = getIP();
	makeI32Immediate(imm, reg, toStr);
	sub(to, l, reg, 32, toStr);
	releaseIP(reg);
}

void CodeGen::sub(const Register* to, const Register* l, const Register* r, int len, CodeString* toStr)
{
	toStr->addInstruction("SUB", regName(to, len), regName(l, len), regName(r, len));
}

void CodeGen::mathRRInst(const Register* to, const Register* l, const Register* r,
                         Instruction::OpID op, int len, CodeString* toStr)
{
	switch (op) // NOLINT(clang-diagnostic-switch-enum)
	{
		case Instruction::add: return add(to, l, r, len, toStr);
		case Instruction::sub: return sub(to, l, r, len, toStr);
		case Instruction::mul: return toStr->addInstruction("MUL", regName(to, len), regName(l, len), regName(r, len));
		case Instruction::sdiv: return toStr->
				addInstruction("SDIV", regName(to, len), regName(l, len), regName(r, len));
		case Instruction::srem:
			{
				auto ip = getIP();
				toStr->addInstruction("SDIV", regName(ip, len), regName(l, len),
				                      regName(r, len));
				toStr->addInstruction("MSUB", regName(to, len), regName(ip, len),
				                      regName(r, len),
				                      regName(l, len));
				releaseIP(ip);
				return;
			}
		default: break;
	}
	throw runtime_error("unexpected");
}


void CodeGen::maddsub(MOperand* to, MOperand* l, MOperand* r, MOperand* s, bool isAdd, CodeString* toStr)
{
	auto tor = dynamic_cast<Register*>(to);
	ASSERT(tor != nullptr);
	auto li = dynamic_cast<Immediate*>(l);
	auto ri = dynamic_cast<Immediate*>(r);
	auto si = dynamic_cast<Immediate*>(s);
	ASSERT(si == nullptr || li == nullptr || ri == nullptr);
	const Register* lr = dynamic_cast<Register*>(l);
	const Register* rr = dynamic_cast<Register*>(r);
	const Register* sr = dynamic_cast<Register*>(s);
	ASSERT((si || sr) && (li || lr) && (ri || rr));
	bool isI = tor->isIntegerRegister();
	lr = op2reg(l, 32, isI, toStr);
	rr = op2reg(r, 32, isI, toStr);
	sr = op2reg(s, 32, isI, toStr);
	if (isAdd)
		toStr->addInstruction(isI ? "MADD" : "FMADD", regName(tor, 32), regName(lr, 32), regName(rr, 32),
		                      regName(sr, 32));
	else
		toStr->addInstruction(isI ? "MSUB" : "FMSUB", regName(tor, 32), regName(lr, 32), regName(rr, 32),
		                      regName(sr, 32));
	releaseIP(sr);
	releaseIP(lr);
	releaseIP(rr);
}

void CodeGen::mneg(MOperand* to, MOperand* l, MOperand* r, CodeString* toStr)
{
	auto tor = dynamic_cast<Register*>(to);
	ASSERT(tor != nullptr);
	auto li = dynamic_cast<Immediate*>(l);
	auto ri = dynamic_cast<Immediate*>(r);
	const Register* lr = dynamic_cast<Register*>(l);
	const Register* rr = dynamic_cast<Register*>(r);
	ASSERT((li || lr) && (ri || rr));
	bool isI = tor->isIntegerRegister();
	lr = op2reg(l, 32, isI, toStr);
	rr = op2reg(r, 32, isI, toStr);
	toStr->addInstruction("MNEG", regName(tor, 32), regName(lr, 32), regName(rr, 32));
	releaseIP(lr);
	releaseIP(rr);
}

void CodeGen::fsub(const Register* to, const Register* l, const Register* r, CodeString* toStr)
{
	return toStr->addInstruction("FSUB", regName(to, 32), regName(l, 32), regName(r, 32));
}

void CodeGen::copy(const Register* to, const Register* from, int len, CodeString* toStr)
{
	if (to == from) return;
	if (to->isIntegerRegister() && from->isIntegerRegister())
		return toStr->addInstruction("MOV", regName(to, len), regName(from, len));
	return toStr->addInstruction("FMOV", regName(to, len), regName(from, len));
}

void CodeGen::copy(const Register* to, const Immediate* from, int len, CodeString* toStr)
{
	if (to->isIntegerRegister())
	{
		if (len == 32) return makeI32Immediate(from->asInt(), to, toStr);
		return makeI64Immediate(from->as64BitsInt(), to, toStr);
	}
	makeF32Immediate(from->asFloat(), to, toStr);
}

void CodeGen::copy(const MOperand* to, const MOperand* from, int len, CodeString* toStr)
{
	ASSERT(len == 32 || len == 64);
	auto tor = dynamic_cast<const Register*>(to);
	ASSERT(tor);
	if (const Register* fromr = dynamic_cast<const Register*>(from); fromr != nullptr)
	{
		copy(tor, fromr, len, toStr);
		return;
	}
	if (const Immediate* fromr = dynamic_cast<const Immediate*>(from); fromr != nullptr)
	{
		copy(tor, fromr, len, toStr);
		return;
	}
	if (const GlobalAddress* fromr = dynamic_cast<const GlobalAddress*>(from); fromr != nullptr)
	{
		copy(tor, fromr, len, toStr);
		return;
	}
	if (const FrameIndex* fromr = dynamic_cast<const FrameIndex*>(from); fromr != nullptr)
	{
		copy(tor, fromr, len, toStr);
		return;
	}
	throw runtime_error("unexpected");
}

void CodeGen::copy(const Register* to, const GlobalAddress* from, int len, CodeString* toStr)
{
	ASSERT(len == 64);
	return toStr->addInstruction("LDR", regName(to, 64), "=" + from->name_);
}

void CodeGen::copy(const Register* to, const FrameIndex* from, int len, CodeString* toStr)
{
	list<string> ret;
	auto offset = frameOffset(from, false, toStr);
	add64(to, sp(), offset, toStr);
}

void CodeGen::makeInstruction(MInstruction* instruction)
{
	instruction->str = new CodeString{};
	auto toStr = instruction->str;
	LOG(color::green("# " + instruction->print()));
	if (auto i = dynamic_cast<MCopy*>(instruction); i != nullptr)
		copy(instruction->operands()[1], instruction->operands()[0], (i->copy_len()), toStr);
	else if (auto i2 = dynamic_cast<MSTR*>(instruction); i2 != nullptr)
	{
		auto fc = i2->forCall_;
		str(instruction->operands()[0], instruction->operands()[1], (i2->width()), toStr, fc);
	}
	else if (auto i3 = dynamic_cast<MLDR*>(instruction); i3 != nullptr)
		ldr(instruction->operands()[0], instruction->operands()[1], (i3->width()), toStr);
	else if (auto i4 = dynamic_cast<MST1V16B*>(instruction); i4 != nullptr)
		st1(instruction->operands()[0], i4->storeCount_, i4->offset_, toStr);
	else if (auto i5 = dynamic_cast<MLD1V16B*>(instruction); i5 != nullptr)
		ld1(instruction->operands()[0], i5->loadCount_, i5->offset_, toStr);
	else if (auto i6 = dynamic_cast<MST1ZTV16B*>(instruction); i6 != nullptr)
		clearV(i6->loadCount_, toStr);
	else if (auto i7 = dynamic_cast<MMathInst*>(instruction); i7 != nullptr)
		mathInst(instruction->operands()[0], instruction->operands()[1], instruction->operands()[2], i7->op(),
		         (i7->width()),
		         toStr);
	else if (auto i8 = dynamic_cast<MBL*>(instruction); i8 != nullptr)
		call(instruction->operands()[0], toStr);
	else if (auto i9 = dynamic_cast<MRet*>(instruction); i9 != nullptr)
		ret(toStr);
	else if (auto i12 = dynamic_cast<MCMP*>(instruction); i12 != nullptr)
		compare(i12, i12->operands()[0], i12->operands()[1], !i12->itff_, toStr);
	else if (auto i13 = dynamic_cast<MCSET*>(instruction); i13 != nullptr)
	{
		if (!i13->disable_)
			cset(i13->operands()[0], i13->op_, toStr);
	}
	else if (auto i14 = dynamic_cast<MFCVTZS*>(instruction); i14 != nullptr)
		f2i(i14->operands()[1], i14->operands()[0], toStr);
	else if (auto i15 = dynamic_cast<MSCVTF*>(instruction); i15 != nullptr)
		i2f(i15->operands()[1], i15->operands()[0], toStr);
	else if (auto i16 = dynamic_cast<MSXTW*>(instruction); i16 != nullptr)
		extend32To64(i16->operands()[0], i16->operands()[1], toStr);
	else if (auto i10 = dynamic_cast<MB*>(instruction); i10 != nullptr)
	{
	}
	else if (auto i11 = dynamic_cast<MMAddSUB*>(instruction); i11 != nullptr)
		maddsub(i11->operand(0), i11->operand(1), i11->operand(2), i11->operand(3), i11->add_, toStr);
	else if (auto i17 = dynamic_cast<MNeg*>(instruction); i17 != nullptr)
		mneg(i17->operand(0), i17->operand(1), i17->operand(2), toStr);
	else if (auto i18 = dynamic_cast<M2SIMDCopy*>(instruction); i18 != nullptr)
		simdcp(i18->operand(0), i18->operand(1), i18->copy_len(), i18->lane(), i18->isLoad(), toStr);
	else
		ASSERT(false);
}

std::string CodeGen::regName(const Register* reg, int len)
{
	if (len == 32) return reg->shortName_;
	if (len == 64)
	{
		if (reg->isIntegerRegister()) return reg->name_;
		return "D" + to_string(reg->id());
	}
	return reg->name_;
}

std::string CodeGen::simd32RegName(const Register* reg, int lane)
{
	return "V" + to_string(reg->id()) + ".s[" + to_string(lane) + "]";
}

std::string CodeGen::immediate(int i)
{
	return "#" + to_string(i);
}

std::string CodeGen::immediate(unsigned i)
{
	return "#" + to_string(i);
}

std::string CodeGen::immediate(long long i)
{
	return "#" + to_string(i);
}

std::string CodeGen::fimmediate(float i)
{
	return "#" + to_string(i);
}

std::string CodeGen::immediate(float i)
{
	int f = 0;
	memcpy(&f, &i, sizeof(float));
	return "#" + to_string(f);
}

std::string CodeGen::poolImmediate(int i)
{
	return "=" + to_string(i);
}

std::string CodeGen::poolImmediate(unsigned i)
{
	return "=" + to_string(i);
}

Register* CodeGen::floatRegister(int i) const
{
	return m_->fregs_[i];
}

void CodeGen::decideCond(MInstruction* instruction, int cond)
{
	Instruction::OpID op;
	auto b = dynamic_cast<MB*>(instruction);
	auto cset = dynamic_cast<MCSET*>(instruction);
	if (b != nullptr) op = b->op_;
	else
	{
		ASSERT(cset);
		op = cset->op_;
	}
	bool ok = false;
	switch (op) // NOLINT(clang-diagnostic-switch-enum)
	{
		case Instruction::ge:
			{
				ok = cond >= 0;
				break;
			}
		case Instruction::gt:
			{
				ok = cond > 0;
				break;
			}
		case Instruction::le:
			{
				ok = cond <= 0;
				break;
			}
		case Instruction::lt:
			{
				ok = cond < 0;
				break;
			}
		case Instruction::eq:
			{
				ok = cond == 0;
				break;
			}
		case Instruction::ne:
			{
				ok = cond != 0;
				break;
			}
		default: break;
	}
	op = Instruction::add;
	if (!ok) op = Instruction::sub;
	if (b == nullptr) cset->op_ = op;
	else
	{
		if (!ok) b->removeL();
		else b->removeR();
	}
}


std::string CodeGen::poolImmediate(long long i)
{
	return "=" + to_string(i);
}

Register* CodeGen::zeroRegister() const
{
	return Register::getZERO(m_);
}


void CodeGen::makeI64Immediate(long long i, const Register* placeIn, CodeString* toStr)
{
	if (i == 0) return copy(placeIn, zeroRegister(), 32, toStr);
	if (useZRRegisterAsCommonRegister)
	{
		if (i > 0)
		{
			if (inUImm12(i) || inUImm12L12(i)) return add32(placeIn, zeroRegister(), static_cast<int>(i), toStr);
		}
		else if (inUImm12(-i) || inUImm12L12(-i)) return sub32(placeIn, zeroRegister(), static_cast<int>(-i), toStr);
	}
	unsigned a[4] = {
		static_cast<unsigned>(ll2ullKeepBits((i >> 48) & 65535)),
		static_cast<unsigned>(ll2ullKeepBits((i >> 32) & 65535)),
		static_cast<unsigned>(ll2ullKeepBits((i >> 16) & 65535)),
		static_cast<unsigned>(ll2ullKeepBits(i & 65535))
	};
	int offset[4] = {48, 32, 16, 0};
	int nc = 0;
	int zc = 0;
	for (int t = 0; t < 3; t++)
	{
		if (a[t] == 65535u) nc++;
		else if (a[t] == 0u) zc++;
	}
	bool uz = zc >= nc;
	int count = 4 - (uz ? zc : nc);
	if (count > useLDRInsteadOfMov2CreateIntegerWhenMovCountBiggerThan)
	{
		return toStr->addInstruction("LDR", regName(placeIn, 64), poolImmediate(i));
	}
	if (uz)
	{
		toStr->addInstruction("MOVZ", regName(placeIn, 64), immediate(a[3]));
		for (int t = 2; t > -1; t--)
		{
			if (a[t] != 0u)
				toStr->addInstruction("MOVK", regName(placeIn, 64),
				                      immediate(a[t]), leftShift(offset[t]));
		}
	}
	else
	{
		toStr->addInstruction("MOVN", regName(placeIn, 64), immediate((~a[3]) & 65535u));
		for (int t = 2; t > -1; t--)
		{
			if (a[t] != 65535u)
				toStr->addInstruction("MOVK", regName(placeIn, 64),
				                      immediate(a[t]), leftShift(offset[t]));
		}
	}
}


int CodeGen::makeI64ImmediateNeedInstCount(long long i)
{
	if (i == 0) return 1;
	if (useZRRegisterAsCommonRegister)
	{
		if (i > 0)
		{
			if (inUImm12(i) || inUImm12L12(i)) return 1;
		}
		else if (inUImm12(-i) || inUImm12L12(-i)) return 1;
	}
	unsigned a[4] = {
		static_cast<unsigned>(ll2ullKeepBits((i >> 48) & 65535)),
		static_cast<unsigned>(ll2ullKeepBits((i >> 32) & 65535)),
		static_cast<unsigned>(ll2ullKeepBits((i >> 16) & 65535)),
		static_cast<unsigned>(ll2ullKeepBits(i & 65535))
	};
	int nc = 0;
	int zc = 0;
	for (int t = 0; t < 3; t++)
	{
		if (a[t] == 65535u) nc++;
		else if (a[t] == 0u) zc++;
	}
	bool uz = zc >= nc;
	int count = 4 - (uz ? zc : nc);
	if (count > useLDRInsteadOfMov2CreateIntegerWhenMovCountBiggerThan)
	{
		return 5;
	}
	int num = 0;
	if (uz)
	{
		num++;
		for (int t = 2; t > -1; t--)
		{
			if (a[t] != 0u)

				num++;
		}
	}
	else
	{
		num++;
		for (int t = 2; t > -1; t--)
		{
			if (a[t] != 65535u)

				num++;
		}
	}
	return num;
}

void CodeGen::makeI32Immediate(int i, const Register* placeIn, CodeString* toStr)
{
	if (i == 0) return copy(placeIn, zeroRegister(), 32, toStr);
	if (useZRRegisterAsCommonRegister)
	{
		if (i > 0)
		{
			if (inUImm12(i) || inUImm12L12(i)) return add32(placeIn, zeroRegister(), i, toStr);
		}
		else
		{
			if (inUImm12(-i) || inUImm12L12(-i)) return sub32(placeIn, zeroRegister(), -i, toStr);
		}
	}
	unsigned a[2] = {
		static_cast<unsigned>(ll2ullKeepBits((i >> 16) & 65535)), static_cast<unsigned>(ll2ullKeepBits(i & 65535))
	};
	int offset[4] = {16, 0};
	int nc = 0;
	int zc = 0;
	if (a[0] == 65535u) nc++;
	else if (a[0] == 0u) zc++;
	bool uz = zc >= nc;
	int count = 2 - (uz ? zc : nc);
	if (count > useLDRInsteadOfMov2CreateIntegerWhenMovCountBiggerThan)
	{
		return toStr->addInstruction("LDR", regName(placeIn, 32), poolImmediate(i));
	}
	if (uz)
	{
		toStr->addInstruction("MOVZ", regName(placeIn, 32), immediate(a[1]));
		if (a[0] != 0u)
			toStr->addInstruction("MOVK", regName(placeIn, 32),
			                      immediate(a[0]), leftShift(offset[0]));
	}
	else
	{
		toStr->addInstruction("MOVN", regName(placeIn, 32), immediate(~a[1] & 65535u));
		if (a[0] != 65535u)
			toStr->addInstruction("MOVK", regName(placeIn, 32),
			                      immediate(a[0]), leftShift(offset[0]));
	}
}

void CodeGen::makeF32Immediate(float i, const Register* placeIn, CodeString* toStr)
{
	unsigned u = 0;
	memcpy(&u, &i, sizeof(float));
	if (floatMovSupport(u))
		return toStr->addInstruction("FMOV", regName(placeIn, 32), fimmediate(i));
	unsigned a[2] = {
		((u >> 16u) & 65535u), (u & 65535u)
	};

	int cu = u2iKeepBits(u);

	if (useZRRegisterAsCommonRegister)
	{
		if (cu > 0)
		{
			if (inUImm12(cu) || inUImm12L12(cu)) return add32(placeIn, zeroRegister(), cu, toStr);
		}
		else
		{
			if (inUImm12(-cu) || inUImm12L12(-cu)) return sub32(placeIn, zeroRegister(), -cu, toStr);
		}
	}

	int offset[4] = {16, 0};
	int nc = 0;
	int zc = 0;
	if (a[0] == 65535u) nc++;
	else if (a[0] == 0u) zc++;
	bool uz = zc >= nc;
	int count = 2 - (uz ? zc : nc);
	if (count > useLDRInsteadOfMov2CreateFloatWhenMovCountBiggerThan)
	{
		return toStr->addInstruction("LDR", regName(placeIn, 32), poolImmediate(u));
	}
	auto reg = getIP();
	if (uz)
	{
		toStr->addInstruction("MOVZ", regName(reg, 32), immediate(a[1]));
		if (a[0] != 0u)
			toStr->addInstruction("MOVK", regName(reg, 32),
			                      immediate(a[0]), leftShift(offset[0]));
	}
	else
	{
		toStr->addInstruction("MOVN", regName(reg, 32), immediate(~a[1] & 65535u));
		if (a[0] != 65535u)
			toStr->addInstruction("MOVK", regName(reg, 32),
			                      immediate(a[0]), leftShift(offset[0]));
	}
	copy(placeIn, reg, 32, toStr);
	releaseIP(reg);
}

void CodeGen::makeImmediate(const Immediate* i, bool flt, int len, const Register* placeIn, CodeString* toStr)
{
	ASSERT(len == 32 || len == 64);
	if (flt)
	{
		ASSERT(len == 32);
		ASSERT(placeIn->isIntegerRegister() == false);
		return makeF32Immediate(i->asFloat(), placeIn, toStr);
	}
	ASSERT(placeIn->isIntegerRegister());
	return len == 32
		       ? makeI32Immediate(i->asInt(), placeIn, toStr)
		       : makeI64Immediate(i->as64BitsInt(), placeIn, toStr);
}

std::string CodeGen::leftShift12()
{
	return "LSL #12";
}

std::string CodeGen::leftShift(int count)
{
	return "LSL #" + to_string(count);
}

std::string CodeGen::functionSize(const std::string& name)
{
	return "	.size " + name + ", .-" + name;
}

std::string CodeGen::regDataOffset(const Register* reg, int offset)
{
	return "[" + reg->name_ + ", " + immediate(offset) + "]";
}

std::string CodeGen::regDataRegLSLOffset(const Register* reg, const Register* regofs, int offset)
{
	return "[" + reg->name_ + ", " + regofs->name_ + ", " + leftShift(offset) + "]";
}

std::string CodeGen::regData(const Register* reg)
{
	return "[" + reg->name_ + "]";
}

const char* CodeGen::genMemcpy()
{
	return R"123(	.globl __memcpy__
	.type __memcpy__, @function
__memcpy__:
	LSR X16, X2, #3
__memcpy__1:
	CBZ X16, __memcpy__2
	LD1 {V0.16B,V1.16B,V2.16B,V3.16B}, [X1], #64
	ST1 {V0.16B,V1.16B,V2.16B,V3.16B}, [X0], #64
	LD1 {V0.16B,V1.16B,V2.16B,V3.16B}, [X1], #64
	ST1 {V0.16B,V1.16B,V2.16B,V3.16B}, [X0], #64
	SUBS X16, X16, #1
	B.NE __memcpy__1
__memcpy__2:
	AND X16, X2, #0x7
__memcpy__3:
	CBZ X16, __memcpy__4
	LDR Q0, [X1], #16
	STR Q0, [X0], #16
	SUBS X16, X16, #1
	B.NE __memcpy__3
__memcpy__4:
	RET
	.size __memcpy__, .-__memcpy__)123";
}

const char* CodeGen::genMemclr()
{
	return R"123(	.globl __memclr__
	.type __memclr__, @function
__memclr__:
	EOR V0.16B, V0.16B, V0.16B
	EOR V1.16B, V1.16B, V1.16B
	EOR V2.16B, V2.16B, V2.16B
	EOR V3.16B, V3.16B, V3.16B
	LSR X16, X1, #3
__memclr__1:
	CBZ X16, __memclr__2
	ST1 {V0.16B,V1.16B,V2.16B,V3.16B}, [X0], #64
	ST1 {V0.16B,V1.16B,V2.16B,V3.16B}, [X0], #64
	SUBS X16, X16, #1
	B.NE __memclr__1
__memclr__2:
	AND X16, X1, #0x7
__memclr__3:
	CBZ X16, __memclr__4
	STR Q0, [X0], #16
	SUBS X16, X16, #1
	B.NE __memclr__3
__memclr__4:
	RET
	.size __memclr__, .-__memclr__)123";
}

bool CodeGen::canLSInOneSPMove(const std::vector<pair<Register*, long long>>& offsets)
{
	if (offsets.empty()) return true;
	if (offsets.back().second > 1016) return false;
	int lc = 0;
	int rc = 0;
	for (auto [i,j] : offsets)
	{
		if (i->isIntegerRegister()) lc++;
		else rc++;
	}
	if ((lc & 1) != 0 && !inImm9(offsets[0].second)) return false;
	if ((rc & 1) != 0 && !inImm9(offsets[lc].second)) return false;
	if (!inImm7L3(offsets[offsets.size() - 1].second)) return false;
	return true;
}

// 区分不同的参数栈帧: caller 永远不会往自己的参数栈帧存值, 只会往 callee 的栈帧存值

// 获得 FrameIndex 在当前语境偏移, 如必要, 插入函数调用前的 SP 移动
long long CodeGen::frameOffset(const FrameIndex* index, bool forCall, CodeString* appendSlot)
{
	if (func2Call_)
	{
		if (!index->stack_t_fix_f() && forCall)
		{
			ASSERT(index->func() == func2Call_);
			return index->offset() - index->func()->stack_move_offset();
		}
		ASSERT(index->func() == func_);
		return index->offset() + func2Call_->fix_move_offset();
	}
	if (!index->stack_t_fix_f() && forCall)
	{
		sub64(sp(), sp(), index->func()->fix_move_offset(), appendSlot);
		LOG(color::cyan("store, set func2Call ") + index->func()->name());
		func2Call_ = index->func();
		return frameOffset(index, forCall, appendSlot);
	}
	ASSERT(index->func() == func_);
	return index->offset();
}

Register* CodeGen::sp() const
{
	return Register::getSP(m_);
}

Register* CodeGen::getIP()
{
	ip16_ = !ip16_;
	if (ip16_)
	{
		if (ipcd_[0] == false)
		{
			ipcd_[0] = true;
			return Register::getIIP0(m_);
		}
		if (ipcd_[1] == false)
		{
			ipcd_[1] = true;
			return Register::getIIP1(m_);
		}
	}
	else
	{
		if (ipcd_[1] == false)
		{
			ipcd_[1] = true;
			return Register::getIIP1(m_);
		}
		if (ipcd_[0] == false)
		{
			ipcd_[0] = true;
			return Register::getIIP0(m_);
		}
	}
	return nullptr;
}

Register* CodeGen::getIPOfType(bool flt)
{
	return flt ? getFIP() : getIP();
}

Register* CodeGen::getFIP()
{
	fip16_ = !fip16_;
	if (fip16_)
	{
		if (fipcd_[0] == false)
		{
			fipcd_[0] = true;
			return Register::getFIP0(m_);
		}
		if (fipcd_[1] == false)
		{
			fipcd_[1] = true;
			return Register::getFIP1(m_);
		}
	}
	else
	{
		if (fipcd_[1] == false)
		{
			fipcd_[1] = true;
			return Register::getFIP1(m_);
		}
		if (fipcd_[0] == false)
		{
			fipcd_[0] = true;
			return Register::getFIP0(m_);
		}
	}
	return nullptr;
}

void CodeGen::releaseIP(const Register* r)
{
	if (r == nullptr) return;
	if (r->isIntegerRegister())
	{
		if (r == Register::getIIP0(m_)) ipcd_[0] = false;
		else if (r == Register::getIIP1(m_)) ipcd_[1] = false;
	}
	else if (r == Register::getFIP0(m_)) fipcd_[0] = false;
	else if (r == Register::getFIP1(m_))fipcd_[1] = false;
}

void CodeGen::setBuf(int id, const MOperand* op)
{
	ASSERT(op);
	ASSERT(!opbuffer[id]);
	opbuffer[id] = op;
}

const MOperand* CodeGen::getBuf(int id) const
{
	ASSERT(opbuffer[id]);
	auto ret = opbuffer[id];
	return ret;
}

void CodeGen::releaseBuf(int id)
{
	ASSERT(opbuffer[id]);
	opbuffer[id] = nullptr;
}

CodeGen::CodeGen(MModule* m): MachinePass(m)
{
}

void CodeGen::run()
{
	m_->modulePrefix_ = new CodeString{};
	m_->moduleSuffix_ = new CodeString{};
	if (const auto globalConstants = m_->constGlobalAddresses(); !globalConstants.empty())
	{
		m_->modulePrefix_->addSection("section .rodata, \"a\", @progbits");
		for (auto& glob : globalConstants)
			(makeGlobal(glob));
	}
	if (const auto globalZeros = m_->ncZeroGlobalAddresses(); !globalZeros.empty())
	{
		m_->modulePrefix_->addSection("bss");
		for (auto& glob : globalZeros)
			(makeGlobal(glob));
	}
	if (const auto globals = m_->ncnzGlobalAddresses(); !globals.empty())
	{
		m_->modulePrefix_->addSection("data");
		for (auto& glob : globals)
			(makeGlobal(glob));
	}
	m_->modulePrefix_->addSection("text");
	m_->modulePrefix_->addAlign(4, false);
	bool haveMemcpy = false;
	bool haveMemclr = false;
	int cpid = m_->memcpy_->id();
	int clid = m_->memclr_->id();
	for (auto& f : m_->functions())
	{
		func_ = f;
		makeFunction();
		if (!haveMemcpy && f->called().test(cpid)) haveMemcpy = true;
		if (!haveMemclr && f->called().test(clid)) haveMemclr = true;
	}
	if (haveMemcpy) m_->moduleSuffix_->addCommonStr(genMemcpy());
	if (haveMemclr)m_->moduleSuffix_->addCommonStr(genMemclr());
}

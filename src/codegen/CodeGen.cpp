#include "CodeGen.hpp"

#include <cassert>

#include "Ast.hpp"
#include "Config.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineInstruction.hpp"
#include "MachineIR.hpp"
#include "MachineOperand.hpp"

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
		assert(of >= 0);
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


std::string CodeGen::section(const std::string& secName)
{
	return "\t." + secName;
}

std::string CodeGen::objectTypeDeclare(const std::string& name)
{
	return "\t.type " + name + ", @object";
}

std::string CodeGen::functionTypeDeclare(const std::string& name)
{
	return "\t.type " + name + ", @function";
}

std::string CodeGen::align(int dataSize, bool glob)
{
	if ((dataSize > alignTo16NeedBytes) || (glob && dataSize > 8)) return "\t.align 4";
	if (dataSize > 4) return "\t.align 3";
	if (dataSize > 2) return "\t.align 2";
	if (dataSize > 1) return "\t.align 1";
	return "\t.align 0";
}

std::string CodeGen::align(long long dataSize, bool glob)
{
	if ((dataSize > alignTo16NeedBytes) || (glob && dataSize > 8)) return "\t.align 4";
	if (dataSize > 4) return "\t.align 3";
	if (dataSize > 2) return "\t.align 2";
	if (dataSize > 1) return "\t.align 1";
	return "\t.align 0";
}

std::string CodeGen::global(const std::string& globName)
{
	return "\t.globl " + globName;
}

std::string CodeGen::label(const std::string& name)
{
	return name + ":";
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

void CodeGen::append(const std::string& txt)
{
	txt_.emplace_back(txt);
}

void CodeGen::append(const std::list<std::string>& txt)
{
	for (auto& basic_string : txt) txt_.emplace_back(basic_string);
}

std::list<std::string> CodeGen::makeGlobal(const GlobalAddress* address)
{
	auto allSize = logicalRightShift(address->size_, 3);
	list<string> l;
	l.emplace_back(align(allSize, true));
	l.emplace_back(global(address->name_));
	l.emplace_back(objectTypeDeclare(address->name_));
	l.emplace_back(label(address->name_));
	auto init = address->data_;
	for (int i = 0, segCount = init->segmentCount(); i < segCount; i++)
	{
		auto [iv, dc] = init->segment(i);
		if (dc == 0)
			l.emplace_back(word(iv));
		else l.emplace_back(zero(dc << 2));
	}
	l.emplace_back(size(address->name_, allSize));
	return l;
}

std::list<std::string> CodeGen::makeFunction(MFunction* function)
{
	function->removeEmptyBBs();
	list<string> l;
	l.emplace_back(global(function->name_));
	l.emplace_back(functionTypeDeclare(function->name_));
	l.emplace_back(label(function->name_));
	merge(l, functionPrefix(function));
	for (auto bb : function->blocks())
	{
		if (!function->useList()[BlockAddress::get(bb)].empty())
			l.emplace_back(label(bb->name()));
		for (auto inst : bb->instructions())
			merge(l, makeInstruction(inst));
	}
	l.emplace_back(functionSize(function->name()));
	return l;
}

std::list<std::string> CodeGen::functionPrefix(const MFunction* function)
{
	std::list<std::string> l;
	int lc = 0;
	int rc = 0;
	auto& calleeSaved = function->callee_saved();
	for (auto [i,j] : calleeSaved)
	{
		if (i->isIntegerRegister()) lc++;
		else rc++;
	}
	if (canLSInOneSPMove(calleeSaved))
	{
		merge(l, sub64(sp(), sp(), function->stack_move_offset()));
		if (lc & 1) merge(l, str(calleeSaved[0].first, sp(), calleeSaved[0].second, 64));
		if (lc >= 2)
		{
			for (int i = lc & 1; i < lc; i += 2)
			{
				merge(l, stp(calleeSaved[i].first, calleeSaved[i + 1].first, sp(), u2iNegThrow(calleeSaved[i].second), 64));
			}
		}
		if (rc & 1) merge(l, str(calleeSaved[lc].first, sp(), calleeSaved[lc].second, 64));
		if (rc >= 2)
		{
			for (int i = rc & 1; i < rc; i += 2)
			{
				merge(l, stp(calleeSaved[lc + i].first, calleeSaved[lc + i + 1].first, sp(), u2iNegThrow(calleeSaved[lc + i].second),
				             64));
			}
		}
		return l;
	}
	long long minOffset = upAlignTo16(function->stack_move_offset() - calleeSaved[0].second);
	long long nextOffset = function->stack_move_offset() - minOffset;
	merge(l, sub64(sp(), sp(), minOffset));
	if (lc & 1) merge(l, str(calleeSaved[0].first, sp(), calleeSaved[0].second - nextOffset, 64));
	if (lc >= 2)
	{
		for (int i = lc & 1; i < lc; i += 2)
		{
			merge(l, stp(calleeSaved[i].first, calleeSaved[i + 1].first, sp(), u2iNegThrow(calleeSaved[i].second - nextOffset), 64));
		}
	}
	if (rc & 1) merge(l, str(calleeSaved[lc].first, sp(), calleeSaved[lc].second - nextOffset, 64));
	if (rc >= 2)
	{
		for (int i = rc & 1; i < rc; i += 2)
		{
			merge(l, stp(calleeSaved[lc + i].first, calleeSaved[lc + i + 1].first, sp(),
			             u2iNegThrow(calleeSaved[lc + i].second - nextOffset), 64));
		}
	}
	merge(l, sub64(sp(), sp(), nextOffset));
	return l;
}

std::list<std::string> CodeGen::functionSuffix(const MFunction* function)
{
	std::list<std::string> l;
	int lc = 0;
	int rc = 0;
	auto& calleeSaved = function->callee_saved();
	for (auto [i,j] : calleeSaved)
	{
		if (i->isIntegerRegister()) lc++;
		else rc++;
	}
	if (canLSInOneSPMove(calleeSaved))
	{
		if (rc >= 2)
		{
			for (int i = rc - 2; i >= 0; i -= 2)
			{
				merge(l, ldp(calleeSaved[lc + i].first, calleeSaved[lc + i + 1].first, sp(),
				             u2iNegThrow(calleeSaved[lc + i].second),
				             64));
			}
		}
		if (rc & 1) merge(l, ldr(calleeSaved[lc].first, sp(), calleeSaved[lc].second, 64));
		if (lc >= 2)
		{
			for (int i = lc - 2; i >= 0; i -= 2)
			{
				merge(l, ldp(calleeSaved[i].first, calleeSaved[i + 1].first, sp(), u2iNegThrow(calleeSaved[i].second),
				             64));
			}
		}
		if (lc & 1) merge(l, ldr(calleeSaved[0].first, sp(), calleeSaved[0].second, 64));
		merge(l, add64(sp(), sp(), function->stack_move_offset()));
		return l;
	}
	long long minOffset = upAlignTo16(function->stack_move_offset() - calleeSaved[0].second);
	long long nextOffset = function->stack_move_offset() - minOffset;
	merge(l, add64(sp(), sp(), nextOffset));
	if (rc >= 2)
	{
		for (int i = rc - 2; i >= 0; i -= 2)
		{
			merge(l, ldp(calleeSaved[lc + i].first, calleeSaved[lc + i + 1].first, sp(),
			             u2iNegThrow(calleeSaved[lc + i].second - nextOffset), 64));
		}
	}
	if (rc & 1) merge(l, ldr(calleeSaved[lc].first, sp(), calleeSaved[lc].second - nextOffset, 64));
	if (lc >= 2)
	{
		for (int i = lc - 2; i >= 0; i -= 2)
		{
			merge(l, ldp(calleeSaved[i].first, calleeSaved[i + 1].first, sp(),
			             u2iNegThrow(calleeSaved[i].second - nextOffset), 64));
		}
	}
	if (lc & 1) merge(l, ldr(calleeSaved[0].first, sp(), calleeSaved[0].second - nextOffset, 64));
	merge(l, add64(sp(), sp(), minOffset));
	return l;
}

list<std::string> CodeGen::add(const Register* to, const Register* l, const Register* r, int len)
{
	return {instruction("ADD", regName(to, len), regName(l, len), regName(r, len))};
}

std::list<std::string> CodeGen::fadd(const Register* to, const Register* l, const Register* r)
{
	return {instruction("FADD", regName(to, 32), regName(l, 32), regName(r, 32))};
}

std::list<std::string> CodeGen::fmul(const Register* to, const Register* l, const Register* r)
{
	return {instruction("FMUL", regName(to, 32), regName(l, 32), regName(r, 32))};
}

std::list<std::string> CodeGen::fdiv(const Register* to, const Register* l, const Register* r)
{
	return {instruction("FDIV", regName(to, 32), regName(l, 32), regName(r, 32))};
}

std::list<std::string> CodeGen::stp(const Register* a, const Register* b, const Register* c, int offset, int len)
{
	assert(len == 32 || len == 64);
	assert(len == 32 ? inImm7L2(offset) : inImm7L3(offset));
	return {instruction("STP", regName(a, len), regName(b, len), regDataOffset(c, offset))};
}

std::list<std::string> CodeGen::str(const Register* a, const Register* c, long long offset, int len)
{
	assert(len == 32 || len == 64 || len == 128);
	assert(len == 32 ? align4(offset) : align8(offset));
	if (inImm9(offset)) return {instruction("STR", regName(a, len), regDataOffset(c, u2iNegThrow(offset)))};
	long long absOff = offset < 0 ? -offset : offset;
	int maxMov = m_countr_zero(ll2ullKeepBits(absOff));
	int lenLevel = m_countr_zero(i2uKeepBits(len)) - 3;
	if (maxMov > lenLevel) maxMov = lenLevel;
	offset = offset < 0 ? -(absOff >> maxMov) : (absOff >> maxMov);
	auto reg = getIP();
	list<string> ret;
	merge(ret, makeI64Immediate(offset, reg));
	ret.emplace_back(instruction("STR", regName(a, len), regDataRegLSLOffset(c, reg, maxMov)));
	releaseIP(reg);
	return ret;
}

std::list<std::string> CodeGen::str(const MOperand* regLike, const MOperand* stackLike, int len)
{
	list<string> ret;
	if (const Register* i = dynamic_cast<const Register*>(stackLike); i != nullptr)
	{
		const Register* l = op2reg(regLike, len, ret);
		merge(ret, str(l, i, 0, len));
		releaseIP(l);
		return ret;
	}
	assert(dynamic_cast<const Immediate*>(stackLike) == nullptr);
	if (const GlobalAddress* i = dynamic_cast<const GlobalAddress*>(stackLike); i != nullptr)
	{
		const Register* l = op2reg(regLike, len, ret);
		auto r = op2reg(i, 64, ret);
		merge(ret, str(l, r, 0, len));
		releaseIP(l);
		releaseIP(r);
		return ret;
	}
	if (const FrameIndex* i = dynamic_cast<const FrameIndex*>(stackLike); i != nullptr)
	{
		auto offset = frameOffset(i, true, ret);
		const Register* l = op2reg(regLike, len, ret);
		merge(ret, str(l, sp(), offset, len));
		releaseIP(l);
		return ret;
	}
	throw runtime_error("unexpected");
}

const Register* CodeGen::op2reg(const MOperand* op, int len, bool useIntReg, std::list<std::string>& appendSlot)
{
	if (const Register* fromr = dynamic_cast<const Register*>(op); fromr != nullptr)
	{
		if (len == 128)
		{
			assert(!fromr->isIntegerRegister());
			return fromr;
		}
		if (fromr->isIntegerRegister() != useIntReg)
		{
			auto reg = getIP();
			merge(appendSlot, copy(reg, fromr, len));
			return reg;
		}
		return fromr;
	}
	assert(len == 32 || len == 64);
	auto r = useIntReg ? getIP() : getFIP();
	merge(appendSlot, copy(r, op, len));
	return r;
}

const Register* CodeGen::op2reg(const MOperand* op, int len, std::list<std::string>& appendSlot)
{
	if (const Register* fromr = dynamic_cast<const Register*>(op); fromr != nullptr)
	{
		if (len == 128)
		{
			assert(!fromr->isIntegerRegister());
			return fromr;
		}
		return fromr;
	}
	assert(len == 32 || len == 64);
	auto r = getIP();
	merge(appendSlot, copy(r, op, len));
	return r;
}

std::list<std::string> CodeGen::ldp(const Register* a, const Register* b, const Register* c, int offset, int len)
{
	assert(len == 32 || len == 64);
	assert(len == 32 ? inImm7L2(offset) : inImm7L3(offset));
	return {instruction("LDP", regName(a, len), regName(b, len), regDataOffset(c, offset))};
}

std::list<std::string> CodeGen::ldr(const Register* a, const Register* baseOffsetReg, long long offset, int len)
{
	assert(len == 32 || len == 64 || len == 128);
	assert(len == 32 ? align4(offset) : align8(offset));
	if (inImm9(offset)) return {instruction("LDR", regName(a, len), regDataOffset(baseOffsetReg, u2iNegThrow(offset)))};
	long long absOff = offset < 0 ? -offset : offset;
	int maxMov = m_countr_zero(ll2ullKeepBits(absOff));
	int lenLevel = m_countr_zero(i2uKeepBits(len)) - 3;
	if (maxMov > lenLevel) maxMov = lenLevel;
	offset = offset < 0 ? -(absOff >> maxMov) : (absOff >> maxMov);
	auto reg = getIP();
	list<string> ret;
	merge(ret, makeI64Immediate(offset, reg));
	ret.emplace_back(instruction("LDR", regName(a, len), regDataRegLSLOffset(baseOffsetReg, reg, maxMov))); // NOLINT(readability-suspicious-call-argument)
	releaseIP(reg);
	return ret;
}

std::list<std::string> CodeGen::ldr(const MOperand* a, const MOperand* stackLike, int len)
{
	list<string> ret;
	auto l = dynamic_cast<const Register*>(a);
	assert(l != nullptr);
	if (const Register* i = dynamic_cast<const Register*>(stackLike); i != nullptr)
	{
		merge(ret, ldr(l, i, 0, len));
		return ret;
	}
	assert(dynamic_cast<const Immediate*>(stackLike) == nullptr);
	if (const GlobalAddress* i = dynamic_cast<const GlobalAddress*>(stackLike); i != nullptr)
	{
		auto r = op2reg(i, 64, ret);
		merge(ret, ldr(l, r, 0, len));
		releaseIP(r);
		return ret;
	}
	if (const FrameIndex* i = dynamic_cast<const FrameIndex*>(stackLike); i != nullptr)
	{
		auto offset = frameOffset(i, false, ret);
		merge(ret, ldr(l, sp(), offset, len));
		return ret;
	}
	throw runtime_error("unexpected");
}

std::list<std::string> CodeGen::ld1(const MOperand* stackLike, int count, int offset)
{
	if (const Register* i = dynamic_cast<const Register*>(stackLike); i != nullptr)
	{
		return ld1(i, count, offset);
	}
	throw runtime_error("unexpected");
}

std::list<std::string> CodeGen::clearV(int count)
{
	list<std::string> ret;
	for (int i = 0; i < count; i++)
	{
		auto reg = "V" + to_string(i) + ".16B";
		ret.emplace_back(instruction("EOR", reg, reg, reg));
	}
	return {ret};
}

std::list<std::string> CodeGen::mathInst(const MMathInst* inst, const MOperand* t, const MOperand* l, const MOperand* r,
                                         Instruction::OpID op,
                                         int len)
{
	auto target = dynamic_cast<const Register*>(t);
	assert(op >= Instruction::add && op <= Instruction::fdiv);
	assert(op <= Instruction::srem || len != 64);
	assert(target != nullptr);
	bool flt = op >= Instruction::fadd;
	auto immL = dynamic_cast<const Immediate*>(l);
	auto immR = dynamic_cast<const Immediate*>(r);
	if (immR && immR->isZero(flt, len))
	{
		if (op == Instruction::sdiv || op == Instruction::srem || op == Instruction::fdiv)
			return copy(t, zeroRegister(), len);
	}
	if (immL && immR)
	{
		assert(op <= Instruction::srem || len != 64);
		if (op <= Instruction::srem)
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
					default:
						throw runtime_error("unexpected");
				}
				return makeI32Immediate(i, target);
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
				default:
					throw runtime_error("unexpected");
			}
			return makeI64Immediate(i, target);
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
		return makeF32Immediate(f, target);
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
				return copy(target, r, len);
			}
			if (op == Instruction::mul || op == Instruction::sdiv || op == Instruction::srem || op == Instruction::fmul
			    ||
			    op == Instruction::fdiv)
			{
				return copy(target, zeroRegister(), len);
			}
		}
		if (immL->isNotFloatOne(flt, len))
		{
			if (op == Instruction::mul)
			{
				return copy(target, r, len);
			}
		}
	}
	if (immR)
	{
		if (immR->isZero(flt, len))
		{
			if (op == Instruction::add || op == Instruction::fadd)
			{
				return copy(target, l, len);
			}
			if (op == Instruction::mul || op == Instruction::fmul)
			{
				return copy(target, zeroRegister(), len);
			}
		}
		if (immR->isNotFloatOne(flt, len))
		{
			if (op == Instruction::mul || op == Instruction::sdiv || op == Instruction::srem)
			{
				return copy(target, l, len);
			}
		}
	}
	list<string> ret;
	auto regL = dynamic_cast<const Register*>(l);
	auto regR = dynamic_cast<const Register*>(r);
	if (flt)
	{
		if (immL)
		{
			regL = getFIP();
			merge(ret, makeF32Immediate(immL->asFloat(), regL));
			immL = nullptr;
		}
		else if (immR)
		{
			regR = getFIP();
			merge(ret, makeF32Immediate(immR->asFloat(), regR));
			immR = nullptr;
		}
	}
	bool orl = regL || immL;
	bool orr = regR || immR;
	if ((!(orl || orr)) || (!(orl || orr) && op != Instruction::add && op != Instruction::sub && len != 64))
	{
		releaseIP(regL);
		releaseIP(regR);
		return {};
	}
	if (flt)
	{
		switch (op) // NOLINT(clang-diagnostic-switch-enum)
		{
			case Instruction::fadd:
				merge(ret, fadd(target, regL, regR));
				break;
			case Instruction::fsub:
				merge(ret, fsub(target, regL, regR));
				break;
			case Instruction::fmul:
				merge(ret, fmul(target, regL, regR));
				break;
			case Instruction::fdiv:
				merge(ret, fdiv(target, regL, regR));
				break;
			default:
				throw runtime_error("unexpected");
		}
		releaseIP(regL);
		releaseIP(regR);
		return ret;
	}
	auto fl = dynamic_cast<const FrameIndex*>(l);
	auto fr = dynamic_cast<const FrameIndex*>(r);
	if (fl && immR)
	{
		long long offset = frameOffset(fl, false, ret);
		immR = Immediate::getImmediate(immR->as64BitsInt() + offset, m_);
		r = immR;
		regL = sp();
		l = sp();
		fl = nullptr;
	}
	if (fr && immL)
	{
		long long offset = frameOffset(fr, false, ret);
		immL = Immediate::getImmediate(immL->as64BitsInt() + offset, m_);
		l = immL;
		r = sp();
		fr = nullptr;
	}
	if (!regL)
	{
		regL = op2reg(l, len, true, ret);
		fl = nullptr;
	}
	if (immR && (op == Instruction::add || op == Instruction::sub))
	{
		if (op == Instruction::add)
		{
			if (len == 32)
				merge(ret, add32(target, regL, immR->asInt()));
			else merge(ret, add64(target, regL, immR->as64BitsInt()));
			releaseIP(regL);
			return ret;
		}
		if (len == 32)
			merge(ret, sub32(target, regL, immR->asInt()));
		else merge(ret, sub64(target, regL, immR->as64BitsInt()));
		releaseIP(regL);
		return ret;
	}
	regR = op2reg(r, len, true, ret);
	fr = nullptr;
	merge(ret, mathRRInst(target, regL, regR, op, len));
	releaseIP(regL);
	releaseIP(regR);
	return ret;
}

std::list<std::string> CodeGen::ld1(const Register* stackLike, int count, int offset)
{
	if (count == 1) return ldr(floatRegister(0), stackLike, 128);
	string ret = "\tLD1 {";
	for (int i = 0; i < count; i++)
		ret += "V" + to_string(i) + ".16B,";
	ret.pop_back();
	ret += "}, ";
	ret += regData(stackLike);
	if (offset != 0)
	{
		ret += ", " + immediate(offset);
	}
	return {ret};
}

std::list<std::string> CodeGen::st1(const MOperand* stackLike, int count, int offset)
{
	if (const Register* i = dynamic_cast<const Register*>(stackLike); i != nullptr)
	{
		return st1(i, count, offset);
	}
	throw runtime_error("unexpected");
}

std::list<std::string> CodeGen::add32(const Register* to, const Register* l, int imm)
{
	if (imm < 0) return sub32(to, l, -imm);
	if (imm == 0) return copy(to, l, 32);
	if (inUImm12(imm))
		return {instruction("ADD", regName(to, 32), regName(l, 32), immediate(imm))};
	if (inUImm12L12(imm))
		return {instruction("ADD", regName(to, 32), regName(l, 32), immediate(imm >> 12), leftShift12())};
	if (inUImm24(imm))
	{
		auto ret = add32(to, l, imm & 4095);
		merge(ret, add32(to, to, imm & 16773120));
		return ret;
	}
	auto reg = getIP();
	auto ret = makeI32Immediate(imm, reg);
	merge(ret, add(to, l, reg, 32));
	releaseIP(reg);
	return ret;
}

std::list<std::string> CodeGen::add64(const Register* to, const Register* l, long long imm)
{
	if (imm < 0) return sub64(to, l, -imm);
	if (imm == 0) return copy(to, l, 64);
	if (inUImm12(imm))
		return {instruction("ADD", regName(to, 64), regName(l, 64), immediate(imm))};
	if (inUImm12L12(imm))
		return {instruction("ADD", regName(to, 64), regName(l, 64), immediate(imm >> 12), leftShift12())};
	if (inUImm24(imm))
	{
		auto ret = add64(to, l, imm & 4095);
		merge(ret, add64(to, to, imm & 16773120));
		return ret;
	}
	auto reg = getIP();
	auto ret = makeI64Immediate(imm, reg);
	merge(ret, add(to, l, reg, 64));
	releaseIP(reg);
	return ret;
}

std::list<std::string> CodeGen::st1(const Register* stackLike, int count, int offset)
{
	if (count == 1) return str(floatRegister(0), stackLike, 128);
	string ret = "\tST1 {";
	for (int i = 0; i < count; i++)
		ret += "V" + to_string(i) + ".16B,";
	ret.pop_back();
	ret += "}, ";
	ret += regData(stackLike);
	if (offset != 0)
	{
		ret += ", " + immediate(offset);
	}
	return {ret};
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

std::list<std::string> CodeGen::sub64(const Register* to, const Register* l, long long imm)
{
	if (imm < 0) return add64(to, l, -imm);
	if (imm == 0) return copy(to, l, 64);
	if (inUImm12(imm))
		return {instruction("SUB", regName(to, 64), regName(l, 64), immediate(imm))};
	if (inUImm12L12(imm))
		return {instruction("SUB", regName(to, 64), regName(l, 64), immediate(imm >> 12), leftShift12())};
	if (inUImm24(imm))
	{
		auto ret = sub64(to, l, imm & 4095);
		merge(ret, sub64(to, to, imm & 16773120));
		return ret;
	}
	auto reg = getIP();
	auto ret = makeI64Immediate(imm, reg);
	merge(ret, sub(to, l, reg, 64));
	releaseIP(reg);
	return ret;
}

std::list<std::string> CodeGen::call(const MOperand* address)
{
	auto f = dynamic_cast<const FuncAddress*>(address);
	assert(f != nullptr);
	list<string> ret = {instruction("BL", f->function()->name())};
	if (func2Call_ != nullptr)
	{
		assert(f->function() == func2Call_);
		merge(ret, add64(sp(), sp(), func2Call_->fix_move_offset()));
		func2Call_ = nullptr;
	}
	return ret;
}

std::list<std::string> CodeGen::ret()
{
	auto ret = functionSuffix(func_);
	ret.emplace_back("\tRET");
	return ret;
}

std::list<std::string> CodeGen::bcc(const MOperand* to, Instruction::OpID cond)
{
	if (cond == Instruction::add)
		return branch(to);
	if (cond == Instruction::sub)
		return {};
	auto bb = dynamic_cast<const BlockAddress*>(to);
	assert(bb != nullptr);
	return {instruction("B." + condName(cond), bb->block()->name())};
}

std::list<std::string> CodeGen::branch(const MOperand* to)
{
	auto bb = dynamic_cast<const BlockAddress*>(to);
	assert(bb != nullptr);
	return {instruction("B", bb->block()->name())};
}

std::list<std::string> CodeGen::cset(const MOperand* to, Instruction::OpID cond)
{
	if (cond == Instruction::add)
		return makeI32Immediate(1, dynamic_cast<const Register*>(to));
	if (cond == Instruction::sub)
		return makeI32Immediate(0, dynamic_cast<const Register*>(to));
	auto bb = dynamic_cast<const Register*>(to);
	assert(bb != nullptr);
	return {instruction("CSET", regName(bb, 32), condName(cond))};
}


std::list<std::string> CodeGen::extend32To64(const MOperand* from, const MOperand* to)
{
	auto f = dynamic_cast<const Register*>(from);
	auto t = dynamic_cast<const Register*>(to);
	assert(t);
	if (auto imm = dynamic_cast<const Immediate*>(from); imm != nullptr)
	{
		long long i = imm->asInt();
		return makeI64Immediate(i, t);
	}
	assert(f);
	return {instruction("SXTW", regName(t, 64), regName(f, 32))};
}

std::list<std::string> CodeGen::f2i(const MOperand* from, const MOperand* to)
{
	auto f = dynamic_cast<const Register*>(from);
	auto t = dynamic_cast<const Register*>(to);
	assert(t);
	if (auto imm = dynamic_cast<const Immediate*>(from); imm != nullptr)
	{
		int i = static_cast<int>(imm->asFloat());
		return makeI32Immediate(i, t);
	}
	assert(f);
	return {instruction("FCVTZS", regName(t, 32), regName(f, 32))};
}

std::list<std::string> CodeGen::i2f(const MOperand* from, const MOperand* to)
{
	auto f = dynamic_cast<const Register*>(from);
	auto t = dynamic_cast<const Register*>(to);
	assert(t);
	if (auto imm = dynamic_cast<const Immediate*>(from); imm != nullptr)
	{
		float i = static_cast<float>(imm->asInt());
		return makeF32Immediate(i, t);
	}
	assert(f);
	return {instruction("SCVTF", regName(t, 32), regName(f, 32))};
}

std::list<std::string> CodeGen::compare(const MCMP* inst, const MOperand* l, const MOperand* r, bool flt)
{
	auto lm = dynamic_cast<const Immediate*>(l);
	auto rm = dynamic_cast<const Immediate*>(r);
	auto lr = dynamic_cast<const Register*>(l);
	auto rr = dynamic_cast<const Register*>(r);
	assert(lm != nullptr || lr != nullptr);
	assert(rm != nullptr || rr != nullptr);
	if (flt)
	{
		if (lm && rm)
		{
			bool b = lm->asFloat() > rm->asFloat();
			bool e = lm->asFloat() == rm->asFloat();
			if (b)
				decideCond(inst->tiedWith_, 1);
			else if (e)
				decideCond(inst->tiedWith_, 0);
			else decideCond(inst->tiedWith_, -1);
			return {};
		}
		if (lm && lm->isZero(true, 32))
		{
			reverseCmpOp(inst);
			return {instruction("FCMP", regName(rr, 32), immediate(0))};
		}
		if (rm && rm->isZero(true, 32))
			return {instruction("FCMP", regName(lr, 32), immediate(0))};
		list<string> ret;
		lr = op2reg(l, 32, false, ret);
		rr = op2reg(r, 32, false, ret);
		ret.emplace_back(instruction("FCMP", regName(lr, 32), regName(rr, 32)));
		releaseIP(lr);
		releaseIP(rr);
		return ret;
	}
	if (lm && rm)
	{
		bool b = lm->asInt() > rm->asInt();
		bool e = lm->asInt() == rm->asInt();
		if (b)
			decideCond(inst->tiedWith_, 1);
		else if (e)
			decideCond(inst->tiedWith_, 0);
		else decideCond(inst->tiedWith_, -1);
		return {};
	}
	if (lm && (inUImm12(lm->asInt()) || inUImm12L12(lm->asInt())))
	{
		reverseCmpOp(inst);
		return {instruction("CMP", regName(rr, 32), immediate(lm->asInt()))};
	}
	if (rm && (inUImm12(rm->asInt()) || inUImm12L12(rm->asInt())))
		return {instruction("CMP", regName(lr, 32), immediate(rm->asInt()))};
	list<string> ret;
	lr = op2reg(l, 32, true, ret);
	rr = op2reg(r, 32, true, ret);
	ret.emplace_back(instruction("CMP", regName(lr, 32), regName(rr, 32)));
	releaseIP(lr);
	releaseIP(rr);
	return ret;
}


void CodeGen::reverseCmpOp(const MCMP* inst)
{
	auto next = inst->tiedWith_;
	assert(next != nullptr);
	auto cset = dynamic_cast<MCSET*>(next);
	if (cset != nullptr)
	{
		cset->op_ = lrShiftOp(cset->op_);
		return;
	}
	auto bcc = dynamic_cast<MBcc*>(next);
	if (bcc != nullptr)
	{
		bcc->op_ = lrShiftOp(bcc->op_);
		return;
	}
	assert(false);
}

std::list<std::string> CodeGen::sub32(const Register* to, const Register* l, int imm)
{
	if (imm < 0) return add32(to, l, -imm);
	if (imm == 0) return copy(to, l, 32);
	if (inUImm12(imm))
		return {instruction("SUB", regName(to, 32), regName(l, 32), immediate(imm))};
	if (inUImm12L12(imm))
		return {instruction("SUB", regName(to, 32), regName(l, 32), immediate(imm >> 12), leftShift12())};
	if (inUImm24(imm))
	{
		auto ret = sub32(to, l, imm & 4095);
		merge(ret, sub32(to, to, imm & 16773120));
		return ret;
	}
	auto reg = getIP();
	auto ret = makeI32Immediate(imm, reg);
	merge(ret, sub(to, l, reg, 32));
	releaseIP(reg);
	return ret;
}

std::list<std::string> CodeGen::sub(const Register* to, const Register* l, const Register* r, int len)
{
	return {instruction("SUB", regName(to, len), regName(l, len), regName(r, len))};
}

std::list<std::string> CodeGen::mathRRInst(const Register* to, const Register* l, const Register* r,
                                           Instruction::OpID op, int len)
{
	switch (op) // NOLINT(clang-diagnostic-switch-enum)
	{
		case Instruction::add: return add(to, l, r, len);
		case Instruction::sub: return sub(to, l, r, len);
		case Instruction::mul: return {instruction("MUL", regName(to, len), regName(l, len), regName(r, len))};
		case Instruction::sdiv: return {instruction("SDIV", regName(to, len), regName(l, len), regName(r, len))};
		case Instruction::srem:
			{
				auto ip = getIP();
				list ret = {
					instruction("SDIV", regName(ip, len), regName(l, len),
					            regName(r, len))
				};
				ret.emplace_back(instruction("MSUB", regName(to, len), regName(ip, len),
				                             regName(r, len),
				                             regName(l, len)));
				releaseIP(ip);
				return ret;
			}
		default: break;
	}
	throw runtime_error("unexpected");
}

std::list<std::string> CodeGen::fsub(const Register* to, const Register* l, const Register* r)
{
	return {instruction("FSUB", regName(to, 32), regName(l, 32), regName(r, 32))};
}

std::list<std::string> CodeGen::copy(const Register* to, const Register* from, int len)
{
	if (to == from) return {};
	if (to->isIntegerRegister() && from->isIntegerRegister())
		return {instruction("MOV", regName(to, len), regName(from, len))};
	assert(len == 32);
	return {instruction("FMOV", regName(to, len), regName(from, len))};
}

std::list<std::string> CodeGen::copy(const Register* to, const Immediate* from, int len)
{
	if (to->isIntegerRegister())
	{
		if (len == 32) return makeI32Immediate(from->asInt(), to);
		return makeI64Immediate(from->as64BitsInt(), to);
	}
	return makeF32Immediate(from->asFloat(), to);
}

std::list<std::string> CodeGen::copy(const MOperand* to, const MOperand* from, int len)
{
	assert(len == 32 || len == 64);
	auto tor = dynamic_cast<const Register*>(to);
	assert(tor);
	if (const Register* fromr = dynamic_cast<const Register*>(from); fromr != nullptr)
		return copy(tor, fromr, len);
	if (const Immediate* fromr = dynamic_cast<const Immediate*>(from); fromr != nullptr)
		return copy(tor, fromr, len);
	if (const GlobalAddress* fromr = dynamic_cast<const GlobalAddress*>(from); fromr != nullptr)
		return copy(tor, fromr, len);
	if (const FrameIndex* fromr = dynamic_cast<const FrameIndex*>(from); fromr != nullptr)
		return copy(tor, fromr, len);
	throw runtime_error("unexpected");
}

std::list<std::string> CodeGen::copy(const Register* to, const GlobalAddress* from, int len)
{
	assert(len == 64);
	return {instruction("LDR", regName(to, 64), "=" + from->name_)};
}

std::list<std::string> CodeGen::copy(const Register* to, const FrameIndex* from, int len)
{
	list<string> ret;
	auto offset = frameOffset(from, false, ret);
	merge(ret, add64(to, sp(), offset));
	return ret;
}

std::list<std::string> CodeGen::makeInstruction(MInstruction* instruction)
{
	list<string> l;
	RUN(l.emplace_back(color::green("# " + instruction->print())));
	if (auto i = dynamic_cast<MCopy*>(instruction); i != nullptr)
		merge(l, copy(instruction->operands()[1], instruction->operands()[0], (i->copy_len())));
	else if (auto i2 = dynamic_cast<MSTR*>(instruction); i2 != nullptr)
		merge(l, str(instruction->operands()[0], instruction->operands()[1], (i2->width())));
	else if (auto i3 = dynamic_cast<MLDR*>(instruction); i3 != nullptr)
		merge(l, ldr(instruction->operands()[0], instruction->operands()[1], (i3->width())));
	else if (auto i4 = dynamic_cast<MST1V16B*>(instruction); i4 != nullptr)
		merge(l, st1(instruction->operands()[0], i4->storeCount_, i4->offset_));
	else if (auto i5 = dynamic_cast<MLD1V16B*>(instruction); i5 != nullptr)
		merge(l, ld1(instruction->operands()[0], i5->loadCount_, i5->offset_));
	else if (auto i6 = dynamic_cast<MST1ZTV16B*>(instruction); i6 != nullptr)
		merge(l, clearV(i6->loadCount_));
	else if (auto i7 = dynamic_cast<MMathInst*>(instruction); i7 != nullptr)
		merge(l, mathInst(i7, instruction->operands()[0], instruction->operands()[1], instruction->operands()[2],
		                  i7->op(),
		                  (i7->width())));
	else if (auto i8 = dynamic_cast<MBL*>(instruction); i8 != nullptr)
		merge(l, call(instruction->operands()[0]));
	else if (auto i9 = dynamic_cast<MRet*>(instruction); i9 != nullptr)
		merge(l, ret());
	else if (auto i10 = dynamic_cast<MBcc*>(instruction); i10 != nullptr)
		merge(l, bcc(i10->operands()[0], i10->op()));
	else if (auto i11 = dynamic_cast<MB*>(instruction); i11 != nullptr)
		merge(l, branch(i11->operands()[0]));
	else if (auto i12 = dynamic_cast<MCMP*>(instruction); i12 != nullptr)
		merge(l, compare(i12, i12->operands()[0], i12->operands()[1], !i12->itff_));
	else if (auto i13 = dynamic_cast<MCSET*>(instruction); i13 != nullptr)
		merge(l, cset(i13->operands()[0], i13->op_));
	else if (auto i14 = dynamic_cast<MFCVTZS*>(instruction); i14 != nullptr)
		merge(l, f2i(i14->operands()[1], i14->operands()[0]));
	else if (auto i15 = dynamic_cast<MSCVTF*>(instruction); i15 != nullptr)
		merge(l, i2f(i15->operands()[1], i15->operands()[0]));
	else if (auto i16 = dynamic_cast<MSXTW*>(instruction); i16 != nullptr)
		merge(l, extend32To64(i16->operands()[0], i16->operands()[1]));
	else
		assert(false);
	return l;
}

std::string CodeGen::instruction(const std::string& name, const std::string& op1, const std::string& op2,
                                 const std::string& op3)
{
	return "\t" + name + " " + op1 + ", " + op2 + ", " + op3;
}

std::string CodeGen::instruction(const std::string& name, const std::string& op1, const std::string& op2,
                                 const std::string& op3, const std::string& op4)
{
	return "\t" + name + " " + op1 + ", " + op2 + ", " + op3 + ", " + op4;
}

std::string CodeGen::instruction(const std::string& name, const std::string& op1, const std::string& op2)
{
	return "\t" + name + " " + op1 + ", " + op2;
}

std::string CodeGen::instruction(const std::string& name, const std::string& op1)
{
	return "\t" + name + " " + op1;
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
	auto b = dynamic_cast<MBcc*>(instruction);
	auto cset = dynamic_cast<MCSET*>(instruction);
	if (b != nullptr) op = b->op_;
	else
	{
		assert(cset);
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
	else b->op_ = op;
}


std::string CodeGen::poolImmediate(long long i)
{
	return "=" + to_string(i);
}

Register* CodeGen::zeroRegister() const
{
	return Register::getZERO(m_);
}


std::list<std::string> CodeGen::makeI64Immediate(long long i, const Register* placeIn)
{
	if (i == 0) return copy(placeIn, zeroRegister(), 32);
	if (useZRRegisterAsCommonRegister)
	{
		if (i > 0)
		{
			if (inUImm12(i) || inUImm12L12(i)) return add32(placeIn, zeroRegister(), static_cast<int>(i));
		}
		else if (inUImm12(-i) || inUImm12L12(-i)) return sub32(placeIn, zeroRegister(), static_cast<int>(-i));
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
		return {instruction("LDR", regName(placeIn, 64), poolImmediate(i))};
	}
	list<string> l;
	if (uz)
	{
		l.emplace_back(instruction("MOVZ", regName(placeIn, 64), immediate(a[3])));
		for (int t = 2; t > -1; t--)
		{
			if (a[t] != 0u)
				l.emplace_back(instruction("MOVK", regName(placeIn, 64),
				                           immediate(a[t]), leftShift(offset[t])));
		}
	}
	else
	{
		l.emplace_back(instruction("MOVN", regName(placeIn, 64), immediate((~a[3]) & 65535u)));
		for (int t = 2; t > -1; t--)
		{
			if (a[t] != 65535u)
				l.emplace_back(instruction("MOVK", regName(placeIn, 64),
				                           immediate(a[t]), leftShift(offset[t])));
		}
	}
	return l;
}

std::list<std::string> CodeGen::makeI32Immediate(int i, const Register* placeIn)
{
	if (i == 0) return copy(placeIn, zeroRegister(), 32);
	if (useZRRegisterAsCommonRegister)
	{
		if (i > 0)
		{
			if (inUImm12(i) || inUImm12L12(i)) return add32(placeIn, zeroRegister(), i);
		}
		else
		{
			if (inUImm12(-i) || inUImm12L12(-i)) return sub32(placeIn, zeroRegister(), -i);
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
		return {instruction("LDR", regName(placeIn, 32), poolImmediate(i))};
	}
	list<string> l;
	if (uz)
	{
		l.emplace_back(instruction("MOVZ", regName(placeIn, 32), immediate(a[1])));
		if (a[0] != 0u)
			l.emplace_back(instruction("MOVK", regName(placeIn, 32),
			                           immediate(a[0]), leftShift(offset[0])));
	}
	else
	{
		l.emplace_back(instruction("MOVN", regName(placeIn, 32), immediate(~a[1] & 65535u)));
		if (a[0] != 65535u)
			l.emplace_back(instruction("MOVK", regName(placeIn, 32),
			                           immediate(a[0]), leftShift(offset[0])));
	}
	return l;
}

std::list<std::string> CodeGen::makeF32Immediate(float i, const Register* placeIn)
{
	unsigned u = 0;
	memcpy(&u, &i, sizeof(float));
	if (floatMovSupport(u))
		return {instruction("FMOV", regName(placeIn, 32), fimmediate(i))};
	unsigned a[2] = {
		((u >> 16u) & 65535u), (u & 65535u)
	};

	int cu = u2iKeepBits(u);

	if (useZRRegisterAsCommonRegister)
	{
		if (cu > 0)
		{
			if (inUImm12(cu) || inUImm12L12(cu)) return add32(placeIn, zeroRegister(), cu);
		}
		else
		{
			if (inUImm12(-cu) || inUImm12L12(-cu)) return sub32(placeIn, zeroRegister(), -cu);
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
		return {instruction("LDR", regName(placeIn, 32), poolImmediate(u))};
	}
	auto reg = getIP();
	list<string> l;
	if (uz)
	{
		l.emplace_back(instruction("MOVZ", regName(reg, 32), immediate(a[1])));
		if (a[0] != 0u)
			l.emplace_back(instruction("MOVK", regName(reg, 32),
			                           immediate(a[0]), leftShift(offset[0])));
	}
	else
	{
		l.emplace_back(instruction("MOVN", regName(reg, 32), immediate(~a[1] & 65535u)));
		if (a[0] != 65535u)
			l.emplace_back(instruction("MOVK", regName(reg, 32),
			                           immediate(a[0]), leftShift(offset[0])));
	}
	merge(l, copy(placeIn, reg, 32));
	releaseIP(reg);
	return l;
}

std::list<std::string> CodeGen::makeImmediate(const Immediate* i, bool flt, int len, const Register* placeIn)
{
	assert(len == 32 || len == 64);
	if (flt)
	{
		assert(len == 32);
		assert(placeIn->isIntegerRegister() == false);
		return makeF32Immediate(i->asFloat(), placeIn);
	}
	assert(placeIn->isIntegerRegister());
	return len == 32 ? makeI32Immediate(i->asInt(), placeIn) : makeI64Immediate(i->as64BitsInt(), placeIn);
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

void CodeGen::merge(std::list<std::string>& l, const std::list<std::string>& r)
{
	for (const auto& basic_string : r) l.emplace_back(basic_string);
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
long long CodeGen::frameOffset(const FrameIndex* index, bool isStore, list<string>& appendSlot)
{
	if (func2Call_)
	{
		if (!index->stack_t_fix_f() && isStore)
		{
			assert(index->func() == func2Call_);
			return index->offset() - index->func()->stack_move_offset();
		}
		assert(index->func() == func_);
		return index->offset() + func2Call_->fix_move_offset();
	}
	if (!index->stack_t_fix_f() && isStore)
	{
		merge(appendSlot, sub64(sp(), sp(), index->func()->fix_move_offset()));
		func2Call_ = index->func();
		return frameOffset(index, isStore, appendSlot);
	}
	assert(index->func() == func_);
	return index->offset();
}

Register* CodeGen::sp() const
{
	return Register::getSP(m_);
}

Register* CodeGen::getIP()
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
	return nullptr;
}

Register* CodeGen::getIPOfType(bool flt)
{
	return flt ? getFIP() : getIP();
}

Register* CodeGen::getFIP()
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
	assert(op);
	assert(!opbuffer[id]);
	opbuffer[id] = op;
}

const MOperand* CodeGen::getBuf(int id) const
{
	assert(opbuffer[id]);
	auto ret = opbuffer[id];
	return ret;
}

void CodeGen::releaseBuf(int id)
{
	assert(opbuffer[id]);
	opbuffer[id] = nullptr;
}

CodeGen::CodeGen(MModule* m): m_(m)
{
}

void CodeGen::run()
{
	if (const auto globalConstants = m_->constGlobalAddresses(); !globalConstants.empty())
	{
		append(section("section .rodata, \"a\", @progbits"));
		for (auto& glob : globalConstants)
			append(makeGlobal(glob));
	}
	if (const auto globalZeros = m_->ncZeroGlobalAddresses(); !globalZeros.empty())
	{
		append(section("bss"));
		for (auto& glob : globalZeros)
			append(makeGlobal(glob));
	}
	if (const auto globals = m_->ncnzGlobalAddresses(); !globals.empty())
	{
		append(section("data"));
		for (auto& glob : globals)
			append(makeGlobal(glob));
	}
	append(section("text"));
	append(align(4, false));
	for (auto& f : m_->functions())
		f->rankFrameIndexesAndCalculateOffsets();
	bool haveMemcpy = false;
	bool haveMemclr = false;
	int cpid = m_->memcpy_->id();
	int clid = m_->memclr_->id();
	for (auto& f : m_->functions())
	{
		func_ = f;
		append(makeFunction(f));
		if (!haveMemcpy && f->called().test(cpid)) haveMemcpy = true;
		if (!haveMemclr && f->called().test(clid)) haveMemclr = true;
	}
	if (haveMemcpy) append(genMemcpy());
	if (haveMemclr) append(genMemclr());
}

std::string CodeGen::print() const
{
	string ret;
	for (auto& i : txt_)
	{
		ret += i;
		ret += '\n';
	}
	return ret;
}

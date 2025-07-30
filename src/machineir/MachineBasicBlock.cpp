#include "MachineBasicBlock.hpp"

#include <algorithm>
#include <stdexcept>

#include "BasicBlock.hpp"
#include "Config.hpp"
#include "Constant.hpp"
#include "Instruction.hpp"
#include "MachineIR.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include "Type.hpp"

using namespace std;


namespace
{
	Instruction::OpID asCmpGetOp(const Instruction* instruction)
	{
		auto ty = instruction->get_instr_type();
		if (instruction->is_fcmp()) return static_cast<Instruction::OpID>(ty - 6);
		return ty;
	}

	Instruction::OpID reverseOp(Instruction::OpID op)
	{
		// ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
		// ReSharper disable once CppIncompleteSwitchStatement
		switch (op) // NOLINT(clang-diagnostic-switch)
		{
			case Instruction::ge: return Instruction::lt;
			case Instruction::gt: return Instruction::le;
			case Instruction::le: return Instruction::gt;
			case Instruction::lt: return Instruction::ge;
			case Instruction::eq: return Instruction::ne;
			case Instruction::ne: return Instruction::eq;
		}
		throw runtime_error("invalid op");
	}
}


MBasicBlock::MBasicBlock(std::string name, MFunction* function) : function_(function), name_(std::move(name))
{
}

MBasicBlock::~MBasicBlock()
{
	for (auto i : instructions_) delete i;
}

MBasicBlock* MBasicBlock::createBasicBlock(const std::string& name, MFunction* function)
{
	return new MBasicBlock{name, function};
}

void MBasicBlock::accept(BasicBlock* block,
                         map<Value*, MOperand*>& opMap, std::map<BasicBlock*, MBasicBlock*>& blockMap,
                         std::map<MBasicBlock*, list<MCopy*>>& phiMap, std::map<Function*, MFunction*>& funcMap)
{
	for (auto inst : block->get_instructions())
	{
		switch (inst->get_instr_type())
		{
			case Instruction::ret:
				acceptReturnInst(inst, opMap, this);
				break;
			case Instruction::br:
				acceptBranchInst(inst, blockMap, this);
				break;
			case Instruction::add:
			case Instruction::sub:
			case Instruction::mul:
			case Instruction::sdiv:
			case Instruction::srem:
			case Instruction::fadd:
			case Instruction::fsub:
			case Instruction::fmul:
			case Instruction::fdiv:
				acceptMathInst(inst, opMap, this);
				break;
			case Instruction::alloca_:
				break;
			case Instruction::load:
				acceptLoadInst(inst, opMap, this);
				break;
			case Instruction::store:
				acceptStoreInst(inst, opMap, this);
				break;
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
				acceptCmpInst(inst, opMap, this);
				break;
			case Instruction::phi:
				acceptPhiInst(inst, opMap, blockMap, phiMap, this);
				break;
			case Instruction::call:
				acceptCallInst(inst, opMap, funcMap, this);
				break;
			case Instruction::getelementptr:
				acceptGetElementPtrInst(inst, opMap, this);
				break;
			case Instruction::zext:
				acceptZextInst(inst, opMap, this);
				break;
			case Instruction::fptosi:
				acceptFpToSiInst(inst, opMap, this);
				break;
			case Instruction::sitofp:
				acceptSiToFpInst(inst, opMap, this);
				break;
			case Instruction::memcpy_:
				acceptMemCpyInst(inst, opMap, this);
				break;
			case Instruction::memclear_:
				acceptMemClearInst(inst, opMap, this);
				break;
			case Instruction::nump2charp:
			case Instruction::global_fix:
				acceptCopyInst(inst, opMap, this);
				break;
		}
	}
}

void MBasicBlock::acceptReturnInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto retu = dynamic_cast<ReturnInst*>(instruction);
	if (!retu->is_void_ret())
	{
		auto reg = block->function()->getOperandFor(instruction->get_operand(0), opMap);
		auto copy = new MCopy{
			block, reg, Register::getParameterRegisterWithType(0, instruction->get_operand(0)->get_type(), module()), 32
		};
		instructions_.emplace_back(copy);
	}
	auto lrc = new MCopy{block, function_->lr_guard(), Register::getLR(module()), 64};
	instructions_.emplace_back(lrc);
	auto ret = new MRet{block, instruction->get_parent()->get_parent()};
	instructions_.emplace_back(ret);
}

void MBasicBlock::acceptBranchInst(Instruction* instruction, std::map<BasicBlock*, MBasicBlock*>& bmap,
                                   MBasicBlock* block)
{
	auto br = dynamic_cast<BranchInst*>(instruction);
	if (br->is_cond_br())
	{
		auto t = BlockAddress::get(bmap[dynamic_cast<BasicBlock*>(br->get_operand(1))]);
		auto f = BlockAddress::get(bmap[dynamic_cast<BasicBlock*>(br->get_operand(2))]);
		auto op0 = dynamic_cast<Instruction*>(br->get_operand(0));
		auto op = asCmpGetOp(op0);
		if (f->block() == next_)
		{
			auto cmp = new MBcc{block, op, t};
			instructions_.emplace_back(cmp);
		}
		else if (t->block() == next_)
		{
			auto cmp = new MBcc{block, reverseOp(op), f};
			instructions_.emplace_back(cmp);
		}
		else
		{
			auto cmp = new MBcc{block, op, t};
			auto cmp1 = new MB{block, f};
			instructions_.emplace_back(cmp);
			instructions_.emplace_back(cmp1);
		}
	}
	else
	{
		auto t = BlockAddress::get(bmap[dynamic_cast<BasicBlock*>(br->get_operand(0))]);
		if (t->block() != next_)
		{
			auto cmp = new MB{block, t};
			instructions_.emplace_back(cmp);
		}
	}
}

void MBasicBlock::acceptMathInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto l0 = instruction->get_operand(0);
	auto r0 = instruction->get_operand(1);
	auto l = block->function()->getOperandFor(l0, opMap);
	auto r = block->function()->getOperandFor(r0, opMap);
	auto t = block->function()->getOperandFor(instruction, opMap);
	auto op = instruction->get_instr_type();
	auto ret = new MMathInst{block, op, l, r, t, 32};
	instructions_.emplace_back(ret);
}

namespace
{
	struct FrameCond
	{
		FrameIndex* frame_index_;
		AllocaInst* inst_;
		float weight_;
		bool cache_;
	};
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void MBasicBlock::acceptAllocaInsts(BasicBlock* block, std::map<Value*, MOperand*>& opMap,
                                    std::map<BasicBlock*, MBasicBlock*>& bmap)
{
	vector<FrameCond> vec;
	auto func = block->get_parent();
	for (auto inst : block->get_instructions().phi_and_allocas())
	{
		auto alloc = dynamic_cast<AllocaInst*>(inst);
		if (alloc != nullptr)
		{
			auto cost = func->opWeight(alloc, bmap);
			auto fi = function()->allocaStack(alloc);
			vec.emplace_back(FrameCond{fi, alloc, cost, false});
		}
	}
	for (auto& i : vec)
		if (i.weight_ >= static_cast<float>(replaceAllocaAddressWithRegisterNeedUseCount))
			i.cache_ = true;
	for (auto& i : vec)
	{
		if (i.cache_)
		{
			auto reg = VirtualRegister::createVirtualIRegister(function_, 64);
			reg->replacePrefer_ = i.frame_index_;
			auto inst = new MCopy{this, i.frame_index_, reg, 64};
			instructions_.emplace_back(inst);
			opMap[i.inst_] = reg;
		}
		else
			opMap[i.inst_] = i.frame_index_;
	}
}

void MBasicBlock::acceptLoadInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto regLike = block->function()->getOperandFor(instruction, opMap);
	auto stackLike = block->function()->getOperandFor(instruction->get_operand(0), opMap);
	auto ret = new MLDR{block, regLike, stackLike, 32};
	instructions_.emplace_back(ret);
}

void MBasicBlock::acceptStoreInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap,
                                  MBasicBlock* block)
{
	auto regLike = block->function()->getOperandFor(instruction->get_operand(0), opMap);
	auto stackLike = block->function()->getOperandFor(instruction->get_operand(1), opMap);
	auto ret = new MSTR{block, regLike, stackLike, 32};
	instructions_.emplace_back(ret);
}

void MBasicBlock::acceptCmpInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto l = block->function()->getOperandFor(instruction->get_operand(0), opMap);
	auto r = block->function()->getOperandFor(instruction->get_operand(1), opMap);
	auto ret = new MCMP{block, l, r, instruction->get_operand(0)->get_type() != Types::FLOAT};
	instructions_.emplace_back(ret);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void MBasicBlock::acceptPhiInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap,
                                std::map<BasicBlock*, MBasicBlock*>& bmap,
                                std::map<MBasicBlock*, std::list<MCopy*>>& phiMap, const MBasicBlock* block)
{
	auto phi = dynamic_cast<PhiInst*>(instruction);
	auto pairs = phi->get_phi_pairs();
	for (auto& [val, bb] : pairs)
	{
		auto mbb = bmap[bb];
		auto cp = new MCopy{
			mbb, block->function()->getOperandFor(val, opMap), block->function()->getOperandFor(phi, opMap),
			val->get_type()->sizeInBitsInArm64()
		};
		phiMap[mbb].emplace_back(cp);
	}
}


void MBasicBlock::acceptCallInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap,
                                 std::map<Function*, MFunction*>& funcMap, MBasicBlock* block)
{
	auto func = dynamic_cast<Function*>(instruction->get_operand(0));
	auto mfunc = funcMap[func];
	int ic = 0;
	int fc = 0;
	int nc = 0;
	int idx = 0;
	for (auto& arg : func->get_args())
	{
		++idx;
		if (arg.get_type() == Types::FLOAT)
		{
			if (fc < 8)
			{
				auto cp = new MCopy{
					block, function()->getOperandFor(instruction->get_operand(idx), opMap),
					Register::getFParameterRegister(fc, module()), 32
				};
				instructions_.emplace_back(cp);
				++fc;
				continue;
			}
		}
		else
		{
			if (ic < 8)
			{
				auto cp = new MCopy{
					block, function()->getOperandFor(instruction->get_operand(idx), opMap),
					Register::getIParameterRegister(ic, module()), arg.get_type()->sizeInBitsInArm64()
				};
				instructions_.emplace_back(cp);
				++ic;
				continue;
			}
		}
		auto st = new MSTR{
			block, function()->getOperandFor(instruction->get_operand(idx), opMap),
			mfunc->getFix(nc), arg.get_type()->sizeInBitsInArm64()
		};
		instructions_.emplace_back(st);
		++nc;
	}
	auto bl = new MBL{block, FuncAddress::get(mfunc), func};
	instructions_.emplace_back(bl);
	if (instruction->get_type() != Types::VOID)
	{
		opMap[instruction] = Register::getParameterRegisterWithType(0, instruction->get_type(), module());
	}
	block->function()->called().set(mfunc->id());
}

void MBasicBlock::acceptGetElementPtrInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap,
                                          MBasicBlock* block)
{
	auto gep = dynamic_cast<GetElementPtrInst*>(instruction);
	auto type = gep->get_operand(0)->get_type()->toPointerType();
	auto& ops = gep->get_operands();
	auto idxs = vector{ops.begin() + 1, ops.end()};
	auto typeVal = type->typeContained();
	vector<unsigned> dimLen;
	if (!typeVal->isArrayType())
		dimLen.emplace_back(1);
	else
	{
		auto ar = typeVal->toArrayType();
		dimLen = ar->dimensions();
		dimLen.emplace_back(1);
	}
	int dimCount = static_cast<int>(instruction->get_num_operand()) - 1;
	auto frameIndex = this->function()->getOperandFor(gep->get_operand(0), opMap);
	auto immOperand = Immediate::getImmediate(0, module());
	MOperand* operand = immOperand;
	Immediate* immB = nullptr;
	Immediate* immA = nullptr;
	unsigned width = use64BitsMathOperationInPointerOp ? 64 : 32;
	// A * op + B
	for (int i = 0; i < dimCount; i++)
	{
		long long len = dimLen[i];
		if (len == 0)
		{
			immOperand = Immediate::getImmediate(0, module());
			operand = immOperand;
			immB = nullptr;
			immA = nullptr;
			continue;
		}
		if (i == dimCount - 1)
		{
			len <<= 2;
			int rs = static_cast<int>(dimLen.size()) - 1;
			for (int j = i + 1; j < rs; j++)
			{
				len *= dimLen[j];
			}
		}
		auto op = gep->get_operand(i + 1);
		auto mop = function()->getOperandFor(op, opMap);
		auto immMop = dynamic_cast<Immediate*>(mop);
		if (use64BitsMathOperationInPointerOp && immMop == nullptr)
		{
			auto vr = dynamic_cast<VirtualRegister*>(mop);
			assert(vr != nullptr);
			if (vr->size() != 64)
			{
				auto r = VirtualRegister::createVirtualIRegister(function_, 64);
				auto exp = new MSXTW{block, vr, r};
				instructions_.emplace_back(exp);
				mop = r;
			}
		}
		if (immOperand != nullptr)
		{
			if (immMop != nullptr)
			{
				// C + D -> (C + D)
				if (immMop->as64BitsInt() != 0)
				{
					immOperand = Immediate::getImmediate(immOperand->as64BitsInt() + immMop->as64BitsInt(), module());
					operand = immOperand;
				}
			}
			else
			{
				// C + X -> X + C
				immB = immOperand;
				immOperand = nullptr;
				operand = mop;
			}
		}
		else
		{
			if (immMop != nullptr)
			{
				// Ax + B + C -> Ax + (B + C)
				if (immMop->as64BitsInt() != 0)
				{
					if (immB == nullptr) immB = immMop;
					else immB = Immediate::getImmediate(immB->as64BitsInt() + immMop->as64BitsInt(), module());
				}
			}
			else
			{
				// Ax + B + y -> (Ax + y) + B
				if (immA != nullptr)
				{
					auto reg = VirtualRegister::createVirtualIRegister(function(), static_cast<short>(width));
					auto inst = new MMathInst{block, Instruction::mul, immA, operand, reg, width};
					instructions_.emplace_back(inst);
					operand = reg;
					immA = nullptr;
				}
				auto reg2 = VirtualRegister::createVirtualIRegister(function(), static_cast<short>(width));
				auto inst = new MMathInst{block, Instruction::add, operand, immMop, reg2, width};
				instructions_.emplace_back(inst);
				operand = reg2;
			}
		}
		if (len == 1) continue;
		if (immOperand != nullptr)
		{
			immOperand = Immediate::getImmediate(immOperand->as64BitsInt() * len, module());
		}
		else
		{
			if (immA == nullptr) immA = Immediate::getImmediate(len, module());
			else immA = Immediate::getImmediate(immA->as64BitsInt() * len, module());
			if (immB != nullptr) immB = Immediate::getImmediate(immB->as64BitsInt() * len, module());
		}
	}
	auto targetReg = function()->getOperandFor(instruction, opMap);
	if (immOperand != nullptr)
	{
		if (immOperand->as64BitsInt() == 0)
		{
			function()->replaceAllOperands(targetReg, frameIndex);
			opMap[instruction] = frameIndex;
		}
		else
		{
			auto inst = new MMathInst{block, Instruction::add, frameIndex, immOperand, targetReg, 64};
			instructions_.emplace_back(inst);
		}
	}
	else
	{
		if (immA != nullptr)
		{
			auto reg = VirtualRegister::createVirtualIRegister(function(), static_cast<short>(width));
			auto inst = new MMathInst{block, Instruction::mul, immA, operand, reg, width};
			instructions_.emplace_back(inst);
			operand = reg;
		}
		if (immB != nullptr)
		{
			auto reg = VirtualRegister::createVirtualIRegister(function(), static_cast<short>(width));
			auto inst = new MMathInst{block, Instruction::add, immB, operand, reg, width};
			instructions_.emplace_back(inst);
			operand = reg;
		}
		auto vop = dynamic_cast<VirtualRegister*>(operand);
		assert(vop != nullptr);
		if (vop->size() < 64)
		{
			auto reg = VirtualRegister::createVirtualIRegister(function(), 64);
			auto exp = new MSXTW{block, operand, reg};
			instructions_.emplace_back(exp);
			operand = reg;
		}
		auto inst = new MMathInst{block, Instruction::add, frameIndex, operand, targetReg, 64};
		instructions_.emplace_back(inst);
	}
}

void MBasicBlock::mergePhiCopies(const std::list<MCopy*>& copies)
{
	if (instructions_.empty())
	{
		for (auto& cp : copies)
			instructions_.emplace_back(cp);
		return;
	}
	if (dynamic_cast<MBcc*>(instructions_.back()) != nullptr || dynamic_cast<MB*>(instructions_.back()) != nullptr)
	{
		auto bcc = instructions_.back();
		instructions_.pop_back();
		for (auto& cp : copies)
			instructions_.emplace_back(cp);
		instructions_.emplace_back(bcc);
		return;
	}
	for (auto& cp : copies)
		instructions_.emplace_back(cp);
}

void MBasicBlock::acceptZextInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto op0 = dynamic_cast<Instruction*>(instruction->get_operand(0));
	auto op = asCmpGetOp(op0);
	auto t = function()->getOperandFor(instruction, opMap);
	auto cmp = new MCSET{block, op, t};
	instructions_.emplace_back(cmp);
}

void MBasicBlock::acceptFpToSiInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto fp = function()->getOperandFor(instruction->get_operand(0), opMap);
	auto si = function()->getOperandFor(instruction, opMap);
	instructions_.emplace_back(new MFCVTZS{block, fp, si});
}

void MBasicBlock::acceptSiToFpInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto si = function()->getOperandFor(instruction->get_operand(0), opMap);
	auto fp = function()->getOperandFor(instruction, opMap);
	instructions_.emplace_back(new MSCVTF{block, si, fp});
}

void MBasicBlock::acceptMemCpyInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto mcp = dynamic_cast<MemCpyInst*>(instruction);
	auto count = mcp->get_copy_bytes() >> 4;
	auto srcF = function()->getOperandFor(instruction->get_operand(0), opMap);
	auto desF = function()->getOperandFor(instruction->get_operand(1), opMap);
	if (count <= maxCopyInstCountToInlineMemcpy)
	{
		auto src = Register::getIIP0(module());
		auto des = Register::getIIP1(module());
		auto cp0 = new MCopy{block, srcF, src, 64};
		auto cp1 = new MCopy{block, desF, des, 64};
		instructions_.emplace_back(cp0);
		instructions_.emplace_back(cp1);
		while (count >= 4)
		{
			auto load = new MLD1V16B{block, src, 4, 0, false};
			auto store = new MST1V16B{block, des, 4, count > 4 ? 64 : 0, count > 4};
			instructions_.emplace_back(load);
			instructions_.emplace_back(store);
			count -= 4;
		}
		if (count > 0)
		{
			auto load = new MLD1V16B{block, src, count, 0, false};
			auto store = new MST1V16B{block, des, count, 0, false};
			instructions_.emplace_back(load);
			instructions_.emplace_back(store);
		}
	}
	else
	{
		auto cr0 = new MCopy{block, desF, Register::getIParameterRegister(0, module()), 64};
		auto cr1 = new MCopy{block, srcF, Register::getIParameterRegister(1, module()), 64};
		auto cr2 = new MCopy{
			block, Immediate::getImmediate(count, module()),
			Register::getIParameterRegister(2, module()), 64
		};
		auto bl = new MBL{block, FuncAddress::get(module()->memcpyFunc()), true};
		instructions_.emplace_back(cr0);
		instructions_.emplace_back(cr1);
		instructions_.emplace_back(cr2);
		instructions_.emplace_back(bl);
		block->function()->called().set(module()->memcpyFunc()->id());
	}
}

void MBasicBlock::acceptMemClearInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto mcp = dynamic_cast<MemClearInst*>(instruction);
	auto count = mcp->get_clear_bytes() >> 4;
	auto desF = function()->getOperandFor(instruction->get_operand(0), opMap);
	if (count <= maxCopyInstCountToInlineMemclr)
	{
		instructions_.emplace_back(new MLD1RV16B{block, Register::getZERO(module()), count > 4 ? 4 : count});
		auto des = Register::getIIP0(module());
		auto cp0 = new MCopy{block, desF, des, 64};
		instructions_.emplace_back(cp0);
		while (count >= 4)
		{
			auto store = new MST1V16B{block, des, 4, count > 4 ? 64 : 0, count > 4};
			instructions_.emplace_back(store);
			count -= 4;
		}
		if (count > 0)
		{
			auto store = new MST1V16B{block, des, count, 0, false};
			instructions_.emplace_back(store);
		}
	}
	else
	{
		auto cr0 = new MCopy{block, desF, Register::getIParameterRegister(0, module()), 64};
		auto cr1 = new MCopy{
			block, Immediate::getImmediate(count, module()),
			Register::getIParameterRegister(1, module()), 64
		};
		auto bl = new MBL{block, FuncAddress::get(module()->memclrFunc()), false};
		instructions_.emplace_back(cr0);
		instructions_.emplace_back(cr1);
		instructions_.emplace_back(bl);
		block->function()->called().set(module()->memclrFunc()->id());
	}
}

void MBasicBlock::acceptCopyInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block) const
{
	auto t = function()->getOperandFor(instruction->get_operand(0), opMap);
	auto f = function()->getOperandFor(instruction, opMap);
	function()->replaceAllOperands(f, t);
	opMap[instruction] = t;
}

MModule* MBasicBlock::module() const
{
	return function_->module();
}

MFunction* MBasicBlock::function() const
{
	return function_;
}

std::string MBasicBlock::print(unsigned& sid) const
{
	string ret = name_ + "[" + to_string(id_) + "]:\t\t\t";
	if (!pre_bbs_.empty())
	{
		ret += "pre_bbs: ";
		for (auto& i : pre_bbs_)
		{
			ret += i->name() + " ";
		}
	}
	if (!suc_bbs_.empty())
	{
		ret += "suc_bbs: ";
		for (auto& i : suc_bbs_)
		{
			ret += i->name() + " ";
		}
	}
	ret += "\n";
	for (auto& i : instructions_)
	{
		ret += to_string(sid++) + ":\t" + i->print() + "\n";
	}
	return ret;
}

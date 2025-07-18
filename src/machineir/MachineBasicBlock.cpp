#include "MachineBasicBlock.hpp"

#include <stdexcept>

#include "BasicBlock.hpp"
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
			case Instruction::eq: return Instruction::ne;
			case Instruction::ne: return Instruction::eq;
		}
		throw runtime_error("invalid op");
	}
}


MBasicBlock::MBasicBlock(std::string name, MFunction* function) : function_(function), name_(std::move(name))
{
}

MBasicBlock* MBasicBlock::createBasicBlock(const std::string& name, MFunction* function)
{
	return new MBasicBlock{name, function};
}

void MBasicBlock::accept(BasicBlock* block,
                         map<Value*, MOperand*>& opMap, std::map<BasicBlock*, MBasicBlock*>& blockMap,
                         const std::map<MBasicBlock*, list<MCopy*>>& phiMap, std::map<Function*, MFunction*>& funcMap)
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
				acceptAllocaInst(inst, opMap, this);
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
		auto reg = block->module()->getOperandFor(retu, opMap);
		auto copy = new MCopy{block, reg, Register::getRegisterWithType(0, retu->get_type(), module()), 32};
		instructions_.emplace_back(copy);
	}
	auto ret = new MRet{block};
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
			auto cmp = new MBcc{block, reverseOp(op), t};
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
	auto l = block->module()->getOperandFor(l0, opMap);
	auto r = block->module()->getOperandFor(r0, opMap);
	auto t = block->module()->getOperandFor(instruction, opMap);
	auto op = instruction->get_instr_type();
	auto ret = new MMathInst{block, op, l, r, t, 32};
	instructions_.emplace_back(ret);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void MBasicBlock::acceptAllocaInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap,
                                   const MBasicBlock* block)
{
	auto fi = block->function()->allocaStack(instruction);
	opMap.emplace(instruction, fi);
}

void MBasicBlock::acceptLoadInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto regLike = block->module()->getOperandFor(instruction, opMap);
	auto stackLike = block->module()->getOperandFor(instruction->get_operand(0), opMap);
	auto ret = new MLDR{block, regLike, stackLike, 32};
	instructions_.emplace_back(ret);
}

void MBasicBlock::acceptStoreInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap,
                                  MBasicBlock* block)
{
	auto regLike = block->module()->getOperandFor(instruction->get_operand(0), opMap);
	auto stackLike = block->module()->getOperandFor(instruction->get_operand(1), opMap);
	auto ret = new MSTR{block, regLike, stackLike, 32};
	instructions_.emplace_back(ret);
}

void MBasicBlock::acceptCmpInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto l = block->module()->getOperandFor(instruction->get_operand(0), opMap);
	auto r = block->module()->getOperandFor(instruction->get_operand(1), opMap);
	auto ret = new MCMP{block, l, r};
	instructions_.emplace_back(ret);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void MBasicBlock::acceptPhiInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap,
                                std::map<BasicBlock*, MBasicBlock*>& bmap,
                                std::map<MBasicBlock*, std::list<MCopy*>> phiMap, const MBasicBlock* block)
{
	auto phi = dynamic_cast<PhiInst*>(instruction);
	auto pairs = phi->get_phi_pairs();
	for (auto& [val, bb] : pairs)
	{
		auto mbb = bmap[bb];
		auto cp = new MCopy{
			mbb, block->module()->getOperandFor(val, opMap), block->module()->getOperandFor(phi, opMap),
			val->get_type()->sizeInBitsInArm64()
		};
		phiMap[mbb].emplace_back(cp);
	}
}


void MBasicBlock::acceptCallInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap,
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
					block, module()->getOperandFor(instruction->get_operand(idx), opMap),
					Register::getFRegister(fc, module()), 32
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
					block, module()->getOperandFor(instruction->get_operand(idx), opMap),
					Register::getIRegister(ic, module()), arg.get_type()->sizeInBitsInArm64()
				};
				instructions_.emplace_back(cp);
				++ic;
				continue;
			}
		}
		auto st = new MSTR{
			block, module()->getOperandFor(instruction->get_operand(idx), opMap),
			mfunc->getFix(nc), arg.get_type()->sizeInBitsInArm64()
		};
		instructions_.emplace_back(st);
		++nc;
	}
	auto bl = new MBL{block, FuncAddress::get(mfunc)};
	instructions_.emplace_back(bl);
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
	auto frameIndex = this->module()->getOperandFor(gep->get_operand(0), opMap);
	auto immOperand = Immediate::getImmediate(0, module());
	MOperand* operand = immOperand;
	Immediate* immB = nullptr;
	Immediate* immA = nullptr;
	// A * op + B
	for (int i = 0; i < dimCount; i++)
	{
		int len = static_cast<int>(dimLen[i]);
		if (len == 0)
		{
			immOperand = Immediate::getImmediate(0, module());
			operand = immOperand;
			immB = nullptr;
			immA = nullptr;
			continue;
		}
		if (i == dimCount - 1) len <<= 2;
		auto op = gep->get_operand(i + 1);
		auto mop = module()->getOperandFor(op, opMap);
		auto immMop = dynamic_cast<Immediate*>(mop);
		if (immOperand != nullptr)
		{
			if (immMop != nullptr)
			{
				// C + D -> (C + D)
				if (immMop->asInt() != 0)
				{
					immOperand = Immediate::getImmediate((immOperand->asInt() + immMop->asInt()) * len, module());
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
				if (immMop->asInt() != 0)
				{
					if (immB == nullptr) immB = immMop;
					else immB = Immediate::getImmediate(immB->asInt() + immMop->asInt(), module());
				}
			}
			else
			{
				// Ax + B + y -> (Ax + y) + B
				if (immA != nullptr)
				{
					auto reg = VirtualRegister::createVirtualIRegister(module());
					auto inst = new MMathInst{block, Instruction::mul, immA, operand, reg, 32};
					instructions_.emplace_back(inst);
					operand = reg;
					immA = nullptr;
				}
				auto reg2 = VirtualRegister::createVirtualIRegister(module());
				auto inst = new MMathInst{block, Instruction::add, operand, immMop, reg2, 32};
				instructions_.emplace_back(inst);
				operand = reg2;
			}
		}
		if (len == 1) continue;
		if (immOperand != nullptr)
		{
			immOperand = Immediate::getImmediate(immOperand->asInt() * len, module());
		}
		else
		{
			if (immA == nullptr) immA = Immediate::getImmediate(len, module());
			else immA = Immediate::getImmediate(immA->asInt() * len, module());
			if (immB != nullptr) immB = Immediate::getImmediate(immB->asInt() * len, module());
		}
	}
	auto targetReg = module()->getOperandFor(instruction, opMap);
	if (immOperand != nullptr)
	{
		if (immOperand->asInt() == 0)
		{
			auto inst = new MCopy{block, frameIndex, targetReg, 64};
			instructions_.emplace_back(inst);
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
			auto reg = VirtualRegister::createVirtualIRegister(module());
			auto inst = new MMathInst{block, Instruction::mul, immA, operand, reg, 32};
			instructions_.emplace_back(inst);
			operand = reg;
		}
		if (immB != nullptr)
		{
			auto reg = VirtualRegister::createVirtualIRegister(module());
			auto inst = new MMathInst{block, Instruction::add, immB, operand, reg, 32};
			instructions_.emplace_back(inst);
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
	if (dynamic_cast<MBcc*>(instructions_.back()) != nullptr)
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
	auto t = module()->getOperandFor(instruction, opMap);
	auto cmp = new MCSET{block, op, t};
	instructions_.emplace_back(cmp);
}

void MBasicBlock::acceptFpToSiInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto fp = module()->getOperandFor(instruction->get_operand(0), opMap);
	auto si = module()->getOperandFor(instruction, opMap);
	instructions_.emplace_back(new MFCVTZS{block, fp, si});
}

void MBasicBlock::acceptSiToFpInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto si = module()->getOperandFor(instruction->get_operand(0), opMap);
	auto fp = module()->getOperandFor(instruction, opMap);
	instructions_.emplace_back(new MSCVTF{block, si, fp});
}

void MBasicBlock::acceptMemCpyInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto mcp = dynamic_cast<MemCpyInst*>(instruction);
	auto count = mcp->get_copy_bytes() >> 4;
	auto src = module()->getOperandFor(instruction->get_operand(0), opMap);
	auto des = module()->getOperandFor(instruction->get_operand(1), opMap);
	if (count <= 4)
	{
		{
			auto reg = VirtualRegister::createVirtualFRegister(module());
			auto load = new MLDR{block, reg, src, 128};
			auto store = new MSTR{block, reg, des, 128};
			instructions_.emplace_back(load);
			instructions_.emplace_back(store);
		}
		for (int i = 1; i < count; i++)
		{
			auto reg = VirtualRegister::createVirtualFRegister(module());
			auto pos = Immediate::getImmediate(i << 4, module());
			auto src2 = VirtualRegister::createVirtualIRegister(module());
			auto des2 = VirtualRegister::createVirtualIRegister(module());
			auto off1 = new MMathInst{block, Instruction::add, src, pos, src2, 64};
			auto off2 = new MMathInst{block, Instruction::add, des, pos, des2, 64};
			auto load = new MLDR{block, reg, src2, 128};
			auto store = new MSTR{block, reg, des2, 128};
			instructions_.emplace_back(off1);
			instructions_.emplace_back(load);
			instructions_.emplace_back(off2);
			instructions_.emplace_back(store);
		}
	}
	else
	{
		auto cr0 = new MCopy{block, des, Register::getIRegister(0, module()), 64};
		auto cr1 = new MCopy{block, src, Register::getIRegister(1, module()), 64};
		auto cr2 = new MCopy{
			block, Immediate::getImmediate(mcp->get_copy_bytes(), module()), Register::getIRegister(2, module()), 64
		};
		auto bl = new MBL{block, FuncAddress::get(module()->memcpyFunc())};
		instructions_.emplace_back(cr0);
		instructions_.emplace_back(cr1);
		instructions_.emplace_back(cr2);
		instructions_.emplace_back(bl);
	}
}

void MBasicBlock::acceptMemClearInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto mcp = dynamic_cast<MemClearInst*>(instruction);
	auto count = mcp->get_clear_bytes() >> 4;
	auto des = module()->getOperandFor(instruction->get_operand(0), opMap);
	if (count <= 8)
	{
		{
			auto store = new MSTR{block, Immediate::getImmediate(0, module()), des, 128};
			instructions_.emplace_back(store);
		}
		for (int i = 1; i < count; i++)
		{
			auto pos = Immediate::getImmediate(i << 4, module());
			auto des2 = VirtualRegister::createVirtualIRegister(module());
			auto off2 = new MMathInst{block, Instruction::add, des, pos, des2, 64};
			auto store = new MSTR{block, Immediate::getImmediate(0, module()), des2, 128};
			instructions_.emplace_back(off2);
			instructions_.emplace_back(store);
		}
	}
	else
	{
		auto cr0 = new MCopy{block, des, Register::getIRegister(0, module()), 64};
		auto cr1 = new MCopy{
			block, Immediate::getImmediate(mcp->get_clear_bytes(), module()), Register::getIRegister(1, module()), 64
		};
		auto bl = new MBL{block, FuncAddress::get(module()->memclrFunc())};
		instructions_.emplace_back(cr0);
		instructions_.emplace_back(cr1);
		instructions_.emplace_back(bl);
	}
}

void MBasicBlock::acceptCopyInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto f = module()->getOperandFor(instruction->get_operand(0), opMap);
	auto t = module()->getOperandFor(instruction, opMap);
	instructions_.emplace_back(new MCopy{block, f, t, 64});
}

MModule* MBasicBlock::module() const
{
	return function_->module();
}

MFunction* MBasicBlock::function() const
{
	return function_;
}

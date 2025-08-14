#include "MachineBasicBlock.hpp"

#include <algorithm>

#include "BasicBlock.hpp"
#include "Config.hpp"
#include "Constant.hpp"
#include "CountLZ.hpp"
#include "Instruction.hpp"
#include "MachineFunction.hpp"
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
			case Instruction::shl:
			case Instruction::ashr:
			case Instruction::and_:
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
	if (retu->discarded_) return;
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
		auto inst = dynamic_cast<MCMP*>(block->instructions().back());
		if (inst == nullptr) inst = dynamic_cast<MCMP*>(block->instructions()[block->instructions().size() - 2]);
		ASSERT(inst != nullptr);
		auto op = asCmpGetOp(op0);
		auto cmp = new MB{block, op, t, f};
		inst->tiedB_ = cmp;
		cmp->tiedWith_ = inst;
		instructions_.emplace_back(cmp);
	}
	else
	{
		auto t = BlockAddress::get(bmap[dynamic_cast<BasicBlock*>(br->get_operand(0))]);
		auto cmp = new MB{block, t};
		instructions_.emplace_back(cmp);
	}
}

void MBasicBlock::acceptMathInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block)
{
	auto l0 = instruction->get_operand(0);
	auto r0 = instruction->get_operand(1);
	auto l = block->function()->getOperandFor(l0, opMap);
	auto r = block->function()->getOperandFor(r0, opMap);
	auto t = block->function()->getOperandFor(instruction, opMap);
	auto m = new MMathInst{block, instruction->get_instr_type(), l, r, t, 32};
	instructions_.emplace_back(m);
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
	for (auto inst : block->get_instructions().phi_and_allocas())
	{
		auto alloc = dynamic_cast<AllocaInst*>(inst);
		if (alloc != nullptr)
		{
			auto cost = Function::opWeight(alloc, bmap);
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
			ASSERT(function() == i.frame_index_->func());
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
	auto regLike = dynamic_cast<VirtualRegister*>(block->function()->getOperandFor(instruction, opMap));
	ASSERT(regLike != nullptr);
	regLike->sinked = true;
	auto stackLike = block->function()->getOperandFor(instruction->get_operand(0), opMap);
	auto ret = new MLDR{block, regLike, stackLike, u2iNegThrow(instruction->get_type()->sizeInBitsInArm64())};
	instructions_.emplace_back(ret);
}

void MBasicBlock::acceptStoreInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap,
                                  MBasicBlock* block)
{
	auto regLike = block->function()->getOperandFor(instruction->get_operand(0), opMap);
	auto stackLike = block->function()->getOperandFor(instruction->get_operand(1), opMap);
	auto ret = new MSTR{
		block, regLike, stackLike, u2iNegThrow(instruction->get_operand(0)->get_type()->sizeInBitsInArm64())
	};
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
	auto dest = dynamic_cast<VirtualRegister*>(block->function()->getOperandFor(phi, opMap));
	ASSERT(dest != nullptr);
	dest->sinked = true;
	for (auto& [val, bb] : pairs)
	{
		auto mbb = bmap[bb];
		auto cp = new MCopy{
			mbb, block->function()->getOperandFor(val, opMap), dest,
			u2iNegThrow(val->get_type()->sizeInBitsInArm64())
		};
		phiMap[mbb].emplace_back(cp);
	}
}


void MBasicBlock::acceptCallInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap,
                                 std::map<Function*, MFunction*>& funcMap, MBasicBlock* block)
{
	Instruction* next = nullptr;
	if(o1Optimization){
		auto p = instruction->get_parent();
		auto it = p->get_instructions().begin();
		auto ed = p->get_instructions().end();
		while (it != ed)
		{
			if (*it == instruction)
			{
				++it;
				next = *it;
				break;
			}
			++it;
		}
		if (next != nullptr && next->is_ret())
		{
			auto ret = dynamic_cast<ReturnInst*>(next);
			if ((ret->get_type() == Types::VOID || ret->get_operand(0) == instruction) && instruction->get_operand(0) ==
			    instruction->get_parent()->get_parent())
				ret->discarded_ = true;
			else next = nullptr;
		}
		else next = nullptr;
	}
	auto func = dynamic_cast<Function*>(instruction->get_operand(0));
	auto mfunc = funcMap[func];
	int ic = 0;
	int fc = 0;
	int nc = 0;
	int idx = 0;
	vector<MInstruction*> buffer;
	for (auto& arg : func->get_args())
	{
		++idx;
		if (arg->get_type() == Types::FLOAT)
		{
			if (fc < 8)
			{
				auto cp = new MCopy{
					block, function()->getOperandFor(instruction->get_operand(idx), opMap),
					Register::getFParameterRegister(fc, module()), 32
				};
				buffer.emplace_back(cp);
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
					Register::getIParameterRegister(ic, module()), u2iNegThrow(arg->get_type()->sizeInBitsInArm64())
				};
				buffer.emplace_back(cp);
				++ic;
				continue;
			}
		}
		auto st = new MSTR{
			block, function()->getOperandFor(instruction->get_operand(idx), opMap),
			mfunc->getFix(nc), u2iNegThrow(arg->get_type()->sizeInBitsInArm64())
		};
		instructions_.emplace_back(st);
		++nc;
		if (next == nullptr)
		{
			st->forCall_ = true;
		}
	}
	for (auto& i : buffer) instructions_.emplace_back(i);
	if (next != nullptr)
	{
		auto t = function()->blocks()[0];
		t->pre_bbs().emplace(block);
		block->suc_bbs().emplace(t);
		auto b = new MB{block, BlockAddress::get(function()->blocks()[0])};
		instructions_.emplace_back(b);
		return;
	}
	auto bl = new MBL{block, FuncAddress::get(mfunc), func};
	instructions_.emplace_back(bl);
	if (instruction->get_type() != Types::VOID)
	{
		auto x0 = Register::getParameterRegisterWithType(0, instruction->get_type(), module());
		auto get = function_->getOperandFor(instruction, opMap);
		instructions_.emplace_back(new MCopy{block, x0, get, 32});
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
	vector<int> dimLen;
	if (!typeVal->isArrayType())
		dimLen.emplace_back(1);
	else
	{
		auto ar = typeVal->toArrayType();
		dimLen = ar->dimensions();
		dimLen.emplace_back(1);
	}
	int dimCount = instruction->get_num_operand() - 1;
	auto frameIndex = this->function()->getOperandFor(gep->get_operand(0), opMap);
	auto immOperand = Immediate::getImmediate(0, module());
	MOperand* operand = immOperand;
	Immediate* immB = nullptr;
	Immediate* immA = nullptr;
	int width = use64BitsMathOperationInPointerOp ? 64 : 32;
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
			int rs = u2iNegThrow(dimLen.size()) - 1;
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
			ASSERT(vr != nullptr);
			if (vr->size() != 64 && !ignoreNegativeArrayIndexes)
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
					auto reg = VirtualRegister::createVirtualIRegister(function(), (width));
					if (use64BitsMathOperationInPointerOp)
					{
						long long c = immA->as64BitsInt();
						if ((c & (c - 1)) == 0)
						{
							auto n = m_countr_zero(c);
							auto inst = new MMathInst{
								block, Instruction::shl, operand, Immediate::getImmediate(n, module()), reg, width
							};
							instructions_.emplace_back(inst);
						}
						else
						{
							auto inst = new MMathInst{block, Instruction::mul, immA, operand, reg, width};
							instructions_.emplace_back(inst);
						}
					}
					else
					{
						int c = immA->asInt();
						if ((c & (c - 1)) == 0)
						{
							auto n = m_countr_zero(c);
							auto inst = new MMathInst{
								block, Instruction::shl, operand, Immediate::getImmediate(n, module()), reg, width
							};
							instructions_.emplace_back(inst);
						}
						else
						{
							auto inst = new MMathInst{block, Instruction::mul, immA, operand, reg, width};
							instructions_.emplace_back(inst);
						}
					}
					operand = reg;
					immA = nullptr;
				}
				auto reg2 = VirtualRegister::createVirtualIRegister(function(), (width));
				auto inst = new MMathInst{block, Instruction::add, operand, mop, reg2, width};
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
			auto reg = VirtualRegister::createVirtualIRegister(function(), (width));
			if (use64BitsMathOperationInPointerOp)
			{
				long long c = immA->as64BitsInt();
				if ((c & (c - 1)) == 0)
				{
					auto n = m_countr_zero(c);
					auto inst = new MMathInst{
						block, Instruction::shl, operand, Immediate::getImmediate(n, module()), reg, width
					};
					instructions_.emplace_back(inst);
				}
				else
				{
					auto inst = new MMathInst{block, Instruction::mul, immA, operand, reg, width};
					instructions_.emplace_back(inst);
				}
			}
			else
			{
				int c = immA->asInt();
				if ((c & (c - 1)) == 0)
				{
					auto n = m_countr_zero(c);
					auto inst = new MMathInst{
						block, Instruction::shl, operand, Immediate::getImmediate(n, module()), reg, width
					};
					instructions_.emplace_back(inst);
				}
				else
				{
					auto inst = new MMathInst{block, Instruction::mul, immA, operand, reg, width};
					instructions_.emplace_back(inst);
				}
			}
			operand = reg;
		}
		if (immB != nullptr)
		{
			auto reg = VirtualRegister::createVirtualIRegister(function(), (width));
			auto inst = new MMathInst{block, Instruction::add, immB, operand, reg, width};
			instructions_.emplace_back(inst);
			operand = reg;
		}
		auto vop = dynamic_cast<VirtualRegister*>(operand);
		ASSERT(vop != nullptr);
		if (vop->size() < 64 && !ignoreNegativeArrayIndexes)
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

namespace
{
	struct CopyUsage
	{
		MCopy* copyToMe_;
		int usage_;
	};
}

void MBasicBlock::mergePhiCopies(std::list<MCopy*>& copies)
{
	map<MOperand*, CopyUsage> m;
	for (auto value : copies)
	{
		if (value->def(0) != value->use(0))
			m.emplace(value->operand(1), CopyUsage{value, 0});
	}
	for (auto value : copies)
	{
		auto it = m.find(value->operand(0));
		if (it != m.end()) it->second.usage_++;
	}
	copies.clear();
	list<MOperand*> rm;
	while (!m.empty())
	{
		rm.clear();
		for (auto [to, cpu] : m)
		{
			if (cpu.usage_ == 0)
			{
				rm.emplace_back(to);
				copies.emplace_back(cpu.copyToMe_);
				auto it = m.find(cpu.copyToMe_->operand(0));
				if (it != m.end()) it->second.usage_--;
			}
		}
		if (rm.empty())
		{
			auto it = m.begin();
			auto to = it->first;
			auto& cpu = it->second;
			auto from = cpu.copyToMe_->operand(0);
			auto& fcpu = m[from];
			ASSERT(fcpu.usage_ == 1);
			fcpu.usage_--;
			auto toR = dynamic_cast<VirtualRegister*>(to);
			ASSERT(toR != nullptr);
			auto vr = VirtualRegister::copy(function_, toR);
			cpu.copyToMe_->replace(from, vr, function_);
			copies.emplace_back(new MCopy{this, from, vr, vr->size()});
		}
		else for (auto i : rm) m.erase(i);
	}

	if (instructions_.empty())
	{
		for (auto& cp : copies)
			instructions_.emplace_back(cp);
		return;
	}
	if (dynamic_cast<MB*>(instructions_.back()) != nullptr)
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
	auto t = dynamic_cast<VirtualRegister*>(function()->getOperandFor(instruction, opMap));
	ASSERT(t != nullptr);
	t->sinked = true;
	auto inst = dynamic_cast<MCMP*>(block->instructions().back());
	ASSERT(inst != nullptr);
	auto cmp = new MCSET{block, op, t};
	inst->tiedC_ = cmp;
	cmp->tiedWith_ = inst;
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
	auto count = logicalRightShift(mcp->get_copy_bytes(), 4);
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
			auto load = new MLD1V16B{block, src, 4, count > 4 ? 64 : 0};
			auto store = new MST1V16B{block, des, 4, count > 4 ? 64 : 0};
			instructions_.emplace_back(load);
			instructions_.emplace_back(store);
			count -= 4;
		}
		if (count > 0)
		{
			auto load = new MLD1V16B{block, src, count, 0};
			auto store = new MST1V16B{block, des, count, 0};
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
	auto count = logicalRightShift(mcp->get_clear_bytes(), 4);
	auto desF = function()->getOperandFor(instruction->get_operand(0), opMap);
	if (count <= maxCopyInstCountToInlineMemclr)
	{
		instructions_.emplace_back(new MST1ZTV16B{block, count > 4 ? 4 : count});
		auto des = Register::getIIP0(module());
		auto cp0 = new MCopy{block, desF, des, 64};
		instructions_.emplace_back(cp0);
		while (count >= 4)
		{
			auto store = new MST1V16B{block, des, 4, count > 4 ? 64 : 0};
			instructions_.emplace_back(store);
			count -= 4;
		}
		if (count > 0)
		{
			auto store = new MST1V16B{block, des, count, 0};
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

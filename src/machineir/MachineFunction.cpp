#include "MachineFunction.hpp"

#include <algorithm>
#include <stdexcept>

#include "BasicBlock.hpp"
#include "Config.hpp"
#include "Constant.hpp"
#include "CountLZ.hpp"
#include "Instruction.hpp"
#include "LiveMessage.hpp"
#include "LoopDetection.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include "MachineModule.hpp"
#include "Module.hpp"
#include "Type.hpp"
#include "Util.hpp"

using namespace std;

MFunction::~MFunction()
{
	for (auto bb : blocks_) delete bb;
	for (auto [i, j] : ba_cache_) delete j;
	for (auto i : stack_) delete i;
	for (auto i : fix_) delete i;
	for (auto i : virtual_iregs_) delete i;
	for (auto i : virtual_fregs_) delete i;
	delete funcSuffix_;
	delete funcPrefix_;
}

MFunction::MFunction(std::string name, MModule* module)
	: module_(module),
	  name_(std::move(name))
{
	id_ = u2iNegThrow(module_->allFuncs_.size());
	module->allFuncs_.emplace_back(this);
}

MFunction* MFunction::createFunc(const std::string& name, MModule* module)
{
	return new MFunction{name, module};
}

FrameIndex* MFunction::allocaStack(Value* value)
{
	auto a = dynamic_cast<AllocaInst*>(value);
	auto ty = a->get_alloca_type();
	FrameIndex* index = new FrameIndex{this, u2iNegThrow(stack_.size()), ty->sizeInBitsInArm64(), true};
	stack_.emplace_back(index);
	return index;
}

FrameIndex* MFunction::allocaStack(int size)
{
	FrameIndex* index = new FrameIndex{this, u2iNegThrow(stack_.size()), size, true};
	stack_.emplace_back(index);
	return index;
}

FrameIndex* MFunction::getFix(int idx) const
{
	return fix_[idx];
}

FrameIndex* MFunction::allocaFix(const Value* value)
{
	auto arg = dynamic_cast<const Argument*>(value);
	ASSERT(arg);
	ASSERT(arg->get_parent()->get_name() == name());
	FrameIndex* index = new FrameIndex{this, u2iNegThrow(fix_.size()), value->get_type()->sizeInBitsInArm64(), false};
	fix_.emplace_back(index);
	return index;
}

void MFunction::preprocess(Function* function)
{
	int ic = 0;
	int fc = 0;
	for (auto argC : function->get_args())
	{
		if (argC->get_type() != Types::FLOAT)
		{
			if (ic < 8)
			{
				++ic;
				continue;
			}
		}
		else
		{
			if (fc < 8)
			{
				++fc;
				continue;
			}
		}
		allocaFix(argC);
	}
}

void MFunction::accept(Function* function, std::map<Function*, MFunction*>& funcMap,
                       std::map<GlobalVariable*, GlobalAddress*>& global_address)
{
	const auto& bbs = function->get_basic_blocks();
	std::map<BasicBlock*, MBasicBlock*> cache;
	int bbc = 0;
	for (const auto& bb : bbs)
	{
		ASSERT(bb != nullptr);
		const auto mbb = MBasicBlock::createBasicBlock(name_ + "_" + bb->get_name() + "_" + to_string(bbc), this);
		mbb->id_ = bbc++;
		blocks_.emplace_back(mbb);
		cache.emplace(bb, mbb);
	}
	for (auto& [bb, mbb] : cache)
	{
		auto& pres = bb->get_pre_basic_blocks();
		for (auto& pre : pres)
			mbb->pre_bbs_.emplace(cache[pre]);
		auto& succ = bb->get_succ_basic_blocks();
		for (auto& suc : succ)
			mbb->suc_bbs_.emplace(cache[suc]);
	}

	auto entryBB = function->get_entry_block();
	map<Value*, MOperand*> opMap;

	auto entryMBB = cache[entryBB];

	lrGuard_ = VirtualRegister::createVirtualIRegister(this, 64);

	PassManager* mng = new PassManager{function->get_parent()};
	LoopDetection* detection = mng->getFuncInfo<LoopDetection>(function);

	for (auto loop : detection->get_loops())
	{
		for (auto bb : loop->get_blocks())
		{
			cache[bb]->weight_ *= static_cast<float>(useMultiplierPerLoop);
		}
	}

	delete mng;

	map<MBasicBlock*, list<MCopy*>> phiMap;

	entryMBB->acceptAllocaInsts(entryBB, opMap);

	for (auto& [glob, mglob] : global_address)
		opMap[glob] = mglob;

	for (auto& [bb, mbb] : cache)
	{
		mbb->accept(bb, opMap, cache, phiMap, funcMap);
	}

	for (auto& [mbb, cp] : phiMap)
	{
		mbb->mergePhiCopies(cp);
	}


	int ic = 0;
	int fc = 0;
	int idx = 0;
	vector<MInstruction*> parameterInsts;
	for (auto& argC : function->get_args())
	{
		if (argC->get_type() != Types::FLOAT)
		{
			if (ic < 8)
			{
				if (!argC->get_use_list().empty())
				{
					auto cp = new MCopy{
						entryMBB, Register::getIParameterRegister(ic, module_),
						getOperandFor(argC, opMap),
						u2iNegThrow(argC->get_type()->sizeInBitsInArm64())
					};
					parameterInsts.emplace_back(cp);
				}
				++ic;
				continue;
			}
		}
		else
		{
			if (fc < 8)
			{
				if (!argC->get_use_list().empty())
				{
					auto cp = new MCopy{
						entryMBB, Register::getFParameterRegister(fc, module_),
						getOperandFor(argC, opMap),
						u2iNegThrow(argC->get_type()->sizeInBitsInArm64())
					};
					parameterInsts.emplace_back(cp);
				}
				++fc;
				continue;
			}
		}
		auto frameIdx = getFix(idx++);
		// 需要先加载出来而非在 RegPrefill 加载, 否则 MIR 不知道一个参数是寄存器参数还是栈参数, 需不需要 load
		if (!argC->get_use_list().empty())
		{
			auto val = getOperandFor(argC, opMap);
			auto vr = dynamic_cast<VirtualRegister*>(val);
			ASSERT(vr != nullptr);
			ASSERT(this == frameIdx->func());
			vr->replacePrefer_ = frameIdx;
			vr->needLoad_ = true;
			frameIdx->tiedWith_ = vr;
			auto cp = new MLDR{entryMBB, val, frameIdx, u2iNegThrow(argC->get_type()->sizeInBitsInArm64())};
			parameterInsts.emplace_back(cp);
		}
	}

	entryMBB->instructions_.insert(entryMBB->instructions_.begin(), parameterInsts.begin(), parameterInsts.end());

	auto eb = new MBasicBlock{function->get_name() + "_prolog", this};
	eb->instructions_.emplace_back(new MCopy{entryMBB, Register::getLR(module_), lrGuard_, 64});
	auto b = new MB{eb, BlockAddress::get(entryMBB)};
	eb->instructions_.emplace_back(b);
	eb->suc_bbs_.emplace(blocks_[0]);
	blocks_[0]->pre_bbs_.emplace(eb);
	blocks_.emplace(blocks_.begin(), eb);
	int id = 0;
	for (auto i : blocks_)
	{
		i->id_ = id++;
	}
}

MModule* MFunction::module() const
{
	return module_;
}

std::string MFunction::print() const
{
	string ret = "function " + name_ + ":";
	for (auto& bb : blocks_)
	{
		int i = 0;
		ret += '\n';
		ret += bb->print(i);
	}
	return ret;
}

MOperand* MFunction::getOperandFor(Value* value, std::map<Value*, MOperand*>& opMap)
{
	auto fd = opMap.find(value);
	if (fd != opMap.end()) return fd->second;
	MOperand* operand;
	if (value->get_type() == Types::FLOAT)
	{
		auto imm = dynamic_cast<Constant*>(value);
		if (imm != nullptr)
		{
			operand = Immediate::getImmediate(imm->getFloatConstant(), module_);
		}
		else
		{
			operand = VirtualRegister::createVirtualFRegister(
				this, u2iNegThrow(value->get_type()->sizeInBitsInArm64()));
		}
	}
	else
	{
		auto imm = dynamic_cast<Constant*>(value);
		if (imm != nullptr)
		{
			operand = Immediate::getImmediate(imm->getIntConstant(), module_);
		}
		else
		{
			operand = VirtualRegister::createVirtualIRegister(
				this, u2iNegThrow(value->get_type()->sizeInBitsInArm64()));
		}
	}
	opMap.emplace(value, operand);
	return operand;
}

void MFunction::spill(VirtualRegister* vreg, LiveMessage* message)
{
	vreg->spilled = true;
	if (vreg->replacePrefer_ != nullptr && !vreg->needLoad_)
	{
		auto entry = blocks_[0];
		auto& inst = entry->instructions();
		int size = u2iNegThrow(inst.size());
		for (int i = 0; i < size; i++)
		{
			auto in = dynamic_cast<MCopy*>(inst[i]);
			if (in != nullptr && in->def(0) == vreg)
			{
				inst.erase(inst.begin() + i);
				in->removeAllUse();
				delete in;
				break;
			}
		}
		replaceAllOperands(vreg, vreg->replacePrefer_);
		return;
	}
	if (vreg->needLoad_)
	{
		MLDR* u = nullptr;
		for (auto use : useList()[vreg->replacePrefer_])
		{
			if (auto inst = dynamic_cast<MLDR*>(use))
			{
				u = inst;
				break;
			}
		}
		ASSERT(u != nullptr);
		auto& insts = u->block()->instructions();
		for (int i = 0, size = u2iNegThrow(insts.size()); i < size; i++)
		{
			if (insts[i] == u)
			{
				ASSERT(u->operands()[0] == vreg && u->operands()[1] ==
					vreg->replacePrefer_);
				insts.erase(insts.begin() + i);
				u->removeAllUse();
				delete u;
				break;
			}
		}
	}
	auto to = dynamic_cast<FrameIndex*>(vreg->replacePrefer_);
	if (to == nullptr)
	{
		to = allocaStack(vreg->size());
		to->spilledFrame_ = true;
	}
	VirtualRegister* nreg = vreg;
	for (auto bb : blocks())
	{
		auto& instructions = bb->instructions();
		int size = u2iNegThrow(instructions.size());
		for (int i = 0; i < size; i++)
		{
			auto inst = instructions[i];
			bool haveUse = inst->haveUseOf(vreg);
			bool haveDef = inst->haveDefOf(vreg);
			if (!(haveUse || haveDef)) continue;
			if (nreg == nullptr)
			{
				nreg = VirtualRegister::copy(this, vreg);
				nreg->spilled = true;
				message->addRegister(nreg);
			}
			inst->replace(vreg, nreg, this);
			if (haveUse)
			{
				auto ld = new MLDR{bb, nreg, to, (nreg->size())};
				instructions.emplace(instructions.begin() + i, ld);
				size++;
				i++;
			}
			if (haveDef)
			{
				auto ld = new MSTR{bb, nreg, to, (nreg->size())};
				instructions.emplace(instructions.begin() + (i + 1), ld);
				size++;
				i++;
			}
			nreg = nullptr;
		}
	}
}

void MFunction::replaceAllOperands(MOperand* from, MOperand* to)
{
	for (auto i : useList_[from])
	{
		i->onlyAddUseReplace(from, to, this);
	}
	useList_[from].clear();
}

void MFunction::replaceAllOperands(std::unordered_map<FrameIndex*, FrameIndex*>& rpm)
{
	std::unordered_map<FrameIndex*, list<MInstruction*>> nextL;
	for (auto& [from, to] : rpm)
	{
		auto& l0 = nextL[to];
		for (auto i : useList_[from])
		{
			i->stayUseReplace(from, to, this);
			l0.emplace_back(i);
		}
		useList_[from].clear();
	}
	for (auto& [m,n] : nextL) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
	{
		auto& l0 = useList_[m];
		for (auto instruction : n)
			l0.emplace(instruction);
	}
}

void MFunction::addUse(MOperand* op, MInstruction* ins)
{
	useList_[op].emplace(ins);
}

void MFunction::removeUse(MOperand* op, MInstruction* ins)
{
	useList_[op].erase(ins);
}

void MFunction::rewriteCallsDefList() const
{
	for (auto call : calls_)
	{
		auto fa = dynamic_cast<FuncAddress*>(call->operands()[0]);
		auto f = fa->function();
		for (auto i : f->destroy_regs())
			if (i < module_->IRegisterCount())
				call->imp_def().emplace_back(module_->IRegs()[i]);
			else call->imp_def().emplace_back(module_->FRegs()[i - module_->IRegisterCount()]);
	}
}

void MFunction::rewriteDestroyRegs()
{
	auto& lb = module_->lib_functions_[0]->destroy_regs();
	for (auto c : called_) destroyRegs_ |= module_->all_funcs()[c]->destroyRegs_;
	if (destroyRegs_ == lb) return;
	for (auto bb : blocks_)
	{
		for (auto inst : bb->instructions_)
		{
			if (!inst->def().empty())
			{
				auto df = dynamic_cast<Register*>(inst->def(0));
				if (df == nullptr || !df->callerSave_)continue;
				int id = df->isIntegerRegister() ? df->id() : df->id() + module_->IRegisterCount();
				destroyRegs_.set(id);
			}
		}
	}
}

std::unordered_map<MOperand*, std::unordered_set<MInstruction*>>& MFunction::useList()
{
	return useList_;
}

std::unordered_set<MInstruction*>& MFunction::useList(MOperand* op)
{
	return useList_[op];
}

const std::vector<VirtualRegister*>& MFunction::IVRegs() const
{
	return virtual_iregs_;
}

const std::vector<VirtualRegister*>& MFunction::FVRegs() const
{
	return virtual_fregs_;
}

int MFunction::virtualIRegisterCount() const
{
	return u2iNegThrow(virtual_iregs_.size());
}

int MFunction::virtualFRegisterCount() const
{
	return u2iNegThrow(virtual_fregs_.size());
}

int MFunction::virtualRegisterCount() const
{
	return u2iNegThrow(virtual_iregs_.size() + virtual_fregs_.size());
}

void MFunction::checkValidUseList()
{
	unordered_map<MOperand*, unordered_set<MInstruction*>> nul;
	for (auto bb : blocks_)
	{
		for (auto inst : bb->instructions_)
		{
			for (auto op : inst->operands())
			{
				nul[op].emplace(inst);
				if (!useList_[op].count(inst)) throw runtime_error("wrong");
			}
		}
	}
	for (auto& [i,j] : useList_)
	{
		auto& ni = nul[i];
		int jsize = u2iNegThrow(j.size());
		int nsize = u2iNegThrow(ni.size());
		if (jsize != nsize) throw runtime_error("wrong");
	}
	for (auto i : stack_)
	{
		if (useList()[i].empty()) throw runtime_error("useless stack");
	}
}

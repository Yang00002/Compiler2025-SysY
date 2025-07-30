#include "MachineIR.hpp"

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
#include "Module.hpp"
#include "Type.hpp"

using namespace std;

MFunction::~MFunction()
{
	for (auto bb : blocks_) delete bb;
	for (auto [i, j] : ba_cache_) delete j;
	for (auto i : stack_) delete i;
	for (auto i : fix_) delete i;
}

MFunction::MFunction(std::string name, MModule* module)
	: module_(module),
	  name_(std::move(name))
{
	id_ = static_cast<int>(module_->allFuncs_.size());
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
	size_t size = ty->sizeInBitsInArm64();
	FrameIndex* index = new FrameIndex{this, static_cast<unsigned int>(stack_.size()), size, true};
	stack_.emplace_back(index);
	return index;
}

FrameIndex* MFunction::allocaStack(short size)
{
	FrameIndex* index = new FrameIndex{this, static_cast<unsigned int>(stack_.size()), static_cast<size_t>(size), true};
	stack_.emplace_back(index);
	return index;
}

FrameIndex* MFunction::getFix(int idx) const
{
	return fix_[idx];
}

FrameIndex* MFunction::allocaFix(Value* value)
{
	auto a = dynamic_cast<AllocaInst*>(value);
	auto ty = a->get_alloca_type();
	size_t size = ty->sizeInBitsInArm64();
	FrameIndex* index = new FrameIndex{this, static_cast<unsigned int>(fix_.size()), size, false};
	fix_.emplace_back(index);
	return index;
}

void MFunction::preprocess(Function* function)
{
	int ic = 0;
	int fc = 0;
	for (auto& argC : function->get_args())
	{
		auto argv = &argC;
		if (argv->get_type() != Types::FLOAT)
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
		allocaFix(argv);
	}
}

void MFunction::accept(Function* function, std::map<Function*, MFunction*>& funcMap, std::map<GlobalVariable*, GlobalAddress*>& global_address)
{
	const auto& bbs = function->get_basic_blocks();
	std::map<BasicBlock*, MBasicBlock*> cache;
	MBasicBlock* preMB = nullptr;
	unsigned int bbc = 0;
	for (const auto& bb : bbs)
	{
		const auto mbb = MBasicBlock::createBasicBlock(name_ + "_" + bb->get_name(), this);
		mbb->id_ = bbc++;
		blocks_.emplace_back(mbb);
		cache.emplace(bb, mbb);
		if (preMB != nullptr)
			preMB->next_ = mbb;
		preMB = mbb;
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
	entryMBB->instructions_.emplace_back(new MCopy{entryMBB, Register::getLR(module_), lrGuard_, 64});

	int ic = 0;
	int fc = 0;
	int idx = 0;
	for (auto& argC : function->get_args())
	{
		auto argv = &argC;
		if (argv->get_type() != Types::FLOAT)
		{
			if (ic < 8)
			{
				auto cp = new MCopy{
					entryMBB, Register::getIParameterRegister(ic, module_),
					VirtualRegister::createVirtualIRegister(
						this, static_cast<short>(argC.get_type()->sizeInBitsInArm64())),
					argC.get_type()->sizeInBitsInArm64()
				};
				entryMBB->instructions_.emplace_back(cp);
				++ic;
				continue;
			}
		}
		else
		{
			if (fc < 8)
			{
				auto cp = new MCopy{
					entryMBB, Register::getFParameterRegister(fc, module_),
					VirtualRegister::createVirtualFRegister(
						this, static_cast<short>(argC.get_type()->sizeInBitsInArm64())),
					argC.get_type()->sizeInBitsInArm64()
				};
				entryMBB->instructions_.emplace_back(cp);
				++fc;
				continue;
			}
		}
		auto val = getOperandFor(argv, opMap);
		auto vr = dynamic_cast<VirtualRegister*>(val);
		assert(vr != nullptr);
		auto frameIdx = getFix(idx++);
		vr->replacePrefer_ = frameIdx;
		auto cp = new MCopy{entryMBB, frameIdx, val, argC.get_type()->sizeInBitsInArm64()};
		entryMBB->instructions_.emplace_back(cp);
	}
	LoopDetection detection{function->get_parent()};
	detection.only_run_on_func(function);

	for (auto loop : detection.get_loops())
	{
		for (auto bb : loop->get_blocks())
		{
			cache[bb]->weight_ *= 10;
		}
	}

	map<MBasicBlock*, list<MCopy*>> phiMap;

	entryMBB->acceptAllocaInsts(entryBB, opMap, cache);

	for (auto& [glob, mglob] : global_address)
	{
		auto& uses = glob->get_use_list();
		int useCount = 0;
		for (auto& i : uses)
		{
			auto user = dynamic_cast<Instruction*>(i.val_);
			if (user->get_parent()->get_parent() == function)
				useCount++;
		}
		if (useCount >= replaceGlobalAddressWithRegisterNeedUseCount)
		{
			auto reg = VirtualRegister::createVirtualIRegister(this, 64);
			reg->replacePrefer_ = mglob;
			entryMBB->instructions_.emplace_back(new MCopy{entryMBB, mglob, reg, 64});
			opMap[glob] = reg;
		}
		else if (useCount > 0) opMap[glob] = mglob;
	}

	for (auto& [bb, mbb] : cache)
	{
		mbb->accept(bb, opMap, cache, phiMap, funcMap);
	}
	for (auto& [mbb, cp] : phiMap)
	{
		mbb->mergePhiCopies(cp);
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
		unsigned i = 0;
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
				this, static_cast<short>(value->get_type()->sizeInBitsInArm64()));
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
				this, static_cast<short>(value->get_type()->sizeInBitsInArm64()));
		}
	}
	opMap.emplace(value, operand);
	return operand;
}

void MFunction::spill(VirtualRegister* vreg, LiveMessage* message)
{
	vreg->spilled = true;
	if (vreg->replacePrefer_ != nullptr)
	{
		auto entry = blocks_[0];
		auto& inst = entry->instructions();
		int size = static_cast<int>(inst.size());
		for (int i = 0; i < size; i++)
		{
			auto in = dynamic_cast<MCopy*>(inst[i]);
			if (in != nullptr && in->def(0) == vreg)
			{
				inst.erase(inst.begin() + i);
				removeUse(vreg, in);
				delete in;
				break;
			}
		}
		replaceAllOperands(vreg, vreg->replacePrefer_);
		return;
	}
	auto to = allocaStack(vreg->size());
	to->spilledFrame_ = true;
	VirtualRegister* nreg = vreg;
	for (auto bb : blocks())
	{
		auto& instructions = bb->instructions();
		int size = static_cast<int>(instructions.size());
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
				auto ld = new MLDR{bb, nreg, to, static_cast<unsigned int>(nreg->size())};
				instructions.emplace(instructions.begin() + i, ld);
				size++;
				i++;
			}
			if (haveDef)
			{
				auto ld = new MSTR{bb, nreg, to, static_cast<unsigned int>(nreg->size())};
				instructions.emplace(instructions.begin() + (i + 1), ld);
				size++;
				i++;
			}
			nreg = nullptr;
		}
	}
}

namespace
{
	struct FrameScore
	{
		FrameIndex* frame_;
		float score_;

		friend bool operator>(const FrameScore& lhs, const FrameScore& rhs)
		{
			return lhs.score_ > rhs.score_;
		}
	};

	int upAlignTo(int of, int s)
	{
		if (s > alignTo16NeedBytes) return ((of + 15) >> 4) << 4;
		if (s > 8) s = 8;
		const static int u[9] = {0, 0, 1, 3, 3, 7, 7, 7, 7};
		const static int t[9] = {0, 0, 1, 2, 2, 3, 3, 3, 3};
		return ((of + u[s]) >> t[s]) << t[s];
	}

	int upAlignTo16(int of)
	{
		return ((of + 15) >> 4) << 4;
	}
}


void MFunction::rankFrameIndexesAndCalculateOffsets()
{
	vector<FrameScore> scores;
	scores.reserve(stack_.size());
	for (auto i : stack_)
		scores.emplace_back(FrameScore{i, 0.0f});
	for (auto i : blocks())
	{
		for (auto inst : i->instructions())
		{
			for (auto op : inst->operands())
			{
				if (auto frame = dynamic_cast<FrameIndex*>(op); frame != nullptr && frame->stack_t_fix_f_)
				{
					auto& score = scores[frame->index_];
					score.score_ += i->weight();
				}
			}
		}
	}
	for (auto& i : scores)
		i.score_ /= static_cast<float>(i.frame_->size_);
	sort(scores.begin(), scores.end(), greater());
	int size = static_cast<int>(scores.size());
	for (int i = 0; i < size; i++)
	{
		scores[i].frame_->index_ = i;
		stack_[i] = scores[i].frame_;
	}

	int of = 0;
	for (auto i : stack_)
	{
		int s = static_cast<int>(i->size()) >> 3;
		i->offset_ = upAlignTo(of, s);
		of = i->offset_ + s;
	}

	DynamicBitset b{module_->RegisterCount()};
	for (auto bb : blocks_)
	{
		for (auto inst : bb->instructions_)
		{
			if (inst->def().empty() == false)
			{
				Register* r = dynamic_cast<Register*>(inst->def(0));
				if (r != nullptr && r->calleeSave_)
				{
					b.set(r->isIntegerRegister() ? r->id() : (r->id() + module_->IRegisterCount()));
				}
			}
		}
	}
	int ic = static_cast<int>(module()->IRegisterCount());
	for (auto i : b)
	{
		int next = ((of + 7) >> 3) << 3;
		if (static_cast<int>(i) >= ic)
			calleeSaved.emplace_back(module()->fregs_[i - ic], next);
		else calleeSaved.emplace_back(module()->iregs_[i], next);
		of = next + 8;
	}
	stackMoveOffset_ = upAlignTo16(of);
	of = stackMoveOffset_;
	for (auto it = fix_.rbegin(), e = fix_.rend(); it != e; ++it)
	{
		auto i = *it;
		int s = static_cast<int>(i->size()) >> 3;
		i->offset_ = upAlignTo(of, s);
		of = i->offset_ + s;
	}
	fixMoveOffset_ = upAlignTo16(of) - stackMoveOffset_;
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

void MFunction::rewriteDestroyRegs() const
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
				if (df == nullptr)continue;
				int id = static_cast<int>(df->isIntegerRegister() ? df->id() : df->id() + module_->IRegisterCount());
				destroyRegs_.set(id);
			}
		}
	}
}

std::unordered_map<MOperand*, std::unordered_set<MInstruction*>>& MFunction::useList()
{
	return useList_;
}

MModule::MModule()
{
	Register* r;
	for (unsigned i = 0; i < 8; i++)
	{
		// 0 - 7
		r = new Register{i, true, "X" + to_string(i), "W" + to_string(i)};
		r->canAllocate_ = true;
		r->callerSave_ = true;
		iregs_.emplace_back(r);
	}
	for (unsigned i = 0; i < 7; i++)
	{
		// 8 - 14
		r = new Register{i + 8, true, "X" + to_string(i + 9), "W" + to_string(i + 9)};
		r->canAllocate_ = true;
		r->callerSave_ = true;
		iregs_.emplace_back(r);
	}
	for (unsigned i = 0; i < 2; i++)
	{
		// 15 - 16 IP0/IP1
		r = new Register{i + 15, true, "X" + to_string(i + 16), "W" + to_string(i + 16)};
		iregs_.emplace_back(r);
	}
	for (unsigned i = 0; i < 10; i++)
	{
		// 17 - 26
		r = new Register{i + 17, true, "X" + to_string(i + 19), "W" + to_string(i + 19)};
		r->canAllocate_ = true;
		r->calleeSave_ = true;
		iregs_.emplace_back(r);
	}
	// (FP, 不过我们不使用帧指针)
	r = new Register{27, true, "X29", "W29"};
	r->canAllocate_ = true;
	r->calleeSave_ = true; // 调用对象保存, 我们会保存调用我们函数的对象的 FP, 如果它使用了帧指针, 其不会被破坏
	iregs_.emplace_back(r);
	// (LR, 需要时保存, 不需要时无需保存, 保存后可当作一般寄存器来分配)
	r = new Register{28, true, "X30", "W30"};
	r->canAllocate_ = true;
	r->callerSave_ = true; // 调用者保存(考虑到进行函数调用会破坏它), 作为代价, 使用一个 vreg 来防止其被破坏
	iregs_.emplace_back(r);
	r = new Register{29, true, "XZR", "WZR"};
	iregs_.emplace_back(r);
	r = new Register{30, true, "SP", "SP"};
	iregs_.emplace_back(r);
	r = new Register{31, true, "NZCV", "NZCV"};
	iregs_.emplace_back(r);
	for (unsigned i = 0; i < 8; i++)
	{
		r = new Register{i, false, "Q" + to_string(i), "S" + to_string(i)};
		r->canAllocate_ = true;
		r->callerSave_ = true;
		fregs_.emplace_back(r);
	}
	for (unsigned i = 8; i < 16; i++)
	{
		r = new Register{i, false, "Q" + to_string(i), "S" + to_string(i)};
		r->canAllocate_ = true;
		r->calleeSave_ = true;
		fregs_.emplace_back(r);
	}
	for (unsigned i = 16; i < 18; i++)
	{
		r = new Register{i, false, "Q" + to_string(i), "S" + to_string(i)};
		fregs_.emplace_back(r);
	}
	for (unsigned i = 18; i < 32; i++)
	{
		r = new Register{i, false, "Q" + to_string(i), "S" + to_string(i)};
		r->canAllocate_ = true;
		r->callerSave_ = true;
		fregs_.emplace_back(r);
	}
	memcpy_ = MFunction::createFunc("__memcpy__", this);
	memcpy_->destroyRegs_ = DynamicBitset{RegisterCount()};
	for (int i = 0; i < 4; i++)
		memcpy_->destroyRegs_.set(i);
	for (int i = 0; i < 4; i++)
		memcpy_->destroyRegs_.set(i + IRegisterCount());
	memclr_ = MFunction::createFunc("__memclr__", this);
	memclr_->destroyRegs_ = DynamicBitset{RegisterCount()};
	for (int i = 0; i < 3; i++)
		memclr_->destroyRegs_.set(i);
	for (int i = 0; i < 4; i++)
		memclr_->destroyRegs_.set(i + IRegisterCount());
}

void MModule::accept(Module* module)
{
	module->set_print_name();
	std::map<GlobalVariable*, GlobalAddress*> global_address;
	for (auto& g : module->get_global_variable())
	{
		global_address.emplace(g, new GlobalAddress{this, g});
	}
	const auto& funcs = module->get_functions();
	std::map<Function*, MFunction*> cache;
	for (const auto& func : funcs)
	{
		auto* mfunc = MFunction::createFunc(func->get_name(), this);
		if (func->is_lib_)
			lib_functions_.emplace_back(mfunc);
		else functions_.emplace_back(mfunc);
		mfunc->destroyRegs_ = DynamicBitset{RegisterCount()};
		for (int i = 0; i < 32; i++)
			if (iregs_[i]->callerSave_)
				mfunc->destroyRegs_.set(i);
		for (int i = 0; i < 32; i++)
			if (fregs_[i]->callerSave_)
				mfunc->destroyRegs_.set(i + IRegisterCount());
		cache.emplace(func, mfunc);
	}

	for (auto& f : allFuncs_) f->called_ = DynamicBitset{static_cast<unsigned>(allFuncs_.size())};

	for (auto& [bb,mbb] : cache)
	{
		if (bb->is_lib_) continue;
		mbb->preprocess(bb);
	}
	for (auto& [bb, mbb] : cache)
	{
		if (bb->is_lib_) continue;
		mbb->accept(bb, cache,global_address);
	}
}

std::string MModule::print() const
{
	string ret;
	for (auto& func : functions_)
	{
		ret += func->print() + "\n";
	}
	if (!ret.empty()) ret.pop_back();
	return ret;
}

unsigned MModule::IRegisterCount() const
{
	return static_cast<unsigned>(iregs_.size());
}

unsigned MModule::FRegisterCount() const
{
	return static_cast<unsigned>(fregs_.size());
}

unsigned MModule::RegisterCount() const
{
	return static_cast<unsigned>(iregs_.size() + fregs_.size());
}

const std::vector<Register*>& MModule::IRegs() const
{
	return iregs_;
}

const std::vector<Register*>& MModule::FRegs() const
{
	return fregs_;
}

const std::vector<VirtualRegister*>& MModule::IVRegs() const
{
	return virtual_iregs_;
}

const std::vector<VirtualRegister*>& MModule::FVRegs() const
{
	return virtual_fregs_;
}

std::vector<GlobalAddress*> MModule::constGlobalAddresses() const
{
	vector<GlobalAddress*> v;
	for (auto i : globals_)
	{
		if (i->const_) v.emplace_back(i);
	}
	return v;
}

std::vector<GlobalAddress*> MModule::ncnzGlobalAddresses() const
{
	vector<GlobalAddress*> v;
	for (auto i : globals_)
	{
		if (!i->const_ && (i->data_->segmentCount() > 1 || !i->data_->segmentIsDefault(0))) v.emplace_back(i);
	}
	return v;
}

std::vector<GlobalAddress*> MModule::ncZeroGlobalAddresses() const
{
	vector<GlobalAddress*> v;
	for (auto i : globals_)
	{
		if (!i->const_ && i->data_->segmentCount() > 1 && i->data_->segmentIsDefault(0)) v.emplace_back(i);
	}
	return v;
}

unsigned MFunction::virtualIRegisterCount() const
{
	return virtual_iregs_;
}

unsigned MFunction::virtualFRegisterCount() const
{
	return virtual_fregs_;
}

unsigned MFunction::virtualRegisterCount() const
{
	return virtual_iregs_ + virtual_fregs_;
}

MModule::~MModule()
{
	for (auto i : globals_) delete i;
	for (auto i : allFuncs_) delete i;
	for (auto [i,j] : func_address_) delete j;
	for (auto [i, j] : imm_cache_) delete j;
	for (auto i : virtual_iregs_) delete i;
	for (auto i : virtual_fregs_) delete i;
	for (auto i : iregs_) delete i;
	for (auto i : fregs_) delete i;
}

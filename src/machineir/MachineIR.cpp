#include "MachineIR.hpp"

#include <stdexcept>

#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Instruction.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include "Module.hpp"
#include "Type.hpp"

using namespace std;

MFunction::MFunction(std::string name, MModule* module)
	: module_(module),
	  name_(std::move(name))
{
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

void MFunction::accept(Function* function, std::map<Function*, MFunction*>& funcMap)
{
	const auto& bbs = function->get_basic_blocks();
	std::map<BasicBlock*, MBasicBlock*> cache;
	MBasicBlock* preMB = nullptr;
	for (const auto& bb : bbs)
	{
		const auto mbb = MBasicBlock::createBasicBlock(name_ + "_" + bb->get_name(), this);
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
	int ic = 0;
	int fc = 0;
	int idx = 0;
	for (auto& argC : function->get_args())
	{
		auto argv = &argC;
		auto val = module_->getOperandFor(argv, opMap);
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
		auto frameIdx = getFix(idx++);
		auto cp = new MCopy{entryMBB, frameIdx, val, argC.get_type()->sizeInBitsInArm64()};
		entryMBB->instructions_.emplace_back(cp);
	}

	map<MBasicBlock*, list<MCopy*>> phiMap;
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

MModule::MModule()
{
	for (unsigned i = 0; i < 32; i++)
	{
		iregs_.emplace_back(new Register{i, true});
		fregs_.emplace_back(new Register{i, false});
	}
	for (unsigned i = 32; i < 33; i++)
	{
		// 特殊寄存器
		iregs_.emplace_back(new Register{i, true});
	}
	memcpy_ = MFunction::createFunc( "__memcpy__", this );
	memclr_ = MFunction::createFunc("__memclr__", this);
}

void MModule::accept(Module* module)
{
	const auto& funcs = module->get_functions();
	std::map<Function*, MFunction*> cache;
	for (const auto& func : funcs)
	{
		auto* mfunc = MFunction::createFunc(func->get_name(), this);
		if (func->is_lib_)
			lib_functions_.emplace_back(mfunc);
		else functions_.emplace_back(mfunc);
		cache.emplace(func, mfunc);
	}
	for (auto& [bb,mbb] : cache)
	{
		if (bb->is_lib_) continue;
		mbb->preprocess(bb);
	}
	for (auto& [bb, mbb] : cache)
	{
		if (bb->is_lib_) continue;
		mbb->accept(bb, cache);
	}
}

MOperand* MModule::getOperandFor(Value* value, std::map<Value*, MOperand*>& opMap)
{
	auto fd = opMap.find(value);
	if (fd != opMap.end()) return fd->second;
	MOperand* operand = nullptr;
	if (value->get_type() == Types::INT)
	{
		auto imm = dynamic_cast<Constant*>(value);
		if (imm != nullptr)
		{
			operand = Immediate::getImmediate(imm->getIntConstant(), this);
		}
		else
			operand = VirtualRegister::createVirtualIRegister(this);
	}
	else
	{
		auto imm = dynamic_cast<Constant*>(value); if (imm != nullptr)
		{
			operand = Immediate::getImmediate(imm->getFloatConstant(), this);
		}
		else
			operand = VirtualRegister::createVirtualFRegister(this);
	}
	opMap.emplace(value, operand);
	return operand;
}

#include "RegPrefill.hpp"

#include "CodeGen.hpp"
#include "Config.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include "FrameOffset.hpp"

#define DEBUG 0
#include "Util.hpp"

using namespace std;

void RegPrefill::runInner() const
{
	LOG(color::cyan("Prefill Register of Func ") + f_->name());
	// 常规 SPILL -> MOV + LDR
	// 全局变量 -> ADR + LDR
	for (auto glob : m_->globalAddresses())
	{
		auto& useList = f_->useList(glob);
		float weight = 0;
		for (auto use : useList)
		{
			weight += use->block()->weight_;
		}
		if (weight >= static_cast<float>(replaceGlobalAddressWithRegisterNeedUseCount))
		{
			auto reg = VirtualRegister::createVirtualIRegister(f_, 64);
			f_->replaceAllOperands(glob, reg);
			auto cp = new MCopy{f_->block(0), glob, reg, 64};
			reg->replacePrefer_ = glob;
			reg->spillCost_ = globalRegisterSpillPriority;
			LOG(color::green("Replace GlobalAddress ") + glob->print() + color::green(" with ") + reg->print());
			LOG(color::yellow("Priority ") + to_string(globalRegisterSpillPriority));
			f_->block(0)->addInstBeforeLast(cp);
		}
	}

	// 加载的栈参数 -> (MOV) + LDR
	// 这种方式能不能起到效果存疑
	for (auto arg : f_->fixFrames())
	{
		if (arg->tiedWith_ != nullptr)
		{
			if(useStackOffset2GetspillCost){
			float cost = static_cast<float>(CodeGen::ldrNeedInstCount(arg->offset(), u2iNegThrow(arg->size()))) * 0.2f;
			arg->tiedWith_->spillCost_ = cost * fixFrameIndexParameterRegisterSpillPriority;
			LOG(color::green("Set Arg ")+ arg->print() + color::green(" Priority ") + to_string(cost));
			}
			else{
				arg->tiedWith_->spillCost_ = fixFrameIndexParameterRegisterSpillPriority;
			LOG(color::green("Set Arg ")+ arg->print() + color::green(" Priority ") + to_string(fixFrameIndexParameterRegisterSpillPriority));
			}
		}
	}

	// 加载的栈参数地址 -> MOV / ADD
	for (auto arg : f_->stackFrames())
	{
		auto& useList = f_->useList(arg);
		float weight = 0;
		for (auto use : useList)
		{
			weight += use->block()->weight_;
		}
		if(useStackOffset2GetspillCost){
			float cost = static_cast<float>(CodeGen::copyFrameNeedInstCount(arg->offset())) * 0.2f;
		if (cost == 0.0f) continue;
		if (weight * cost >= static_cast<float>(replaceAllocaAddressWithRegisterNeedTotalCost))
		{
			auto reg = VirtualRegister::createVirtualIRegister(f_, 64);
			f_->replaceAllOperands(arg, reg);
			auto cp = new MCopy{f_->block(0), arg, reg, 64};
			reg->replacePrefer_ = arg;
			reg->spillCost_ = cost;
			LOG(color::green("Replace StackFrame ") + arg->print() + color::green(" with ") + reg->print());
			LOG(color::yellow("Priority ") + to_string(reg->spillCost_));
			f_->block(0)->addInstBeforeLast(cp);
		}
		}
		else{
			if(weight >= static_cast<float>(replaceAllocaAddressWithRegisterNeedUseCount)){
				auto reg = VirtualRegister::createVirtualIRegister(f_, 64);
			f_->replaceAllOperands(arg, reg);
			auto cp = new MCopy{f_->block(0), arg, reg, 64};
			reg->replacePrefer_ = arg;
			if(arg->size_ >= bigAllocaVariableGate){
				reg->spillCost_ = bigAllocaRegisterSpillPriority;
			}
			else
			 reg->spillCost_ = smallAllocaRegisterSpillPriority;
			LOG(color::green("Replace StackFrame ") + arg->print() + color::green(" with ") + reg->print());
			LOG(color::yellow("Priority ") + to_string(reg->spillCost_));
			f_->block(0)->addInstBeforeLast(cp);
			}
		}
	}

	// 加载的常数 -> MOV / LDR
	for (auto [val, constant] : m_->immediates())
	{
		auto useList = f_->useList(constant);
		float weight = 0;
		for (auto use : useList) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
		{
			if (auto mth = dynamic_cast<MMathInst*>(use); mth != nullptr)
			{
				if (CodeGen::mathInstImmediateCanInline(mth->operand(1), mth->operand(2), mth->op(), mth->width_))
					continue;
			}
			else if (auto cp = dynamic_cast<MCopy*>(use); cp != nullptr)
			{
				if (CodeGen::makeI64ImmediateNeedInstCount(ULL2LLKeepBits(val)) <= 1) continue;
			}
			weight += use->block()->weight_;
		}
		float cost = static_cast<float>(CodeGen::makeI64ImmediateNeedInstCount(ULL2LLKeepBits(val))) * 0.2f;
		if (weight * cost >= static_cast<float>(prefillConstantNeedTotalCost))
		{
			int loadLen = ((val & 0xFFFFFFFF00000000) == 0) ? 32 : 64;
			auto reg = VirtualRegister::createVirtualIRegister(f_, loadLen);
			for (auto use : useList) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
			{
				if (auto mth = dynamic_cast<MMathInst*>(use); mth != nullptr)
				{
					if (CodeGen::mathInstImmediateCanInline(mth->operand(1), mth->operand(2), mth->op(), mth->width_))
						continue;
				}
				else if (auto cp = dynamic_cast<MCopy*>(use); cp != nullptr)
				{
					if (CodeGen::makeI64ImmediateNeedInstCount(ULL2LLKeepBits(val)) <= 1) continue;
				}
				use->replace(constant, reg, f_);
			}
			auto cp = new MCopy{f_->block(0), constant, reg, loadLen};
			reg->replacePrefer_ = constant;
			reg->spillCost_ = cost;
			LOG(color::green("Replace Constant ") + constant->print() + color::green(" with ") + reg->print());
			LOG(color::yellow("Priority ") + to_string(reg->spillCost_));
			f_->block(0)->addInstBeforeLast(cp);
		}
	}
}

void RegPrefill::run()
{
	FrameOffset* fo = new FrameOffset{m_};
	fo->run();
	delete fo;
	for (auto f : m_->functions())
	{
		f_ = f;
		runInner();
	}
	GAP;
	LOG(m_->print());
	GAP;
}

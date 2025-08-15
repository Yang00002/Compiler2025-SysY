#include "RegPrefill.hpp"

#include "CodeGen.hpp"
#include "Config.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include "Util.hpp"
#include "FrameOffset.hpp"

using namespace std;

void RegPrefill::runInner() const
{
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
			auto cp = new MCopy{f_->block(0), glob, reg, 64};
			reg->replacePrefer_ = glob;
			reg->spillCost_ = globalRegisterSpillPriority;
			f_->replaceAllOperands(glob, reg);
			f_->block(0)->addInstBeforeLast(cp);
		}
	}

	// 加载的栈参数 -> (MOV) + LDR
	// TODO 这种方式能不能起到效果存疑
	for (auto arg : f_->fixFrames())
	{
		if (arg->tiedWith_ != nullptr)
		{
			float cost = static_cast<float>(CodeGen::ldrNeedInstCount(arg->offset(), u2iNegThrow(arg->size()))) * 0.2f;
			arg->tiedWith_->spillCost_ = cost;
		}
	}

	// 加载的栈参数地址 -> MOV / ADD
	for (auto arg : f_->stackFrames())
	{
		float cost = static_cast<float>(CodeGen::copyFrameNeedInstCount(arg->offset())) * 0.2f;
		if (cost == 0.0f) continue;
		auto& useList = f_->useList(arg);
		float weight = 0;
		for (auto use : useList)
		{
			weight += use->block()->weight_;
		}
		if (weight >= static_cast<float>(replaceAllocaAddressWithRegisterNeedUseCount))
		{
			auto reg = VirtualRegister::createVirtualIRegister(f_, 64);
			auto cp = new MCopy{f_->block(0), arg, reg, 64};
			reg->replacePrefer_ = arg;
			reg->spillCost_ = cost;
			f_->replaceAllOperands(arg, reg);
			f_->block(0)->addInstBeforeLast(cp);
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
			weight += use->block()->weight_;
		}
		if (weight >= static_cast<float>(prefillConstantNeedWeight))
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
				use->replace(constant, reg, f_);
			}
			auto cp = new MCopy{f_->block(0), constant, reg, loadLen };
			reg->replacePrefer_ = constant;
			reg->spillCost_ = static_cast<float>(CodeGen::makeI64ImmediateNeedInstCount(ULL2LLKeepBits(val))) * 0.2f;
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
}

#include "RegisterAllocate.hpp"

#include "Config.hpp"
#include "MachineOperand.hpp"
#include "MachineFunction.hpp"
#include "MachineDominator.hpp"

#define DEBUG 0
#include <algorithm>

#include "MachineBasicBlock.hpp"
#include "Util.hpp"

// toStr: SPILL 后的一些 LDR 不是必须的
// 例如形如 STR X1 %F; LDR X2 %F 可以被替代为 STR X1 %F; MOV X2 X1

void RegisterAllocate::runOn(MFunction* function)
{
	RUN(function->checkValidUseList());
	dominator_->run_on_func(function);
	LiveMessage liveMessage{function};
	liveMessage_ = &liveMessage;
	currentFunc_ = function;
	if (function->virtualFRegisterCount() > 0)
	{
		auto c = function->virtualFRegisterCount();
		liveMessage.flush(false);
		if (useCallerSaveRegsFirst)
		{
			for (auto reg : module_->FRegs())
				if (reg->canAllocate() && reg->callerSave_)
					liveMessage.addRegister(reg);
			for (auto reg : module_->FRegs())
				if (reg->canAllocate() && !reg->callerSave_)
					liveMessage.addRegister(reg);
		}
		else
		{
			for (auto reg : module_->FRegs())
				if (reg->canAllocate() && !reg->callerSave_)
					liveMessage.addRegister(reg);
			for (auto reg : module_->FRegs())
				if (reg->canAllocate() && reg->callerSave_)
					liveMessage.addRegister(reg);
		}
		for (auto reg : function->FVRegs())
			if (reg->id() < c)
				liveMessage.addRegister(reg);
			else break;
		while (true)
		{
			liveMessage.calculateLiveMessage();
			interfereGraph_.flush();
			interfereGraph_.build();
			interfereGraph_.makeWorklist();
			do
			{
				if (interfereGraph_.simplify())continue;
				if (interfereGraph_.coalesce())continue;
				if (interfereGraph_.freeze())continue;
				interfereGraph_.selectSpill();
			}
			while (interfereGraph_.shouldRepeat());
			interfereGraph_.assignColors();
			if (interfereGraph_.needRewrite())
				interfereGraph_.rewriteProgram();
			else
			{
				interfereGraph_.applyChanges();
				break;
			}
		}
	}
	if (function->virtualIRegisterCount() > 0)
	{
		auto c = function->virtualIRegisterCount();
		liveMessage.flush(true);
		if (useCallerSaveRegsFirst)
		{
			for (auto reg : module_->IRegs())
				if (reg->canAllocate() && reg->callerSave_)
					liveMessage.addRegister(reg);
			for (auto reg : module_->IRegs())
				if (reg->canAllocate() && !reg->callerSave_)
					liveMessage.addRegister(reg);
		}
		else
		{
			for (auto reg : module_->IRegs())
				if (reg->canAllocate() && !reg->callerSave_)
					liveMessage.addRegister(reg);
			for (auto reg : module_->IRegs())
				if (reg->canAllocate() && reg->callerSave_)
					liveMessage.addRegister(reg);
		}
		for (auto reg : function->IVRegs())
			if (reg->id() < c)
				liveMessage.addRegister(reg);
			else break;
		while (true)
		{
			liveMessage.calculateLiveMessage();
			interfereGraph_.flush();
			interfereGraph_.build();
			interfereGraph_.makeWorklist();
			do
			{
				if (interfereGraph_.simplify())continue;
				if (interfereGraph_.coalesce())continue;
				if (interfereGraph_.freeze())continue;
				interfereGraph_.selectSpill();
			}
			while (interfereGraph_.shouldRepeat());
			interfereGraph_.assignColors();
			if (interfereGraph_.needRewrite())
				interfereGraph_.rewriteProgram();
			else
			{
				interfereGraph_.applyChanges();
				break;
			}
		}
	}

	if (mergeStackFrameSpilledWithGraphColoring)
	{
		FrameLiveMessage flm{function};
		flm.calculateLiveMessage();
		calculateFrameInterfereGraph(this, flm);
	}
	RUN(function->checkValidUseList());
}

void RegisterAllocate::spillWithFR(MFunction* function) const
{
	RUN(function->checkValidUseList());
	std::vector<Register*> r64;
	std::vector<std::pair<Register*, bool>> rltrf32;
	for (auto freg : module_->FRegs())
	{
		if (freg->canAllocate() && freg->calleeSave_ && !function->destroy_regs().test(
			    freg->id() + module_->IRegisterCount()))
		{
			r64.emplace_back(freg);
		}
	}
	std::vector<std::pair<FrameIndex*, float>> spilledFrames;
	for (auto stack_frame : function->stackFrames())
	{
		float cost = 0;
		if (stack_frame->spilledFrame_)
		{
			for (auto use : function->useList(stack_frame))
			{
				cost += use->block()->weight_;
			}
		}
		// 64 位保存更难
		if (stack_frame->size() == 64) cost /= 2;
		spilledFrames.emplace_back(stack_frame, cost);
	}
	std::sort(spilledFrames.begin(), spilledFrames.end(),
	          [](const std::pair<FrameIndex*, float>& l, const std::pair<FrameIndex*, float>& r)-> bool
	          {
		          return l.second > r.second;
	          });
	int size = u2iNegThrow(spilledFrames.size());
	std::map<FrameIndex*, std::pair<Register*, bool>> rpm;
	for (int i = 0; i < size; i++)
	{
		auto f = spilledFrames[i].first;
		if (f == nullptr) continue;
		if (f->size() == 64)
		{
			if (r64.empty())
			{
				spilledFrames[i] = {};
				continue;
			}
			auto r = r64.back();
			r64.pop_back();
			rpm.emplace(f, std::pair{r, false});
		}
		else
		{
			if (rltrf32.empty())
			{
				if (!r64.empty())
				{
					auto r = r64.back();
					r64.pop_back();
					rpm.emplace(f, std::pair{r, false});
					rltrf32.emplace_back(r, true);
				}
				spilledFrames[i] = {};
				continue;
			}
			auto r = rltrf32.back();
			rltrf32.pop_back();
			rpm.emplace(f, r);
		}
	}
	for (auto bb : function->blocks())
	{
		auto& instructions = bb->instructions();
		int isize = u2iNegThrow(instructions.size());
		for (int i = 0; i < isize; i++)
		{
			auto inst = instructions[i];
			if (auto st = dynamic_cast<MSTR*>(inst); st != nullptr)
			{
				auto stackLike = dynamic_cast<FrameIndex*>(st->operand(1));
				if (stackLike != nullptr && rpm.count(stackLike))
				{
					auto freg = rpm[stackLike];
					auto regLike = st->operand(0);
					auto simd = new M2SIMDCopy{
						inst->block(), regLike, freg.first, u2iNegThrow(st->width()), freg.second ? 0 : 1, false
					};
					instructions[i] = simd;
					st->removeAllUse();
					delete st;
				}
			}
			else if (auto ld = dynamic_cast<MLDR*>(inst); ld != nullptr)
			{
				auto stackLike = dynamic_cast<FrameIndex*>(ld->operand(1));
				if (stackLike != nullptr && rpm.count(stackLike))
				{
					auto freg = rpm[stackLike];
					auto regLike = ld->operand(0);
					auto simd = new M2SIMDCopy{
						inst->block(), regLike, freg.first, u2iNegThrow(ld->width()), freg.second ? 0 : 1, true
					};
					instructions[i] = simd;
					ld->removeAllUse();
					delete ld;
				}
			}
			else
			{
				for (auto use : inst->use())
				{
					if (rpm.count(dynamic_cast<FrameIndex*>(inst->operand(use))))
					{
						instructions.erase(instructions.begin() + i);
						inst->removeAllUse();
						delete inst;
						i--;
						isize--;
					}
				}
			}
		}
	}
	for (auto& [i,j] : rpm)
	{
		int id = j.first->isIntegerRegister() ? j.first->id() : j.first->id() + module_->IRegisterCount();
		function->destroy_regs().set(id);
	}
	size = u2iNegThrow(function->stackFrames().size());
	auto& fms = function->stackFrames();
	for (int i = 0; i < size; i++)
	{
		auto c = fms[i];
		if (rpm.count(c))
		{
			fms.erase(fms.begin() + i);
			i--;
			size--;
			function->useList().erase(c);
			delete c;
		}
	}
	RUN(function->checkValidUseList());
}

RegisterAllocate::RegisterAllocate(MModule* module): MachinePass(module), interfereGraph_(this), module_(module),
                                                     dominator_(new MachineDominators(m_))
{
}

void RegisterAllocate::run()
{
	auto& funcs = module_->all_funcs();
	std::vector<DynamicBitset> callChain;
	callChain.resize(funcs.size());
	int idx = 0;
	for (auto all_func : funcs)
	{
		callChain[idx++] = all_func->called();
	}

	DynamicBitset leaf{(idx)};
	DynamicBitset workList{(idx)};
	workList.rangeSet(0, (idx));
	while (idx > 0)
	{
		bool changed = false;
		for (auto i : workList)
		{
			if (leaf.include(callChain[i]))
			{
				idx--;
				changed = true;
				workList.reset((i));
				leaf.set((i));
				auto func = funcs[i];
				if (func->blocks().empty()) break;
				func->rewriteCallsDefList();
				runOn(func);
				func->rewriteDestroyRegs();
				if (useFloatRegAsStack2Spill) spillWithFR(func);
				break;
			}
		}
		if (!changed) break;
	}
	for (auto i : workList)
	{
		auto func = funcs[i];
		func->rewriteCallsDefList();
		runOn(func);
	}
	delete dominator_;
	GAP;
	LOG(m_->print());
	GAP;
}

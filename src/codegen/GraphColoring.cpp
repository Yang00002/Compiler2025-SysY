#include "GraphColoring.hpp"

#include "Config.hpp"
#include "MachineOperand.hpp"
#include "MachineIR.hpp"

// TODO: SPILL 后的一些 LDR 不是必须的
// 例如形如 STR X1 %F; LDR X2 %F 可以被替代为 STR X1 %F; MOV X2 X1

void GraphColorSolver::runOn(MFunction* function)
{
	LiveMessage liveMessage{function};
	liveMessage_ = &liveMessage;
	currentFunc_ = function;
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
	if (mergeStackFrameSpilledWithGraphColoring)
	{
		FrameLiveMessage flm{function};
		flm.calculateLiveMessage();
		calculateFrameInterfereGraph(this, flm);
	}
}

GraphColorSolver::GraphColorSolver(MModule* module): interfereGraph_(this), module_(module)
{
}

void GraphColorSolver::run()
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
	workList.set(0, (idx));
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
}

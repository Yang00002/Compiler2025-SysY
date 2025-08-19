#include "RegSpill.hpp"
#include "MachineFunction.hpp"
#include "MachineModule.hpp"
#include "MachineOperand.hpp"
#include "MachineInstruction.hpp"
#include "MachineBasicBlock.hpp"
#define DEBUG 0
#include <algorithm>

#include "Config.hpp"
#include "Util.hpp"

using namespace std;

void RegSpill::runInner() const
{
	RUN(f_->checkValidUseList());
	std::vector<Register*> r64;
	std::vector<std::pair<Register*, bool>> rltrf32;
	for (auto freg : m_->FRegs())
	{
		if (freg->canAllocate() && freg->calleeSave_ && f_->useList()[freg].empty())
		{
			r64.emplace_back(freg);
		}
	}
	std::vector<std::pair<FrameIndex*, float>> spilledFrames;
	for (auto stack_frame : f_->stackFrames())
	{
		if (stack_frame->spilledFrame_)
		{
			float cost = 0;
			for (auto use : f_->useList(stack_frame))
			{
				cost += use->block()->weight_;
			}
			// 64 位保存更难
			if (stack_frame->size() == 64) cost /= 2;
			spilledFrames.emplace_back(stack_frame, cost);
		}
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
			LOG(color::green("Spill ") + f->print() + color::green(" to ") + "D" + std::to_string(r->id()));
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
					LOG(color::green("Spill ") + f->print() + color::green(" to ") + "V" + std::to_string(r->id()) +
						".[0]");
				}
				spilledFrames[i] = {};
				continue;
			}
			auto r = rltrf32.back();
			rltrf32.pop_back();
			rpm.emplace(f, r);
			LOG(color::green("Spill ") + f->print() + color::green(" to ") + "V" + std::to_string(r.first->id()) + (r.
				second
				? ".[1]" : ".[0]"));
		}
	}
	for (auto bb : f_->blocks())
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
						inst->block(), regLike, freg.first, u2iNegThrow(ld->width()), freg.second ? 1 : 0, true
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
						LOG(color::red("Remove Inst ") + inst->print());
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
	size = u2iNegThrow(f_->stackFrames().size());
	auto& fms = f_->stackFrames();
	for (int i = 0; i < size; i++)
	{
		auto c = fms[i];
		if (rpm.count(c))
		{
			fms.erase(fms.begin() + i);
			i--;
			size--;
			f_->useList().erase(c);
			delete c;
		}
		else c->index_ = i;
	}
	RUN(f_->checkValidUseList());
}

void RegSpill::run()
{
	if (useFloatRegAsStack2Spill)
	{
		for (auto f : m_->functions())
		{
			f_ = f;
			runInner();
		}
	}
}

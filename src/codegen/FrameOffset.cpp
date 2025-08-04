#include "FrameOffset.hpp"

#include <algorithm>

#include "Config.hpp"
#include "MachineFunction.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"

using namespace std;

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

	long long upAlignTo(long long of, long long s)
	{
		if (s > alignTo16NeedBytes) return ((of + 15) >> 4) << 4;
		if (s > 8) s = 8;
		const static long long u[9] = {0, 0, 1, 3, 3, 7, 7, 7, 7};
		const static long long t[9] = {0, 0, 1, 2, 2, 3, 3, 3, 3};
		return ((of + u[s]) >> t[s]) << t[s];
	}


	long long upAlignTo16(long long of)
	{
		return ((of + 15) >> 4) << 4;
	}
}


void FrameOffset::run()
{
	for (auto f : m_->functions())
	{
		std::vector<FrameScore> scores;
		scores.reserve(f->stack_.size());
		for (auto i : f->stack_)
			scores.emplace_back(FrameScore{i, 0.0f});
		for (auto i : f->blocks())
		{
			for (auto inst : i->instructions())
			{
				for (auto op : inst->operands())
				{
					if (auto frame = dynamic_cast<FrameIndex*>(op); frame != nullptr && frame->stack_t_fix_f())
					{
						auto& score = scores[frame->index()];
						score.score_ += i->weight();
					}
				}
			}
		}
		for (auto& i : scores)
			i.score_ /= static_cast<float>(i.frame_->size());
		sort(scores.begin(), scores.end(), std::greater());
		int size = u2iNegThrow(scores.size());
		for (int i = 0; i < size; i++)
		{
			scores[i].frame_->set_index(i);
			f->stack_[i] = scores[i].frame_;
		}

		long long of = 0;
		for (auto i : f->stack_)
		{
			auto s = logicalRightShift(i->size(), 3);
			i->set_offset(upAlignTo(of, s));
			of = i->offset() + s;
		}

		DynamicBitset b{m_->RegisterCount()};
		for (auto bb : f->blocks_)
		{
			for (auto inst : bb->instructions())
			{
				if (inst->def().empty() == false)
				{
					Register* r = dynamic_cast<Register*>(inst->def(0));
					if (r != nullptr && r->calleeSave_)
					{
						b.set(r->isIntegerRegister() ? r->id() : (r->id() + m_->IRegisterCount()));
					}
				}
				if (inst->imp_def().empty() == false)
				{
					Register* r = inst->imp_def(0);
					if (r->calleeSave_)
					{
						b.set(r->isIntegerRegister() ? r->id() : (r->id() + m_->IRegisterCount()));
					}
				}
			}
		}
		int ic = (m_->IRegisterCount());
		for (auto i : b)
		{
			auto next = ((of + 7) >> 3) << 3;
			if ((i) >= ic)
				f->calleeSaved.emplace_back(m_->FRegs()[i - ic], next);
			else f->calleeSaved.emplace_back(m_->IRegs()[i], next);
			of = next + 8;
		}
		f->stackMoveOffset_ = upAlignTo16(of);
		of = f->stackMoveOffset_;
		for (auto it = f->fix_.rbegin(), e = f->fix_.rend(); it != e; ++it)
		{
			auto i = *it;
			assert((i->size() & 31) == 0);
			auto s = logicalRightShift(i->size(), 3);
			i->set_offset(upAlignTo(of, s));
			of = i->offset() + s;
		}
		f->fixMoveOffset_ = upAlignTo16(of) - f->stackMoveOffset_;
	}
}

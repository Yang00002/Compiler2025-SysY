#include "RemoveEmptyBlocks.hpp"

#include <stdexcept>

#include "MachineFunction.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"


namespace
{
	MBasicBlock* fdBlock(std::unordered_map<MBasicBlock*, MBasicBlock*>& bbm, MBasicBlock* bb)
	{
		auto fd = bbm.find(bb);
		if (fd == bbm.end()) return bb;
		auto get = fdBlock(bbm, fd->second);
		fd->second = get;
		return get;
	}

	std::string condName(Instruction::OpID cond)
	{
		switch (cond) // NOLINT(clang-diagnostic-switch-enum)
		{
			case Instruction::ge: return "GE";
			case Instruction::gt: return "GT";
			case Instruction::le: return "LE";
			case Instruction::lt: return "LT";
			case Instruction::eq: return "EQ";
			case Instruction::ne: return "NE";
			default: break;
		}
		throw std::runtime_error("unexpected");
	}
}

bool RemoveEmptyBlocks::removeEmptyBBs() const
{
	std::unordered_map<MBasicBlock*, MBasicBlock*> bbm;
	MBasicBlock* preBB = nullptr;
	for (auto bb : f_->blocks_)
	{
		if (bb->instructions_.empty())
			bbm.emplace(bb, bb->next_);
		else
		{
			if (preBB != nullptr) preBB->next_ = bb;
			preBB = bb;
		}
	}
	for (auto [i, j] : bbm)
	{
		auto rp = fdBlock(bbm, i);
		f_->replaceAllOperands(BlockAddress::get(i), BlockAddress::get(rp));
	}
	bool again = false;
	/*
	 for (auto bb : f_->blocks_)
	{
		if (!bb->instructions_.empty() && bb->next_)
		{
			auto addr = BlockAddress::get(bb->next_);
			auto& ed = bb->instructions_.back();
			if (dynamic_cast<MB*>(ed))
			{
				if (ed->operand(0) == addr)
				{
					auto op = ed->operand(0);
					f_->removeUse(op, ed);
					delete ed;
					bb->instructions_.pop_back();
					if (bb->instructions_.empty()) again = true;
				}
				else
				{
					if (bb->instructions_.size() >= 2)
					{
						auto ed2 = bb->instructions_[bb->instructions_.size() - 2];
						auto bcc = dynamic_cast<MBcc*>(ed2);
						if (bcc && bcc->operand(0) == addr)
						{
							auto op0 = bcc->op();
							bcc->reverseOp();
							bcc->str->replaceBack(condName(op0), condName(bcc->op()));
							bcc->str->replaceBack(addr->block()->name(),
							                      dynamic_cast<BlockAddress*>(ed->operand(0))->block()->name());
							bcc->replace(addr, ed->operand(0), f_);
							f_->removeUse(ed->operand(0), ed);
							delete ed;
							bb->instructions_.pop_back();
							if (bb->instructions_.empty()) again = true;
						}
					}
				}
			}
			else if (dynamic_cast<MBcc*>(ed))
			{
				if (ed->operand(0) == addr)
				{
					auto op = ed->operand(0);
					f_->removeUse(op, ed);
					delete ed;
					bb->instructions_.pop_back();
					auto e2 = bb->instructions_.back();
					assert(dynamic_cast<MCMP*>(e2));
					for (auto op2 : e2->operands()) f_->removeUse(op2, e2);
					delete e2;
					bb->instructions_.pop_back();
					if (bb->instructions_.empty()) again = true;
				}
			}
		}
	}
	 */
	return again;
}

void RemoveEmptyBlocks::runOnFunc() const
{
	while (removeEmptyBBs())
	{
	}
}

void RemoveEmptyBlocks::run()
{
	for (auto i : m_->functions())
	{
		f_ = i;
		runOnFunc();
	}
}

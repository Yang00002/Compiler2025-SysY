#include "MachineBasicBlock.hpp"

#include <algorithm>
#include <stdexcept>

#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "CountLZ.hpp"
#include "Instruction.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"

#define DEBUG 0
#include "Util.hpp"

using namespace std;


MBasicBlock::MBasicBlock(std::string name, MFunction* function) : function_(function), name_(std::move(name))
{
}

MBasicBlock::~MBasicBlock()
{
	for (auto i : instructions_) delete i;
	delete blockPrefix_;
}

MBasicBlock* MBasicBlock::createBasicBlock(const std::string& name, MFunction* function)
{
	return new MBasicBlock{name, function};
}



int MBasicBlock::needBranchCount() const
{
	auto b = dynamic_cast<MB*>(instructions_.back());
	if (b == nullptr) return 0;
	if (b->isCondBranch())
	{
		if (b->block2GoR() == next_) return 1;
		return 2;
	}
	if (b->block2GoL() == next_) return 0;
	return 1;
}

bool MBasicBlock::empty() const
{
	if (needBranchCount() > 0) return false;
	for (auto i : instructions_) if (i->str->lines() > 0) return false;  // NOLINT(readability-use-anyofallof)
	return true;
}

void MBasicBlock::removeInst(MInstruction* inst)
{
	auto it = instructions_.begin();
	auto ed = instructions_.end();
	while (it != ed)
	{
		if (*it == inst)
		{
			instructions_.erase(it);
			break;
		}
		++it;
	}
	for (auto op : inst->operands())
		function_->removeUse(op, inst);
	delete inst;
}

void MBasicBlock::addInstBeforeLast(MInstruction* inst)
{
	auto b = instructions_.back();
	instructions_.back() = inst;
	instructions_.emplace_back(b);
}

int MBasicBlock::collapseBranch()
{
	auto b = dynamic_cast<MB*>(instructions_.back());
	if (b == nullptr) return 0;
	auto l = b->block2GoL();
	auto r = b->block2GoR();
	if (l != nullptr && l->empty() && l->next_ != nullptr)
	{
		ASSERT(l != l->next_);
		l = l->next_;
		b->replaceL(l);
	}
	if (r != nullptr && r->empty() && r->next_ != nullptr)
	{
		ASSERT(r != r->next_);
		r = r->next_;
		b->replaceR(r);
	}
	int c = 0;
	if (l == r)
	{
		b->removeR();
		if (b->tiedWith_->tiedC_ == nullptr)
		{
			c = b->tiedWith_->str->lines();
			removeInst(b->tiedWith_);
			b->tiedWith_ = nullptr;
		}
	}
	if (next_ == nullptr) return 0;
	if (next_->empty() && next_->next_) next_ = next_->next_;
	if (r != nullptr && l == next_) b->changeLRBlocks();
	return c;
}

MModule* MBasicBlock::module() const
{
	return function_->module();
}

MFunction* MBasicBlock::function() const
{
	return function_;
}

std::string MBasicBlock::print(int& sid) const
{
	string ret = name_ + "[" + to_string(id_) + "]:\t\t\t";
	if (!pre_bbs_.empty())
	{
		ret += "pre_bbs: ";
		for (auto& i : pre_bbs_)
		{
			ret += i->name() + " ";
		}
	}
	if (!suc_bbs_.empty())
	{
		ret += "suc_bbs: ";
		for (auto& i : suc_bbs_)
		{
			ret += i->name() + " ";
		}
	}
	ret += "\n";
	for (auto& i : instructions_)
	{
		ret += to_string(sid++) + ":\t" + i->print() + "\n";
	}
	return ret;
}

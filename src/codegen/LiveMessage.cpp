#include "LiveMessage.hpp"

#include <queue>
#include <set>

#include "MachineBasicBlock.hpp"
#include "MachineOperand.hpp"
#include "MachineInstruction.hpp"
#include "MachineFunction.hpp"
#include "DynamicBitset.hpp"

#define DEBUG 0
#include "Util.hpp"

using namespace std;

/*
 DFS
		std::stack<NodeType*> dfsWorkList;
		std::stack<NextNodeIterator> dfsVisitList;
		dfsWorkList.emplace(block);
		dfsVisitList.emplace(block->nextBegin());
		PRE_JOB
		while (!dfsWorkList.empty())
		{
			auto b = dfsWorkList.top();
			auto& it = dfsVisitList.top();
			if (it == b->nextEnd())
			{
				dfsVisitList.pop();
				SUFF_JOB
				dfsWorkList.pop();
				continue;
			}
			auto get = *it;
			if (VALID CHECK)
			{
				PRE_JOB
				dfsVisitList.push(get->nextBegin());
				dfsWorkList.push(get);
			}
			it UPDATE
		}
 */

namespace
{
	void dfsBlocks(bool* visited, vector<MBasicBlock*>& orders, MBasicBlock* block,
	               int& allocatedIndex)
	{
		std::stack<MBasicBlock*> dfsWorkList;
		std::stack<set<MBasicBlock*>::const_iterator> dfsVisitList;
		dfsWorkList.emplace(block);
		dfsVisitList.emplace(block->suc_bbs().begin());
		visited[block->id()] = true;
		while (!dfsWorkList.empty())
		{
			auto b = dfsWorkList.top();
			auto& it = dfsVisitList.top();
			if (it == b->suc_bbs().end())
			{
				dfsVisitList.pop();
				orders[allocatedIndex--] = dfsWorkList.top();
				dfsWorkList.pop();
				continue;
			}
			auto get = *it;
			if (!visited[get->id()])
			{
				visited[get->id()] = true;
				dfsVisitList.push(get->suc_bbs().begin());
				dfsWorkList.push(get);
			}
			++it;
		}
	}
}

int LiveMessage::regIdOf(const RegisterLike* register_like) const
{
	auto uid = register_like->uid();
	if (uid >= registerIds_.size()) return -1;
	return registerIds_[uid] - 1;
}

LiveMessage::LiveMessage(MFunction* function): int_(true)
{
	auto& blocks = function->blocks();
	auto bs = blocks.size();
	rpos_.resize(bs);
	bool* visited = new bool[bs]{};
	int allocatedIndex = u2iNegThrow(bs) - 1;
	if (!visited[blocks[0]->id()]) dfsBlocks(visited, rpos_, blocks[0], allocatedIndex);
	delete[] visited;
	liveIn_.resize(rpos_.size());
	liveOut_.resize(rpos_.size());
	liveUse_.resize(rpos_.size());
	liveDef_.resize(rpos_.size());
}

void LiveMessage::addRegister(RegisterLike* reg)
{
	register_.emplace_back(reg);
	if (registerIds_.size() <= reg->uid())
	{
		registerIds_.reserve(registerIds_.size() << 1);
		registerIds_.resize(reg->uid() + 1);
	}
	registerIds_[reg->uid()] = u2iNegThrow(register_.size());
}

void LiveMessage::calculateLiveMessage()
{
	for (auto& bb : rpos_)
	{
		liveUse_[bb->id()] = DynamicBitset{u2iNegThrow(register_.size())};
		liveDef_[bb->id()] = DynamicBitset{ u2iNegThrow(register_.size())};
		liveIn_[bb->id()] = DynamicBitset{ u2iNegThrow(register_.size())};
		liveOut_[bb->id()] = DynamicBitset{ u2iNegThrow(register_.size())};
		for (auto& ins : bb->instructions())
		{
			for (auto& reg : ins->imp_use())
			{
				if (reg != nullptr && reg->canAllocate() && reg->isIntegerRegister() == int_)
				{
					if (int id = regIdOf(reg); id > -1)
					{
						if (!liveDef_[bb->id()].test(id))
							liveUse_[bb->id()].set(id);
					}
				}
			}
			for (auto& use : ins->use())
			{
				auto reg = dynamic_cast<RegisterLike*>(ins->operands()[use]);
				if (reg != nullptr && reg->canAllocate() && reg->isIntegerRegister() == int_)
				{
					if (int id = regIdOf(reg); id > -1)
					{
						if (!liveDef_[bb->id()].test(id))
							liveUse_[bb->id()].set(id);
					}
				}
			}
			for (auto& reg : ins->imp_def())
			{
				if (reg != nullptr && reg->canAllocate() && reg->isIntegerRegister() == int_)
				{
					if (int id = regIdOf(reg); id > -1)
					{
						liveDef_[bb->id()].set(id);
					}
				}
			}
			for (auto& def : ins->def())
			{
				auto reg = dynamic_cast<RegisterLike*>(ins->operands()[def]);
				if (reg != nullptr && reg->canAllocate() && reg->isIntegerRegister() == int_)
				{
					if (int id = regIdOf(reg); id > -1)
					{
						liveDef_[bb->id()].set(id);
					}
				}
			}
		}
	}

	auto bs = u2iNegThrow(rpos_.size());
	bool* visited = new bool[bs]{};
	std::queue<MBasicBlock*> Q;
	for (int i = bs - 1; i > -1; i--)
		Q.push(rpos_[i]);

	while (!Q.empty())
	{
		MBasicBlock* block = Q.front();
		Q.pop();
		visited[block->id()] = true;

		liveOut_[block->id()].reset();

		for (auto& s : block->suc_bbs()) liveOut_[block->id()] |= liveIn_[s->id()];

		auto nextIn = (liveOut_[block->id()] - liveDef_[block->id()]) | liveUse_[block->id()];

		if (liveIn_[block->id()] != nextIn)
		{
			liveIn_[block->id()] = nextIn;
			for (auto& p : block->pre_bbs())
				if (visited[p->id()])
				{
					Q.push(p);
					visited[p->id()] = false;
				}
		}
	}

	delete[] visited;
}

void LiveMessage::flush(bool isInt)
{
	int_ = isInt;
	register_.clear();
	registerIds_.clear();
}

bool LiveMessage::care(MOperand* op) const
{
	auto reg = dynamic_cast<RegisterLike*>(op);
	return reg != nullptr && reg->canAllocate() && reg->isIntegerRegister() == int_ && regIdOf(reg) != -1;
}

bool LiveMessage::careVirtual(MOperand* op) const
{
	auto reg = dynamic_cast<VirtualRegister*>(op);
	return reg != nullptr && reg->canAllocate() && reg->isIntegerRegister() == int_ && regIdOf(reg) != -1;
}

DynamicBitset LiveMessage::translate(const std::vector<MOperand*>& vec, const std::vector<int>& idx) const
{
	DynamicBitset ret{u2iNegThrow(register_.size())};
	for (auto r : idx)
	{
		auto i = vec[r];
		auto reg = dynamic_cast<RegisterLike*>(i);
		if (reg != nullptr && reg->canAllocate() && reg->isIntegerRegister() == int_)
		{
			auto id = regIdOf(reg);
			if (id != -1) ret.set(id);
		}
	}
	return ret;
}

DynamicBitset LiveMessage::translate(const std::vector<Register*>& vec) const
{
	DynamicBitset ret{u2iNegThrow(register_.size())};
	for (auto reg : vec)
	{
		if (reg != nullptr && reg->canAllocate() && reg->isIntegerRegister() == int_)
		{
			auto id = regIdOf(reg);
			if (id != -1) ret.set(id);
		}
	}
	return ret;
}

int LiveMessage::regCount() const
{
	return u2iNegThrow(register_.size());
}

RegisterLike* LiveMessage::getReg(int id) const
{
	return register_[id];
}

std::vector<RegisterLike*>& LiveMessage::regs()
{
	return register_;
}

DynamicBitset LiveMessage::physicalRegMask() const
{
	DynamicBitset s{u2iNegThrow(register_.size())};
	for (auto i : register_)
	{
		if (i->isPhysicalRegister())
			s.set(registerIds_[i->uid()] - 1);
	}
	return s;
}

FrameLiveMessage::FrameLiveMessage(MFunction* function)
{
	function_ = function;
	auto& blocks = function->blocks();
	auto bs = blocks.size();
	rpos_.resize(bs);
	bool* visited = new bool[bs]{};
	int allocatedIndex = u2iNegThrow(bs) - 1;
	if (!visited[blocks[0]->id()]) dfsBlocks(visited, rpos_, blocks[0], allocatedIndex);
	delete[] visited;
	liveIn_.resize(rpos_.size());
	liveOut_.resize(rpos_.size());
	liveUse_.resize(rpos_.size());
	liveDef_.resize(rpos_.size());
}

void FrameLiveMessage::calculateLiveMessage()
{
	auto& frames = function_->stackFrames();
	int size = u2iNegThrow(frames.size());
	for (auto& bb : rpos_)
	{
		liveUse_[bb->id()] = DynamicBitset{size};
		liveDef_[bb->id()] = DynamicBitset{size};
		liveIn_[bb->id()] = DynamicBitset{size};
		liveOut_[bb->id()] = DynamicBitset{size};
		for (auto& ins : bb->instructions())
		{
			if (auto ldr = dynamic_cast<MLDR*>(ins); ldr != nullptr)
			{
				auto frame = dynamic_cast<FrameIndex*>(ldr->operands()[1]);
				if (frame != nullptr && frame->spilledFrame_)
				{
					if (!liveDef_[bb->id()].test((frame->index())))
						liveUse_[bb->id()].set((frame->index()));
				}
			}
			if (auto str = dynamic_cast<MSTR*>(ins); str != nullptr)
			{
				auto frame = dynamic_cast<FrameIndex*>(str->operands()[1]);
				if (frame != nullptr && frame->spilledFrame_)
				{
					liveDef_[bb->id()].set((frame->index()));
				}
			}
		}
	}

	auto bs = u2iNegThrow(rpos_.size());
	bool* visited = new bool[bs]{};
	std::queue<MBasicBlock*> Q;
	for (int i = bs - 1; i > -1; i--)
		Q.push(rpos_[i]);

	while (!Q.empty())
	{
		MBasicBlock* block = Q.front();
		Q.pop();
		visited[block->id()] = true;

		liveOut_[block->id()].reset();

		for (auto& s : block->suc_bbs()) liveOut_[block->id()] |= liveIn_[s->id()];

		auto nextIn = (liveOut_[block->id()] - liveDef_[block->id()]) | liveUse_[block->id()];

		if (liveIn_[block->id()] != nextIn)
		{
			liveIn_[block->id()] = nextIn;
			for (auto& p : block->pre_bbs())
				if (visited[p->id()])
				{
					Q.push(p);
					visited[p->id()] = false;
				}
		}
	}

	delete[] visited;
}

#include "LoadStoreEliminate.hpp"

#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"

void LoadStoreEliminate::run()
{
	for (auto f : m_->functions())
	{
		for (auto b : f->blocks())
		{
			f_ = f;
			b_ = b;
			regData_.clear();
			stackData_.clear();
			runInner();
		}
	}
}


bool LoadStoreEliminate::storeDataSame(MOperand* val, MOperand* stack)
{
	auto old = stackData_.find(stack);
	if (old == stackData_.end()) return false;
	if (val != old->second.first) return false;
	if (val->isRegisterLike())
	{
		if (!val->isCanAllocateRegister()) return false;
		auto get = regData_.find(val);
		if (get == regData_.end()) return false;
		int newId = get->second;
		int oldId = old->second.second;
		if (newId != oldId) return false;
	}
	return true;
}

bool LoadStoreEliminate::storeDataRegUnchange(MOperand* stack)
{
	auto old = stackData_.find(stack);
	if (old == stackData_.end()) return false;
	auto val = old->second.first;
	if (!val->isRegisterLike() || !val->isCanAllocateRegister()) return false;
	auto get = regData_.find(val);
	if (get == regData_.end()) return false;
	int newId = get->second;
	int oldId = old->second.second;
	if (newId != oldId) return false;
	return true;
}

bool LoadStoreEliminate::storeDataSameReg(MOperand* val0, MOperand* stack)
{
	auto old = stackData_.find(stack);
	if (old == stackData_.end()) return false;
	auto val = old->second.first;
	if (!val->isRegisterLike()) return false;
	if (!val0->isRegisterLike()) return false;
	return val == val0;
}

void LoadStoreEliminate::updateStack(MOperand* val, MOperand* stack)
{
	auto& get = stackData_[stack];
	get.first = val;
	if (val->isRegisterLike())
	{
		auto oldReg = regData_.find(val);
		int currentId = 0;
		if (oldReg == regData_.end()) regData_[val] = 0;
		else currentId = get.second;
		get.second = currentId;
	}
}

void LoadStoreEliminate::updateReg(MOperand* val)
{
	if (!val->isRegisterLike()) return;
	auto fd = regData_.find(val);
	if (fd == regData_.end()) regData_[val] = 0;
	else regData_[val]++;
	if (stackData_.count(val)) stackData_.erase(val);
}

void LoadStoreEliminate::runInner()
{
	auto& instructions = b_->instructions();
	int size = u2iNegThrow(instructions.size());
	for (int i = 0; i < size; i++)
	{
		auto inst = instructions[i];
		auto store = dynamic_cast<MSTR*>(inst);
		if (store != nullptr)
		{
			auto stack = store->operand(1);
			auto use = store->operand(0);
			if (storeDataSame(use, stack))
			{
				inst->removeAllUse();
				instructions.erase(instructions.begin() + i);
				i--;
				size--;
				delete inst;
				continue;
			}
			updateStack(use, stack);
			continue;
		}
		auto load = dynamic_cast<MLDR*>(inst);
		if (load != nullptr)
		{
			auto stack = store->operand(1);
			auto def = store->operand(0);
			if (storeDataRegUnchange(stack))
			{
				inst->removeAllUse();
				if (storeDataSameReg(def, stack))
				{
					instructions.erase(instructions.begin() + i);
					i--;
					size--;
					delete inst;
					continue;
				}
				instructions[i] = new MCopy{b_, stackData_.at(stack).first, def, load->width()};
				delete inst;
				updateReg(def);
				continue;
			}
			updateReg(def);
			continue;
		}
		for (auto def : inst->def())
		{
			updateReg(inst->def(def));
			continue;
		}
		for (auto def : inst->imp_def())
		{
			updateReg(def);
			continue;
		}
	}
}

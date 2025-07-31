#pragma once
#include <map>
#include <string>
#include <vector>

class FrameIndex;
class Register;
class MOperand;
class RegisterLike;
class DynamicBitset;
class MInstruction;
class MFunction;
class MBasicBlock;
class MModule;

class LiveMessage
{
	std::vector<MBasicBlock*> rpos_;
	std::vector<RegisterLike*> register_;
	std::vector<int> registerIds_;
	std::vector<DynamicBitset> liveIn_;
	std::vector<DynamicBitset> liveOut_;
	std::vector<DynamicBitset> liveUse_;
	std::vector<DynamicBitset> liveDef_;
	bool int_;

public:
	[[nodiscard]] const std::vector<DynamicBitset>& live_in() const
	{
		return liveIn_;
	}

	[[nodiscard]] const std::vector<DynamicBitset>& live_out() const
	{
		return liveOut_;
	}

	[[nodiscard]] const std::vector<DynamicBitset>& live_use() const
	{
		return liveUse_;
	}

	[[nodiscard]] const std::vector<DynamicBitset>& live_def() const
	{
		return liveDef_;
	}

	int regIdOf(const RegisterLike* register_like) const;
	LiveMessage(MFunction* function);
	void addRegister(RegisterLike* reg);
	void calculateLiveMessage();
	void flush(bool isInt);
	bool care(MOperand* op) const;
	bool careVirtual(MOperand* op) const;
	[[nodiscard]] DynamicBitset translate(const std::vector<MOperand*>& vec, const std::vector<int>& idx) const;
	[[nodiscard]] DynamicBitset translate(const std::vector<Register*>& vec) const;
	[[nodiscard]] int regCount() const;
	[[nodiscard]] RegisterLike* getReg(int id) const;
	[[nodiscard]] std::vector<RegisterLike*>& regs();
	[[nodiscard]] DynamicBitset physicalRegMask() const;
};


class FrameLiveMessage
{
	std::vector<MBasicBlock*> rpos_;
	std::vector<DynamicBitset> liveIn_;
	std::vector<DynamicBitset> liveOut_;
	std::vector<DynamicBitset> liveUse_;
	std::vector<DynamicBitset> liveDef_;
	MFunction* function_;

public:
	[[nodiscard]] const std::vector<DynamicBitset>& live_in() const
	{
		return liveIn_;
	}

	[[nodiscard]] const std::vector<DynamicBitset>& live_out() const
	{
		return liveOut_;
	}

	[[nodiscard]] const std::vector<DynamicBitset>& live_use() const
	{
		return liveUse_;
	}

	[[nodiscard]] const std::vector<DynamicBitset>& live_def() const
	{
		return liveDef_;
	}

	FrameLiveMessage(MFunction* function);
	void calculateLiveMessage();
};

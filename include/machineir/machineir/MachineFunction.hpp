#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "DynamicBitset.hpp"

class CodeString;
class GlobalVariable;
class GlobalAddress;
class LiveMessage;
class Function;
class Value;
class MBL;
class MModule;
class BlockAddress;
class FrameIndex;
class VirtualRegister;
class MInstruction;
class MOperand;
class Register;
class MBasicBlock;

class MFunction
{
	friend class FrameOffset;
	friend class MModule;
	friend class CodeGen;
	friend class VirtualRegister;
	friend class RemoveEmptyBlocks;

public:
	CodeString* funcPrefix_ = nullptr;
	CodeString* funcSuffix_ = nullptr;
	std::string sizeSuffix_;

	[[nodiscard]] const std::string& name() const
	{
		return name_;
	}

	[[nodiscard]] std::vector<MBasicBlock*>& blocks()
	{
		return blocks_;
	}

	[[nodiscard]] MBasicBlock* block(int index) const
	{
		return blocks_[index];
	}

	~MFunction();

private:
	friend class FrameIndex;
	friend class BlockAddress;
	MModule* module_;
	std::string name_;
	std::vector<MBasicBlock*> blocks_;
	std::map<MBasicBlock*, BlockAddress*> ba_cache_;
	std::vector<FrameIndex*> stack_;
	std::vector<FrameIndex*> fix_;
	VirtualRegister* lrGuard_ = nullptr;
	std::unordered_map<MOperand*, std::unordered_set<MInstruction*>> useList_;
	DynamicBitset called_;
	DynamicBitset destroyRegs_;
	std::vector<MBL*> calls_;
	std::vector<std::pair<Register*, long long>> calleeSaved;
	std::vector<VirtualRegister*> virtual_iregs_;
	std::vector<VirtualRegister*> virtual_fregs_;
public:
	std::unordered_set<GlobalAddress*> constGlobals_;
private:
	long long stackMoveOffset_;
	long long fixMoveOffset_;
	int id_;
public:
	MFunction(const MFunction& other) = delete;
	MFunction(MFunction&& other) noexcept = delete;
	MFunction& operator=(const MFunction& other) = delete;
	MFunction& operator=(MFunction&& other) noexcept = delete;

	[[nodiscard]] const std::vector<std::pair<Register*, long long>>& callee_saved() const
	{
		return calleeSaved;
	}

	[[nodiscard]] long long stack_move_offset() const
	{
		return stackMoveOffset_;
	}

	[[nodiscard]] long long fix_move_offset() const
	{
		return fixMoveOffset_;
	}

	[[nodiscard]] int id() const
	{
		return id_;
	}

	[[nodiscard]] DynamicBitset& called()
	{
		return called_;
	}

	[[nodiscard]] std::vector<FrameIndex*>& stackFrames()
	{
		return stack_;
	}

	[[nodiscard]] std::vector<FrameIndex*>& fixFrames()
	{
		return fix_;
	}

	[[nodiscard]] VirtualRegister* lr_guard() const
	{
		return lrGuard_;
	}

private:
	MFunction(std::string name, MModule* module);

	FrameIndex* allocaFix(const Value* value);

public:
	[[nodiscard]] DynamicBitset& destroy_regs()
	{
		return destroyRegs_;
	}

	void addCall(MBL* m)
	{
		calls_.emplace_back(m);
	}

	static MFunction* createFunc(const std::string& name, MModule* module);
	FrameIndex* allocaStack(Value* value);
	FrameIndex* allocaStack(int size);
	[[nodiscard]] FrameIndex* getFix(int idx) const;
	void preprocess(Function* function);
	void accept(Function* function, std::map<Function*, MFunction*>& funcMap,
	            std::map<GlobalVariable*, GlobalAddress*>& global_address);
	[[nodiscard]] MModule* module() const;
	[[nodiscard]] std::string print() const;
	MOperand* getOperandFor(Value* value, std::map<Value*, MOperand*>& opMap);
	void spill(VirtualRegister* vreg, LiveMessage* message);
	void replaceAllOperands(MOperand* from, MOperand* to);
	void replaceAllOperands(std::unordered_map<FrameIndex*, FrameIndex*>& rpm);
	void addUse(MOperand* op, MInstruction* ins);
	void removeUse(MOperand* op, MInstruction* ins);
	void rewriteCallsDefList() const;
	void rewriteDestroyRegs();
	std::unordered_map<MOperand*, std::unordered_set<MInstruction*>>& useList();
	std::unordered_set<MInstruction*>& useList(MOperand* op);

	[[nodiscard]] const std::vector<VirtualRegister*>& IVRegs() const;
	[[nodiscard]] const std::vector<VirtualRegister*>& FVRegs() const;
	[[nodiscard]] int virtualIRegisterCount() const;
	[[nodiscard]] int virtualFRegisterCount() const;
	[[nodiscard]] int virtualRegisterCount() const;
	void checkValidUseList();
};

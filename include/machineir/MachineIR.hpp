#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "DynamicBitset.hpp"

class GlobalAddress;
class Register;
class MBL;
class LiveMessage;
class MInstruction;
class GlobalVariable;
class FuncAddress;
class Module;
class MOperand;
class Value;
class Function;
class MBasicBlock;
class MFunction;
class MModule;
class BlockAddress;
class FrameIndex;
class VirtualRegister;
class Immediate;

class MFunction
{
	friend class MModule;
	friend class CodeGen;
	friend class VirtualRegister;

public:
	[[nodiscard]] const std::string& name() const
	{
		return name_;
	}

	[[nodiscard]] std::vector<MBasicBlock*>& blocks()
	{
		return blocks_;
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
	void rankFrameIndexesAndCalculateOffsets();
	void replaceAllOperands(MOperand* from, MOperand* to);
	void replaceAllOperands(std::unordered_map<FrameIndex*, FrameIndex*>& rpm);
	void addUse(MOperand* op, MInstruction* ins);
	void removeUse(MOperand* op, MInstruction* ins);
	void rewriteCallsDefList() const;
	void rewriteDestroyRegs();
	std::unordered_map<MOperand*, std::unordered_set<MInstruction*>>& useList();

	[[nodiscard]] const std::vector<VirtualRegister*>& IVRegs() const;
	[[nodiscard]] const std::vector<VirtualRegister*>& FVRegs() const;
	[[nodiscard]] int virtualIRegisterCount() const;
	[[nodiscard]] int virtualFRegisterCount() const;
	[[nodiscard]] int virtualRegisterCount() const;
};

class MModule
{
	friend class CodeGen;
	friend class MFunction;

public:
	[[nodiscard]] std::vector<MFunction*>& functions()
	{
		return functions_;
	}

	~MModule();
	MModule(const MModule& other) = delete;
	MModule(MModule&& other) noexcept = delete;
	MModule& operator=(const MModule& other) = delete;
	MModule& operator=(MModule&& other) noexcept = delete;

private:
	friend class Immediate;
	friend class VirtualRegister;
	friend class Register;
	friend class FuncAddress;
	friend class GlobalAddress;
	std::vector<GlobalAddress*> globals_;
	std::vector<MFunction*> functions_;
	std::vector<MFunction*> lib_functions_;
	std::vector<MFunction*> allFuncs_;
	std::map<MFunction*, FuncAddress*> func_address_;
	std::map<unsigned long long, Immediate*> imm_cache_;
	std::vector<Register*> iregs_;
	std::vector<Register*> fregs_;
	MFunction* memcpy_;
	MFunction* memclr_;

public:
	[[nodiscard]] const std::vector<MFunction*>& all_funcs() const
	{
		return allFuncs_;
	}

	MModule();
	void accept(Module* module);

	[[nodiscard]] MFunction* memcpyFunc() const
	{
		return memcpy_;
	}

	[[nodiscard]] MFunction* memclrFunc() const
	{
		return memclr_;
	}

	[[nodiscard]] std::string print() const;
	[[nodiscard]] int IRegisterCount() const;
	[[nodiscard]] int FRegisterCount() const;
	[[nodiscard]] int RegisterCount() const;
	[[nodiscard]] const std::vector<Register*>& IRegs() const;
	[[nodiscard]] const std::vector<Register*>& FRegs() const;
	[[nodiscard]] std::vector<GlobalAddress*> constGlobalAddresses() const;
	[[nodiscard]] std::vector<GlobalAddress*> ncnzGlobalAddresses() const;
	[[nodiscard]] std::vector<GlobalAddress*> ncZeroGlobalAddresses() const;
};

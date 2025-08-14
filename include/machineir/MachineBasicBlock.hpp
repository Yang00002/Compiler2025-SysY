#pragma once
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

class CodeString;
class FrameIndex;
class VirtualRegister;
class Function;
class MCopy;
class MModule;
class MOperand;
class Value;
class Instruction;
class BasicBlock;
class MInstruction;
class MFunction;

class MBasicBlock
{
	friend class MachineLoopDetection;
	friend class BlockLayout;
	friend class ReturnMerge;

public:
	[[nodiscard]] std::set<MBasicBlock*>& pre_bbs()
	{
		return pre_bbs_;
	}

	[[nodiscard]] std::set<MBasicBlock*>& suc_bbs()
	{
		return suc_bbs_;
	}

	[[nodiscard]] std::vector<MInstruction*>& instructions()
	{
		return instructions_;
	}

	[[nodiscard]] const std::string& name() const
	{
		return name_;
	}

	[[nodiscard]] int id() const
	{
		return id_;
	}

	[[nodiscard]] float weight() const
	{
		return weight_;
	}

private:
	friend class MFunction;
	friend class RemoveEmptyBlocks;
	MFunction* function_;
	std::string name_;
	std::vector<MInstruction*> instructions_;
	std::set<MBasicBlock*> pre_bbs_;
	std::set<MBasicBlock*> suc_bbs_;
	MBasicBlock* next_ = nullptr;
	int id_ = 0;


	MBasicBlock(std::string name, MFunction* function);

public:
	MBasicBlock(const MBasicBlock& other) = delete;
	MBasicBlock(MBasicBlock&& other) noexcept = delete;
	MBasicBlock& operator=(const MBasicBlock& other) = delete;
	MBasicBlock& operator=(MBasicBlock&& other) noexcept = delete;

	~MBasicBlock();
	float weight_ = 1;

	CodeString* blockPrefix_ = nullptr;

	static MBasicBlock* createBasicBlock(const std::string& name, MFunction* function);
	void accept(BasicBlock* block, std::map<Value*, MOperand*>& opMap, std::map<BasicBlock*, MBasicBlock*>& blockMap,
	            std::map<MBasicBlock*, std::list<MCopy*>>& phiMap, std::map<Function*, MFunction*>& funcMap);
	void acceptReturnInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptBranchInst(Instruction* instruction, std::map<BasicBlock*, MBasicBlock*>& bmap, MBasicBlock* block);
	void acceptMathInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptMSubInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptAllocaInsts(BasicBlock* block, std::map<Value*, MOperand*>& opMap,
	                       std::map<BasicBlock*, MBasicBlock*>& bmap);
	void acceptLoadInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptStoreInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptCmpInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptPhiInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap,
	                   std::map<BasicBlock*, MBasicBlock*>& bmap, std::map<MBasicBlock*, std::list<MCopy*>>& phiMap,
	                   const MBasicBlock* block);
	void acceptCallInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap,
	                    std::map<Function*, MFunction*>& funcMap,
	                    MBasicBlock* block);
	void acceptGetElementPtrInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap
	                             , MBasicBlock* block);
	void mergePhiCopies(std::list<MCopy*>& copies);
	void acceptZextInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptFpToSiInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptSiToFpInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptMemCpyInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptMemClearInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptCopyInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block) const;
	[[nodiscard]] int needBranchCount() const;
	[[nodiscard]] bool empty() const;
	void removeInst(MInstruction* inst);
	int collapseBranch();
	[[nodiscard]] MModule* module() const;
	[[nodiscard]] MFunction* function() const;
	[[nodiscard]] std::string print(int& sid) const;
};

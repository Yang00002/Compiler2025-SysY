#pragma once
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

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
	friend class MFunction;
	MFunction* function_;
	std::string name_;
	std::vector<MInstruction*> instructions_;
	std::set<MBasicBlock*> pre_bbs_;
	std::set<MBasicBlock*> suc_bbs_;
	MBasicBlock* next_ = nullptr;

	MBasicBlock(std::string name, MFunction* function);

public:
	static MBasicBlock* createBasicBlock(const std::string& name, MFunction* function);
	void accept(BasicBlock* block, std::map<Value*, MOperand*>& opMap, std::map<BasicBlock*, MBasicBlock*>& blockMap,
	            const std::map<MBasicBlock*, std::list<MCopy*>>& phiMap, std::map<Function*, MFunction*>& funcMap);
	void acceptReturnInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptBranchInst(Instruction* instruction, std::map<BasicBlock*, MBasicBlock*>& bmap, MBasicBlock* block);
	void acceptMathInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptAllocaInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, const MBasicBlock* block);
	void acceptLoadInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptStoreInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptCmpInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptPhiInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap,
	                   std::map<BasicBlock*, MBasicBlock*>& bmap, std::map<MBasicBlock*, std::list<MCopy*>> phiMap,
	                   const MBasicBlock* block);
	void acceptCallInst(const Instruction* instruction, std::map<Value*, MOperand*>& opMap,
	                    std::map<Function*, MFunction*>& funcMap,
	                    MBasicBlock* block);
	void acceptGetElementPtrInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap
	                             , MBasicBlock* block);
	void mergePhiCopies(const std::list<MCopy*>& copies);
	void acceptZextInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptFpToSiInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptSiToFpInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptMemCpyInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptMemClearInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	void acceptCopyInst(Instruction* instruction, std::map<Value*, MOperand*>& opMap, MBasicBlock* block);
	[[nodiscard]] MModule* module() const;
	[[nodiscard]] MFunction* function() const;
};

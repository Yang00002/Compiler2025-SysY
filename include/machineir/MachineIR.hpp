#pragma once
#include <map>
#include <string>
#include <vector>

class FuncAddress;
class Module;
class MOperand;
class Value;
class Function;
class MBasicBlock;
class MFunction;
class MModule;

class MFunction
{
	friend class FrameIndex;
	friend class BlockAddress;
	MModule* module_;
	std::string name_;
	std::vector<MBasicBlock*> blocks_;
	std::map<MBasicBlock*, BlockAddress*> ba_cache_;
	std::vector<FrameIndex*> stack_;
	std::vector<FrameIndex*> fix_;

	MFunction(std::string name, MModule* module);

	FrameIndex* allocaFix(Value* value);

public:
	static MFunction* createFunc(const std::string& name, MModule* module);
	FrameIndex* allocaStack(Value* value);
	[[nodiscard]] FrameIndex* getFix(int idx) const;
	void preprocess(Function* function);
	void accept(Function* function, std::map<Function*, MFunction*>& funcMap);
	[[nodiscard]] MModule* module() const;
};

class MModule
{
	friend class Immediate;
	friend class VirtualRegister;
	friend class Register;
	friend class FuncAddress;
	std::vector<MFunction*> functions_;
	std::vector<MFunction*> lib_functions_;
	std::map<MFunction*, FuncAddress*> func_address_;
	std::map<unsigned int, Immediate*> imm_cache_;
	std::vector<VirtualRegister*> virtual_iregs_;
	std::vector<VirtualRegister*> virtual_fregs_;
	std::vector<Register*> iregs_;
	std::vector<Register*> fregs_;
	MFunction* memcpy_;
	MFunction* memclr_;

public:
	MModule();
	void accept(Module* module);
	MOperand* getOperandFor(Value* value, std::map<Value*, MOperand*>& opMap);

	[[nodiscard]] MFunction* memcpyFunc() const
	{
		return memcpy_;
	}

	[[nodiscard]] MFunction* memclrFunc() const
	{
		return memclr_;
	}
};

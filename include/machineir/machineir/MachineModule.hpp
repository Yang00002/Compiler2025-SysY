#pragma once
#include <map>
#include <string>
#include <vector>
#include "DynamicBitset.hpp"

class CodeString;
class Module;
class MFunction;
class GlobalAddress;
class FuncAddress;
class Immediate;
class Register;

class MModule
{
	friend class CodeGen;
	friend class MFunction;

public:
	CodeString* modulePrefix_ = nullptr;
	CodeString* moduleSuffix_ = nullptr;

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
	friend std::ostream& operator<<(std::ostream& os, const MModule* module);
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
	[[nodiscard]] std::vector<GlobalAddress*>& globalAddresses();
	[[nodiscard]] std::vector<GlobalAddress*> constGlobalAddresses() const;
	[[nodiscard]] std::vector<GlobalAddress*> ncnzGlobalAddresses() const;
	[[nodiscard]] std::vector<GlobalAddress*> ncZeroGlobalAddresses() const;
	std::map<unsigned long long, Immediate*>& immediates() ;
};

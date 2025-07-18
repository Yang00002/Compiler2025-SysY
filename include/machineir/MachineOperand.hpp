#pragma once
#include "MachineIR.hpp"

class Type;
class MBasicBlock;
class MModule;

class MOperand
{
public:
	MOperand(const MOperand&) = delete;
	MOperand(MOperand&&) = delete;
	MOperand& operator=(const MOperand&) = delete;
	MOperand& operator=(MOperand&&) = delete;
	MOperand() = default;
	virtual ~MOperand() = default;
	bool isRegisterLike();
};

class Immediate final : public MOperand
{
	unsigned int val_;
	explicit Immediate(unsigned int value);

public:
	static Immediate* getImmediate(int intImm, MModule* m);
	static Immediate* getImmediate(float floatImm, MModule* m);
	[[nodiscard]] int asInt() const;
	[[nodiscard]] float asFloat() const;
};

class VirtualRegister final : public MOperand
{
	unsigned int id_;
	bool ireg_t_freg_f_;
	explicit VirtualRegister(unsigned int id, bool ireg_t_freg_f);

public:
	static VirtualRegister* getVirtualIRegister(int id, const MModule* m);
	static VirtualRegister* createVirtualIRegister(MModule* m);
	static VirtualRegister* getVirtualFRegister(int id, const MModule* m);
	static VirtualRegister* createVirtualFRegister(MModule* m);
};

class Register final : public MOperand
{
	friend class MModule;
	unsigned int id_;
	bool ireg_t_freg_f_;
	explicit Register(unsigned int id, bool ireg_t_freg_f);

public:
	static Register* getIRegister(int id, const MModule* m);
	static Register* getFRegister(int id, const MModule* m);
	static Register* getRegisterWithType(int id, const Type* ty, const MModule* m);
	static Register* getLR(const MModule* m);
	static Register* getNZCV(const MModule* m);
};

class BlockAddress final : public MOperand
{
	friend class MModule;
	MBasicBlock* block_;
	explicit BlockAddress(MBasicBlock* block);

public:
	static BlockAddress* get(MBasicBlock* bb);

	[[nodiscard]] MBasicBlock* block() const
	{
		return block_;
	}
};

class FuncAddress final : public MOperand
{
	friend class MModule;
	MFunction* func_;
	explicit FuncAddress(MFunction* block);

public:
	static FuncAddress* get(MFunction* bb);

	[[nodiscard]] MFunction* function() const
	{
		return func_;
	}
};

class FrameIndex final : public MOperand
{
	friend class MFunction;
	MFunction* func_;
	size_t size_;
	unsigned int index_;
	bool stack_t_fix_f_;
	friend class MModule;
	explicit FrameIndex(MFunction* func, unsigned int idx, size_t size, bool stack_t_fix_f);
};

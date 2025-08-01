#pragma once
#include "MachineIR.hpp"
#include "Tensor.hpp"

class ConstantValue;
class GlobalVariable;
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
	virtual bool isRegisterLike();
	virtual bool isCanAllocateRegister();
	virtual std::string print() = 0;
};

class Immediate final : public MOperand
{
	unsigned long long int val_;
	explicit Immediate(unsigned long long value);

public:
	static Immediate* getImmediate(long long pImm, MModule* m);
	static Immediate* getImmediate(int intImm, MModule* m);
	static Immediate* getImmediate(float floatImm, MModule* m);
	[[nodiscard]] int asInt() const;
	[[nodiscard]] float asFloat() const;
	[[nodiscard]] long long as64BitsInt() const;
	[[nodiscard]] bool isZero(bool flt, int len) const;
	[[nodiscard]] bool isNotFloatOne(bool flt, int len) const;
	std::string print() override;
};

class RegisterLike : public MOperand
{
public:
	[[nodiscard]] virtual bool isVirtualRegister() const;
	[[nodiscard]] virtual bool isPhysicalRegister() const;
	[[nodiscard]] virtual bool isIntegerRegister() const = 0;

	[[nodiscard]] virtual bool canAllocate() const
	{
		return true;
	}

	[[nodiscard]] virtual int id() const = 0;
	[[nodiscard]] int uid() const;
};

class VirtualRegister final : public RegisterLike
{
	int id_;
	bool ireg_t_freg_f_;

public:
	bool spilled = false;

private:
	short size_;
	explicit VirtualRegister(int id, bool ireg_t_freg_f, int size);

public:
	MOperand* replacePrefer_ = nullptr; // 被 spill 时优先进行替换而非分配栈帧
	[[nodiscard]] int size() const
	{
		return size_;
	}

	[[nodiscard]] bool isVirtualRegister() const override;

	[[nodiscard]] bool isIntegerRegister() const override
	{
		return ireg_t_freg_f_;
	}

	[[nodiscard]] int id() const override
	{
		return id_;
	}

	static VirtualRegister* createVirtualIRegister(MFunction* f, int size);
	static VirtualRegister* createVirtualFRegister(MFunction* f, int size);
	static VirtualRegister* copy(MFunction* f, const VirtualRegister* v);
	std::string print() override;
};

class Register final : public RegisterLike
{
public:
	[[nodiscard]] bool isPhysicalRegister() const override;

	[[nodiscard]] int id() const override
	{
		return id_;
	}

	[[nodiscard]] bool canAllocate() const override
	{
		return canAllocate_;
	}

	void setCanAllocate(bool can_allocate)
	{
		canAllocate_ = can_allocate;
	}

private:
	friend class MModule;
	int id_;
	bool ireg_t_freg_f_;
	bool canAllocate_ = false;
	explicit Register(int id, bool ireg_t_freg_f, const std::string& name, const std::string& sname);

public:
	bool callerSave_ = false;
	bool calleeSave_ = false;
	std::string name_;
	std::string shortName_;

	[[nodiscard]] bool isIntegerRegister() const override
	{
		return ireg_t_freg_f_;
	}

	static Register* getIParameterRegister(int id, const MModule* m);
	static Register* getFParameterRegister(int id, const MModule* m);
	static Register* getParameterRegisterWithType(int id, const Type* ty, const MModule* m);
	static Register* getIReturnRegister(const MModule* m);
	static Register* getFReturnRegister(const MModule* m);
	static Register* getReturnRegisterWithType(const Type* ty, const MModule* m);
	static Register* getLR(const MModule* m);
	static Register* getNZCV(const MModule* m);
	static Register* getZERO(const MModule* m);
	static Register* getFP(const MModule* m);
	static Register* getSP(const MModule* m);
	static Register* getIIP0(const MModule* m);
	static Register* getIIP1(const MModule* m);
	static Register* getFIP0(const MModule* m);
	static Register* getFIP1(const MModule* m);
	std::string print() override;
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

	std::string print() override;
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

	std::string print() override;
};

class GlobalAddress final : public MOperand
{
	friend class CodeGen;
	friend class MModule;
	size_t size_;
	int index_;
	bool const_;
	PlainTensor<ConstantValue>* data_;
	std::string name_;
	explicit GlobalAddress(MModule* module, GlobalVariable* var);

public:
	GlobalAddress(const GlobalAddress& other) = delete;
	GlobalAddress(GlobalAddress&& other) noexcept = delete;
	GlobalAddress& operator=(const GlobalAddress& other) = delete;
	GlobalAddress& operator=(GlobalAddress&& other) noexcept = delete;
	~GlobalAddress() override;
	std::string print() override;
};

class FrameIndex final : public MOperand
{
	friend class MFunction;
	MFunction* func_;
	int size_;
	int index_;
	int offset_ = 0;
	bool stack_t_fix_f_;
	friend class MModule;
	explicit FrameIndex(MFunction* func, int idx, int size, bool stack_t_fix_f);

public:
	[[nodiscard]] int offset() const
	{
		return offset_;
	}

	[[nodiscard]] MFunction* func() const
	{
		return func_;
	}

	[[nodiscard]] bool stack_t_fix_f() const
	{
		return stack_t_fix_f_;
	}

	[[nodiscard]] bool spilled_frame() const
	{
		return spilledFrame_;
	}

	void set_offset(int offset)
	{
		offset_ = offset;
	}

	[[nodiscard]] int size() const
	{
		return size_;
	}

	[[nodiscard]] bool isParameterFrame() const
	{
		return !stack_t_fix_f_;
	}

	void set_size(int size)
	{
		size_ = size;
	}

	[[nodiscard]] int index() const
	{
		return index_;
	}

	bool spilledFrame_;
	std::string print() override;
};

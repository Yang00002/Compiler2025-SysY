#pragma once
#include <vector>

#include "CodeString.hpp"
#include "Instruction.hpp"


class MCMP;
class MCSET;
class MMAddSUB;
class MFunction;
class VirtualRegister;
class FuncAddress;
class BlockAddress;
class Register;
class MOperand;
class MBasicBlock;

class MInstruction
{
	friend class InterfereGraph;

protected:
	MBasicBlock* block_;
	std::vector<MOperand*> operands_;
	std::vector<int> def_;
	std::vector<int> use_;
	std::vector<Register*> imp_def_;
	std::vector<Register*> imp_use_;

public:
	CodeString* str = nullptr;
	MInstruction(const MInstruction&) = delete;
	MInstruction(MInstruction&&) = delete;
	MInstruction& operator=(const MInstruction&) = delete;
	MInstruction& operator=(MInstruction&&) = delete;

	virtual ~MInstruction()
	{
		delete str;
	}

	MInstruction(MBasicBlock* block);

	[[nodiscard]] MBasicBlock* block() const
	{
		return block_;
	}

	[[nodiscard]] std::vector<MOperand*>& operands()
	{
		return operands_;
	}

	[[nodiscard]] MOperand* operand(int i) const
	{
		return operands_[i];
	}

	[[nodiscard]] const std::vector<int>& def() const
	{
		return def_;
	}

	[[nodiscard]] const std::vector<int>& use() const
	{
		return use_;
	}

	[[nodiscard]] MOperand* def(int i) const
	{
		return operands_[def_[i]];
	}

	[[nodiscard]] MOperand* use(int i) const
	{
		return operands_[use_[i]];
	}

	[[nodiscard]] std::vector<Register*>& imp_def()
	{
		return imp_def_;
	}

	[[nodiscard]] Register* imp_def(int i) const
	{
		return imp_def_[i];
	}

	[[nodiscard]] const std::vector<Register*>& imp_use() const
	{
		return imp_use_;
	}

	virtual std::string print() = 0;
	bool haveUseOf(const MOperand* reg) const;
	bool haveDefOf(const MOperand* reg) const;
	virtual void replace(MOperand* from, MOperand* to, MFunction* parent);
	virtual void onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent);
	virtual void stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent);
};

class MRet final : public MInstruction
{
	friend class ReturnMerge;
	explicit MRet(MBasicBlock* block);

public:
	explicit MRet(MBasicBlock* block, const Function* function);
	std::string print() override;
};

class MCopy final : public MInstruction
{
public:
	[[nodiscard]] int copy_len() const
	{
		return copyLen_;
	}

private:
	int copyLen_;

public:
	explicit MCopy(MBasicBlock* block, MOperand* src, MOperand* des, int copyLen);
	std::string print() override;
};


class MB final : public MInstruction
{
public:
	MCMP* tiedWith_ = nullptr;
	Instruction::OpID op_;

	[[nodiscard]] Instruction::OpID op() const
	{
		return op_;
	}

	[[nodiscard]] bool isCondBranch() const
	{
		return operands_.size() > 1;
	}

	void reverseOp()
	{
		switch (op_) // NOLINT(clang-diagnostic-switch-enum)
		{
			case Instruction::ge:
				{
					op_ = Instruction::lt;
					break;
				}
			case Instruction::gt:
				{
					op_ = Instruction::le;
					break;
				}
			case Instruction::le:
				{
					op_ = Instruction::gt;
					break;
				}
			case Instruction::lt:
				{
					op_ = Instruction::ge;
					break;
				}
			case Instruction::eq:
				{
					op_ = Instruction::ne;
					break;
				}
			case Instruction::ne:
				{
					op_ = Instruction::eq;
					break;
				}
			default: break;
		}
	}

	void changeLRBlocks()
	{
		if (operands_.size() == 1) return;
		auto buf = operands_[0];
		operands_[0] = operands_[1];
		operands_[1] = buf;
		reverseOp();
	}

	explicit MB(MBasicBlock* block, BlockAddress* t);
	explicit MB(MBasicBlock* block, Instruction::OpID op, BlockAddress* t, BlockAddress* f);
	std::string print() override;
	[[nodiscard]] MBasicBlock* block2GoL() const;
	[[nodiscard]] MBasicBlock* block2GoR() const;
	void removeL();
	void removeR();
	void replaceL(MBasicBlock* to);
	void replaceR(MBasicBlock* to);
};

class MMathInst final : public MInstruction
{
public:
	int width_;
	Instruction::OpID op_;

	[[nodiscard]] Instruction::OpID op() const
	{
		return op_;
	}

	[[nodiscard]] int width() const
	{
		return width_;
	}

	explicit MMathInst(MBasicBlock* block, Instruction::OpID op, MOperand* l, MOperand* r, MOperand* t, int width);
	void replace(MOperand* from, MOperand* to, MFunction* parent) override;
	void onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent) override;
	void stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent) override;
	std::string print() override;
};

class MLDR final : public MInstruction
{
	int width_;

public:
	[[nodiscard]] int width() const
	{
		return width_;
	}

	explicit MLDR(MBasicBlock* block, MOperand* regLike, MOperand* stackLike, int width);
	std::string print() override;
};

class MSTR final : public MInstruction
{
	int width_;

public:
	bool forCall_ = false;
	[[nodiscard]] int width() const
	{
		return width_;
	}

	explicit MSTR(MBasicBlock* block, MOperand* regLike, MOperand* stackLike, int width);
	std::string print() override;
	void replace(MOperand* from, MOperand* to, MFunction* parent) override;
	void onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent) override;
	void stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent) override;
};

class MCMP final : public MInstruction
{
public:
	MCSET* tiedC_ = nullptr;
	MB* tiedB_ = nullptr;
	bool itff_;
	explicit MCMP(MBasicBlock* block, MOperand* l, MOperand* r, bool itff);
	std::string print() override;
	void replace(MOperand* from, MOperand* to, MFunction* parent) override;
	void onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent) override;
	void stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent) override;
};

class MBL final : public MInstruction
{
public:

	explicit MBL(MBasicBlock* block, FuncAddress* addr, Function* function);
	explicit MBL(MBasicBlock* block, FuncAddress* addr, bool cptclf);
	std::string print() override;
};

class MCSET final : public MInstruction
{
public:
	MCMP* tiedWith_ = nullptr;

	Instruction::OpID op_;
	explicit MCSET(MBasicBlock* block, Instruction::OpID op, MOperand* t);
	std::string print() override;
};

class MFCVTZS final : public MInstruction
{
public:
	explicit MFCVTZS(MBasicBlock* block, MOperand* fp, MOperand* si);
	std::string print() override;
};

class MSCVTF final : public MInstruction
{
public:
	explicit MSCVTF(MBasicBlock* block, MOperand* si, MOperand* fp);
	std::string print() override;
};

class MLD1V16B final : public MInstruction
{
public:
	int loadCount_;
	int offset_;
	explicit MLD1V16B(MBasicBlock* block, MOperand* stackLike, int count, int offset);
	std::string print() override;
};

class MST1ZTV16B final : public MInstruction
{
public:
	int loadCount_;
	explicit MST1ZTV16B(MBasicBlock* block, int count);
	std::string print() override;
};

class MST1V16B final : public MInstruction
{
public:
	int storeCount_;
	int offset_;
	explicit MST1V16B(MBasicBlock* block, MOperand* stackLike, int count, int offset);
	std::string print() override;
};

class MSXTW final : public MInstruction
{
public:
	explicit MSXTW(MBasicBlock* block, MOperand* from, MOperand* to);
	std::string print() override;
};

class MMAddSUB final : public MInstruction
{
public:
	bool add_;
	explicit MMAddSUB(MBasicBlock* block, MOperand* t, MOperand* l, MOperand* r, MOperand* s, bool isAdd);
	std::string print() override;
	void replace(MOperand* from, MOperand* to, MFunction* parent) override;
	void onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent) override;
	void stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent) override;
};

class MNeg final : public MInstruction
{
public:
	explicit MNeg(MBasicBlock* block, MOperand* t, MOperand* l, MOperand* r);
	std::string print() override;
	void replace(MOperand* from, MOperand* to, MFunction* parent) override;
	void onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent) override;
	void stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent) override;
};

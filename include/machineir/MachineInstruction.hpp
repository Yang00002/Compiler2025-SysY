#pragma once
#include <vector>

#include "Instruction.hpp"


class FuncAddress;
class BlockAddress;
class Register;
class MOperand;
class MBasicBlock;

class MInstruction
{
protected:
	MBasicBlock* block_;
	std::vector<MOperand*> operands_;
	std::vector<MOperand*> def_;
	std::vector<MOperand*> use_;
	std::set<Register*> imp_def_;
	std::set<Register*> imp_use_;

public:
	MInstruction(const MInstruction&) = delete;
	MInstruction(MInstruction&&) = delete;
	MInstruction& operator=(const MInstruction&) = delete;
	MInstruction& operator=(MInstruction&&) = delete;
	virtual ~MInstruction() = default;
	MInstruction(MBasicBlock* block);
};

class MRet final : public MInstruction
{
public:
	explicit MRet(MBasicBlock* block);
};

class MCopy final : public MInstruction
{
	unsigned int copyLen_;

public:
	explicit MCopy(MBasicBlock* block, MOperand* src, MOperand* des, unsigned int copyLen);
};

class MBcc final : public MInstruction
{
	Instruction::OpID op_;

public:
	explicit MBcc(MBasicBlock* block, Instruction::OpID op, BlockAddress* t);
};

class MB final : public MInstruction
{
public:
	explicit MB(MBasicBlock* block, BlockAddress* t);
};

class MMathInst final : public MInstruction
{
	Instruction::OpID op_;
	unsigned int width_;

public:
	explicit MMathInst(MBasicBlock* block, Instruction::OpID op, MOperand* l, MOperand* r, MOperand* t, unsigned width);
};


class MLDR final : public MInstruction
{
	unsigned int width_;

public:
	explicit MLDR(MBasicBlock* block, MOperand* regLike, MOperand* stackLike, unsigned int width);
};

class MSTR final : public MInstruction
{
	unsigned int width_;

public:
	explicit MSTR(MBasicBlock* block, MOperand* regLike, MOperand* stackLike, unsigned int width);
};

class MCMP final : public MInstruction
{
public:
	explicit MCMP(MBasicBlock* block, MOperand* l, MOperand* r);
};

class MBL final : public MInstruction
{
public:
	explicit MBL(MBasicBlock* block, FuncAddress* addr);
};

class MCSET final : public MInstruction
{
	Instruction::OpID op_;

public:
	explicit MCSET(MBasicBlock* block, Instruction::OpID op, MOperand* t);
};

class MFCVTZS final : public MInstruction
{
public:
	explicit MFCVTZS(MBasicBlock* block, MOperand* fp, MOperand* si);
};

class MSCVTF final : public MInstruction
{
public:
	explicit MSCVTF(MBasicBlock* block, MOperand* si, MOperand* fp);
};

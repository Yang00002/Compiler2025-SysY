#include "MachineInstruction.hpp"
#include "MachineOperand.hpp"
#include "MachineIR.hpp"
#include "MachineBasicBlock.hpp"

using namespace std;


MInstruction::MInstruction(MBasicBlock* block) : block_(block)
{
}

MRet::MRet(MBasicBlock* block)
	: MInstruction(block)
{
	imp_use_.emplace(Register::getLR(block->module()));
}

MCopy::MCopy(MBasicBlock* block, MOperand* src, MOperand* des, unsigned int copyLen) : MInstruction(block),
	copyLen_(copyLen)
{
	operands_.resize(2);
	operands_[0] = src;
	operands_[1] = des;
	def_.resize(1);
	def_[0] = des;
	use_.resize(1);
	use_[0] = src;
}

MBcc::MBcc(MBasicBlock* block, Instruction::OpID op, BlockAddress* t) : MInstruction(block), op_(op)
{
	operands_.resize(1);
	operands_[0] = t;
	imp_use_.emplace(Register::getNZCV(block->module()));
}

MB::MB(MBasicBlock* block, BlockAddress* t) : MInstruction(block)
{
	operands_.resize(1);
	operands_[0] = t;
	imp_use_.emplace(Register::getNZCV(block->module()));
}

MMathInst::MMathInst(MBasicBlock* block, Instruction::OpID op, MOperand* l, MOperand* r,
                     MOperand* t, unsigned width) : MInstruction(block), op_(op), width_(width)
{
	operands_.resize(3);
	operands_[0] = t;
	operands_[1] = l;
	operands_[2] = r;
	def_.resize(1);
	def_[0] = t;
	use_.resize(2);
	use_[0] = l;
	use_[1] = r;
}

MLDR::MLDR(MBasicBlock* block, MOperand* regLike, MOperand* stackLike, unsigned int width): MInstruction(block),
	width_(width)
{
	operands_.resize(2);
	operands_[0] = regLike;
	operands_[1] = stackLike;
	if (regLike->isRegisterLike()) def_.emplace_back(regLike);
	if (stackLike->isRegisterLike()) use_.emplace_back(stackLike);
}

MSTR::MSTR(MBasicBlock* block, MOperand* regLike, MOperand* stackLike, unsigned int width) : MInstruction(block),
	width_(width)
{
	operands_.resize(2);
	operands_[0] = regLike;
	operands_[1] = stackLike;
	if (regLike->isRegisterLike()) use_.emplace_back(regLike);
	if (stackLike->isRegisterLike()) use_.emplace_back(stackLike);
}

MCMP::MCMP(MBasicBlock* block, MOperand* l, MOperand* r) : MInstruction(block)
{
	operands_.resize(2);
	operands_[0] = l;
	operands_[1] = r;
	use_.resize(2);
	use_[0] = l;
	use_[1] = r;
	imp_def_.emplace(Register::getNZCV(block->module()));
}

MBL::MBL(MBasicBlock* block, FuncAddress* addr) : MInstruction(block)
{
	operands_.resize(1);
	operands_[0] = addr;
	imp_def_.emplace(Register::getNZCV(block->module()));
	imp_use_.emplace(Register::getLR(block->module()));
	for (int i = 0; i < 8; i++) imp_def_.emplace(Register::getIRegister(i, block->module()));
	for (int i = 9; i < 16; i++) imp_def_.emplace(Register::getIRegister(i, block->module()));
	for (int i = 0; i < 8; i++) imp_def_.emplace(Register::getFRegister(i, block->module()));
	for (int i = 16; i < 32; i++) imp_def_.emplace(Register::getFRegister(i, block->module()));
}

MCSET::MCSET(MBasicBlock* block, Instruction::OpID op, MOperand* t): MInstruction(block), op_(op)
{
	operands_.resize(1);
	operands_[0] = t;
	def_.resize(1);
	def_[0] = t;
	imp_use_.emplace(Register::getNZCV(block->module()));
}

MFCVTZS::MFCVTZS(MBasicBlock* block, MOperand* fp, MOperand* si) : MInstruction(block)
{
	operands_.resize(2);
	operands_[0] = si;
	operands_[1] = fp;
	def_.resize(1);
	def_[0] = si;
	use_.resize(1);
	use_[0] = fp;
}

MSCVTF::MSCVTF(MBasicBlock* block, MOperand* si, MOperand* fp) : MInstruction(block)
{
	operands_.resize(2);
	operands_[0] = fp;
	operands_[1] = si;
	def_.resize(1);
	def_[0] = fp;
	use_.resize(1);
	use_[0] = si;
}

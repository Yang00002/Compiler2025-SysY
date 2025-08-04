#include "MachineInstruction.hpp"


#include "Instruction.hpp"
#include "IRPrinter.hpp"
#include "MachineOperand.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "Type.hpp"

using namespace std;


MInstruction::MInstruction(MBasicBlock* block) : block_(block)
{
}


bool MInstruction::haveUseOf(const VirtualRegister* reg) const
{
	for (auto i : use_) // NOLINT(readability-use-anyofallof)
	{
		if (operands_[i] == reg) return true;
	}
	return false;
}

bool MInstruction::haveDefOf(const VirtualRegister* reg) const
{
	for (auto i : def_) // NOLINT(readability-use-anyofallof)
	{
		if (operands_[i] == reg) return true;
	}
	return false;
}

void MInstruction::replace(MOperand* from, MOperand* to, MFunction* parent)
{
	if (from == to) return;
	for (auto& i : operands_)
	{
		if (i == from) i = to;
	}
	parent->removeUse(from, this);
	parent->addUse(to, this);
}

void MInstruction::onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent)
{
	if (from == to) return;
	for (auto& i : operands_)
	{
		if (i == from) i = to;
	}
	parent->addUse(to, this);
}

void MInstruction::stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent)
{
	if (from == to) return;
	for (auto& i : operands_)
	{
		if (i == from) i = to;
	}
}

MRet::MRet(MBasicBlock* block) : MInstruction(block)
{
}

MRet::MRet(MBasicBlock* block, const Function* function)
	: MInstruction(block)
{
	imp_use_.emplace_back(Register::getLR(block->module()));
	auto rt = function->get_return_type();
	if (rt == Types::VOID) return;
	imp_use_.emplace_back(Register::getReturnRegisterWithType(rt, block->module()));
}

std::string MRet::print()
{
	return "RET\t\t\t;imp_use LR";
}

MCopy::MCopy(MBasicBlock* block, MOperand* src, MOperand* des, int copyLen) : MInstruction(block),
                                                                              copyLen_(copyLen)
{
	operands_.resize(2);
	operands_[0] = src;
	operands_[1] = des;
	def_.resize(1);
	def_[0] = 1;
	use_.resize(1);
	use_[0] = 0;
	auto func = block->function();
	func->addUse(src, this);
	func->addUse(des, this);
}

std::string MCopy::print()
{
	return operands_[1]->print() + " = COPY " + operands_[0]->print() + " [" + to_string(copyLen_) + "]";
}

std::string MB::print()
{
	if (operands().size() == 1)
		return "B " + operands_[0]->print();
	return "B." + print_instr_op_name(op_) + " " + operands_[0]->print() + "\t\t\t;imp_use NZCV";
}

MBasicBlock* MB::block2GoL() const
{
	return dynamic_cast<BlockAddress*>(operands_[0])->block();
}

MBasicBlock* MB::block2GoR() const
{
	if (operands_.size() == 1) return nullptr;
	return dynamic_cast<BlockAddress*>(operands_[1])->block();
}

void MB::removeL()
{
	assert(operands_.size() > 1);
	if (operands_[0] == operands_[1])
	{
		operands_.pop_back();
	}
	else
	{
		auto rm = dynamic_cast<BlockAddress*>(operands_[0])->block();
		block()->suc_bbs().erase(rm);
		rm->pre_bbs().erase(block());
		block()->function()->removeUse(operands_[0], this);
		operands_[0] = operands_[1];
		operands_.pop_back();
	}
}

void MB::removeR()
{
	assert(operands_.size() > 1);
	if (operands_[0] == operands_[1])
		operands_.pop_back();
	else
	{
		auto rm = dynamic_cast<BlockAddress*>(operands_[1])->block();
		block()->suc_bbs().erase(rm);
		rm->pre_bbs().erase(block());
		block()->function()->removeUse(operands_[1], this);
		operands_.pop_back();
	}
}

void MB::replaceL(MBasicBlock* to)
{
	auto op = BlockAddress::get(to);
	if (op == operands_[0]) return;
	if (operands_.size() > 1)
	{
		if (operands_[0] == operands_[1])
		{
			operands_[0] = op;
			auto ad = op->block();
			block()->suc_bbs().emplace(ad);
			ad->pre_bbs().emplace(block());
			block()->function()->addUse(op, this);
			return;
		}
		auto rm = dynamic_cast<BlockAddress*>(operands_[0])->block();
		block()->suc_bbs().erase(rm);
		rm->pre_bbs().erase(block());
		block()->function()->removeUse(operands_[0], this);
		operands_[0] = op;
		if (op != operands_[1])
		{
			auto ad = op->block();
			block()->suc_bbs().emplace(ad);
			ad->pre_bbs().emplace(block());
			block()->function()->addUse(op, this);
		}
	}
	auto rm = dynamic_cast<BlockAddress*>(operands_[0])->block();
	block()->suc_bbs().erase(rm);
	rm->pre_bbs().erase(block());
	block()->function()->removeUse(operands_[0], this);
	auto ad = op->block();
	block()->suc_bbs().emplace(ad);
	ad->pre_bbs().emplace(block());
	block()->function()->addUse(op, this);
	operands_[0] = op;
}

void MB::replaceR(MBasicBlock* to)
{
	assert(operands_.size() > 1);
	auto op = BlockAddress::get(to);
	if (op == operands_[1]) return;
	if (operands_[0] == operands_[1])
	{
		operands_[1] = op;
		auto ad = op->block();
		block()->suc_bbs().emplace(ad);
		ad->pre_bbs().emplace(block());
		block()->function()->addUse(op, this);
		return;
	}
	auto rm = dynamic_cast<BlockAddress*>(operands_[1])->block();
	block()->suc_bbs().erase(rm);
	rm->pre_bbs().erase(block());
	block()->function()->removeUse(operands_[1], this);
	operands_[1] = op;
	if (op != operands_[0])
	{
		auto ad = op->block();
		block()->suc_bbs().emplace(ad);
		ad->pre_bbs().emplace(block());
		block()->function()->addUse(op, this);
	}
}

MB::MB(MBasicBlock* block, BlockAddress* t) : MInstruction(block)
{
	operands_.resize(1);
	operands_[0] = t;
	auto func = block->function();
	func->addUse(t, this);
}

MB::MB(MBasicBlock* block, Instruction::OpID op, BlockAddress* t, BlockAddress* f) : MInstruction(block), op_(op)
{
	operands_.resize(2);
	operands_[0] = t;
	operands_[1] = f;
	imp_use_.emplace_back(Register::getNZCV(block->module()));
	auto func = block->function();
	func->addUse(t, this);
	func->addUse(f, this);
}

MMathInst::MMathInst(MBasicBlock* block, Instruction::OpID op, MOperand* l, MOperand* r,
                     MOperand* t, int width) : MInstruction(block), width_(width), op_(op)
{
	assert(l != nullptr);
	assert(r != nullptr);
	assert(t != nullptr);
	operands_.resize(3);
	operands_[0] = t;
	operands_[1] = l;
	operands_[2] = r;
	def_.resize(1);
	def_[0] = 0;
	if (l == r)
	{
		use_.resize(1);
		use_[0] = 1;
	}
	else
	{
		use_.resize(2);
		use_[0] = 1;
		use_[1] = 2;
	}
	auto func = block->function();
	func->addUse(t, this);
	func->addUse(l, this);
	func->addUse(r, this);
}

void MMathInst::replace(MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::replace(from, to, parent);
	if (use_.size() == 2 && operands_[1] == operands_[2])
		use_.pop_back();
}

void MMathInst::onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::onlyAddUseReplace(from, to, parent);
	if (use_.size() == 2 && operands_[1] == operands_[2])
		use_.pop_back();
}

void MMathInst::stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::stayUseReplace(from, to, parent);
	if (use_.size() == 2 && operands_[1] == operands_[2])
		use_.pop_back();
}

std::string MMathInst::print()
{
	return operands_[0]->print() + " = " + print_instr_op_name(op_) + " " + operands_[1]->print() + " " + operands_[2]->
	       print() + " [" + to_string(width_) + "]";
}

MLDR::MLDR(MBasicBlock* block, MOperand* regLike, MOperand* stackLike, int width): MInstruction(block),
	width_(width)
{
	operands_.resize(2);
	operands_[0] = regLike;
	operands_[1] = stackLike;
	if (regLike->isRegisterLike()) def_.emplace_back(0);
	if (stackLike->isRegisterLike()) use_.emplace_back(1);
	auto func = block->function();
	func->addUse(regLike, this);
	func->addUse(stackLike, this);
}

std::string MLDR::print()
{
	return operands_[0]->print() + " = LDR " + operands_[1]->print() + " [" + to_string(width_) + "]";
}

MSTR::MSTR(MBasicBlock* block, MOperand* regLike, MOperand* stackLike, int width) : MInstruction(block),
	width_(width)
{
	operands_.resize(2);
	operands_[0] = regLike;
	operands_[1] = stackLike;
	if (regLike->isRegisterLike()) use_.emplace_back(0);
	if (regLike != stackLike && stackLike->isRegisterLike()) use_.emplace_back(1);
	auto func = block->function();
	func->addUse(regLike, this);
	func->addUse(stackLike, this);
}

std::string MSTR::print()
{
	return "STR " + operands_[0]->print() + " " + operands_[1]->print() + " [" + to_string(width_) + "]";
}

void MSTR::replace(MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::replace(from, to, parent);
	if (use_.size() == 2 && operands_[0] == operands_[1])
		use_.pop_back();
}

void MSTR::onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::onlyAddUseReplace(from, to, parent);
	if (use_.size() == 2 && operands_[0] == operands_[1])
		use_.pop_back();
}

void MSTR::stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::stayUseReplace(from, to, parent);
	if (use_.size() == 2 && operands_[0] == operands_[1])
		use_.pop_back();
}

MCMP::MCMP(MBasicBlock* block, MOperand* l, MOperand* r, bool itff) : MInstruction(block)
{
	operands_.resize(2);
	operands_[0] = l;
	operands_[1] = r;
	if (l == r)
	{
		use_.resize(1);
		use_[0] = 0;
	}
	else
	{
		use_.resize(2);
		use_[0] = 0;
		use_[1] = 1;
	}
	imp_def_.emplace_back(Register::getNZCV(block->module()));
	auto func = block->function();
	func->addUse(l, this);
	func->addUse(r, this);
	itff_ = itff;
}

std::string MCMP::print()
{
	return "CMP " + operands_[0]->print() + " " + operands_[1]->print() + "\t\t\t;imp_def NZCV";
}

void MCMP::replace(MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::replace(from, to, parent);
	if (use_.size() == 2 && operands_[0] == operands_[1])
		use_.pop_back();
}

void MCMP::onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::onlyAddUseReplace(from, to, parent);
	if (use_.size() == 2 && operands_[0] == operands_[1])
		use_.pop_back();
}

void MCMP::stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::stayUseReplace(from, to, parent);
	if (use_.size() == 2 && operands_[0] == operands_[1])
		use_.pop_back();
}

MBL::MBL(MBasicBlock* block, FuncAddress* addr, Function* function) : MInstruction(block)
{
	block->function()->addCall(this);
	operands_.resize(1);
	operands_[0] = addr;
	imp_def_.emplace_back(Register::getNZCV(block->module()));

	int ic = 0;
	int fc = 0;
	for (auto& arg : function->get_args())
	{
		if (arg.get_type() == Types::FLOAT)
		{
			if (fc < 8)
			{
				imp_use_.emplace_back(Register::getFParameterRegister(fc, block->module()));
				fc++;
			}
		}
		else
		{
			if (ic < 8)
			{
				imp_use_.emplace_back(Register::getIParameterRegister(fc, block->module()));
				ic++;
			}
		}
	}
}

MBL::MBL(MBasicBlock* block, FuncAddress* addr, bool cptclf) : MInstruction(block)
{
	block->function()->addCall(this);
	operands_.resize(1);
	operands_[0] = addr;
	imp_def_.emplace_back(Register::getNZCV(block->module()));
	imp_def_.emplace_back(Register::getLR(block->module()));
	if (cptclf)
	{
		for (int i = 0; i < 3; i++) imp_use_.emplace_back(Register::getIParameterRegister(i, block->module()));
	}
	else
	{
		for (int i = 0; i < 2; i++) imp_use_.emplace_back(Register::getIParameterRegister(i, block->module()));
	}
}

std::string MBL::print()
{
	return "BL " + operands_[0]->print() + "\t\t\t;imp_def NZCV imp_use LR";
}

MCSET::MCSET(MBasicBlock* block, Instruction::OpID op, MOperand* t): MInstruction(block), op_(op)
{
	operands_.resize(1);
	operands_[0] = t;
	def_.resize(1);
	def_[0] = 0;
	imp_use_.emplace_back(Register::getNZCV(block->module()));
	auto func = block->function();
	func->addUse(t, this);
}

std::string MCSET::print()
{
	return operands_[0]->print() + " = CSET." + print_instr_op_name(op_) + "\t\t\t;imp_use NZCV";
}

MFCVTZS::MFCVTZS(MBasicBlock* block, MOperand* fp, MOperand* si) : MInstruction(block)
{
	operands_.resize(2);
	operands_[0] = si;
	operands_[1] = fp;
	def_.resize(1);
	def_[0] = 0;
	use_.resize(1);
	use_[0] = 1;
	auto func = block->function();
	func->addUse(si, this);
	func->addUse(fp, this);
}

std::string MFCVTZS::print()
{
	return operands_[0]->print() + " = FCVTZS " + operands_[1]->print();
}

MSCVTF::MSCVTF(MBasicBlock* block, MOperand* si, MOperand* fp) : MInstruction(block)
{
	operands_.resize(2);
	operands_[0] = fp;
	operands_[1] = si;
	def_.resize(1);
	def_[0] = 0;
	use_.resize(1);
	use_[0] = 1;
	auto func = block->function();
	func->addUse(si, this);
	func->addUse(fp, this);
}

std::string MSCVTF::print()
{
	return operands_[0]->print() + " = SCVTF " + operands_[1]->print();
}

MLD1V16B::MLD1V16B(MBasicBlock* block, MOperand* stackLike, int count, int offset): MInstruction(block)
{
	operands_.resize(1 + count);
	operands_[0] = stackLike;
	for (int i = 0; i < count; i++) operands_[i + 1] = Register::getFParameterRegister(i, block->module());
	def_.resize(count);
	for (int i = 0; i < count; i++) def_[i] = i + 1;
	if (offset > 0)
	{
		def_.emplace_back(0);
	}
	use_.resize(1);
	use_[0] = 0;
	loadCount_ = count;
	offset_ = offset;
}

std::string MLD1V16B::print()
{
	string ret = "LD1 {";
	for (int i = 0; i < loadCount_; i++)
		ret += "V" + to_string(i) + ".16B,";
	if (loadCount_ > 0) ret.pop_back();
	ret += "}, ";
	ret += operands_[0]->print() + " #" + to_string(offset_);
	return ret;
}

MST1ZTV16B::MST1ZTV16B(MBasicBlock* block, int count) : MInstruction(block)
{
	operands_.resize(count);
	for (int i = 0; i < count; i++) operands_[i] = Register::getFParameterRegister(i, block->module());
	def_.resize(count);
	for (int i = 0; i < count; i++) def_[i] = i;
	loadCount_ = count;
}

std::string MST1ZTV16B::print()
{
	string ret = "EORCLR";
	for (int i = 0; i < loadCount_; i++)
	{
		ret += " V" + to_string(i) + ".16B,";
	}
	ret.pop_back();
	return ret;
}

MST1V16B::MST1V16B(MBasicBlock* block, MOperand* stackLike, int count, int offset) : MInstruction(block)
{
	operands_.resize(1 + count);
	operands_[0] = stackLike;
	for (int i = 0; i < count; i++) operands_[i + 1] = Register::getFParameterRegister(i, block->module());
	use_.resize(count + 1);
	for (int i = 0; i < count + 1; i++) use_[i] = i;
	if (offset > 0)
	{
		def_.emplace_back(0);
	}
	storeCount_ = count;
	offset_ = offset;
}

std::string MST1V16B::print()
{
	string ret = "ST1 {";
	for (int i = 0; i < storeCount_; i++)
		ret += "V" + to_string(i) + ".16B,";
	if (storeCount_ > 0) ret.pop_back();
	ret += "}, ";
	ret += operands_[0]->print() + " #" + to_string(offset_);
	return ret;
}

MSXTW::MSXTW(MBasicBlock* block, MOperand* from, MOperand* to) : MInstruction(block)
{
	operands_.resize(2);
	operands_[0] = from;
	operands_[1] = to;
	use_.resize(1);
	use_[0] = 0;
	def_.resize(1);
	def_[0] = 1;
}

std::string MSXTW::print()
{
	return operands_[1]->print() + " = SXTW " + operands_[0]->print();
}

MMSUB::MMSUB(MBasicBlock* block, MOperand* t, MOperand* l, MOperand* r, MOperand* s) : MInstruction(block)
{
	operands_.resize(4);
	operands_[0] = t;
	operands_[1] = l;
	operands_[2] = r;
	operands_[3] = s;
	def_.resize(1);
	def_[0] = 0;
	use_.emplace_back(1);
	if (r != l) use_.emplace_back(2);
	if (s != r && s != l) use_.emplace_back(3);
	auto func = block->function();
	func->addUse(t, this);
	func->addUse(l, this);
	func->addUse(r, this);
	func->addUse(s, this);
}

std::string MMSUB::print()
{
	return operands_[0]->print() + " = MSUB " + operands_[1]->print() + " " + operands_[2]->print() + " " + operands_[3]
	       ->print();
}

void MMSUB::replace(MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::replace(from, to, parent);
	if (use_.size() == 1) return;
	if (use_.size() == 2)
	{
		if (operands_[0] == operands_[1])
			use_.pop_back();
		return;
	}
	if (operands_[0] == operands_[1])
	{
		use_[1] = use_[2];
		use_.pop_back();
		return;
	}
	if (operands_[0] == operands_[2] || operands_[1] == operands_[2])
	{
		use_.pop_back();
	}
}

void MMSUB::onlyAddUseReplace(const MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::onlyAddUseReplace(from, to, parent);
	if (use_.size() == 1) return;
	if (use_.size() == 2)
	{
		if (operands_[0] == operands_[1])
			use_.pop_back();
		return;
	}
	if (operands_[0] == operands_[1])
	{
		use_[1] = use_[2];
		use_.pop_back();
		return;
	}
	if (operands_[0] == operands_[2] || operands_[1] == operands_[2])
	{
		use_.pop_back();
	}
}

void MMSUB::stayUseReplace(const MOperand* from, MOperand* to, MFunction* parent)
{
	MInstruction::stayUseReplace(from, to, parent);
	if (use_.size() == 1) return;
	if (use_.size() == 2)
	{
		if (operands_[0] == operands_[1])
			use_.pop_back();
		return;
	}
	if (operands_[0] == operands_[1])
	{
		use_[1] = use_[2];
		use_.pop_back();
		return;
	}
	if (operands_[0] == operands_[2] || operands_[1] == operands_[2])
	{
		use_.pop_back();
	}
}

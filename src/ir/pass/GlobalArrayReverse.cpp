#include "GlobalArrayReverse.hpp"

#include <unordered_set>

#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "FuncInfo.hpp"
#include "Instruction.hpp"
#include "LoopDetection.hpp"
#include "Tensor.hpp"
#include "Type.hpp"

#define DEBUG 0
#include "Util.hpp"


bool GlobalArrayReverse::isOnlyFunc(const Function* f, int argNo)
{
	Value* v = nullptr;
	for (auto use : f->get_use_list())
	{
		auto val = dynamic_cast<CallInst*>(use.val_);
		auto arg = ptrFrom(val->get_operand(argNo + 1));
		if (v == nullptr) v = arg;
		else if (v != arg) return false;
	}
	return true;
}

bool GlobalArrayReverse::legalGlobalVar(const Value* val, bool inCall)
{
	auto c0 = Constant::create(m_, 0);
	for (auto use : val->get_use_list())
	{
		auto val2 = dynamic_cast<Instruction*>(use.val_);
		switch (val2->get_instr_type()) // NOLINT(clang-diagnostic-switch-enum)
		{
			case Instruction::load:
			case Instruction::store:
			case Instruction::memclear_: break;
			case Instruction::call:
				{
					if (inCall) return false;
					// 不能确定库函数的行为
					auto callF = dynamic_cast<Function*>(val2->get_operand(0));
					if (callF->is_lib_) return false;
					auto arg = callF->get_arg(use.arg_no_ - 1);
					if (!isOnlyFunc(callF, use.arg_no_ - 1)) return false;
					if (!legalGlobalVar(arg, true)) return false;
					break;
				}
			case Instruction::getelementptr:
				{
					if (val2->get_operands().size() == 2)
					{
						if (val2->get_operand(1) != c0) return false;
						if (!legalGlobalVar(val2, inCall)) return false;
					}
					else
					{
						if (inCall)
						{
							if (u2iNegThrow(val2->get_operands().size()) == varDimSize_ + 1)
							{
								importance_.emplace(dynamic_cast<GetElementPtrInst*>(val2));
							}
							else return false;
						}
						else
						{
							if (u2iNegThrow(val2->get_operands().size()) == varDimSize_ + 2)
							{
								importance_.emplace(dynamic_cast<GetElementPtrInst*>(val2));
							}
							else if (u2iNegThrow(val2->get_operands().size()) == 3)
							{
								if (val2->get_operand(1) != c0 || val2->get_operand(2) != c0) return false;
								if (!legalGlobalVar(val2, inCall)) return false;
							}
							else return false;
						}
					}
					break;
				}
			case Instruction::memcpy_: return false;
			case Instruction::nump2charp:
			case Instruction::global_fix:
				{
					if (!legalGlobalVar(val2, inCall)) return false;
					break;
				}
			case Instruction::phi: return false;
			default:
				ASSERT(false);
				return false;
		}
	}
	return true;
}

// IO 操作耗费时间很多, 忽略这部分影响不大的部分
bool GlobalArrayReverse::noValueForReverse(BasicBlock* bb)
{
	auto loop = manager_->getFuncInfo<LoopDetection>(bb->get_parent());
	auto l = loop->loopOfBlock(bb);
	if (l == nullptr) return false;
	if (noValue_.count(l)) return true;
	for (auto bbb : l->get_blocks())
	{
		for (auto inst : bbb->get_instructions())
		{
			if (inst->is_call() && dynamic_cast<Function*>(inst->get_operand(0))->is_lib_)
			{
				noValue_.emplace(l);
				return true;
			}
		}
	}
	return false;
}

void GlobalArrayReverse::run()
{
	//PASS_SUFFIX;
	info_ = manager_->getGlobalInfo<FuncInfo>();
	std::list<GlobalVariable*>& globs = m_->get_global_variable();
	for (auto glob : globs)
	{
		if (!glob->get_type()->toPointerType()->typeContained()->isArrayType()) continue;
		if (glob->get_init()->segmentCount() != 1 || glob->get_init()->segment(0).second == 0) continue;
		auto& dims = glob->get_type()->toPointerType()->typeContained()->toArrayType()->dimensions();
		varDimSize_ = u2iNegThrow(dims.size());
		if (varDimSize_ == 1) continue;
		importance_.clear();
		if (!legalGlobalVar(glob, false)) continue;
		if (varDimSize_ != 2) continue;
		if (dims[0] != dims[1]) continue;
		if (importance_.empty()) continue;
		int needCount = 0;
		LOG(color::yellow("Check ") + glob->print());
		PUSH;
		for (auto i : importance_)
		{
			auto bb = i->get_parent();
			if (noValueForReverse(bb)) continue;
			LOG(color::yellow("Check ") + i->print());
			int begin = 1;
			if (u2iNegThrow(i->get_operands().size()) == varDimSize_ + 2) begin = 2;
			Value* arg0 = i->get_operand(begin);
			Value* arg1 = i->get_operand(begin + 1);
			if (arg0 == arg1) continue;
			auto ld = manager_->getFuncInfo<LoopDetection>(i->get_parent()->get_parent());
			Instruction* i0 = dynamic_cast<Instruction*>(arg0);
			Instruction* i1 = dynamic_cast<Instruction*>(arg1);
			Loop* l0;
			Loop* l1;
			if (i0 == nullptr) l0 = nullptr;
			else l0 = ld->loopOfBlock(i0->get_parent());
			if (i1 == nullptr) l1 = nullptr;
			else l1 = ld->loopOfBlock(i1->get_parent());
			int l0d = l0 == nullptr ? 0 : l0->depth();
			int l1d = l1 == nullptr ? 0 : l1->depth();
			int md = l0d > l1d ? l0d : l1d;
			int dd = l0d > l1d ? md : -md;
			if (l0d == l1d) dd = 0;
			PUSH;
			LOGIF(color::red("Bad"), dd > 0);
			LOGIF(color::green("Good"), dd < 0);
			POP;
			needCount += dd;
		}
		POP;
		if (needCount > 0)
		{
			LOG(color::pink("Reverse ") + glob->print());
			for (auto i : importance_)
			{
				int begin = 1;
				if (u2iNegThrow(i->get_operands().size()) == varDimSize_ + 2) begin = 2;
				Value* arg0 = i->get_operand(begin);
				Value* arg1 = i->get_operand(begin + 1);
				i->set_operand(begin, arg1);
				i->set_operand(begin + 1, arg0);
			}
		}
	}
	//PASS_SUFFIX;
}

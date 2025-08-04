#include "MachineModule.hpp"

#include <algorithm>
#include <stdexcept>
#include <ostream>

#include "MachineFunction.hpp"
#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "CountLZ.hpp"
#include "Instruction.hpp"
#include "LoopDetection.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineOperand.hpp"
#include "Module.hpp"
#include "CodeString.hpp"
#include "MachineInstruction.hpp"

using namespace std;


MModule::MModule()
{
	Register* r;
	for (int i = 0; i < 8; i++)
	{
		// 0 - 7
		r = new Register{i, true, "X" + to_string(i), "W" + to_string(i)};
		r->canAllocate_ = true;
		r->callerSave_ = true;
		iregs_.emplace_back(r);
	}
	for (int i = 0; i < 7; i++)
	{
		// 8 - 14
		r = new Register{i + 8, true, "X" + to_string(i + 9), "W" + to_string(i + 9)};
		r->canAllocate_ = true;
		r->callerSave_ = true;
		iregs_.emplace_back(r);
	}
	for (int i = 0; i < 2; i++)
	{
		// 15 - 16 IP0/IP1
		r = new Register{i + 15, true, "X" + to_string(i + 16), "W" + to_string(i + 16)};
		iregs_.emplace_back(r);
	}
	for (int i = 0; i < 10; i++)
	{
		// 17 - 26
		r = new Register{i + 17, true, "X" + to_string(i + 19), "W" + to_string(i + 19)};
		r->canAllocate_ = true;
		r->calleeSave_ = true;
		iregs_.emplace_back(r);
	}
	// (FP, 不过我们不使用帧指针)
	r = new Register{27, true, "X29", "W29"};
	r->canAllocate_ = true;
	r->calleeSave_ = true; // 调用对象保存, 我们会保存调用我们函数的对象的 FP, 如果它使用了帧指针, 其不会被破坏
	iregs_.emplace_back(r);
	// (LR, 需要时保存, 不需要时无需保存, 保存后可当作一般寄存器来分配)
	r = new Register{28, true, "X30", "W30"};
	r->canAllocate_ = true;
	r->callerSave_ = true; // 调用者保存(考虑到进行函数调用会破坏它), 作为代价, 使用一个 vreg 来防止其被破坏
	iregs_.emplace_back(r);
	r = new Register{29, true, "XZR", "WZR"};
	iregs_.emplace_back(r);
	r = new Register{30, true, "SP", ""};
	iregs_.emplace_back(r);
	r = new Register{31, true, "NZCV", "NZCV"};
	iregs_.emplace_back(r);
	for (int i = 0; i < 8; i++)
	{
		r = new Register{i, false, "Q" + to_string(i), "S" + to_string(i)};
		r->canAllocate_ = true;
		r->callerSave_ = true;
		fregs_.emplace_back(r);
	}
	for (int i = 8; i < 16; i++)
	{
		r = new Register{i, false, "Q" + to_string(i), "S" + to_string(i)};
		r->canAllocate_ = true;
		r->calleeSave_ = true;
		fregs_.emplace_back(r);
	}
	for (int i = 16; i < 18; i++)
	{
		r = new Register{i, false, "Q" + to_string(i), "S" + to_string(i)};
		fregs_.emplace_back(r);
	}
	for (int i = 18; i < 32; i++)
	{
		r = new Register{i, false, "Q" + to_string(i), "S" + to_string(i)};
		r->canAllocate_ = true;
		r->callerSave_ = true;
		fregs_.emplace_back(r);
	}
	memcpy_ = MFunction::createFunc("__memcpy__", this);
	memcpy_->destroyRegs_ = DynamicBitset{RegisterCount()};
	for (int i = 0; i < 2; i++)
		memcpy_->destroyRegs_.set(i);
	for (int i = 0; i < 4; i++)
		memcpy_->destroyRegs_.set(i + IRegisterCount());
	memclr_ = MFunction::createFunc("__memclr__", this);
	memclr_->destroyRegs_ = DynamicBitset{RegisterCount()};
	memclr_->destroyRegs_.set(0);
	for (int i = 0; i < 4; i++)
		memclr_->destroyRegs_.set(i + IRegisterCount());
}

void MModule::accept(Module* module)
{
	module->set_print_name();
	std::map<GlobalVariable*, GlobalAddress*> global_address;
	for (auto& g : module->get_global_variable())
	{
		global_address.emplace(g, new GlobalAddress{this, g});
	}
	const auto& funcs = module->get_functions();
	std::map<Function*, MFunction*> cache;
	for (const auto& func : funcs)
	{
		auto* mfunc = MFunction::createFunc(func->get_name(), this);
		if (func->is_lib_)
			lib_functions_.emplace_back(mfunc);
		else functions_.emplace_back(mfunc);
		mfunc->destroyRegs_ = DynamicBitset{RegisterCount()};
		for (int i = 0; i < 32; i++)
			if (iregs_[i]->callerSave_)
				mfunc->destroyRegs_.set(i);
		for (int i = 0; i < 32; i++)
			if (fregs_[i]->callerSave_)
				mfunc->destroyRegs_.set(i + IRegisterCount());
		cache.emplace(func, mfunc);
	}

	for (auto& f : allFuncs_) f->called_ = DynamicBitset{u2iNegThrow(allFuncs_.size())};

	for (auto& [bb,mbb] : cache)
	{
		if (bb->is_lib_) continue;
		mbb->preprocess(bb);
	}
	for (auto& [bb, mbb] : cache)
	{
		if (bb->is_lib_) continue;
		mbb->accept(bb, cache, global_address);
	}
}

std::string MModule::print() const
{
	string ret;
	for (auto& func : functions_)
	{
		ret += func->print() + "\n";
	}
	if (!ret.empty()) ret.pop_back();
	return ret;
}

int MModule::IRegisterCount() const
{
	return u2iNegThrow(iregs_.size());
}

int MModule::FRegisterCount() const
{
	return u2iNegThrow(fregs_.size());
}

int MModule::RegisterCount() const
{
	return u2iNegThrow(iregs_.size() + fregs_.size());
}

const std::vector<Register*>& MModule::IRegs() const
{
	return iregs_;
}

const std::vector<Register*>& MModule::FRegs() const
{
	return fregs_;
}


std::vector<GlobalAddress*> MModule::constGlobalAddresses() const
{
	vector<GlobalAddress*> v;
	for (auto i : globals_)
	{
		if (i->const_) v.emplace_back(i);
	}
	return v;
}

std::vector<GlobalAddress*> MModule::ncnzGlobalAddresses() const
{
	vector<GlobalAddress*> v;
	for (auto i : globals_)
	{
		if (!i->const_ && (i->data_->segmentCount() > 1 || !i->data_->segmentIsDefault(0))) v.emplace_back(i);
	}
	return v;
}

std::vector<GlobalAddress*> MModule::ncZeroGlobalAddresses() const
{
	vector<GlobalAddress*> v;
	for (auto i : globals_)
	{
		if (!i->const_ && i->data_->segmentCount() == 1 && i->data_->segmentIsDefault(0)) v.emplace_back(i);
	}
	return v;
}


namespace
{
	string condName(Instruction::OpID cond)
	{
		switch (cond)
		{
		case Instruction::ge: return "GE";
		case Instruction::gt: return "GT";
		case Instruction::le: return "LE";
		case Instruction::lt: return "LT";
		case Instruction::eq: return "EQ";
		case Instruction::ne: return "NE";
		default: break;
		}
		throw runtime_error("unexpected");
	}
}
MModule::~MModule()
{
	for (auto i : globals_) delete i;
	for (auto i : allFuncs_) delete i;
	for (auto [i,j] : func_address_) delete j;
	for (auto [i, j] : imm_cache_) delete j;
	for (auto i : iregs_) delete i;
	for (auto i : fregs_) delete i;
	delete moduleSuffix_;
	delete modulePrefix_;
}

std::ostream& operator<<(std::ostream& os, const MModule* module)
{
	if (module->modulePrefix_) os << module->modulePrefix_;
	for (auto f : module->functions_)
	{
		if (f->funcPrefix_) os << f->funcPrefix_;
		for (auto bb : f->blocks())
		{
			if (!f->useList()[BlockAddress::get(bb)].empty())
			{
				if (bb->name() == "search_2")
				{
					int iii = 0;
				}
				os << bb->name() << ":\n";
			}
			if (bb->blockPrefix_) os << bb->blockPrefix_;
			for (auto inst : bb->instructions())
				os << inst->str;
			if (int c = bb->needBranchCount())
			{
				auto br = dynamic_cast<MB*>(bb->instructions().back());
				if (br->isCondBranch())
				{
					os << "\tB." + condName(br->op()) + " " + br->block2GoL()->name() + '\n';
					if (c > 1) os << "\tB " + br->block2GoR()->name() + '\n';
				}
				else os << "\tB " + br->block2GoL()->name() + '\n';
			}
		}
		if (f->funcSuffix_) os << f->funcSuffix_;
		os << f->sizeSuffix_;
	}
	if (module->moduleSuffix_) os << module->moduleSuffix_;
	return os;
}

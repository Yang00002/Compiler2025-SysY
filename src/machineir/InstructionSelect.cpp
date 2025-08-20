#include "InstructionSelect.hpp"

#include "BasicBlock.hpp"
#include "Config.hpp"
#include "Constant.hpp"
#include "Instruction.hpp"

#define DEBUG 0
#include "Util.hpp"

using namespace std;

void InstructionSelect::runInner() const
{
	LOG(color::cyan("InstructionSelect On ") + f_->get_name() + color::cyan(" Block ") + b_->get_name());
	auto& insts = b_->get_instructions();
	auto it = insts.begin();
	auto ed = insts.end();
	while (it != ed)
	{
		auto inst = it.get_and_add();
		if (inst->get_use_list().size() != 1) continue;
		auto u = inst->get_use_list().front();
		auto use = dynamic_cast<Instruction*>(u.val_);
		if (use->get_parent() != b_) continue;
		int id = u.arg_no_;
		if (inst->is_gep())
		{
			if (!use->is_gep()) continue;
			LOG(color::yellow("Merge"));
			LOG(inst->print());
			LOG(use->print());
			vector<Value*> nidx{inst->get_operands().begin() + 1, inst->get_operands().end()};
			auto& opu = use->get_operands();
			ASSERT(
				dynamic_cast<Constant*>(opu[1]) && dynamic_cast<Constant*>(opu[1])->
				getIntConstant() == 0);
			int size = u2iNegThrow(opu.size());
			for (int i = 2; i < size; i++) nidx.emplace_back(opu[i]);
			auto ninst = GetElementPtrInst::create_gep(inst->get_operand(0), nidx, nullptr);
			ninst->set_parent(b_);
			use->replace_all_use_with(ninst);
			LOG(color::green("Get"));
			LOG(ninst->print());
			LOG("");
			--it;
			b_->erase_instr(use);
			delete use;
			delete it.replaceWith(ninst);
			continue;
		}
		if (useBinaryInstMerge)
		{
			if (inst->is_mul())
			{
				if (use->is_sub())
				{
					if (id != 1) continue;
					if (onlyMergeMulAndASWhenASUseAllReg)
					{
						auto c = dynamic_cast<Constant*>(use->get_operand(0));
						if (c != nullptr && c->getIntConstant() != 0) continue;
					}
					LOG(color::yellow("Merge"));
					LOG(inst->print());
					LOG(use->print());
					auto ninst = MulIntegratedInst::create_msub(inst->get_operand(0), inst->get_operand(1),
					                                            use->get_operand(0),
					                                            nullptr);
					ninst->set_parent(b_);
					use->replace_all_use_with(ninst);
					LOG(color::green("Get"));
					LOG(ninst->print());
					LOG("");
					--it;
					b_->erase_instr(use);
					delete use;
					delete it.replaceWith(ninst);
					continue;
				}
				if (use->is_add())
				{
					if (id == 1)
					{
						if (onlyMergeMulAndASWhenASUseAllReg)
						{
							auto c = dynamic_cast<Constant*>(use->get_operand(0));
							if (c != nullptr && c->getIntConstant() != 0) continue;
						}
						LOG(color::yellow("Merge"));
						LOG(inst->print());
						LOG(use->print());
						auto ninst = MulIntegratedInst::create_madd(inst->get_operand(0), inst->get_operand(1),
							use->get_operand(0),
							nullptr);
						ninst->set_parent(b_);
						use->replace_all_use_with(ninst);
						LOG(color::green("Get"));
						LOG(ninst->print());
						LOG("");
						--it;
						b_->erase_instr(use);
						delete use;
						delete it.replaceWith(ninst);
					}
					else
					{

						if (onlyMergeMulAndASWhenASUseAllReg)
						{
							auto c = dynamic_cast<Constant*>(use->get_operand(1));
							if (c != nullptr && c->getIntConstant() != 0) continue;
						}
						LOG(color::yellow("Merge"));
						LOG(inst->print());
						LOG(use->print());
						auto ninst = MulIntegratedInst::create_madd(inst->get_operand(0), inst->get_operand(1),
							use->get_operand(1),
							nullptr);
						ninst->set_parent(b_);
						use->replace_all_use_with(ninst);
						LOG(color::green("Get"));
						LOG(ninst->print());
						LOG("");
						--it;
						b_->erase_instr(use);
						delete use;
						delete it.replaceWith(ninst);
					}
				}
				continue;
			}
			if (inst->is_fmul())
			{
				if (mergeFloatBinaryInst)
				{
					if (use->is_fsub())
					{
						if (id != 1) continue;
						LOG(color::yellow("Merge"));
						LOG(inst->print());
						LOG(use->print());
						auto ninst = MulIntegratedInst::create_msub(inst->get_operand(0), inst->get_operand(1),
						                                            use->get_operand(0),
						                                            nullptr);
						ninst->set_parent(b_);
						use->replace_all_use_with(ninst);
						LOG(color::green("Get"));
						LOG(ninst->print());
						LOG("");
						--it;
						b_->erase_instr(use);
						delete use;
						delete it.replaceWith(ninst);
						continue;
					}
					if (use->is_fadd())
					{
						if (id != 1) continue;
						LOG(color::yellow("Merge"));
						LOG(inst->print());
						LOG(use->print());
						auto ninst = MulIntegratedInst::create_madd(inst->get_operand(0), inst->get_operand(1),
						                                            use->get_operand(0),
						                                            nullptr);
						ninst->set_parent(b_);
						use->replace_all_use_with(ninst);
						LOG(color::green("Get"));
						LOG(ninst->print());
						LOG("");
						--it;
						b_->erase_instr(use);
						delete use;
						delete it.replaceWith(ninst);
					}
					continue;
				}
				if (use->is_fsub())
				{
					if (id != 1) continue;
					if (auto c = dynamic_cast<Constant*>(use->get_operand(0));
						c == nullptr || c->getFloatConstant() != 0.0f)
						continue;
					LOG(color::yellow("Merge"));
					LOG(inst->print());
					LOG(use->print());
					auto ninst = MulIntegratedInst::create_mneg(inst->get_operand(0), inst->get_operand(1), nullptr);
					ninst->set_parent(b_);
					use->replace_all_use_with(ninst);
					LOG(color::green("Get"));
					LOG(ninst->print());
					LOG("");
					--it;
					b_->erase_instr(use);
					delete use;
					delete it.replaceWith(ninst);
					continue;
				}
			}
		}
	}
}

void InstructionSelect::run()
{
	PASS_SUFFIX;
	PREPARE_PASS_MSG;
	LOG(color::cyan("Run InstructionSelect Pass"));
	PUSH;
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_) continue;
		f_ = f;
		for (auto bb : f->get_basic_blocks())
		{
			b_ = bb;
			runInner();
		}
	}
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("InstructionSelect Done"));
}

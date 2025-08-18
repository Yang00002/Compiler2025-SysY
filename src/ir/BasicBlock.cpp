#include "BasicBlock.hpp"
#include "Function.hpp"
#include "IRPrinter.hpp"
#include "System.hpp"
#include "Type.hpp"
#include "Util.hpp"


BasicBlock::BasicBlock(Module* m, const std::string& name = "",
                       Function* parent = nullptr)
	: Value(Types::LABEL, name), parent_(parent)
{
	ASSERT(parent && "currently parent should not be nullptr");
	parent_->add_basic_block(this);
}

Module* BasicBlock::get_module() const { return get_parent()->get_parent(); }
void BasicBlock::erase_from_parent() { this->get_parent()->remove(this); }

BasicBlock::~BasicBlock()
{
	for (const auto i : instr_list_)
		delete i;
}

void BasicBlock::redirect_suc_basic_block(BasicBlock* from, BasicBlock* to)
{
	if (instr_list_.back()->replaceAllOperandMatchs(from, to))
	{
		succ_bbs_.remove(from);
		succ_bbs_.emplace_back(to);
		from->pre_bbs_.remove(this);
		to->pre_bbs_.emplace_back(this);
	}
}

void BasicBlock::reset_suc_basic_block(Value* cond, BasicBlock* l, BasicBlock* r)
{
	if (succ_bbs_.size() == 2)
	{
		if (succ_bbs_.front() != l && succ_bbs_.front() != r)
		{
			succ_bbs_.front()->remove_pre_basic_block(this);
			succ_bbs_.pop_front();
		}
	}
	if (succ_bbs_.back() != l && succ_bbs_.back() != r)
	{
		succ_bbs_.back()->remove_pre_basic_block(this);
		succ_bbs_.pop_back();
	}
	if (succ_bbs_.empty() || (succ_bbs_.front() != l && succ_bbs_.back() != l))
	{
		succ_bbs_.emplace_back(l);
		l->pre_bbs_.emplace_back(this);
	}
	if (succ_bbs_.front() != r && succ_bbs_.back() != r)
	{
		succ_bbs_.emplace_back(r);
		r->pre_bbs_.emplace_back(this);
	}
	auto back = instr_list_.back();
	if (l == r)
	{
		back->remove_all_operands();
		back->add_operand(l);
	}
	else
	{
		back->remove_all_operands();
		back->add_operand(cond);
		back->add_operand(l);
		back->add_operand(r);
	}
}

void BasicBlock::remove_succ_block_and_update_br(BasicBlock* rm)
{
	auto inst = instr_list_.back();
	if (inst->get_operands().size() != 3)
	{
		if (inst->get_operand(0) == rm)
		{
			instr_list_.pop_back();
			succ_bbs_.remove(rm);
			rm->pre_bbs_.remove(this);
			delete inst;
		}
		return;
	}
	BasicBlock* ot = nullptr;
	bool ok = false;
	for (int i = 1; i < 3; i++)
	{
		BasicBlock* op = dynamic_cast<BasicBlock*>(inst->get_operand(i));
		if (op == rm)
		{
			ok = true;
		}
		else ot = op;
	}
	if (!ok) return;
	succ_bbs_.remove(rm);
	rm->pre_bbs_.remove(this);
	inst->remove_all_operands();
	inst->add_operand(ot);
}

bool BasicBlock::replace_self_with_block(BasicBlock* bb)
{
	if (bb == this) return false;
	const auto& pb = get_pre_basic_blocks();
	bool multiPre = pb.size() > 1;
	BasicBlock* preBB = pb.empty() ? nullptr : pb.front();
	// 收集使用该基本块定值的 phi 指令
	std::list<std::pair<PhiInst*, int>> phiInsts;
	for (auto use : get_use_list())
	{
		auto phi = dynamic_cast<PhiInst*>(use.val_);
		{
			if (phi != nullptr)
			{
				// 理论上不该对这种基本块运行该指令
				ASSERT(preBB != nullptr);
				// 替代会增加 phi 的长度
				if (multiPre) return false;
				auto& ops = phi->get_operands();
				int s = u2iNegThrow(ops.size());
				for (int i = 1; i < s; i += 2)
				{
					// 依赖跳转进行定值
					if (ops[i] == preBB) return false;
				}
				phiInsts.emplace_back(phi, use.arg_no_);
			}
		}
	}
	if (!phiInsts.empty())
	{
		for (auto phi_inst : phiInsts)
		{
			auto phi = phi_inst.first;
			auto id = phi_inst.second;
			auto val = phi->get_operand(id - 1);
			phi->add_phi_pair_operand(val, preBB);
		}
	}
	for (auto pre_basic_block : get_pre_basic_blocks())
	{
		pre_basic_block->remove_succ_basic_block(this);
		pre_basic_block->add_succ_basic_block(bb);
		bb->add_pre_basic_block(pre_basic_block);
		auto term = pre_basic_block->get_terminator();
		int size = u2iNegThrow(term->get_operands().size());
		for (int i = 0; i < size; i++)
		{
			auto op = term->get_operand(i);
			if (op == this)
			{
				term->set_operand(i, bb);
			}
		}
	}
	pre_bbs_.clear();
	return true;
}

void BasicBlock::add_block_before(BasicBlock* bb)
{
	bb->instr_list_.addAllPhiAndAllocas(instr_list_);
	instr_list_.remove_phi_and_allocas();
	for (auto inst : bb->instr_list_) inst->set_parent(bb);
	auto pres = pre_bbs_;
	for (auto pre : pres) pre->redirect_suc_basic_block(this, bb);
	BranchInst::create_br(this, bb);
}

void BasicBlock::add_block_before(BasicBlock* bb, const BasicBlock* stillGotoSelf)
{
	auto begin = instr_list_.begin();
	while (begin != instr_list_.phi_alloca_end())
	{
		std::map<BasicBlock*, Value*> selfPhiMap;
		std::map<BasicBlock*, Value*> bbPhiMap;
		auto phi = dynamic_cast<PhiInst*>(begin.get_and_add());
		for (auto& [i, j] : phi->get_phi_pairs())
		{
			if (stillGotoSelf == j) selfPhiMap.emplace(j, i);
			else bbPhiMap.emplace(j, i);
		}
		ASSERT(!bbPhiMap.empty());
		if (bbPhiMap.size() == 1)
		{
			phi->replaceAllOperandMatchs(bbPhiMap.begin()->first, bb);
		}
		else
		{
			auto phiAtBB = PhiInst::create_phi(phi->get_type(), bb);
			for (auto& [i, j] : bbPhiMap) phiAtBB->add_phi_pair_operand(j, i);
			selfPhiMap.emplace(bb, phiAtBB);
			if (selfPhiMap.size() == 1)
			{
				phi->replace_all_use_with(phiAtBB);
				begin.remove_pre();
				delete phi;
			}
			else
			{
				phi->remove_all_operands();
				for (auto& [i, j] : selfPhiMap) phi->add_phi_pair_operand(j, i);
			}
		}
	}
	auto pres = pre_bbs_;
	for (auto pre : pres)
		if (pre != stillGotoSelf) pre->redirect_suc_basic_block(this, bb);
	BranchInst::create_br(this, bb);
}

void BasicBlock::add_block_before(BasicBlock* bb, const std::unordered_set<BasicBlock*>& stillGotoSelf)
{
	auto begin = instr_list_.begin();
	while (begin != instr_list_.phi_alloca_end())
	{
		std::map<BasicBlock*, Value*> selfPhiMap;
		std::map<BasicBlock*, Value*> bbPhiMap;
		auto phi = dynamic_cast<PhiInst*>(begin.get_and_add());
		for (auto& [i, j] : phi->get_phi_pairs())
		{
			if (stillGotoSelf.count(j)) selfPhiMap.emplace(j, i);
			else bbPhiMap.emplace(j, i);
		}
		ASSERT(!bbPhiMap.empty());
		if (bbPhiMap.size() == 1)
		{
			phi->replaceAllOperandMatchs(bbPhiMap.begin()->first, bb);
		}
		else
		{
			auto phiAtBB = PhiInst::create_phi(phi->get_type(), bb);
			for (auto& [i, j] : bbPhiMap) phiAtBB->add_phi_pair_operand(j, i);
			selfPhiMap.emplace(bb, phiAtBB);
			if (selfPhiMap.size() == 1)
			{
				phi->replace_all_use_with(phiAtBB);
				begin.remove_pre();
				delete phi;
			}
			else
			{
				phi->remove_all_operands();
				for (auto& [i, j] : selfPhiMap) phi->add_phi_pair_operand(j, i);
			}
		}
	}
	auto pres = pre_bbs_;
	for (auto pre : pres)
		if (!stillGotoSelf.count(pre)) pre->redirect_suc_basic_block(this, bb);
	BranchInst::create_br(this, bb);
}

void BasicBlock::add_block_before(BasicBlock* bb, const std::set<BasicBlock*>& stillGotoSelf)
{
	auto begin = instr_list_.begin();
	while (begin != instr_list_.phi_alloca_end())
	{
		std::map<BasicBlock*, Value*> selfPhiMap;
		std::map<BasicBlock*, Value*> bbPhiMap;
		auto phi = dynamic_cast<PhiInst*>(begin.get_and_add());
		for (auto& [i, j] : phi->get_phi_pairs())
		{
			if (stillGotoSelf.count(j)) selfPhiMap.emplace(j, i);
			else bbPhiMap.emplace(j, i);
		}
		Value* fromBBGet = nullptr;
		ASSERT(!bbPhiMap.empty());
		if (bbPhiMap.size() == 1)
		{
			phi->replaceAllOperandMatchs(bbPhiMap.begin()->first, bb);
		}
		else
		{
			auto phiAtBB = PhiInst::create_phi(phi->get_type(), bb);
			for (auto& [i, j] : bbPhiMap) phiAtBB->add_phi_pair_operand(j, i);
			selfPhiMap.emplace(bb, phiAtBB);
			if (selfPhiMap.size() == 1)
			{
				phi->replace_all_use_with(phiAtBB);
				begin.remove_pre();
				delete phi;
			}
			else
			{
				phi->remove_all_operands();
				for (auto& [i, j] : selfPhiMap) phi->add_phi_pair_operand(j, i);
			}
		}
	}
	auto pres = pre_bbs_;
	for (auto pre : pres)
		if (!stillGotoSelf.count(pre)) pre->redirect_suc_basic_block(this, bb);
	BranchInst::create_br(this, bb);
}

void BasicBlock::replace_terminate_with_return_value(Value* value)
{
	ASSERT(is_terminated() && "not terminated");
	auto pre = get_terminator();
	auto preRet = dynamic_cast<ReturnInst*>(pre);
	auto preBr = dynamic_cast<BranchInst*>(pre);
	if (!preRet)
	{
		if (preBr->is_cond_br())
		{
			auto t1 = dynamic_cast<BasicBlock*>(preBr->get_operand(1));
			auto t2 = dynamic_cast<BasicBlock*>(preBr->get_operand(2));
			t1->remove_pre_basic_block(this);
			t2->remove_pre_basic_block(this);
			remove_succ_basic_block(t1);
			remove_succ_basic_block(t2);
		}
		else
		{
			auto t1 = dynamic_cast<BasicBlock*>(preBr->get_operand(0));
			t1->remove_pre_basic_block(this);
			remove_succ_basic_block(t1);
		}
	}
	instr_list_.pop_back();
	ReturnInst::create_ret(value, this);
}

bool BasicBlock::is_terminated() const
{
	if (instr_list_.empty())
	{
		return false;
	}
	switch (instr_list_.back()->get_instr_type()) // NOLINT(clang-diagnostic-switch-enum)
	{
		case Instruction::ret:
		case Instruction::br:
			return true;
		default:
			return false;
	}
}

Instruction* BasicBlock::get_terminator() const
{
	ASSERT(is_terminated() &&
		"Trying to get terminator from an bb which is not terminated");
	return instr_list_.back();
}

Instruction* BasicBlock::get_terminator_or_null() const
{
	if (is_terminated()) return instr_list_.back();
	return nullptr;
}

void BasicBlock::add_instruction(Instruction* instr)
{
	ifThenOrThrow(instr->is_phi(), !is_entry_block(), "Phi can only insert to not entry block");
	ifThenOrThrow(instr->is_alloca(), is_entry_block(), "Alloca can only insert to entry block");
	ifThenOrThrow(!instr->is_alloca() && !instr->is_phi(), !is_terminated(), "Can not insert to terminated block");
	instr_list_.emplace_back(instr);
}

void BasicBlock::add_instr_begin(Instruction* instr)
{
	ifThenOrThrow(instr->is_phi(), !is_entry_block(), "Phi can only insert to not entry block");
	ifThenOrThrow(instr->is_alloca(), is_entry_block(), "Alloca can only insert to entry block");
	instr_list_.emplace_front(instr);
}

void BasicBlock::erase_instr(const Instruction* instr)
{
	instr_list_.remove_first(instr);
}

void BasicBlock::erase_instr_from_last(const Instruction* instr)
{
	instr_list_.remove_last(instr);
}

std::string BasicBlock::print()
{
	std::string bb_ir;
	bb_ir += this->get_name();
	bb_ir += ":";
	// print prebb
	if (!this->get_pre_basic_blocks().empty())
	{
		bb_ir += "\t\t; preds = ";
	}
	for (auto bb : this->get_pre_basic_blocks())
	{
		if (bb != *this->get_pre_basic_blocks().begin())
		{
			bb_ir += ", ";
		}
		bb_ir += print_as_op(bb, false);
	}
	if (!this->get_succ_basic_blocks().empty())
	{
		bb_ir += "\t\t; succs = ";
	}
	for (auto bb : this->get_succ_basic_blocks())
	{
		if (bb != *this->get_succ_basic_blocks().begin())
		{
			bb_ir += ", ";
		}
		bb_ir += print_as_op(bb, false);
	}
	// print prebb
	if (!this->get_parent())
	{
		bb_ir += "\n";
		bb_ir += "; Error: Block without parent!";
	}
	bb_ir += "\n";
	for (const auto instr : this->get_instructions())
	{
		bb_ir += "  ";
		bb_ir += instr->print();
		bb_ir += "\n";
	}

	return bb_ir;
}

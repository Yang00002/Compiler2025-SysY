#include "LoopRotate.hpp"
#include "LoopDetection.hpp"

#define DEBUG 0
#include "Config.hpp"
#include "Util.hpp"
using namespace std;

void LoopRotate::run()
{
	LOG(m_->print());
	PREPARE_PASS_MSG;
	LOG(color::cyan("Run LoopRotate Pass"));
	PUSH;
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_)continue;
		f_ = f;
		loops_ = manager_->getFuncInfo<LoopDetection>(f_);
		dominators_ = manager_->getFuncInfo<Dominators>(f_);
		runOnFunc();
	}
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("LoopRotate Done"));
}

void LoopRotate::runOnFunc()
{
	LOG(color::cyan("Run LoopRotate On ") + f_->get_name());
	auto lps = loops_->get_loops();
	bool ret = false;
	for (auto l : lps)
	{
		if (l->get_parent() == nullptr)
		{
			ret |= runOnLoops(l->get_sub_loops());
			loop_ = l;
			ret |= runOnLoop();
		}
	}
	if (ret) manager_->flushFuncInfo<Dominators>();
}

bool LoopRotate::runOnLoops(const std::vector<Loop*>& loops)
{
	bool ret = false;
	for (auto loop : loops)
	{
		ret |= runOnLoops(loop->get_sub_loops());
		loop_ = loop;
		ret |= runOnLoop();
	}
	return ret;
}

void LoopRotate::splicePreheader() const
{
	auto head = loop_->get_header();
	auto pre = loop_->get_preheader();
	auto newPreHead = BasicBlock::create(m_, "", f_);
	pre->redirect_suc_basic_block(head, newPreHead);
	BranchInst::create_br(head, newPreHead);
	loop_->add_block_casecade(newPreHead, false);
	loop_->set_preheader(newPreHead);
	for (auto bb : newPreHead->get_succ_basic_blocks())
	{
		for (auto phi : bb->get_instructions().phi_and_allocas()) phi->replaceAllOperandMatchs(pre, newPreHead);
	}
}

void LoopRotate::copyInst2(const BasicBlock* enter, BasicBlock* from, BasicBlock* to,
                           unordered_map<Value*, Value*>& v_map)
{
	auto ed = to->get_instructions().back();
	to->get_instructions().pop_back();
	for (auto ins : from->get_instructions().phi_and_allocas())
	{
		auto phi = dynamic_cast<PhiInst*>(ins);
		v_map.emplace(phi, phi->get_phi_val(enter));
	}
	for (auto ins : from->get_instructions().common_instructions())
	{
		auto cp = ins->copy(v_map);
		if (cp != nullptr)
			to->add_instruction(cp);
	}
	to->add_instruction(ed);
}


// 当旋转后, head 不能执行它的指令第二次, 而是应该推迟到 latch 执行
void LoopRotate::moveHeadInst2Latch(const unordered_map<Value*, Value*>& v_map) const
{
	auto latch = loop_->get_latch();

	auto blocksExcludeHeader = loop_->get_blocks();
	blocksExcludeHeader.erase(loop_->get_header());

	list<Instruction*> needPhi;

	// 收集需要插入 phi 的指令
	for (auto ci : loop_->get_header()->get_instructions().common_instructions())
	{
		for (auto use : ci->get_use_list())
		{
			auto inst = dynamic_cast<Instruction*>(use.val_);
			if (blocksExcludeHeader.count(inst->get_parent())) needPhi.emplace_back(inst);
		}
	}

	// 收集 phi 在循环内的值
	unordered_map<Value*, Value*> moveDownInstPhiReplace;
	for (auto pi : loop_->get_header()->get_instructions().phi_and_allocas())
	{
		auto phi = dynamic_cast<PhiInst*>(pi);
		auto val = phi->get_phi_val(latch);
		moveDownInstPhiReplace.emplace(pi, val);
	}

	// 插入值的 Phi, 更改使用
	for (auto i : needPhi)
	{
		auto outCorrespond = getOrDefault(v_map, i);
		auto phi = PhiInst::create_phi(i->get_type(), loop_->get_header());
		phi->add_phi_pair_operand(outCorrespond, loop_->get_preheader());
		phi->add_phi_pair_operand(i, latch);
		i->replace_use_with_if(phi, [&blocksExcludeHeader](const Use& u)-> bool
		{
			return blocksExcludeHeader.count(dynamic_cast<Instruction*>(u.val_)->get_parent());
		});
	}

	// 替换值, 下沉
	auto back = latch->get_instructions().back();
	latch->get_instructions().pop_back();
	auto insts = loop_->get_header()->get_instructions().common_instructions();
	auto it = insts.begin();
	auto ep = insts.rbegin();
	while (it != ep)
	{
		auto inst = it.get_and_add();
		int size = u2iNegThrow(inst->get_operands().size());
		for (int i = 0; i < size; i++)
		{
			auto op = inst->get_operand(i);
			if (moveDownInstPhiReplace.count(op))
			{
				inst->set_operand(i, moveDownInstPhiReplace.at(op));
			}
		}
		it.remove_pre();
		inst->set_parent(latch);
		latch->add_instruction(inst);
	}
	latch->add_instruction(back);

	// 不是 while true
	if ((*ep)->get_operands().size() == 3)
	{
		// 修改 latch 跳转
		auto bbl = dynamic_cast<BasicBlock*>((*ep)->get_operand(1));
		if (loop_->get_blocks().count(bbl))
			latch->reset_suc_basic_block((*ep)->get_operand(0), loop_->get_header(),
			                             dynamic_cast<BasicBlock*>((*ep)->get_operand(2)));
		else
			latch->reset_suc_basic_block((*ep)->get_operand(0), bbl, loop_->get_header());

		// 修改 head 跳转
		auto ext = loop_->exits().at(loop_->get_header());
		loop_->get_header()->remove_succ_block_and_update_br(ext);
		for (auto phi : ext->get_instructions().phi_and_allocas())
		{
			phi->replaceAllOperandMatchs(loop_->get_header(), latch);
		}

		// 维护 exit
		loop_->remove_exit_casecade(loop_->get_header());
		loop_->add_exit_casecade(latch, ext);
	}
}

Value*  LoopRotate::getOrDefault(const unordered_map<Value*, Value*>& vmap, Value* val)
{
	if (vmap.count(val)) return vmap.at(val);
	return val;
}

void LoopRotate::toWhileTrue(const Loop::Iterator& msg) const
{
	auto pre = loop_->get_preheader();
	auto pp = pre;
	splicePreheader();
	pre = loop_->get_preheader();
	unordered_map<Value*, Value*> vmap;
	copyInst2(pre, loop_->get_header(), pp, vmap);
	pp->reset_suc_basic_block(getOrDefault(vmap,msg.br_->get_operand(0)), dynamic_cast<BasicBlock*>(msg.br_->get_operand(1)),
	                          dynamic_cast<BasicBlock*>(msg.br_->get_operand(2)));
	pp->redirect_suc_basic_block(msg.toLoop(loop_), pre);
	for (auto inst : loop_->exits().at(loop_->get_header())->get_instructions().phi_and_allocas())
	{
		auto phi = dynamic_cast<PhiInst*>(inst);
		dynamic_cast<PhiInst*>(inst)->add_phi_pair_operand(getOrDefault(vmap, phi->get_phi_val(loop_->get_header())), pp);
		dynamic_cast<PhiInst*>(inst)->remove_phi_operand(loop_->get_header());
	}
	loop_->get_header()->remove_succ_block_and_update_br(msg.exit(loop_));
	loop_->remove_exit_casecade(loop_->get_header());
	moveHeadInst2Latch(vmap);
}

void LoopRotate::erasePhi(const Loop::Iterator& msg) const
{
	auto pre = loop_->get_preheader();
	auto pp = pre;
	splicePreheader();
	pre = loop_->get_preheader();
	unordered_map<Value*, Value*> vmap;
	copyInst2(pre, loop_->get_header(), pp, vmap);
	pp->reset_suc_basic_block(msg.br_->get_operand(0), dynamic_cast<BasicBlock*>(msg.br_->get_operand(1)),
	                          dynamic_cast<BasicBlock*>(msg.br_->get_operand(2)));
	pp->redirect_suc_basic_block(msg.toLoop(loop_), pre);
	for (auto inst : loop_->exits().at(loop_->get_header())->get_instructions().phi_and_allocas())
	{
		auto phi = dynamic_cast<PhiInst*>(inst);
		dynamic_cast<PhiInst*>(inst)->add_phi_pair_operand(getOrDefault(vmap, phi->get_phi_val(loop_->get_header())), pp);
	}
	msg.cmp_->replaceAllOperandMatchs(msg.iterator_, msg.toLoopIterateValue(loop_));
	moveHeadInst2Latch(vmap);
}

void LoopRotate::rotate(const Loop::Iterator& msg) const
{
	auto pre = loop_->get_preheader();
	auto pp = pre;
	splicePreheader();
	pre = loop_->get_preheader();
	unordered_map<Value*, Value*> vmap;
	copyInst2(pre, loop_->get_header(), pp, vmap);
	pp->reset_suc_basic_block(msg.br_->get_operand(0), dynamic_cast<BasicBlock*>(msg.br_->get_operand(1)),
	                          dynamic_cast<BasicBlock*>(msg.br_->get_operand(2)));
	pp->redirect_suc_basic_block(msg.toLoop(loop_), pre);
	for (auto inst : loop_->exits().at(loop_->get_header())->get_instructions().phi_and_allocas())
	{
		auto phi = dynamic_cast<PhiInst*>(inst);
		dynamic_cast<PhiInst*>(inst)->add_phi_pair_operand(getOrDefault(vmap, phi->get_phi_val(loop_->get_header())), pp);
	}
	moveHeadInst2Latch(vmap);
}

bool LoopRotate::runOnLoop() const
{
	LOG("");
	LOG(color::blue("Run On Loop ") + loop_->print());
	PUSH;
	auto pre = loop_->get_preheader();
	if (pre == nullptr) return false;
	auto msg = loop_->getIterator();
	// 没有循环不变量, 因此也不需要旋转
	if (pre->get_instructions().commonInstSize() == 1)
	{
		if (!rotateLoopEvenIfNotHaveInvariant) return false;
		if (msg.notHaveIterator_) return false;
		if (msg.outIterateInsteadOfIn_)
		{
			toWhileTrue(msg);
			return true;
		}
		if (msg.phiDefinedByOut_)
		{
			erasePhi(msg);
			return true;
		}
		return false;
	}
	// 无法插入 guard
	if (msg.notHaveIterator_) return false;
	// 插入 guard, 改为 while true
	if (msg.outIterateInsteadOfIn_) toWhileTrue(msg);
		// 插入 guard, 消除 phi
	else if (msg.phiDefinedByOut_) erasePhi(msg);
	else rotate(msg);
	POP;
	return true;
}

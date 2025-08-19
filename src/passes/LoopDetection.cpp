#include "LoopDetection.hpp"
#include "BasicBlock.hpp"
#include "Dominators.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <queue>
#include <set>

#include "Constant.hpp"
#include "Instruction.hpp"


#define DEBUG 0
#include "Util.hpp"


LoopDetection::~LoopDetection()
{
	for (const auto& loop : loops_)
	{
		delete loop;
	}
}

void LoopDetection::run()
{
	Dominators* dominators = manager_->getFuncInfo<Dominators>(f_);
	for (auto bb : dominators->get_dom_post_order(f_))
	{
		BBset latches;
		for (auto& pred : bb->get_pre_basic_blocks())
		{
			if (dominators->is_dominate(bb, pred))
			{
				// pred is a back edge
				// pred -> bb , pred is the latch node
				latches.insert(pred);
			}
		}
		if (latches.empty())
		{
			continue;
		}
		// create loop
		auto loop = new Loop{this, bb};
		bb_to_loop_[bb] = loop;
		// add latch nodes
		for (auto& latch : latches)
		{
			loop->add_latch(latch);
		}
		loops_.push_back(loop);
		discover_loop_and_sub_loops(bb, latches, loop, dominators);
	}
}

bool Loop::valueInLoop(Value* val) const
{
	auto inst = dynamic_cast<Instruction*>(val);
	if (!inst) return false;
	return blocks_.count(inst->get_parent());
}

int Loop::depthTo(Loop* l) const
{
	int d = 0;
	Loop* p = const_cast<Loop*>(this);
	while (p != l && p != nullptr)
	{
		d++;
		p = p->parent_;
	}
	if (l == nullptr || p != nullptr) return d;
	int d1 = 0;
	if (p == nullptr && l != nullptr)
	{
		p = l;
		while (p != this && p != nullptr)
		{
			d1--;
			p = p->parent_;
		}
	}
	return d1;
}

int Loop::depthTo(Loop* a, Loop* l)
{
	if (a == nullptr && l == nullptr) return 0;
	Loop* p = nullptr;
	if (a != nullptr)
	{
		int d = 0;
		p = a;
		while (p != l && p != nullptr)
		{
			d++;
			p = p->parent_;
		}
		if (l == nullptr || p != nullptr) return d;
	}
	int d1 = 0;
	if (p == nullptr && l != nullptr)
	{
		p = l;
		while (p != a && p != nullptr)
		{
			d1--;
			p = p->parent_;
		}
	}
	return d1;
}

Value* Loop::Iterator::toLoopIterateValue(const Loop* loop) const
{
	auto phi = dynamic_cast<PhiInst*>(iterator_);
	if (phi == nullptr) return nullptr;
	return phi->get_phi_val(*loop->latches_.begin());
}

Value* Loop::Iterator::exitIterateValue(Loop* loop) const
{
	auto phi = dynamic_cast<PhiInst*>(iterator_);
	if (phi == nullptr) return nullptr;
	return phi->get_phi_val(exit(loop));
}

BasicBlock* Loop::Iterator::exit(Loop* loop) const
{
	if (br_ == nullptr) return nullptr;
	return loop->exits().at(br_->get_parent());
}

BasicBlock* Loop::Iterator::toLoop(const Loop* loop) const
{
	if (br_ == nullptr) return nullptr;
	for (int i = 1; i < 3; i++)
	{
		auto op = dynamic_cast<BasicBlock*>(br_->get_operand(i));
		if (loop->blocks_.count(op)) return op;
	}
	return nullptr;
}

void Loop::add_block_casecade(BasicBlock* bb, bool includeSelf)
{
	bool b = true;
	auto p = includeSelf ? this : parent_;
	while (p != nullptr)
	{
		p->add_block(bb);
		if (b)
		{
			detect_->setLoopOfBlock(bb, p);
			b = false;
		}
		p = p->parent_;
	}
}

void Loop::remove_sub_loop(const Loop* loop)
{
	auto it = sub_loops_.begin();
	auto ed = sub_loops_.end();
	while (it != ed)
	{
		if (*it == loop)
		{
			sub_loops_.erase(it);
			return;
		}
		++it;
	}
}

BasicBlock* Loop::get_latch() const
{
	if (latches_.empty()) return nullptr;
	return *latches_.begin();
}

void Loop::remove_exit_casecade(BasicBlock* bb)
{
	auto lp = this;
	while (lp != nullptr && lp->exits_.count(bb))
	{
		lp->exits_.erase(bb);
		lp = lp->parent_;
	}
}

void Loop::add_exit_casecade(BasicBlock* from, BasicBlock* to)
{
	auto lp = this;
	while (lp != nullptr && !lp->blocks_.count(to))
	{
		lp->exits_.emplace(from, to);
		lp = lp->parent_;
	}
}

std::string Loop::print() const
{
	std::string ret;
	std::unordered_set<BasicBlock*> subs;
	for (auto l : sub_loops_) subs.emplace(l->header_);
	for (auto bb : blocks_)
	{
		if (bb->get_name().empty()) bb->get_module()->set_print_name();
		ret += bb->get_name();
		if (bb == header_)
		{
			if (preheader_ == nullptr)
				ret += "<H>";
			else
			{
				ret += "<H ";
				ret += preheader_->get_name();
				ret += ">";
			}
		}
		if (latches_.count(bb)) ret += "<L>";
		if (subs.count(bb)) ret += "<SH>";
		else if (detect_->loopOfBlock(bb) != this) ret += "<S>";
		if (exits_.count(bb))
		{
			ret += "<E ";
			ret += exits_.at(bb)->get_name();
			ret += ">";
		}
		ret += ", ";
	}
	if (!ret.empty())
	{
		ret.pop_back();
		ret.pop_back();
	}
	return ret;
}

Loop::Iterator Loop::getIterator() const
{
	// 复杂循环
	if (!exits_.count(header_)) return Iterator{};
	bool haveOtherOut = exits_.size() > 1;
	auto headBr = header_->get_instructions().back();
	auto headBrCond = dynamic_cast<Instruction*>(headBr->get_operand(0));
	auto condL = headBrCond->get_operand(0);
	auto condR = headBrCond->get_operand(1);
	bool lInLoop = valueInLoop(condL);
	bool rInLoop = valueInLoop(condR);
	// 复杂循环
	if (lInLoop && rInLoop) return Iterator{};
	if (!lInLoop && !rInLoop)
	{
		// out 由外部判断, 可以优化为 guard + while(true)
		Iterator it{true, haveOtherOut};
		it.cmp_ = headBrCond;
		it.br_ = headBr;
		it.start_ = condL;
		it.end_ = condR;
		return it;
	}
	if (rInLoop)
	{
		auto c2 = condL;
		condL = condR;
		condR = c2;
	}
	auto lphi = dynamic_cast<PhiInst*>(condL);
	if (lphi == nullptr) return Iterator{};
	auto pairs = lphi->get_phi_pairs();
	ASSERT(pairs.size() == 2);
	Value* inner;
	Value* outer;
	if (pairs[0].second == preheader_)
	{
		outer = pairs[0].first;
		inner = pairs[1].first;
	}
	else
	{
		outer = pairs[1].first;
		inner = pairs[0].first;
	}
	if (!valueInLoop(inner))
	{
		// inner 根据是否刚进入循环来自不同外部变量, 可以插入 guard
		auto ret = Iterator{false, haveOtherOut};
		ret.phiDefinedByOut_ = true;
		ret.start_ = outer;
		ret.iterator_ = lphi;
		ret.end_ = condR;
		ret.br_ = headBr;
		ret.cmp_ = headBrCond;
		return ret;
	}
	auto innerInst = dynamic_cast<Instruction*>(inner);
	if (innerInst->isBinary())
	{
		bool haveIt = false;
		Value* another = nullptr;
		for (auto op : innerInst->get_operands())
		{
			if (op == condL)
			{
				haveIt = true;
			}
			else another = op;
		}
		// 由内部运算决定, 复杂循环
		if (!haveIt) return Iterator{};
		if (valueInLoop(another)) return Iterator{};
		Iterator ret{false, haveOtherOut};
		ret.start_ = outer;
		ret.iterator_ = dynamic_cast<Instruction*>(condL);
		ret.end_ = condR;
		ret.cmp_ = headBrCond;
		ret.br_ = headBr;
		return ret;
	}
	return Iterator{};
}

/**
 * @brief 发现循环及其子循环
 * @param bb 循环的header块
 * @param latches 循环的回边终点(latch)集合
 * @param loop 当前正在处理的循环对象
 */
void LoopDetection::discover_loop_and_sub_loops(BasicBlock* bb, BBset& latches,
                                                Loop* loop, Dominators* dom)
{
	// DONE List:
	// 1. 初始化工作表，将所有latch块加入
	// 2. 实现主循环逻辑
	// 3. 处理未分配给任何循环的节点
	// 4. 处理已属于其他循环的节点
	// 5. 建立正确的循环嵌套关系

	BBvec work_list = {latches.begin(), latches.end()}; // 初始化工作表

	while (!work_list.empty())
	{
		// 当工作表非空时继续处理
		auto subbb = work_list.back();
		work_list.pop_back();
		// DONE-1: 处理未分配给任何循环的节点
		if (bb_to_loop_.find(subbb) == bb_to_loop_.end())
		{
			loop->add_block(subbb);
			bb_to_loop_[subbb] = loop;
			for (auto pre : subbb->get_pre_basic_blocks())
			{
				if (dom->is_dominate(bb, pre))
				{
					work_list.push_back(pre);
				}
			}
		}
		// DONE-2: 处理已属于其他循环的节点
		else if (bb_to_loop_[subbb] != loop)
		{
			auto subloop = bb_to_loop_[subbb];
			auto parent = subloop->get_parent();
			while (parent != nullptr)
			{
				subloop = parent;
				parent = parent->get_parent();
			}
			if (subloop != loop)
			{
				loop->add_sub_loop(subloop);
				subloop->set_parent(loop);
				for (auto pb : subloop->get_blocks())
				{
					loop->add_block(pb);
				}
				for (auto pre : subloop->get_header()->get_pre_basic_blocks())
				{
					if (dom->is_dominate(bb, pre))
					{
						work_list.push_back(pre);
					}
				}
			}
		}
	}
}

/**
 * @brief 打印循环检测的结果
 *
 * 为每个检测到的循环打印：
 * 1. 循环的header块
 * 2. 循环包含的所有基本块
 * 3. 循环的所有子循环
 */
void LoopDetection::print() const
{
	f_->get_parent()->set_print_name();
	std::cout << "Loop Detection Result:" << '\n';
	for (auto& loop : loops_)
	{
		std::cout << "\nLoop header: " << loop->get_header()->get_name()
			<< '\n';
		std::cout << "Loop blocks: ";
		for (auto& bb : loop->get_blocks())
		{
			std::cout << bb->get_name() << " ";
		}
		std::cout << '\n';
		std::cout << "Sub loops: ";
		for (auto& sub_loop : loop->get_sub_loops())
		{
			std::cout << sub_loop->get_header()->get_name() << " ";
		}
		std::cout << '\n';
	}
}

Loop* LoopDetection::loopOfBlock(BasicBlock* b) const
{
	if (bb_to_loop_.count(b)) return bb_to_loop_.at(b);
	return nullptr;
}

int LoopDetection::costOfLatch(Loop* loop, BasicBlock* bb, const Dominators* idoms)
{
	bool out = false;
	for (auto i : bb->get_succ_basic_blocks())
	{
		if (loop->get_blocks().count(i))
		{
			out = true;
			break;
		}
	}
	auto hd = loop->get_header();
	std::unordered_set<BasicBlock*> blocks;
	blocks.emplace(bb);
	while (bb != hd)
	{
		auto subLoop = bb_to_loop_[bb];
		if (subLoop != nullptr && subLoop->get_header() == bb && subLoop->get_parent() == loop)
			blocks.insert(subLoop->get_blocks().begin(), subLoop->get_blocks().end());
		bb = idoms->get_idom(bb);
		blocks.emplace(bb);
		if (!out)
		{
			for (auto i : bb->get_succ_basic_blocks())
			{
				if (loop->get_blocks().count(i))
				{
					out = true;
					break;
				}
			}
		}
	}
	int cost = u2iNegThrow(blocks.size());
	if (out) cost += INT_MAX >> 1;
	return cost;
}

void LoopDetection::collectInnerLoopMessage(Loop* loop, BasicBlock* bb, BasicBlock* preHeader, Loop* innerLoop,
                                            Dominators* idoms)
{
	innerLoop->set_preheader(preHeader);
	auto hd = loop->get_header();
	innerLoop->add_block(bb);
	innerLoop->add_latch(bb);
	auto pbb = bb;
	while (pbb != hd)
	{
		auto subLoop = bb_to_loop_[pbb];
		if (subLoop != nullptr && subLoop->get_header() == pbb && subLoop->get_parent() == loop)
		{
			innerLoop->get_blocks().insert(subLoop->get_blocks().begin(), subLoop->get_blocks().end());
			innerLoop->add_sub_loop(subLoop);
			subLoop->set_parent(innerLoop);
		}
		pbb = idoms->get_idom(pbb);
		innerLoop->add_block(pbb);
	}
	std::vector<Loop*> newSubs;
	for (auto sub : loop->get_sub_loops())
	{
		if (sub->get_parent() == loop)
		{
			newSubs.emplace_back(sub);
		}
	}
	loop->get_sub_loops() = newSubs;
	loop->add_sub_loop(innerLoop);
	loop->add_block(preHeader);
	innerLoop->set_parent(loop);
	loop->set_header(preHeader);
	bb_to_loop_[preHeader] = loop;
	loops_.emplace_back(innerLoop);
	auto hid = idoms->get_idom(hd);
	idoms->set_idom(hd, preHeader);
	idoms->set_idom(preHeader, hid);
	loop->remove_latch(bb);
	for (auto lpb : innerLoop->get_blocks())
	{
		if (bb_to_loop_[lpb] == loop) bb_to_loop_[lpb] = innerLoop;
	}
}

void LoopDetection::addNewExitTo(Loop* loop, BasicBlock* bb, BasicBlock* out, BasicBlock* preOut)
{
	loop->add_exit(bb, out);
	auto lp = loop->get_parent();
	while (lp != nullptr)
	{
		if (lp->get_blocks().count(preOut))
			lp->add_block(out);
		lp = lp->get_parent();
	}
}

void LoopDetection::validate()
{
	for (auto loop : loops_)
	{
		for (auto bb : loop->get_blocks())
		{
			auto bb2l = bb_to_loop_[bb];
			if (bb2l == nullptr) throw std::runtime_error("bb2l destory");
			auto g = bb2l;
			while (g != loop)
			{
				g = g->get_parent();
				if (g == nullptr) throw std::runtime_error("bb2l destory");
			}
		}
	}
	for (auto [i,j] : bb_to_loop_)
	{
		if (!j->get_blocks().count(i)) throw std::runtime_error("bb2l destory");
	}
}

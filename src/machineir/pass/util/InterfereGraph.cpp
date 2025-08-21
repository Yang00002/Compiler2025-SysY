#include "InterfereGraph.hpp"

#include <cassert>
#include <unordered_set>
#include <cfloat>
#include <vector>
#include <algorithm>

#include "Config.hpp"
#include "RegisterAllocate.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineInstruction.hpp"
#include "MachineFunction.hpp"
#include "MachineOperand.hpp"

#define DEBUG 0
#include <algorithm>
#include <iostream>

#include "MachineDominator.hpp"
#include "Util.hpp"

InterfereGraph::InterfereGraph(RegisterAllocate* parent) : parent_(parent), worklistMoves_(), activeMoves_(),
                                                           coalescedMoves_(),
                                                           constrainedMoves_(),
                                                           frozenMoves_(),
                                                           initial_(),
                                                           spillWorklist_(),
                                                           freezeWorklist_(),
                                                           simplifyWorklist_(),
                                                           coalescedNodes_(),
                                                           selectStack_(),
                                                           spilledNode_()
{
}

void InterfereGraphNode::add(InterfereGraphNode* target)
{
	if (graphColoringWeakNodeCheck && target->parent_ == this)
	{
		LOG(color::red("duplicate add of ") + target->reg_->print());
		return;
	}
	ASSERT(target->parent_ == nullptr); // 不负责删除
	target->parent_ = this;
	target->next_ = this;
	target->pre_ = pre_;
	target->pre_->next_ = target;
	target->next_->pre_ = target;
}

void InterfereGraphNode::remove(InterfereGraphNode* target) const
{
	if (graphColoringWeakNodeCheck)
	{
		if (target->parent_ == this || (target->parent_ == nullptr && target->reg_->isPhysicalRegister()))
		{
			if (target->parent_ == nullptr)
			{
				LOG(color::red("empty remove of ") + target->reg_->print());
				return;
			}
			target->parent_ = nullptr;
			target->pre_->next_ = target->next_;
			target->next_->pre_ = target->pre_;
			return;
		}
	}
	ASSERT(target->parent_ == this);
	target->parent_ = nullptr;
	target->pre_->next_ = target->next_;
	target->next_->pre_ = target->pre_;
}

void InterfereGraphNode::clear()
{
	pre_ = this;
	next_ = this;
}

bool InterfereGraphNode::contain(const InterfereGraphNode* target) const
{
	return target->parent_ == this;
}

bool InterfereGraphNode::preColored() const
{
	return reg_->isPhysicalRegister();
}

bool InterfereGraphNode::empty() const
{
	return next_ == this;
}

InterfereGraphNode* InterfereGraphNode::top() const
{
	ASSERT(next_ != this);
	return pre_;
}

InterfereGraphNode* InterfereGraphNode::alias()
{
	if (alias_ == nullptr) return this;
	auto a = alias_->alias();
	alias_ = a;
	return a;
}

void MoveInstNode::add(MoveInstNode* target)
{
	ASSERT(target->parent_ == nullptr); // 不负责删除
	target->parent_ = this;
	target->next_ = this;
	target->pre_ = pre_;
	target->pre_->next_ = target;
	target->next_->pre_ = target;
}

void MoveInstNode::remove(MoveInstNode* target) const
{
	ASSERT(target->parent_ == this);
	target->parent_ = nullptr;
	target->pre_->next_ = target->next_;
	target->next_->pre_ = target->pre_;
}

void MoveInstNode::clear()
{
	pre_ = this;
	next_ = this;
}

bool MoveInstNode::contain(const MoveInstNode* target) const
{
	return target->parent_ == this;
}

bool MoveInstNode::empty() const
{
	return next_ == this;
}

MoveInstNode* MoveInstNode::top() const
{
	ASSERT(next_ != this);
	return pre_;
}

InterfereGraphNode* InterfereGraph::getOrCreateRegNode(RegisterLike* reg)
{
	auto g = regNodes_[reg];
	if (g == nullptr)
	{
		g = new InterfereGraphNode{nullptr, nullptr, nullptr, reg, {}, {}, 0, 0.0f, nullptr, nullptr};
		regNodes_[reg] = g;
	}
	return g;
}

MoveInstNode* InterfereGraph::getOrCreateMoveNode(MCopy* reg)
{
	auto g = moveNodes_[reg];
	if (g == nullptr)
	{
		g = new MoveInstNode{
			nullptr, nullptr, nullptr, getOrCreateRegNode(dynamic_cast<RegisterLike*>(reg->def(0))),
			getOrCreateRegNode(dynamic_cast<RegisterLike*>(reg->use(0)))
		};
		moveNodes_[reg] = g;
	}
	return g;
}

void InterfereGraph::addEdge(int i, int j)
{
	if (i != j && !adjSet_.test(i, j))
	{
		adjSet_.set(i, j);
		adjSet_.set(j, i);
		auto ri = parent_->live_message()->getReg(i);
		auto rj = parent_->live_message()->getReg(j);
		auto vri = getOrCreateRegNode(ri);
		auto vrj = getOrCreateRegNode(rj);
		if (ri->isVirtualRegister())
		{
			vri->adjList_.emplace_back(vrj);
			vri->degree_++;
		}
		if (rj->isVirtualRegister())
		{
			vrj->adjList_.emplace_back(vri);
			vrj->degree_++;
		}
	}
}

void InterfereGraph::combineAddEdge(int i, int noAdd)
{
	if (i != noAdd && !adjSet_.test(i, noAdd))
	{
		adjSet_.set(i, noAdd);
		adjSet_.set(noAdd, i);
		auto ri = parent_->live_message()->getReg(i);
		auto rj = parent_->live_message()->getReg(noAdd);
		auto vri = getOrCreateRegNode(ri);
		auto vrj = getOrCreateRegNode(rj);
		if (ri->isVirtualRegister())
		{
			vri->adjList_.emplace_back(vrj);
			vri->degree_++;
		}
		if (rj->isVirtualRegister())
		{
			vrj->adjList_.emplace_back(vri);
		}
	}
}

bool InterfereGraph::moveRelated(const InterfereGraphNode* node) const
{
	for (auto i : node->moveList_) // NOLINT(readability-use-anyofallof)
		if (worklistMoves_.contain(i) || activeMoves_.contain(i)) return true;
	return false;
}

void InterfereGraph::decrementDegree(InterfereGraphNode* node)
{
	auto d = node->degree_;
	node->degree_--;
	// 从高度数到低度数
	if (d == K_)
	{
		enableMoves(node);
		// 与其相邻的节点的传送指令都有可能会可合并, 需要重新计算
		for (auto m : node->adjList_)
			if (inGraph(m)) enableMoves(m);
		// 从高度数节点表移除节点
		spillWorklist_.remove(node);
		if (moveRelated(node))
		{
			freezeWorklist_.add(node);
		}
		else
		{
			simplifyWorklist_.add(node);
		}
	}
}

void InterfereGraph::addWorkList(InterfereGraphNode* node)
{
	if (node->degree_ < K_ && !node->preColored() && !moveRelated(node))
	{
		freezeWorklist_.remove(node);
		simplifyWorklist_.add(node);
	}
}


bool InterfereGraph::GeorgeTest(const InterfereGraphNode* virtualReg, const InterfereGraphNode* physicReg) const
{
	for (auto t : virtualReg->adjList_) // NOLINT(readability-use-anyofallof)
	{
		if (inGraph(t) && t->degree_ >= K_ && !t->preColored() && !adj(t, physicReg))
			return false;
	}
	return true;
}

bool InterfereGraph::BriggsTest(const InterfereGraphNode* l,
                                const InterfereGraphNode* r) const
{
	DynamicBitset bits{parent_->live_message()->regCount()};
	int k = 0;
	for (auto n : l->adjList_)
	{
		if (inGraph(n))
		{
			if (auto id = regIdOf(n); !bits.test(id))
			{
				bits.set(id);
				if (n->degree_ >= K_ || n->preColored()) k++;
			}
		}
	}
	for (auto n : r->adjList_)
	{
		if (inGraph(n))
		{
			if (auto id = regIdOf(n); !bits.test(id))
			{
				bits.set(id);
				if (n->degree_ >= K_ || n->preColored()) k++;
			}
		}
	}
	return k < K_;
}

bool InterfereGraph::isCareCopy(MInstruction* inst) const
{
	auto cp = dynamic_cast<MCopy*>(inst);
	if (cp == nullptr) return false;
	auto l = cp->def(0);
	auto r = cp->use(0);
	auto msg = parent_->live_message();
	if (!msg->care(l) || !msg->care(r)) return false;
	auto regl = dynamic_cast<VirtualRegister*>(l);
	auto regr = dynamic_cast<VirtualRegister*>(r);
	return regl || regr;
}

void InterfereGraph::combine(InterfereGraphNode* u, InterfereGraphNode* v)
{
	// 不可能是传送无关节点
	if (freezeWorklist_.contain(v))
	{
		freezeWorklist_.remove(v);
	}
	else
	{
		spillWorklist_.remove(v);
	}
	coalescedNodes_.add(v);
	ASSERT(v == v->alias());
	v->alias_ = u;
	// 合并传送指令关联
	std::unordered_set<MoveInstNode*> g;
	for (auto i : u->moveList_) g.emplace(i);
	for (auto i : v->moveList_) if (g.count(i) == 0) u->moveList_.emplace_back(i);
	enableMoves(v);
	// 合并冲突关系
	for (auto t : v->adjList_)
	{
		if (inGraph(t))
		{
			combineAddEdge(regIdOf(u), regIdOf(t));
			// addEdge + decrementDegree(t);
		}
	}
	if (u->degree_ >= K_ && freezeWorklist_.contain(u))
	{
		freezeWorklist_.remove(u);
		spillWorklist_.add(u);
	}
}

int InterfereGraph::regIdOf(const InterfereGraphNode* n) const
{
	return parent_->live_message()->regIdOf(n->reg_);
}

void InterfereGraph::enableMoves(const InterfereGraphNode* u)
{
	for (auto n : u->moveList_)
	{
		if (activeMoves_.contain(n))
		{
			activeMoves_.remove(n);
			worklistMoves_.add(n);
		}
	}
}

void InterfereGraph::freezeMove(InterfereGraphNode* u)
{
	for (auto m : u->moveList_)
	{
		if (inGraph(m))
		{
			// 传送指令的另一端的节点
			InterfereGraphNode* v;
			auto y = m->fromY_;
			if (y->alias() == u->alias())
				v = m->toX_->alias();
			else v = y->alias();
			activeMoves_.remove(m);
			frozenMoves_.add(m);
			if (v->degree_ < K_ && !moveRelated(v))
			{
				freezeWorklist_.remove(v);
				simplifyWorklist_.add(v);
			}
		}
	}
}

bool InterfereGraph::inGraph(const MoveInstNode* n) const
{
	return activeMoves_.contain(n) || worklistMoves_.contain(n);
}

bool InterfereGraph::inGraph(const InterfereGraphNode* n) const
{
	return !coalescedNodes_.contain(n) && !selectStack_.contain(n);
}

bool InterfereGraph::adj(const InterfereGraphNode* l, const InterfereGraphNode* r) const
{
	auto msg = parent_->live_message();
	return adjSet_.test(msg->regIdOf(l->reg_), msg->regIdOf(r->reg_));
}

InterfereGraph::~InterfereGraph()
{
	for (auto& [i, j] : regNodes_) delete j;
	for (auto& [i, j] : moveNodes_) delete j;
}

bool InterfereGraph::shouldRepeat() const
{
	return !simplifyWorklist_.empty() || !worklistMoves_.empty()
	       || !freezeWorklist_.empty() || !spillWorklist_.empty();
}

bool InterfereGraph::simplify()
{
	if (simplifyWorklist_.empty()) return false;
	auto n = simplifyWorklist_.top();
	ASSERT(!moveRelated(n));
	simplifyWorklist_.remove(n);
	// 简化, 入栈
	selectStack_.add(n);
	// 减少相邻节点的边数量
	for (auto adj : n->adjList_)
		if (inGraph(adj) && !adj->preColored()) decrementDegree(adj);
	//  if (inGraph(adj)) decrementDegree(adj);
	return true;
}

bool InterfereGraph::coalesce()
{
	if (worklistMoves_.empty()) return false;
	auto m = worklistMoves_.top();
	// 如果源属于物理寄存器, 则是源, 否则是目标
	auto x = m->toX_->alias();
	// 如果源属于物理寄存器, 则是目标, 否则是源
	auto y = m->fromY_->alias();
	if (y->preColored())
	{
		auto y2 = y;
		y = x;
		x = y2;
	}
	worklistMoves_.remove(m);
	if (x == y)
	{
		// 传送指令有相同操作数, 直接合并
		coalescedMoves_.add(m);
		// 尝试转移到简化
		addWorkList(x);
	}
	else
	{
		// 二者都是物理寄存器或者二者冲突
		if (y->preColored() || adj(x, y))
		{
			// 冲突, 无法合并
			constrainedMoves_.add(m);
			addWorkList(x);
			addWorkList(y);
		}
		else if ((x->preColored() && GeorgeTest(x, y)) || (
			         !x->preColored() && BriggsTest(x, y)))
		{
			// 进行合并
			coalescedMoves_.add(m);
			combine(x, y);
			addWorkList(x);
		}
		else
		{
			// 不满足合并要求
			activeMoves_.add(m);
		}
	}
	return true;
}

bool InterfereGraph::freeze()
{
	if (freezeWorklist_.empty()) return false;
	auto u = freezeWorklist_.top();
	freezeWorklist_.remove(u);
	simplifyWorklist_.add(u);
	freezeMove(u);
	return true;
}

bool InterfereGraph::selectSpill()
{
	if (spillWorklist_.empty()) return false;
	float mw = FLT_MAX;
	InterfereGraphNode* m = nullptr;
	for (auto node = spillWorklist_.next_; node != &spillWorklist_; node = node->next_)
	{
		float f;
		auto vreg = dynamic_cast<VirtualRegister*>(node->reg_);
		ASSERT(!node->preColored());
		// 不能溢出无冲突/因为 spill 分配/ 128 位的虚拟寄存器
		if (node->degree_ == 0 || vreg->spilled || vreg->size() == 128)
			f = FLT_MAX;
		else
		{
			f = node->weight_ / static_cast<float>(node->degree_);
			if (vreg->replacePrefer_ != nullptr) f *= vreg->spillCost_;
		}
		if (f < mw)
		{
			mw = f;
			m = node;
		}
	}
	// 不知道为什么有时候要溢出 spill
	if (m == nullptr)
	{
		for (auto node = spillWorklist_.next_; node != &spillWorklist_; node = node->next_)
		{
			float f;
			auto vreg = dynamic_cast<VirtualRegister*>(node->reg_);
			ASSERT(!node->preColored());
			if (node->degree_ == 0 || vreg->size() == 128)
				f = FLT_MAX;
			else
			{
				f = node->weight_ / static_cast<float>(node->degree_);
				if (vreg->replacePrefer_ != nullptr) f *= vreg->spillCost_;
			}
			if (f < mw)
			{
				mw = f;
				m = node;
			}
		}
	}
	ASSERT(m != nullptr);
	spillWorklist_.remove(m);
	simplifyWorklist_.add(m);
	freezeMove(m);
	POP;
	return true;
}

void InterfereGraph::assignColors()
{
	auto message = parent_->live_message();
	DynamicBitset mask = message->physicalRegMask();
	for (auto i : mask)
	{
		auto r = message->getReg(i);
		auto g = regNodes_.find(r);
		if (g != regNodes_.end())
		{
			g->second->color_ = dynamic_cast<Register*>(r);
		}
	}
	while (!selectStack_.empty())
	{
		auto n = selectStack_.pre_;
		selectStack_.remove(n);
		DynamicBitset oks = mask;
		for (auto w : n->adjList_)
		{
			auto c = w->alias()->color_;
			if (c != nullptr) oks.reset(message->regIdOf(c));
		}
		if (oks.allZeros())
		{
			spilledNode_.add(n);
		}
		else
			n->color_ = dynamic_cast<Register*>(message->getReg(*oks.begin()));
	}
	for (auto it = coalescedNodes_.next_; it != &coalescedNodes_; it = it->next_)
	{
		it->color_ = it->alias()->color_;
	}
}

bool InterfereGraph::needRewrite() const
{
	return !spilledNode_.empty();
}

void InterfereGraph::rewriteProgram() const
{
	LOG(color::cyan("RewriteProgram"));
	PUSH;
	auto dominators = parent_->dominators();
	auto func = parent_->currentFunc();
	std::list<VirtualRegister*> stillNeedWorks;
	bool needSpill = true;
	int count = 0;
	for (auto it = spilledNode_.next_; it != &spilledNode_; it = it->next_) count++;
	for (auto it = spilledNode_.next_; it != &spilledNode_; it = it->next_)
	{
		auto reg = dynamic_cast<VirtualRegister*>(it->reg_);
		if (useSinkForVirtualRegister && count >= useSinkGate)
		{
			if (reg->sinked)
			{
				stillNeedWorks.emplace_back(reg);
				continue;
			}
			auto uses = func->useList()[reg];
			MInstruction* def = nullptr;
			for (auto i : uses)
			{
				if (!i->def().empty() && reg == i->def(0))
				{
					def = i;
					break;
				}
			}
			for (auto i : def->use())
			{
				if (dynamic_cast<Register*>(def->operand(i)))
				{
					reg->sinked = true;
					stillNeedWorks.emplace_back(reg);
					break;
				}
			}
			if (reg->sinked) continue;
			uses.erase(def);
			auto parent = dominators->latestCommonParent(uses);
			if (!dominators->is_dominate(def->block(), parent))
			{
				reg->sinked = true;
				stillNeedWorks.emplace_back(reg);
				continue;
			}
			int idx = 0;
			auto& pinsts = parent->instructions();
			for (auto i : pinsts)
			{
				if (uses.count(i)) break;
				idx++;
			}
			int preIdx = 0;
			auto& insts = def->block()->instructions();
			for (auto i : insts)
			{
				if (i == def) break;
				preIdx++;
			}
			if (parent == def->block())
			{
				if (idx == u2iNegThrow(insts.size()))
				{
					auto bcc = dynamic_cast<MB*>(insts.back());
					if (bcc != nullptr && bcc->isCondBranch())
					{
						idx = u2iNegThrow(insts.size()) - 2;
					}
					else
						idx = u2iNegThrow(insts.size()) - 1;
				}
				LOG(color::green("sink ") + def->print() + color::green(" to before ") + insts[idx]->print());
				if (preIdx >= idx - 1)
				{
					stillNeedWorks.emplace_back(reg);
					reg->sinked = true;
					continue;
				}
				insts.erase(insts.begin() + preIdx);
				idx--;
				insts.emplace(insts.begin() + idx, def);
				reg->sinked = true;
				needSpill = false;
				continue;
			}
			if (idx == u2iNegThrow(pinsts.size()))
			{
				auto bcc = dynamic_cast<MB*>(pinsts.back());
				if (bcc != nullptr && bcc->isCondBranch())
				{
					idx = u2iNegThrow(pinsts.size()) - 2;
				}
				else
					idx = u2iNegThrow(pinsts.size()) - 1;
			}
			LOG(color::green("sink ") + def->print() + color::green(" to before ") + pinsts[idx]->print());
			insts.erase(insts.begin() + preIdx);
			parent->instructions().emplace(parent->instructions().begin() + idx, def);
			def->block_ = parent;
			reg->sinked = true;
			needSpill = false;
		}
		else
		{
			LOG(color::green("spill ") + reg->print());
			func->spill(reg, parent_->live_message());
		}
	}
	if (useSinkForVirtualRegister && count >= useSinkGate && needSpill)
	{
		for (auto i : stillNeedWorks)
		{
			LOG(color::green("spill ") + i->print());
			func->spill(i, parent_->live_message());
		}
	}
	POP;
}

void InterfereGraph::applyChanges()
{
	auto live = parent_->live_message();
	auto func = parent_->currentFunc();
	for (auto i : func->blocks())
	{
		auto& instructions = i->instructions();
		int size = u2iNegThrow(i->instructions().size());
		for (int idx = 0; idx < size; idx++)
		{
			auto inst = instructions[idx];
			for (auto& op : inst->operands_)
			{
				auto reg = dynamic_cast<VirtualRegister*>(op);
				if (reg != nullptr && live->care(reg))
					inst->replace(op, regNodes_[reg]->color_, func);
			}
			if (auto cp = dynamic_cast<MCopy*>(inst); cp != nullptr && cp->operands_[0] == cp->operands_[1])
			{
				instructions.erase(instructions.begin() + idx);
				func->removeUse(cp->operands_[0], inst);
				delete inst;
				size--;
				idx--;
			}
		}
	}
}


void InterfereGraph::build()
{
	auto& message = *parent_->live_message();
	for (auto bb : parent_->currentFunc()->blocks())
	{
		// 当前活跃(需要保存)的变量
		DynamicBitset live = message.live_out()[bb->id()];
		auto& insts = bb->instructions();
		// 逆序扫描
		for (auto iit = insts.rbegin(); iit != insts.rend(); ++iit)
		{
			auto inst = *iit;
			// 计算指令重要性
			for (auto i : inst->use_)
			{
				if (message.careVirtual(inst->operand(i)))
				{
					getOrCreateRegNode(dynamic_cast<RegisterLike*>(inst->operand(i)))->weight_ += bb->weight_;
				}
			}
			for (auto i : inst->def())
			{
				if (message.care(inst->operand(i)))
				{
					getOrCreateRegNode(dynamic_cast<RegisterLike*>(inst->operand(i)))->weight_ += bb->weight_;
				}
			}
			// 指令所有用到的寄存器
			auto use = message.translate(inst->operands_, inst->use_) | message.translate(inst->imp_use());
			// 指令所有定义的寄存器
			auto def = message.translate(inst->operands_, inst->def_) | message.translate(inst->imp_def());
			if (isCareCopy(inst))
			{
				auto cp = dynamic_cast<MCopy*>(inst);
				auto l = dynamic_cast<RegisterLike*>(inst->def(0));
				auto r = dynamic_cast<RegisterLike*>(inst->use(0));
				auto cpn = getOrCreateMoveNode(cp);
				// 传送指令不会制造 def/use 冲突(因为值 def = use)
				live.reset(message.regIdOf(r));
				// 构建传送指令
				getOrCreateRegNode(l)->moveList_.emplace_back(cpn);
				if (r != l) getOrCreateRegNode(r)->moveList_.emplace_back(cpn);
				worklistMoves_.add(cpn);
			}
			for (auto i : def)
			{
				// 指令后活跃的节点不能被定义所破坏
				for (auto j : live)
					addEdge((j), (i));
				// 定义的节点必须同时存在, 不能分配到同一寄存器
				for (auto j : def)
					addEdge((j), (i));
			}
			// 定值在指令前不活跃
			live -= def;
			// 使用在指令前活跃
			live |= use;
		}
	}
}

void InterfereGraph::flush()
{
	LOG(color::yellow("flush"));
	for (auto& [i, j] : regNodes_) delete j;
	for (auto& [i, j] : moveNodes_) delete j;
	regNodes_.clear();
	moveNodes_.clear();
	worklistMoves_.clear();
	activeMoves_.clear();
	coalescedMoves_.clear();
	constrainedMoves_.clear();
	frozenMoves_.clear();
	initial_.clear();
	spillWorklist_.clear();
	freezeWorklist_.clear();
	simplifyWorklist_.clear();
	coalescedNodes_.clear();
	selectStack_.clear();
	spilledNode_.clear();
	int rc = parent_->live_message()->regCount();
	K_ = 0;
	// 初始化
	for (int i = 0; i < rc; i++)
	{
		auto r = parent_->live_message()->getReg(i);
		if (r->isVirtualRegister())
		{
			initial_.add(getOrCreateRegNode(r));
		}
		else K_++;
	}
	adjSet_ = AdjSet{rc};
}

void InterfereGraph::makeWorklist()
{
	for (auto it = initial_.next_, itp = it->next_; it != &initial_; it = itp, itp = itp->next_)
	{
		initial_.remove(it);
		if (it->degree_ >= K_)
		{
			spillWorklist_.add(it);
		}
		else if (!it->moveList_.empty()) // 传送指令均未分类
		{
			freezeWorklist_.add(it);
		}
		else
		{
			simplifyWorklist_.add(it);
		}
	}
}


void calculateFrameInterfereGraph(const RegisterAllocate* parent, const FrameLiveMessage& flm)
{
	auto func = parent->currentFunc();
	int oc = u2iNegThrow(func->stackFrames().size());
	int frameCount_ = 0;
	int* denseIds_ = new int[oc]{};
	for (auto f : func->stackFrames())
	{
		if (f->spilledFrame_)
		{
			frameCount_++;
			denseIds_[f->index()] = frameCount_;
		}
	}
	if (frameCount_ < 1)
	{
		delete[] denseIds_;
		return;
	}
	int* frameIds1_ = new int[frameCount_ + 1];
	for (int i = 0; i < oc; i++)
	{
		if (denseIds_[i] != 0)
			frameIds1_[denseIds_[i]] = i;
	}
	int* colors1_ = new int[frameCount_ + 1]{};
	int* edgesCount1_ = new int[frameCount_ + 1]{};
	AdjSet adj{frameCount_ + 1};

	for (auto bb : func->blocks())
	{
		DynamicBitset live = flm.live_out()[bb->id()];
		auto& insts = bb->instructions();
		for (auto iit = insts.rbegin(); iit != insts.rend(); ++iit)
		{
			auto inst = *iit;
			DynamicBitset def{oc};
			if (auto ldr = dynamic_cast<MSTR*>(inst); ldr != nullptr)
			{
				auto frame = dynamic_cast<FrameIndex*>(ldr->operands()[1]);
				if (frame != nullptr && frame->spilledFrame_)
				{
					live.reset((frame->index()));
					for (auto i : live)
					{
						auto fi = denseIds_[i];
						auto fj = denseIds_[frame->index()];
						if (!adj.test(fi, fj))
						{
							edgesCount1_[fi]++;
							edgesCount1_[fj]++;
							adj.set(fi, fj);
							adj.set(fj, fi);
						}
					}
					continue;
				}
			}
			if (auto ldr = dynamic_cast<MLDR*>(inst); ldr != nullptr)
			{
				auto frame = dynamic_cast<FrameIndex*>(ldr->operands()[1]);
				if (frame != nullptr && frame->spilledFrame_)
					live.set(frame->index());
			}
		}
	}

	int* indexes1_ = new int[frameCount_ + 1];
	for (int i = 0; i <= frameCount_; i++) indexes1_[i] = i;

	std::sort(indexes1_ + 1, indexes1_ + frameCount_ + 1, [edgesCount1_](int a, int b)-> bool
	{
		return edgesCount1_[a] > edgesCount1_[b];
	});

	int color = 1;
	int beg = 1;
	while (beg <= frameCount_)
	{
		int pb = beg;
		for (int i = beg; i <= frameCount_; i++)
		{
			if (colors1_[indexes1_[i]] != 0)
				beg++;
			else break;
		}
		for (int i = beg; i <= frameCount_; i++)
		{
			if (colors1_[indexes1_[i]] != 0) continue;
			bool ok = true;
			for (int j = pb; j < i; j++)
			{
				if (adj.test(indexes1_[i], indexes1_[j]) && colors1_[indexes1_[j]] == color)
				{
					ok = false;
					break;
				}
			}
			if (ok)
			{
				colors1_[indexes1_[i]] = color;
			}
		}
		color++;
	}

	color -= 2;

	if (color < frameCount_)
	{
		auto& frames = func->stackFrames();

		std::unordered_map<FrameIndex*, FrameIndex*> replaceMap;

		int begin = frameIds1_[1];
		for (int i = 1; i <= frameCount_; i++)
		{
			auto l = frames[frameIds1_[i]];
			auto r = frames[begin + colors1_[i] - 1];
			if (l->size() > r->size()) r->set_size(l->size());
			replaceMap[l] = r;
		}

		func->replaceAllOperands(replaceMap);

		for (int i = begin + color; i < oc; i++)
		{
			delete frames[i];
		}
		frames.resize(begin + color);
	}

	delete[] denseIds_;
	delete[] frameIds1_;
	delete[] colors1_;
	delete[] edgesCount1_;
	delete[] indexes1_;
}

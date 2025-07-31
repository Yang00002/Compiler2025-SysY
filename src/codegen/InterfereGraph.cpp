#include "InterfereGraph.hpp"

#include <cassert>
#include <unordered_set>
#include <climits>
#include <cfloat>
#include <string>

#include "Config.hpp"
#include "GraphColoring.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineInstruction.hpp"
#include "MachineIR.hpp"
#include "MachineOperand.hpp"

#define DEBUG 0
#include <algorithm>

#include "Type.hpp"
#include "Util.hpp"

namespace
{
	InterfereGraphNode* makeInterfereGraphNode()
	{
		auto n = new InterfereGraphNode{};
		n->parent_ = nullptr;
		n->pre_ = n;
		n->next_ = n;
		return n;
	}

	MoveInstNode* makeMoveInstNode()
	{
		auto n = new MoveInstNode{};
		n->parent_ = nullptr;
		n->pre_ = n;
		n->next_ = n;
		return n;
	}
}

InterfereGraph::InterfereGraph(GraphColorSolver* parent) : parent_(parent), worklistMoves_(), activeMoves_(),
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
	assert(target->parent_ == nullptr); // 不负责删除
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
	assert(target->parent_ == this);
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
	assert(next_ != this);
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
	assert(target->parent_ == nullptr); // 不负责删除
	target->parent_ = this;
	target->next_ = this;
	target->pre_ = pre_;
	target->pre_->next_ = target;
	target->next_->pre_ = target;
}

void MoveInstNode::remove(MoveInstNode* target) const
{
	assert(target->parent_ == this);
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
	assert(next_ != this);
	return pre_;
}

InterfereGraphNode* InterfereGraph::getOrCreateRegNode(RegisterLike* reg)
{
	auto g = regNodes_[reg];
	if (g == nullptr)
	{
		g = new InterfereGraphNode{nullptr, nullptr, nullptr, reg};
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

void InterfereGraph::addEdge(unsigned i, unsigned j)
{
	if (i != j && !adjSet_.test(i, j))
	{
		adjSet_.set(i, j);
		adjSet_.set(j, i);
		auto ri = parent_->live_message()->getReg(static_cast<int>(i));
		auto rj = parent_->live_message()->getReg(static_cast<int>(j));
		LOG(color::green("addEdge ") + ri->print() + " " + rj->print());
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

void InterfereGraph::combineAddEdge(unsigned i, unsigned noAdd)
{
	if (i != noAdd && !adjSet_.test(i, noAdd))
	{
		adjSet_.set(i, noAdd);
		adjSet_.set(noAdd, i);
		auto ri = parent_->live_message()->getReg(static_cast<int>(i));
		auto rj = parent_->live_message()->getReg(static_cast<int>(noAdd));
		LOG(color::green("addEdge ") + ri->print() + " " + rj->print());
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
	for (auto i : node->moveList_)
		if (worklistMoves_.contain(i) || activeMoves_.contain(i)) return true;
	return false;
}

void InterfereGraph::decrementDegree(InterfereGraphNode* node)
{
	LOG(color::green("decrementDegree " ) + node->reg_->print());
	PUSH;
	auto d = node->degree_;
	node->degree_--;
	// 从高度数到低度数
	if (d == K_)
	{
		enableMoves(node);
		// 与其相邻的节点的传送指令都有可能会可合并, 需要重新计算
		for (auto m : node->adjList_)
			if (inGraph(m)) enableMoves(m);
		LOG(color::pink("spillWorklist_ - ") + node->reg_->print());
		// 从高度数节点表移除节点
		spillWorklist_.remove(node);
		if (moveRelated(node))
		{
			LOG(color::pink("freezeWorklist_ + ") + node->reg_->print());
			freezeWorklist_.add(node);
		}
		else
		{
			LOG(color::pink("simplifyWorklist_ + ") + node->reg_->print());
			simplifyWorklist_.add(node);
		}
	}
	POP;
}

void InterfereGraph::addWorkList(InterfereGraphNode* node)
{
	if (node->degree_ < K_ && !node->preColored() && !moveRelated(node))
	{
		LOG(color::green("addWorkList ") + node->reg_->print());
		PUSH;
		LOG(color::pink("freezeWorklist_ - ") + node->reg_->print());
		freezeWorklist_.remove(node);
		LOG(color::pink("simplifyWorklist_ + ") + node->reg_->print());
		simplifyWorklist_.add(node);
		POP;
	}
}


bool InterfereGraph::GeorgeTest(const InterfereGraphNode* virtualReg, const InterfereGraphNode* physicReg) const
{
	for (auto t : virtualReg->adjList_)
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
	LOG(color::green("combine ") + u->reg_->print() + " " + v->reg_->print());
	PUSH;
	// 不可能是传送无关节点
	if (freezeWorklist_.contain(v))
	{
		LOG(color::pink("freezeWorklist_ - ") + v->reg_->print());
		freezeWorklist_.remove(v);
	}
	else
	{
		LOG(color::pink("spillWorklist_ - ") + v->reg_->print());
		spillWorklist_.remove(v);
	}
	LOG(color::pink("coalescedNodes_ + ") + v->reg_->print());
	coalescedNodes_.add(v);
	assert(v == v->alias());
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
		LOG(color::pink("freezeWorklist_ - ") + u->reg_->print());
		freezeWorklist_.remove(u);
		LOG(color::pink("spillWorklist_ + ") + u->reg_->print());
		spillWorklist_.add(u);
	}
	POP;
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
	LOG(color::green("freezeMove ") + u->reg_->print());
	PUSH;
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
				LOG(color::pink("freezeWorklist_ - ") + v->reg_->print());
				freezeWorklist_.remove(v);
				LOG(color::pink("simplifyWorklist_ + ") + v->reg_->print());
				simplifyWorklist_.add(v);
			}
		}
	}
	POP;
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
	LOG(color::cyan("simplify"));
	PUSH;
	auto n = simplifyWorklist_.top();
	assert(!moveRelated(n));
	LOG(color::pink("simplifyWorklist_ - ") + n->reg_->print());
	simplifyWorklist_.remove(n);
	LOG(color::pink("selectStack_ + ") + n->reg_->print());
	// 简化, 入栈
	selectStack_.add(n);
	// 减少相邻节点的边数量
	for (auto adj : n->adjList_)
		if (inGraph(adj) && !adj->preColored()) decrementDegree(adj);
	//  if (inGraph(adj)) decrementDegree(adj);
	POP;
	return true;
}

bool InterfereGraph::coalesce()
{
	if (worklistMoves_.empty()) return false;
	LOG(color::cyan("coalesce"));
	PUSH;
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
	POP;
	return true;
}

bool InterfereGraph::freeze()
{
	if (freezeWorklist_.empty()) return false;
	LOG(color::cyan("freeze"));
	PUSH;
	auto u = freezeWorklist_.top();
	LOG(color::pink("freezeWorklist_ - ") + u->reg_->print());
	freezeWorklist_.remove(u);
	LOG(color::pink("simplifyWorklist_ + ") + u->reg_->print());
	simplifyWorklist_.add(u);
	freezeMove(u);
	POP;
	return true;
}

bool InterfereGraph::selectSpill()
{
	if (spillWorklist_.empty()) return false;
	LOG(color::cyan("selectSpill"));
	PUSH;
	float mw = FLT_MAX;
	InterfereGraphNode* m = nullptr;
	for (auto node = spillWorklist_.next_; node != &spillWorklist_; node = node->next_)
	{
		float f;
		auto vreg = dynamic_cast<VirtualRegister*>(node->reg_);
		assert(!node->preColored());
		// 不能溢出无冲突/因为 spill 分配/ 128 位的虚拟寄存器
		if (node->degree_ == 0 || vreg->spilled || vreg->size() == 128)
			f = FLT_MAX;
		else
		{
			f = node->weight_ / static_cast<float>(node->degree_);
			if (vreg->replacePrefer_ != nullptr)
			{
				if (dynamic_cast<GlobalAddress*>(vreg->replacePrefer_)) f *= globalRegisterSpillPriority;
				else
				{
					auto d = dynamic_cast<FrameIndex*>(vreg->replacePrefer_);
					assert(d != nullptr);
					if (d->isParameterFrame()) f *= fixFrameIndexParameterRegisterSpillPriority;
					else if (static_cast<int>(d->size() >> 3) >= bigAllocaVariableGate)
						f *= bigAllocaRegisterSpillPriority;
					else f *= smallAllocaRegisterSpillPriority;
				}
			}
		}
		if (f < mw)
		{
			mw = f;
			m = node;
		}
	}
	LOG(color::pink("spillWorklist_ - ") + m->reg_->print());
	spillWorklist_.remove(m);
	LOG(color::pink("simplifyWorklist_ + ") + m->reg_->print());
	simplifyWorklist_.add(m);
	freezeMove(m);
	POP;
	return true;
}

void InterfereGraph::assignColors()
{
	LOG(color::cyan("assignColors"));
	PUSH;
	auto message = parent_->live_message();
	DynamicBitset mask = message->physicalRegMask();
	for (auto i : mask)
	{
		auto r = message->getReg(static_cast<int>(i));
		auto g = regNodes_.find(r);
		if (g != regNodes_.end())
		{
			g->second->color_ = dynamic_cast<Register*>(r);
		}
	}
	while (!selectStack_.empty())
	{
		auto n = selectStack_.pre_;
		LOG(color::pink("selectStack_ - ") + n->reg_->print());
		selectStack_.remove(n);
		DynamicBitset oks = mask;
		for (auto w : n->adjList_)
		{
			auto c = w->alias()->color_;
			if (c != nullptr) oks.reset(message->regIdOf(c));
		}
		if (oks.allZeros())
		{
			LOG(color::pink("spilledNode_ + ") + n->reg_->print());
			spilledNode_.add(n);
		}
		else
			n->color_ = dynamic_cast<Register*>(message->getReg(static_cast<int>(*oks.begin())));
	}
	for (auto it = coalescedNodes_.next_; it != &coalescedNodes_; it = it->next_)
	{
		it->color_ = it->alias()->color_;
	}
	POP;
}

bool InterfereGraph::needRewrite() const
{
	return !spilledNode_.empty();
}

void InterfereGraph::rewriteProgram() const
{
	auto func = parent_->currentFunc();
	for (auto it = spilledNode_.next_; it != &spilledNode_; it = it->next_)
	{
		auto reg = dynamic_cast<VirtualRegister*>(it->reg_);
		func->spill(reg, parent_->live_message());
	}
}

void InterfereGraph::applyChanges()
{
	auto live = parent_->live_message();
	auto func = parent_->currentFunc();
	for (auto i : func->blocks())
	{
		auto& instructions = i->instructions();
		int size = static_cast<int>(i->instructions().size());
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
	LOG(color::cyan("build"));
	PUSH;
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
					addEdge(static_cast<unsigned>(j), static_cast<unsigned>(i));
				// 定义的节点必须同时存在, 不能分配到同一寄存器
				for (auto j : def)
					addEdge(static_cast<unsigned>(j), static_cast<unsigned>(i));
			}
			// 定值在指令前不活跃
			live -= def;
			// 使用在指令前活跃
			live |= use;
		}
	}
	POP;
}

void InterfereGraph::flush()
{
	LOG(color::yellow("flush"));
	PUSH;
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
			LOG(color::pink("initial_ + ") + r->print());
			initial_.add(getOrCreateRegNode(r));
		}
		else K_++;
	}
	adjSet_ = AdjSet{rc};
	POP;
}

void InterfereGraph::makeWorklist()
{
	LOG(color::cyan("makeWorklist"));
	PUSH;
	for (auto it = initial_.next_, itp = it->next_; it != &initial_; it = itp, itp = itp->next_)
	{
		LOG(color::pink("initial_ - ") + it->reg_->print());
		initial_.remove(it);
		if (it->degree_ >= K_)
		{
			LOG(color::pink("spillWorklist_ + ") + it->reg_->print());
			spillWorklist_.add(it);
		}
		else if (!it->moveList_.empty()) // 传送指令均未分类
		{
			LOG(color::pink("freezeWorklist_ + ") + it->reg_->print());
			freezeWorklist_.add(it);
		}
		else
		{
			LOG(color::pink("simplifyWorklist_ + ") + it->reg_->print());
			simplifyWorklist_.add(it);
		}
	}
	POP;
}


void calculateFrameInterfereGraph(const GraphColorSolver* parent, const FrameLiveMessage& flm)
{
	auto func = parent->currentFunc();
	int oc = static_cast<int>(func->stackFrames().size());
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
					live.reset(frame->index());
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

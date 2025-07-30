#include "InterfereGraph.hpp"

#include <cassert>
#include <unordered_set>

#include "Config.hpp"
#include "GraphColoring.hpp"
#include "MachineBasicBlock.hpp"
#include "MachineInstruction.hpp"
#include "MachineIR.hpp"
#include "MachineOperand.hpp"

#define DEBUG 0
#include <algorithm>

#include "Util.hpp"

namespace
{
	// 不负责删除
	void addNode(MoveInstNode* parent, MoveInstNode* target)
	{
		assert(target->parent_ == nullptr); // 不负责删除
		target->parent_ = parent;
		target->next_ = parent;
		target->pre_ = parent->pre_;
		target->pre_->next_ = target;
		target->next_->pre_ = target;
	}

	// 不负责删除
	void addNode(InterfereGraphNode* parent, InterfereGraphNode* target)
	{
		assert(target->parent_ == nullptr); // 不负责删除
		target->parent_ = parent;
		target->next_ = parent;
		target->pre_ = parent->pre_;
		target->pre_->next_ = target;
		target->next_->pre_ = target;
	}

	void takeNode(const InterfereGraphNode* parent, InterfereGraphNode* target)
	{
		assert(target->parent_ == parent || (target->parent_ == nullptr && target->reg_->isPhysicalRegister()));
		if (target->parent_ == nullptr)
		{
			LOG(color::red("empty remove of ") + target->reg_->print());
			return;
		}
		target->parent_ = nullptr;
		target->pre_->next_ = target->next_;
		target->next_->pre_ = target->pre_;
	}

	void takeNode(const MoveInstNode* parent, MoveInstNode* target)
	{
		assert(target->parent_ == parent);
		target->parent_ = nullptr;
		target->pre_->next_ = target->next_;
		target->next_->pre_ = target->pre_;
	}

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

	void resetNode(InterfereGraphNode* node)
	{
		node->pre_ = node;
		node->next_ = node;
	}

	void resetNode(MoveInstNode* node)
	{
		node->pre_ = node;
		node->next_ = node;
	}
}

InterfereGraph::InterfereGraph(GraphColorSolver* parent) : parent_(parent)
{
	worklistMoves_ = makeMoveInstNode();
	activeMoves_ = makeMoveInstNode();
	coalescedMoves_ = makeMoveInstNode();
	constrainedMoves_ = makeMoveInstNode();
	frozenMoves_ = makeMoveInstNode();
	initial_ = makeInterfereGraphNode();
	spillWorklist_ = makeInterfereGraphNode();
	freezeWorklist_ = makeInterfereGraphNode();
	simplifyWorklist_ = makeInterfereGraphNode();
	coalescedNodes_ = makeInterfereGraphNode();
	selectStack_ = makeInterfereGraphNode();
	spilledNode_ = makeInterfereGraphNode();
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
		g = new MoveInstNode{nullptr, nullptr, nullptr, reg};
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
		LOG(color::green("addEdge + ") + ri->print() + " " + rj->print());
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

bool InterfereGraph::moveRelated(const InterfereGraphNode* node) const
{
	for (auto i : node->moveList_)
		if (i->parent_ == worklistMoves_ || i->parent_ == activeMoves_) return true;
	return false;
}

void InterfereGraph::decrementDegree(InterfereGraphNode* node) const
{
	LOG(color::green("decrementDegree " ) + node->reg_->print());
	PUSH;
	auto d = node->degree_;
	node->degree_--;
	if (d == K_)
	{
		enableMoves(node);
		for (auto m : node->adjList_)
		{
			if (m->parent_ == selectStack_ || m->parent_ == coalescedNodes_) continue;
			assert(m != node);
			enableMoves(m);
		}
		LOG(color::pink("spillWorklist_ - ") + node->reg_->print());
		takeNode(spillWorklist_, node);
		if (moveRelated(node))
		{
			LOG(color::pink("freezeWorklist_ + ") + node->reg_->print());
			addNode(freezeWorklist_, node);
		}
		else
		{
			LOG(color::pink("simplifyWorklist_ + ") + node->reg_->print());
			addNode(simplifyWorklist_, node);
		}
	}
	POP;
}

InterfereGraphNode* InterfereGraph::getAlias(InterfereGraphNode* node)
{
	if (node->parent_ == coalescedNodes_)
	{
		auto ret = getAlias(node->alias_);
		node->alias_ = ret;
		return ret;
	}
	return node;
}

void InterfereGraph::addWorkList(InterfereGraphNode* node) const
{
	if (node->degree_ < K_ && !node->reg_->isPhysicalRegister() && !moveRelated(node))
	{
		LOG(color::green("addWorkList ") + node->reg_->print());
		PUSH;
		LOG(color::pink("freezeWorklist_ - ") + node->reg_->print());
		takeNode(freezeWorklist_, node);
		LOG(color::pink("simplifyWorklist_ + ") + node->reg_->print());
		addNode(simplifyWorklist_, node);
		POP;
	}
}

bool InterfereGraph::ok(const InterfereGraphNode* t, const InterfereGraphNode* u) const
{
	return t->degree_ < K_ || t->reg_->isPhysicalRegister() || adjSet_.test(
		       parent_->live_message()->regIdOf(t->reg_), parent_->live_message()->regIdOf(u->reg_)
	       );
}

bool InterfereGraph::allOk(const InterfereGraphNode* u, const InterfereGraphNode* v) const
{
	for (auto t : v->adjList_)
	{
		if (t->parent_ == selectStack_ || t->parent_ == coalescedNodes_) continue;
		if (!ok(t, u)) return false;
	}
	return true;
}

bool InterfereGraph::conservative(const std::vector<InterfereGraphNode*>& nodes,
                                  const std::vector<InterfereGraphNode*>& nodes2) const
{
	DynamicBitset bits{parent_->live_message()->regCount()};
	unsigned k = 0;
	for (auto n : nodes)
	{
		if (n->parent_ == selectStack_ || n->parent_ == coalescedNodes_) continue;
		auto id = parent_->live_message()->regIdOf(n->reg_);
		if (bits.test(id)) continue;
		bits.set(id);
		if (n->degree_ >= K_) k++;
	}
	for (auto n : nodes2)
	{
		if (n->parent_ == selectStack_ || n->parent_ == coalescedNodes_) continue;
		auto id = parent_->live_message()->regIdOf(n->reg_);
		if (bits.test(id)) continue;
		bits.set(id);
		if (n->degree_ >= K_) k++;
	}
	return k < K_;
}

void InterfereGraph::combine(InterfereGraphNode* u, InterfereGraphNode* v)
{
	LOG(color::green("combine ") + u->reg_->print() + " " + v->reg_->print());
	PUSH;
	if (v->parent_ == freezeWorklist_)
	{
		LOG(color::pink("freezeWorklist_ - ") + v->reg_->print());
		takeNode(freezeWorklist_, v);
	}
	else
	{
		LOG(color::pink("spillWorklist_ - ") + v->reg_->print());
		takeNode(spillWorklist_, v);
	}
	LOG(color::pink("coalescedNodes_ + ") + v->reg_->print());
	addNode(coalescedNodes_, v);
	v->alias_ = u;
	std::unordered_set<MoveInstNode*> g;
	for (auto i : u->moveList_) g.emplace(i);
	for (auto i : v->moveList_) if (g.count(i) == 0) u->moveList_.emplace_back(i);
	enableMoves(v);
	for (auto t : v->adjList_)
	{
		if (t->parent_ == selectStack_ || t->parent_ == coalescedNodes_) continue;
		addEdge(idOf(t), idOf(u));
		decrementDegree(t);
	}
	if (u->degree_ >= K_ && u->parent_ == freezeWorklist_)
	{
		LOG(color::pink("freezeWorklist_ - ") + u->reg_->print());
		takeNode(freezeWorklist_, u);
		LOG(color::pink("spillWorklist_ + ") + u->reg_->print());
		addNode(spillWorklist_, u);
	}
	POP;
}

void InterfereGraph::enableMoves(const InterfereGraphNode* u) const
{
	for (auto n : u->moveList_)
	{
		if (n->parent_ == activeMoves_)
		{
			takeNode(activeMoves_, n);
			addNode(worklistMoves_, n);
		}
	}
}

int InterfereGraph::idOf(const InterfereGraphNode* n) const
{
	return parent_->live_message()->regIdOf(n->reg_);
}

void InterfereGraph::freezeMove(InterfereGraphNode* u)
{
	LOG(color::green("freezeMove ") + u->reg_->print());
	PUSH;
	for (auto m : u->moveList_)
	{
		if (!nodeMoveInMoveList(m))continue;
		InterfereGraphNode* v;
		auto y = yOf(m);
		if (getAlias(y) == getAlias(u))
			v = getAlias(xOf(m));
		else v = getAlias(y);
		takeNode(activeMoves_, m);
		addNode(frozenMoves_, m);
		if (v->degree_ < K_ && emptyNodeMove(v))
		{
			LOG(color::pink("freezeWorklist_ - ") + v->reg_->print());
			takeNode(freezeWorklist_, v);
			LOG(color::pink("simplifyWorklist_ + ") + v->reg_->print());
			addNode(simplifyWorklist_, v);
		}
	}
	POP;
}

bool InterfereGraph::nodeMoveInMoveList(const MoveInstNode* n) const
{
	return n->parent_ == activeMoves_ || n->parent_ == worklistMoves_;
}

bool InterfereGraph::emptyNodeMove(const InterfereGraphNode* n) const
{
	for (auto i : n->moveList_) if (nodeMoveInMoveList(i)) return false;
	return true;
}

InterfereGraphNode* InterfereGraph::xOf(const MoveInstNode* m)
{
	auto inst = m->inst_;
	return getOrCreateRegNode(dynamic_cast<RegisterLike*>(inst->def(0)));
}

InterfereGraphNode* InterfereGraph::yOf(const MoveInstNode* m)
{
	auto inst = m->inst_;
	return getOrCreateRegNode(dynamic_cast<RegisterLike*>(inst->use(0)));
}

InterfereGraph::~InterfereGraph()
{
	for (auto& [i, j] : regNodes_) delete j;
	for (auto& [i, j] : moveNodes_) delete j;
	delete worklistMoves_;
	delete activeMoves_;
	delete coalescedMoves_;
	delete constrainedMoves_;
	delete frozenMoves_;
	delete initial_;
	delete spillWorklist_;
	delete freezeWorklist_;
	delete simplifyWorklist_;
	delete coalescedNodes_;
	delete selectStack_;
	delete spilledNode_;
}

bool InterfereGraph::shouldRepeat() const
{
	return simplifyWorklist_->next_ != simplifyWorklist_ || worklistMoves_->next_ != worklistMoves_
	       || freezeWorklist_->next_ != freezeWorklist_ || spillWorklist_->next_ != spillWorklist_;
}

bool InterfereGraph::simplify() const
{
	if (simplifyWorklist_->next_ == simplifyWorklist_) return false;
	LOG(color::cyan("simplify"));
	PUSH;
	auto n = simplifyWorklist_->next_;
	LOG(color::pink("simplifyWorklist_ - ") + n->reg_->print());
	takeNode(simplifyWorklist_, n);
	LOG(color::pink("selectStack_ + ") + n->reg_->print());
	addNode(selectStack_, n);
	for (auto adj : n->adjList_)
		if (adj->parent_ != selectStack_ && adj->parent_ != coalescedNodes_)
			decrementDegree(adj);
	POP;
	return true;
}

bool InterfereGraph::coalesce()
{
	if (worklistMoves_->next_ == worklistMoves_)
		return false;
	LOG(color::cyan("coalesce"));
	PUSH;
	auto m = worklistMoves_->next_;
	auto x = xOf(m);
	auto y = yOf(m);
	x = getAlias(x);
	y = getAlias(y);
	if (y->reg_->isPhysicalRegister())
	{
		auto y2 = y;
		y = x;
		x = y2;
	}
	auto rx = x->reg_;
	auto ry = y->reg_;
	takeNode(worklistMoves_, m);
	if (x == y)
	{
		addNode(coalescedMoves_, m);
		addWorkList(x);
	}
	else
	{
		auto idx = parent_->live_message()->regIdOf(rx);
		auto idy = parent_->live_message()->regIdOf(ry);
		if (ry->isPhysicalRegister() || adjSet_.test(idx, idy))
		{
			addNode(constrainedMoves_, m);
			addWorkList(x);
			addWorkList(y);
		}
		else if ((rx->isPhysicalRegister() && allOk(x, y)) || (
			         rx->isVirtualRegister() && conservative(x->adjList_, y->adjList_)))
		{
			addNode(coalescedMoves_, m);
			combine(x, y);
			addWorkList(x);
		}
		else
		{
			addNode(activeMoves_, m);
		}
	}
	POP;
	return true;
}

bool InterfereGraph::freeze()
{
	if (freezeWorklist_->next_ == freezeWorklist_) return false;
	LOG(color::cyan("freeze"));
	PUSH;
	auto u = freezeWorklist_->next_;
	LOG(color::pink("freezeWorklist_ - ") + u->reg_->print());
	takeNode(freezeWorklist_, u);
	LOG(color::pink("simplifyWorklist_ + ") + u->reg_->print());
	addNode(simplifyWorklist_, u);
	freezeMove(u);
	POP;
	return true;
}

bool InterfereGraph::selectSpill()
{
	if (spillWorklist_->next_ == spillWorklist_) return false;
	LOG(color::cyan("selectSpill"));
	PUSH;
	float mw = FLT_MAX;
	InterfereGraphNode* m = nullptr;
	for (auto node = spillWorklist_->next_; node != spillWorklist_; node = node->next_)
	{
		float f;
		auto vreg = dynamic_cast<VirtualRegister*>(node->reg_);
		if (node->degree_ == 0 || node->reg_->isPhysicalRegister() || vreg->
		    spilled || vreg->size() == 128)
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
	takeNode(spillWorklist_, m);
	LOG(color::pink("simplifyWorklist_ + ") + m->reg_->print());
	addNode(simplifyWorklist_, m);
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
	while (selectStack_->next_ != selectStack_)
	{
		auto n = selectStack_->pre_;
		LOG(color::pink("selectStack_ - ") + n->reg_->print());
		takeNode(selectStack_, n);
		DynamicBitset oks = mask;
		for (auto w : n->adjList_)
		{
			auto c = getAlias(w)->color_;
			if (c != nullptr) oks.reset(message->regIdOf(c));
		}
		if (oks.allZeros())
		{
			LOG(color::pink("spilledNode_ + ") + n->reg_->print());
			addNode(spilledNode_, n);
		}
		else
			n->color_ = dynamic_cast<Register*>(message->getReg(static_cast<int>(*oks.begin())));
	}
	for (auto it = coalescedNodes_->next_; it != coalescedNodes_; it = it->next_)
	{
		it->color_ = getAlias(it)->color_;
	}
	POP;
}

bool InterfereGraph::needRewrite() const
{
	return spilledNode_->next_ != spilledNode_;
}

void InterfereGraph::rewriteProgram() const
{
	auto func = parent_->currentFunc();
	for (auto it = spilledNode_->next_; it != spilledNode_; it = it->next_)
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
		DynamicBitset live = message.live_out()[bb->id()];
		auto& insts = bb->instructions();
		for (auto iit = insts.rbegin(); iit != insts.rend(); ++iit)
		{
			auto inst = *iit;
			for (auto i : inst->use_)
			{
				auto vr = dynamic_cast<VirtualRegister*>(inst->operands_[i]);
				if (vr != nullptr && message.care(vr))
				{
					getOrCreateRegNode(vr)->weight_ += bb->weight_;
				}
			}
			for (auto i : inst->def())
			{
				auto vr = dynamic_cast<VirtualRegister*>(inst->operands_[i]);
				if (vr != nullptr && message.care(vr))
				{
					getOrCreateRegNode(vr)->weight_ += bb->weight_;
				}
			}
			auto use = message.translate(inst->operands_, inst->use_) | message.translate(inst->imp_use());
			auto def = message.translate(inst->operands_, inst->def_) | message.translate(inst->imp_def());
			if (auto cp = dynamic_cast<MCopy*>(inst);
				cp != nullptr && cp->def(0)->isCanAllocateRegister() && cp->use(0)->isCanAllocateRegister())
			{
				LOGIF(color::red("Copy Regs"),
				      ((dynamic_cast<VirtualRegister*> (cp->def(0)) != nullptr) || (dynamic_cast<VirtualRegister*>(cp->
					      use
					      (0)) != nullptr)));
				live -= use;
				auto ud = use | def;
				for (auto i : ud)
					getOrCreateRegNode(message.getReg(static_cast<int>(i)))->moveList_.
					                                                         emplace_back(getOrCreateMoveNode(cp));
				addNode(worklistMoves_, getOrCreateMoveNode(cp));
			}
			live |= def;
			for (auto i : def)
				for (auto j : live)
					addEdge(static_cast<unsigned>(j), static_cast<unsigned>(i));
			live -= def;
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
	resetNode(worklistMoves_);
	resetNode(activeMoves_);
	resetNode(coalescedMoves_);
	resetNode(constrainedMoves_);
	resetNode(frozenMoves_);
	resetNode(initial_);
	resetNode(spillWorklist_);
	resetNode(freezeWorklist_);
	resetNode(simplifyWorklist_);
	resetNode(coalescedNodes_);
	resetNode(selectStack_);
	resetNode(spilledNode_);
	int rc = parent_->live_message()->regCount();
	K_ = 0;
	for (int i = 0; i < rc; i++)
	{
		auto r = parent_->live_message()->getReg(i);
		if (r->isVirtualRegister())
		{
			LOG(color::pink("initial_ + ") + r->print());
			addNode(initial_, getOrCreateRegNode(r));
		}
		else K_++;
	}
	adjSet_ = AdjSet{rc};
	POP;
}

void InterfereGraph::makeWorklist() const
{
	LOG(color::cyan("makeWorklist"));
	PUSH;
	for (auto it = initial_->next_, itp = it->next_; it != initial_; it = itp, itp = itp->next_)
	{
		LOG(color::pink("initial_ - ") + it->reg_->print());
		takeNode(initial_, it);
		if (it->degree_ >= K_)
		{
			LOG(color::pink("spillWorklist_ + ") + it->reg_->print());
			addNode(spillWorklist_, it);
		}
		else if (moveRelated(it))
		{
			LOG(color::pink("freezeWorklist_ + ") + it->reg_->print());
			addNode(freezeWorklist_, it);
		}
		else
		{
			LOG(color::pink("simplifyWorklist_ + ") + it->reg_->print());
			addNode(simplifyWorklist_, it);
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
			{
				beg++;
				continue;
			}
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
				colors1_[indexes1_[i]] = color;
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
			if (l->size() < r->size()) l->set_size(r->size());
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

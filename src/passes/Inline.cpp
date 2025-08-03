#include "Inline.hpp"

#include <queue>

#include "BasicBlock.hpp"
#include "Instruction.hpp"

#define DEBUG 1
#include "BBRank.hpp"
#include "Config.hpp"
#include "Type.hpp"
#include "Util.hpp"

void IBBNode::add(IBBNode* target)
{
	if (target->type_ != nullptr) target->type_->remove(target);
	target->type_ = this;
	target->r_ = this;
	target->l_ = l_;
	target->l_->r_ = target;
	target->r_->l_ = target;
}

void IBBNode::remove(IBBNode* target) const
{
	if (target->type_ == this)
	{
		target->type_ = nullptr;
		target->l_->r_ = target->r_;
		target->r_->l_ = target->l_;
	}
}

void IBBNode::clear()
{
	l_ = this;
	r_ = this;
}

void IBBNode::discard()
{
	if (type_ != nullptr)
	{
		l_->r_ = r_;
		r_->l_ = l_;
		type_ = nullptr;
	}
}

bool IBBNode::contain(const IBBNode* target) const
{
	return target->type_ == this;
}

bool IBBNode::empty() const
{
	return r_ == this;
}

IBBNode* IBBNode::top() const
{
	assert(r_ != this);
	return l_;
}

IBBNode* IBBNode::pop() const
{
	assert(r_ != this);
	auto l = l_;
	remove(l_);
	return l;
}

bool IBBNode::haveNext(const IBBNode* n) const
{
	return next_.test(n->id_);
}

bool IBBNode::havePre(const IBBNode* n) const
{
	return pre_.test(n->id_);
}

Inline::Inline(Module* m) : Pass(m), waitlist_(new IBBNode{nullptr, nullptr, nullptr, nullptr, {}, {}, 0, 0, 0}),
                            done_(new IBBNode{nullptr, nullptr, nullptr, nullptr, {}, {}, 0, 0, 0}),
                            return_(new IBBNode{nullptr, nullptr, nullptr, nullptr, {}, {}, 0, 0, 0})
{
	waitlist_->clear();
	done_->clear();
	return_->clear();
}

void Inline::run()
{
	LOG(color::cyan("Run Inline Pass"));
	PUSH;
	std::vector<Function*> funcs;
	for (const auto& func : m_->get_functions())
	{
		if (func->is_lib_) continue;
		funcWorkList_.emplace(func);
		funcVisited_.emplace(func);
	}
	while (!funcWorkList_.empty())
	{
		f_ = funcWorkList_.front();
		funcWorkList_.pop();
		if (removedFunc_.count(f_)) continue;
		funcVisited_.erase(f_);
		runOnFunc();
	}
	/*
	 auto& rmf = removedFunc_;
	m_->get_functions().remove_if([&rmf](Function* f)
	{
		return rmf.count(f);
	});
	 */
	//for (auto i : removedFunc_) delete i;
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("Inline Done"));
}

void Inline::collectNodes()
{
	nodes_.clear();
	waitlist_->clear();
	done_->clear();
	return_->clear();
	auto& bbs = f_->get_basic_blocks();
	int abc = u2iNegThrow(bbs.size());
	for (auto i : allNodes_) delete i;
	allNodes_.resize(abc);
	int bc = 0;
	for (auto i : bbs)
	{
		auto n = new IBBNode{nullptr, nullptr, nullptr, i, {abc}, {abc}, 0, 0, bc};
		nodes_.emplace(i, n);
		allNodes_[bc++] = n;
		waitlist_->add(n);
	}
	for (auto i : bbs)
	{
		auto in = nodes_[i];
		for (auto s : i->get_succ_basic_blocks())
		{
			auto sn = nodes_[s];
			initializeAddEdge(in, sn);
		}
	}
}

void Inline::mergeBlocks()
{
	LOG(color::blue("Begin Merge Blocks of ") + f_->get_name());
	PUSH;
	while (!waitlist_->empty())
	{
		auto pre = waitlist_->pop();
		done_->add(pre);
		if (pre->nc_ == 1)
		{
			auto next = firstNext(pre);
			if (next->pc_ == 1)
			{
				merge(pre, next);
				next->discard();
			}
		}
	}
	done2WaitList();
	POP;
}

void Inline::mergeReturns()
{
	LOG(color::blue("Begin Merge Returns of ") + f_->get_name());
	PUSH;
	std::unordered_map<Value*, IBBNode*> nodeMap;
	selectReturnNodes(nodeMap);
	while (!return_->empty())
	{
		auto ret = return_->pop();
		auto inst = ret->block_->get_instructions().back();
		bool again = false;
		for (auto pre : ret->pre_)
		{
			auto preNode = node(pre);
			if (preNode->nc_ == 1)
			{
				auto& insts = preNode->block_->get_instructions();
				delete insts.pop_back();
				inst->copy(preNode->block_);
				removeEdge(preNode, ret);
				if (insts.size() == 1)
				{
					LOG(color::pink("Duplicate Return Node ") + preNode->block_->get_name() + color::pink(" replace ") +
						ret->block_->get_name());
					replaceGoTo(ret, preNode);
					ret->discard();
					f_->remove(ret->block_);
					ret->block_ = nullptr;
					ret = preNode;
					again = true;
					break;
				}
			}
		}
		if (again)
		{
			return_->add(ret);
		}
		else
		{
			if (ret->pc_ == 0 && ret->block_ != f_->get_entry_block())
			{
				LOG(color::pink("Remove Dead Node ") + ret->block_->get_name());
				f_->remove(ret->block_);
				ret->discard();
			}
			else
				done_->add(ret);
		}
	}
	done2WaitList();
	POP;
}

void Inline::mergeBranchs()
{
	LOG(color::blue("Begin Merge Branchs of ") + f_->get_name());
	PUSH;
	selectBranchNodes();
	simplifyBranchs();
	while (!waitlist_->empty())
	{
		auto branch = waitlist_->pop();
		auto inst = dynamic_cast<BranchInst*>(branch->block_->get_instructions().back());
		if (branch->nc_ > 1)
		{
			for (auto pre : branch->pre_)
			{
				auto preNode = node(pre);
				if (preNode->nc_ > 1) continue;
				auto& insts = preNode->block_->get_instructions();
				auto br = dynamic_cast<BranchInst*>(insts.back());
				assert(br != nullptr);
				LOG(color::yellow("Merge Branch ") + preNode->block_->get_name() + color::yellow(" to ") + branch->
					block_->get_name());
				br->remove_all_operands();
				br->add_operand(inst->get_operand(0));
				br->add_operand(inst->get_operand(1));
				br->add_operand(inst->get_operand(2));
				removeEdge(preNode, branch);
				for (auto next : branch->next_)
				{
					auto nextNode = node(next);
					addEdge(preNode, nextNode);
					for (auto phii : nextNode->block_->get_instructions().phi_and_allocas())
					{
						auto phi = dynamic_cast<PhiInst*>(phii);
						auto val = phi->get_phi_val(branch->block_);
						if (val != nullptr) phi->add_phi_pair_operand(val, preNode->block_);
					}
				}
			}
		}
		else
		{
			auto nextNode = firstNext(branch);
			for (auto pre : branch->pre_)
			{
				auto preNode = node(pre);
				if (preNode->nc_ == 1 || !preNode->haveNext(nextNode))
				{
					auto& insts = preNode->block_->get_instructions();
					auto br = dynamic_cast<BranchInst*>(insts.back());
					assert(br != nullptr);
					LOG(color::yellow("Merge Branch ") + preNode->block_->get_name() + color::yellow(" to ") + branch->
						block_->get_name());
					br->replaceAllOperandMatchs(branch->block_, nextNode->block_);
					removeEdge(preNode, branch);
					addEdge(preNode, nextNode);
					for (auto phii : nextNode->block_->get_instructions().phi_and_allocas())
					{
						auto phi = dynamic_cast<PhiInst*>(phii);
						auto val = phi->get_phi_val(branch->block_);
						if (val != nullptr) phi->add_phi_pair_operand(val, preNode->block_);
					}
				}
				else
				{
					bool ok = true;
					for (auto phii : nextNode->block_->get_instructions().phi_and_allocas())
					{
						auto phi = dynamic_cast<PhiInst*>(phii);
						auto preVal = phi->get_phi_val(preNode->block_);
						auto nextVal = phi->get_phi_val(branch->block_);
						if (preVal != nullptr && nextVal != nullptr && preVal != nextVal)
						{
							ok = false;
							break;
						}
					}
					if (ok)
					{
						auto& insts = preNode->block_->get_instructions();
						auto br = dynamic_cast<BranchInst*>(insts.back());
						assert(br != nullptr);
						LOG(color::yellow("Merge Branch ") + preNode->block_->get_name() + color::yellow(" to ") +
							branch->
							block_->get_name());
						br->remove_all_operands();
						br->add_operand(nextNode->block_);
						removeEdge(preNode, branch);
						addEdge(preNode, nextNode);
						for (auto phii : nextNode->block_->get_instructions().phi_and_allocas())
						{
							auto phi = dynamic_cast<PhiInst*>(phii);
							auto preVal = phi->get_phi_val(preNode->block_);
							auto nextVal = phi->get_phi_val(branch->block_);
							if (nextVal != nullptr && preVal == nullptr)
								phi->add_phi_pair_operand(
									nextVal, preNode->block_);
						}
					}
				}
			}
		}
		if (branch->pc_ == 0)
		{
			for (auto next : branch->next_)
			{
				auto nextNode = node(next);
				for (auto p : nextNode->block_->get_instructions().phi_and_allocas())
				{
					auto phi = dynamic_cast<PhiInst*>(p);
					assert(phi != nullptr);
					phi->remove_phi_operand(branch->block_);
					removeEdge(branch, nextNode);
				}
			}
			f_->remove(branch->block_);
			branch->block_ = nullptr;
			branch->discard();
		}
		else done_->add(branch);
	}
	POP;
}

void Inline::simplifyBranchs() const
{
	LOG(color::blue("Begin Simplify Branchs of ") + f_->get_name());
	PUSH;
	for (auto i : f_->get_basic_blocks())
	{
		auto inst = dynamic_cast<BranchInst*>(i->get_instructions().back());
		if (inst != nullptr && inst->is_cond_br() && i->get_succ_basic_blocks().size() == 1)
		{
			auto to = inst->get_operand(1);
			inst->remove_all_operands();
			inst->add_operand(to);
			LOG(color::yellow("Simpify ") + i->get_name() + color::yellow(" same cond branch to ") +
				dynamic_cast<BasicBlock*>(to)->get_name());
		}
	}
	POP;
}

void Inline::replaceGoTo(IBBNode* from, IBBNode* to) const
{
	LOG(color::yellow("Replace -> ") + from->block_->get_name() + color::yellow(" with -> ") + to->block_->get_name(
	));
	from->block_->replace_all_use_with(to->block_);
	for (auto pre : from->pre_)
	{
		auto preNode = node(pre);
		removeEdge(preNode, from);
		addEdge(preNode, to);
	}
}

void Inline::selectReturnNodes(std::unordered_map<Value*, IBBNode*>& nodeMap) const
{
	LOG(color::blue("Discover Return Nodes of ") + f_->get_name());
	PUSH;
	while (!waitlist_->empty())
	{
		auto n = waitlist_->pop();
		auto& insts = n->block_->get_instructions();
		if (insts.size() == 1 && dynamic_cast<ReturnInst*>(insts.back()))
		{
			auto val = insts.back()->get_operand(0);
			auto fd = nodeMap.find(val);
			if (fd == nodeMap.end())
			{
				LOG(color::pink("Return Node ") + n->block_->get_name() + color::pink(" return ") + val->print());
				return_->add(n);
				nodeMap.emplace(val, n);
			}
			else
			{
				LOG(color::pink("Duplicate Return Node ") + n->block_->get_name() + color::pink(" return ") + val->print
					());
				auto rp = fd->second;
				replaceGoTo(n, rp);
				n->discard();
				f_->remove(n->block_);
				n->block_ = nullptr;
			}
		}
		else done_->add(n);
	}
	POP;
	LOG(color::green("All Collected"));
}

void Inline::selectBranchNodes()
{
	LOG(color::blue("Discover Branch Nodes of ") + f_->get_name());
	PUSH;
	while (!waitlist_->empty())
	{
		auto n = waitlist_->pop();
		auto& insts = n->block_->get_instructions();
		if (insts.size() == 1 && dynamic_cast<BranchInst*>(insts.back()))
		{
			LOG(color::pink("Branch Node ") + n->block_->get_name());
			return_->add(n);
		}
		else done_->add(n);
	}
	auto w = waitlist_;
	waitlist_ = return_;
	return_ = w;
	POP;
	LOG(color::green("All Collected"));
}

void Inline::merge(IBBNode* pre, IBBNode* next) const
{
	LOG(color::yellow("merge ") + pre->block_->get_name() + " " + next->block_->get_name());
	auto prebb = pre->block_;
	auto nextbb = next->block_;
	auto ed = nextbb->get_instructions().
	                  phi_and_allocas().end();
	auto it = nextbb->get_instructions().phi_and_allocas().begin();
	while (it != ed)
	{
		auto phi = dynamic_cast<PhiInst*>(it.get_and_add());
		assert(phi != nullptr && phi->get_phi_pairs().size() == 1);
		auto [val, bb] = phi->get_phi_pairs()[0];
		assert(bb == prebb);
		phi->replace_all_use_with(val);
		delete it.remove_pre();
	}
	auto& insts = prebb->get_instructions();
	delete insts.pop_back();
	nextbb->replace_all_use_with(prebb);
	for (auto i : nextbb->get_instructions()) i->set_parent(prebb);
	insts.addAll(nextbb->get_instructions());
	nextbb->get_instructions().clear();
	removeEdge(pre, next);
	for (auto i : next->next_)
	{
		auto nn = node(i);
		removeEdge(next, nn);
		addEdge(pre, nn);
	}
	f_->remove(next->block_);
	next->block_ = nullptr;
}

void Inline::done2WaitList()
{
	auto d = done_;
	done_ = waitlist_;
	waitlist_ = d;
}

namespace
{
	Value* getOrDefault(std::unordered_map<Value*, Value*>& m, Value* t)
	{
		auto fd = m.find(t);
		if (fd == m.end()) return t;
		return fd->second;
	}
}

void Inline::mergeFunc()
{
	LOG(color::blue("Merge Func ") + f_->get_name());
	PUSH;
	auto& ii = f_->get_entry_block()->get_instructions();
	std::vector<CallInst*> uses;
	uses.reserve(f_->get_use_list().size());
	for (auto i : f_->get_use_list())
	{
		auto usage = dynamic_cast<CallInst*>(i.val_);
		assert(usage);
		uses.emplace_back(usage);
	}
	for (auto call : uses)
	{
		auto bb = call->get_parent();
		auto f = bb->get_parent();
		LOG(color::yellow("Merge Func ") + f_->get_name() + color::yellow(" in ") + f->get_name());
		auto& insts = bb->get_instructions();
		auto begin = insts.begin();
		auto ed = insts.end();
		while (begin != ed)
		{
			auto nod = *begin;
			if (nod == call)
			{
				auto pos = begin;
				std::unordered_map<Value*, Value*> valMap;
				int idx = 0;
				for (auto& arg : f_->get_args())
				{
					auto to = call->get_operand(++idx);
					valMap.emplace(&arg, to);
				}
				for (auto in : ii)
				{
					auto inst = in->copy(valMap);
					if (inst != nullptr)
					{
						inst->set_parent(bb);
						insts.emplace_common_inst_after(inst, pos);
						++pos;
					}
				}
				auto ret = dynamic_cast<ReturnInst*>(ii.back());
				if (f_->get_return_type() == Types::VOID)
				{
					delete insts.remove(begin);
					break;
				}
				nod->replace_all_use_with(getOrDefault(valMap,ret->get_operand(0)));
				delete insts.remove(begin);
				break;
			}
			++begin;
		}
		if (!funcVisited_.count(f))
		{
			funcWorkList_.emplace(f);
			funcVisited_.emplace(f);
		}
	}
	removedFunc_.emplace(f_);
	POP;
}

IBBNode* Inline::node(int i) const
{
	return allNodes_[i];
}

IBBNode* Inline::firstNext(const IBBNode* n) const
{
	if (n->nc_ == 0) return nullptr;
	return allNodes_[*n->next_.begin()];
}

IBBNode* Inline::firstPre(const IBBNode* n) const
{
	if (n->pc_ == 0) return nullptr;
	return allNodes_[*n->pre_.begin()];
}

void Inline::runOnFunc()
{
	LOG(color::blue("Begin Inline of ") + f_->get_name());
	PUSH;
	collectNodes();
	mergeBlocks();
	mergeReturns();
	mergeBranchs();
	if (f_->get_basic_blocks().size() == 1)
	{
		auto& insts = f_->get_entry_block()->get_instructions();
		if (insts.size() <= funcInlineGate && f_->get_name() != "main")
		{
			for (auto i : insts)
			{
				if (i->is_alloca())
				{
					POP;
					return;
				}
			}
			mergeFunc();
		}
	}
	POP;
}

void Inline::removeEdge(IBBNode* from, IBBNode* to)
{
	bool f = from->next_.resetAndGet(to->id_);
	from->nc_ -= f;
	bool t = to->pre_.resetAndGet(from->id_);
	to->pc_ -= t;
	if (f) from->block_->remove_succ_basic_block(to->block_);
	if (t) to->block_->remove_pre_basic_block(from->block_);
}

void Inline::addEdge(IBBNode* from, IBBNode* to)
{
	bool f = from->next_.setAndGet(to->id_);
	from->nc_ += f;
	bool t = to->pre_.setAndGet(from->id_);
	to->pc_ += t;
	if (f) from->block_->add_succ_basic_block(to->block_);
	if (t) to->block_->add_pre_basic_block(from->block_);
}

void Inline::initializeAddEdge(IBBNode* from, IBBNode* to)
{
	bool f = from->next_.setAndGet(to->id_);
	from->nc_ += f;
	bool t = to->pre_.setAndGet(from->id_);
	to->pc_ += t;
}

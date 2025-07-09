#include "InstructionList.hpp"

#include <stdexcept>

#include "Instruction.hpp"

InstructionListNode::InstructionListNode(Instruction* instruction, InstructionListNode* next, InstructionListNode* pre)
	: instruction(instruction),
	  next(next),
	  pre(pre)
{
}

InstructionListNode::InstructionListNode()
{
	instruction = nullptr;
	next = this;
	pre = this;
}

InstructionListIterator::InstructionListIterator(InstructionList* parent, InstructionListNode* current)
	: parent_(parent),
	  current_(current)
{
}

InstructionListIterator& InstructionListIterator::operator++()
{
	current_ = current_->next;
	return *this;
}

InstructionListIterator& InstructionListIterator::operator--()
{
	current_ = current_->pre;
	return *this;
}

Instruction* InstructionListIterator::operator*() const
{
	if (current_ == nullptr) return nullptr;
	return current_->instruction;
}

// ReSharper disable once CppMemberFunctionMayBeConst
Instruction* InstructionListIterator::remove_next()
{
	return parent_->erase(current_->next);
}

// ReSharper disable once CppMemberFunctionMayBeConst
Instruction* InstructionListIterator::remove_pre()
{
	return parent_->erase(current_->pre);
}

Instruction* InstructionListIterator::get_and_add()
{
	const auto ret = **this;
	if (ret != nullptr)
		++*this;
	return ret;
}

Instruction* InstructionListIterator::get_and_sub()
{
	const auto ret = **this;
	if (ret != nullptr)
		--*this;
	return ret;
}


InstructionListView::InstructionListView(InstructionListNode* begin, InstructionListNode* end,
                                         InstructionList* instructions): begin_(begin), end_(end), parent_(instructions)
{
}

InstructionListIterator InstructionListView::begin() const
{
	return InstructionListIterator{parent_, begin_->next};
}

const InstructionListNode* InstructionListView::end() const
{
	return end_;
}

InstructionListIterator InstructionListView::rbegin() const
{
	return InstructionListIterator{parent_, end_->pre};
}

const InstructionListNode* InstructionListView::rend() const
{
	return begin_;
}

Instruction* InstructionListView::erase(const InstructionListIterator& iterator) const
{
	if (iterator.current_ == begin_ || iterator.current_ == end_) return nullptr;
	return parent_->erase(iterator);
}

Instruction* InstructionListView::remove(const InstructionListIterator& iterator) const
{
	if (iterator.current_ == begin_ || iterator.current_ == end_) return nullptr;
	return parent_->remove(iterator);
}

bool InstructionListView::remove_first(const Instruction* instruction) const
{
	if (begin_ == end_) return false;
	auto node = begin_->next;
	while (node != end_)
	{
		if (node->instruction == instruction)
		{
			node->pre->next = node->next;
			node->next->pre = node->pre;
			if (instruction->is_phi() || instruction->is_alloca())
				parent_->phi_alloca_size_--;
			else parent_->common_inst_size_--;
			delete node;
			return true;
		}
		node = node->next;
	}
	return false;
}

bool InstructionListView::remove_last(const Instruction* instruction) const
{
	if (begin_ == end_) return false;
	auto node = end_->pre;
	while (node != begin_)
	{
		if (node->instruction == instruction)
		{
			node->pre->next = node->next;
			node->next->pre = node->pre;
			delete node;
			if (instruction->is_phi() || instruction->is_alloca())
				parent_->phi_alloca_size_--;
			else parent_->common_inst_size_--;
			return true;
		}
		node = node->pre;
	}
	return false;
}

bool InstructionListView::erase_first(const Instruction* instruction) const
{
	return remove_first(instruction);
}

bool InstructionListView::erase_last(const Instruction* instruction) const
{
	return erase_first(instruction);
}

InstructionListIterator InstructionList::begin()
{
	return InstructionListIterator{this, end_node_->next};
}

const InstructionListNode* InstructionList::end() const
{
	return end_node_;
}

InstructionListIterator InstructionList::rbegin()
{
	return InstructionListIterator{this, end_node_->pre};
}

const InstructionListNode* InstructionList::rend() const
{
	return end_node_;
}

InstructionList::InstructionList() : end_node_(new InstructionListNode())
{
	common_inst_begin_ = end_node_;
	phi_alloca_size_ = 0;
	common_inst_size_ = 0;
}

InstructionListView InstructionList::all_instructions() const
{
	return InstructionListView{end_node_, end_node_, const_cast<InstructionList*>(this)};
}

InstructionListView InstructionList::phi_and_allocas() const
{
	return InstructionListView{end_node_, common_inst_begin_, const_cast<InstructionList*>(this)};
}

bool InstructionList::empty() const
{
	return phi_alloca_size_ == 0 && common_inst_size_ == 0;
}

int InstructionList::size() const
{
	return phi_alloca_size_ + common_inst_size_;
}

Instruction* InstructionList::back() const
{
	return end_node_->pre->instruction;
}

Instruction* InstructionList::remove(const InstructionListIterator& iterator)
{
	return erase(iterator.current_);
}

void InstructionList::addAll(const InstructionList& instructions)
{
	if (instructions.phi_alloca_size_ > 0)
	{
		auto appendNode = common_inst_begin_->pre;
		phi_alloca_size_ += instructions.phi_alloca_size_;
		auto b = instructions.end_node_->next;
		auto e = instructions.common_inst_begin_;
		while (b != e)
		{
			InstructionListNode* node = new InstructionListNode{b->instruction, nullptr, appendNode};
			appendNode->next = node;
			appendNode = node;
			b = b->next;
		}
		appendNode->next = common_inst_begin_;
		common_inst_begin_->pre = appendNode;
	}
	if (instructions.common_inst_size_ > 0)
	{
		auto appendNode = end_node_->pre;
		common_inst_size_ += instructions.common_inst_size_;
		auto b = instructions.common_inst_begin_;
		auto e = instructions.end_node_;
		while (b != e)
		{
			InstructionListNode* node = new InstructionListNode{b->instruction, nullptr, appendNode};
			appendNode->next = node;
			appendNode = node;
			if (common_inst_begin_ == end_node_) common_inst_begin_ = node;
			b = b->next;
		}
		appendNode->next = end_node_;
		end_node_->pre = appendNode;
	}
}

bool InstructionList::remove_first(const Instruction* instruction)
{
	if (instruction->is_alloca() || instruction->is_phi())
	{
		auto node = end_node_->next;
		while (node != common_inst_begin_)
		{
			if (node->instruction == instruction)
			{
				node->pre->next = node->next;
				node->next->pre = node->pre;
				delete node;
				phi_alloca_size_--;
				return true;
			}
			node = node->next;
		}
		return false;
	}
	if (common_inst_begin_ == end_node_) return false;
	if (common_inst_begin_->instruction == instruction)
	{
		common_inst_begin_->pre->next = common_inst_begin_->next;
		common_inst_begin_->next->pre = common_inst_begin_->pre;
		const auto del = common_inst_begin_;
		common_inst_begin_ = common_inst_begin_->next;
		delete del;
		common_inst_size_--;
		return true;
	}
	auto node = common_inst_begin_->next;
	while (node != end_node_)
	{
		if (node->instruction == instruction)
		{
			node->pre->next = node->next;
			node->next->pre = node->pre;
			delete node;
			common_inst_size_--;
			return true;
		}
		node = node->next;
	}
	return false;
}

bool InstructionList::remove_last(const Instruction* instruction)
{
	if (instruction->is_alloca() || instruction->is_phi())
	{
		auto node = common_inst_begin_->pre;
		while (node != end_node_)
		{
			if (node->instruction == instruction)
			{
				node->pre->next = node->next;
				node->next->pre = node->pre;
				delete node;
				phi_alloca_size_--;
				return true;
			}
			node = node->pre;
		}
		return false;
	}
	if (common_inst_begin_ == end_node_) return false;
	if (common_inst_begin_->instruction == instruction)
	{
		common_inst_begin_->pre->next = common_inst_begin_->next;
		common_inst_begin_->next->pre = common_inst_begin_->pre;
		const auto del = common_inst_begin_;
		common_inst_begin_ = common_inst_begin_->next;
		delete del;
		common_inst_size_--;
		return true;
	}
	auto node = end_node_->pre;
	while (node != common_inst_begin_)
	{
		if (node->instruction == instruction)
		{
			node->pre->next = node->next;
			node->next->pre = node->pre;
			delete node;
			common_inst_size_--;
			return true;
		}
		node = node->pre;
	}
	return false;
}

bool InstructionList::erase_first(const Instruction* instruction)
{
	return remove_first(instruction);
}

bool InstructionList::erase_last(const Instruction* instruction)
{
	return remove_last(instruction);
}

void InstructionList::emplace_back(Instruction* instruction)
{
	if (instruction->is_phi() || instruction->is_alloca()) emplace_back_phi_alloca_inst(instruction);
	else emplace_back_common_inst(instruction);
}

void InstructionList::emplace_front(Instruction* instruction)
{
	if (instruction->is_phi() || instruction->is_alloca()) emplace_front_phi_alloca_inst(instruction);
	else emplace_front_common_inst(instruction);
}

Instruction* InstructionList::erase(const InstructionListIterator& iterator)
{
	return erase(iterator.current_);
}

void InstructionList::emplace_back_common_inst(Instruction* instruction)
{
	assert(!instruction->is_phi() && !instruction->is_alloca() &&
		"Phi or alloca is not common instruction");
	const auto node = new InstructionListNode{instruction, end_node_, end_node_->pre};
	end_node_->pre->next = node;
	end_node_->pre = node;
	if (common_inst_begin_ == end_node_) common_inst_begin_ = node;
	common_inst_size_++;
}

void InstructionList::emplace_back_phi_alloca_inst(Instruction* instruction)
{
	assert((instruction->is_phi() || instruction->is_alloca()) &&
		"Instruction not phi or alloca");
	const auto node = new InstructionListNode{instruction, common_inst_begin_, common_inst_begin_->pre};
	common_inst_begin_->pre->next = node;
	common_inst_begin_->pre = node;
	phi_alloca_size_++;
}

void InstructionList::emplace_front_phi_alloca_inst(Instruction* instruction)
{
	assert((instruction->is_phi() || instruction->is_alloca()) &&
		"Instruction not phi or alloca");
	const auto node = new InstructionListNode{instruction, end_node_->next, end_node_};
	end_node_->next->pre = node;
	end_node_->next = node;
	phi_alloca_size_++;
}

void InstructionList::emplace_front_common_inst(Instruction* instruction)
{
	assert(!instruction->is_phi() && !instruction->is_alloca() &&
		"Phi or alloca is not common instruction");
	const auto node = new InstructionListNode{instruction, common_inst_begin_, common_inst_begin_->pre};
	common_inst_begin_->pre->next = node;
	common_inst_begin_->pre = node;
	common_inst_begin_ = node;
	common_inst_size_++;
}

Instruction* InstructionList::erase(const InstructionListNode* node)
{
	if (node == end_node_) return nullptr;
	node->pre->next = node->next;
	node->next->pre = node->pre;
	if (node == common_inst_begin_)
		common_inst_begin_ = node->next;
	const auto ret = node->instruction;
	delete node;
	if (ret->is_phi() || ret->is_alloca())
		phi_alloca_size_--;
	else common_inst_size_--;
	return ret;
}

bool operator==(const InstructionListIterator& lhs, const InstructionListIterator& rhs)
{
	return lhs.current_ == rhs.current_ && lhs.parent_ == rhs.parent_;
}

bool operator!=(const InstructionListIterator& lhs, const InstructionListIterator& rhs)
{
	return lhs.current_ != rhs.current_ || lhs.parent_ != rhs.parent_;
}

bool operator==(const InstructionListIterator& lhs, const InstructionListNode* rhs)
{
	return lhs.current_ == rhs;
}

bool operator!=(const InstructionListIterator& lhs, const InstructionListNode* rhs)
{
	return lhs.current_ != rhs;
}

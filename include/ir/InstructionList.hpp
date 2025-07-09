#pragma once

class Instruction;
class InstructionList;

class InstructionListNode
{
	friend class InstructionListReversedIterator;
	friend class InstructionListIterator;
	friend class InstructionList;
	friend class InstructionListView;

public:
	InstructionListNode(Instruction* instruction, InstructionListNode* next, InstructionListNode* pre);

	InstructionListNode();

private:
	Instruction* instruction;
	InstructionListNode* next;
	InstructionListNode* pre;
};

class InstructionListIterator
{
	friend class InstructionListView;
	friend class InstructionList;
	friend bool operator==(const InstructionListIterator& lhs, const InstructionListIterator& rhs);

	friend bool operator!=(const InstructionListIterator& lhs, const InstructionListIterator& rhs);

	friend bool operator==(const InstructionListIterator& lhs, const InstructionListNode* rhs);

	friend bool operator!=(const InstructionListIterator& lhs, const InstructionListNode* rhs);

public:
	InstructionListIterator(InstructionList* parent, InstructionListNode* current);
	InstructionListIterator& operator++();
	InstructionListIterator& operator--();

	Instruction* operator*() const;
	/**
	 * 去掉迭代器下一个指向的指令, 只会更新迭代器自身和 InstructionList, 如果不指向指令则不去掉.
	 * 注意其不检查迭代器是否越界, 一般配合 while 和 get_and_sub 使用
	 * @return 去掉的指令, 如果未去掉则返回 nullptr
	 */
	Instruction* remove_next();
	/**
	 * 去掉迭代器下一个指向的指令, 只会更新迭代器自身和 InstructionList, 如果不指向指令则不去掉.
	 * 注意其不检查迭代器是否越界, 一般配合 while 和 get_and_add 使用
	 * @return 去掉的指令, 如果未去掉则返回 nullptr
	 */
	Instruction* remove_pre();
	/**
	 * 获取指令并将迭代器自增, 如果获取指令为空则不自增
	 * @return 获取的指令, 如果不存在则返回 nullptr
	 */
	[[nodiscard]] Instruction* get_and_add();
	/**
	 * 获取指令并将迭代器自减, 如果获取指令为空则不自减
	 * @return 获取的指令, 如果不存在则返回 nullptr
	 */
	[[nodiscard]] Instruction* get_and_sub();

private:
	InstructionList* parent_;
	InstructionListNode* current_;
};

/**
 * 指令表的一个视图, 对视图的操作会更新主表, 但是反过来不会. 任何更新视图边界的操作都会破坏视图
 * (例如对只含 phi alloca 的视图, 删除主表的第一个 common instruction)
 */
class InstructionListView
{
	friend class InstructionList;
	friend class InstructionListIterator;

	InstructionListView(InstructionListNode* begin, InstructionListNode* end,
		InstructionList* instructions);
public:
	/**
	 * 获取一个从头开始的正向迭代器
	 * @return 正向迭代器
	 */
	[[nodiscard]] InstructionListIterator begin() const;
	/**
	 * 获取正向迭代器的终止位置
	 * @return 终止位置, 它不能用于进行反向迭代, 请用 rbegin 和 rend
	 */
	[[nodiscard]] const InstructionListNode* end() const;
	/**
	 * 获取一个从尾开始的反向迭代器
	 * @return 反向迭代器
	 */
	[[nodiscard]] InstructionListIterator rbegin() const;
	/**
	 * 获取反向迭代器的终止位置
	 * @return 终止位置, 它不能用于进行正向迭代, 请用 begin 和 end
	 */
	[[nodiscard]] const InstructionListNode* rend() const;
	/**
	 * 去除 iterator 指向的指令, 如果其未指向指令则不去除. 迭代器不会更新, 只要不删除视图边界(它的 end), 视图不会被破坏.
	 * @param iterator 迭代器
	 * @return 被去掉的指令, 它不会被 delete. 如果未去掉任何指令, 返回 nullptr
	 */
	[[nodiscard]] Instruction* erase(const InstructionListIterator& iterator) const;
	/**
	 * 去除 iterator 指向的指令, 如果其未指向指令则不去除. 迭代器不会更新, 只要不删除视图边界(它的 end), 视图不会被破坏.
	 * @param iterator 迭代器
	 * @return 被去掉的指令, 它不会被 delete. 如果未去掉任何指令, 返回 nullptr
	 */
	[[nodiscard]] Instruction* remove(const InstructionListIterator& iterator) const;


	/**
	 * 去除第一个匹配的指令.
	 * @param instruction 要去除的指令
	 * @return 是否去除了指令
	 */
	bool remove_first(const Instruction* instruction) const;
	/**
	 * 去除最后一个匹配的指令.
	 * @param instruction 要去除的指令
	 * @return 是否去除了指令
	 */
	bool remove_last(const Instruction* instruction) const;
	/**
	 * 去除第一个匹配的指令.
	 * @param instruction 要去除的指令
	 * @return 是否去除了指令
	 */
	bool erase_first(const Instruction* instruction) const;
	/**
	 * 去除最后一个匹配的指令.
	 * @param instruction 要去除的指令
	 * @return 是否去除了指令
	 */
	bool erase_last(const Instruction* instruction) const;

private:
	InstructionListNode* begin_;
	InstructionListNode* end_;
	InstructionList* parent_;
};

/**
 * 表示一个指令的列表, 其中 phi 和 alloca 单独放置在列表最前面
 * 删除它不会删除其中指令
 */
class InstructionList
{
	friend class InstructionListView;
	friend class InstructionListReversedIterator;
	friend class InstructionListIterator;

public:
	~InstructionList();
	InstructionList(const InstructionList&) = delete;
	InstructionList(InstructionList&&) = delete;
	InstructionList& operator=(const InstructionList&) = delete;
	InstructionList& operator=(InstructionList&&) = delete;
	/**
	 * 获取一个从头开始的正向迭代器
	 * @return 正向迭代器
	 */
	InstructionListIterator begin();
	/**
	 * 获取正向迭代器的终止位置
	 * @return 终止位置, 它不能用于进行反向迭代, 请用 rbegin 和 rend
	 */
	[[nodiscard]] const InstructionListNode* end() const;
	/**
	 * 获取一个从尾开始的反向迭代器
	 * @return 反向迭代器
	 */
	InstructionListIterator rbegin();
	/**
	 * 获取反向迭代器的终止位置
	 * @return 终止位置, 它不能用于进行正向迭代, 请用 begin 和 end
	 */
	[[nodiscard]] const InstructionListNode* rend() const;

	InstructionList();

	[[nodiscard]] InstructionListView all_instructions() const;
	[[nodiscard]] InstructionListView phi_and_allocas() const;

	[[nodiscard]] bool empty() const;

	[[nodiscard]] int size() const;

	/**
	 * 返回指令列表末尾指令
	 * @return 如果非空返回指令(可能是 phi/alloca 或普通指令)，否则返回 nullptr, 
	 */
	[[nodiscard]] Instruction* back() const;
	/**
	 * 去除 iterator 指向的指令, 如果其未指向指令则不去除. 迭代器和视图不会更新.
	 * @param iterator 迭代器
	 * @return 被去掉的指令, 它不会被 delete. 如果未去掉任何指令, 返回 nullptr
	 */
	Instruction* erase(const InstructionListIterator& iterator);
	/**
	 * 去除 iterator 指向的指令, 如果其未指向指令则不去除. 迭代器和视图不会更新.
	 * @param iterator 迭代器
	 * @return 被去掉的指令, 它不会被 delete. 如果未去掉任何指令, 返回 nullptr
	 */
	Instruction* remove(const InstructionListIterator& iterator);
	/**
	 * 去除最后一个指令
	 * @return 被去掉的指令, 它不会被 delete. 如果未去掉任何指令, 返回 nullptr
	 */
	Instruction* pop_back();
	/**
	 * 作为单纯的容器使用时, 将另一个列表中所有元素添加进来. 不会检查重复和排序.
	 */
	void addAll(const InstructionList& instructions);
	/**
	 * 去除第一个匹配的指令.
	 * @param instruction 要去除的指令
	 * @return 是否去除了指令
	 */
	bool remove_first(const Instruction* instruction);
	/**
	 * 去除最后一个匹配的指令.
	 * @param instruction 要去除的指令
	 * @return 是否去除了指令
	 */
	bool remove_last(const Instruction* instruction);
	/**
	 * 去除第一个匹配的指令.
	 * @param instruction 要去除的指令
	 * @return 是否去除了指令
	 */
	bool erase_first(const Instruction* instruction);
	/**
	 * 去除最后一个匹配的指令.
	 * @param instruction 要去除的指令
	 * @return 是否去除了指令
	 */
	bool erase_last(const Instruction* instruction);

	void emplace_back(Instruction* instruction);

	void emplace_front(Instruction* instruction);

	void emplace_back_common_inst(Instruction* instruction);

	void emplace_back_phi_alloca_inst(Instruction* instruction);

	void emplace_front_phi_alloca_inst(Instruction* instruction);

	void emplace_front_common_inst(Instruction* instruction);

private:
	InstructionListNode* common_inst_begin_;
	InstructionListNode* end_node_;
	int phi_alloca_size_;
	int common_inst_size_;
	/**
	 * @return 被去掉的指令, 它不会被 delete. 如果未去掉任何指令, 返回 nullptr
	 */
	Instruction* erase(const InstructionListNode* node);
};

inline InstructionList::~InstructionList()
{
	while (end_node_->next != end_node_)
	{
		const auto toDel = end_node_->next;
		end_node_->next = toDel->next;
		delete toDel;
	}
	delete end_node_;
}

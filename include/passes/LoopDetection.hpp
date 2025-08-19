#pragma once
#include "Dominators.hpp"
#include "PassManager.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Instruction.hpp"

class LoopDetection;
class BasicBlock;
class Dominators;
class Function;
class Module;

using BBset = std::set<BasicBlock*>;
using BBvec = std::vector<BasicBlock*>;

class Loop
{
	LoopDetection* detect_;
	// attribute:
	// preheader, header, blocks, parent, sub_loops, latches
	BasicBlock* preheader_ = nullptr;
	BasicBlock* header_;

	Loop* parent_ = nullptr;
	std::unordered_set<BasicBlock*> blocks_;
	std::set<BasicBlock*> latches_;
	std::map<BasicBlock*, BasicBlock*> exits_;
	std::vector<Loop*> sub_loops_;
	bool valueInLoop(Value* val) const;

public:
	// 该循环比目标循环深多少
	int depthTo(Loop* l) const;
	// 该循环比目标循环深多少
	int depth() const;
	// A 循环比目标循环深多少
	static int depthTo(Loop* a, Loop* l);

	struct Iterator
	{
		// 循环没有从入口的出边或者循环的出判断的两个变量都在循环内计算
		bool notHaveIterator_;
		// 判断变量是外部变量, 所以实际上可以变成 guard + while(true)
		bool outIterateInsteadOfIn_;
		// 循环具有别的出边
		bool haveOtherExitEdge_;
		// 循环迭代变量的 phi 结果实际上全部由外部变量决定
		bool phiDefinedByOut_;
		Instruction* cmp_;
		Instruction* br_;
		Instruction* iterator_;
		Value* start_;
		Value* end_;
		Instruction* step_;

		Iterator()
		{
			notHaveIterator_ = true;
			outIterateInsteadOfIn_ = false;
			haveOtherExitEdge_ = false;
			phiDefinedByOut_ = false;
			cmp_ = nullptr;
			br_ = nullptr;
			iterator_ = nullptr;
			start_ = nullptr;
			end_ = nullptr;
			step_ = nullptr;
		}

		Iterator(bool outIterateInsteadOfIn, bool haveOtherExitEdge)
		{
			notHaveIterator_ = false;
			outIterateInsteadOfIn_ = outIterateInsteadOfIn;
			haveOtherExitEdge_ = haveOtherExitEdge;
			phiDefinedByOut_ = false;
			cmp_ = nullptr;
			br_ = nullptr;
			iterator_ = nullptr;
			start_ = nullptr;
			end_ = nullptr;
			step_ = nullptr;
		}

		Value* toLoopIterateValue(const Loop* loop) const;
		Value* exitIterateValue(Loop* loop) const;

		BasicBlock* exit(Loop* loop) const;
		BasicBlock* toLoop(const Loop* loop) const;
	};

	Loop(const Loop&) = delete;
	Loop(Loop&&) = delete;
	Loop& operator=(const Loop&) = delete;
	Loop& operator=(Loop&&) = delete;

	Loop(LoopDetection* d, BasicBlock* header) : detect_(d), header_(header)
	{
		blocks_.emplace(header);
	}

	~Loop() = default;
	void add_block(BasicBlock* bb) { blocks_.emplace(bb); }
	[[nodiscard]] BasicBlock* get_header() const { return header_; }
	[[nodiscard]] BasicBlock* get_preheader() const { return preheader_; }
	[[nodiscard]] Loop* get_parent() const { return parent_; }
	void set_parent(Loop* parent) { parent_ = parent; }
	void set_header(BasicBlock* head) { header_ = head; }
	void set_preheader(BasicBlock* bb) { preheader_ = bb; }
	void add_block_casecade(BasicBlock* bb, bool includeSelf);
	void add_sub_loop(Loop* loop) { sub_loops_.push_back(loop); }
	void remove_sub_loop(const Loop* loop);
	std::unordered_set<BasicBlock*>& get_blocks() { return blocks_; }
	[[nodiscard]] bool have(BasicBlock* bb) const { return blocks_.count(bb); }
	std::vector<Loop*>& get_sub_loops() { return sub_loops_; }
	const std::set<BasicBlock*>& get_latches() { return latches_; }
	[[nodiscard]] BasicBlock* get_latch() const;
	void add_latch(BasicBlock* bb) { latches_.insert(bb); }
	void remove_latch(BasicBlock* bb) { latches_.erase(bb); }
	void remove_exit_casecade(BasicBlock* bb);
	void add_exit(BasicBlock* from, BasicBlock* to) { exits_.emplace(from, to); }
	void add_exit_casecade(BasicBlock* from, BasicBlock* to);
	std::map<BasicBlock*, BasicBlock*>& exits() { return exits_; }

	[[nodiscard]] std::string print() const;

	// 尝试获取该 loop 进入退出依赖的迭代变量
	// 只有简单循环 (在 head 有一个出边) 才有这种变量
	Iterator getIterator() const;
};

class LoopDetection : public FuncInfoPass
{
	std::vector<Loop*> loops_;
	// map from header to loop
	std::unordered_map<BasicBlock*, Loop*> bb_to_loop_;
	void discover_loop_and_sub_loops(BasicBlock* bb, BBset& latches,
	                                 Loop* loop, Dominators* dom);

public:
	LoopDetection(const LoopDetection&) = delete;
	LoopDetection(LoopDetection&&) = delete;
	LoopDetection& operator=(const LoopDetection&) = delete;
	LoopDetection& operator=(LoopDetection&&) = delete;

	explicit LoopDetection(PassManager* m, Function* f) : FuncInfoPass(m, f)
	{
	}

	~LoopDetection() override;

	void run() override;
	void print() const;
	Loop* loopOfBlock(BasicBlock* b) const;
	void setLoopOfBlock(BasicBlock* b, Loop* l) { bb_to_loop_[b] = l; }
	std::vector<Loop*>& get_loops() { return loops_; }
	int costOfLatch(Loop* loop, BasicBlock* bb, const Dominators* idoms);
	void collectInnerLoopMessage(Loop* loop, BasicBlock* bb, BasicBlock* preHeader, Loop* innerLoop, Dominators* idoms);
	static void addNewExitTo(Loop* loop, BasicBlock* bb, BasicBlock* out, BasicBlock* preOut);
	// FuncInfo 目前是否是正确的
	void validate();
};

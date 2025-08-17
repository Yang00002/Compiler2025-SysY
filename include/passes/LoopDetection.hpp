#pragma once
#include "Dominators.hpp"
#include "PassManager.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>

class BasicBlock;
class Dominators;
class Function;
class Module;

using BBset = std::set<BasicBlock*>;
using BBvec = std::vector<BasicBlock*>;

class Loop
{
	// attribute:
	// preheader, header, blocks, parent, sub_loops, latches
	BasicBlock* preheader_ = nullptr;
	BasicBlock* header_;

	Loop* parent_ = nullptr;
	std::unordered_set<BasicBlock*> blocks_;
	std::set<BasicBlock*> latches_;
	std::map<BasicBlock*, BasicBlock*> exits_;
	std::vector<Loop*> sub_loops_;

public:
	Loop(const Loop&) = delete;
	Loop(Loop&&) = delete;
	Loop& operator=(const Loop&) = delete;
	Loop& operator=(Loop&&) = delete;

	Loop(BasicBlock* header) : header_(header)
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
	void add_sub_loop(Loop* loop) { sub_loops_.push_back(loop); }
	void remove_sub_loop(const Loop* loop);
	std::unordered_set<BasicBlock*>& get_blocks() { return blocks_; }
	[[nodiscard]] bool have(BasicBlock* bb) const { return blocks_.count(bb); }
	std::vector<Loop*>& get_sub_loops() { return sub_loops_; }
	const std::set<BasicBlock*>& get_latches() { return latches_; }
	void add_latch(BasicBlock* bb) { latches_.insert(bb); }
	void remove_latch(BasicBlock* bb) { latches_.erase(bb); }
	void addExit(BasicBlock* from, BasicBlock* to) { exits_.emplace(from, to); }
	std::map<BasicBlock*, BasicBlock*>& exits() { return exits_; }

	std::string print() const;
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
	std::vector<Loop*>& get_loops() { return loops_; }
	int costOfLatch(Loop* loop, BasicBlock* bb, const Dominators* idoms);
	void collectInnerLoopMessage(Loop* loop, BasicBlock* bb, BasicBlock* preHeader, Loop* innerLoop, Dominators* idoms);
	static void addNewExitTo(Loop* loop, BasicBlock* bb, BasicBlock* out, BasicBlock* preOut);
};

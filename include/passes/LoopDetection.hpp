#pragma once
#include "Dominators.hpp"
#include "PassManager.hpp"
#include <memory>
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

	std::shared_ptr<Loop> parent_ = nullptr;
	BBvec blocks_;
	std::vector<std::shared_ptr<Loop>> sub_loops_;

	std::unordered_set<BasicBlock*> latches_;

public:
	Loop(const Loop&) = delete;
	Loop(Loop&&) = delete;
	Loop& operator=(const Loop&) = delete;
	Loop& operator=(Loop&&) = delete;
	Loop(BasicBlock* header) : header_(header)
	{
		blocks_.push_back(header);
	}

	~Loop() = default;
	void add_block(BasicBlock* bb) { blocks_.push_back(bb); }
	[[nodiscard]] BasicBlock* get_header() const { return header_; }
	[[nodiscard]] BasicBlock* get_preheader() const { return preheader_; }
	std::shared_ptr<Loop> get_parent() { return parent_; }
	void set_parent(const std::shared_ptr<Loop>& parent) { parent_ = parent; }
	void set_preheader(BasicBlock* bb) { preheader_ = bb; }
	void add_sub_loop(const std::shared_ptr<Loop>& loop) { sub_loops_.push_back(loop); }
	const BBvec& get_blocks() { return blocks_; }
	const std::vector<std::shared_ptr<Loop>>& get_sub_loops() { return sub_loops_; }
	const std::unordered_set<BasicBlock*>& get_latches() { return latches_; }
	void add_latch(BasicBlock* bb) { latches_.insert(bb); }
};

class LoopDetection : public Pass
{
	Function* func_;
	std::unique_ptr<Dominators> dominators_;
	std::vector<std::shared_ptr<Loop>> loops_;
	// map from header to loop
	std::unordered_map<BasicBlock*, std::shared_ptr<Loop>> bb_to_loop_;
	void discover_loop_and_sub_loops(BasicBlock* bb, BBset& latches,
	                                 const std::shared_ptr<Loop>& loop);

public:
	LoopDetection(const LoopDetection&) = delete;
	LoopDetection(LoopDetection&&) = delete;
	LoopDetection& operator=(const LoopDetection&) = delete;
	LoopDetection& operator=(LoopDetection&&) = delete;

	explicit LoopDetection(Module* m) : Pass(m), func_(nullptr)
	{
	}

	~LoopDetection() override = default;

	void run() override;
	void run_on_func(Function* f);
	void print() const;
	std::vector<std::shared_ptr<Loop>>& get_loops() { return loops_; }
};

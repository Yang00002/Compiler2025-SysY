#pragma once
#include <vector>

#include "MachineBasicBlock.hpp"
#include "MachineDominator.hpp"
#include "MachineFunction.hpp"
#include "MachinePassManager.hpp"

class MachineLoop
{
	MBasicBlock* header_;

	MachineLoop* parent_ = nullptr;
	DynamicBitset blocks_;
	DynamicBitset latches_;
	std::vector<MachineLoop*> sub_loops_;

public:
	MachineLoop(const MachineLoop&) = delete;
	MachineLoop(MachineLoop&&) = delete;
	MachineLoop& operator=(const MachineLoop&) = delete;
	MachineLoop& operator=(MachineLoop&&) = delete;

	MachineLoop(MBasicBlock* header) : header_(header), blocks_{u2iNegThrow(header->function()->blocks().size())},
	                                   latches_{u2iNegThrow(header->function()->blocks().size())}
	{
		blocks_.set(header->id());
	}

	~MachineLoop() = default;
	void add_block(int bb) { blocks_.set(bb); }
	[[nodiscard]] MBasicBlock* get_header() const { return header_; }
	[[nodiscard]] MachineLoop* get_parent() const { return parent_; }
	void set_parent(MachineLoop* parent) { parent_ = parent; }
	void add_sub_loop(MachineLoop* loop) { sub_loops_.push_back(loop); }
	DynamicBitset& get_blocks() { return blocks_; }
	const std::vector<MachineLoop*>& get_sub_loops() { return sub_loops_; }
	const DynamicBitset& get_latches() { return latches_; }
	void add_latch(int bb) { latches_.set(bb); }
};

class MachineLoopDetection : public MachinePass
{
	MFunction* func_;
	MachineDominators dominators_;
	std::vector<MachineLoop*> loops_;
	// map from header to loop
	std::vector<MachineLoop*> bb_to_loop_;
	void discover_loop_and_sub_loops(const MBasicBlock* bb, const DynamicBitset& latches,
	                                 MachineLoop* loop);
	void removeUnreachable() const;

public:
	MachineLoopDetection(const MachineLoopDetection&) = delete;
	MachineLoopDetection(MachineLoopDetection&&) = delete;
	MachineLoopDetection& operator=(const MachineLoopDetection&) = delete;
	MachineLoopDetection& operator=(MachineLoopDetection&&) = delete;

	explicit MachineLoopDetection(MModule* m) : MachinePass(m), func_(nullptr), dominators_(m)
	{
	}

	~MachineLoopDetection() override;

	void run() override;
	void run_on_func(MFunction* f);
	void print() const;
	std::vector<MachineLoop*>& get_loops() { return loops_; }
};

#pragma once
#include <unordered_set>

#include "MachineInstruction.hpp"
#include "MachinePassManager.hpp"


class MBasicBlock;

class MachineDominators : public MachinePass
{
	MFunction* func_ = nullptr;
	std::vector<MBasicBlock*> idom_{}; // 直接支配
	std::vector<DynamicBitset> dom_frontier_{}; // 支配边界集合
	std::vector<DynamicBitset> dom_tree_succ_blocks_{}; // 支配树中的后继节点

	// 支配树上的dfs序L,R
	std::vector<int> dom_tree_L_;
	std::vector<int> dom_tree_R_;

	std::vector<MBasicBlock*> dom_dfs_order_;
	std::vector<MBasicBlock*> dom_post_order_;

	void create_dom_dfs_order();

public:
	MachineDominators(const MachineDominators&) = delete;
	MachineDominators(MachineDominators&&) = delete;
	MachineDominators& operator=(const MachineDominators&) = delete;
	MachineDominators& operator=(MachineDominators&&) = delete;

	explicit MachineDominators(MModule* m) : MachinePass(m)
	{
	}

	~MachineDominators() override = default;
	void run() override;
	void run_on_func(MFunction* f);

	// functions for getting information
	MBasicBlock* get_idom(const MBasicBlock* bb) const;

	const DynamicBitset& get_dominance_frontier(const MBasicBlock* bb);

	const DynamicBitset& get_dom_tree_succ_blocks(const MBasicBlock* bb);

	// functions for dominance tree
	bool is_dominate(const MBasicBlock* bb1, const MBasicBlock* bb2) const;

	const std::vector<MBasicBlock*>& get_dom_dfs_order();

	const std::vector<MBasicBlock*>& get_dom_post_order();

	MBasicBlock* latestCommonParent(const std::unordered_set<MInstruction*>& childs) const;
};

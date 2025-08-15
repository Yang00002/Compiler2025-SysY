#pragma once

#include "BasicBlock.hpp"
#include "PassManager.hpp"

#include <set>

class Dominators : public Pass
{
	std::unordered_map<BasicBlock*, BasicBlock*> idom_{}; // 直接支配
	std::unordered_map<BasicBlock*, std::set<BasicBlock*>> dom_frontier_{}; // 支配边界集合
	std::unordered_map<BasicBlock*, std::set<BasicBlock*>> dom_tree_succ_blocks_{}; // 支配树中的后继节点

	// 支配树上的dfs序L,R
	std::unordered_map<BasicBlock*, int> dom_tree_L_;
	std::unordered_map<BasicBlock*, int> dom_tree_R_;

	std::unordered_map<Function*, std::vector<BasicBlock*>> dom_dfs_order_;
	std::unordered_map<Function*, std::vector<BasicBlock*>> dom_post_order_;

	void create_dom_dfs_order(Function* f);

public:
	Dominators(const Dominators&) = delete;
	Dominators(Dominators&&) = delete;
	Dominators& operator=(const Dominators&) = delete;
	Dominators& operator=(Dominators&&) = delete;

	explicit Dominators(Module* m) : Pass(m)
	{
	}

	~Dominators() override = default;
	void run() override;
	void run_on_func(Function* f);

	// functions for getting information
	BasicBlock* get_idom(BasicBlock* bb) const;

	const std::set<BasicBlock*>& get_dominance_frontier(BasicBlock* bb);

	const std::set<BasicBlock*>& get_dom_tree_succ_blocks(BasicBlock* bb);

	// print cfg or dominance tree
	static void dump_cfg(Function* f);
	void dump_dominator_tree(Function* f);

	// functions for dominance tree
	bool is_dominate(BasicBlock* bb1, BasicBlock* bb2) const;

	const std::vector<BasicBlock*>& get_dom_dfs_order(Function* function);

	const std::vector<BasicBlock*>& get_dom_post_order(Function* function);

	// for debug
	void print_idom(Function* f) const;
	void print_dominance_frontier(Function* f);
};

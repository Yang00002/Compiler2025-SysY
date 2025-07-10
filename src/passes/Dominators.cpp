#include "Dominators.hpp"
#include "BasicBlock.hpp"
#include "Function.hpp"
#include "Module.hpp"
#include <fstream>
#include <list>
#include <map>
#include <ostream>
#include <set>
#include <vector>
#include <climits>


namespace
{
	class LTDominatorTreeSolver
	{
		std::unordered_map<BasicBlock*, int> fromBlockPtrToBlockId_;
		BasicBlock** fromBlockIdToBlockPtr1_;
		int* edges0_;
		int* idToEdgeIdxs2_;
		int beginBlockId_, blockCount_;
		int *blockIdRankByDFSEnteringOrder1_, *DFSEnteringOrderOfblockId1_, *DFSParent1_;
		int *setParent1_, *mn1_;
		int *idom1_, *sdom1_;
		int edgeLinkingSize_;

		struct EL
		{
			int v, x;
		};

		EL* edgeLinking1_;
		int* edgeLinkingIdx1_;

		void dfs(const int u, int& idx)
		{
			DFSEnteringOrderOfblockId1_[u] = ++idx;
			blockIdRankByDFSEnteringOrder1_[idx] = u;
			for (int i = idToEdgeIdxs2_[u << 1]; i < idToEdgeIdxs2_[(u << 1) + 1]; i++)
			{
				if (const int v = edges0_[i]; !DFSEnteringOrderOfblockId1_[v])
				{
					dfs(v, idx);
					DFSParent1_[v] = u;
				}
			}
		}

		int findSetRootOf(int x)
		{
			if (setParent1_[x] == x)
			{
				return x;
			}
			int tmp = setParent1_[x];
			setParent1_[x] = findSetRootOf(setParent1_[x]);
			if (DFSEnteringOrderOfblockId1_[sdom1_[mn1_[tmp]]] < DFSEnteringOrderOfblockId1_[sdom1_[mn1_[x]]])
			{
				mn1_[x] = mn1_[tmp];
			}
			return setParent1_[x];
		}

		void add(int from, int to)
		{
			edgeLinking1_[++edgeLinkingSize_] = {to, edgeLinkingIdx1_[from]};
			edgeLinkingIdx1_[from] = edgeLinkingSize_;
		}

	public:
		LTDominatorTreeSolver(const LTDominatorTreeSolver&) = delete;
		LTDominatorTreeSolver(LTDominatorTreeSolver&&) = delete;
		LTDominatorTreeSolver& operator=(const LTDominatorTreeSolver&) = delete;
		LTDominatorTreeSolver& operator=(LTDominatorTreeSolver&&) = delete;

		LTDominatorTreeSolver(Function* function)
		{
			bool rm;
			do
			{
				rm = false;
				auto blocks = function->get_basic_blocks();
				for (const auto& block : blocks)
					if (!block->is_entry_block() && block->get_pre_basic_blocks().empty())
					{
						function->remove(block);
						rm = true;
					}
			}
			while (rm);
			blockCount_ = static_cast<int>(function->get_basic_blocks().size());
			fromBlockIdToBlockPtr1_ = new BasicBlock*[blockCount_ + 1];
			int edgeCount = 0;
			int allocateId = 0;
			for (const auto& block : function->get_basic_blocks())
			{
				fromBlockIdToBlockPtr1_[++allocateId] = block;
				fromBlockPtrToBlockId_.emplace(block, allocateId);
				edgeCount += static_cast<int>(block->get_succ_basic_blocks().size());
			}
			edges0_ = new int[edgeCount << 1];
			idToEdgeIdxs2_ = new int[(blockCount_ << 1) + 3];
			allocateId = 0;
			for (int i = 1; i <= blockCount_; i++)
			{
				auto& block = fromBlockIdToBlockPtr1_[i];
				idToEdgeIdxs2_[i << 1] = allocateId;
				for (auto& succ_basic_block : block->get_succ_basic_blocks())
				{
					edges0_[allocateId++] = fromBlockPtrToBlockId_[succ_basic_block];
				}
				idToEdgeIdxs2_[1 + (i << 1)] = allocateId;
				for (auto& succ_basic_block : block->get_pre_basic_blocks())
				{
					edges0_[allocateId++] = fromBlockPtrToBlockId_[succ_basic_block];
				}
			}
			idToEdgeIdxs2_[(blockCount_ << 1) + 2] = allocateId;
			beginBlockId_ = fromBlockPtrToBlockId_[function->get_entry_block()];
			blockIdRankByDFSEnteringOrder1_ = new int[blockCount_ + 1];
			DFSEnteringOrderOfblockId1_ = new int[blockCount_ + 1]{};
			DFSParent1_ = new int[blockCount_ + 1];
			setParent1_ = new int[blockCount_ + 1];
			edgeLinkingSize_ = 0;
			edgeLinking1_ = new EL[blockCount_ + 1];
			edgeLinkingIdx1_ = new int[blockCount_ + 1]{};
			mn1_ = new int[blockCount_ + 1];
			idom1_ = new int[blockCount_ + 1];
			sdom1_ = new int[blockCount_ + 1];
		}

		~LTDominatorTreeSolver()
		{
			delete[] fromBlockIdToBlockPtr1_;
			delete[] edges0_;
			delete[] idToEdgeIdxs2_;
			delete[] blockIdRankByDFSEnteringOrder1_;
			delete[] DFSEnteringOrderOfblockId1_;
			delete[] DFSParent1_;
			delete[] setParent1_;
			delete[] mn1_;
			delete[] idom1_;
			delete[] sdom1_;
		}

		void solve()
		{
			int idx = 0;
			dfs(beginBlockId_, idx);
			for (int i = 1; i <= blockCount_; ++i)
			{
				idom1_[i] = setParent1_[i] = sdom1_[i] = mn1_[i] = i;
			}
			for (int i = blockCount_; i >= 2; --i)
			{
				int u = blockIdRankByDFSEnteringOrder1_[i], res = INT_MAX;
				for (int j = idToEdgeIdxs2_[(u << 1) + 1]; j < idToEdgeIdxs2_[(u << 1) + 2]; j++)
				{
					int v = edges0_[j];
					if (!DFSEnteringOrderOfblockId1_[v])
					{
						continue;
					}
					findSetRootOf(v);
					if (DFSEnteringOrderOfblockId1_[v] < DFSEnteringOrderOfblockId1_[u])
					{
						res = std::min(res, DFSEnteringOrderOfblockId1_[v]);
					}
					else
					{
						res = std::min(res, DFSEnteringOrderOfblockId1_[sdom1_[mn1_[v]]]);
					}
				}
				sdom1_[u] = blockIdRankByDFSEnteringOrder1_[res];
				setParent1_[u] = DFSParent1_[u];
				add(sdom1_[u], u);
				u = DFSParent1_[u];
				for (int j = edgeLinkingIdx1_[u]; j; j = edgeLinking1_[j].x)
				{
					int v = edgeLinking1_[j].v;
					findSetRootOf(v);
					if (sdom1_[mn1_[v]] == u)
					{
						idom1_[v] = u;
					}
					else
					{
						idom1_[v] = mn1_[v];
					}
				}
				edgeLinkingIdx1_[u] = 0;
			}
			for (int i = 2; i <= blockCount_; ++i)
			{
				int u = blockIdRankByDFSEnteringOrder1_[i];
				if (idom1_[u] != sdom1_[u])
				{
					idom1_[u] = idom1_[idom1_[u]];
				}
			}
		}

		void dumpIdom(std::map<BasicBlock*, BasicBlock*>& ret) const
		{
			for (int i = 1; i <= blockCount_; i++)
				ret.emplace(fromBlockIdToBlockPtr1_[i], fromBlockIdToBlockPtr1_[idom1_[i]]);
		}

		void dumpTreeSucc(std::map<BasicBlock*, std::set<BasicBlock*>>& ret) const
		{
			for (int i = 1; i <= blockCount_; i++)
			{
				if (idom1_[i] == i) continue;
				ret[fromBlockIdToBlockPtr1_[idom1_[i]]].emplace(fromBlockIdToBlockPtr1_[i]);
			}
		}

		void dumpFrontier(std::map<BasicBlock*, std::set<BasicBlock*>>& ret) const
		{
			for (int i = 1; i <= blockCount_; i++)
			{
				int id = idom1_[i];
				int preBegin = idToEdgeIdxs2_[(i << 1) + 1];
				int preEnd = idToEdgeIdxs2_[(i << 1) + 2];
				if (preEnd - preBegin > 1)
				{
					for (; preBegin < preEnd; ++preBegin)
					{
						int p = edges0_[preBegin];
						while (p != id)
						{
							ret[fromBlockIdToBlockPtr1_[p]].emplace(fromBlockIdToBlockPtr1_[i]);
							p = idom1_[p];
						}
					}
				}
			}
		}
	};
}

/**
 * @brief 支配器分析的入口函数
 *
 * 遍历模块中的所有函数，对每个非声明的函数执行支配关系分析。
 */
void Dominators::run()
{
	for (auto& f : m_->get_functions())
	{
		run_on_func(f);
	}
}

void Dominators::run_on_func(Function* f)
{
	if (f->is_lib_) return;
	LTDominatorTreeSolver solver{f};
	solver.solve();
	solver.dumpIdom(idom_);
	solver.dumpTreeSucc(dom_tree_succ_blocks_);
	solver.dumpFrontier(dom_frontier_);
	create_dom_dfs_order(f);
}


BasicBlock* Dominators::get_idom(BasicBlock* bb) const { return idom_.at(bb); }

const std::set<BasicBlock*>& Dominators::get_dominance_frontier(BasicBlock* bb)
{
	return dom_frontier_[bb];
}

const std::set<BasicBlock*>& Dominators::get_dom_tree_succ_blocks(BasicBlock* bb)
{
	return dom_tree_succ_blocks_.at(bb);
}

/**
 * @brief 打印函数的直接支配关系
 * @param f 要打印的函数
 *
 * 该函数以可读格式打印函数中所有基本块的直接支配者(immediate dominator)。
 * 输出格式为：
 * 基本块名: 其直接支配者名
 * 如果基本块没有直接支配者(如入口块)，则显示"null"。
 */
void Dominators::print_idom(Function* f) const
{
	f->get_parent()->set_print_name();
	int counter = 0;
	std::map<BasicBlock*, std::string> bb_id;
	for (auto& bb : f->get_basic_blocks())
	{
		if (bb->get_name().empty())
			bb_id[bb] = "bb" + std::to_string(counter);
		else
			bb_id[bb] = bb->get_name();
		counter++;
	}
	printf("Immediate dominance of function %s:\n", f->get_name().c_str());
	for (auto& bb : f->get_basic_blocks())
	{
		std::string output;
		output = bb_id[bb] + ": ";
		if (get_idom(bb))
		{
			output += bb_id[get_idom(bb)];
		}
		else
		{
			output += "null";
		}
		printf("%s\n", output.c_str());
	}
}

/**
 * @brief 打印函数的支配边界信息
 * @param f 要打印的函数
 *
 * 该函数以可读格式打印函数中所有基本块的支配边界(dominance frontier)。
 * 输出格式为：
 * 基本块名: 支配边界中的基本块列表
 * 如果基本块没有支配边界，则显示"null"。
 */
void Dominators::print_dominance_frontier(Function* f)
{
	f->get_parent()->set_print_name();
	int counter = 0;
	std::map<BasicBlock*, std::string> bb_id;
	for (auto& bb : f->get_basic_blocks())
	{
		if (bb->get_name().empty())
			bb_id[bb] = "bb" + std::to_string(counter);
		else
			bb_id[bb] = bb->get_name();
		counter++;
	}
	printf("Dominance Frontier of function %s:\n", f->get_name().c_str());
	for (auto& bb : f->get_basic_blocks())
	{
		std::string output;
		output = bb_id[bb] + ": ";
		if (get_dominance_frontier(bb).empty())
		{
			output += "null";
		}
		else
		{
			bool first = true;
			for (auto df : get_dominance_frontier(bb))
			{
				if (first)
				{
					first = false;
				}
				else
				{
					output += ", ";
				}
				output += bb_id[df];
			}
		}
		printf("%s\n", output.c_str());
	}
}

/**
 * @brief 将函数的控制流图(CFG)导出为图形文件
 * @param f 要导出的函数
 *
 * 该函数生成函数的控制流图的DOT格式描述，并使用graphviz将其转换为PNG图像。
 * 生成两个文件：
 * - {函数名}_cfg.dot：DOT格式的图形描述
 * - {函数名}_cfg.png：可视化的控制流图
 */
void Dominators::dump_cfg(Function* f)
{
	f->get_parent()->set_print_name();
	if (f->is_declaration())
		return;
	std::vector<std::string> edge_set;
	bool has_edges = false;
	for (const auto& bb : f->get_basic_blocks())
	{
		auto& succ_blocks = bb->get_succ_basic_blocks();
		if (!succ_blocks.empty())
			has_edges = true;
		for (const auto succ : succ_blocks)
		{
			edge_set.push_back('\t' + bb->get_name() + "->" + succ->get_name() +
			                   ";\n");
		}
	}
	std::string digraph = "digraph G {\n";
	if (!has_edges && !f->get_basic_blocks().empty())
	{
		// 如果没有边且至少有一个基本块，添加一个自环以显示唯一的基本块
		auto& bb = f->get_basic_blocks().front();
		digraph += '\t' + bb->get_name() + ";\n";
	}
	else
	{
		for (const auto& edge : edge_set)
		{
			digraph += edge;
		}
	}
	digraph += "}\n";
	std::ofstream file_output;
	file_output.open(f->get_name() + "_cfg.dot", std::ios::out);
	file_output << digraph;
	file_output.close();
	const std::string dot_cmd = "dot -Tpng " + f->get_name() + "_cfg.dot" + " -o " +
	                            f->get_name() + "_cfg.png";
	std::system(dot_cmd.c_str());
}

/**
 * @brief 将函数的支配树导出为图形文件
 * @param f 要导出的函数
 *
 * 该函数生成函数的支配树的DOT格式描述，并使用graphviz将其转换为PNG图像。
 * 生成两个文件：
 * - {函数名}_dom_tree.dot：DOT格式的图形描述
 * - {函数名}_dom_tree.png：可视化的支配树
 */
void Dominators::dump_dominator_tree(Function* f)
{
	f->get_parent()->set_print_name();
	if (f->is_declaration())
		return;

	std::vector<std::string> edge_set;
	bool has_edges = false; // 用于检查是否有边存在

	for (auto& b : f->get_basic_blocks())
	{
		if (idom_.find(b) != idom_.end() && idom_[b] != b)
		{
			edge_set.push_back('\t' + idom_[b]->get_name() + "->" +
			                   b->get_name() + ";\n");
			has_edges = true; // 如果存在支配边，标记为 true
		}
	}

	std::string digraph = "digraph G {\n";

	if (!has_edges && !f->get_basic_blocks().empty())
	{
		// 如果没有边且至少有一个基本块，直接添加该块以显示它
		auto& b = f->get_basic_blocks().front();
		digraph += '\t' + b->get_name() + ";\n";
	}
	else
	{
		for (auto& edge : edge_set)
		{
			digraph += edge;
		}
	}

	digraph += "}\n";

	std::ofstream file_output;
	file_output.open(f->get_name() + "_dom_tree.dot", std::ios::out);
	file_output << digraph;
	file_output.close();

	std::string dot_cmd = "dot -Tpng " + f->get_name() + "_dom_tree.dot" +
	                      " -o " + f->get_name() + "_dom_tree.png";
	std::system(dot_cmd.c_str());
}

bool Dominators::is_dominate(BasicBlock* bb1, BasicBlock* bb2) const
{
	return dom_tree_L_.at(bb1) <= dom_tree_L_.at(bb2) &&
	       dom_tree_R_.at(bb1) >= dom_tree_R_.at(bb2);
}

const std::vector<BasicBlock*>& Dominators::get_dom_dfs_order(Function* function)
{
	return dom_dfs_order_[function];
}

const std::vector<BasicBlock*>& Dominators::get_dom_post_order(Function* function)
{
	return dom_post_order_[function];
}


/**
 * @brief 为支配树创建深度优先搜索序
 * @param f 要处理的函数
 *
 * 该函数通过深度优先搜索遍历支配树，为每个基本块分配两个序号：
 * 1. dom_tree_L_：记录DFS首次访问该节点的时间戳
 * 2. dom_tree_R_：记录DFS完成访问该节点子树的时间戳
 *
 * 同时维护：
 * - dom_dfs_order_：按DFS访问顺序记录基本块
 * - dom_post_order_：dom_dfs_order_的逆序
 *
 * 这些序号和顺序可用于快速判断支配关系：
 * 如果节点A支配节点B，则A的L值小于B的L值，且A的R值大于B的R值
 */
void Dominators::create_dom_dfs_order(Function* f)
{
	// 分析得到 f 中各个基本块的支配树上的dfs序L,R
	unsigned int order = 0;
	auto& od = dom_dfs_order_[f];
	std::function<void(BasicBlock*)> dfs = [&](BasicBlock* bb)
	{
		dom_tree_L_[bb] = ++order;
		od.emplace_back(bb);
		for (auto& succ : dom_tree_succ_blocks_[bb])
		{
			dfs(succ);
		}
		dom_tree_R_[bb] = order;
	};
	dfs(f->get_entry_block());
	dom_post_order_[f] = std::vector(od.rbegin(),
	                                 od.rend());
}

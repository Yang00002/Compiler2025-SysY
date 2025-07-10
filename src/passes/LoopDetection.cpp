#include "LoopDetection.hpp"
#include "BasicBlock.hpp"
#include "Dominators.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <set>


LoopDetection::~LoopDetection()
{
	for (const auto& loop : loops_)
	{
		delete loop;
	} 
}

/**
 * @brief 循环检测Pass的主入口函数
 *
 * 该函数执行以下步骤：
 * 1. 创建支配树分析实例
 * 2. 遍历模块中的所有函数
 * 3. 对每个非声明函数执行循环检测
 */
void LoopDetection::run()
{
	dominators_ = std::make_unique<Dominators>(m_);
	for (const auto& f : m_->get_functions())
	{
		if (f->is_declaration())
			continue;
		func_ = f;
		run_on_func(f);
	}
}

/**
 * @brief 发现循环及其子循环
 * @param bb 循环的header块
 * @param latches 循环的回边终点(latch)集合
 * @param loop 当前正在处理的循环对象
 */
void LoopDetection::discover_loop_and_sub_loops(BasicBlock* bb, BBset& latches,
                                                Loop* loop)
{
	// DONE List:
	// 1. 初始化工作表，将所有latch块加入
	// 2. 实现主循环逻辑
	// 3. 处理未分配给任何循环的节点
	// 4. 处理已属于其他循环的节点
	// 5. 建立正确的循环嵌套关系

	BBvec work_list = {latches.begin(), latches.end()}; // 初始化工作表

	while (!work_list.empty())
	{
		// 当工作表非空时继续处理
		auto subbb = work_list.back();
		work_list.pop_back();
		// DONE-1: 处理未分配给任何循环的节点
		if (bb_to_loop_.find(subbb) == bb_to_loop_.end())
		{
			loop->add_block(subbb);
			bb_to_loop_[subbb] = loop;
			for (auto pre : subbb->get_pre_basic_blocks())
			{
				if (dominators_->is_dominate(bb, pre))
				{
					work_list.push_back(pre);
				}
			}
		}
		// DONE-2: 处理已属于其他循环的节点
		else if (bb_to_loop_[subbb] != loop)
		{
			auto subloop = bb_to_loop_[subbb];
			auto parent = subloop->get_parent();
			while (parent != nullptr)
			{
				subloop = parent;
				parent = parent->get_parent();
			}
			if (subloop != loop)
			{
				loop->add_sub_loop(subloop);
				subloop->set_parent(loop);
				for (auto pb : subloop->get_blocks())
				{
					loop->add_block(pb);
				}
				for (auto pre : subloop->get_header()->get_pre_basic_blocks())
				{
					if (dominators_->is_dominate(bb, pre))
					{
						work_list.push_back(pre);
					}
				}
			}
		}
	}
}

/**
 * @brief 对单个函数执行循环检测
 * @param f 要分析的函数
 *
 * 该函数通过以下步骤检测循环：
 * 1. 运行支配树分析
 * 2. 按支配树后序遍历所有基本块
 * 3. 对每个块，检查其前驱是否存在回边
 * 4. 如果存在回边，创建新的循环并：
 *    - 设置循环header
 *    - 添加latch节点
 *    - 发现循环体和子循环
 */
void LoopDetection::run_on_func(Function* f)
{
	dominators_->run_on_func(f);
	for (auto bb : dominators_->get_dom_post_order(f))
	{
		BBset latches;
		for (auto& pred : bb->get_pre_basic_blocks())
		{
			if (dominators_->is_dominate(bb, pred))
			{
				// pred is a back edge
				// pred -> bb , pred is the latch node
				latches.insert(pred);
			}
		}
		if (latches.empty())
		{
			continue;
		}
		// create loop
		auto loop = new Loop{ bb };
		bb_to_loop_[bb] = loop;
		// add latch nodes
		for (auto& latch : latches)
		{
			loop->add_latch(latch);
		}
		loops_.push_back(loop);
		discover_loop_and_sub_loops(bb, latches, loop);
	}
}

/**
 * @brief 打印循环检测的结果
 *
 * 为每个检测到的循环打印：
 * 1. 循环的header块
 * 2. 循环包含的所有基本块
 * 3. 循环的所有子循环
 */
void LoopDetection::print() const
{
	m_->set_print_name();
	std::cout << "Loop Detection Result:" << '\n';
	for (auto& loop : loops_)
	{
		std::cout << "\nLoop header: " << loop->get_header()->get_name()
			<< '\n';
		std::cout << "Loop blocks: ";
		for (auto& bb : loop->get_blocks())
		{
			std::cout << bb->get_name() << " ";
		}
		std::cout << '\n';
		std::cout << "Sub loops: ";
		for (auto& sub_loop : loop->get_sub_loops())
		{
			std::cout << sub_loop->get_header()->get_name() << " ";
		}
		std::cout << '\n';
	}
}

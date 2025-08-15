#include "LoopSimplify.hpp"

#include "LoopDetection.hpp"

#define DEBUG 1
#include "Util.hpp"

using namespace std;

LoopSimplify::LoopSimplify(Module* m)
	: Pass(m), loops_(new LoopDetection{m_}), f_(nullptr)
{
}

LoopSimplify::~LoopSimplify()
{
	delete loops_;
}

void LoopSimplify::run()
{
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_)continue;
		f_ = f;
		loops_->only_run_on_func(f);
		runOnFunc();
	}
}

void LoopSimplify::runOnFunc()
{
	for (auto l : loops_->get_loops())
	{
		if (l->get_parent() == nullptr)
		{
			runOnLoops(l->get_sub_loops());
			runOnLoop(l);
		}
	}
}

void LoopSimplify::runOnLoops(std::vector<Loop*>& loops)
{
	for (auto loop : loops)
	{
		runOnLoops(loop->get_sub_loops());
		runOnLoop(loop);
	}
}

void LoopSimplify::runOnLoop(Loop* loop)
{
	auto head = loop->get_header();

}

BasicBlock* LoopSimplify::preHeader(Loop* loop)
{
	auto header = loop->get_header();
	if (header->get_pre_basic_blocks().size() > 1) return nullptr;
	auto pre = header->get_pre_basic_blocks().front();
	if (pre->get_succ_basic_blocks().size() > 1) return nullptr;
	return pre;
}

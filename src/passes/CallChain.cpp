#include "CallChain.hpp"

#include "PassManager.hpp"

// 生成函数的调用链
// 对非库函数进行排序, 其中没有调用任何函数的叶子函数放在最前面
// 当不能找到任何叶子函数时, 调用链尝试去掉一条边, 并继续运行
// 每个函数还包含添加其到序列时去掉的边的数量
class CallChain final : Pass
{
public:
	std::vector<std::pair<Function*, int>> chained_;

	explicit CallChain(Module* m)
		: Pass(m)
	{
	}

	void run() override;
};

void CallChain::run()
{
}

#pragma once

#include <deque>

#include "FuncInfo.hpp"
#include "PassManager.hpp"

/**
 * 死代码消除
 **/
class DeadCode final : public Pass {
  public:
	explicit DeadCode(Module *m) : Pass(m), func_info(std::make_shared<FuncInfo>(m)) {}

    void run() override;

  private:
    std::shared_ptr<FuncInfo> func_info;
    std::deque<Instruction *> work_list{};
    std::unordered_map<Instruction *, bool> marked{};

    void mark(Function *func);
    void mark(const Instruction *ins);
    bool sweep(Function *func);
    // 删除不可达基本块
    // 不可达基本块的指令删除:
    // 删除可达基本块中 Phi 包括不可达基本块的定值
    // 可达基本块无法被不可达支配 -> 可达基本块中包含不可达的定值一定有 Phi 定值 -> 删除 Phi 定值就删除了所有的定值
	static bool clear_basic_blocks(Function *func);
    // 删除无用的 alloca
    // 无用的 alloca 不存在任何的 load, 其作为函数参数参与函数调用时, 也不存在任何的 load
	bool clear_not_use_allocas(Function* func);
	bool is_critical(Instruction *ins) const;
    void sweep_globally() const;
};

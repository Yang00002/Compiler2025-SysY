#pragma once

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
    int ins_count{0}; // 用以衡量死代码消除的性能
    std::deque<Instruction *> work_list{};
    std::unordered_map<Instruction *, bool> marked{};

    void mark(Function *func);
    void mark(const Instruction *ins);
    bool sweep(Function *func);
	static bool clear_basic_blocks(Function *func);
    bool is_critical(Instruction *ins) const;
    void sweep_globally() const;
};

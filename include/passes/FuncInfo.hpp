#pragma once

#include "PassManager.hpp"

#include <deque>
#include <unordered_map>

class StoreInst;
class LoadInst;
class Instruction;
class Value;
class Function;
class Module;
/**
 * 计算哪些函数是纯函数
 * WARN:
 * 假定所有函数都是纯函数，除非他写入了全局变量、修改了传入的数组、或者直接间接调用了非纯函数
 */
class FuncInfo : public Pass {
  public:
      FuncInfo(Module* m);

    void run() override;

    bool is_pure_function(Function* func) const;

  private:
    std::deque<Function *> worklist;
    std::unordered_map<Function *, bool> is_pure;

    void trivial_mark(Function *func);
    void process(const Function *func);
      static Value *get_first_addr(Value *val);

      static bool is_side_effect_inst(Instruction *inst);
      static bool is_local_load(const LoadInst *inst);
      static bool is_local_store(const StoreInst *inst);
};

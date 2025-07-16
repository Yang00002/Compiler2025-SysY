#pragma once

#include "PassManager.hpp"

/**
 * TODO 函数/基本块内联 还未实现 目前实现的也有 bug, 会导致偶发性的异常
 **/
class Inline final : public Pass {
public:
    explicit Inline(Module* m) : Pass(m) {}

    void run() override;

private:
    /**
     * 去除函数中空的基本块(仅含一条指令, 这条指令有唯一目标)
     * @param function 处理的函数
     */
    static bool removeEmptyBasicBlock(Function* function);
};

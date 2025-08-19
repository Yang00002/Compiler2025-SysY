#pragma once

// 全局变量在函数的使用次数大于等于这个阈值时，它的地址在函数开始时会加载到寄存器中，而非直接寻址
extern int replaceGlobalAddressWithRegisterNeedUseCount;
// alloca 对象的地址在函数的使用次数 * spill 开销大于等于这个阈值时，它的地址在函数开始时会加载到寄存器中，而非直接寻址
extern float replaceAllocaAddressWithRegisterNeedTotalCost;
// alloca 对象的地址在函数的使用次数大于等于这个阈值时，它的地址在函数开始时会加载到寄存器中，而非直接寻址, 这只在不使用 stackOffset 计算 cost 时有效
extern int replaceAllocaAddressWithRegisterNeedUseCount;
// 常量的使用次数 * spill 开销大于等于这个阈值时, 它会被加载到寄存器而非每次使用拼凑
extern float prefillConstantNeedTotalCost;
// 假定每个循环会运行几次, 在循环内的一次使用就相当于循环外的几次使用
extern int useMultiplierPerLoop;
// 全局变量存在寄存器中的地址在寄存器不够时放弃使用寄存器加载地址的优先级, 值越低则优先级越高(全局变量一般使用文字池加载地址)
extern float globalRegisterSpillPriority;
// 大的 alloca 变量存在寄存器中的地址在寄存器不够时放弃使用寄存器加载地址的优先级, 值越低则优先级越高
extern float bigAllocaRegisterSpillPriority;
// 小的 alloca 变量存在寄存器中的地址在寄存器不够时放弃使用寄存器加载地址的优先级, 值越低则优先级越高
extern float smallAllocaRegisterSpillPriority;
// 多大的 alloca 变量被视为大的. 通常越大的变量离 sp 越远(因为栈排序), 使用单指令完成寻址越困难
extern int bigAllocaVariableGate;
// 在溢出之后, 是否使用图着色尝试合并不冲突的 FrameIndex, 以缩减栈的大小
extern bool mergeStackFrameSpilledWithGraphColoring;
// 是否在图着色时首先使用调用者保存的寄存器. 对于较为简单的函数调用可以省去保存寄存器的工作, 但这会造成更大的编译器运行压力.
extern bool useCallerSaveRegsFirst;
// 对于不能用一条 FMOV 拼凑的浮点数, 使用一条 LDR 而非两条 MOV 一条 FMOV 拼凑.
extern bool useLDRInsteadOfMovFMove2CreateFloat;
// 对于要用 MOV 拼凑的整数, 当需要的 MOV 数量大于这个数量, 改为用 LDR.
extern int useLDRInsteadOfMov2CreateIntegerWhenMovCountBiggerThan;
// 对于要用 MOV 拼凑的浮点数, 当需要的 MOV 数量大于这个数量, 改为用 LDR. 如果不用 LDR, 会多一条 FMOV, 所以这个值一般比整数小 1.
extern int useLDRInsteadOfMov2CreateFloatWhenMovCountBiggerThan;
// 栈中传递的参数在寄存器不够时放弃使用寄存器加载地址的优先级, 值越低则优先级越高
extern float fixFrameIndexParameterRegisterSpillPriority;
// 大于多少字节的数组需要对齐到 16 字节(ABI 强制要求大于 8 字节全局数组对齐到 16 字节, 不受该选项控制)
extern int alignTo16NeedBytes;
// 当 memcpy 的字节数小于等于这个选项 x 16 时, 使用内联实现而非调用函数(一般而言这个数字 -4, 指令减少 2 条, 12 的时候是 8 条)
extern int maxCopyInstCountToInlineMemcpy;
// 当 memset 的字节数小于等于这个选项 x 16 时, 使用内联实现而非调用函数(一般而言这个数字 -4, 指令减少 1 条, 8 的时候是 8 条)
extern int maxCopyInstCountToInlineMemclr;
// 当开启该选项时, 对指针偏移量的运算使用 64 位计算(例如 getelement), 否则使用 32 位 (由于没做上游支持, 大概率都达不到预期功能)
extern bool use64BitsMathOperationInPointerOp;
// 是否像普通寄存器一样使用零寄存器, 因此生成的一些指令 ADD W0, WZR, #2 在某些环境下会报错
extern bool useZRRegisterAsCommonRegister;
// 当开启时, 忽略图着色中的部分 ASSERT 检查
extern bool graphColoringWeakNodeCheck;
// 测试 AST, 生成 C 文件
extern bool emitAST;
// 测试 IR, 生成 LLVM 文件
extern bool emitIR;
// 使用栈式分配
extern bool useStack;
// 使用 O1 优化
extern bool o1Optimization;
// 进行运行前检查, 检测目标架构的某些功能是否符合预期
extern bool testArchi;
// 当函数的指令数(无跳转)小于等于该值时(包括 ret), 它会被内联
extern int funcInlineGate;
// 函数的所有函数结束尾声加起来大于等于这个数字, 需要单独开辟一个返回基本块, 而不是将尾声内联到 RET
extern int epilogShouldMerge;
// 推断 srem 的左操作数和结果符号保持相同, 这并不总是有效的, 尤其是当 ar[op % 4], 此时推断 op >= 0, 但是其可能是 -4 的倍数
extern bool dangerousSignalInfer;
// 忽略可能存在的负数组偏移, 这代表不再使用 SXTW 将 getelement 的偏移计算从 32 拓展到 64 位
extern bool ignoreNegativeArrayIndexes;
// 在一个虚拟寄存器需要 spill 时, 首先尝试将它的定值放置到尽可能靠后的地方(这称为 sink) 而非存入栈
extern bool useSinkForVirtualRegister;
// 是否使用算数指令合并
extern bool useBinaryInstMerge;
// 是否尝试合并浮点指令, 例如将 FMUL 和 FSUB 合并为 FMSUB, 这可能导致与分开时不同的结果(由于舍入误差)
extern bool mergeFloatBinaryInst;
// 只有当加减指令的操作数全是寄存器, 才尝试与乘法合并, 这样可以确保合并不会增加指令
extern bool onlyMergeMulAndASWhenASUseAllReg;
// 使用 stackOffset 来计算 spill 的消耗
extern bool useStackOffset2GetspillCost;
// 在 AST 中就进行循环旋转和添加 loop guard
extern bool loopRotateAndAddGuardInAST;
// 即使没有循环不变量, 只要循环旋转可以消除 phi 或 cbr, 就进行旋转
extern bool rotateLoopEvenIfNotHaveInvariant;
// 大于等于这个数量的循环不变量才会导致循环被旋转
extern int invariantNeed2RotateLoop;
// 禁止条件比较变量的循环外提, 因为 i1 外提后还必须 cset 再在比较处用到
extern bool disableCondLICM;
// 禁止条件比较变量的 GVN, 出于 disableCondLICM 同样的原因
extern bool disableCondGVN;
// 使用 sink 并不能就很有效的在少量 spill 下减小寄存器压力(因为使用相同操作数的概率较小), 只有在 spill 大于等于这个数字才使用 sink
extern int useSinkGate;
// 使用浮点寄存器进行 spill, 使用 FMOV 而非 LDR/STR
extern bool useFloatRegAsStack2Spill;
#include "Config.hpp"


// CodeGen

// 全局变量在函数的使用次数大于等于这个阈值时，它的地址在函数开始时会加载到寄存器中，而非直接寻址
int replaceGlobalAddressWithRegisterNeedUseCount = 2;
// alloca 对象的地址在函数的使用次数大于等于这个阈值时，它的地址在函数开始时会加载到寄存器中，而非直接寻址
int replaceAllocaAddressWithRegisterNeedUseCount = 3;
// 假定每个循环会运行几次, 在循环内的一次使用就相当于循环外的几次使用
int useMultiplierPerLoop = 10;
// 全局变量存在寄存器中的地址在寄存器不够时放弃使用寄存器加载地址的优先级, 值越低则优先级越高(全局变量一般使用文字池加载地址)
float globalRegisterSpillPriority = 0.8f;
// 大的 alloca 变量存在寄存器中的地址在寄存器不够时放弃使用寄存器加载地址的优先级, 值越低则优先级越高
float bigAllocaRegisterSpillPriority = 0.8f;
// 小的 alloca 变量存在寄存器中的地址在寄存器不够时放弃使用寄存器加载地址的优先级, 值越低则优先级越高
float smallAllocaRegisterSpillPriority = 0.6f;
// 多大的 alloca 变量被视为大的. 通常越大的变量离 sp 越远(因为栈排序), 使用单指令完成寻址越困难
int bigAllocaVariableGate = 12;
// 在溢出之后, 是否使用图着色尝试合并不冲突的 FrameIndex, 以缩减栈的大小
bool mergeStackFrameSpilledWithGraphColoring = true;
// 是否在图着色时首先使用调用者保存的寄存器. 对于较为简单的函数调用可以省去保存寄存器的工作, 但这会造成更大的编译器运行压力.
bool useCallerSaveRegsFirst = true;
// 对于不能用一条 FMOV 拼凑的浮点数, 使用一条 LDR 而非两条 MOV 一条 FMOV 拼凑.
bool useLDRInsteadOfMovFMove2CreateFloat = true;
// 对于要用 MOV 拼凑的整数, 当需要的 MOV 数量大于这个数量, 改为用 LDR.
int useLDRInsteadOfMov2CreateIntegerWhenMovCountBiggerThan = 2;
// 对于要用 MOV 拼凑的浮点数, 当需要的 MOV 数量大于这个数量, 改为用 LDR. 如果不用 LDR, 会多一条 FMOV, 所以这个值一般比整数小 1.
int useLDRInsteadOfMov2CreateFloatWhenMovCountBiggerThan = 1;
// 栈中传递的参数在寄存器不够时放弃使用寄存器加载地址的优先级, 值越低则优先级越高
float fixFrameIndexParameterRegisterSpillPriority = 0.9f;
// 大于多少字节的数组需要对齐到 16 字节(ABI 强制要求大于 8 字节全局数组对齐到 16 字节, 不受该选项控制)
int alignTo16NeedBytes = 8;
// 当 memcpy 的字节数小于等于这个选项 x 16 时, 使用内联实现而非调用函数(一般而言这个数字 -4, 指令减少 2 条, 12 的时候是 8 条)
int maxCopyInstCountToInlineMemcpy = 12;
// 当 memset 的字节数小于等于这个选项 x 16 时, 使用内联实现而非调用函数(一般而言这个数字 -4, 指令减少 1 条, 8 的时候是 8 条)
int maxCopyInstCountToInlineMemclr = 8;
// 当开启该选项时, 对指针偏移量的运算使用 64 位计算(例如 getelement), 否则使用 32 位 (由于没做上游支持, 大概率都达不到预期功能)
bool use64BitsMathOperationInPointerOp = false;
// 是否像普通寄存器一样使用零寄存器, 因此生成的一些指令 ADD W0, WZR, #2 在某些环境下会报错
bool useZRRegisterAsCommonRegister = false;
// 当开启时, 忽略图着色中的部分 assert 检查
bool graphColoringWeakNodeCheck = false;
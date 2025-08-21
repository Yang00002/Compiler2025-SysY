
# IR

ir 使用 llvm ir，相较于正常 llvm ir instruction，添加了如下命令

```
class MemCpyInst : public BaseInst<MemCpyInst>;
class MemClearInst : public BaseInst<MemClearInst>;
class Nump2CharpInst : public BaseInst<Nump2CharpInst>;
class GlobalFixInst : public BaseInst<GlobalFixInst>;
class MulIntegratedInst : public BaseInst<MulIntegratedInst>;
```

## GlobalFixInst

ir 的全局变量初始化列表使用 `PlainTensor<ConstantValue>*` 表示，gate = 1，例如

```CPP
int a[4][2] = {1, 2, {3}, 4};
```

对应 PlainTensor 段

```
{1, 2, 3, 0, 4}
3
```

对应 llvm ir

```ll
@a = global <{[5 x i32], [3 x i32]}> <{[5 x i32] [i32 1, i32 2, i32 3, i32 0, i32 4], [3 x i32] zeroinitializer}>
```

如果我们需要使用 `@a`, 就会发现 `@a` 的类型是 `<{[5 x i32], [3 x i32]}>` 而非希望的 `[4 x [2 x i32]]`。GlobalFixInst 指令执行转换来修复该问题：

```
%op0 = bitcast <{[5 x i32], [3 x i32]}>* @a to [4 x [2 x i32]]*
```
它仅接收一个全局变量作为操作数。


## Mem 系列

我们还有局部变量初始化列表，其中有一些变量，有一些常量。

```cpp
void f(int b) {
	int a[5][5] = {1, 2, 3, 0, 0, 0, 0, 0, b, 0, 1, 0, b, b, b, b};
}
```

llvm ir 具有 memcpy 和 memset 函数用于这种情况。ir 的大致思路如下：
- 对于初始化列表的一部分，使用 memset 设置为全 0。
- 对于一些非 0 常数，创建一个常全局变量，使用 memcpy 进行复制。
- 对于其它不一致的地方，使用 store 指令设置。

memcpy 和 memset 都接收 i8* 作为参数，所以用 Nump2CharpInst 指令将 float/i8* 转换为 i8*
```
%op6 = bitcast i32* %op5 to i8*
```

初始化充分利用 SIMD 功能，我们的 arm 机器上有 128 比特的寄存器，也就是 4 个 i32 宽度，正好适合 memcpy 和 memset。可以考虑如下消耗模型：

- memcpy 4 i32: 2
- memset 4 i32: 1
- store 变量 1 i32: 2
- store 常量 1 i32: 1

在实际的实现中，将数组 a 4 个一组分块：

- 若一块中全是变量，则使用 store，消耗 8
- 若一块中全是 0，则使用 memset，消耗 1
- 若不满足上述情况，使用 memcpy 填充常量(变量的部分填 0)，store 填充变量，消耗 2 - 8
剩下不能分块的全部用 store，消耗 0 - 6。

对于上面的 a，总消耗是 18。它一共有 25 个元素，一个一个填充需要消耗 30，如果不考虑 memset，只考虑 memcpy，则消耗 28。若不考虑 memcpy，只考虑 memset，则消耗 21。这种算法很好的完成了分配。

MemCpyInst 对应 memcpy，MemClearInst 对应 memset。


## MulIntegratedInst

这用于指令的聚合，例如从 add mul 变为 madd，发生在进入后端前的最后一步 InstructionSelection Pass。

这是可开关的，可以使用 `Config` 中 `useBinaryInstMerge` 开关。

这个指令不能输出为合法的 llvm ir。

## 其它

`IBinaryInst` 添加了 mull 类型操作，它与 mul 相同，但一定有一个操作数是  $1\pm 2^k$，这发生在 Arithmetic Pass，你可以当作 mul 使用。

---

对于多维数组，`GetElementPtr` 通常具有多个索引，目前我们的做法是一开始不生成多个 `GetElementPtr`，而是所有索引合在一起。

这是为了便于 GlobalArrayReverse Pass，如果你不需要这个，可以一开始就分开。
我们会在 GlobalArrayReverse Pass 后紧随的 `GetElementSplit` Pass 将它们分开。

分开的 `GetElementPtr` 将为 LICM 提供部分索引外提的机会，这具有很大益处。

当到了后端时，合并的索引可能包含几个常数，从而可以在编译时计算出来，这个时候合并的索引更有好处，我们在 InstructionSelection Pass 中合并它，这也是我们 IR 的最后一个 Pass。

对于几个作用显著的 Pass 的评估放到 [README](./pass/README.md) 中，那里还给出一个 Pass 列表，列举了达到我们编译器性能所需要的主要 Pass。
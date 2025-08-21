# 通用库


## System

独立了平台特定代码，主要是大小端代码。

```CPP
// 系统是大端还是小端
namespace system_about
{
	extern bool SMALL_END;
	// 逻辑上 8 个元素数组的左端
	extern int LOGICAL_LEFT_END_8;
	// 逻辑上 8 个元素数组的右端
	extern int LOGICAL_RIGHT_END_8;
	// 逻辑上 2 个元素数组的右端
	extern int LOGICAL_RIGHT_END_2;
}
```

提供一组全局变量，以判断系统是大端还是小端。以及一个具有 $k$ 元素数组逻辑上左右端的下标。

**用例:**

在 ast 中，数组使用初始化列表。其中每个元素是 int/float 常量或者变量。

如下结构可以表达这种定义

```
ASTExpression* ar[]
class ASTExpression: ASTNode;
class ASTNumber: ASTExpression;
```

其中常量占用空间是 $24$ 字节，包括：
- ar 中 ASTNode 的指针，$8$ 字节
- ASTNumber 的 vptr，用于支持多态，$8$ 字节
- ASTNumber 的字段，用于存储数据，$8$ 字节

其实上 ASTNumber 还要要 $8$ 字节其它数据，这就总共 $32$ 字节。

但我们其实可以用
```
InitializeValue* ar[]
union InitializeValue
{
	ASTExpression* _expression;
	int _int_value[2];
	float _float_value[2];
	char _segment[8];
};
```
鉴于 $64$ 位机器用户空间的地址总是以 `0000` 开头，这 $4$ 位可以作为区分联合中存储数据类型的标志。我们知道，如果 _expression 左端，也就是 _segment[] 左端是 `0000`，存储的就是指针，否则是其它几种常数。很自然的

```
_segment[0] == 0 // is_pointer
```

问题在于，小端的机器中地址开头的 `0000` 并不是 _segment[0], 而是 _segment[7]，使用逻辑左端 LOGICAL_LEFT_END_8 就可以避免考虑这一些细节。

## Type

给出了一个类型系统。

使用枚举 TypeIDs 标注不同的类型

```CPP
enum class TypeIDs : int8_t
{
	Void, // Void
	Integer, // Integers 32 bit
	Boolean, // Boolean 1 bit
	Function, // Functions
	Array, // Arrays
	ArrayInParameter, // Array In Parameter
	Float, // float 32 bit

	Label, // Labels, e.g., BasicBlock
	Pointer, // 指针类型, 仅在 IR 部分会用到指针类型
	Char
};
```

自然，枚举无法存储类型的附加信息，也无法定义方法，所以它们被封装为类 Type 以便使用。

基本类型：
```CPP
namespace Types
{
	extern Type* VOID;
	extern Type* LABEL;
	extern Type* INT;
	extern Type* BOOL;
	extern Type* FLOAT;
	extern Type* CHAR;
	// 获得 TypeIDs 对应的 simpleType, 对于复合类型的 TypeIDs 返回 nullptr
	Type* simpleType(const TypeIDs& contained);
}
```

基本类型使用 simpleType 定义，它们也被预定义并放在 Types 命名空间中，可以直接引用。

复合类型包括指针，数组和函数类型，可以用对应的 functionType, arrayType 和 pointerType 定义。

为了比较两个类型是否是同一类型，可以直接比较它们的指针。

SysY 的语法仅支持 TypeIDs 中前 $7$ 种类型，其中函数的数组参数类型是 ArrayInParameter，它是 Array 的一个变种，第一维未确定。

在 IR 中，ArrayInParameter 被转换为 Pointer，并且标签引入了 Label 类型，初始化列表填充引入了 Char 类型。

## Tensor


定义了张量，用于初始化列表。

Tensor 可以以初始化列表定义时的结构存储数据。

一个 Tensor 具有：

- 形状，如果是标量就为空
- 默认值，对应初始化列表中省略的值
- 有且仅有一个 TensorData，是其实际存储的数据

TensorData 具有

- 一个数据列表，其元素要么是数据，要么是 TensorData*
- 所属张量，默认值与形状在此维护
- 开始的维度，与所属张量共同决定其形状


如下定义了一个整型张量，它形状是 [4, 3, 2]，也就是 `int[4][3][2]`，其初始值为 0。

```CPP
Tensor<int>* tensor = new Tensor{std::vector{4, 3, 2}, 0};
```

目前其还未存储任何数据，如果你使用 getData 方法，那你将得到其 TensorData，其中数据列表为空。 目前类似于 `int[4][3][2] = {};`。如此得到的 TensorData 开始的维度为 0，也就是其形状为 [4, 3, 2]。如果这个张量有一个 TensorData 起始维度是 1 ，这个 TensorData 形状就是 [3, 2]。

我们可以用 append 添加数据，不过只能在其 TensorData 上操作，你可以认为 Tensor 只存储信息，TensorData 才有数据。

下面插入两个数据

```CPP
TensorData<int>* data = tensor->getData();
data->append(1);
data->append(2);
// 相当于 int[4][3][2] = {1, 2}; // 1 2 0 0 0 0 ...
```

初始化列表可以嵌套，这就是 TensorData 可以嵌套的意义，可以使用 makeSubTensor 向 TensorData 插入一个子张量数据
```CPP
TensorData<int>* data = tensor->getData();
data->append(1);
data->append(2);
TensorData<int>* data2 = data->makeSubTensor();
data2->append(3);
data->append(4);
// 相当于 int[4][3][2] = {1, 2, {3}, 4}; // 1 2 3 0 4 0 0 0 ...
```
插入子张量数据的逻辑与初始化列表中嵌套的逻辑相似。

例如 data 经过上文操作后，相当于分配了 4 int 的空间，还剩下 20 int 空间。其中 data2 分配了 1 int 空间，还剩下 1 int，如果你再往 data2 连续插入两个数据，就会报错。

当使用 makeSubTensor 时，程序会尝试为子张量分配一个尽可能大的空间，其刚好是数组某一维的大小，并且已经分配的空间是将要分配空间的倍数。 

在这个例子中，分配 data2 时，可以分配的空间是 2 int/ 6(= 3 x 2) int，由于目前只分配了 2 int，所以只能为 data2 分配 2 int。

```CPP
TensorData<int>* data = tensor->getData();
data->append(1);
data->append(2);
TensorData<int>* data2 = data->makeSubTensor();
data2->append(3);
data->append(4);
data->append(5);
TensorData<int>* data3 = data->makeSubTensor();
// 相当于 int[4][3][2] = {1, 2, {3}, 4, 5, {}};
```

分配 data3 时，已经分配了 6 int 空间，所以 data3 会分配到 6 int。

如果已经分配的空间不是任何一个维度的倍数，就会分配失败报错。

当拥有 Tensor 后，可以使用其 Iterator 进行迭代

```CPP
for (Tensor<int>::Iterator it = tensor->getIterator(); !it.isEnd(); ++it)
{
	switch (it.getCurrentIterateType())
	{
		case TensorIterateType::SUB_TENSOR_BEGIN:
            auto begin_data = it.getSubTensor();
			break;
		case TensorIterateType::SUB_TENSOR_END:
            auto end_data = it.getSubTensor();
			break;
		case TensorIterateType::VALUE:
            auto value = it.getValue();
			break;
		case TensorIterateType::DEFAULT_VALUES:
			auto [defaultValue, length] = it.getDefaultValues();
			break;
	}
}
// int[4][3][2] = {1, 2, {3}, 4, 5, {}};
// VALUE(1)
// VALUE(2)
// SUB_TENSOR_BEGIN(data1)
// VALUE(3)
// DEFAULT_VALUES(0, 1)
// SUB_TENSOR_END(data1)
// VALUE(4)
// VALUE(5)
// SUB_TENSOR_BEGIN(data2)
// DEFAULT_VALUES(0, 6)
// SUB_TENSOR_END(data2)
// DEFAULT_VALUES(0, 12)
```

基本上迭代时得到数据的顺序与对应初始化列表中的输入顺序一致。

可以将结构化的 Tensor 铺平变为 PlainTensor

```CPP
PlainTensor<int>* plain = tensor->toPlain(1);
int count = plain->segmentCount();
for (int i = 0; i < count; i++) plain->segment(i);
// {1, 2, 3, 0, 4, 5}
// 18
```
无论 Tensor 中数据结构如何，PlainTensor 都会将数据分类为添加的值(不同于默认值的值)和默认值，并且分段存储它们。

上文定义的 Tensor 被铺平为两段，第一段是一段添加的值，是 `1, 2, 3, 0, 4, 5`，第二段是一段默认值，长度为 18 int。

toPlain 方法提供一个参数 gate，只有大于 gate 的默认值才会被单独视为一段，不然会像上文一样加入添加值段中。 例如如果上文 gate 为 0，将得到四段：

```CPP
PlainTensor<int>* plain = tensor->toPlain(0);
int count = plain->segmentCount();
for (int i = 0; i < count; i++) plain->segment(i);
// {1, 2, 3}
// 1
// {4, 5}
// 18
```


## Util

Util 有两个选项可以如下开关：

```C
#define OPEN_ASSERT 0
#define DEBUG 1
#include "Util.hpp"
```

其中 `OPEN_ASSERT` 默认是 1，而 `DEBUG` 默认是 0。

Util 允许用 `ASSERT` 替代 `assert`，当 `OPEN_ASSERT` 关闭时，其中代码将无论如何不会执行，这可以用于进行常规检查，而在真正运行时关闭以提高性能。

`DEBUG` 选项影响一系列 API，它们都是在关闭不会运行。包括：

`LOG(str)` 将其中内容打印到命令行，并且换行

`LOGIF(str, cond)` 同上，但只有满足条件才打印

`RUN(command)` 执行其中的指令，通常是用于检查

`GAP` 打印一条很长的 `======` 线

`PUSH` 使以后每次 `LOG` 都多打一个空格

`POP` 使以后每次 `LOG` 都少打一个空格

`PREPARE_PASS_MSG` 定义在 IR PASS 内，为所有值分配名称

`PASS_SUFFIX` 定义在 IR PASS 内，打印出 IR 的内容并插入分隔符

## Color

定义了一些颜色输出方案，例如使用

```
LOG(color::red("12") + "34")
```

可以打印出红色的 12，这是使用 ANSI 转义实现的。`red` 是预定义的颜色，也可以通过其 API 定义新的颜色。

## DynamicBitset

可以在运行时指定宽度的 bitset，用法见其头文件。



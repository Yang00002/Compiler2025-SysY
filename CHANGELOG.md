# Changelog

### 2025-5-10

#### Features

- 实现了基于 ANTLR4 的词法和语法分析，这基于 SysY 的 EBNF 文法。

### 2025-5-11

#### Features

- 为 ANTLR4 添加了配置和使用指南。

### 2025-5-20

#### Features

- 实现了测试脚本，并配置了它的文档。

### 2025-5-22

#### Features

- IR: 有了IR Builder的接口声明和大部分实现。

- SysYIRGenerator: 完成了一部分语义分析，更换了main.cpp的visitor。


#### Fixed

- SysY.g4:
  - DecimalConst：写反了，非零十进制数在前;
  - unaryOp和addExp：减号字符不对;
  - funcRParams：{',' Exp}可以出现任意次数，也就是( ',' exp )*。
  - lAndExp和lOrExp：修改了文法以支持短路计算。

### 2025-5-27

#### Features

- 实现了 AST
	- AST 已经测试通过所有测试样例

### 2025-6-1

#### Features

- 实现了 IR
	- IR 已经测试通过所有测试样例


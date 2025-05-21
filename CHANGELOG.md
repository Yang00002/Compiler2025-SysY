# Changelog

### 2025-5-10

#### Features

- 实现了基于 ANTLR4 的词法和语法分析，这基于 SysY 的 EBNF 文法。

#### TODO

- 块注释也许会导致报错的行数偏移，这有待验证。
- 需要一个分析器的文档。

### 2025-5-11

#### Features

- 为 ANTLR4 添加了配置和使用指南。

### 2025-5-20

#### Features

- 实现了测试脚本，并配置了它的文档。

### 2025-5-22

#### FIXED or IMPROVED

- SysY.g4:
  - DecimalConst：写反了，非零十进制数在前;
  - unaryOp和addExp：减号字符不对;
  - funcRParams：{',' Exp}可以出现任意次数，也就是( ',' exp )*。
  - lAndExp和lOrExp：修改了文法以支持短路计算。

#### IMPLEMENTED Features

- IR: 有了IR Builder的接口声明和大部分实现。

- SysYIRGenerator: 完成了一部分语义分析，更换了main.cpp的visitor。

#### TODO

- SysYIRGenerator:  
  - 补全语义分析部分；
  - 处理 sylib 库函数的参数；
  - 未区分Int1和Int32，有些地方需要调整，例如：需要添加位扩展指令、将布尔类型的变量改为1位；
  - 验证可用性；
  - 需要错误处理。

- IR: 验证IR库的可用性，可能需要实现更多的类型。

- IR Builder: 生成更多指令，例如srem。

- 输出中间代码。


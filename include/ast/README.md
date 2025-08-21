# AST


AST 类继承关系

```
ASTNode
├── ASTCompUnit
├── ASTDecl
│	├── ASTVarDecl
│	└── ASTFuncDecl
├── ASTStmt
│	├── ASTExpression
│	│   ├── ASTRVal
│	│   ├── ASTNumber
│	│   ├── ASTCast
│	│   ├── ASTMathExp
│	│   ├── ASTLogicExp
│	│   ├── ASTEqual
│	│   ├── ASTRelation
│	│   ├── ASTCall
│	│   ├── ASTNeg
│	│   └── ASTNot
│	├── ASTBlock
│	├── ASTAssign
│	├── ASTIf
│	├── ASTWhile
│	├── ASTBreak
│	├── ASTContinue
│	└── ASTReturn
└── ASTLVal
```

AST 基本上执行所有的语义检查。其树中仅有 ASTFuncDecl, ASTBlock 和 ASTCompUnit 节点存在符号表。

**虽然 sysy 的语言规范并未说明，但其函数参数实际上可以与局部变量同名，因此函数定义也要有符号表。**

AST 具有 toString 函数，可以搜索整棵树，输出等价的 C++ 程序。

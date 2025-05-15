## 使用 ANTLR4 SysY 语法树


### 配置 ANTLR4 运行库

> 测试环境 WSL2 Ubuntu 22.04

运行[脚本](antlr4_install.sh)以自动配置并加入环境变量。

### CMake

```CMAKE
# 指定 CMake 的最低版本
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(SysYCompiler)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# antlr 目录下所有 .h 文件
file(GLOB HEADERS "antlr/*.h")

# antlr 目录下所有 .cpp 文件
file(GLOB SOURCES "antlr/*.cpp")

# 添加源文件
add_executable(SysYCompiler ./src/main.cpp ${SOURCES})

# 链接 antlr4-runtime 库
target_link_libraries(SysYCompiler PRIVATE antlr4-runtime)
```


### 访问语法树

```CPP
#include "../antlr/SysYBaseListener.h"
#include "../antlr/SysYLexer.h"
#include "../antlr/SysYParser.h"
#include <CharStream.h>
#include <iostream>

using namespace std;

class MyListener : public SysYBaseListener {
public:
  void enterCompUnit(SysYParser::CompUnitContext *ctx) { cout << "enter"; }
};

int main() {
  antlr4::ANTLRInputStream inputStream("int main(){}");
  SysYLexer lexer{&inputStream};
  antlr4::CommonTokenStream tokens(&lexer);
  SysYParser parser(&tokens);
  MyListener listener;
  antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, parser.compUnit());
  return 0;
}
```

上图是以 Walker 访问语法树的模式，即从根开始自顶向下访问。

它还有一个 Visit 模式，对应相反的行为

```CPP
#include "../antlr/SysYBaseVisitor.h"
#include "../antlr/SysYLexer.h"
#include "../antlr/SysYParser.h"
#include <CharStream.h>
#include <iostream>
#include <tree/ParseTree.h>
#include <tree/ParseTreeVisitor.h>
#include <tree/ParseTreeWalker.h>

using namespace std;

class MyVisitor : public SysYBaseVisitor {
public:
  std::any visitCompUnit(SysYParser::CompUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  std::any visitDecl(SysYParser::DeclContext *ctx) override {
    return visitChildren(ctx);
  }
};

int main() {
  antlr4::ANTLRInputStream inputStream("int main(){}");
  SysYLexer lexer{&inputStream};
  antlr4::CommonTokenStream tokens(&lexer);
  SysYParser parser(&tokens);
  antlr4::tree::ParseTree *tree = parser.compUnit();
  MyVisitor visitor;
  visitor.visit(tree);
  return 0;
}
```
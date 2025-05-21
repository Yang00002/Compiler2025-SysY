grammar SysY;


//////////////////////////////////语法////////////////////////////////////////////////////

//关键字
INT : 'int';
FLOAT : 'float';
VOID : 'void';
IF : 'if';
ELSE : 'else';
WHILE : 'while';
CONTINUE : 'continue';
BREAK : 'break';
RETURN : 'return';
CONST : 'const';

ASSIGN : '=' ;

ADD : '+' ;
SUB : '-' ;
MUL : '*' ;
DIV : '/' ;
MOD : '%' ;

EQ : '==' ;
NE : '!=' ;
LT : '<' ;
GT : '>' ;
LE : '<=' ;
GE : '>=' ;

OR : '||' ;
AND : '&&' ;
NOT : '!' ;

LPAREN : '(' ;
RPAREN : ')' ;
LBRACK : '[' ;
RBRACK : ']' ;
LBRACE : '{' ;
RBRACE : '}' ;
COMMA : ',' ;

//编译单元
compUnit : (decl | funcDef)+ EOF;

//声明
decl : constDecl | varDecl;

//常量声明
constDecl : 'const' bType constDef ( ',' constDef )* ';';

//基本类型
bType : 'int' | 'float';

//常数定义
constDef : ID ( '[' constExp ']' )* '=' constInitVal;

//常量初值
constInitVal : constExp |  '{' ( constInitVal ( ',' constInitVal )* )? '}';

//变量声明
varDecl : bType varDef ( ',' varDef )* ';';

//变量定义
varDef : ID ( '[' constExp ']' )* | ID ( '[' constExp ']' )* '=' initVal;

//变量初值
initVal : exp | '{' ( initVal ( ',' initVal )* )? '}';

//函数定义
funcDef : funcType ID '(' (funcFParams)? ')' block;

//函数类型
funcType : 'void' | 'int' | 'float';

//函数形参表
funcFParams : funcFParam ( ',' funcFParam )*;

//函数形参
funcFParam : bType ID ('[' ']' ( '[' exp ']' )*)?;

//语句块
block : '{' ( blockItem )* '}';

//语句块项
blockItem : decl | stmt;

//语句
stmt : lVal '=' exp ';'
	| (exp)? ';'
	| block
	| 'if' '(' cond ')' stmt ( 'else' stmt )?
	| 'while' '(' cond ')' stmt
	| 'break' ';'
	| 'continue' ';'
	| 'return' (exp)? ';';

//表达式(注： SysY 表达式是 int/float 型表达式)
exp : addExp;

//条件表达式
cond : lOrExp;

//左值表达式
lVal : ID ('[' exp ']')*;

//基本表达式
primaryExp : '(' exp ')' | lVal | number;

//数值
number : IntConst | FloatConst;

//一元表达式
unaryExp : primaryExp | ID '(' (funcRParams)? ')' | unaryOp unaryExp;

//单目运算符(注： '!'仅出现在条件表达式中) 这是语法分析未检查的 该语法也许不允许 !(a == b)
unaryOp : '+' | '-' | '!';

//函数实参表
funcRParams : exp ( ',' exp )*;

//乘除模表达式
mulExp : unaryExp | mulExp ('*' | '/' | '%') unaryExp;

//加减表达式
addExp : mulExp | addExp ('+' | '-') mulExp;

//关系表达式
relExp : addExp | relExp ('<' | '>' | '<=' | '>=') addExp;

//相等性表达式
eqExp : relExp | eqExp ('==' | '!=') relExp;

//逻辑与表达式
lAndExp : eqExp ( '&&' eqExp )* ;

//逻辑或表达式
lOrExp : lAndExp ( '||' lAndExp )* ;

//常量表达式(注：使用的 ID 必须是常量)
constExp : addExp;

//////////////////////////////////词法////////////////////////////////////////////////////

//标识符
ID : NonDigit (NonDigit | Digit)*;

fragment NonDigit : [a-zA-Z_];

fragment Digit : [0-9];

//整形常量
IntConst : DecimalConst | OctalConst | HexAdecimalConst;

fragment DecimalConst : NonZeroDigit Digit* ;

fragment NonZeroDigit : [1-9];

fragment OctalConst : '0' OctalDigit*;

fragment OctalDigit : [0-7];

fragment HexAdecimalConst : HexAdecimalPrefix HexAdecimalDigit+;

fragment HexAdecimalPrefix : '0'[xX];

fragment HexAdecimalDigit : [0-9a-fA-F];

//浮点常量
FloatConst : DecimalFloatConst | HexadecimalFloatConst;

fragment DecimalFloatConst : (Digit+)? '.' Digit+ ExponentPart?
							| Digit+ '.' ExponentPart?
							| Digit+ ExponentPart;

fragment ExponentPart : [eE] [+-]? Digit+;

fragment HexadecimalFloatConst :  HexAdecimalPrefix (HexAdecimalDigit+)? '.' HexAdecimalDigit+ BinaryExponentPart
								| HexAdecimalConst '.' BinaryExponentPart
								| HexAdecimalConst BinaryExponentPart;

fragment BinaryExponentPart : [pP] [+-]? Digit+;


//////////////////////////////////注释和空白////////////////////////////////////////////////////

//块注释
BlockComment
    : '/*' .*? '*/' -> channel(HIDDEN)
    ;

//行注释
LineComment
    : '//' ~[\r\n]* -> channel(HIDDEN)
    ;

//空白符
Whitespace
    : [ \t]+ -> channel(HIDDEN)
    ;

//换行符
Newline
    : ('\r' '\n'? | '\n') -> channel(HIDDEN)
    ;
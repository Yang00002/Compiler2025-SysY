grammar SysY;


//////////////////////////////////语法////////////////////////////////////////////////////

//编译单元 考虑符号表顺序
compUnit : compDecl+ EOF;
compDecl: ( decl | funcDef );

//声明
decl : constDecl | varDecl;

//常量声明
constDecl : CONST bType constDef ( COMMA constDef )* SEMICO;

//基本类型
bType : INT | FLOAT;

//常数定义
constDef : ID ( LBRACK constExp RBRACK )* ASSIGN constInitVal;

//常量初值
constInitVal : constExp |  LBRACE ( constArrayInitVal ( COMMA constArrayInitVal )* )? RBRACE;

//常量数组初值, 区分标量和数组
constArrayInitVal :  constExp |  LBRACE ( constArrayInitVal ( COMMA constArrayInitVal )* )? RBRACE;

//变量声明
varDecl : bType varDef ( COMMA varDef )* SEMICO;

//变量定义
varDef : ID ( LBRACK constExp RBRACK )* | ID ( LBRACK constExp RBRACK )* ASSIGN initVal;

//变量初值
initVal : exp | LBRACE ( arrayInitVal ( COMMA arrayInitVal )* )? RBRACE;

//变量数组初值
arrayInitVal : exp | LBRACE ( arrayInitVal ( COMMA arrayInitVal )* )? RBRACE;

//函数定义
funcDef : funcType ID LPAREN (funcFParams)? RPAREN block;

//函数类型
funcType : VOID | INT | FLOAT;

//函数形参表
funcFParams : funcFParam ( COMMA funcFParam )*;

//函数形参
funcFParam : bType ID (LBRACK RBRACK ( LBRACK exp RBRACK )*)?;

//语句块
block : LBRACE ( blockItem )* RBRACE;

//语句块项
blockItem : decl | stmt;

//语句
stmt : lVal ASSIGN exp SEMICO
	| (exp)? SEMICO
	| block
	| IF LPAREN cond RPAREN stmt ( ELSE stmt )?
	| WHILE LPAREN cond RPAREN stmt
	| BREAK SEMICO
	| CONTINUE SEMICO
	| RETURN (exp)? SEMICO;

//表达式(注： SysY 表达式是 int/float 型表达式)
exp : addExp;

//条件表达式
cond : lOrExp;

//左值表达式
lVal : ID (LBRACK exp RBRACK)*;

//基本表达式
primaryExp : LPAREN exp RPAREN | lVal | number;

//数值
number : IntConst | FloatConst;

//一元表达式
unaryExp : primaryExp | ID LPAREN (funcRParams)? rParen | (ADD | SUB | NOT) unaryExp;

//函数参数的右括号
rParen : RPAREN;

//函数实参表
funcRParams : exp ( COMMA exp )*;

//乘除模表达式
mulExp : unaryExp | mulExp (MUL | DIV | MOD) unaryExp;

//加减表达式
addExp : mulExp | addExp (ADD | SUB) mulExp;

//关系表达式
relExp : addExp | relExp (LT | GT | LE | GE) addExp;

//相等性表达式
eqExp : relExp | eqExp (EQ | NE) relExp;

//逻辑与表达式
lAndExp : eqExp ( AND eqExp )* ;

//逻辑或表达式
lOrExp : lAndExp ( OR lAndExp )* ;

//常量表达式(注：使用的 ID 必须是常量)
constExp : addExp;

//////////////////////////////////词法////////////////////////////////////////////////////


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
COMMA : ','  ;
SEMICO : ';' ;

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
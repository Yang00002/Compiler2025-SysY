
// Generated from SysY.g4 by ANTLR 4.13.2


#include "SysYListener.h"
#include "SysYVisitor.h"

#include "SysYParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct SysYParserStaticData final {
  SysYParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  SysYParserStaticData(const SysYParserStaticData&) = delete;
  SysYParserStaticData(SysYParserStaticData&&) = delete;
  SysYParserStaticData& operator=(const SysYParserStaticData&) = delete;
  SysYParserStaticData& operator=(SysYParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag sysyParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<SysYParserStaticData> sysyParserStaticData = nullptr;

void sysyParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (sysyParserStaticData != nullptr) {
    return;
  }
#else
  assert(sysyParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<SysYParserStaticData>(
    std::vector<std::string>{
      "compUnit", "compDecl", "decl", "constDecl", "bType", "constDef", 
      "constInitVal", "constArrayInitVal", "varDecl", "varDef", "initVal", 
      "arrayInitVal", "funcDef", "funcType", "funcFParams", "funcFParam", 
      "block", "blockItem", "stmt", "exp", "cond", "lVal", "primaryExp", 
      "number", "unaryExp", "funcRParams", "mulExp", "addExp", "relExp", 
      "eqExp", "lAndExp", "lOrExp", "constExp"
    },
    std::vector<std::string>{
      "", "'int'", "'float'", "'void'", "'if'", "'else'", "'while'", "'continue'", 
      "'break'", "'return'", "'const'", "'='", "'+'", "'-'", "'*'", "'/'", 
      "'%'", "'=='", "'!='", "'<'", "'>'", "'<='", "'>='", "'||'", "'&&'", 
      "'!'", "'('", "')'", "'['", "']'", "'{'", "'}'", "','", "';'"
    },
    std::vector<std::string>{
      "", "INT", "FLOAT", "VOID", "IF", "ELSE", "WHILE", "CONTINUE", "BREAK", 
      "RETURN", "CONST", "ASSIGN", "ADD", "SUB", "MUL", "DIV", "MOD", "EQ", 
      "NE", "LT", "GT", "LE", "GE", "OR", "AND", "NOT", "LPAREN", "RPAREN", 
      "LBRACK", "RBRACK", "LBRACE", "RBRACE", "COMMA", "SEMICO", "ID", "IntConst", 
      "FloatConst", "BlockComment", "LineComment", "Whitespace", "Newline"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,40,392,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,1,0,4,0,68,8,0,11,0,12,0,69,
  	1,0,1,0,1,1,1,1,3,1,76,8,1,1,2,1,2,3,2,80,8,2,1,3,1,3,1,3,1,3,1,3,5,3,
  	87,8,3,10,3,12,3,90,9,3,1,3,1,3,1,4,1,4,1,5,1,5,1,5,1,5,1,5,5,5,101,8,
  	5,10,5,12,5,104,9,5,1,5,1,5,1,5,1,6,1,6,1,6,1,6,1,6,5,6,114,8,6,10,6,
  	12,6,117,9,6,3,6,119,8,6,1,6,3,6,122,8,6,1,7,1,7,1,7,1,7,1,7,5,7,129,
  	8,7,10,7,12,7,132,9,7,3,7,134,8,7,1,7,3,7,137,8,7,1,8,1,8,1,8,1,8,5,8,
  	143,8,8,10,8,12,8,146,9,8,1,8,1,8,1,9,1,9,1,9,1,9,1,9,5,9,155,8,9,10,
  	9,12,9,158,9,9,1,9,1,9,1,9,1,9,1,9,5,9,165,8,9,10,9,12,9,168,9,9,1,9,
  	1,9,3,9,172,8,9,1,10,1,10,1,10,1,10,1,10,5,10,179,8,10,10,10,12,10,182,
  	9,10,3,10,184,8,10,1,10,3,10,187,8,10,1,11,1,11,1,11,1,11,1,11,5,11,194,
  	8,11,10,11,12,11,197,9,11,3,11,199,8,11,1,11,3,11,202,8,11,1,12,1,12,
  	1,12,1,12,3,12,208,8,12,1,12,1,12,1,12,1,13,1,13,1,14,1,14,1,14,5,14,
  	218,8,14,10,14,12,14,221,9,14,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,
  	5,15,231,8,15,10,15,12,15,234,9,15,3,15,236,8,15,1,16,1,16,5,16,240,8,
  	16,10,16,12,16,243,9,16,1,16,1,16,1,17,1,17,3,17,249,8,17,1,18,1,18,1,
  	18,1,18,1,18,1,18,3,18,257,8,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,
  	18,1,18,3,18,268,8,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,
  	18,1,18,1,18,3,18,282,8,18,1,18,3,18,285,8,18,1,19,1,19,1,20,1,20,1,21,
  	1,21,1,21,1,21,1,21,5,21,296,8,21,10,21,12,21,299,9,21,1,22,1,22,1,22,
  	1,22,1,22,1,22,3,22,307,8,22,1,23,1,23,1,24,1,24,1,24,1,24,3,24,315,8,
  	24,1,24,1,24,1,24,3,24,320,8,24,1,25,1,25,1,25,5,25,325,8,25,10,25,12,
  	25,328,9,25,1,26,1,26,1,26,1,26,1,26,1,26,5,26,336,8,26,10,26,12,26,339,
  	9,26,1,27,1,27,1,27,1,27,1,27,1,27,5,27,347,8,27,10,27,12,27,350,9,27,
  	1,28,1,28,1,28,1,28,1,28,1,28,5,28,358,8,28,10,28,12,28,361,9,28,1,29,
  	1,29,1,29,1,29,1,29,1,29,5,29,369,8,29,10,29,12,29,372,9,29,1,30,1,30,
  	1,30,5,30,377,8,30,10,30,12,30,380,9,30,1,31,1,31,1,31,5,31,385,8,31,
  	10,31,12,31,388,9,31,1,32,1,32,1,32,0,4,52,54,56,58,33,0,2,4,6,8,10,12,
  	14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,
  	60,62,64,0,8,1,0,1,2,1,0,1,3,1,0,35,36,2,0,12,13,25,25,1,0,14,16,1,0,
  	12,13,1,0,19,22,1,0,17,18,408,0,67,1,0,0,0,2,75,1,0,0,0,4,79,1,0,0,0,
  	6,81,1,0,0,0,8,93,1,0,0,0,10,95,1,0,0,0,12,121,1,0,0,0,14,136,1,0,0,0,
  	16,138,1,0,0,0,18,171,1,0,0,0,20,186,1,0,0,0,22,201,1,0,0,0,24,203,1,
  	0,0,0,26,212,1,0,0,0,28,214,1,0,0,0,30,222,1,0,0,0,32,237,1,0,0,0,34,
  	248,1,0,0,0,36,284,1,0,0,0,38,286,1,0,0,0,40,288,1,0,0,0,42,290,1,0,0,
  	0,44,306,1,0,0,0,46,308,1,0,0,0,48,319,1,0,0,0,50,321,1,0,0,0,52,329,
  	1,0,0,0,54,340,1,0,0,0,56,351,1,0,0,0,58,362,1,0,0,0,60,373,1,0,0,0,62,
  	381,1,0,0,0,64,389,1,0,0,0,66,68,3,2,1,0,67,66,1,0,0,0,68,69,1,0,0,0,
  	69,67,1,0,0,0,69,70,1,0,0,0,70,71,1,0,0,0,71,72,5,0,0,1,72,1,1,0,0,0,
  	73,76,3,4,2,0,74,76,3,24,12,0,75,73,1,0,0,0,75,74,1,0,0,0,76,3,1,0,0,
  	0,77,80,3,6,3,0,78,80,3,16,8,0,79,77,1,0,0,0,79,78,1,0,0,0,80,5,1,0,0,
  	0,81,82,5,10,0,0,82,83,3,8,4,0,83,88,3,10,5,0,84,85,5,32,0,0,85,87,3,
  	10,5,0,86,84,1,0,0,0,87,90,1,0,0,0,88,86,1,0,0,0,88,89,1,0,0,0,89,91,
  	1,0,0,0,90,88,1,0,0,0,91,92,5,33,0,0,92,7,1,0,0,0,93,94,7,0,0,0,94,9,
  	1,0,0,0,95,102,5,34,0,0,96,97,5,28,0,0,97,98,3,64,32,0,98,99,5,29,0,0,
  	99,101,1,0,0,0,100,96,1,0,0,0,101,104,1,0,0,0,102,100,1,0,0,0,102,103,
  	1,0,0,0,103,105,1,0,0,0,104,102,1,0,0,0,105,106,5,11,0,0,106,107,3,12,
  	6,0,107,11,1,0,0,0,108,122,3,64,32,0,109,118,5,30,0,0,110,115,3,14,7,
  	0,111,112,5,32,0,0,112,114,3,14,7,0,113,111,1,0,0,0,114,117,1,0,0,0,115,
  	113,1,0,0,0,115,116,1,0,0,0,116,119,1,0,0,0,117,115,1,0,0,0,118,110,1,
  	0,0,0,118,119,1,0,0,0,119,120,1,0,0,0,120,122,5,31,0,0,121,108,1,0,0,
  	0,121,109,1,0,0,0,122,13,1,0,0,0,123,137,3,64,32,0,124,133,5,30,0,0,125,
  	130,3,14,7,0,126,127,5,32,0,0,127,129,3,14,7,0,128,126,1,0,0,0,129,132,
  	1,0,0,0,130,128,1,0,0,0,130,131,1,0,0,0,131,134,1,0,0,0,132,130,1,0,0,
  	0,133,125,1,0,0,0,133,134,1,0,0,0,134,135,1,0,0,0,135,137,5,31,0,0,136,
  	123,1,0,0,0,136,124,1,0,0,0,137,15,1,0,0,0,138,139,3,8,4,0,139,144,3,
  	18,9,0,140,141,5,32,0,0,141,143,3,18,9,0,142,140,1,0,0,0,143,146,1,0,
  	0,0,144,142,1,0,0,0,144,145,1,0,0,0,145,147,1,0,0,0,146,144,1,0,0,0,147,
  	148,5,33,0,0,148,17,1,0,0,0,149,156,5,34,0,0,150,151,5,28,0,0,151,152,
  	3,64,32,0,152,153,5,29,0,0,153,155,1,0,0,0,154,150,1,0,0,0,155,158,1,
  	0,0,0,156,154,1,0,0,0,156,157,1,0,0,0,157,172,1,0,0,0,158,156,1,0,0,0,
  	159,166,5,34,0,0,160,161,5,28,0,0,161,162,3,64,32,0,162,163,5,29,0,0,
  	163,165,1,0,0,0,164,160,1,0,0,0,165,168,1,0,0,0,166,164,1,0,0,0,166,167,
  	1,0,0,0,167,169,1,0,0,0,168,166,1,0,0,0,169,170,5,11,0,0,170,172,3,20,
  	10,0,171,149,1,0,0,0,171,159,1,0,0,0,172,19,1,0,0,0,173,187,3,38,19,0,
  	174,183,5,30,0,0,175,180,3,22,11,0,176,177,5,32,0,0,177,179,3,22,11,0,
  	178,176,1,0,0,0,179,182,1,0,0,0,180,178,1,0,0,0,180,181,1,0,0,0,181,184,
  	1,0,0,0,182,180,1,0,0,0,183,175,1,0,0,0,183,184,1,0,0,0,184,185,1,0,0,
  	0,185,187,5,31,0,0,186,173,1,0,0,0,186,174,1,0,0,0,187,21,1,0,0,0,188,
  	202,3,38,19,0,189,198,5,30,0,0,190,195,3,22,11,0,191,192,5,32,0,0,192,
  	194,3,22,11,0,193,191,1,0,0,0,194,197,1,0,0,0,195,193,1,0,0,0,195,196,
  	1,0,0,0,196,199,1,0,0,0,197,195,1,0,0,0,198,190,1,0,0,0,198,199,1,0,0,
  	0,199,200,1,0,0,0,200,202,5,31,0,0,201,188,1,0,0,0,201,189,1,0,0,0,202,
  	23,1,0,0,0,203,204,3,26,13,0,204,205,5,34,0,0,205,207,5,26,0,0,206,208,
  	3,28,14,0,207,206,1,0,0,0,207,208,1,0,0,0,208,209,1,0,0,0,209,210,5,27,
  	0,0,210,211,3,32,16,0,211,25,1,0,0,0,212,213,7,1,0,0,213,27,1,0,0,0,214,
  	219,3,30,15,0,215,216,5,32,0,0,216,218,3,30,15,0,217,215,1,0,0,0,218,
  	221,1,0,0,0,219,217,1,0,0,0,219,220,1,0,0,0,220,29,1,0,0,0,221,219,1,
  	0,0,0,222,223,3,8,4,0,223,235,5,34,0,0,224,225,5,28,0,0,225,232,5,29,
  	0,0,226,227,5,28,0,0,227,228,3,38,19,0,228,229,5,29,0,0,229,231,1,0,0,
  	0,230,226,1,0,0,0,231,234,1,0,0,0,232,230,1,0,0,0,232,233,1,0,0,0,233,
  	236,1,0,0,0,234,232,1,0,0,0,235,224,1,0,0,0,235,236,1,0,0,0,236,31,1,
  	0,0,0,237,241,5,30,0,0,238,240,3,34,17,0,239,238,1,0,0,0,240,243,1,0,
  	0,0,241,239,1,0,0,0,241,242,1,0,0,0,242,244,1,0,0,0,243,241,1,0,0,0,244,
  	245,5,31,0,0,245,33,1,0,0,0,246,249,3,4,2,0,247,249,3,36,18,0,248,246,
  	1,0,0,0,248,247,1,0,0,0,249,35,1,0,0,0,250,251,3,42,21,0,251,252,5,11,
  	0,0,252,253,3,38,19,0,253,254,5,33,0,0,254,285,1,0,0,0,255,257,3,38,19,
  	0,256,255,1,0,0,0,256,257,1,0,0,0,257,258,1,0,0,0,258,285,5,33,0,0,259,
  	285,3,32,16,0,260,261,5,4,0,0,261,262,5,26,0,0,262,263,3,40,20,0,263,
  	264,5,27,0,0,264,267,3,36,18,0,265,266,5,5,0,0,266,268,3,36,18,0,267,
  	265,1,0,0,0,267,268,1,0,0,0,268,285,1,0,0,0,269,270,5,6,0,0,270,271,5,
  	26,0,0,271,272,3,40,20,0,272,273,5,27,0,0,273,274,3,36,18,0,274,285,1,
  	0,0,0,275,276,5,8,0,0,276,285,5,33,0,0,277,278,5,7,0,0,278,285,5,33,0,
  	0,279,281,5,9,0,0,280,282,3,38,19,0,281,280,1,0,0,0,281,282,1,0,0,0,282,
  	283,1,0,0,0,283,285,5,33,0,0,284,250,1,0,0,0,284,256,1,0,0,0,284,259,
  	1,0,0,0,284,260,1,0,0,0,284,269,1,0,0,0,284,275,1,0,0,0,284,277,1,0,0,
  	0,284,279,1,0,0,0,285,37,1,0,0,0,286,287,3,54,27,0,287,39,1,0,0,0,288,
  	289,3,62,31,0,289,41,1,0,0,0,290,297,5,34,0,0,291,292,5,28,0,0,292,293,
  	3,38,19,0,293,294,5,29,0,0,294,296,1,0,0,0,295,291,1,0,0,0,296,299,1,
  	0,0,0,297,295,1,0,0,0,297,298,1,0,0,0,298,43,1,0,0,0,299,297,1,0,0,0,
  	300,301,5,26,0,0,301,302,3,38,19,0,302,303,5,27,0,0,303,307,1,0,0,0,304,
  	307,3,42,21,0,305,307,3,46,23,0,306,300,1,0,0,0,306,304,1,0,0,0,306,305,
  	1,0,0,0,307,45,1,0,0,0,308,309,7,2,0,0,309,47,1,0,0,0,310,320,3,44,22,
  	0,311,312,5,34,0,0,312,314,5,26,0,0,313,315,3,50,25,0,314,313,1,0,0,0,
  	314,315,1,0,0,0,315,316,1,0,0,0,316,320,5,27,0,0,317,318,7,3,0,0,318,
  	320,3,48,24,0,319,310,1,0,0,0,319,311,1,0,0,0,319,317,1,0,0,0,320,49,
  	1,0,0,0,321,326,3,38,19,0,322,323,5,32,0,0,323,325,3,38,19,0,324,322,
  	1,0,0,0,325,328,1,0,0,0,326,324,1,0,0,0,326,327,1,0,0,0,327,51,1,0,0,
  	0,328,326,1,0,0,0,329,330,6,26,-1,0,330,331,3,48,24,0,331,337,1,0,0,0,
  	332,333,10,1,0,0,333,334,7,4,0,0,334,336,3,48,24,0,335,332,1,0,0,0,336,
  	339,1,0,0,0,337,335,1,0,0,0,337,338,1,0,0,0,338,53,1,0,0,0,339,337,1,
  	0,0,0,340,341,6,27,-1,0,341,342,3,52,26,0,342,348,1,0,0,0,343,344,10,
  	1,0,0,344,345,7,5,0,0,345,347,3,52,26,0,346,343,1,0,0,0,347,350,1,0,0,
  	0,348,346,1,0,0,0,348,349,1,0,0,0,349,55,1,0,0,0,350,348,1,0,0,0,351,
  	352,6,28,-1,0,352,353,3,54,27,0,353,359,1,0,0,0,354,355,10,1,0,0,355,
  	356,7,6,0,0,356,358,3,54,27,0,357,354,1,0,0,0,358,361,1,0,0,0,359,357,
  	1,0,0,0,359,360,1,0,0,0,360,57,1,0,0,0,361,359,1,0,0,0,362,363,6,29,-1,
  	0,363,364,3,56,28,0,364,370,1,0,0,0,365,366,10,1,0,0,366,367,7,7,0,0,
  	367,369,3,56,28,0,368,365,1,0,0,0,369,372,1,0,0,0,370,368,1,0,0,0,370,
  	371,1,0,0,0,371,59,1,0,0,0,372,370,1,0,0,0,373,378,3,58,29,0,374,375,
  	5,24,0,0,375,377,3,58,29,0,376,374,1,0,0,0,377,380,1,0,0,0,378,376,1,
  	0,0,0,378,379,1,0,0,0,379,61,1,0,0,0,380,378,1,0,0,0,381,386,3,60,30,
  	0,382,383,5,23,0,0,383,385,3,60,30,0,384,382,1,0,0,0,385,388,1,0,0,0,
  	386,384,1,0,0,0,386,387,1,0,0,0,387,63,1,0,0,0,388,386,1,0,0,0,389,390,
  	3,54,27,0,390,65,1,0,0,0,42,69,75,79,88,102,115,118,121,130,133,136,144,
  	156,166,171,180,183,186,195,198,201,207,219,232,235,241,248,256,267,281,
  	284,297,306,314,319,326,337,348,359,370,378,386
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  sysyParserStaticData = std::move(staticData);
}

}

SysYParser::SysYParser(TokenStream *input) : SysYParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

SysYParser::SysYParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  SysYParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *sysyParserStaticData->atn, sysyParserStaticData->decisionToDFA, sysyParserStaticData->sharedContextCache, options);
}

SysYParser::~SysYParser() {
  delete _interpreter;
}

const atn::ATN& SysYParser::getATN() const {
  return *sysyParserStaticData->atn;
}

std::string SysYParser::getGrammarFileName() const {
  return "SysY.g4";
}

const std::vector<std::string>& SysYParser::getRuleNames() const {
  return sysyParserStaticData->ruleNames;
}

const dfa::Vocabulary& SysYParser::getVocabulary() const {
  return sysyParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView SysYParser::getSerializedATN() const {
  return sysyParserStaticData->serializedATN;
}


//----------------- CompUnitContext ------------------------------------------------------------------

SysYParser::CompUnitContext::CompUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::CompUnitContext::EOF() {
  return getToken(SysYParser::EOF, 0);
}

std::vector<SysYParser::CompDeclContext *> SysYParser::CompUnitContext::compDecl() {
  return getRuleContexts<SysYParser::CompDeclContext>();
}

SysYParser::CompDeclContext* SysYParser::CompUnitContext::compDecl(size_t i) {
  return getRuleContext<SysYParser::CompDeclContext>(i);
}


size_t SysYParser::CompUnitContext::getRuleIndex() const {
  return SysYParser::RuleCompUnit;
}

void SysYParser::CompUnitContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompUnit(this);
}

void SysYParser::CompUnitContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompUnit(this);
}


std::any SysYParser::CompUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCompUnit(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::CompUnitContext* SysYParser::compUnit() {
  CompUnitContext *_localctx = _tracker.createInstance<CompUnitContext>(_ctx, getState());
  enterRule(_localctx, 0, SysYParser::RuleCompUnit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(67); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(66);
      compDecl();
      setState(69); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1038) != 0));
    setState(71);
    match(SysYParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CompDeclContext ------------------------------------------------------------------

SysYParser::CompDeclContext::CompDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::DeclContext* SysYParser::CompDeclContext::decl() {
  return getRuleContext<SysYParser::DeclContext>(0);
}

SysYParser::FuncDefContext* SysYParser::CompDeclContext::funcDef() {
  return getRuleContext<SysYParser::FuncDefContext>(0);
}


size_t SysYParser::CompDeclContext::getRuleIndex() const {
  return SysYParser::RuleCompDecl;
}

void SysYParser::CompDeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompDecl(this);
}

void SysYParser::CompDeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompDecl(this);
}


std::any SysYParser::CompDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCompDecl(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::CompDeclContext* SysYParser::compDecl() {
  CompDeclContext *_localctx = _tracker.createInstance<CompDeclContext>(_ctx, getState());
  enterRule(_localctx, 2, SysYParser::RuleCompDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(75);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
    case 1: {
      setState(73);
      decl();
      break;
    }

    case 2: {
      setState(74);
      funcDef();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclContext ------------------------------------------------------------------

SysYParser::DeclContext::DeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::ConstDeclContext* SysYParser::DeclContext::constDecl() {
  return getRuleContext<SysYParser::ConstDeclContext>(0);
}

SysYParser::VarDeclContext* SysYParser::DeclContext::varDecl() {
  return getRuleContext<SysYParser::VarDeclContext>(0);
}


size_t SysYParser::DeclContext::getRuleIndex() const {
  return SysYParser::RuleDecl;
}

void SysYParser::DeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDecl(this);
}

void SysYParser::DeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDecl(this);
}


std::any SysYParser::DeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitDecl(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::DeclContext* SysYParser::decl() {
  DeclContext *_localctx = _tracker.createInstance<DeclContext>(_ctx, getState());
  enterRule(_localctx, 4, SysYParser::RuleDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(79);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::CONST: {
        enterOuterAlt(_localctx, 1);
        setState(77);
        constDecl();
        break;
      }

      case SysYParser::INT:
      case SysYParser::FLOAT: {
        enterOuterAlt(_localctx, 2);
        setState(78);
        varDecl();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstDeclContext ------------------------------------------------------------------

SysYParser::ConstDeclContext::ConstDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::ConstDeclContext::CONST() {
  return getToken(SysYParser::CONST, 0);
}

SysYParser::BTypeContext* SysYParser::ConstDeclContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

std::vector<SysYParser::ConstDefContext *> SysYParser::ConstDeclContext::constDef() {
  return getRuleContexts<SysYParser::ConstDefContext>();
}

SysYParser::ConstDefContext* SysYParser::ConstDeclContext::constDef(size_t i) {
  return getRuleContext<SysYParser::ConstDefContext>(i);
}

tree::TerminalNode* SysYParser::ConstDeclContext::SEMICO() {
  return getToken(SysYParser::SEMICO, 0);
}

std::vector<tree::TerminalNode *> SysYParser::ConstDeclContext::COMMA() {
  return getTokens(SysYParser::COMMA);
}

tree::TerminalNode* SysYParser::ConstDeclContext::COMMA(size_t i) {
  return getToken(SysYParser::COMMA, i);
}


size_t SysYParser::ConstDeclContext::getRuleIndex() const {
  return SysYParser::RuleConstDecl;
}

void SysYParser::ConstDeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConstDecl(this);
}

void SysYParser::ConstDeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConstDecl(this);
}


std::any SysYParser::ConstDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitConstDecl(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ConstDeclContext* SysYParser::constDecl() {
  ConstDeclContext *_localctx = _tracker.createInstance<ConstDeclContext>(_ctx, getState());
  enterRule(_localctx, 6, SysYParser::RuleConstDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(81);
    match(SysYParser::CONST);
    setState(82);
    bType();
    setState(83);
    constDef();
    setState(88);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::COMMA) {
      setState(84);
      match(SysYParser::COMMA);
      setState(85);
      constDef();
      setState(90);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(91);
    match(SysYParser::SEMICO);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BTypeContext ------------------------------------------------------------------

SysYParser::BTypeContext::BTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::BTypeContext::INT() {
  return getToken(SysYParser::INT, 0);
}

tree::TerminalNode* SysYParser::BTypeContext::FLOAT() {
  return getToken(SysYParser::FLOAT, 0);
}


size_t SysYParser::BTypeContext::getRuleIndex() const {
  return SysYParser::RuleBType;
}

void SysYParser::BTypeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBType(this);
}

void SysYParser::BTypeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBType(this);
}


std::any SysYParser::BTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBType(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::BTypeContext* SysYParser::bType() {
  BTypeContext *_localctx = _tracker.createInstance<BTypeContext>(_ctx, getState());
  enterRule(_localctx, 8, SysYParser::RuleBType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(93);
    _la = _input->LA(1);
    if (!(_la == SysYParser::INT

    || _la == SysYParser::FLOAT)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstDefContext ------------------------------------------------------------------

SysYParser::ConstDefContext::ConstDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::ConstDefContext::ID() {
  return getToken(SysYParser::ID, 0);
}

tree::TerminalNode* SysYParser::ConstDefContext::ASSIGN() {
  return getToken(SysYParser::ASSIGN, 0);
}

SysYParser::ConstInitValContext* SysYParser::ConstDefContext::constInitVal() {
  return getRuleContext<SysYParser::ConstInitValContext>(0);
}

std::vector<tree::TerminalNode *> SysYParser::ConstDefContext::LBRACK() {
  return getTokens(SysYParser::LBRACK);
}

tree::TerminalNode* SysYParser::ConstDefContext::LBRACK(size_t i) {
  return getToken(SysYParser::LBRACK, i);
}

std::vector<SysYParser::ConstExpContext *> SysYParser::ConstDefContext::constExp() {
  return getRuleContexts<SysYParser::ConstExpContext>();
}

SysYParser::ConstExpContext* SysYParser::ConstDefContext::constExp(size_t i) {
  return getRuleContext<SysYParser::ConstExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::ConstDefContext::RBRACK() {
  return getTokens(SysYParser::RBRACK);
}

tree::TerminalNode* SysYParser::ConstDefContext::RBRACK(size_t i) {
  return getToken(SysYParser::RBRACK, i);
}


size_t SysYParser::ConstDefContext::getRuleIndex() const {
  return SysYParser::RuleConstDef;
}

void SysYParser::ConstDefContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConstDef(this);
}

void SysYParser::ConstDefContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConstDef(this);
}


std::any SysYParser::ConstDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitConstDef(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ConstDefContext* SysYParser::constDef() {
  ConstDefContext *_localctx = _tracker.createInstance<ConstDefContext>(_ctx, getState());
  enterRule(_localctx, 10, SysYParser::RuleConstDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(95);
    match(SysYParser::ID);
    setState(102);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::LBRACK) {
      setState(96);
      match(SysYParser::LBRACK);
      setState(97);
      constExp();
      setState(98);
      match(SysYParser::RBRACK);
      setState(104);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(105);
    match(SysYParser::ASSIGN);
    setState(106);
    constInitVal();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstInitValContext ------------------------------------------------------------------

SysYParser::ConstInitValContext::ConstInitValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::ConstExpContext* SysYParser::ConstInitValContext::constExp() {
  return getRuleContext<SysYParser::ConstExpContext>(0);
}

tree::TerminalNode* SysYParser::ConstInitValContext::LBRACE() {
  return getToken(SysYParser::LBRACE, 0);
}

tree::TerminalNode* SysYParser::ConstInitValContext::RBRACE() {
  return getToken(SysYParser::RBRACE, 0);
}

std::vector<SysYParser::ConstArrayInitValContext *> SysYParser::ConstInitValContext::constArrayInitVal() {
  return getRuleContexts<SysYParser::ConstArrayInitValContext>();
}

SysYParser::ConstArrayInitValContext* SysYParser::ConstInitValContext::constArrayInitVal(size_t i) {
  return getRuleContext<SysYParser::ConstArrayInitValContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::ConstInitValContext::COMMA() {
  return getTokens(SysYParser::COMMA);
}

tree::TerminalNode* SysYParser::ConstInitValContext::COMMA(size_t i) {
  return getToken(SysYParser::COMMA, i);
}


size_t SysYParser::ConstInitValContext::getRuleIndex() const {
  return SysYParser::RuleConstInitVal;
}

void SysYParser::ConstInitValContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConstInitVal(this);
}

void SysYParser::ConstInitValContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConstInitVal(this);
}


std::any SysYParser::ConstInitValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitConstInitVal(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ConstInitValContext* SysYParser::constInitVal() {
  ConstInitValContext *_localctx = _tracker.createInstance<ConstInitValContext>(_ctx, getState());
  enterRule(_localctx, 12, SysYParser::RuleConstInitVal);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(121);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::ADD:
      case SysYParser::SUB:
      case SysYParser::NOT:
      case SysYParser::LPAREN:
      case SysYParser::ID:
      case SysYParser::IntConst:
      case SysYParser::FloatConst: {
        enterOuterAlt(_localctx, 1);
        setState(108);
        constExp();
        break;
      }

      case SysYParser::LBRACE: {
        enterOuterAlt(_localctx, 2);
        setState(109);
        match(SysYParser::LBRACE);
        setState(118);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 121433501696) != 0)) {
          setState(110);
          constArrayInitVal();
          setState(115);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysYParser::COMMA) {
            setState(111);
            match(SysYParser::COMMA);
            setState(112);
            constArrayInitVal();
            setState(117);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(120);
        match(SysYParser::RBRACE);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstArrayInitValContext ------------------------------------------------------------------

SysYParser::ConstArrayInitValContext::ConstArrayInitValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::ConstExpContext* SysYParser::ConstArrayInitValContext::constExp() {
  return getRuleContext<SysYParser::ConstExpContext>(0);
}

tree::TerminalNode* SysYParser::ConstArrayInitValContext::LBRACE() {
  return getToken(SysYParser::LBRACE, 0);
}

tree::TerminalNode* SysYParser::ConstArrayInitValContext::RBRACE() {
  return getToken(SysYParser::RBRACE, 0);
}

std::vector<SysYParser::ConstArrayInitValContext *> SysYParser::ConstArrayInitValContext::constArrayInitVal() {
  return getRuleContexts<SysYParser::ConstArrayInitValContext>();
}

SysYParser::ConstArrayInitValContext* SysYParser::ConstArrayInitValContext::constArrayInitVal(size_t i) {
  return getRuleContext<SysYParser::ConstArrayInitValContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::ConstArrayInitValContext::COMMA() {
  return getTokens(SysYParser::COMMA);
}

tree::TerminalNode* SysYParser::ConstArrayInitValContext::COMMA(size_t i) {
  return getToken(SysYParser::COMMA, i);
}


size_t SysYParser::ConstArrayInitValContext::getRuleIndex() const {
  return SysYParser::RuleConstArrayInitVal;
}

void SysYParser::ConstArrayInitValContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConstArrayInitVal(this);
}

void SysYParser::ConstArrayInitValContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConstArrayInitVal(this);
}


std::any SysYParser::ConstArrayInitValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitConstArrayInitVal(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ConstArrayInitValContext* SysYParser::constArrayInitVal() {
  ConstArrayInitValContext *_localctx = _tracker.createInstance<ConstArrayInitValContext>(_ctx, getState());
  enterRule(_localctx, 14, SysYParser::RuleConstArrayInitVal);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(136);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::ADD:
      case SysYParser::SUB:
      case SysYParser::NOT:
      case SysYParser::LPAREN:
      case SysYParser::ID:
      case SysYParser::IntConst:
      case SysYParser::FloatConst: {
        enterOuterAlt(_localctx, 1);
        setState(123);
        constExp();
        break;
      }

      case SysYParser::LBRACE: {
        enterOuterAlt(_localctx, 2);
        setState(124);
        match(SysYParser::LBRACE);
        setState(133);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 121433501696) != 0)) {
          setState(125);
          constArrayInitVal();
          setState(130);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysYParser::COMMA) {
            setState(126);
            match(SysYParser::COMMA);
            setState(127);
            constArrayInitVal();
            setState(132);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(135);
        match(SysYParser::RBRACE);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDeclContext ------------------------------------------------------------------

SysYParser::VarDeclContext::VarDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::BTypeContext* SysYParser::VarDeclContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

std::vector<SysYParser::VarDefContext *> SysYParser::VarDeclContext::varDef() {
  return getRuleContexts<SysYParser::VarDefContext>();
}

SysYParser::VarDefContext* SysYParser::VarDeclContext::varDef(size_t i) {
  return getRuleContext<SysYParser::VarDefContext>(i);
}

tree::TerminalNode* SysYParser::VarDeclContext::SEMICO() {
  return getToken(SysYParser::SEMICO, 0);
}

std::vector<tree::TerminalNode *> SysYParser::VarDeclContext::COMMA() {
  return getTokens(SysYParser::COMMA);
}

tree::TerminalNode* SysYParser::VarDeclContext::COMMA(size_t i) {
  return getToken(SysYParser::COMMA, i);
}


size_t SysYParser::VarDeclContext::getRuleIndex() const {
  return SysYParser::RuleVarDecl;
}

void SysYParser::VarDeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVarDecl(this);
}

void SysYParser::VarDeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVarDecl(this);
}


std::any SysYParser::VarDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitVarDecl(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::VarDeclContext* SysYParser::varDecl() {
  VarDeclContext *_localctx = _tracker.createInstance<VarDeclContext>(_ctx, getState());
  enterRule(_localctx, 16, SysYParser::RuleVarDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(138);
    bType();
    setState(139);
    varDef();
    setState(144);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::COMMA) {
      setState(140);
      match(SysYParser::COMMA);
      setState(141);
      varDef();
      setState(146);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(147);
    match(SysYParser::SEMICO);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDefContext ------------------------------------------------------------------

SysYParser::VarDefContext::VarDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::VarDefContext::ID() {
  return getToken(SysYParser::ID, 0);
}

std::vector<tree::TerminalNode *> SysYParser::VarDefContext::LBRACK() {
  return getTokens(SysYParser::LBRACK);
}

tree::TerminalNode* SysYParser::VarDefContext::LBRACK(size_t i) {
  return getToken(SysYParser::LBRACK, i);
}

std::vector<SysYParser::ConstExpContext *> SysYParser::VarDefContext::constExp() {
  return getRuleContexts<SysYParser::ConstExpContext>();
}

SysYParser::ConstExpContext* SysYParser::VarDefContext::constExp(size_t i) {
  return getRuleContext<SysYParser::ConstExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::VarDefContext::RBRACK() {
  return getTokens(SysYParser::RBRACK);
}

tree::TerminalNode* SysYParser::VarDefContext::RBRACK(size_t i) {
  return getToken(SysYParser::RBRACK, i);
}

tree::TerminalNode* SysYParser::VarDefContext::ASSIGN() {
  return getToken(SysYParser::ASSIGN, 0);
}

SysYParser::InitValContext* SysYParser::VarDefContext::initVal() {
  return getRuleContext<SysYParser::InitValContext>(0);
}


size_t SysYParser::VarDefContext::getRuleIndex() const {
  return SysYParser::RuleVarDef;
}

void SysYParser::VarDefContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVarDef(this);
}

void SysYParser::VarDefContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVarDef(this);
}


std::any SysYParser::VarDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitVarDef(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::VarDefContext* SysYParser::varDef() {
  VarDefContext *_localctx = _tracker.createInstance<VarDefContext>(_ctx, getState());
  enterRule(_localctx, 18, SysYParser::RuleVarDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(171);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(149);
      match(SysYParser::ID);
      setState(156);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysYParser::LBRACK) {
        setState(150);
        match(SysYParser::LBRACK);
        setState(151);
        constExp();
        setState(152);
        match(SysYParser::RBRACK);
        setState(158);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(159);
      match(SysYParser::ID);
      setState(166);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysYParser::LBRACK) {
        setState(160);
        match(SysYParser::LBRACK);
        setState(161);
        constExp();
        setState(162);
        match(SysYParser::RBRACK);
        setState(168);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(169);
      match(SysYParser::ASSIGN);
      setState(170);
      initVal();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InitValContext ------------------------------------------------------------------

SysYParser::InitValContext::InitValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::ExpContext* SysYParser::InitValContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

tree::TerminalNode* SysYParser::InitValContext::LBRACE() {
  return getToken(SysYParser::LBRACE, 0);
}

tree::TerminalNode* SysYParser::InitValContext::RBRACE() {
  return getToken(SysYParser::RBRACE, 0);
}

std::vector<SysYParser::ArrayInitValContext *> SysYParser::InitValContext::arrayInitVal() {
  return getRuleContexts<SysYParser::ArrayInitValContext>();
}

SysYParser::ArrayInitValContext* SysYParser::InitValContext::arrayInitVal(size_t i) {
  return getRuleContext<SysYParser::ArrayInitValContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::InitValContext::COMMA() {
  return getTokens(SysYParser::COMMA);
}

tree::TerminalNode* SysYParser::InitValContext::COMMA(size_t i) {
  return getToken(SysYParser::COMMA, i);
}


size_t SysYParser::InitValContext::getRuleIndex() const {
  return SysYParser::RuleInitVal;
}

void SysYParser::InitValContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInitVal(this);
}

void SysYParser::InitValContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInitVal(this);
}


std::any SysYParser::InitValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitInitVal(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::InitValContext* SysYParser::initVal() {
  InitValContext *_localctx = _tracker.createInstance<InitValContext>(_ctx, getState());
  enterRule(_localctx, 20, SysYParser::RuleInitVal);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(186);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::ADD:
      case SysYParser::SUB:
      case SysYParser::NOT:
      case SysYParser::LPAREN:
      case SysYParser::ID:
      case SysYParser::IntConst:
      case SysYParser::FloatConst: {
        enterOuterAlt(_localctx, 1);
        setState(173);
        exp();
        break;
      }

      case SysYParser::LBRACE: {
        enterOuterAlt(_localctx, 2);
        setState(174);
        match(SysYParser::LBRACE);
        setState(183);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 121433501696) != 0)) {
          setState(175);
          arrayInitVal();
          setState(180);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysYParser::COMMA) {
            setState(176);
            match(SysYParser::COMMA);
            setState(177);
            arrayInitVal();
            setState(182);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(185);
        match(SysYParser::RBRACE);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArrayInitValContext ------------------------------------------------------------------

SysYParser::ArrayInitValContext::ArrayInitValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::ExpContext* SysYParser::ArrayInitValContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

tree::TerminalNode* SysYParser::ArrayInitValContext::LBRACE() {
  return getToken(SysYParser::LBRACE, 0);
}

tree::TerminalNode* SysYParser::ArrayInitValContext::RBRACE() {
  return getToken(SysYParser::RBRACE, 0);
}

std::vector<SysYParser::ArrayInitValContext *> SysYParser::ArrayInitValContext::arrayInitVal() {
  return getRuleContexts<SysYParser::ArrayInitValContext>();
}

SysYParser::ArrayInitValContext* SysYParser::ArrayInitValContext::arrayInitVal(size_t i) {
  return getRuleContext<SysYParser::ArrayInitValContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::ArrayInitValContext::COMMA() {
  return getTokens(SysYParser::COMMA);
}

tree::TerminalNode* SysYParser::ArrayInitValContext::COMMA(size_t i) {
  return getToken(SysYParser::COMMA, i);
}


size_t SysYParser::ArrayInitValContext::getRuleIndex() const {
  return SysYParser::RuleArrayInitVal;
}

void SysYParser::ArrayInitValContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArrayInitVal(this);
}

void SysYParser::ArrayInitValContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArrayInitVal(this);
}


std::any SysYParser::ArrayInitValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitArrayInitVal(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ArrayInitValContext* SysYParser::arrayInitVal() {
  ArrayInitValContext *_localctx = _tracker.createInstance<ArrayInitValContext>(_ctx, getState());
  enterRule(_localctx, 22, SysYParser::RuleArrayInitVal);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(201);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::ADD:
      case SysYParser::SUB:
      case SysYParser::NOT:
      case SysYParser::LPAREN:
      case SysYParser::ID:
      case SysYParser::IntConst:
      case SysYParser::FloatConst: {
        enterOuterAlt(_localctx, 1);
        setState(188);
        exp();
        break;
      }

      case SysYParser::LBRACE: {
        enterOuterAlt(_localctx, 2);
        setState(189);
        match(SysYParser::LBRACE);
        setState(198);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 121433501696) != 0)) {
          setState(190);
          arrayInitVal();
          setState(195);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysYParser::COMMA) {
            setState(191);
            match(SysYParser::COMMA);
            setState(192);
            arrayInitVal();
            setState(197);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(200);
        match(SysYParser::RBRACE);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncDefContext ------------------------------------------------------------------

SysYParser::FuncDefContext::FuncDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::FuncTypeContext* SysYParser::FuncDefContext::funcType() {
  return getRuleContext<SysYParser::FuncTypeContext>(0);
}

tree::TerminalNode* SysYParser::FuncDefContext::ID() {
  return getToken(SysYParser::ID, 0);
}

tree::TerminalNode* SysYParser::FuncDefContext::LPAREN() {
  return getToken(SysYParser::LPAREN, 0);
}

tree::TerminalNode* SysYParser::FuncDefContext::RPAREN() {
  return getToken(SysYParser::RPAREN, 0);
}

SysYParser::BlockContext* SysYParser::FuncDefContext::block() {
  return getRuleContext<SysYParser::BlockContext>(0);
}

SysYParser::FuncFParamsContext* SysYParser::FuncDefContext::funcFParams() {
  return getRuleContext<SysYParser::FuncFParamsContext>(0);
}


size_t SysYParser::FuncDefContext::getRuleIndex() const {
  return SysYParser::RuleFuncDef;
}

void SysYParser::FuncDefContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFuncDef(this);
}

void SysYParser::FuncDefContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFuncDef(this);
}


std::any SysYParser::FuncDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncDef(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncDefContext* SysYParser::funcDef() {
  FuncDefContext *_localctx = _tracker.createInstance<FuncDefContext>(_ctx, getState());
  enterRule(_localctx, 24, SysYParser::RuleFuncDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(203);
    funcType();
    setState(204);
    match(SysYParser::ID);
    setState(205);
    match(SysYParser::LPAREN);
    setState(207);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysYParser::INT

    || _la == SysYParser::FLOAT) {
      setState(206);
      funcFParams();
    }
    setState(209);
    match(SysYParser::RPAREN);
    setState(210);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncTypeContext ------------------------------------------------------------------

SysYParser::FuncTypeContext::FuncTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::FuncTypeContext::VOID() {
  return getToken(SysYParser::VOID, 0);
}

tree::TerminalNode* SysYParser::FuncTypeContext::INT() {
  return getToken(SysYParser::INT, 0);
}

tree::TerminalNode* SysYParser::FuncTypeContext::FLOAT() {
  return getToken(SysYParser::FLOAT, 0);
}


size_t SysYParser::FuncTypeContext::getRuleIndex() const {
  return SysYParser::RuleFuncType;
}

void SysYParser::FuncTypeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFuncType(this);
}

void SysYParser::FuncTypeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFuncType(this);
}


std::any SysYParser::FuncTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncType(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncTypeContext* SysYParser::funcType() {
  FuncTypeContext *_localctx = _tracker.createInstance<FuncTypeContext>(_ctx, getState());
  enterRule(_localctx, 26, SysYParser::RuleFuncType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(212);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 14) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncFParamsContext ------------------------------------------------------------------

SysYParser::FuncFParamsContext::FuncFParamsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysYParser::FuncFParamContext *> SysYParser::FuncFParamsContext::funcFParam() {
  return getRuleContexts<SysYParser::FuncFParamContext>();
}

SysYParser::FuncFParamContext* SysYParser::FuncFParamsContext::funcFParam(size_t i) {
  return getRuleContext<SysYParser::FuncFParamContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::FuncFParamsContext::COMMA() {
  return getTokens(SysYParser::COMMA);
}

tree::TerminalNode* SysYParser::FuncFParamsContext::COMMA(size_t i) {
  return getToken(SysYParser::COMMA, i);
}


size_t SysYParser::FuncFParamsContext::getRuleIndex() const {
  return SysYParser::RuleFuncFParams;
}

void SysYParser::FuncFParamsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFuncFParams(this);
}

void SysYParser::FuncFParamsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFuncFParams(this);
}


std::any SysYParser::FuncFParamsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncFParams(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncFParamsContext* SysYParser::funcFParams() {
  FuncFParamsContext *_localctx = _tracker.createInstance<FuncFParamsContext>(_ctx, getState());
  enterRule(_localctx, 28, SysYParser::RuleFuncFParams);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(214);
    funcFParam();
    setState(219);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::COMMA) {
      setState(215);
      match(SysYParser::COMMA);
      setState(216);
      funcFParam();
      setState(221);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncFParamContext ------------------------------------------------------------------

SysYParser::FuncFParamContext::FuncFParamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::BTypeContext* SysYParser::FuncFParamContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

tree::TerminalNode* SysYParser::FuncFParamContext::ID() {
  return getToken(SysYParser::ID, 0);
}

std::vector<tree::TerminalNode *> SysYParser::FuncFParamContext::LBRACK() {
  return getTokens(SysYParser::LBRACK);
}

tree::TerminalNode* SysYParser::FuncFParamContext::LBRACK(size_t i) {
  return getToken(SysYParser::LBRACK, i);
}

std::vector<tree::TerminalNode *> SysYParser::FuncFParamContext::RBRACK() {
  return getTokens(SysYParser::RBRACK);
}

tree::TerminalNode* SysYParser::FuncFParamContext::RBRACK(size_t i) {
  return getToken(SysYParser::RBRACK, i);
}

std::vector<SysYParser::ExpContext *> SysYParser::FuncFParamContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::FuncFParamContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}


size_t SysYParser::FuncFParamContext::getRuleIndex() const {
  return SysYParser::RuleFuncFParam;
}

void SysYParser::FuncFParamContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFuncFParam(this);
}

void SysYParser::FuncFParamContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFuncFParam(this);
}


std::any SysYParser::FuncFParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncFParam(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncFParamContext* SysYParser::funcFParam() {
  FuncFParamContext *_localctx = _tracker.createInstance<FuncFParamContext>(_ctx, getState());
  enterRule(_localctx, 30, SysYParser::RuleFuncFParam);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(222);
    bType();
    setState(223);
    match(SysYParser::ID);
    setState(235);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysYParser::LBRACK) {
      setState(224);
      match(SysYParser::LBRACK);
      setState(225);
      match(SysYParser::RBRACK);
      setState(232);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysYParser::LBRACK) {
        setState(226);
        match(SysYParser::LBRACK);
        setState(227);
        exp();
        setState(228);
        match(SysYParser::RBRACK);
        setState(234);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockContext ------------------------------------------------------------------

SysYParser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::BlockContext::LBRACE() {
  return getToken(SysYParser::LBRACE, 0);
}

tree::TerminalNode* SysYParser::BlockContext::RBRACE() {
  return getToken(SysYParser::RBRACE, 0);
}

std::vector<SysYParser::BlockItemContext *> SysYParser::BlockContext::blockItem() {
  return getRuleContexts<SysYParser::BlockItemContext>();
}

SysYParser::BlockItemContext* SysYParser::BlockContext::blockItem(size_t i) {
  return getRuleContext<SysYParser::BlockItemContext>(i);
}


size_t SysYParser::BlockContext::getRuleIndex() const {
  return SysYParser::RuleBlock;
}

void SysYParser::BlockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBlock(this);
}

void SysYParser::BlockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBlock(this);
}


std::any SysYParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::BlockContext* SysYParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 32, SysYParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(237);
    match(SysYParser::LBRACE);
    setState(241);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 130023438294) != 0)) {
      setState(238);
      blockItem();
      setState(243);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(244);
    match(SysYParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockItemContext ------------------------------------------------------------------

SysYParser::BlockItemContext::BlockItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::DeclContext* SysYParser::BlockItemContext::decl() {
  return getRuleContext<SysYParser::DeclContext>(0);
}

SysYParser::StmtContext* SysYParser::BlockItemContext::stmt() {
  return getRuleContext<SysYParser::StmtContext>(0);
}


size_t SysYParser::BlockItemContext::getRuleIndex() const {
  return SysYParser::RuleBlockItem;
}

void SysYParser::BlockItemContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBlockItem(this);
}

void SysYParser::BlockItemContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBlockItem(this);
}


std::any SysYParser::BlockItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBlockItem(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::BlockItemContext* SysYParser::blockItem() {
  BlockItemContext *_localctx = _tracker.createInstance<BlockItemContext>(_ctx, getState());
  enterRule(_localctx, 34, SysYParser::RuleBlockItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(248);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::INT:
      case SysYParser::FLOAT:
      case SysYParser::CONST: {
        enterOuterAlt(_localctx, 1);
        setState(246);
        decl();
        break;
      }

      case SysYParser::IF:
      case SysYParser::WHILE:
      case SysYParser::CONTINUE:
      case SysYParser::BREAK:
      case SysYParser::RETURN:
      case SysYParser::ADD:
      case SysYParser::SUB:
      case SysYParser::NOT:
      case SysYParser::LPAREN:
      case SysYParser::LBRACE:
      case SysYParser::SEMICO:
      case SysYParser::ID:
      case SysYParser::IntConst:
      case SysYParser::FloatConst: {
        enterOuterAlt(_localctx, 2);
        setState(247);
        stmt();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StmtContext ------------------------------------------------------------------

SysYParser::StmtContext::StmtContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::LValContext* SysYParser::StmtContext::lVal() {
  return getRuleContext<SysYParser::LValContext>(0);
}

tree::TerminalNode* SysYParser::StmtContext::ASSIGN() {
  return getToken(SysYParser::ASSIGN, 0);
}

SysYParser::ExpContext* SysYParser::StmtContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

tree::TerminalNode* SysYParser::StmtContext::SEMICO() {
  return getToken(SysYParser::SEMICO, 0);
}

SysYParser::BlockContext* SysYParser::StmtContext::block() {
  return getRuleContext<SysYParser::BlockContext>(0);
}

tree::TerminalNode* SysYParser::StmtContext::IF() {
  return getToken(SysYParser::IF, 0);
}

tree::TerminalNode* SysYParser::StmtContext::LPAREN() {
  return getToken(SysYParser::LPAREN, 0);
}

SysYParser::CondContext* SysYParser::StmtContext::cond() {
  return getRuleContext<SysYParser::CondContext>(0);
}

tree::TerminalNode* SysYParser::StmtContext::RPAREN() {
  return getToken(SysYParser::RPAREN, 0);
}

std::vector<SysYParser::StmtContext *> SysYParser::StmtContext::stmt() {
  return getRuleContexts<SysYParser::StmtContext>();
}

SysYParser::StmtContext* SysYParser::StmtContext::stmt(size_t i) {
  return getRuleContext<SysYParser::StmtContext>(i);
}

tree::TerminalNode* SysYParser::StmtContext::ELSE() {
  return getToken(SysYParser::ELSE, 0);
}

tree::TerminalNode* SysYParser::StmtContext::WHILE() {
  return getToken(SysYParser::WHILE, 0);
}

tree::TerminalNode* SysYParser::StmtContext::BREAK() {
  return getToken(SysYParser::BREAK, 0);
}

tree::TerminalNode* SysYParser::StmtContext::CONTINUE() {
  return getToken(SysYParser::CONTINUE, 0);
}

tree::TerminalNode* SysYParser::StmtContext::RETURN() {
  return getToken(SysYParser::RETURN, 0);
}


size_t SysYParser::StmtContext::getRuleIndex() const {
  return SysYParser::RuleStmt;
}

void SysYParser::StmtContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStmt(this);
}

void SysYParser::StmtContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStmt(this);
}


std::any SysYParser::StmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitStmt(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::StmtContext* SysYParser::stmt() {
  StmtContext *_localctx = _tracker.createInstance<StmtContext>(_ctx, getState());
  enterRule(_localctx, 36, SysYParser::RuleStmt);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(284);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(250);
      lVal();
      setState(251);
      match(SysYParser::ASSIGN);
      setState(252);
      exp();
      setState(253);
      match(SysYParser::SEMICO);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(256);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 120359759872) != 0)) {
        setState(255);
        exp();
      }
      setState(258);
      match(SysYParser::SEMICO);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(259);
      block();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(260);
      match(SysYParser::IF);
      setState(261);
      match(SysYParser::LPAREN);
      setState(262);
      cond();
      setState(263);
      match(SysYParser::RPAREN);
      setState(264);
      stmt();
      setState(267);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx)) {
      case 1: {
        setState(265);
        match(SysYParser::ELSE);
        setState(266);
        stmt();
        break;
      }

      default:
        break;
      }
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(269);
      match(SysYParser::WHILE);
      setState(270);
      match(SysYParser::LPAREN);
      setState(271);
      cond();
      setState(272);
      match(SysYParser::RPAREN);
      setState(273);
      stmt();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(275);
      match(SysYParser::BREAK);
      setState(276);
      match(SysYParser::SEMICO);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(277);
      match(SysYParser::CONTINUE);
      setState(278);
      match(SysYParser::SEMICO);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(279);
      match(SysYParser::RETURN);
      setState(281);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 120359759872) != 0)) {
        setState(280);
        exp();
      }
      setState(283);
      match(SysYParser::SEMICO);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpContext ------------------------------------------------------------------

SysYParser::ExpContext::ExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::AddExpContext* SysYParser::ExpContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}


size_t SysYParser::ExpContext::getRuleIndex() const {
  return SysYParser::RuleExp;
}

void SysYParser::ExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExp(this);
}

void SysYParser::ExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExp(this);
}


std::any SysYParser::ExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitExp(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ExpContext* SysYParser::exp() {
  ExpContext *_localctx = _tracker.createInstance<ExpContext>(_ctx, getState());
  enterRule(_localctx, 38, SysYParser::RuleExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(286);
    addExp(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CondContext ------------------------------------------------------------------

SysYParser::CondContext::CondContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::LOrExpContext* SysYParser::CondContext::lOrExp() {
  return getRuleContext<SysYParser::LOrExpContext>(0);
}


size_t SysYParser::CondContext::getRuleIndex() const {
  return SysYParser::RuleCond;
}

void SysYParser::CondContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCond(this);
}

void SysYParser::CondContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCond(this);
}


std::any SysYParser::CondContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCond(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::CondContext* SysYParser::cond() {
  CondContext *_localctx = _tracker.createInstance<CondContext>(_ctx, getState());
  enterRule(_localctx, 40, SysYParser::RuleCond);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(288);
    lOrExp();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LValContext ------------------------------------------------------------------

SysYParser::LValContext::LValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::LValContext::ID() {
  return getToken(SysYParser::ID, 0);
}

std::vector<tree::TerminalNode *> SysYParser::LValContext::LBRACK() {
  return getTokens(SysYParser::LBRACK);
}

tree::TerminalNode* SysYParser::LValContext::LBRACK(size_t i) {
  return getToken(SysYParser::LBRACK, i);
}

std::vector<SysYParser::ExpContext *> SysYParser::LValContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::LValContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::LValContext::RBRACK() {
  return getTokens(SysYParser::RBRACK);
}

tree::TerminalNode* SysYParser::LValContext::RBRACK(size_t i) {
  return getToken(SysYParser::RBRACK, i);
}


size_t SysYParser::LValContext::getRuleIndex() const {
  return SysYParser::RuleLVal;
}

void SysYParser::LValContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLVal(this);
}

void SysYParser::LValContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLVal(this);
}


std::any SysYParser::LValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLVal(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::LValContext* SysYParser::lVal() {
  LValContext *_localctx = _tracker.createInstance<LValContext>(_ctx, getState());
  enterRule(_localctx, 42, SysYParser::RuleLVal);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(290);
    match(SysYParser::ID);
    setState(297);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(291);
        match(SysYParser::LBRACK);
        setState(292);
        exp();
        setState(293);
        match(SysYParser::RBRACK); 
      }
      setState(299);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimaryExpContext ------------------------------------------------------------------

SysYParser::PrimaryExpContext::PrimaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::PrimaryExpContext::LPAREN() {
  return getToken(SysYParser::LPAREN, 0);
}

SysYParser::ExpContext* SysYParser::PrimaryExpContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

tree::TerminalNode* SysYParser::PrimaryExpContext::RPAREN() {
  return getToken(SysYParser::RPAREN, 0);
}

SysYParser::LValContext* SysYParser::PrimaryExpContext::lVal() {
  return getRuleContext<SysYParser::LValContext>(0);
}

SysYParser::NumberContext* SysYParser::PrimaryExpContext::number() {
  return getRuleContext<SysYParser::NumberContext>(0);
}


size_t SysYParser::PrimaryExpContext::getRuleIndex() const {
  return SysYParser::RulePrimaryExp;
}

void SysYParser::PrimaryExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrimaryExp(this);
}

void SysYParser::PrimaryExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrimaryExp(this);
}


std::any SysYParser::PrimaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitPrimaryExp(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::PrimaryExpContext* SysYParser::primaryExp() {
  PrimaryExpContext *_localctx = _tracker.createInstance<PrimaryExpContext>(_ctx, getState());
  enterRule(_localctx, 44, SysYParser::RulePrimaryExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(306);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::LPAREN: {
        enterOuterAlt(_localctx, 1);
        setState(300);
        match(SysYParser::LPAREN);
        setState(301);
        exp();
        setState(302);
        match(SysYParser::RPAREN);
        break;
      }

      case SysYParser::ID: {
        enterOuterAlt(_localctx, 2);
        setState(304);
        lVal();
        break;
      }

      case SysYParser::IntConst:
      case SysYParser::FloatConst: {
        enterOuterAlt(_localctx, 3);
        setState(305);
        number();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NumberContext ------------------------------------------------------------------

SysYParser::NumberContext::NumberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::NumberContext::IntConst() {
  return getToken(SysYParser::IntConst, 0);
}

tree::TerminalNode* SysYParser::NumberContext::FloatConst() {
  return getToken(SysYParser::FloatConst, 0);
}


size_t SysYParser::NumberContext::getRuleIndex() const {
  return SysYParser::RuleNumber;
}

void SysYParser::NumberContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNumber(this);
}

void SysYParser::NumberContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNumber(this);
}


std::any SysYParser::NumberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitNumber(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::NumberContext* SysYParser::number() {
  NumberContext *_localctx = _tracker.createInstance<NumberContext>(_ctx, getState());
  enterRule(_localctx, 46, SysYParser::RuleNumber);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(308);
    _la = _input->LA(1);
    if (!(_la == SysYParser::IntConst

    || _la == SysYParser::FloatConst)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryExpContext ------------------------------------------------------------------

SysYParser::UnaryExpContext::UnaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::PrimaryExpContext* SysYParser::UnaryExpContext::primaryExp() {
  return getRuleContext<SysYParser::PrimaryExpContext>(0);
}

tree::TerminalNode* SysYParser::UnaryExpContext::ID() {
  return getToken(SysYParser::ID, 0);
}

tree::TerminalNode* SysYParser::UnaryExpContext::LPAREN() {
  return getToken(SysYParser::LPAREN, 0);
}

tree::TerminalNode* SysYParser::UnaryExpContext::RPAREN() {
  return getToken(SysYParser::RPAREN, 0);
}

SysYParser::FuncRParamsContext* SysYParser::UnaryExpContext::funcRParams() {
  return getRuleContext<SysYParser::FuncRParamsContext>(0);
}

SysYParser::UnaryExpContext* SysYParser::UnaryExpContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

tree::TerminalNode* SysYParser::UnaryExpContext::ADD() {
  return getToken(SysYParser::ADD, 0);
}

tree::TerminalNode* SysYParser::UnaryExpContext::SUB() {
  return getToken(SysYParser::SUB, 0);
}

tree::TerminalNode* SysYParser::UnaryExpContext::NOT() {
  return getToken(SysYParser::NOT, 0);
}


size_t SysYParser::UnaryExpContext::getRuleIndex() const {
  return SysYParser::RuleUnaryExp;
}

void SysYParser::UnaryExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnaryExp(this);
}

void SysYParser::UnaryExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnaryExp(this);
}


std::any SysYParser::UnaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitUnaryExp(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::UnaryExpContext* SysYParser::unaryExp() {
  UnaryExpContext *_localctx = _tracker.createInstance<UnaryExpContext>(_ctx, getState());
  enterRule(_localctx, 48, SysYParser::RuleUnaryExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(319);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(310);
      primaryExp();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(311);
      match(SysYParser::ID);
      setState(312);
      match(SysYParser::LPAREN);
      setState(314);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 120359759872) != 0)) {
        setState(313);
        funcRParams();
      }
      setState(316);
      match(SysYParser::RPAREN);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(317);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 33566720) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(318);
      unaryExp();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncRParamsContext ------------------------------------------------------------------

SysYParser::FuncRParamsContext::FuncRParamsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysYParser::ExpContext *> SysYParser::FuncRParamsContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::FuncRParamsContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::FuncRParamsContext::COMMA() {
  return getTokens(SysYParser::COMMA);
}

tree::TerminalNode* SysYParser::FuncRParamsContext::COMMA(size_t i) {
  return getToken(SysYParser::COMMA, i);
}


size_t SysYParser::FuncRParamsContext::getRuleIndex() const {
  return SysYParser::RuleFuncRParams;
}

void SysYParser::FuncRParamsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFuncRParams(this);
}

void SysYParser::FuncRParamsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFuncRParams(this);
}


std::any SysYParser::FuncRParamsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncRParams(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncRParamsContext* SysYParser::funcRParams() {
  FuncRParamsContext *_localctx = _tracker.createInstance<FuncRParamsContext>(_ctx, getState());
  enterRule(_localctx, 50, SysYParser::RuleFuncRParams);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(321);
    exp();
    setState(326);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::COMMA) {
      setState(322);
      match(SysYParser::COMMA);
      setState(323);
      exp();
      setState(328);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MulExpContext ------------------------------------------------------------------

SysYParser::MulExpContext::MulExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::UnaryExpContext* SysYParser::MulExpContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::MulExpContext* SysYParser::MulExpContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

tree::TerminalNode* SysYParser::MulExpContext::MUL() {
  return getToken(SysYParser::MUL, 0);
}

tree::TerminalNode* SysYParser::MulExpContext::DIV() {
  return getToken(SysYParser::DIV, 0);
}

tree::TerminalNode* SysYParser::MulExpContext::MOD() {
  return getToken(SysYParser::MOD, 0);
}


size_t SysYParser::MulExpContext::getRuleIndex() const {
  return SysYParser::RuleMulExp;
}

void SysYParser::MulExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMulExp(this);
}

void SysYParser::MulExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMulExp(this);
}


std::any SysYParser::MulExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitMulExp(this);
  else
    return visitor->visitChildren(this);
}


SysYParser::MulExpContext* SysYParser::mulExp() {
   return mulExp(0);
}

SysYParser::MulExpContext* SysYParser::mulExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::MulExpContext *_localctx = _tracker.createInstance<MulExpContext>(_ctx, parentState);
  SysYParser::MulExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 52;
  enterRecursionRule(_localctx, 52, SysYParser::RuleMulExp, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(330);
    unaryExp();
    _ctx->stop = _input->LT(-1);
    setState(337);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<MulExpContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleMulExp);
        setState(332);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(333);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 114688) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(334);
        unaryExp(); 
      }
      setState(339);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- AddExpContext ------------------------------------------------------------------

SysYParser::AddExpContext::AddExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::MulExpContext* SysYParser::AddExpContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

SysYParser::AddExpContext* SysYParser::AddExpContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

tree::TerminalNode* SysYParser::AddExpContext::ADD() {
  return getToken(SysYParser::ADD, 0);
}

tree::TerminalNode* SysYParser::AddExpContext::SUB() {
  return getToken(SysYParser::SUB, 0);
}


size_t SysYParser::AddExpContext::getRuleIndex() const {
  return SysYParser::RuleAddExp;
}

void SysYParser::AddExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAddExp(this);
}

void SysYParser::AddExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAddExp(this);
}


std::any SysYParser::AddExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitAddExp(this);
  else
    return visitor->visitChildren(this);
}


SysYParser::AddExpContext* SysYParser::addExp() {
   return addExp(0);
}

SysYParser::AddExpContext* SysYParser::addExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::AddExpContext *_localctx = _tracker.createInstance<AddExpContext>(_ctx, parentState);
  SysYParser::AddExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 54;
  enterRecursionRule(_localctx, 54, SysYParser::RuleAddExp, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(341);
    mulExp(0);
    _ctx->stop = _input->LT(-1);
    setState(348);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<AddExpContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleAddExp);
        setState(343);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(344);
        _la = _input->LA(1);
        if (!(_la == SysYParser::ADD

        || _la == SysYParser::SUB)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(345);
        mulExp(0); 
      }
      setState(350);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- RelExpContext ------------------------------------------------------------------

SysYParser::RelExpContext::RelExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::AddExpContext* SysYParser::RelExpContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::RelExpContext* SysYParser::RelExpContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

tree::TerminalNode* SysYParser::RelExpContext::LT() {
  return getToken(SysYParser::LT, 0);
}

tree::TerminalNode* SysYParser::RelExpContext::GT() {
  return getToken(SysYParser::GT, 0);
}

tree::TerminalNode* SysYParser::RelExpContext::LE() {
  return getToken(SysYParser::LE, 0);
}

tree::TerminalNode* SysYParser::RelExpContext::GE() {
  return getToken(SysYParser::GE, 0);
}


size_t SysYParser::RelExpContext::getRuleIndex() const {
  return SysYParser::RuleRelExp;
}

void SysYParser::RelExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRelExp(this);
}

void SysYParser::RelExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRelExp(this);
}


std::any SysYParser::RelExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitRelExp(this);
  else
    return visitor->visitChildren(this);
}


SysYParser::RelExpContext* SysYParser::relExp() {
   return relExp(0);
}

SysYParser::RelExpContext* SysYParser::relExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::RelExpContext *_localctx = _tracker.createInstance<RelExpContext>(_ctx, parentState);
  SysYParser::RelExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 56;
  enterRecursionRule(_localctx, 56, SysYParser::RuleRelExp, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(352);
    addExp(0);
    _ctx->stop = _input->LT(-1);
    setState(359);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<RelExpContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleRelExp);
        setState(354);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(355);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 7864320) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(356);
        addExp(0); 
      }
      setState(361);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- EqExpContext ------------------------------------------------------------------

SysYParser::EqExpContext::EqExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::RelExpContext* SysYParser::EqExpContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

SysYParser::EqExpContext* SysYParser::EqExpContext::eqExp() {
  return getRuleContext<SysYParser::EqExpContext>(0);
}

tree::TerminalNode* SysYParser::EqExpContext::EQ() {
  return getToken(SysYParser::EQ, 0);
}

tree::TerminalNode* SysYParser::EqExpContext::NE() {
  return getToken(SysYParser::NE, 0);
}


size_t SysYParser::EqExpContext::getRuleIndex() const {
  return SysYParser::RuleEqExp;
}

void SysYParser::EqExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEqExp(this);
}

void SysYParser::EqExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEqExp(this);
}


std::any SysYParser::EqExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitEqExp(this);
  else
    return visitor->visitChildren(this);
}


SysYParser::EqExpContext* SysYParser::eqExp() {
   return eqExp(0);
}

SysYParser::EqExpContext* SysYParser::eqExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::EqExpContext *_localctx = _tracker.createInstance<EqExpContext>(_ctx, parentState);
  SysYParser::EqExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 58;
  enterRecursionRule(_localctx, 58, SysYParser::RuleEqExp, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(363);
    relExp(0);
    _ctx->stop = _input->LT(-1);
    setState(370);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<EqExpContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleEqExp);
        setState(365);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(366);
        _la = _input->LA(1);
        if (!(_la == SysYParser::EQ

        || _la == SysYParser::NE)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(367);
        relExp(0); 
      }
      setState(372);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- LAndExpContext ------------------------------------------------------------------

SysYParser::LAndExpContext::LAndExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysYParser::EqExpContext *> SysYParser::LAndExpContext::eqExp() {
  return getRuleContexts<SysYParser::EqExpContext>();
}

SysYParser::EqExpContext* SysYParser::LAndExpContext::eqExp(size_t i) {
  return getRuleContext<SysYParser::EqExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::LAndExpContext::AND() {
  return getTokens(SysYParser::AND);
}

tree::TerminalNode* SysYParser::LAndExpContext::AND(size_t i) {
  return getToken(SysYParser::AND, i);
}


size_t SysYParser::LAndExpContext::getRuleIndex() const {
  return SysYParser::RuleLAndExp;
}

void SysYParser::LAndExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLAndExp(this);
}

void SysYParser::LAndExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLAndExp(this);
}


std::any SysYParser::LAndExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLAndExp(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::LAndExpContext* SysYParser::lAndExp() {
  LAndExpContext *_localctx = _tracker.createInstance<LAndExpContext>(_ctx, getState());
  enterRule(_localctx, 60, SysYParser::RuleLAndExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(373);
    eqExp(0);
    setState(378);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::AND) {
      setState(374);
      match(SysYParser::AND);
      setState(375);
      eqExp(0);
      setState(380);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LOrExpContext ------------------------------------------------------------------

SysYParser::LOrExpContext::LOrExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysYParser::LAndExpContext *> SysYParser::LOrExpContext::lAndExp() {
  return getRuleContexts<SysYParser::LAndExpContext>();
}

SysYParser::LAndExpContext* SysYParser::LOrExpContext::lAndExp(size_t i) {
  return getRuleContext<SysYParser::LAndExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::LOrExpContext::OR() {
  return getTokens(SysYParser::OR);
}

tree::TerminalNode* SysYParser::LOrExpContext::OR(size_t i) {
  return getToken(SysYParser::OR, i);
}


size_t SysYParser::LOrExpContext::getRuleIndex() const {
  return SysYParser::RuleLOrExp;
}

void SysYParser::LOrExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLOrExp(this);
}

void SysYParser::LOrExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLOrExp(this);
}


std::any SysYParser::LOrExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLOrExp(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::LOrExpContext* SysYParser::lOrExp() {
  LOrExpContext *_localctx = _tracker.createInstance<LOrExpContext>(_ctx, getState());
  enterRule(_localctx, 62, SysYParser::RuleLOrExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(381);
    lAndExp();
    setState(386);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::OR) {
      setState(382);
      match(SysYParser::OR);
      setState(383);
      lAndExp();
      setState(388);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstExpContext ------------------------------------------------------------------

SysYParser::ConstExpContext::ConstExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::AddExpContext* SysYParser::ConstExpContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}


size_t SysYParser::ConstExpContext::getRuleIndex() const {
  return SysYParser::RuleConstExp;
}

void SysYParser::ConstExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConstExp(this);
}

void SysYParser::ConstExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConstExp(this);
}


std::any SysYParser::ConstExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitConstExp(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ConstExpContext* SysYParser::constExp() {
  ConstExpContext *_localctx = _tracker.createInstance<ConstExpContext>(_ctx, getState());
  enterRule(_localctx, 64, SysYParser::RuleConstExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(389);
    addExp(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool SysYParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 26: return mulExpSempred(antlrcpp::downCast<MulExpContext *>(context), predicateIndex);
    case 27: return addExpSempred(antlrcpp::downCast<AddExpContext *>(context), predicateIndex);
    case 28: return relExpSempred(antlrcpp::downCast<RelExpContext *>(context), predicateIndex);
    case 29: return eqExpSempred(antlrcpp::downCast<EqExpContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool SysYParser::mulExpSempred(MulExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::addExpSempred(AddExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 1: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::relExpSempred(RelExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 2: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::eqExpSempred(EqExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 3: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void SysYParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  sysyParserInitialize();
#else
  ::antlr4::internal::call_once(sysyParserOnceFlag, sysyParserInitialize);
#endif
}

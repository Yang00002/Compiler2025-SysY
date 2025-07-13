
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
      "number", "unaryExp", "rParen", "funcRParams", "mulExp", "addExp", 
      "relExp", "eqExp", "lAndExp", "lOrExp", "constExp"
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
  	4,1,40,396,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,1,0,4,0,70,8,0,11,
  	0,12,0,71,1,0,1,0,1,1,1,1,3,1,78,8,1,1,2,1,2,3,2,82,8,2,1,3,1,3,1,3,1,
  	3,1,3,5,3,89,8,3,10,3,12,3,92,9,3,1,3,1,3,1,4,1,4,1,5,1,5,1,5,1,5,1,5,
  	5,5,103,8,5,10,5,12,5,106,9,5,1,5,1,5,1,5,1,6,1,6,1,6,1,6,1,6,5,6,116,
  	8,6,10,6,12,6,119,9,6,3,6,121,8,6,1,6,3,6,124,8,6,1,7,1,7,1,7,1,7,1,7,
  	5,7,131,8,7,10,7,12,7,134,9,7,3,7,136,8,7,1,7,3,7,139,8,7,1,8,1,8,1,8,
  	1,8,5,8,145,8,8,10,8,12,8,148,9,8,1,8,1,8,1,9,1,9,1,9,1,9,1,9,5,9,157,
  	8,9,10,9,12,9,160,9,9,1,9,1,9,1,9,1,9,1,9,5,9,167,8,9,10,9,12,9,170,9,
  	9,1,9,1,9,3,9,174,8,9,1,10,1,10,1,10,1,10,1,10,5,10,181,8,10,10,10,12,
  	10,184,9,10,3,10,186,8,10,1,10,3,10,189,8,10,1,11,1,11,1,11,1,11,1,11,
  	5,11,196,8,11,10,11,12,11,199,9,11,3,11,201,8,11,1,11,3,11,204,8,11,1,
  	12,1,12,1,12,1,12,3,12,210,8,12,1,12,1,12,1,12,1,13,1,13,1,14,1,14,1,
  	14,5,14,220,8,14,10,14,12,14,223,9,14,1,15,1,15,1,15,1,15,1,15,1,15,1,
  	15,1,15,5,15,233,8,15,10,15,12,15,236,9,15,3,15,238,8,15,1,16,1,16,5,
  	16,242,8,16,10,16,12,16,245,9,16,1,16,1,16,1,17,1,17,3,17,251,8,17,1,
  	18,1,18,1,18,1,18,1,18,1,18,3,18,259,8,18,1,18,1,18,1,18,1,18,1,18,1,
  	18,1,18,1,18,1,18,3,18,270,8,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,
  	18,1,18,1,18,1,18,1,18,3,18,284,8,18,1,18,3,18,287,8,18,1,19,1,19,1,20,
  	1,20,1,21,1,21,1,21,1,21,1,21,5,21,298,8,21,10,21,12,21,301,9,21,1,22,
  	1,22,1,22,1,22,1,22,1,22,3,22,309,8,22,1,23,1,23,1,24,1,24,1,24,1,24,
  	3,24,317,8,24,1,24,1,24,1,24,3,24,322,8,24,1,25,1,25,1,26,1,26,1,26,5,
  	26,329,8,26,10,26,12,26,332,9,26,1,27,1,27,1,27,1,27,1,27,1,27,5,27,340,
  	8,27,10,27,12,27,343,9,27,1,28,1,28,1,28,1,28,1,28,1,28,5,28,351,8,28,
  	10,28,12,28,354,9,28,1,29,1,29,1,29,1,29,1,29,1,29,5,29,362,8,29,10,29,
  	12,29,365,9,29,1,30,1,30,1,30,1,30,1,30,1,30,5,30,373,8,30,10,30,12,30,
  	376,9,30,1,31,1,31,1,31,5,31,381,8,31,10,31,12,31,384,9,31,1,32,1,32,
  	1,32,5,32,389,8,32,10,32,12,32,392,9,32,1,33,1,33,1,33,0,4,54,56,58,60,
  	34,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,
  	48,50,52,54,56,58,60,62,64,66,0,8,1,0,1,2,1,0,1,3,1,0,35,36,2,0,12,13,
  	25,25,1,0,14,16,1,0,12,13,1,0,19,22,1,0,17,18,411,0,69,1,0,0,0,2,77,1,
  	0,0,0,4,81,1,0,0,0,6,83,1,0,0,0,8,95,1,0,0,0,10,97,1,0,0,0,12,123,1,0,
  	0,0,14,138,1,0,0,0,16,140,1,0,0,0,18,173,1,0,0,0,20,188,1,0,0,0,22,203,
  	1,0,0,0,24,205,1,0,0,0,26,214,1,0,0,0,28,216,1,0,0,0,30,224,1,0,0,0,32,
  	239,1,0,0,0,34,250,1,0,0,0,36,286,1,0,0,0,38,288,1,0,0,0,40,290,1,0,0,
  	0,42,292,1,0,0,0,44,308,1,0,0,0,46,310,1,0,0,0,48,321,1,0,0,0,50,323,
  	1,0,0,0,52,325,1,0,0,0,54,333,1,0,0,0,56,344,1,0,0,0,58,355,1,0,0,0,60,
  	366,1,0,0,0,62,377,1,0,0,0,64,385,1,0,0,0,66,393,1,0,0,0,68,70,3,2,1,
  	0,69,68,1,0,0,0,70,71,1,0,0,0,71,69,1,0,0,0,71,72,1,0,0,0,72,73,1,0,0,
  	0,73,74,5,0,0,1,74,1,1,0,0,0,75,78,3,4,2,0,76,78,3,24,12,0,77,75,1,0,
  	0,0,77,76,1,0,0,0,78,3,1,0,0,0,79,82,3,6,3,0,80,82,3,16,8,0,81,79,1,0,
  	0,0,81,80,1,0,0,0,82,5,1,0,0,0,83,84,5,10,0,0,84,85,3,8,4,0,85,90,3,10,
  	5,0,86,87,5,32,0,0,87,89,3,10,5,0,88,86,1,0,0,0,89,92,1,0,0,0,90,88,1,
  	0,0,0,90,91,1,0,0,0,91,93,1,0,0,0,92,90,1,0,0,0,93,94,5,33,0,0,94,7,1,
  	0,0,0,95,96,7,0,0,0,96,9,1,0,0,0,97,104,5,34,0,0,98,99,5,28,0,0,99,100,
  	3,66,33,0,100,101,5,29,0,0,101,103,1,0,0,0,102,98,1,0,0,0,103,106,1,0,
  	0,0,104,102,1,0,0,0,104,105,1,0,0,0,105,107,1,0,0,0,106,104,1,0,0,0,107,
  	108,5,11,0,0,108,109,3,12,6,0,109,11,1,0,0,0,110,124,3,66,33,0,111,120,
  	5,30,0,0,112,117,3,14,7,0,113,114,5,32,0,0,114,116,3,14,7,0,115,113,1,
  	0,0,0,116,119,1,0,0,0,117,115,1,0,0,0,117,118,1,0,0,0,118,121,1,0,0,0,
  	119,117,1,0,0,0,120,112,1,0,0,0,120,121,1,0,0,0,121,122,1,0,0,0,122,124,
  	5,31,0,0,123,110,1,0,0,0,123,111,1,0,0,0,124,13,1,0,0,0,125,139,3,66,
  	33,0,126,135,5,30,0,0,127,132,3,14,7,0,128,129,5,32,0,0,129,131,3,14,
  	7,0,130,128,1,0,0,0,131,134,1,0,0,0,132,130,1,0,0,0,132,133,1,0,0,0,133,
  	136,1,0,0,0,134,132,1,0,0,0,135,127,1,0,0,0,135,136,1,0,0,0,136,137,1,
  	0,0,0,137,139,5,31,0,0,138,125,1,0,0,0,138,126,1,0,0,0,139,15,1,0,0,0,
  	140,141,3,8,4,0,141,146,3,18,9,0,142,143,5,32,0,0,143,145,3,18,9,0,144,
  	142,1,0,0,0,145,148,1,0,0,0,146,144,1,0,0,0,146,147,1,0,0,0,147,149,1,
  	0,0,0,148,146,1,0,0,0,149,150,5,33,0,0,150,17,1,0,0,0,151,158,5,34,0,
  	0,152,153,5,28,0,0,153,154,3,66,33,0,154,155,5,29,0,0,155,157,1,0,0,0,
  	156,152,1,0,0,0,157,160,1,0,0,0,158,156,1,0,0,0,158,159,1,0,0,0,159,174,
  	1,0,0,0,160,158,1,0,0,0,161,168,5,34,0,0,162,163,5,28,0,0,163,164,3,66,
  	33,0,164,165,5,29,0,0,165,167,1,0,0,0,166,162,1,0,0,0,167,170,1,0,0,0,
  	168,166,1,0,0,0,168,169,1,0,0,0,169,171,1,0,0,0,170,168,1,0,0,0,171,172,
  	5,11,0,0,172,174,3,20,10,0,173,151,1,0,0,0,173,161,1,0,0,0,174,19,1,0,
  	0,0,175,189,3,38,19,0,176,185,5,30,0,0,177,182,3,22,11,0,178,179,5,32,
  	0,0,179,181,3,22,11,0,180,178,1,0,0,0,181,184,1,0,0,0,182,180,1,0,0,0,
  	182,183,1,0,0,0,183,186,1,0,0,0,184,182,1,0,0,0,185,177,1,0,0,0,185,186,
  	1,0,0,0,186,187,1,0,0,0,187,189,5,31,0,0,188,175,1,0,0,0,188,176,1,0,
  	0,0,189,21,1,0,0,0,190,204,3,38,19,0,191,200,5,30,0,0,192,197,3,22,11,
  	0,193,194,5,32,0,0,194,196,3,22,11,0,195,193,1,0,0,0,196,199,1,0,0,0,
  	197,195,1,0,0,0,197,198,1,0,0,0,198,201,1,0,0,0,199,197,1,0,0,0,200,192,
  	1,0,0,0,200,201,1,0,0,0,201,202,1,0,0,0,202,204,5,31,0,0,203,190,1,0,
  	0,0,203,191,1,0,0,0,204,23,1,0,0,0,205,206,3,26,13,0,206,207,5,34,0,0,
  	207,209,5,26,0,0,208,210,3,28,14,0,209,208,1,0,0,0,209,210,1,0,0,0,210,
  	211,1,0,0,0,211,212,5,27,0,0,212,213,3,32,16,0,213,25,1,0,0,0,214,215,
  	7,1,0,0,215,27,1,0,0,0,216,221,3,30,15,0,217,218,5,32,0,0,218,220,3,30,
  	15,0,219,217,1,0,0,0,220,223,1,0,0,0,221,219,1,0,0,0,221,222,1,0,0,0,
  	222,29,1,0,0,0,223,221,1,0,0,0,224,225,3,8,4,0,225,237,5,34,0,0,226,227,
  	5,28,0,0,227,234,5,29,0,0,228,229,5,28,0,0,229,230,3,38,19,0,230,231,
  	5,29,0,0,231,233,1,0,0,0,232,228,1,0,0,0,233,236,1,0,0,0,234,232,1,0,
  	0,0,234,235,1,0,0,0,235,238,1,0,0,0,236,234,1,0,0,0,237,226,1,0,0,0,237,
  	238,1,0,0,0,238,31,1,0,0,0,239,243,5,30,0,0,240,242,3,34,17,0,241,240,
  	1,0,0,0,242,245,1,0,0,0,243,241,1,0,0,0,243,244,1,0,0,0,244,246,1,0,0,
  	0,245,243,1,0,0,0,246,247,5,31,0,0,247,33,1,0,0,0,248,251,3,4,2,0,249,
  	251,3,36,18,0,250,248,1,0,0,0,250,249,1,0,0,0,251,35,1,0,0,0,252,253,
  	3,42,21,0,253,254,5,11,0,0,254,255,3,38,19,0,255,256,5,33,0,0,256,287,
  	1,0,0,0,257,259,3,38,19,0,258,257,1,0,0,0,258,259,1,0,0,0,259,260,1,0,
  	0,0,260,287,5,33,0,0,261,287,3,32,16,0,262,263,5,4,0,0,263,264,5,26,0,
  	0,264,265,3,40,20,0,265,266,5,27,0,0,266,269,3,36,18,0,267,268,5,5,0,
  	0,268,270,3,36,18,0,269,267,1,0,0,0,269,270,1,0,0,0,270,287,1,0,0,0,271,
  	272,5,6,0,0,272,273,5,26,0,0,273,274,3,40,20,0,274,275,5,27,0,0,275,276,
  	3,36,18,0,276,287,1,0,0,0,277,278,5,8,0,0,278,287,5,33,0,0,279,280,5,
  	7,0,0,280,287,5,33,0,0,281,283,5,9,0,0,282,284,3,38,19,0,283,282,1,0,
  	0,0,283,284,1,0,0,0,284,285,1,0,0,0,285,287,5,33,0,0,286,252,1,0,0,0,
  	286,258,1,0,0,0,286,261,1,0,0,0,286,262,1,0,0,0,286,271,1,0,0,0,286,277,
  	1,0,0,0,286,279,1,0,0,0,286,281,1,0,0,0,287,37,1,0,0,0,288,289,3,56,28,
  	0,289,39,1,0,0,0,290,291,3,64,32,0,291,41,1,0,0,0,292,299,5,34,0,0,293,
  	294,5,28,0,0,294,295,3,38,19,0,295,296,5,29,0,0,296,298,1,0,0,0,297,293,
  	1,0,0,0,298,301,1,0,0,0,299,297,1,0,0,0,299,300,1,0,0,0,300,43,1,0,0,
  	0,301,299,1,0,0,0,302,303,5,26,0,0,303,304,3,38,19,0,304,305,5,27,0,0,
  	305,309,1,0,0,0,306,309,3,42,21,0,307,309,3,46,23,0,308,302,1,0,0,0,308,
  	306,1,0,0,0,308,307,1,0,0,0,309,45,1,0,0,0,310,311,7,2,0,0,311,47,1,0,
  	0,0,312,322,3,44,22,0,313,314,5,34,0,0,314,316,5,26,0,0,315,317,3,52,
  	26,0,316,315,1,0,0,0,316,317,1,0,0,0,317,318,1,0,0,0,318,322,3,50,25,
  	0,319,320,7,3,0,0,320,322,3,48,24,0,321,312,1,0,0,0,321,313,1,0,0,0,321,
  	319,1,0,0,0,322,49,1,0,0,0,323,324,5,27,0,0,324,51,1,0,0,0,325,330,3,
  	38,19,0,326,327,5,32,0,0,327,329,3,38,19,0,328,326,1,0,0,0,329,332,1,
  	0,0,0,330,328,1,0,0,0,330,331,1,0,0,0,331,53,1,0,0,0,332,330,1,0,0,0,
  	333,334,6,27,-1,0,334,335,3,48,24,0,335,341,1,0,0,0,336,337,10,1,0,0,
  	337,338,7,4,0,0,338,340,3,48,24,0,339,336,1,0,0,0,340,343,1,0,0,0,341,
  	339,1,0,0,0,341,342,1,0,0,0,342,55,1,0,0,0,343,341,1,0,0,0,344,345,6,
  	28,-1,0,345,346,3,54,27,0,346,352,1,0,0,0,347,348,10,1,0,0,348,349,7,
  	5,0,0,349,351,3,54,27,0,350,347,1,0,0,0,351,354,1,0,0,0,352,350,1,0,0,
  	0,352,353,1,0,0,0,353,57,1,0,0,0,354,352,1,0,0,0,355,356,6,29,-1,0,356,
  	357,3,56,28,0,357,363,1,0,0,0,358,359,10,1,0,0,359,360,7,6,0,0,360,362,
  	3,56,28,0,361,358,1,0,0,0,362,365,1,0,0,0,363,361,1,0,0,0,363,364,1,0,
  	0,0,364,59,1,0,0,0,365,363,1,0,0,0,366,367,6,30,-1,0,367,368,3,58,29,
  	0,368,374,1,0,0,0,369,370,10,1,0,0,370,371,7,7,0,0,371,373,3,58,29,0,
  	372,369,1,0,0,0,373,376,1,0,0,0,374,372,1,0,0,0,374,375,1,0,0,0,375,61,
  	1,0,0,0,376,374,1,0,0,0,377,382,3,60,30,0,378,379,5,24,0,0,379,381,3,
  	60,30,0,380,378,1,0,0,0,381,384,1,0,0,0,382,380,1,0,0,0,382,383,1,0,0,
  	0,383,63,1,0,0,0,384,382,1,0,0,0,385,390,3,62,31,0,386,387,5,23,0,0,387,
  	389,3,62,31,0,388,386,1,0,0,0,389,392,1,0,0,0,390,388,1,0,0,0,390,391,
  	1,0,0,0,391,65,1,0,0,0,392,390,1,0,0,0,393,394,3,56,28,0,394,67,1,0,0,
  	0,42,71,77,81,90,104,117,120,123,132,135,138,146,158,168,173,182,185,
  	188,197,200,203,209,221,234,237,243,250,258,269,283,286,299,308,316,321,
  	330,341,352,363,374,382,390
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
    setState(69); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(68);
      compDecl();
      setState(71); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1038) != 0));
    setState(73);
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
    setState(77);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
    case 1: {
      setState(75);
      decl();
      break;
    }

    case 2: {
      setState(76);
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
    setState(81);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::CONST: {
        enterOuterAlt(_localctx, 1);
        setState(79);
        constDecl();
        break;
      }

      case SysYParser::INT:
      case SysYParser::FLOAT: {
        enterOuterAlt(_localctx, 2);
        setState(80);
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
    setState(83);
    match(SysYParser::CONST);
    setState(84);
    bType();
    setState(85);
    constDef();
    setState(90);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::COMMA) {
      setState(86);
      match(SysYParser::COMMA);
      setState(87);
      constDef();
      setState(92);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(93);
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
    setState(95);
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
    setState(97);
    match(SysYParser::ID);
    setState(104);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::LBRACK) {
      setState(98);
      match(SysYParser::LBRACK);
      setState(99);
      constExp();
      setState(100);
      match(SysYParser::RBRACK);
      setState(106);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(107);
    match(SysYParser::ASSIGN);
    setState(108);
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
    setState(123);
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
        setState(110);
        constExp();
        break;
      }

      case SysYParser::LBRACE: {
        enterOuterAlt(_localctx, 2);
        setState(111);
        match(SysYParser::LBRACE);
        setState(120);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 121433501696) != 0)) {
          setState(112);
          constArrayInitVal();
          setState(117);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysYParser::COMMA) {
            setState(113);
            match(SysYParser::COMMA);
            setState(114);
            constArrayInitVal();
            setState(119);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(122);
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
    setState(138);
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
        setState(125);
        constExp();
        break;
      }

      case SysYParser::LBRACE: {
        enterOuterAlt(_localctx, 2);
        setState(126);
        match(SysYParser::LBRACE);
        setState(135);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 121433501696) != 0)) {
          setState(127);
          constArrayInitVal();
          setState(132);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysYParser::COMMA) {
            setState(128);
            match(SysYParser::COMMA);
            setState(129);
            constArrayInitVal();
            setState(134);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(137);
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
    setState(140);
    bType();
    setState(141);
    varDef();
    setState(146);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::COMMA) {
      setState(142);
      match(SysYParser::COMMA);
      setState(143);
      varDef();
      setState(148);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(149);
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
    setState(173);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(151);
      match(SysYParser::ID);
      setState(158);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysYParser::LBRACK) {
        setState(152);
        match(SysYParser::LBRACK);
        setState(153);
        constExp();
        setState(154);
        match(SysYParser::RBRACK);
        setState(160);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(161);
      match(SysYParser::ID);
      setState(168);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysYParser::LBRACK) {
        setState(162);
        match(SysYParser::LBRACK);
        setState(163);
        constExp();
        setState(164);
        match(SysYParser::RBRACK);
        setState(170);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(171);
      match(SysYParser::ASSIGN);
      setState(172);
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
    setState(188);
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
        setState(175);
        exp();
        break;
      }

      case SysYParser::LBRACE: {
        enterOuterAlt(_localctx, 2);
        setState(176);
        match(SysYParser::LBRACE);
        setState(185);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 121433501696) != 0)) {
          setState(177);
          arrayInitVal();
          setState(182);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysYParser::COMMA) {
            setState(178);
            match(SysYParser::COMMA);
            setState(179);
            arrayInitVal();
            setState(184);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(187);
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
    setState(203);
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
        setState(190);
        exp();
        break;
      }

      case SysYParser::LBRACE: {
        enterOuterAlt(_localctx, 2);
        setState(191);
        match(SysYParser::LBRACE);
        setState(200);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 121433501696) != 0)) {
          setState(192);
          arrayInitVal();
          setState(197);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysYParser::COMMA) {
            setState(193);
            match(SysYParser::COMMA);
            setState(194);
            arrayInitVal();
            setState(199);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(202);
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
    setState(205);
    funcType();
    setState(206);
    match(SysYParser::ID);
    setState(207);
    match(SysYParser::LPAREN);
    setState(209);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysYParser::INT

    || _la == SysYParser::FLOAT) {
      setState(208);
      funcFParams();
    }
    setState(211);
    match(SysYParser::RPAREN);
    setState(212);
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
    setState(214);
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
    setState(216);
    funcFParam();
    setState(221);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::COMMA) {
      setState(217);
      match(SysYParser::COMMA);
      setState(218);
      funcFParam();
      setState(223);
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
    setState(224);
    bType();
    setState(225);
    match(SysYParser::ID);
    setState(237);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysYParser::LBRACK) {
      setState(226);
      match(SysYParser::LBRACK);
      setState(227);
      match(SysYParser::RBRACK);
      setState(234);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysYParser::LBRACK) {
        setState(228);
        match(SysYParser::LBRACK);
        setState(229);
        exp();
        setState(230);
        match(SysYParser::RBRACK);
        setState(236);
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
    setState(239);
    match(SysYParser::LBRACE);
    setState(243);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 130023438294) != 0)) {
      setState(240);
      blockItem();
      setState(245);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(246);
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
    setState(250);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::INT:
      case SysYParser::FLOAT:
      case SysYParser::CONST: {
        enterOuterAlt(_localctx, 1);
        setState(248);
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
        setState(249);
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
    setState(286);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(252);
      lVal();
      setState(253);
      match(SysYParser::ASSIGN);
      setState(254);
      exp();
      setState(255);
      match(SysYParser::SEMICO);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(258);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 120359759872) != 0)) {
        setState(257);
        exp();
      }
      setState(260);
      match(SysYParser::SEMICO);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(261);
      block();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(262);
      match(SysYParser::IF);
      setState(263);
      match(SysYParser::LPAREN);
      setState(264);
      cond();
      setState(265);
      match(SysYParser::RPAREN);
      setState(266);
      stmt();
      setState(269);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx)) {
      case 1: {
        setState(267);
        match(SysYParser::ELSE);
        setState(268);
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
      setState(271);
      match(SysYParser::WHILE);
      setState(272);
      match(SysYParser::LPAREN);
      setState(273);
      cond();
      setState(274);
      match(SysYParser::RPAREN);
      setState(275);
      stmt();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(277);
      match(SysYParser::BREAK);
      setState(278);
      match(SysYParser::SEMICO);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(279);
      match(SysYParser::CONTINUE);
      setState(280);
      match(SysYParser::SEMICO);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(281);
      match(SysYParser::RETURN);
      setState(283);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 120359759872) != 0)) {
        setState(282);
        exp();
      }
      setState(285);
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
    setState(288);
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
    setState(290);
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
    setState(292);
    match(SysYParser::ID);
    setState(299);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(293);
        match(SysYParser::LBRACK);
        setState(294);
        exp();
        setState(295);
        match(SysYParser::RBRACK); 
      }
      setState(301);
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
    setState(308);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::LPAREN: {
        enterOuterAlt(_localctx, 1);
        setState(302);
        match(SysYParser::LPAREN);
        setState(303);
        exp();
        setState(304);
        match(SysYParser::RPAREN);
        break;
      }

      case SysYParser::ID: {
        enterOuterAlt(_localctx, 2);
        setState(306);
        lVal();
        break;
      }

      case SysYParser::IntConst:
      case SysYParser::FloatConst: {
        enterOuterAlt(_localctx, 3);
        setState(307);
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
    setState(310);
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

SysYParser::RParenContext* SysYParser::UnaryExpContext::rParen() {
  return getRuleContext<SysYParser::RParenContext>(0);
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
    setState(321);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(312);
      primaryExp();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(313);
      match(SysYParser::ID);
      setState(314);
      match(SysYParser::LPAREN);
      setState(316);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 120359759872) != 0)) {
        setState(315);
        funcRParams();
      }
      setState(318);
      rParen();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(319);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 33566720) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(320);
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

//----------------- RParenContext ------------------------------------------------------------------

SysYParser::RParenContext::RParenContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::RParenContext::RPAREN() {
  return getToken(SysYParser::RPAREN, 0);
}


size_t SysYParser::RParenContext::getRuleIndex() const {
  return SysYParser::RuleRParen;
}

void SysYParser::RParenContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRParen(this);
}

void SysYParser::RParenContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysYListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRParen(this);
}


std::any SysYParser::RParenContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitRParen(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::RParenContext* SysYParser::rParen() {
  RParenContext *_localctx = _tracker.createInstance<RParenContext>(_ctx, getState());
  enterRule(_localctx, 50, SysYParser::RuleRParen);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(323);
    match(SysYParser::RPAREN);
   
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
  enterRule(_localctx, 52, SysYParser::RuleFuncRParams);
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
    setState(325);
    exp();
    setState(330);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::COMMA) {
      setState(326);
      match(SysYParser::COMMA);
      setState(327);
      exp();
      setState(332);
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
  size_t startState = 54;
  enterRecursionRule(_localctx, 54, SysYParser::RuleMulExp, precedence);

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
    setState(334);
    unaryExp();
    _ctx->stop = _input->LT(-1);
    setState(341);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<MulExpContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleMulExp);
        setState(336);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(337);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 114688) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(338);
        unaryExp(); 
      }
      setState(343);
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
  size_t startState = 56;
  enterRecursionRule(_localctx, 56, SysYParser::RuleAddExp, precedence);

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
    setState(345);
    mulExp(0);
    _ctx->stop = _input->LT(-1);
    setState(352);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<AddExpContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleAddExp);
        setState(347);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(348);
        _la = _input->LA(1);
        if (!(_la == SysYParser::ADD

        || _la == SysYParser::SUB)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(349);
        mulExp(0); 
      }
      setState(354);
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
  size_t startState = 58;
  enterRecursionRule(_localctx, 58, SysYParser::RuleRelExp, precedence);

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
    setState(356);
    addExp(0);
    _ctx->stop = _input->LT(-1);
    setState(363);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<RelExpContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleRelExp);
        setState(358);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(359);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 7864320) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(360);
        addExp(0); 
      }
      setState(365);
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
  size_t startState = 60;
  enterRecursionRule(_localctx, 60, SysYParser::RuleEqExp, precedence);

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
    setState(367);
    relExp(0);
    _ctx->stop = _input->LT(-1);
    setState(374);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<EqExpContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleEqExp);
        setState(369);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(370);
        _la = _input->LA(1);
        if (!(_la == SysYParser::EQ

        || _la == SysYParser::NE)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(371);
        relExp(0); 
      }
      setState(376);
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
  enterRule(_localctx, 62, SysYParser::RuleLAndExp);
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
    setState(377);
    eqExp(0);
    setState(382);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::AND) {
      setState(378);
      match(SysYParser::AND);
      setState(379);
      eqExp(0);
      setState(384);
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
  enterRule(_localctx, 64, SysYParser::RuleLOrExp);
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
    setState(385);
    lAndExp();
    setState(390);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::OR) {
      setState(386);
      match(SysYParser::OR);
      setState(387);
      lAndExp();
      setState(392);
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
  enterRule(_localctx, 66, SysYParser::RuleConstExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(393);
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
    case 27: return mulExpSempred(antlrcpp::downCast<MulExpContext *>(context), predicateIndex);
    case 28: return addExpSempred(antlrcpp::downCast<AddExpContext *>(context), predicateIndex);
    case 29: return relExpSempred(antlrcpp::downCast<RelExpContext *>(context), predicateIndex);
    case 30: return eqExpSempred(antlrcpp::downCast<EqExpContext *>(context), predicateIndex);

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
